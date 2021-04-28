#ifndef BARRIER_H
#define BARRIER_H

#include "mbed.h"
#include "player.h"
#include "missile.h"
#include "globals.h"

extern uLCD_4DGL uLCD;


/// This struct contains the status of a barrier
typedef struct
{
    int barrier_blk_x;   ///< horizontal position in the grid
    int barrier_blk_y;   ///< vertical position in the grid
    //unsigned int player_old_blk_x; ///< old horizontal position in the grid
    //unsigned int player_old_blk_y; ///< old vertical position in the grid
    int barrier_width;
    int barrier_height;
    int barrier_color;   ///< color of the barrier
    bool barrierPixels[20*20]; // bool array to signify whether the pixel of the barrier is "destroyed" or still part of the barrier.
} barrier_t;

void barrier_init(barrier_t * g, int blk_x, int blk_y, int color);
void barrier_show(barrier_t *g);
void barrier_erase(barrier_t *g);
void check_barrier(barrier_t *g, missile_t *h);
#endif //PLAYER_H