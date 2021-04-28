#include "missile.h"


void draw_missile_object(int blk_x, int blk_y, int missile_color, int m_width, int m_height)
{
    uLCD.filled_rectangle(blk_x,blk_y-1,blk_x+m_width,blk_y-m_height,missile_color);
}

void erase_missile(int blk_x, int blk_y, int m_width, int m_height)
{
    uLCD.filled_rectangle(blk_x,blk_y-1,blk_x+m_width,blk_y-m_height,BACKGROUND_COLOR);
}

void update_missile_pos(missile_t *g)
{
    if (g->missile_blk_y < 18)
    {
        g->status = PLAYER_MISSILE_EXPLODED;
    }
    
    if (g->status == PLAYER_MISSILE_ACTIVE)
    {
        missile_erase(g);
        g->missile_blk_y = g->missile_blk_y-3;
        missile_show(g);
    }
    else if (g->status == PLAYER_MISSILE_EXPLODED)
    {
        missile_erase(g);
        g->status = PLAYER_MISSILE_INACTIVE;
        g->missile_blk_y = 109;
    }
}

void update_enemy_missile_pos(missile_t *g, int level)
{
    if (g->missile_blk_y > 128)
    {
        g->status = ENEMY_MISSILE_EXPLODED;
    }
    
    if (g->status == ENEMY_MISSILE_ACTIVE)
    {
        missile_erase(g);
        //g->missile_blk_y = g->missile_blk_y+1;
        
        // higher difficulties/levels have faster enemy missiles
        if (level == 1) {
            g->missile_blk_y = g->missile_blk_y+1;
        } else if (level == 2) {
            g->missile_blk_y = g->missile_blk_y+3;
        } else if (level == 3) {
            g->missile_blk_y = g->missile_blk_y+6;
        }
        
        missile_show(g);
    }
    else if (g->status == ENEMY_MISSILE_EXPLODED)
    {
        missile_erase(g);
        g->status = ENEMY_MISSILE_INACTIVE;
        g->missile_blk_y = 2;
    }
}

void missile_init(missile_t * g, int blk_x, int blk_y, int color)
{
    g->missile_blk_x = blk_x;
    g->missile_blk_y = blk_y;
    g->missile_color = color;
    g->missile_width = 1;
    g->missile_height = 8;
    g->status = PLAYER_MISSILE_INACTIVE;
}

void enemy_missile_init(missile_t * g, int blk_x, int blk_y, int color)
{
    g->missile_blk_x = blk_x;
    g->missile_blk_y = blk_y;
    g->missile_color = color;
    g->missile_width = 1;
    g->missile_height = 8;
    g->status = ENEMY_MISSILE_INACTIVE;
}

void missile_show(missile_t * g)
{
    draw_missile_object(g->missile_blk_x, g->missile_blk_y, g->missile_color, g->missile_width, g->missile_height);
}

void missile_erase(missile_t *g)
{
    erase_missile(g->missile_blk_x, g->missile_blk_y, g->missile_width, g->missile_height);
}