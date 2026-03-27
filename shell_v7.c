#include "text_editor.h"
#include "helper_funcs.h"
#include "picture_array.h"
#include "visual_system.h"


void main_shell() {
	clear_screen();
	clear_char();
	/*
	char curr_user_buff[80] = {'>', 0};	//Size for a whole row (80 chars should be plenty)
	char prev_user_buffs[5][80] = {0};	//Keep 5 last lines of shell for user memory
	*/
	char user_buffs[SHELL_ROWS][80] = {0};
	int buff_sizes[SHELL_ROWS] = {0};			//Start at 1 since first char is always '>' and is immutable
	short int buff_colors[SHELL_ROWS] = {0};

	for (int i = 0; i < SHELL_ROWS; ++i) {		//Initialize prev buffs with '>'
		user_buffs[i][0] = '>';
		buff_sizes[i] = 1;
		print_row(user_buffs[i], i);
	}

	buff_colors[ACTIVE_ROW] = 0x001f; //blue for active row
	draw_shell_arrow_box(ACTIVE_ROW, buff_colors[ACTIVE_ROW]);

	unsigned char ps2_code = 0;
	int ps2_scan;
	
	int skip_next = 0;
	int extended = 0;
	int last_line = ACTIVE_ROW;

	while (1) {
		flush_mouse();
		ps2_scan = *ps2_ptr;
		if (ps2_scan & 0x8000) {	//Something to read from buffer
			ps2_code = (unsigned char) (ps2_scan & 0xff);		//Get ps2 data
			
			if (skip_next > 0) {
				skip_next--;
			}
			else if (ps2_code == 0xf0) {
				skip_next = 1;
				extended = 0;
			}
			else if (ps2_code == 0xe0) {	//handle extended ps2 code
				extended = 1;
			}
			else {
				if (extended) {
					extended = 0;
					if (ps2_code == 0x75 && last_line > 0) {		//Up arrow handling
						//copy row at --position into active row
						last_line--;
						for (int i = 1; i < 80; ++i) {
							user_buffs[ACTIVE_ROW][i] = user_buffs[last_line][i];
						}
						buff_sizes[ACTIVE_ROW] = buff_sizes[last_line];
					}
					else if (ps2_code == 0x72) {				//Down arrow handling
						last_line++;
						if (last_line >= ACTIVE_ROW) {
							last_line = ACTIVE_ROW;
							for (int i = 1; i < 80; ++i) {
								user_buffs[ACTIVE_ROW][i] = 0x00;
							}
							buff_sizes[ACTIVE_ROW] = 1;
						}
						else {
							for (int i = 1; i < 80; ++i) {
								user_buffs[ACTIVE_ROW][i] = user_buffs[last_line][i];
							}
							buff_sizes[ACTIVE_ROW] = buff_sizes[last_line];
						}
					}
				}

				else {
					if (ps2_code == 0x5A) {			//If user presses enter, handle current command
						for (int i = 0; i < SHELL_ROWS; ++i) {
							draw_shell_arrow_box(i, 0x0000);
						}
						buff_colors[ACTIVE_ROW] = execute_command(user_buffs[ACTIVE_ROW], buff_sizes[ACTIVE_ROW]);
						//Now, copy current buffer and its size into previous to keep track and clear current
						//in other words, shift buffers up
						for (int i = 0; i < SHELL_ROWS - 1; ++i) {
							for (int j = 1; j < 80; ++j) {
								user_buffs[i][j] = user_buffs[i+1][j];
							}
							buff_sizes[i] = buff_sizes[i+1];
							buff_colors[i] = buff_colors[i+1];
						}
						for (int i = 1; i < 80; ++i) {
							user_buffs[ACTIVE_ROW][i] = 0;
						}
						buff_sizes[ACTIVE_ROW] = 1;
						buff_colors[ACTIVE_ROW] = 0x001f;

						for (int i = 0; i < SHELL_ROWS; ++i) {
							print_row(user_buffs[i], i);
							draw_shell_arrow_box(i, buff_colors[i]);
						}
						last_line = ACTIVE_ROW;
					}

					else if (ps2_code == 0x66) {	//Backspace
						if (buff_sizes[ACTIVE_ROW] > 1) {
							buff_sizes[ACTIVE_ROW]--;
						}
						user_buffs[ACTIVE_ROW][buff_sizes[ACTIVE_ROW]] = 0;		
					}

					else if (ps2_code != 0x76) {	//Not ESC 		code for ESC is 0x76 (ESC to close program)
						char ascii_char = ps2_to_ascii(ps2_code);
						if (buff_sizes[ACTIVE_ROW] < 78) {		//Leave 1 char at the end for null-termination always
							user_buffs[ACTIVE_ROW][buff_sizes[ACTIVE_ROW]++] = ascii_char;
						}
					}
					
					else break;		//Escape pressed
				}

				print_row(user_buffs[ACTIVE_ROW], ACTIVE_ROW);
			}
		}
	
	}
}

int main(void) {
	//Set up timer
	*timer_ptr = 0;
	*(timer_ptr + 2) = BLINK_DELAY;
	*(timer_ptr + 3) = (BLINK_DELAY >> 16);
	*(timer_ptr + 1) = 0b0110;

	//set up mouse
	init_mouse();
	
	main_shell();		//Run main shell loop
	print_row(">exited.", 5);
	
	return 0;
}

