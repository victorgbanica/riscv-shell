#include "text_editor.h"
#include "helper_funcs.h"
#include "picture_array.h"


//new functions with screen panning
char get_char_from_content(int col, int row) {
    if (col < 0 || col >= 120 || row < 0 || row >= 60) return 0;
    return text_content[col][row];
}

void put_char_in_content(char letter, int col, int row) {
    if (col < 0 || col >= 120 || row < 0 || row >= 60) return;
    text_content[col][row] = letter;
}

void redraw_screen() {
    for (int row = 0; row < 58; ++row) {
        for (int col = 0; col < 80; ++col) {
            int logical_col = col + view_offset;
            char c = (logical_col < 120) ? text_content[logical_col][row] : 0;
            *(char_ptr + (row << 7) + col) = c;
        }
    }
}

void update_view() {
    //Update offset if the cursor is off the screen
    int new_offset = view_offset;
    if (cursor_col >= view_offset + 79) {
        new_offset = cursor_col - 79;
    }
    else if (cursor_col < view_offset) {
        new_offset = cursor_col;
    }

    //If an update happened, redraw screen to accomodate pan
    if (new_offset != view_offset) {
        view_offset = new_offset;
        redraw_screen();
    }
}

//modified to accomodate for offset
void draw_vga_underscore_cursor(short int color) {
    int display_col = cursor_col - view_offset;
    if (display_col < 0 || display_col >= 80) return;
    int pixel_x = display_col * 4;
    int pixel_y = cursor_row * 4 + 3;
    for (int i = 0; i < 4; ++i)
        *(vga_ptr + (pixel_y << 9) + (pixel_x + i)) = color;
}

int find_row_end(int row) {
    int col = 0;
    while (col < 119 && text_content[col][row]) {
        col++;
    }
    return col;
}

//useful for inserting a character in place
void shift_row_right(int from_col, int row) {
    int end = find_row_end(row);
    if (end >= 119) return; //row is full, can't shift any further
    while (end > from_col) {
        text_content[end][row] = text_content[end-1][row];
        end--;
    }
    text_content[from_col][row] = 0;
}

//useful for backspacing
void shift_row_left(int from_col, int row) {
    int col = from_col;
    while (col < 119 && text_content[col][row]) {
        text_content[col][row] = text_content[col+1][row];
        col++;
    }
    text_content[col][row] = 0;
}
//end of new screen panning functions



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
	
	draw_vga_underscore_cursor(0xffff);
	
	char curr_user_buff[80] = {'>', 0};	//Size for a whole row (80 chars should be plenty)
	int curr_buff_size = 1;			//Keep track of buffer sizes
	
	print_row(curr_user_buff, 58);
	
	unsigned char ps2_code = 0;
	int ps2_scan;
	
	int skip_next = 0;
	int extended = 0;
	while (1) {
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
			else if (ps2_code == 0xe0) {
				extended = 1;	//handle extended ps2 key
			}
			else {
				if (extended) {
					extended = 0;
					continue;
				}
				if (ps2_code == 0x5A) {			//If user presses enter, handle current command
					draw_vga_underscore_cursor(0x0000);
					
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
						redraw_screen();
						draw_vga_underscore_cursor(0xffff);
					}
					
					else if (result == DELETE_ROW) {
						//delete row
						cursor_col = 0;
						for (int i = 0; i < 120; ++i) {
							text_content[i][cursor_row] = 0;
						}
						redraw_screen();
                        draw_vga_underscore_cursor(0xffff);
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
				
				else if (ps2_code == 0x76) {    //esc
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
					draw_vga_underscore_cursor(0x0000); //clear old underscore always
					position_blinker = 0;
				
					if (ps2_code == 0x6b) {		//left arrow
                        if (cursor_col > 0)
						    cursor_col--;
                        else if (cursor_row > 0) {  //wrap around to prev row
                            cursor_row--;
                            cursor_col = find_row_end(cursor_row);
                        }
                    }
					else if (ps2_code == 0x74) { //right arrow
                        if (cursor_col < find_row_end(cursor_row)) {
                            cursor_col++;
                        }
                        else if (cursor_row < 57) {
                            cursor_row++;
                            cursor_col = 0;
                        }
					}
                    else if (ps2_code == 0x75) { //up arrow
                        if (cursor_row > 0) {
                            cursor_row--;
                            if (text_content[cursor_col][cursor_row] == 0) cursor_col = find_row_end(cursor_row);
                        }
                        else {
                            cursor_col = 0;
                        }                        
                    }
                    else if (ps2_code == 0x72) { //down arrow
                        if (cursor_row < 57) {
                            cursor_row++;
                            if (text_content[cursor_col][cursor_row] == 0) cursor_col = find_row_end(cursor_row);
                        }
                        else {
                            cursor_col = 119;
                        }
                    }
                    update_view();
				}
				else {
					char ascii_char = ps2_to_ascii(ps2_code);
					if (ps2_code == 0x76) { 		//Escape to exit insert mode
						draw_vga_underscore_cursor(0xffff);
						break;
					}

					//put_char_in_buffer(blink_under, position);	//To ensure blinking underscore cleared
					draw_vga_underscore_cursor(0x0000);
					position_blinker = 0;

					if (ascii_char == '\n') {			//Enter
						if (cursor_row < 57) {
                            cursor_row++;
                            cursor_col = 0;
                            update_view();
                        }
					}
					else if (ps2_code == 0x66) {		//Backspace
                        if (cursor_col > 0) {
                            cursor_col--;
                            shift_row_left(cursor_col, cursor_row);
                            redraw_screen();
                        }
                        else if (cursor_row > 0) {
                            cursor_row--;
                            cursor_col = find_row_end(cursor_row);
                        }
                        update_view();
					}
					else if (ascii_char) {
						shift_row_right(cursor_col, cursor_row);
                        put_char_in_content(ascii_char, cursor_col, cursor_row);
                        redraw_screen();
                        cursor_col = (cursor_col < 119) ? cursor_col + 1 : cursor_col;
                        update_view();
					}
					//blink_under = *(char_ptr + ((int)(position / 80) << 7) + position % 80);
				}
			}
		}
		if (*timer_ptr & 0b1) {
			position_blinker = (position_blinker) ? 0 : 1;
			if (position_blinker) draw_vga_underscore_cursor(0xffff); //put_char_in_buffer('_', position);
			else draw_vga_underscore_cursor(0x0000); //put_char_in_buffer(blink_under, position);
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

