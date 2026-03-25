#ifndef HELPER_FUNCS
#define HELPER_FUNCS

#define TEXT_HELP 101
#define SHELL_HELP 102

#define SHELL_ROWS 6
#define ACTIVE_ROW SHELL_ROWS-1

const char* shell_help_content = " word  : Open text editor.\n paint : Open visual editor.\n ahmed : Open visual paradise.\n ^ESC  : Power off.";

void clear_screen();
int strcmp(const char* str1, const char* str2);
void print_row(char* buffer, int row);
short int execute_command(char* command, int size);
void open_help_dialog(int dialog_type);
void draw_shell_arrow_box(int row, short int color);

#endif