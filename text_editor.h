#ifndef TEXT_EDITOR
#define TEXT_EDITOR

#define KEYBOARD_BASE 0xff200108
#define CHAR_BUFFER 0x9000000
#define VGA_BUFFER 0x8000000
#define TIMER_BASE 0xff202000
	
#define BLINK_DELAY 50000000
	
#define SAVE_AND_EXIT 1
#define DELETE_ROW 3

volatile int * ps2_ptr = (int*) KEYBOARD_BASE;
volatile char * char_ptr = (char*) CHAR_BUFFER;
volatile short * vga_ptr = (short*) VGA_BUFFER;
volatile int * timer_ptr = (int*) TIMER_BASE;

//char text_content[80][60] = {0}; This is from text_editor.c plain
//int position = 0;

char text_content[120][60] = {0};   //Save last 2 rows for command
int cursor_col = 0;
int cursor_row = 0;
int view_offset = 0;

short int position_blinker = 0;
char blink_under = 0;

const char* text_help_content = " sw : Save work and exit.\n dr : Delete row.\n i  : Enter editing mode.\n^ESC: Exit help dialog.";

void clear_char();
void draw_vga_underscore(int pos, short int color);
char ps2_to_ascii(unsigned char code);
void put_char_in_buffer(char letter, int position);
int execute_editor_command(char* command, int size);
void text_editor_command();
void text_editor_insert();
void text_editor();

//new functions with screen panning
char get_char_from_content(int col, int row);
void put_char_in_content(char letter, int col, int row);
void redraw_screen();
void update_view();
//modified to accomodate for offset
void draw_vga_underscore_cursor(short int color);
int find_row_end(int row);
//useful for inserting a character in place
void shift_row_right(int from_col, int row);
//useful for backspacing
void shift_row_left(int from_col, int row);

#endif