#include "text_editor.h"
#include "helper_funcs.h"

void clear_char() {
	for (int i = 0; i < 80; i++) {
		for (int j = 0; j < 60; j++) {
			*(char_ptr + (j << 7) + i) = 0x00;
		}
	}
}

void draw_vga_underscore(int pos, short int color) {
	int col = pos % 80;
	int row = (int) pos / 80;
	int pixel_x = col * 4;
	int pixel_y = row * 4 + 3; //add 3 to get the bottom row of character cell
	
	for (int i = 0; i < 4; ++i) {
		*(vga_ptr + (pixel_y << 9) + (pixel_x + i)) = color;
	}
}

char ps2_to_ascii(unsigned char code) {
    switch(code) {
        case 0x1C: return 'a';
        case 0x32: return 'b';
        case 0x21: return 'c';
        case 0x23: return 'd';
        case 0x24: return 'e';
        case 0x2B: return 'f';
        case 0x34: return 'g';
        case 0x33: return 'h';
        case 0x43: return 'i';
        case 0x3B: return 'j';
        case 0x42: return 'k';
        case 0x4B: return 'l';
        case 0x3A: return 'm';
        case 0x31: return 'n';
        case 0x44: return 'o';
        case 0x4D: return 'p';
        case 0x15: return 'q';
        case 0x2D: return 'r';
        case 0x1B: return 's';
        case 0x2C: return 't';
        case 0x3C: return 'u';
        case 0x2A: return 'v';
        case 0x1D: return 'w';
        case 0x22: return 'x';
        case 0x35: return 'y';
        case 0x1A: return 'z';
        case 0x29: return ' ';   // space
        case 0x5A: return '\n';  // enter
        default:   return 0;
    }
}

void put_char_in_buffer(char letter, int position) {
	int x = position % 80;
	int y = (int) position / 80;
	*(char_ptr + (y << 7) + x) = letter;
}

void text_editor_command() {
	clear_char();	//Bring back previous content
	for (int i = 0; i < 80; i++) {
		for (int j = 0; j < 60; j++) {
			*(char_ptr + (j << 7) + i) = text_content[i][j];
		}
	}
	
	draw_vga_underscore(position, 0xffff);
	
	char curr_user_buff[80] = {'>', 0};	//Size for a whole row (80 chars should be plenty)
	int curr_buff_size = 1;			//Keep track of buffer sizes
	
	print_row(curr_user_buff, 58);
	
	unsigned char ps2_code = 0;
	int ps2_scan;
	
	int skip_next = 0;
	while (1) {
		ps2_scan = *ps2_ptr;
		if (ps2_scan & 0x8000) {	//Something to read from buffer
			ps2_code = (unsigned char) (ps2_scan & 0xff);		//Get ps2 data
			
			if (skip_next > 0) {
				skip_next--;
			}
			else if (ps2_code == 0xf0) {
				skip_next = 1;
			}
			else {
				if (ps2_code == 0x5A) {			//If user presses enter, handle current command
					//save work before processing command
					for (int i = 0; i < 80; i++) {
						for (int j = 0; j < 60; j++) {
							text_content[i][j] = *(char_ptr + (j << 7) + i);
						}
					}
					draw_vga_underscore(position, 0x0000);
					
					int result = execute_editor_command(curr_user_buff, curr_buff_size);
					
					//Now, erase current command buff
					for (int i = 1; i < 80; ++i) {
						curr_user_buff[i] = 0;
					}
					curr_buff_size = 1;
					
					if (result == SAVE_AND_EXIT) {
						break;
					}
					
					else if (result == TEXT_HELP) {
						clear_char();
						//bring back prev text
						for (int i = 0; i < 80; i++) {
							for (int j = 0; j < 60; j++) {
								*(char_ptr + (j << 7) + i) = text_content[i][j];
							}
						}
						draw_vga_underscore(position, 0xffff);
					}
					
					else if (result == DELETE_ROW) {
						//delete row
						position = position - (position % 80);
						for (int i = 0; i < 80; ++i) {
							*(char_ptr + ((int) (position / 80) << 7) + i % 80) = 0x00;
						}
						//blink_under = 0x00;
						draw_vga_underscore(position, 0xffff);
					}
					
					//else continue normally
				}
				
				else if (ps2_code == 0x43 && curr_user_buff[curr_buff_size-1] == '>') {	//Enter insert mode
					for (int i = 1; i < 80; ++i) {
						curr_user_buff[i] = 0;
					}
					curr_buff_size = 1;
					text_editor_insert();
				}
				
				else if (ps2_code == 0x76) {
					print_row("Use \"sw\" to save and exit.", 59);
				}

				else if (ps2_code == 0x66) {	//Backspace
					if (curr_buff_size > 1) {
						curr_buff_size--;
					}
					curr_user_buff[curr_buff_size] = 0;		
				}

				else {
					char ascii_char = ps2_to_ascii(ps2_code);
					if (curr_buff_size < 78) {		//Leave 1 char at the end for null-termination always
						curr_user_buff[curr_buff_size++] = ascii_char;
					}
				}
				print_row(curr_user_buff, 58);
			}
		}
	}
}


