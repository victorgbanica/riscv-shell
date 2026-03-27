#include "visual_system.h"
#include "helper_funcs.h"
#include "text_editor.h"
#include "picture_array.h"

void init_mouse() {
    *(ps2_ptr2) = 0xf4; //enable data streaming from mouse

    //wait for mouse acknowledge response
    while (1) {
        int data = *ps2_ptr2;
        if (data & 0x8000) {
            unsigned char byte = (unsigned char) (data & 0xff);
            if (byte == 0xfa) break;    //break on acknowledge
        }
    }
}

void read_mouse(int* dx, int* dy, int* buttons) {
    //mouse always sends packets of 3 bytes
    unsigned char bytes[3];
    int count = 0;

    while (1) {
        int data = *ps2_ptr2;
        if (data & 0x8000) { //data available to read in input fifo
            unsigned char byte = (unsigned char)(data & 0xff);
            if (byte & 0b1000) {  // bit 3 always set in byte 0
                bytes[0] = byte;
                count = 1;
                break;
            }
        }
    }

    //read remaining bytes after valid first came through
    while (count < 3) {
        int data = *ps2_ptr2;
        if (data & 0x8000) {
            bytes[count++] = (unsigned char)(data & 0xff);
        }
    }

    *buttons = bytes[0] & 0b111; //3 right bits of packet 0 are middle, right, left buttons
    
    *dx = (int)bytes[1] - ((bytes[0] & 0x10) ? 256 : 0);  // bit 4 of byte 0 = x sign
    *dy = (int)bytes[2] - ((bytes[0] & 0x20) ? 256 : 0);  // bit 5 of byte0 = y sign
}

void flush_mouse() {
    while (*ps2_ptr2 & 0x8000) {
        (void) (*ps2_ptr2); //flush mouse when not using it to avoid buffer overflow in input fifo
    }
}

void refresh_under_mouse() {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            under_mouse[i][j] = *(vga_ptr + ((mouse_y + j) << 9) + mouse_x + i);
        }
    }
}

void draw_visual_interactive_wallpaper(short int start_color, short int end_color) {
    int red_start = (start_color >> 11) & 0x1f;
    int red_end = (end_color >> 11) & 0x1f;
    int green_start = (start_color >> 5) & 0x3f;
    int green_end = (end_color >> 5) & 0x3f;
    int blue_start = start_color & 0x1f;
    int blue_end = end_color & 0x1f;

    for (int x = 0; x < 320; ++x) {
        for (int y = 0; y < 240; ++y) {
            int grad = (x * 256 / 319 + y * 256 / 239) / 2; //must multiply by 256 since otherwise, it would just go to 0. Divide by 256 later
            int r = red_start + (red_end - red_start) * grad / 256;
            int g = green_start + (green_end - green_start) * grad / 256;
            int b = blue_start + (blue_end - blue_start) * grad / 256;

            *(vga_ptr + (y << 9) + x) = (short int) ((r << 11) | (g << 5) | b);
        }
    }
}

void draw_visual_app_icons() {
    int x = 9;
    int y = 9;

    for (int i = 0; i < LOGO_WIDTH; ++i) {
        for (int j = 0; j < LOGO_HEIGHT; ++j) {
            *(vga_ptr + ((y + j) << 9) + x + i) = terminal_logo[j * 40 + i];
        }
    }

    x = x + LOGO_WIDTH + 10;
    
    for (int i = 0; i < LOGO_WIDTH; ++i) {
        for (int j = 0; j < LOGO_HEIGHT; ++j) {
            *(vga_ptr + ((y + j) << 9) + x + i) = notepad_logo[j * 40 + i];
        }
    }

    x = x + LOGO_WIDTH + 10;

    for (int i = 0; i < LOGO_WIDTH; ++i) {
        for (int j = 0; j < LOGO_HEIGHT; ++j) {
            *(vga_ptr + ((y + j) << 9) + x + i) = paint_logo[j * 40 + i];
        }
    }
}

void draw_mouse() {
    //redraw what was under the mouse
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            *(vga_ptr + ((prev_mouse_y + j) << 9) + prev_mouse_x + i) = under_mouse[i][j];
        }
    }

    //update position
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            //update new under_mouse
            under_mouse[i][j] = *(vga_ptr + ((mouse_y + j) << 9) + mouse_x + i);
        }
    }
    //draw mouse at new position (hardcoded with this pattern)
    /*
    █    
    █ █    
    █  █  
    ████   
    */
    short int color = 0x0000;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == 0 || j == 3) color = 0x0000;
            else if (i == 2 && j == 1) color = 0x0000;
            else if (i == 3 && j == 2) color = 0x0000;
            else if (i <= j) color = 0xffff;
            else continue;
            *(vga_ptr + ((mouse_y + j) << 9) + mouse_x + i) = color;
        }
    }
}

short int add_blue_highlight(short int color) {
    int r = ((color >> 11) & 0x1f) >> 1;       //shift one extra for red and green to tone down their color
    int g = ((color >> 5) & 0x3f) >> 1;
    int b = (color & 0x1f) << 1;

    b = (b > 0x1f) ? 0x1f : b;

    return (short) ((r << 11) | (g << 5) | b);
}


