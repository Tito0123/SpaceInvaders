#ifndef missile_H
#define missile_H

#include "mbed.h"
#include "missile.h"


#include "globals.h"

extern uLCD_4DGL uLCD;

// Enumeration of Missile Status
typedef enum {
    PLAYER_MISSILE_ACTIVE=0,    // missile is active
    PLAYER_MISSILE_INACTIVE=1,  // missile is inactive
    PLAYER_MISSILE_EXPLODED=2,   // missile has been destroyed
    ENEMY_MISSILE_ACTIVE=3,    // missile is active
    ENEMY_MISSILE_INACTIVE=4,  // missile is inactive
    ENEMY_MISSILE_EXPLODED=5   // missile has been destroyed
} PLAYER_MISSILE_STATUS;

/// This struct contains the status of a missile
typedef struct
{
    int missile_blk_x;   ///< horizontal position in the grid
    int missile_blk_y;   ///< vertical position in the grid
    unsigned int missile_old_blk_x; ///< old horizontal position in the grid
    unsigned int missile_old_blk_y; ///< old vertical position in the grid
    int missile_width;
    int missile_height;
    int missile_color;   ///< color of the missile
    PLAYER_MISSILE_STATUS status;
} missile_t;

void missile_init(missile_t * g, int blk_x, int blk_y, int color);
void enemy_missile_init(missile_t * g, int blk_x, int blk_y, int color);
void missile_show(missile_t *g);
void missile_erase(missile_t *g);
void update_missile_pos(missile_t *g);
void update_enemy_missile_pos(missile_t *g, int level);

#endif //missile_H