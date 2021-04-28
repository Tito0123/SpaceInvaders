#include "mbed.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "enemy.h"
#include "player.h"
#include "barrier.h"
#include "missile.h"
#include "globals.h"
#include "rtos.h"
#include "SparkfunAnalogJoystick.h"
#include "LSM9DS1.h"
#include <string>

// Navigation Switch replaced with Analog Joystick and IMU
/* ==== Navigation Switch ===== */
/*
class Nav_Switch
{
public:
    Nav_Switch(PinName up,PinName down,PinName left,PinName right,PinName fire);
    int read();
//boolean functions to test each switch
    bool up();
    bool down();
    bool left();
    bool right();
    bool fire();
//automatic read on RHS
    operator int ();
//index to any switch array style
    bool operator[](int index) {
        return _pins[index];
    };
private:
    BusIn _pins;
 
};
Nav_Switch::Nav_Switch (PinName up,PinName down,PinName left,PinName right,PinName fire):
    _pins(up, down, left, right, fire)
{
    _pins.mode(PullUp); //needed if pullups not on board or a bare nav switch is used - delete otherwise
    wait(0.001); //delays just a bit for pullups to pull inputs high
}
inline bool Nav_Switch::up()
{
    return !(_pins[0]);
}
inline bool Nav_Switch::down()
{
    return !(_pins[1]);
}
inline bool Nav_Switch::left()
{
    return !(_pins[2]);
}
inline bool Nav_Switch::right()
{
    return !(_pins[3]);
}
inline bool Nav_Switch::fire()
{
    return !(_pins[4]);
}
inline int Nav_Switch::read()
{
    return _pins.read();
}
inline Nav_Switch::operator int ()
{
    return _pins.read();
}
*/

// Platform initialization
uLCD_4DGL uLCD(p28,p27,p29);  // LCD (serial tx, serial rx, reset pin;)
AnalogOut DACout(p18);      // speaker
wave_player waver(&DACout); // wav player
SDFileSystem sd(p5, p6, p7, p8, "sd"); // SD card and filesystem (mosi, miso, sck, cs)
//Nav_Switch myNav(p16, p10, p11, p9, p12); //pin order on Sparkfun breakout. move U from p13 to p16 so we can use another Serial to communicate with mbed
DigitalIn pb(p20); // push button for player misisle fire
Serial pc(USBTX, USBRX); // communicate with pc (for debugging)
RawSerial secondMbed(p13, p14); // potentially communicate with a second mbed for 2 player.
PwmOut red(p21); // added to dim and brighten red LED (in RGB LED) -- Brice
PwmOut green(p22); // added to dim and brighten green LED (in RGB LED) -- Brice
PwmOut blue(p23); // added to dim and brighten blue LED (in RGB LED) -- Brice
SparkfunAnalogJoystick stick(p15, p16, p17); // Sparkfun analog joystick to replace the tactile switch (menu control and more fluid control of ship)
LSM9DS1 IMU(p9, p10, 0xD6, 0x3C); // IMU added. Use the IMU accelerometer for a second optional tilt control for menu and ship.
// Initialize all global enemy objects
enemy_t enemy_1;
enemy_t enemy_2;
enemy_t enemy_3;
enemy_t enemy_4;
enemy_t enemy_5;
enemy_t enemy_6;
enemy_t enemy_7;
enemy_t enemy_8;
enemy_t enemy_9;
enemy_t enemy_10;
enemy_t enemy_11;
enemy_t enemy_12;
enemy_t enemy_13;
enemy_t enemy_14;
enemy_t enemy_15;

// initialize all global barrier objects.
barrier_t barrier_1;
barrier_t barrier_2;
barrier_t barrier_3;
barrier_t barrier_4;

// Initialize variables
// Brice added "volatile" here to protect global variables.
volatile int numOfEnemies = 0; // number of enemies
volatile int ENEMY_MOVE = 1; // enemy moves
volatile int MOVE_DOWN = 0; // enemies move down
volatile int DIRECTION = 1; // direction of movement
volatile int firing_col = 0; // col down which missiles are fired
volatile int hit_player = 0; // determines if player was hit
volatile bool lose = false; // determines if player loses the game
volatile int lives1 = 3; // number of lives in level 1
volatile int lives2 = 2; // number of lives in level 2
volatile int lives3 = 1; // number of lives in level 3
volatile int level = 1; // the level, number of lives/difficulty
volatile bool game_menu = false; // boolean to determine if in the game_menu state
volatile bool begin_game = false; // boolean to determine if the game has begun
volatile bool gameover = false; // boolean to determine if the game was lost (game_over)
//volatile int numPlayers = 1; // ONLY NEEDED FOR 2 PLAYER
//volatile bool first_player_ready = false; // 2 Player: Keep track of each player being ready
//volatile bool second_player_ready = false; // 2 Player: Keep track of each player being ready
//volatile bool begin_game2 = false; // ONLY NEEDED FOR 2 PLAYER. 2 PLAYER game started
//volatile int numWins = 0; // MAY BE USED FOR 2 PLAYER. Keep track of the number of wins each player got.
//volatile bool two_player_win = false; // global for a two_player_win (win on this mbed) // ONLY NEEDED FOR 2 PLAYER
//volatile bool two_player_lose = false; // global for a two_player_lose (loss on this mbed) // ONLY NEEDED FOR 2 PLAYER
volatile bool win = false; // add win to global variables so that we can have LED effects and sound effects for a win. Keeps track of the victory state.
Timer bestTimer; // Timer started when the game begins and stopped when the game ends. Used to determine if there's a new best time.
Mutex SDLock; // Used to put a mutex lock on the SD card for audio or for reading/writing the best time to the SD card.
//Mutex mbedLock; // MIGHT BE NEEDED FOR 2 PLAYER

// Initialize global player object
player_t player;

// Intialize global player and enemy missile
missile_t missile; // player missile
missile_t enemy_missile; // enemy missile
//missile_t enemy_missile2; // first extra enemy missile for level 2 (medium difficulty)

// Array of enemy objects
enemy_t * enemyArray[15];

// Function Prototypes
void move_enemy_down();
void playstart(void const *args); // PUT BACK IN

// Draws the enemies at the initial starting location    
void draw_enemies_level()
{
// Initialize local variables
    unsigned int start_x_pos = 12;
    unsigned int start_enemy_y_pos = 20; 
    numOfEnemies = 15;
    
    // First Row of Enemies
    enemy_init(&enemy_1,start_x_pos,start_enemy_y_pos,WHITE); // initialize x-pos and y-pos and color of enemy
    enemy_show(&enemy_1); // displays the enemy on uLCD
    
    enemy_init(&enemy_2,start_x_pos+15,start_enemy_y_pos,WHITE);
    enemy_show(&enemy_2);
    
    enemy_init(&enemy_3,start_x_pos+30,start_enemy_y_pos,WHITE);
    enemy_show(&enemy_3);
    
    enemy_init(&enemy_4,start_x_pos+45,start_enemy_y_pos,WHITE);
    enemy_show(&enemy_4);
    
    enemy_init(&enemy_5,start_x_pos+60,start_enemy_y_pos,WHITE);
    enemy_show(&enemy_5);
    
    // Second Row of Enemies
    enemy_init(&enemy_6,start_x_pos,start_enemy_y_pos+12,WHITE);
    enemy_show(&enemy_6);
    
    enemy_init(&enemy_7,start_x_pos+15,start_enemy_y_pos+12,WHITE);
    enemy_show(&enemy_7);
    
    enemy_init(&enemy_8,start_x_pos+30,start_enemy_y_pos+12,WHITE);
    enemy_show(&enemy_8);
    
    enemy_init(&enemy_9,start_x_pos+45,start_enemy_y_pos+12,WHITE);
    enemy_show(&enemy_9);
    
    enemy_init(&enemy_10,start_x_pos+60,start_enemy_y_pos+12,WHITE);
    enemy_show(&enemy_10);
    
    // Third Row of Enemies
    enemy_init(&enemy_11,start_x_pos,start_enemy_y_pos+24,WHITE);
    enemy_show(&enemy_11);
    
    enemy_init(&enemy_12,start_x_pos+15,start_enemy_y_pos+24,WHITE);
    enemy_show(&enemy_12);
    
    enemy_init(&enemy_13,start_x_pos+30,start_enemy_y_pos+24,WHITE);
    enemy_show(&enemy_13);
    
    enemy_init(&enemy_14,start_x_pos+45,start_enemy_y_pos+24,WHITE);
    enemy_show(&enemy_14);
    
    enemy_init(&enemy_15,start_x_pos+60,start_enemy_y_pos+24,WHITE);
    enemy_show(&enemy_15);
    
    // Put enemy objects into array
    enemyArray[0] = &enemy_1;
    enemyArray[1] = &enemy_2;
    enemyArray[2] = &enemy_3;
    enemyArray[3] = &enemy_4;
    enemyArray[4] = &enemy_5;
    enemyArray[5] = &enemy_6;
    enemyArray[6] = &enemy_7;
    enemyArray[7] = &enemy_8;
    enemyArray[8] = &enemy_9;
    enemyArray[9] = &enemy_10;
    enemyArray[10] = &enemy_11;
    enemyArray[11] = &enemy_12;
    enemyArray[12] = &enemy_13;
    enemyArray[13] = &enemy_14;
    enemyArray[14] = &enemy_15;
}