//add a layer of blue over the clicked logo
void highlight_clicked_logo(int logo_position) {
    int x = 9 + 10 * logo_position + LOGO_WIDTH * logo_position;
    int y = 9;

    for (int i = 0; i < LOGO_WIDTH; ++i) {
        for (int j = 0; j < LOGO_HEIGHT; ++j) {
            *(vga_ptr + ((y + j) << 9) + x + i) = add_blue_highlight(*(vga_ptr + ((y + j) << 9) + x + i));
        }
    }
}

void visual_interactive_system() {
    clear_char();
    draw_visual_interactive_wallpaper(BLUE_GRAD_START, BLUE_GRAD_END);
    draw_visual_app_icons();
    flush_mouse();

    int shell_icon_clicked, notepad_icon_clicked, paint_icon_clicked;
    shell_icon_clicked = notepad_icon_clicked = paint_icon_clicked = 0;
    prev_mouse_x = mouse_x = 160;
    prev_mouse_y = mouse_y = 120;

    int prev_buttons = 0;

    //Initialize under_mouse to background
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            under_mouse[i][j] = *(vga_ptr + ((mouse_y + j) << 9) + mouse_x + i);
        }
    }

    draw_mouse();

    while (1) {
        int mouse_buttons, mouse_dx, mouse_dy;
        read_mouse(&mouse_dx, &mouse_dy, &mouse_buttons);

        //handle mouse movement
        mouse_x += mouse_dx;
        mouse_y += mouse_dy;
        if (mouse_x <= 0) mouse_x = 0;
        else if (mouse_x >= 316) mouse_x = 316; //slightly trimmed to account for mouse width
        if (mouse_y <= 0) mouse_y = 0;
        else if (mouse_y >= 236) mouse_y = 236;

        draw_mouse();

        prev_mouse_x = mouse_x;
        prev_mouse_y = mouse_y;

        int just_clicked = (mouse_buttons & 0b1) && !(prev_buttons & 0b1); //used to detect press vs release of left mouse button
        prev_buttons = mouse_buttons;


        if (just_clicked) {  //handle mouse click (left mouse button only for now)
            //check if mouse clicked on an icon
            if (mouse_y > 9 && mouse_y < 9 + LOGO_HEIGHT) { //check top bar for all 3 to reduce redundancy since all logos fall in this range

                if (mouse_x > 9 && mouse_x < 9 + LOGO_WIDTH) {  //terminal logo, just break while loop
                    notepad_icon_clicked = paint_icon_clicked = 0;   //reset other icons if they were clicked
                    draw_visual_app_icons();
                    if (shell_icon_clicked == 0) {
                        shell_icon_clicked++;
                        highlight_clicked_logo(SHELL_LOGO_ENCODE);
                        refresh_under_mouse();
                    }
                    else if (shell_icon_clicked == 1) break;
                }

                else if (mouse_x > LOGO_WIDTH + 19 && mouse_x < 19 + 2 * LOGO_WIDTH) {  //notepad logo, open notepad
                    shell_icon_clicked = paint_icon_clicked = 0;
                    draw_visual_app_icons();
                    if (notepad_icon_clicked == 0) {
                        notepad_icon_clicked++;
                        highlight_clicked_logo(NOTEPAD_LOGO_ENCODE);
                        refresh_under_mouse();
                    }
                    else if (notepad_icon_clicked == 1) {
                        text_editor();
                        //reset coming from text_editor
                        clear_char();
                        draw_visual_interactive_wallpaper(BLUE_GRAD_START, BLUE_GRAD_END);
                        draw_visual_app_icons();
                        flush_mouse();
                        refresh_under_mouse();
                        draw_mouse();
                        notepad_icon_clicked = 0;
                        prev_buttons = 0;
                    }
                }

                else if (mouse_x > 29 + 2 * LOGO_WIDTH && mouse_x < 29 + 3 * LOGO_WIDTH) {  //paint logo, open paint
                    shell_icon_clicked = notepad_icon_clicked = 0;
                    draw_visual_app_icons();
                    if (paint_icon_clicked == 0) {
                        paint_icon_clicked++;
                        highlight_clicked_logo(PAINT_LOGO_ENCODE);
                        refresh_under_mouse();
                    }
                    else if (paint_icon_clicked == 1) {
                        paint_app();
                        clear_char();
                        draw_visual_interactive_wallpaper(BLUE_GRAD_START, BLUE_GRAD_END);
                        draw_visual_app_icons();
                        flush_mouse();
                        refresh_under_mouse();
                        draw_mouse();
                        paint_icon_clicked = 0;
                        prev_buttons = 0;
                    }
                }

                else {
                    if (shell_icon_clicked || notepad_icon_clicked || paint_icon_clicked) {
                        draw_visual_app_icons();
                    }
                    shell_icon_clicked = notepad_icon_clicked = paint_icon_clicked = 0;
                }
            }

            else {
                if (shell_icon_clicked || notepad_icon_clicked || paint_icon_clicked) {
                    draw_visual_app_icons();
                }
                shell_icon_clicked = notepad_icon_clicked = paint_icon_clicked = 0;
            }

        }
    }
}