#include "globals.h"
#include "mbed.h"

// Modified from the RPG Game from ECE 2035. Draw more complex player object (with changing color).
void draw_img(int u, int v, int width, int height, const char* img)
{
    int colors[width*height];
    // create the array used by uLCD.BLIT to create more complex sprites.
    for (int i = 0; i < width*height; i++)
    {
        if (img[i] == 'G') colors[i] = GREEN;
        else if (img[i] == 'B') colors[i] = BLUE;
        else if (img[i] == 'P') colors[i] = PINK;
        else if (img[i] == 'U') colors[i] = PURPLE;
        else if (img[i] == 'Y') colors[i] = YELLOW;
        else if (img[i] == 'R') colors[i] = RED;
        else colors[i] = BLACK;
    }
    uLCD.BLIT(u, v, width, height, colors);
    wait_us(250); // Recovery time!
}