#include "visual_system.h"
#include "helper_funcs.h"
#include "text_editor.h"

#define PAINT_BG_COLOR       0xFFFF
#define PAINT_BORDER_COLOR   0x0000
#define PAINT_PANEL_COLOR    0xC618
#define PAINT_BUTTON_COLOR   0x7BEF
#define PAINT_ACTIVE_COLOR   0x001F

#define PAINT_CANVAS_LEFT    32
#define PAINT_CANVAS_TOP     24
#define PAINT_CANVAS_RIGHT   319
#define PAINT_CANVAS_BOTTOM  239

#define PALETTE_SIZE         14
#define PALETTE_GAP          4
#define PALETTE_X            8
#define PALETTE_Y            32

static short int paint_palette[6] = {
    0x0000, // black
    0xF800, // red
    0x07E0, // green
    0x001F, // blue
    0xFFE0, // yellow
    0xFFFF  // white / eraser
};

static void paint_fill_rect(int x0, int y0, int x1, int y1, short int color) {
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > 319) x1 = 319;
    if (y1 > 239) y1 = 239;

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            *(vga_ptr + (y << 9) + x) = color;
        }
    }
}

static void paint_draw_rect_outline(int x0, int y0, int x1, int y1, short int color) {
    for (int x = x0; x <= x1; ++x) {
        *(vga_ptr + (y0 << 9) + x) = color;
        *(vga_ptr + (y1 << 9) + x) = color;
    }
    for (int y = y0; y <= y1; ++y) {
        *(vga_ptr + (y << 9) + x0) = color;
        *(vga_ptr + (y << 9) + x1) = color;
    }
}

static void paint_draw_brush_preview(int palette_index) {
    for (int i = 0; i < 6; ++i) {
        int x0 = PALETTE_X + i * (PALETTE_SIZE + PALETTE_GAP);
        int y0 = PALETTE_Y;
        int x1 = x0 + PALETTE_SIZE - 1;
        int y1 = y0 + PALETTE_SIZE - 1;

        paint_fill_rect(x0, y0, x1, y1, paint_palette[i]);
        paint_draw_rect_outline(
            x0 - 1,
            y0 - 1,
            x1 + 1,
            y1 + 1,
            (i == palette_index) ? PAINT_ACTIVE_COLOR : PAINT_BORDER_COLOR
        );
    }
}

static void paint_draw_ui(int palette_index) {
    clear_char();
    clear_screen();

    paint_fill_rect(0, 0, 319, 23, PAINT_PANEL_COLOR);
    paint_fill_rect(0, 24, 31, 239, PAINT_PANEL_COLOR);
    paint_fill_rect(PAINT_CANVAS_LEFT, PAINT_CANVAS_TOP, PAINT_CANVAS_RIGHT, PAINT_CANVAS_BOTTOM, PAINT_BG_COLOR);

    paint_draw_rect_outline(PAINT_CANVAS_LEFT - 1, PAINT_CANVAS_TOP - 1,
                            PAINT_CANVAS_RIGHT, PAINT_CANVAS_BOTTOM, PAINT_BORDER_COLOR);

    paint_fill_rect(6, 6, 24, 20, PAINT_BUTTON_COLOR);      // clear button box
    paint_fill_rect(34, 6, 52, 20, PAINT_BUTTON_COLOR);     // brush preview box

    print_row(" Paint app  ESC=exit", 0);
    print_row(" Click CLR box to wipe", 1);
    print_row(" Left draw, right erase", 2);

    paint_draw_brush_preview(palette_index);
    paint_fill_rect(39, 9, 47, 17, paint_palette[palette_index]);
    paint_draw_rect_outline(34, 6, 52, 20, PAINT_BORDER_COLOR);
}

static int paint_palette_hit(int x, int y) {
    for (int i = 0; i < 6; ++i) {
        int x0 = PALETTE_X + i * (PALETTE_SIZE + PALETTE_GAP);
        int y0 = PALETTE_Y;
        int x1 = x0 + PALETTE_SIZE - 1;
        int y1 = y0 + PALETTE_SIZE - 1;

        if (x >= x0 && x <= x1 && y >= y0 && y <= y1) {
            return i;
        }
    }
    return -1;
}

static int paint_in_canvas(int x, int y) {
    return x >= PAINT_CANVAS_LEFT && x <= PAINT_CANVAS_RIGHT &&
           y >= PAINT_CANVAS_TOP && y <= PAINT_CANVAS_BOTTOM;
}

static void paint_plot_brush(int x, int y, short int color) {
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int px = x + dx;
            int py = y + dy;
            if (paint_in_canvas(px, py)) {
                *(vga_ptr + (py << 9) + px) = color;
            }
        }
    }
}

static int paint_keyboard_exit_requested(void) {
    static int skip_next = 0;
    static int extended = 0;

    int ps2_scan = *ps2_ptr;
    if (!(ps2_scan & 0x8000)) return 0;

    unsigned char ps2_code = (unsigned char)(ps2_scan & 0xFF);

    if (skip_next > 0) {
        skip_next--;
        return 0;
    }
    if (ps2_code == 0xF0) {
        skip_next = 1;
        extended = 0;
        return 0;
    }
    if (ps2_code == 0xE0) {
        extended = 1;
        return 0;
    }

    if (extended) {
        extended = 0;
        return 0;
    }

    return (ps2_code == 0x76);
}

void paint_app(void) {
    int palette_index = 0;
    int prev_buttons = 0;

    paint_draw_ui(palette_index);
    flush_mouse();

    prev_mouse_x = mouse_x = 160;
    prev_mouse_y = mouse_y = 120;

    refresh_under_mouse();
    draw_mouse();

    while (1) {
        if (paint_keyboard_exit_requested()) {
            break;
        }

        int mouse_dx, mouse_dy, mouse_buttons;
        read_mouse(&mouse_dx, &mouse_dy, &mouse_buttons);

        mouse_x += mouse_dx;
        mouse_y += mouse_dy;
        if (mouse_x < 0) mouse_x = 0;
        else if (mouse_x > 316) mouse_x = 316;
        if (mouse_y < 0) mouse_y = 0;
        else if (mouse_y > 236) mouse_y = 236;

        int just_left_clicked = (mouse_buttons & 0b001) && !(prev_buttons & 0b001);
        int just_right_clicked = (mouse_buttons & 0b010) && !(prev_buttons & 0b010);

        if (just_left_clicked) {
            if (mouse_x >= 6 && mouse_x <= 24 && mouse_y >= 6 && mouse_y <= 20) {
                paint_draw_ui(palette_index);
            }
            else {
                int picked = paint_palette_hit(mouse_x, mouse_y);
                if (picked >= 0) {
                    palette_index = picked;
                    paint_draw_ui(palette_index);
                }
            }
        }

        if ((mouse_buttons & 0b001) && paint_in_canvas(mouse_x, mouse_y)) {
            paint_plot_brush(mouse_x, mouse_y, paint_palette[palette_index]);
        }
        if ((mouse_buttons & 0b010) && paint_in_canvas(mouse_x, mouse_y)) {
            paint_plot_brush(mouse_x, mouse_y, PAINT_BG_COLOR);
        }

        if (just_right_clicked && mouse_x >= 6 && mouse_x <= 24 && mouse_y >= 6 && mouse_y <= 20) {
            paint_draw_ui(palette_index);
        }

        prev_buttons = mouse_buttons;
        draw_mouse();
        prev_mouse_x = mouse_x;
        prev_mouse_y = mouse_y;
    }

    flush_mouse();
    clear_char();
}
