#include "player.h"
#include "missile.h"
/*
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define PINK 0xFFC0CB
#define PURPLE 0x800080
#define YELLOW 0xFFFF00
#define RED 0xFF0000

// Modified from the RPG Game from ECE 2035. Draw more complex player object (with changing color).
void draw_img(int u, int v, int width, int height, const char* img)
{
    int colors[width*height];
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
*/

// draw the player as a nicer looking sprite (closer to original game). Draw the sprite with the appropriately colored pixels.
void draw_player_object(int blk_x, int blk_y, int player_color, int p_width, int p_height)
{
    char* colors;
    //uLCD.filled_rectangle(blk_x,blk_y,blk_x+p_width,blk_y+p_height,player_color);
    if (player_color == GREEN) {
        colors = "000000G00000000000GGG0000000000GGG000000GGGGGGGGGGG0GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";
    } else if (player_color == BLUE) {
        colors = "000000B00000000000BBB0000000000BBB000000BBBBBBBBBBB0BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";
    } else if (player_color == PINK) {
        colors = "000000P00000000000PPP0000000000PPP000000PPPPPPPPPPP0PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP";
    } else if (player_color == PURPLE) {
        colors = "000000U00000000000UUU0000000000UUU000000UUUUUUUUUUU0UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU";
    } else if (player_color == RED) {
        colors = "000000R00000000000RRR0000000000RRR000000RRRRRRRRRRR0RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR";
    } else {
        colors = "000000Y00000000000YYY0000000000YYY000000YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY";
    }
         
/*         "000000G000000
         00000GGG00000
         00000GGG00000
         0GGGGGGGGGGG0
         GGGGGGGGGGGGG
         GGGGGGGGGGGGG
         GGGGGGGGGGGGG
         GGGGGGGGGGGGG    */
    draw_img(blk_x, blk_y, p_width, p_height, colors);
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
    //g->player_height = 8;
    //g->player_width = 8;
    g->player_height = 8;
    g->player_width = 13; // change width of player to 13 for a nicer looking sprite
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