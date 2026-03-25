#ifndef VISUAL_SYSTEM
#define VISUAL_SYSTEM

#define MOUSE_BASE 0xff200108

volatile int * ps2_ptr2 = (int*) MOUSE_BASE;

void read_mouse(int* dx, int* dy, int* buttons);

#endif