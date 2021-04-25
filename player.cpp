#include "player.h"
#include "missile.h"

#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define PINK 0xFFC0CB
#define YELLOW 0xFFFF00
#define BROWN  0xD2691E
#define DIRT   BROWN
void draw_img(int u, int v, const char* img)
{
    int colors[11*11];
    for (int i = 0; i < 11*11; i++)
    {
        if (img[i] == 'R') colors[i] = RED;
        else if (img[i] == 'Y') colors[i] = YELLOW;
        else if (img[i] == 'G') colors[i] = GREEN;
        else if (img[i] == 'D') colors[i] = DIRT;
        else if (img[i] == '5') colors[i] = LGREY;
        else if (img[i] == '3') colors[i] = DGREY;
        else if (img[i] == 'W') colors[i] = WHITE;
        else if (img[i] == 'U') colors[i] = BLUE;
        else colors[i] = BLACK;
    }
    uLCD.BLIT(u, v, 11, 11, colors);
    wait_us(250); // Recovery time!
}

void draw_player_object(int blk_x, int blk_y, int player_color, int p_width, int p_height)
{
    //uLCD.filled_rectangle(blk_x,blk_y,blk_x+p_width,blk_y+p_height,player_color);
    const int* colors = [
    uLCD.BLIT(blk_x, blk_y, e_width, e_height, 
}

void erase_player(int blk_x, int blk_y, int p_width, int p_height)
{
    uLCD.filled_rectangle(blk_x,blk_y,blk_x+p_width,blk_y+p_height,BACKGROUND_COLOR);
}

void player_init(player_t * g, int blk_x, int blk_y, int color)
{
    g->player_blk_x = blk_x;
    g->player_blk_y = blk_y;
    g->player_color = 0x00FF00;
    g->player_height = 8;
    g->player_width = 8;
    g->status = PLAYER_ALIVE;
}

void player_show(player_t * g)
{
    draw_player_object(g->player_blk_x, g->player_blk_y, g->player_color, g->player_width, g->player_height);
}

void player_erase(player_t *g)
{
    erase_player(g->player_blk_x, g->player_blk_y, g->player_width, g->player_height);
}

int check_player(player_t * g, missile_t * h)
{
    int player_died = 0;
    if (g->status == PLAYER_ALIVE 
    && ((h->missile_blk_x >= g->player_blk_x) && (h->missile_blk_x <= (g->player_blk_x + g->player_width)))) 
    {
        player_erase(g);
        g->status = PLAYER_DEAD;
        player_died = 1;
        h->status = ENEMY_MISSILE_EXPLODED;                         
    }  
    
    return player_died;
}