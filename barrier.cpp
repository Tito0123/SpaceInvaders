#include "barrier.h"
#include "missile.h"
#include "globals.h"
/*
#define GREEN 0x00FF00

// Modified from the RPG Game from ECE 2035. Draw more complex player object (with changing color).
void draw_img(int u, int v, int width, int height, const char* img)
{
    int colors[width*height];
    for (int i = 0; i < width*height; i++)
    {
        if (img[i] == 'G') colors[i] = GREEN;
        //else if (img[i] == 'B') colors[i] = BLUE;
        //else if (img[i] == 'P') colors[i] = PINK;
        //else if (img[i] == 'U') colors[i] = PURPLE;
        //else if (img[i] == 'Y') colors[i] = YELLOW;
        //else if (img[i] == 'R') colors[i] = RED;
        else colors[i] = BLACK;
    }
    uLCD.BLIT(u, v, width, height, colors);
    wait_us(250); // Recovery time!
}
*/

void draw_barrier_object(int blk_x, int blk_y, int barrier_color, int barrier_width, int barrier_height, barrier_t *g)
{
    char* colors;
    //uLCD.filled_rectangle(blk_x,blk_y,blk_x+p_width,blk_y+p_height,player_color);
    if (barrier_color == GREEN) {
        colors = "000000GGGGGGGG00000000000GGGGGGGGGG000000000GGGGGGGGGGGG0000000GGGGGGGGGGGGGG00000GGGGGGGGGGGGGGGG000GGGGGGGGGGGGGGGGGG0GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG00000000GGGGGGGGGGG0000000000GGGGGGGGG000000000000GGGGGGGG000000000000GGGGGGGG000000000000GGGG";
    }
         
        /* 000000GGGGGGGG000000
         00000GGGGGGGGGG00000
         0000GGGGGGGGGGGG0000
         000GGGGGGGGGGGGGG000
         00GGGGGGGGGGGGGGGG00
         0GGGGGGGGGGGGGGGGGG0
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGGGGGGGGGGGGGGGG
         GGGGGG00000000GGGGGG
         GGGGG0000000000GGGGG
         GGGG000000000000GGGG
         GGGG000000000000GGGG
         GGGG000000000000GGGG  */
    // This initializes the boolean array of pixels to be 1 when the pixel is green and 0 when the pixel is black.
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 20; y++) {
            g->barrierPixels[y*20 + x] = (colors[y*20 + x] == 'G');
        }
    }
    draw_img(blk_x, blk_y, barrier_width, barrier_height, colors); // draw the barrier
}

void erase_barrier(int blk_x, int blk_y, int barrier_width, int barrier_height)
{
    uLCD.filled_rectangle(blk_x,blk_y,blk_x+barrier_width,blk_y+barrier_height,BACKGROUND_COLOR);
    
}

void barrier_init(barrier_t * g, int blk_x, int blk_y, int color)
{
    g->barrier_blk_x = blk_x;
    g->barrier_blk_y = blk_y;
    g->barrier_color = 0x00FF00;
    //g->player_height = 8;
    //g->player_width = 8;
    g->barrier_height = 20;
    g->barrier_width = 20;
    memset(g->barrierPixels, 0, sizeof(bool) * 20 * 20);
    //g->status = PLAYER_ALIVE;
}

void barrier_show(barrier_t * g)
{
    draw_barrier_object(g->barrier_blk_x, g->barrier_blk_y, g->barrier_color, g->barrier_width, g->barrier_height, g);
}

void barrier_erase(barrier_t *g)
{
    erase_barrier(g->barrier_blk_x, g->barrier_blk_y, g->barrier_width, g->barrier_height);
    memset(g->barrierPixels, 0, sizeof(bool) * 20 * 20);
}