// Draw and initialize the 4 barriers that the player can hide behind.
void draw_barriers_level() {
        // Initialize local variables
    unsigned int start_x_pos = 5; // where the first barrier begins
    unsigned int start_barrier_y_pos = 95;  // the y position of the barriers.
    
    barrier_init(&barrier_1,start_x_pos,start_barrier_y_pos,GREEN); // initialize x-pos and y-pos and color of barrier
    barrier_show(&barrier_1); // displays the barrier on uLCD
    barrier_init(&barrier_2,start_x_pos+32,start_barrier_y_pos,GREEN);
    barrier_show(&barrier_2);
    barrier_init(&barrier_3,start_x_pos+64,start_barrier_y_pos,GREEN);
    barrier_show(&barrier_3);
    barrier_init(&barrier_4,start_x_pos+96,start_barrier_y_pos,GREEN);
    barrier_show(&barrier_4);
}

// Draws the player at the initial starting location
void draw_initial_player()
{
    int start_x_pos = 59; // x pos of player (initial)
    int start_y_pos = 120; // y pos of player
    
    player_init(&player,start_x_pos,start_y_pos,WHITE); // intialize x-pos and y-pos and color of player
    player_show(&player); // display player on uLCD
}

// Checks all enemies in row 1
void check_hit_enemy_row1()
{
    int hit_enemy;
    
    // First Row of Enemies
    hit_enemy = check_enemy(&enemy_1, &missile); // checks if the missile hits the enemy and returns 1 if hit, 0 if not hit
    numOfEnemies = numOfEnemies - hit_enemy; // updates the number of enemies left
    hit_enemy = check_enemy(&enemy_2, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_3, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_4, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_5, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
}

// Same as check_hit_enemy_row1, but checks enemies at row 2
void check_hit_enemy_row2()
{
    int hit_enemy;
    
    // Second Row of Enemies
    hit_enemy = check_enemy(&enemy_6, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_7, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_8, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_9, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_10, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
}

// Same as check_hit_enemy_row1, but checks enemies at row 3
void check_hit_enemy_row3()
{
    int hit_enemy;

    // Third Row of Enemies
    hit_enemy = check_enemy(&enemy_11, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_12, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_13, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_14, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
    hit_enemy = check_enemy(&enemy_15, &missile);
    numOfEnemies = numOfEnemies - hit_enemy;
}

// Checks if player is hit
void check_player_hit()
{    
    hit_player = check_player(&player, &enemy_missile); // checks if the missile hits the player and returns 1 if hit, 0 if not hit
}

// Randomly selects an enemy to fire and updates the position of where the missile will fire from
void random_attack_gen()
{
    firing_col = rand() % 5; // selects a random column
    
    // first checks if the 3rd row closest to the player is alive
    if (enemyArray[firing_col+10]->status == ENEMY_ALIVE)
    {
        // If alive, the enemy missile position is updated to the center of the enemy
        enemy_missile.missile_blk_x = enemyArray[firing_col+10]->enemy_blk_x + (enemyArray[firing_col+10]->enemy_width/2);
        enemy_missile.missile_blk_y = enemyArray[firing_col+10]->enemy_blk_y + enemyArray[firing_col+10]->enemy_height + 1;
        enemy_missile.status = ENEMY_MISSILE_ACTIVE; // sets the enemy missile as active
    }
    // if enemy at 3rd row is dead, checks the enemy in the 2nd row
    else if (enemyArray[firing_col+5]->status == ENEMY_ALIVE)
    {
        // If alive, the enemy missile position is updated to the center of the enemy
        enemy_missile.missile_blk_x = enemyArray[firing_col+5]->enemy_blk_x + (enemyArray[firing_col+5]->enemy_width/2);
        enemy_missile.missile_blk_y = enemyArray[firing_col+5]->enemy_blk_y + enemyArray[firing_col+5]->enemy_height + 1;
        enemy_missile.status = ENEMY_MISSILE_ACTIVE;
    }
    // if enemy at 2nd and 3rd row is dead, checks the enemy in the 1st row
    else if (enemyArray[firing_col]->status == ENEMY_ALIVE)
    {
        // If alive, the enemy missile position is updated to the center of the enemy
        enemy_missile.missile_blk_x = enemyArray[firing_col]->enemy_blk_x + (enemyArray[firing_col]->enemy_width/2);
        enemy_missile.missile_blk_y = enemyArray[firing_col]->enemy_blk_y + enemyArray[firing_col]->enemy_height + 1;
        enemy_missile.status = ENEMY_MISSILE_ACTIVE;
    }
}

// moves the enemy
void enemy_motion()
{
    // will move the enemy every 6 loops
    if (ENEMY_MOVE % 6 == 0)
    {
        // FIrst Row of Enemies
        MOVE_DOWN = move_enemy(&enemy_1, MOVE_DOWN, DIRECTION); // moves the enemy based on the DIRECTION passed in and also sends global MOVE_DOWN to update in enemy.cpp
        MOVE_DOWN = move_enemy(&enemy_2, MOVE_DOWN, DIRECTION); // MOVE_DOWN will be 1 enemies reach the left or right edge of the screen
        MOVE_DOWN = move_enemy(&enemy_3, MOVE_DOWN, DIRECTION); // MOVE_DOWN will be 2 if enemies reach the player, otherwise 0
        MOVE_DOWN = move_enemy(&enemy_4, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_5, MOVE_DOWN, DIRECTION); 
        
        // Second Row of Enemies
        MOVE_DOWN = move_enemy(&enemy_6, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_7, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_8, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_9, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_10, MOVE_DOWN, DIRECTION);
        
        // Third Row of Enemies
        MOVE_DOWN = move_enemy(&enemy_11, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_12, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_13, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_14, MOVE_DOWN, DIRECTION);
        MOVE_DOWN = move_enemy(&enemy_15, MOVE_DOWN, DIRECTION);
        
        // if MOVE_DOWN is 2, then the enemies have reached the player
        if (MOVE_DOWN == 2)
        {
            lose = true; // set global lose to true to go to gameover screen
        }
        
        // if MOVE_DOWN is 1, update the y-pos of the enemies
        if (MOVE_DOWN == 1)
        {
            move_enemy_down(); // updates the y-pos of the enemies
            
            // Flips the direction when the enemy moves down
            if (DIRECTION == 1)
            {
                DIRECTION = 2;
            }
            else
            {
                DIRECTION = 1;
            }
            MOVE_DOWN = 0; // sets MOVE_DOWN back to 0 to stop down movement until 
        }  
        ENEMY_MOVE += 1;
    }
    else
    {
        ENEMY_MOVE += 1;
    }
}

// moves all the enemies down in the y-direction
void move_enemy_down()
{
    // First Row of Enemies
    enemy_erase(&enemy_1);
    enemy_erase(&enemy_2);
    enemy_erase(&enemy_3);
    enemy_erase(&enemy_4);
    enemy_erase(&enemy_5);
    
    enemy_1.enemy_blk_y += enemy_1.enemy_height+4;
    enemy_2.enemy_blk_y += enemy_2.enemy_height+4;
    enemy_3.enemy_blk_y += enemy_3.enemy_height+4;
    enemy_4.enemy_blk_y += enemy_4.enemy_height+4;
    enemy_5.enemy_blk_y += enemy_5.enemy_height+4;
    
    // Second Row of Enemies
    enemy_erase(&enemy_6);
    enemy_erase(&enemy_7);
    enemy_erase(&enemy_8);
    enemy_erase(&enemy_9);
    enemy_erase(&enemy_10);
    
    enemy_6.enemy_blk_y += enemy_6.enemy_height+4;
    enemy_7.enemy_blk_y += enemy_7.enemy_height+4;
    enemy_8.enemy_blk_y += enemy_8.enemy_height+4;
    enemy_9.enemy_blk_y += enemy_9.enemy_height+4;
    enemy_10.enemy_blk_y += enemy_10.enemy_height+4;
    
    // Third Row of Enemies
    enemy_erase(&enemy_11);
    enemy_erase(&enemy_12);
    enemy_erase(&enemy_13);
    enemy_erase(&enemy_14);
    enemy_erase(&enemy_15);
    
    enemy_11.enemy_blk_y += enemy_11.enemy_height+4;
    enemy_12.enemy_blk_y += enemy_12.enemy_height+4;
    enemy_13.enemy_blk_y += enemy_13.enemy_height+4;
    enemy_14.enemy_blk_y += enemy_14.enemy_height+4;
    enemy_15.enemy_blk_y += enemy_15.enemy_height+4;
}

// thread that plays sounds during game
void playstart(void const *args)//Th
{   //Depending on the state of the game,
    //queue either screen music or play sound effect upon fire
    while(true) {
        FILE *wave_file;
        
        // plays intro music during menu screen: Edited from https://freesound.org/people/VABsounds/sounds/443865/ 
        while(game_menu)
        {
            SDLock.lock(); // mutex lock the SD
            wave_file=fopen("/sd/wavfiles/futureEdit2.wav","r");
            if(wave_file==NULL) pc.printf("file open error!\n\n\r"); // added this for open error check
            waver.play(wave_file);
            fclose(wave_file);
            SDLock.unlock();
        }
        
        // Checks in game sound conditions
        while(begin_game) // // ADD || begin_game2 FOR 2 PLAYER
        {
            // play firing sound when the player fires: Edited from https://freesound.org/people/nsstudios/sounds/321102/
            if(!pb && missile.status == PLAYER_MISSILE_INACTIVE) {
 
                SDLock.lock(); // mutex lock the SD
                wave_file=fopen("/sd/wavfiles/laserEdit.wav","r");
                if(wave_file==NULL) pc.printf("laser file open error!\n\n\r"); // added this for open error check
 
                waver.play(wave_file);
                fclose(wave_file);
                SDLock.unlock();
            }
            
            // if player hit, play hit sound. Edited from https://freesound.org/people/tcpp/sounds/77339/ 
            if (hit_player)
            {
                SDLock.lock(); // mutex lock the SD
                wave_file=fopen("/sd/wavfiles/bigExplosionEdit2.wav","r");
                if(wave_file==NULL) pc.printf("explosion file open error!\n\n\r"); // added this for open error check
                waver.play(wave_file);
                fclose(wave_file);
                SDLock.unlock();
            }
        }
        
        // victory music: Edited from https://freesound.org/people/honeybone82/sounds/513253/ 
        while(win) {
            SDLock.lock(); // mutex lock the SD
            wave_file=fopen("/sd/wavfiles/victoryEdit2.wav", "r");
            if(wave_file==NULL) pc.printf("explosion file open error!\n\n\r"); // added this for open error check
            waver.play(wave_file);
            fclose(wave_file);
            SDLock.unlock();
        }
            
        // gameover music: Edited from https://freesound.org/people/themusicalnomad/sounds/253886/ 
        while(gameover)
        {
            SDLock.lock(); // mutex lock the SD
            wave_file=fopen("/sd/wavfiles/comicalgameoverEdit.wav","r");
            if(wave_file==NULL) pc.printf("gameover file open error!\n\n\r"); // added this for open error check
            waver.play(wave_file);
            fclose(wave_file);
            SDLock.unlock();
        }
    }
}


// thread that adds RGB LED Lighting Effects that coincide with the game -- Brice
void ledEffects(void const *args)//Th
{   //Depending on the state of the game,
    //generate different patterns/colors of lighting
    while(1) {
        // gradually brightens and dims each color independently (maximum of 0.5). (A chill build up effect?)
        while(game_menu)
        {
            red = 0.0;
            green = 0.0;
            blue = 0.0;
            for (float i = 0.0; i < 0.5; i = i + 0.05) {
                red = i;
                Thread::wait(10);
            }
            red = 0.5;
            Thread::wait(300);
            for (float i = 0.5; i > 0.0; i = i - 0.05) {
                red = i;
                Thread::wait(10);
            }
            red = 0.0;
            for (float i = 0.0; i < 0.25; i = i + 0.05) {
                green = i;
                Thread::wait(10);
            }
            green = 0.25;
            Thread::wait(300);
            for (float i = 0.25; i > 0.0; i = i - 0.05) {
                green = i;
                Thread::wait(10);
            }
            green = 0.0;
            for (float i = 0.0; i < 0.5; i = i + 0.05) {
                blue = i;
                Thread::wait(10);
            }
            blue = 0.5;
            Thread::wait(300);
            for (float i = 0.5; i > 0.0; i = i - 0.05) {
                blue = i;
                Thread::wait(10);
            }
            blue = 0.0;
        }
        // Checks in game lighting conditions
        while(begin_game) // ADD || begin_game2 FOR 2 PLAYER
        {
            // play firing sound when the player fires. SEEMS TO BE HIT OR MISS RN
            if(!pb && missile.status == PLAYER_MISSILE_INACTIVE) {
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                //red = 0.5;
                //Thread::wait(200);
                red = 0.5;
                green = 0.15;
                Thread::wait(300);
                red = 0.0;
                green = 0.0;
            }
            
            // if player hit, flash a red led. (0.5 maximum)
            if (hit_player)
            {
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                red = 0.5;
                Thread::wait(60);
                red = 0.0;
                Thread::wait(60);
                red = 0.5;
                Thread::wait(60);
                red = 0.0;
                Thread::wait(60);
                red = 0.5;
                Thread::wait(60);
                red = 0.0;
            }
            Thread::wait(500);
        }
        
        // during a win, slowly brighten the green LED, hold it at 0.25 and then dim it.
        while(win) {
            for (float i = 0.0; i < 0.25; i = i + 0.05) {
                green = i;
                Thread::wait(10);
            }
            green = 0.25;
            Thread::wait(300);
            for (float i = 0.25; i > 0.0; i = i - 0.05) {
                green = i;
                Thread::wait(10);
            }
            green = 0.0;
            Thread::wait(500);
        }
        
        // during a gameover, slowly brighten the red LED, hold it at 0.25, and then dim it
        while(gameover)
        {
            for (float i = 0.0; i < 0.25; i = i + 0.05) {
                red = i;
                Thread::wait(10);
            }
            red = 0.25;
            Thread::wait(300);
            for (float i = 0.25; i > 0.0; i = i - 0.05) {
                red = i;
                Thread::wait(10);
            }
            red = 0.0;
            Thread::wait(500);
        }
    }
}

// SLAVE AND MASTER THREADS BELOW ARE USED ONLY WITH 2 PLAYER (STILL BUGGY AND REQUIRES EXTRA HARDWARE)
// UNCOMMENT THIS THREAD IF SECOND PLAYER AND COMMENT MASTER THREAD

// The slave mbed device (second player) should uncomment this thread -- Brice
/*
void mbedSlave(void const *args) {
    char rx;
    while(1) {
        //while(numPlayers == 1) Thread::yield();
        rx = '0';
        pc.printf("Slave thread running.\n\r");
        if (secondMbed.readable()) {
            rx = secondMbed.getc();
            pc.printf("rx = %c\n\r", rx);
            //if (!begin_game2 && rx == 'S') {
            while (!secondMbed.writeable()) wait(0.5);
            secondMbed.putc(rx);
        }
        Thread::wait(1000);
    }
}
*/

                //first_player_ready = true;
                //second_player_ready = true;
            //    begin_game2 = true;
            //    numPlayers = 2;
            //} else if (begin_game2 && rx == 'W') {
            //    secondMbed.putc(rx);
                //first_player_ready = false;
                //second_player_ready = false;
            //    begin_game2 = false;
            //    two_player_lose = true;
            //}
        //}*/
        /*
        if (begin_game2 && two_player_win) {
            while(!secondMbed.writeable()) wait(0.5);
            secondMbed.putc('W');
            while(!secondMbed.readable()) wait(0.5); // ok to lock up with wait because we don't want to confirm when before anything else. --Brice
            rx = secondMbed.getc();
            if (rx == 'W') {
                begin_game2 = false;
                //first_player_ready = false;
                //second_player_ready = false;
            }
        }
        */
//        Thread::wait(1000);
//    }
//}



// UNCOMMENT THIS THREAD IF FIRST PLAYER AND COMMENT SLAVE THREAD
// The master mbed device (second player) should uncomment this thread -- Brice
/*
void mbedMaster(void const *args) {
    char rx;
    while(1) {
        pc.printf("Master called\n\r");
        while(numPlayers == 1) Thread::yield();
        pc.printf("Num players is 2.\n\r");
        rx = '0';
        if (!begin_game2) {
            while(!secondMbed.writeable()) {
               // pc.printf("not writeable");
                wait(0.5);
            }
            secondMbed.putc('S');
            while(!secondMbed.readable()) {
                // pc.printf("no read\n\r");
                 wait(0.5); // okay to lock up until can confirm game is ready. --Brice
            }
            rx = secondMbed.getc();
            pc.printf("rx = %c\n\r", rx);
            if (rx == 'S') {
                begin_game2 = true;
                pc.printf("both players ready\n\r");
            }
        } else {
            while (begin_game2) {
                rx = '0';
                if (secondMbed.readable()) {
                    rx = secondMbed.getc();
                    if (rx == 'W') {
                        secondMbed.putc(rx);
                        begin_game2 = false;
                        two_player_lose = true;
                    }
                }
                if (two_player_win) {
                    secondMbed.putc('W');
                    while(!secondMbed.readable()) wait(0.5); // ok to lock up with wait because we don't want to confirm when before anything else. --Brice
                    rx = secondMbed.getc();
                    if (rx == 'W') {
                        begin_game2 = false;
                    }
                }
                Thread::wait(1000);
            }
        }
        Thread::wait(1000);
    }
}
*/

int main() {
     IMU.begin(); // start the IMU
     if (!IMU.begin()) { // print error message if error with IMU
         pc.printf("Failed to communicate with IMU\n\r");
    }
    IMU.calibrate(1); // calibrate the IMU
     // Initialization of Setup variables
     int blk_x, blk_y;
     pb.mode(PullUp); // set pushbutton to PullUp mode
     
     Thread thread(playstart); // intializes the thread to play sound
     // Should only have the Slave thread uncommented if second player.
     // Should only have the Master thread uncommented if first player.
     //Thread thread2(mbedSlave); // uncommented if second player -- Brice
     //Thread thread3(mbedMaster); // uncommented if first player -- Brice
     Thread thread4(ledEffects); // thread added for LED lighting effects -- Brice
     //secondMbed.baud(9600); // set the secondMbed serial baud rate to 9600. Two-player only.
     uLCD.baudrate(3000000); // set to 3000000 (the maximum baud rate for the LCD) to increase smooth gameplay
     
     // Initialization of Game Menu variables
     // Additional globals added for two-player and one-player capabilities (by Brice)
    const int title_x_pos = 2; // initial x-pos of title
    const int title_y_pos = 3; // initial y-pos of title
    int start_label_x_pos = 7; // start label x-pos
    int start_label_y_pos = 7; // start label y-pos
    int level_cursor_x_pos = 5; // level cursor x-position
    int level_cursor_y_pos_start = 7; // level cursor y-position
    //int level_cursor_y_pos = 7; // initial level_cursor_y_pos @ start -- Added by Brice for more menu options
    int level_cursor_y_pos_end = 11; // BOTTOM CURSOR POS -- Added by Brice for more menu options
    int gameover_x_pos = 5; // gameover label x-position
    int gameover_y_pos = 5; // gameover label y-position
    int win_x_pos = 2; // congratulations label x-position
    int win_y_pos = 4; // congratulations label y-position
    int startover_x_pos = 3; // startover label x-position
    int startover_y_pos = 7; // startover label y-position
    int newAnalogClick = 0; // to prevent the analog click from changing multiple skins with one click
    int prevAnalogClick = 0; // to prevent the analog click from changing multiple skins with one click
    
    // intialize temporary score and current score
    int temp = 0;
    int score = 0;
     
     // variables and code used to read the best time from the SD card and store in storedTime for display on the LCD
     // idea for best-time reading and updating: https://os.mbed.com/questions/75718/i-want-to-assign-int-value-saved-in-sd-t/
    char buffer[3] = {0};
    char c = {0};
    char *token;
    int i = 0;
    int storedTime = 999;
    SDLock.lock(); // mutex lock
    FILE *sdtime;
    sdtime=fopen("/sd/besttime.txt","r"); // open besttime.txt file
    if(sdtime==NULL) pc.printf("file open error!\n\n\r"); // added this for open error check
    // read the characters for the best time stored in besttime.txt from SD card and store in buffer
    while ((c != '\n') && (i < 3)) {
        c = fgetc(sdtime);
        buffer[i] = c;
        i++;
    }
    fclose(sdtime);
    SDLock.unlock();
    token = strtok(buffer, "\n");
    storedTime = atoi(token); // convert string from file to integer. this is the stored best time.
    // Begin game loop
    while(1)
    {
        // initialize all starting conditions for the beginning of the game
        game_menu = true; // defaults to display menu screen
        ENEMY_MOVE = true; // defaults to move enemy
        DIRECTION = 1; // default to move right
        hit_player = 0; // default to not player hit
        MOVE_DOWN = 0; // default to not move down
        lose = false; // default to not lose
        lives1 = 3; // defaults to 3 lives
        lives2 = 2; // 2 lives for medium
        lives3 = 1; // 1 life for hard
        score = 0; // default to score of 0
        int level_cursor_y_pos = 7; // initial level_cursor_y_pos @ start -- Added by Brice for more menu options
        uLCD.cls();
        // Brice moved this out of the loop since it shouldn't change 
        //uLCD.locate(title_x_pos,title_y_pos); // "SPACE INVADERS" title position
        //uLCD.printf("SPACE INVADERS"); // Title
        // Implementation of game_menu
        while(game_menu) 
        {
            float accelY = 0.0; // y acceleration
            // Brice added this in order to move the cursor through the menu options
            uLCD.locate(level_cursor_x_pos, level_cursor_y_pos);
            uLCD.printf("  ");
            //if (myNav.down() && level_cursor_y_pos < level_cursor_y_pos_end) { // for previous Nav switch
            // Control menu with Analog Joystick
            if ((stick.angle() <= 280 && stick.angle() >= 260) && level_cursor_y_pos < level_cursor_y_pos_end) {
                level_cursor_y_pos += 2; // move cursor down if analog is pushed down. (and don't let it go past bottom option).
            //} else if (myNav.up() && level_cursor_y_pos > level_cursor_y_pos_start) {
            } else if ((stick.angle() <= 100 && stick.angle() >= 80) && level_cursor_y_pos > level_cursor_y_pos_start) {
                level_cursor_y_pos -= 2; // move cursor up if analog is pushed up (and don't let it go past top option)
            }
            // Control Menu with IMU. Read the y acceleration if it can.
            if (IMU.accelAvailable()) {
                IMU.readAccel();
                accelY = IMU.calcAccel(IMU.ay);
                //pc.printf("Calc Accel: %f", accelY);
            }
            if ((accelY >= 0.25) && level_cursor_y_pos < level_cursor_y_pos_end) {
                level_cursor_y_pos += 2; // move cursor down if tilted down (and don't let go past bottom option).
            //} else if (myNav.up() && level_cursor_y_pos > level_cursor_y_pos_start) { // nav switch remnant.
            } else if ((accelY <= -0.25) && level_cursor_y_pos > level_cursor_y_pos_start) {
                level_cursor_y_pos -= 2; // move cursor up if tilted up (and don't let go past top option).
            }
            // end of movable cursor
            uLCD.locate(level_cursor_x_pos,level_cursor_y_pos); // draws cursor next to "START" label
            uLCD.printf("->");
            
            uLCD.locate(title_x_pos,title_y_pos); // "SPACE INVADERS" title position
            uLCD.printf("SPACE INVADERS"); // Title
            
            //uLCD.locate(start_label_x_pos,start_label_y_pos); // "START" label position // original menu option.
            //uLCD.printf("ONE-PLAYER");
            
            uLCD.locate(start_label_x_pos,start_label_y_pos); // "START" label position. LEVEL 1: Easiest level
            uLCD.printf("LEVEL 1");
            
            uLCD.locate(start_label_x_pos,start_label_y_pos + 2); // "LEVEL 2" level. Medium difficulty.
            uLCD.printf("LEVEL 2");
            
            uLCD.locate(start_label_x_pos,start_label_y_pos + 4); // "LEVEL 3" level. Hardest difficulty.
            uLCD.printf("LEVEL 3");
            
            //uLCD.locate(start_label_x_pos, start_label_y_pos + 6);
            //uLCD.printf("TWO-PLAYER");
            
            uLCD.locate(2,13);
            uLCD.printf("Best time: %d s", storedTime); // print the stored best time at the bottom of the LCD.
            // if pushbutton is pressed, game menu is exited and game begins
            if(!pb) 
            { 
                game_menu = false;
                if (level_cursor_y_pos == start_label_y_pos) {
                    //numPlayers = 1;
                    level = 1; // select level 1
                } else if (level_cursor_y_pos == start_label_y_pos + 2) {
                    level = 2; // select level 2
                } else if (level_cursor_y_pos == start_label_y_pos + 4) {
                    level = 3; // select level 3
                }
                //} else if (level_cursor_y_pos == start_label_y_pos + 6) { // USED FOR 2-PLAYER ONLY
                    //numPlayers = 2;
                    //level = 1;
                //}
                Thread::wait(500); // changed this to Thread::wait ... originally wait(0.5);
            }
        }
        //while(numPlayers != 1 && !begin_game2) Thread::yield(); // added to force wait with two-player and one player not ready. -- added by Brice
        //if (numPlayers == 2 && begin_game2) { // FOR 2-PLAYER ONLY
          //  numWins = 0;
          //  bestTimer.start();
        //} else {
        begin_game = true; // defaults begin_game to true
            
        // Start timer to check for best time in single player.
        bestTimer.start();
        //}
        // clear the screen to start drawing the level
        uLCD.cls();

        // Draw the enemies
        draw_enemies_level();

        // Draw the player
        draw_initial_player();
        
        // Draw the barriers
        draw_barriers_level();
        // erase 2 leftmost and rightmost barriers if level 2 or 3 (harder)
        if (level == 2 || level == 3) {
            barrier_erase(&barrier_1);
            barrier_erase(&barrier_4);
        }
        // erase the 3rd barrier if level 3 (even harder)
        if (level == 3) {
            barrier_erase(&barrier_3);
        }
        // sets the initial position of player missile and enemy missile (later updated)
        blk_x = player.player_blk_x+(player.player_width/2);
        blk_y = player.player_blk_y;
        missile_init(&missile, blk_x, blk_y, WHITE);
        int e_blk_x = 0;
        int e_blk_y = 2;
        enemy_missile_init(&enemy_missile, e_blk_x, e_blk_y, WHITE);
        
        // prints lives depending on which level was selected.
        if (level == 1) {
            uLCD.locate(0,0);
            uLCD.printf("Lives:%i", 3);
        } else if (level == 2) {
            uLCD.locate(0,0);
            uLCD.printf("Lives:%i", 2);
        } else if (level == 3) {
            uLCD.locate(0,0);
            uLCD.printf("Lives:%i", 1);
        }
        //uLCD.locate(0,0);
        //uLCD.printf("Lives:%i", lives); // original, basic lives

        // prints score (no competition possible since the score always ends as the same)
        uLCD.locate(9,0);
        uLCD.printf("Score:%i", score);
        
        // game play loop
        while(begin_game) 
        {
            // updates score
            temp = score;
            score = (15-numOfEnemies)*15;
            
            // prints score if score changes (score always ends at same number for win)
            if (score != temp)
            {
                uLCD.locate(9,0);
                uLCD.printf("Score:%i", score);
            }
            
            // move enemy
            enemy_motion();
            
            // check barriers for player missile hit. Player missile hit damages the barriers
            if (missile.missile_blk_y+1-missile.missile_height >= barrier_1.barrier_blk_y
                    && missile.missile_blk_y+1-missile.missile_height <= barrier_1.barrier_blk_y+barrier_1.barrier_height
                    && missile.status == PLAYER_MISSILE_ACTIVE) 
            {
                if (level == 1) {
                    check_barrier(&barrier_1, &missile);
                }
                check_barrier(&barrier_2, &missile);
                if (level == 1 || level == 2) {
                    check_barrier(&barrier_3, &missile);
                }
                if (level == 1) {
                    check_barrier(&barrier_4, &missile);
                }
            }
            
            // checks if player missile passes y-pos of row1
            if (missile.missile_blk_y+1-missile.missile_height <= enemy_1.enemy_blk_y
                    && missile.missile_blk_y+1-missile.missile_height >= enemy_1.enemy_blk_y-enemy_1.enemy_height) 
            {
                check_hit_enemy_row1();
            }

            // checks if player missile passes y-pos of row2
            if (missile.missile_blk_y+1-missile.missile_height <= enemy_6.enemy_blk_y
                    && missile.missile_blk_y+1-missile.missile_height >= enemy_6.enemy_blk_y-enemy_6.enemy_height) 
            {
                check_hit_enemy_row2();
            }

            // checks if player missile passes y-pos of row3
            if (missile.missile_blk_y+1-missile.missile_height <= enemy_11.enemy_blk_y
                    && missile.missile_blk_y+1-missile.missile_height >= enemy_11.enemy_blk_y-enemy_11.enemy_height) 
            {
                check_hit_enemy_row3();
            }
            
            // Random Enemy Fire
            if (enemy_missile.status == ENEMY_MISSILE_INACTIVE) 
            {
                random_attack_gen();
               /* if (level == 2 || level == 3) {
                    random_attack_gen();
                }
                if (level == 3) {
                    random_attack_gen();
                }
                */
            }
            
            // check if enemy missile passes the y-pos of the barrier and updates the barriers if they are hit (damage them, erase blocks of them)
            if (enemy_missile.missile_blk_y >= barrier_1.barrier_blk_y
                    && enemy_missile.missile_blk_y <= barrier_1.barrier_blk_y+barrier_1.barrier_height)
            {
                if (level == 1) {
                    check_barrier(&barrier_1, &enemy_missile);
                }
                check_barrier(&barrier_2, &enemy_missile);
                if (level == 1 || level == 2) {
                    check_barrier(&barrier_3, &enemy_missile);
                }
                if (level == 1) {
                    check_barrier(&barrier_4, &enemy_missile);
                }
            }
            
            // checks if enemy missile passes y-pos of player
            if (enemy_missile.missile_blk_y >= player.player_blk_y
                    && enemy_missile.missile_blk_y <= player.player_blk_y+player.player_height)
            {
                check_player_hit();
            }
            

            update_missile_pos(&missile); // updates player missile position
            update_enemy_missile_pos(&enemy_missile, level); // updates enemy missile position

            // Player Movement checked with navigation switch
            //if (myNav.left() && ((player.player_blk_x-3) > 0))
            
            // With joystick click, change color of player from GREEN -> BLUE -> PINK -> PURPLE -> YELLOW (and loop).
            prevAnalogClick = newAnalogClick;
            newAnalogClick = stick.button();
            if (newAnalogClick && !prevAnalogClick) { // ensures that the button must be released and clicked again before the color changes again. 
                if (player.player_color == 0x00FF00) { // if GREEN (start)
                    player.player_color = 0x0000FF; // BLUE
                } else if (player.player_color == 0x0000FF) { // if BLUE
                    player.player_color = 0xFFC0CB; // pink. hot pink: 0xFF69B4
                } else if (player.player_color == 0xFFC0CB) { // if pink
                    player.player_color = 0x800080; // Purple: 0x800080. periwinkle purple: 0xCCCCFF
                } else if (player.player_color == 0x800080) { // if purple
                    player.player_color = 0xFFFF00; // yellow. metallic gold: 0xD4AF37
                } else if (player.player_color == 0xFFFF00) { // if yellow
                    player.player_color = 0x00FF00; // set back to GREEN
                }
            }
            // Control Player with Analog Joystick -- Brice    
            float stickDist = stick.xAxis();
            if ((stickDist < 0.0) && (player.player_blk_x + stickDist*3 > 0.0)){
                player_erase(&player);
                //player.player_blk_x -= 3;
                player.player_blk_x += (int)(stickDist*3);
                player_show(&player);
            } 
            //else if (myNav.right() && ((player.player_blk_x+3) < (128-player.player_width))) // nav switch
            else if ((stickDist > 0.0) && ((player.player_blk_x + stickDist*3) < (128 - player.player_width))) 
            {
                player_erase(&player);
                player.player_blk_x += (int)(stickDist*3);
                player_show(&player);
            }
            // Control Player with IMU -- Brice
            float accelX = 0.0; // x acceleration
            if (IMU.accelAvailable()) {
                IMU.readAccel();
                accelX = IMU.calcAccel(IMU.ax);
                //pc.printf("Calc Accel: %f", accelY);
            }
            if ((accelX <= -0.25) && (player.player_blk_x + accelX*5) > 0.0) {
                player_erase(&player);
                //player.player_blk_x -= 3;
                player.player_blk_x += (int)(accelX*8);
                player_show(&player);
            //} else if (myNav.up() && level_cursor_y_pos > level_cursor_y_pos_start) {
            } else if ((accelX >= 0.25) && ((player.player_blk_x + accelX*5) < (128 - player.player_width))) {
                player_erase(&player);
                //player.player_blk_x -= 3;
                player.player_blk_x += (int)(accelX*8);
                player_show(&player);
            }
            // Player Fire
            if (pb == 0 && missile.status == PLAYER_MISSILE_INACTIVE) 
            {
                missile.missile_blk_x = player.player_blk_x+(player.player_width/2);
                missile.missile_blk_y = player.player_blk_y;
                missile.status = PLAYER_MISSILE_ACTIVE;
            }
            
            // checks if player destroyed all enemies (a win)
            if (numOfEnemies == 0)
            {
                // idea for best-time reading and updating: https://os.mbed.com/questions/75718/i-want-to-assign-int-value-saved-in-sd-t/
                // check the timer to see if there's a best time.
                int time = bestTimer.read();
                bestTimer.stop();
                bestTimer.reset();
                uLCD.cls();
                char buffer[3] = {0};
                char c = {0};
                char *token;
                int i = 0;
                int storedTime = 999;
                SDLock.lock(); // mutex lock SD
                FILE *sdtime;
                sdtime=fopen("/sd/besttime.txt","r");
                if(sdtime==NULL) pc.printf("file open error!\n\n\r"); // added this for open error check
                // read in characters from stored best time in SD and store in buffer
                while ((c != '\n') && (i < 3)) {
                    c = fgetc(sdtime);
                    buffer[i] = c;
                    i++;
                }
                token = strtok(buffer, "\n");
                storedTime = atoi(token); // convert string from file to integer
                fclose(sdtime);
                // if the new time is less than the stored time, print "NEW BEST TIME" on LCD and the number of seconds recorded by the timer.
                if (time < storedTime) {
                    uLCD.locate(2,10);
                    uLCD.printf("NEW BEST TIME!");
                    uLCD.locate(2,11);
                    uLCD.printf("%d seconds!", time);
                    sdtime = fopen("/sd/besttime.txt", "w");
                    if (sdtime != NULL) {
                        fprintf(sdtime, "%d\r\n", time); // write the new best time to the SD card
                        fclose(sdtime);
                    } else {
                        pc.printf("write: failed!\r\n");
                    }
                }
                SDLock.unlock();
                
                
                win = true; // sets win to true, for win screen
                begin_game = false; // ends game
                
                // displays video clip ????. Original intention here is a little obscure. Might be buggy.
                /*
                uLCD.cls();
                uLCD.media_init();
                uLCD.set_sector_address(0x00, 0x00);
                uLCD.display_video(0,0);
                Thread::wait(1000); // changed from wait(1) to Thread::wait(1000) since we're using threads -- Brice
                */
                
                //uLCD.cls();
                
                // prints "Congratulations" on uLCD    
                uLCD.locate(win_x_pos,win_y_pos);
                uLCD.printf("CONGRATULATIONS!");
                
                // prints "Play Again?" and "Press pb..."
                uLCD.locate(startover_x_pos, startover_y_pos);
                uLCD.printf("Play Again?");
                uLCD.locate(startover_x_pos, startover_y_pos+1);
                uLCD.printf("Press pb...");
                
                // waits at win screen until pushbutton is pressed
                while (win)
                {
                    // if pb is pressed, reset game to start menu
                    if (!pb)
                    {
                        win = false;
                        Thread::wait(500); // changed from wait(0.5) to Thread::wait(500) since we're using threads
                    }
                }
                
            }
            int prevColor; // used to store the previous color of the player
            // checks if player was hit
            if (hit_player)
            {
                // updates lives
                if (level == 1) { 
                    lives1 -= 1; // level 1 lives
                    prevColor = player.player_color;
                    player_erase(&player);
                    player.player_color = 0xFF0000; // briefly flash the player red.
                    player_show(&player);
                    Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    player_erase(&player);
                    player.player_color = prevColor; // briefly flash the player red and RETURN TO THE PREVIOUS CHOSEN COLOR
                    player_show(&player);
                    hit_player = 0; 
                    player_show(&player);
                    player.status = PLAYER_ALIVE;
                
                // prints updated lives number
                    uLCD.locate(0,0);
                    uLCD.printf("Lives:%i", lives1);
                } else if (level == 2) { // level 2 lives
                    lives2 -= 1;
                    prevColor = player.player_color;
                    player_erase(&player);
                    player.player_color = 0xFF0000; // briefly flash the player red.
                    player_show(&player);
                    Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    player_erase(&player);
                    player.player_color = prevColor; // briefly flash the player red and return to previous color
                    player_show(&player);
                    //Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    hit_player = 0;
                    player_show(&player);
                    player.status = PLAYER_ALIVE;
                
                // prints updated lives number
                    uLCD.locate(0,0);
                    uLCD.printf("Lives:%i", lives2);
                } else if (level == 3) {
                    lives3 -= 1; // level 3 lives
                    prevColor = player.player_color;
                    player_erase(&player);
                    player.player_color = 0xFF0000; // briefly flash the player red.
                    player_show(&player);
                    Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    player_erase(&player);
                    player.player_color = prevColor; // briefly flash the player red and return to previous color.
                    player_show(&player);
                    //Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    hit_player = 0;
                    player_show(&player);
                    player.status = PLAYER_ALIVE;
                
                // prints updated lives number
                    uLCD.locate(0,0);
                    uLCD.printf("Lives:%i", lives3);
                }
            }   
            
            // if player loses all lives or enemy reaches the player
            if (lose || lives1 == 0 || lives2 == 0 || lives3 == 0)
            {    
                begin_game = false; // set to false to end game
                uLCD.cls(); // clear the game screen
                
                gameover = true; // set to go to display gameover screen
                
                // prints "GAMEOVER" to uLCD
                uLCD.locate(gameover_x_pos, gameover_y_pos);
                uLCD.printf("GAMEOVER");
                Thread::wait(1000); // changed from wait(1) to thread::wait since we're using threads -- Brice
                
                // prints "Play Again?" and "Press pb..."
                uLCD.locate(startover_x_pos, startover_y_pos);
                uLCD.printf("Play Again?");
                uLCD.locate(startover_x_pos, startover_y_pos+1);
                uLCD.printf("Press pb...");
                
                // stays in gameover screen until pb is pressed
                while (gameover)
                {
                    // if pb is pressed, game is reset to the game menu screen
                    if (!pb)
                    {
                        gameover = false;
                        game_menu = true;
                        Thread::wait(500); // changed wait(0.5) to Thread::wait since we're using threads -- Brice
                    }
                }
            }

        }
                // game play loop
    /* TWO-PLAYER MODE: BUGGY, BUT THE PRIMITIVES ARE IN PLACE.
    // I was able to get the two mbeds to send over the proper character codes to get 1 mbed to start the game
    // in 2 player mode. When the 'W' character was received by this mbed, the game stopped with a loss.
    // We lack hardware and time to fully test and work out all of the bugs here, but it is close to being
    // a working feature with the primitives we've commented out here.
        while(begin_game2) 
        {
            temp = score;
            score = (15-numOfEnemies)*15;
            
            // prints score if score changes
            if (score != temp)
            {
                uLCD.locate(9,0);
                uLCD.printf("Score:%i", score);
            }
            
            // move enemy
            enemy_motion();
            // check barriers for player missile hit
            if (missile.missile_blk_y+1-missile.missile_height >= barrier_1.barrier_blk_y
                    && missile.missile_blk_y+1-missile.missile_height <= barrier_1.barrier_blk_y+barrier_1.barrier_height
                    && missile.status == PLAYER_MISSILE_ACTIVE) 
            {
                check_barrier(&barrier_1, &missile);
                check_barrier(&barrier_2, &missile);
                check_barrier(&barrier_3, &missile);
                check_barrier(&barrier_4, &missile);
            }
            
            // checks if player missile passes y-pos of row1
            if (missile.missile_blk_y+1-missile.missile_height <= enemy_1.enemy_blk_y
                    && missile.missile_blk_y+1-missile.missile_height >= enemy_1.enemy_blk_y-enemy_1.enemy_height) 
            {
                check_hit_enemy_row1();
            }

            // checks if player missile passes y-pos of row2
            if (missile.missile_blk_y+1-missile.missile_height <= enemy_6.enemy_blk_y
                    && missile.missile_blk_y+1-missile.missile_height >= enemy_6.enemy_blk_y-enemy_6.enemy_height) 
            {
                check_hit_enemy_row2();
            }

            // checks if player missile passes y-pos of row3
            if (missile.missile_blk_y+1-missile.missile_height <= enemy_11.enemy_blk_y
                    && missile.missile_blk_y+1-missile.missile_height >= enemy_11.enemy_blk_y-enemy_11.enemy_height) 
            {
                check_hit_enemy_row3();
            }
            
            // Random Enemy Fire
            if (enemy_missile.status == ENEMY_MISSILE_INACTIVE) 
            {
                random_attack_gen();
            }
            
            // check if enemy missile passes the y-pos of the barrier and updates the barriers if they are hit.
            if (enemy_missile.missile_blk_y >= barrier_1.barrier_blk_y
                    && enemy_missile.missile_blk_y <= barrier_1.barrier_blk_y+barrier_1.barrier_height)
            {
                check_barrier(&barrier_1, &enemy_missile);
                check_barrier(&barrier_2, &enemy_missile);
                check_barrier(&barrier_3, &enemy_missile);
                check_barrier(&barrier_4, &enemy_missile);
            }
            
            // checks if enemy missile passes y-pos of player
            if (enemy_missile.missile_blk_y >= player.player_blk_y
                    && enemy_missile.missile_blk_y <= player.player_blk_y+player.player_height)
            {
                check_player_hit();
            }
            

            update_missile_pos(&missile); // updates player missile position
            update_enemy_missile_pos(&enemy_missile, level); // updates enemy missile position

            // Player Movement checked with navigation switch
            //if (myNav.left() && ((player.player_blk_x-3) > 0))
            // With joystick click, change color of player from GREEN -> BLUE -> PINK -> PURPLE -> YELLOW (and loop).
            prevAnalogClick = newAnalogClick;
            newAnalogClick = stick.button();
            if (newAnalogClick && !prevAnalogClick) { 
                if (player.player_color == 0x00FF00) { // if GREEN (start)
                    player.player_color = 0x0000FF; // BLUE
                } else if (player.player_color == 0x0000FF) { // if BLUE
                    player.player_color = 0xFFC0CB; // pink. hot pink: 0xFF69B4
                } else if (player.player_color == 0xFFC0CB) { // if pink
                    player.player_color = 0x800080; // Purple: 0x800080. periwinkle purple: 0xCCCCFF
                } else if (player.player_color == 0x800080) { // if purple
                    player.player_color = 0xFFFF00; // yellow. metallic gold: 0xD4AF37
                } else if (player.player_color == 0xFFFF00) { // if yellow
                    player.player_color = 0x00FF00; // set back to GREEN
                }
            }
            // Control Player with Analog Joystick -- Brice    
            float stickDist = stick.xAxis();
            if ((stickDist < 0.0) && (player.player_blk_x + stickDist*3 > 0.0)){
                player_erase(&player);
                //player.player_blk_x -= 3;
                player.player_blk_x += (int)(stickDist*3);
                player_show(&player);
            } 
            //else if (myNav.right() && ((player.player_blk_x+3) < (128-player.player_width)))
            else if ((stickDist > 0.0) && ((player.player_blk_x + stickDist*3) < (128 - player.player_width))) 
            {
                player_erase(&player);
                player.player_blk_x += (int)(stickDist*3);
                player_show(&player);
            }
            // Control Player with IMU -- Brice
            float accelX = 0.0; // x acceleration
            if (IMU.accelAvailable()) {
                IMU.readAccel();
                accelX = IMU.calcAccel(IMU.ax);
                //pc.printf("Calc Accel: %f", accelY);
            }
            if ((accelX <= -0.25) && (player.player_blk_x + accelX*5) > 0.0) {
                player_erase(&player);
                //player.player_blk_x -= 3;
                player.player_blk_x += (int)(accelX*8);
                player_show(&player);
            //} else if (myNav.up() && level_cursor_y_pos > level_cursor_y_pos_start) {
            } else if ((accelX >= 0.25) && ((player.player_blk_x + accelX*5) < (128 - player.player_width))) {
                player_erase(&player);
                //player.player_blk_x -= 3;
                player.player_blk_x += (int)(accelX*8);
                player_show(&player);
            }
            // Player Fire
            if (pb == 0 && missile.status == PLAYER_MISSILE_INACTIVE) 
            {
                missile.missile_blk_x = player.player_blk_x+(player.player_width/2);
                missile.missile_blk_y = player.player_blk_y;
                missile.status = PLAYER_MISSILE_ACTIVE;
            }
            
            // checks if player destroyed all enemies
            if (numOfEnemies == 0)
            {
                // idea for best-time reading and updating: https://os.mbed.com/questions/75718/i-want-to-assign-int-value-saved-in-sd-t/
                int time = bestTimer.read();
                bestTimer.stop();
                bestTimer.reset();
                uLCD.cls();
                char buffer[3] = {0};
                char c = {0};
                char *token;
                int i = 0;
                int storedTime = 999;
                SDLock.lock();
                FILE *sdtime;
                sdtime=fopen("/sd/besttime.txt","r");
                if(sdtime==NULL) pc.printf("file open error!\n\n\r"); // added this for open error check
                while ((c != '\n') && (i < 3)) {
                    c = fgetc(sdtime);
                    buffer[i] = c;
                    i++;
                }
                token = strtok(buffer, "\n");
                storedTime = atoi(token); // convert string from file to integer
                fclose(sdtime);
                if (time < storedTime) {
                    uLCD.locate(2,10);
                    uLCD.printf("NEW BEST TIME!");
                    uLCD.locate(2,11);
                    uLCD.printf("%d seconds!", time);
                    sdtime = fopen("/sd/besttime.txt", "w");
                    if (sdtime != NULL) {
                        fprintf(sdtime, "%d\r\n", time);
                        fclose(sdtime);
                    } else {
                        pc.printf("write: failed!\r\n");
                    }
                }
                SDLock.unlock();
                
                two_player_win = true; // sets win to true, for win screen
                win = true;
                //begin_game = false;
                
                // displays video clip ????
                
                uLCD.cls();
                uLCD.media_init();
                uLCD.set_sector_address(0x00, 0x00);
                uLCD.display_video(0,0);
                Thread::wait(1000); // changed from wait(1) to Thread::wait(1000) since we're using threads -- Brice
                */
                
                //uLCD.cls();
                /*
                // prints "Congratulations" on uLCD    
                uLCD.locate(win_x_pos,win_y_pos);
                uLCD.printf("CONGRATULATIONS!");
                
                // prints "Play Again?" and "Press pb..."
                uLCD.locate(startover_x_pos, startover_y_pos);
                uLCD.printf("Play again?");
                uLCD.locate(startover_x_pos, startover_y_pos+1);
                uLCD.printf("Press pb...");
                
                // waits at win screen until pushbutton is pressed
                while (two_player_win)
                {
                    // if pb is pressed, reset game to start menu
                    if (!pb)
                    {
                        win = false;
                        two_player_win = false;
                        begin_game2 = false;
                        numPlayers = 1;
                        game_menu = true;
                        Thread::wait(500); // changed from wait(0.5) to Thread::wait(500) since we're using threads
                    }
                }
                REMOVED THIS AND MADE THE WINNER THE FIRST TO 1 WIN
                if (numWins == 3) {
                    two_player_win = true;
                    while(two_player_win) {
                        // if pb is pressed, reset game to start menu
                        Thread::wait(2000);
                        uLCD.locate(win_x_pos,win_y_pos);
                        uLCD.printf("CONGRATULATIONS!");
                
                        // prints "Play Again?" and "Press pb..."
                        uLCD.locate(startover_x_pos, startover_y_pos);
                        uLCD.printf("Play Again?");
                        uLCD.locate(startover_x_pos, startover_y_pos+1);
                        uLCD.printf("Press pb...");
                            if (!pb)
                            {
                                two_player_win = false;
                                begin_game2 = false;
                                game_menu = true;
                                Thread::wait(500); // changed from wait(0.5) to Thread::wait(500) since we're using threads
                            }
                    }   
                }
                
                
            }
            int prevColor;
            // checks if player was hit
            if (hit_player)
            {
                // updates lives
                if (level == 1) {
                    lives1 -= 1;
                    prevColor = player.player_color;
                    player_erase(&player);
                    player.player_color = 0xFF0000; // briefly flash the player red.
                    player_show(&player);
                    Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    player_erase(&player);
                    player.player_color = prevColor; // briefly flash the player red.
                    player_show(&player);
                    hit_player = 0; 
                    player_show(&player);
                    player.status = PLAYER_ALIVE;
                
                // prints updated lives number
                    uLCD.locate(0,0);
                    uLCD.printf("Lives:%i", lives1);
                } else if (level == 2) {
                    lives2 -= 1;
                    prevColor = player.player_color;
                    player_erase(&player);
                    player.player_color = 0xFF0000; // briefly flash the player red.
                    player_show(&player);
                    Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    player_erase(&player);
                    player.player_color = prevColor; // briefly flash the player red.
                    player_show(&player);
                    //Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    hit_player = 0;
                    player_show(&player);
                    player.status = PLAYER_ALIVE;
                
                // prints updated lives number
                    uLCD.locate(0,0);
                    uLCD.printf("Lives:%i", lives2);
                } else if (level == 3) {
                    lives3 -= 1;
                    prevColor = player.player_color;
                    player_erase(&player);
                    player.player_color = 0xFF0000; // briefly flash the player red.
                    player_show(&player);
                    Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    player_erase(&player);
                    player.player_color = prevColor; // briefly flash the player red.
                    player_show(&player);
                    //Thread::wait(500); // changed from wait(0.5) to Thread::wait since we're using threads -- Brice
                    hit_player = 0;
                    player_show(&player);
                    player.status = PLAYER_ALIVE;
                
                // prints updated lives number
                    uLCD.locate(0,0);
                    uLCD.printf("Lives:%i", lives3);
                }
            }   
            
            // if player loses all lives or enemy reaches the player
            if (lose || lives1 == 0 || lives2 == 0 || lives3 == 0)
            {    
                //begin_game = false; // set to false to end game
                lives1 = 3;
                uLCD.cls();
                
                two_player_lose = true;
                gameover = true; // set to go to display gameover screen
                numPlayers = 1;
                
                // prints "GAMEOVER" to uLCD
                uLCD.locate(gameover_x_pos, gameover_y_pos);
                uLCD.printf("GAMEOVER");
                Thread::wait(1000); // changed from wait(1) to thread::wait since we're using threads -- Brice
                
                // prints "Play Again?" and "Press pb..."
                uLCD.locate(startover_x_pos, startover_y_pos);
                uLCD.printf("Play again?");
                uLCD.locate(startover_x_pos, startover_y_pos+1);
                uLCD.printf("Press pb...");
                
                // stays in gameover screen until pb is pressed
                while (gameover)
                {
                    // if pb is pressed, game is reset to the game menu screen
                    if (!pb)
                    {
                        gameover = false;
                        two_player_lose = false;
                        game_menu = true;
                        begin_game2 = false;
                        numPlayers = 1;
                        //game_menu = true;
                        Thread::wait(500); // changed wait(0.5) to Thread::wait since we're using threads -- Brice
                    }
                }
            }
            if (two_player_lose) {
                uLCD.cls();
                numPlayers = 1;
                Thread::wait(2000);
                uLCD.locate(gameover_x_pos, gameover_y_pos);
                uLCD.printf("You lost!");
                Thread::wait(1000); // changed from wait(1) to thread::wait since we're using threads -- Brice
                
                // prints "Play Again?" and "Press pb..."
                uLCD.locate(startover_x_pos, startover_y_pos);
                uLCD.printf("Continue?");
                uLCD.locate(startover_x_pos, startover_y_pos+1);
                uLCD.printf("Press pb...");
            }
            while(two_player_lose) {
                //uLCD.cls();
                
                if (!pb)
                    {
                        two_player_lose = false;
                        game_menu = true;
                        begin_game2 = false;
                        numPlayers = 1;
                        Thread::wait(500); // changed wait(0.5) to Thread::wait since we're using threads -- Brice
                    }
                }
            }
        }*/
    }
}
