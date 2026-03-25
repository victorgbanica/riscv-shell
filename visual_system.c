#include "visual_system.h"

void read_mouse(int* dx, int* dy, int* buttons) {
    //mouse always sends packets of 3 bytes
    unsigned char bytes[3];
    int count = 0;

    while (count < 3) {
        int data = *ps2_ptr2;
        if (data & 0x8000) {    //data available to read in input fifo
            bytes[count++] = (unsigned char)(data & 0xff);
        }
    }

    *buttons = bytes[0] & 0b111; //3 right bits of packet 0 are middle, right, left buttons
    *dx = (bytes[0] & 0b10000) ? -1 * (int) bytes[1] : (int) bytes[1];      //bit 4 of byte 0 is sign of x, opposite for y. Invert if 1
    *dy = (bytes[0] & 0b100000) ? (int) bytes[2] : -1 * (int) bytes[2];
}