void check_barrier(barrier_t * g, missile_t * h)
{
    //int barrier_died = 0;
    // handle the two possible types of missiles: player and enemy.
    if (h->status == ENEMY_MISSILE_ACTIVE) {
        if (((h->missile_blk_x >= g->barrier_blk_x) && (h->missile_blk_x <= (g->barrier_blk_x + g->barrier_width)))
        && ((h->missile_blk_y >= g->barrier_blk_y) && (h->missile_blk_y <= (g->barrier_blk_y + g->barrier_height)))
        && ((g->barrierPixels[h->missile_blk_x - g->barrier_blk_x + (h->missile_blk_y - g->barrier_blk_y)*20])))
        {
            //player_erase(g);
            // "destroy" the pixels surrounding the impact of the missile in a 11x6 rectangle. Set the colors of these pixels to black.
            for (int x = h->missile_blk_x - 5; ((x <= h->missile_blk_x + 5) && (x <= g->barrier_blk_x + g->barrier_width)); x++) {
                for (int y = h->missile_blk_y; ((y <= h->missile_blk_y + 5) && (y <= g->barrier_blk_y + g->barrier_height)); y++) {
                    if (x >= g->barrier_blk_x) { 
                        g->barrierPixels[x - g->barrier_blk_x + (y - g->barrier_blk_y)*20] = 0;
                        uLCD.pixel(x, y, BLACK);
                    }
                }
            }
            h->status = ENEMY_MISSILE_EXPLODED;
        }
    } else {
        //printf("%d\n\r", h->missile_blk_y - h->missile_height <= g->barrier_blk_y);
        //uLCD.filled_rectangle(h->missile_blk_x, h->missile_blk_y - h->missile_height, h->missile_blk_x+1, h->missile_blk_y - h->missile_height + 2, RED);
        //uLCD.filled_rectangle(g->barrier_blk_x, g->barrier_blk_y, g->barrier_blk_x + 1, g->barrier_blk_y + 2, RED);
        //while(1) {
        //    printf("%d\n\r", h->missile_blk_y - h->missile_height >= g->barrier_blk_y);
        //}; 
        //printf("%d\n\r", h->missile_blk_y - h->missile_height);
        //printf("%d\n\r", h->missile_blk_y);
        //printf("%d\n\r", g->barrier_blk_y);
        if (((h->missile_blk_x >= g->barrier_blk_x) && (h->missile_blk_x <= (g->barrier_blk_x + g->barrier_width)))
        && ((h->missile_blk_y - h->missile_height <= g->barrier_blk_y + g->barrier_height) && (h->missile_blk_y - h->missile_height >= g->barrier_blk_y))
        && ((g->barrierPixels[h->missile_blk_x - g->barrier_blk_x + (h->missile_blk_y - h->missile_height - g->barrier_blk_y)*20])))
        {
            //player_erase(g);
            // "destroy" the pixels surrounding the impact of the missile in a 11x6 rectangle. Set the colors of these pixels to black.
            //printf("%d", h->missile_blk_y - h->missile_height <= g->barrier_blk_y + g->barrier_height);
            h->status = PLAYER_MISSILE_EXPLODED;
            for (int x = h->missile_blk_x - 5; ((x <= h->missile_blk_x + 5) && (x <= g->barrier_blk_x + g->barrier_width)); x++) {
                for (int y = h->missile_blk_y - h->missile_height; ((y >= h->missile_blk_y - h->missile_height - 5) && (y >= g->barrier_blk_y)); y--) {
                    if (x >= g->barrier_blk_x) { 
                        g->barrierPixels[x - g->barrier_blk_x + (y - g->barrier_blk_y)*20] = 0;
                        uLCD.pixel(x, y, BLACK);
                    }
                }
            }
        }
    }
        //g->status = PLAYER_DEAD;
        //player_died = 1;
        // make sure to take into account both player and enemy missile types...
        //if (h->status == PLAYER_MISSILE_ACTIVE) {
        //    h->status = PLAYER_MISSILE_EXPLODED; // missile explodes
        //} else {
        //    h->status = ENEMY_MISSILE_EXPLODED; // missile explodes.
        //}                        
    //}  
    //return player_died;
}