#include "enemy.h"
#include "missile.h"

void draw_enemy(int blk_x, int blk_y, int enemy_color, int e_width, int e_height)
{
    uLCD.filled_rectangle(blk_x,blk_y,blk_x+e_width,blk_y-e_height,enemy_color);
    //const int* colors = [
    //uLCD.BLIT(blk_x, blk_y, e_width, e_height, 
}

void erase_enemy(int blk_x, int blk_y, int enemy_color, int e_width, int e_height)
{
    uLCD.filled_rectangle(blk_x,blk_y,blk_x+e_width,blk_y-e_height,BACKGROUND_COLOR);
}

int move_enemy(enemy_t * g, int MOVE_DOWN, int DIRECTION)
{
    if (g->status == ENEMY_ALIVE)
    {   
        if (DIRECTION == 1)
        {
            enemy_erase(g);
            g->enemy_blk_x += 1;
            enemy_show(g);
            
            if (g->enemy_blk_y >= 110)
            {
                MOVE_DOWN = 2;
            }
        }
        else if (DIRECTION == 2)
        {
            enemy_erase(g);
            g->enemy_blk_x -= 1;
            enemy_show(g);   
            
            if (g->enemy_blk_y >= 110)
            {
                MOVE_DOWN = 2;
            }
        }
        
        if ((g->enemy_blk_x+g->enemy_width) > (128-g->enemy_width)
        || (g->enemy_blk_x < (0+g->enemy_width)))
        {
            MOVE_DOWN = 1;
        }    
    }
    return MOVE_DOWN;
}

void enemy_init(enemy_t * g, unsigned int blk_x, unsigned int blk_y, unsigned int color)
{
    g->enemy_blk_x = blk_x;
    g->enemy_blk_y = blk_y;
    g->enemy_color = color;
    g->enemy_width = 8;
    g->enemy_height = 8;
    g->status = ENEMY_ALIVE;
}

void enemy_show(enemy_t * g)
{
    draw_enemy(g->enemy_blk_x, g->enemy_blk_y, g->enemy_color, g->enemy_width, g->enemy_height);
}

void enemy_erase(enemy_t * g)
{
    erase_enemy(g->enemy_blk_x, g->enemy_blk_y, g->enemy_color, g->enemy_width, g->enemy_height);
}

int check_enemy(enemy_t * g, missile_t * h)
{
    int enemy_died = 0;
    if (g->status == ENEMY_ALIVE 
    && ((h->missile_blk_x >= g->enemy_blk_x) && (h->missile_blk_x <= (g->enemy_blk_x + g->enemy_width)))) 
    {
        enemy_erase(g);
        g->status = ENEMY_DEAD;
        enemy_died = 1;
        h->status = PLAYER_MISSILE_EXPLODED;                         
    }  
    
    return enemy_died;
}