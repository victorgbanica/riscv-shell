#include "helper_funcs.h"
#include "text_editor.h"
#include "picture_array.h"
#include "visual_system.h"

void clear_screen() {
	for (int i = 0; i < 320; i++) {
		for (int j = 0; j < 240; j++) {
			*(vga_ptr + (j << 9) + i) = 0x0000;
		}
	}
}

/*
	This function compares 2 null terminated strings
	If the strings are equal, it returns 1
	If the string are not equal, it returns 0
	Has a safety limit of 80 chars
*/
int strcmp(const char* str1, const char* str2) {
	int i;
	for (i = 0; i < 79 && str1[i]; ++i) {
		if (str1[i] != str2[i]) return 0;
	}
	if ((str1[i] != 0 && str2[i] == 0) || (str1[i] == 0 && str2[i] != 0)) return 0;
	
	return 1;
}

/*
	This function safely prints a buffer onto a row. It is limited to 80 characters and can print literals as well.
*/
void print_row(char* buffer, int row) {
	char safe_buffer[80] = {0};
	int buffer_done = 0;
	for (int i = 0; i < 80; ++i) {
		if (buffer[i] == 0) buffer_done = 1;
		
		if (buffer_done == 0) safe_buffer[i] = buffer[i];
		
		*(char_ptr + (row << 7) + i) = safe_buffer[i];
	}
}

//returns a color based on success of command
short int execute_command(char* command, int size) {		//This function handles whatever is in the curr_user_buff when the user presses Enter by launching that app
	if (*(command + 1) == 0) {
		print_row(" ", SHELL_ROWS);
		return 0x0000;	//Black for transparent/neutral
	}
	else if (strcmp(command + 1, "help")) {
		print_row("Opening help dialog.", SHELL_ROWS);
		open_help_dialog(SHELL_HELP);
		clear_char();
		return 0x07e0;	//green success
	}
	else if (strcmp(command + 1, "word")) {
		print_row("Opening text editor.", SHELL_ROWS);
		text_editor();
		clear_char();
		return 0x07e0;
	}
	else if (strcmp(command + 1, "paint")) {
		print_row("Opening visual editor.", SHELL_ROWS);
		paint_app();
		clear_screen();
		clear_char();
		flush_mouse();
		return 0x07e0;
	}
	else if (strcmp(command + 1, "visual")) {
		print_row("Opening visual system.", SHELL_ROWS);
		visual_interactive_system();
		clear_screen();
		flush_mouse();
		return 0x07e0;
	}
	else if (strcmp(command + 1, "ily")) {
		print_row("I love you too!", SHELL_ROWS);
		return 0xf81f;
	}
	else if (strcmp(command + 1, "ahmed")) {
		clear_screen();
		clear_char();
		draw_picture_array();
		clear_screen();
		return 0xffc0;
	}
	else {
		print_row("Command is not known to this system. \"help\" to open help dialog", SHELL_ROWS);
		return 0xe800;
	}
}

void open_help_dialog(int dialog_type) {
	clear_char();
	char* printer;
	
	if (dialog_type == TEXT_HELP) printer = text_help_content;
	else if (dialog_type == SHELL_HELP) printer = shell_help_content;
	else return;
	
	int position = 0;
	while (*printer) {
		if (*printer == '\n') {
			printer++;
			put_char_in_buffer(0x00, position);
			position++;
			while (position % 80) {				//Fill row with zeros until next row
				put_char_in_buffer(0x00, position++);
			}
		}
		else put_char_in_buffer(*(printer++), position++);
	}
	
	//wait until ESC is pressed
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
			else if (ps2_code == 0x76) {		//ESC, exit
				break;				
			}
		}
	}
}

void draw_shell_arrow_box(int row, short int color) {
	row *= 4;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			*(vga_ptr + ((row + j) << 9) + i) = color;
		}
	}
}