void text_editor_insert() {
	int skip_next = 0;
	int extended = 0;
	
	while (1) {
		int ps2_scan = *ps2_ptr;
		if (ps2_scan & 0x8000) { //Data available to read
			unsigned char ps2_code = (unsigned char)(ps2_scan & 0xff);
			
			if (skip_next > 0) {
				skip_next--;
			}
			else if (ps2_code == 0xf0) {
				skip_next = 1;
				extended = 0;
			}
			else if (ps2_code == 0xe0) {
				extended = 1;	//handle extended ps2 key
			}
			else {
				if (extended) {
					extended = 0;
					
					//put_char_in_buffer(blink_under, position);	//To ensure blinking underscore cleared
					draw_vga_underscore(position, 0x0000); //clear old underscore always
					position_blinker = 0;
				
					if (ps2_code == 0x6b && position > 0) {		//left arrow
						position--;
						int new_line = 0;
						while (position > 0 && *(char_ptr + ((int)(position / 80) << 7) + position % 80) == 0 && position % 80) { //case where we have new-line
							position--;
							new_line = 1;
						}
						if (new_line && *(char_ptr + ((int)(position / 80) << 7) + position % 80) != 0) position++;
					}
					else if (ps2_code == 0x74 && position < 4799) { //right arrow
						if (*(char_ptr + ((int)(position / 80) << 7) + position % 80) != 0) {
                        	position++;
                        }
						else {
							if (position % 80 == 0) {
								position++;
							}
							while (*(char_ptr + ((int)(position / 80) << 7) + position % 80) == 0 && position % 80 && position < 4799) {
								position++;
							}
						}
					}
					//get character under blinker
					//blink_under = *(char_ptr + ((int)(position / 80) << 7) + position % 80);
				}
				else {
					char ascii_char = ps2_to_ascii(ps2_code);
					if (ps2_code == 0x76) { 		//Escape to exit insert mode
						draw_vga_underscore(position, 0xffff);
						break;
					}

					//put_char_in_buffer(blink_under, position);	//To ensure blinking underscore cleared
					draw_vga_underscore(position, 0x0000);
					position_blinker = 0;

					if (ascii_char == '\n') {			//Enter
						position++;
						while (position % 80) {				//Fill row with zeros until next row
							put_char_in_buffer(0x00, position++);
						}
					}
					else if (ps2_code == 0x66) {		//Backspace
						position = (!position) ? position : position - 1;										//Limit position to min of 0
						if (*(char_ptr + ((int)(position / 80) << 7) + position % 80) == 0) {					//We're dealing with an Enter
							while (*(char_ptr + ((int)(position / 80) << 7) + position % 80) == 0 && position > 0)		//Go back through all zeros until reach a character
								position--;
							if (position)
								position++;
						}
						else
							put_char_in_buffer(0x00, position);
					}
					else if (ascii_char) {
						put_char_in_buffer(ascii_char, position);
						position++;
					}
					//blink_under = *(char_ptr + ((int)(position / 80) << 7) + position % 80);
				}
			}
		}
		if (*timer_ptr & 0b1) {
			position_blinker = (position_blinker) ? 0 : 1;
			if (position_blinker) draw_vga_underscore(position, 0xffff); //put_char_in_buffer('_', position);
			else draw_vga_underscore(position, 0x0000); //put_char_in_buffer(blink_under, position);
			*timer_ptr = 0;
		}
	}
}

void text_editor() {
	clear_char();
	text_editor_command();
}


int execute_editor_command(char* command, int size) {		//This function handles whatever is in the curr_user_buff when the user presses Enter by launching that app
	if (*(command + 1) == 0) {
		print_row(" ", 59);
		return 0;
	}
	else if (strcmp(command + 1, "sw")) {	//Save and exit
		print_row("Saving work.", 59);
		return SAVE_AND_EXIT;
	}
	else if (strcmp(command + 1, "dr")) {	//Delete row
		print_row("Deleted row.", 59);
		return DELETE_ROW;
	}
	else if (strcmp(command + 1, "h")) {	//Help
		print_row("Opening help dialog.", 59);
		open_help_dialog(TEXT_HELP);
		return TEXT_HELP;
	}
	else {
		print_row("Invalid command. \"h\" to open help dialog.", 59);
		return 0;
	}
}

