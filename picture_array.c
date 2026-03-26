#include "picture_array.h"
#include "helper_funcs.h"
#include "text_editor.h"
#include "visual_system.h"

void draw_picture_array() {
    int x = (int)(320 - AHMED_WIDTH) / 2;
    int y = (int)(240 - AHMED_HEIGHT) / 2;
    for (int i = 0; i < AHMED_WIDTH; ++i) {
        for (int j = 0; j < AHMED_HEIGHT; ++j) {
            *(vga_ptr + ((y + j) << 9) + x + i) = ahmed[j * AHMED_WIDTH + i];
        }
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