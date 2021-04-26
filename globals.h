#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef ULCD_4DGL_H_
#define ULCD_4DGL_H_
#include "uLCD_4DGL.h"
#endif

// === [global object] ===
extern uLCD_4DGL uLCD;

// === [global settings] ===
#define BACKGROUND_COLOR 0x000000

#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define PINK 0xFFC0CB
#define PURPLE 0x800080
#define YELLOW 0xFFFF00
#define RED 0xFF0000

// Modified from the RPG Game from ECE 2035. Draw more complex player object (with changing color).
void draw_img(int u, int v, int width, int height, const char* img);

#endif //GLOBAL_H