#ifndef VISUAL_SYSTEM
#define VISUAL_SYSTEM

#define MOUSE_BASE 0xff200108
#define BLUE_GRAD_START 0x049f
#define BLUE_GRAD_END 0x019f

#define SHELL_LOGO_ENCODE 0
#define NOTEPAD_LOGO_ENCODE 1
#define PAINT_LOGO_ENCODE 2


volatile int * ps2_ptr2 = (int*) MOUSE_BASE;

int prev_mouse_x, prev_mouse_y, mouse_x, mouse_y;

short int under_mouse[4][4] = {0};

void init_mouse();
void read_mouse(int* dx, int* dy, int* buttons);
void flush_mouse();
void refresh_under_mouse();
void visual_interactive_system();
void draw_visual_interactive_wallpaper(short int start_color, short int end_color);
void draw_visual_app_icons();
void draw_mouse();
void highlight_clicked_logo(int logo_position);
short int add_blue_highlight(short int color);

#endif