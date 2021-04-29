// The Main HAM Library
#include <mygba.h>
#include <stdio.h>
#include <stdlib.h>

#define GRAVITY 5
#define SPEED 2

MULTIBOOT

/*********************************************************************************
 * Defines
 ********************************************************************************/
// Set the master mixer frequency (maximum bitrate)
// Note that not all values guarantee clickless playback
// please choose from one of these values:
// 5733  Hz   6689 Hz  10512 Hz   11467 Hz
// 13379 Hz  18157 Hz  20068 Hz   21024 Hz
// 26757 Hz  31536 Hz  36314 Hz   40136 Hz
// 42048 Hz  54471 Hz
#define MIXER_FREQ 18157

//Audio FX Include
#include "assets/sfx/SFX_DIE.C"
//#include "assets/sfx/SFX_HIT.C"
#include "assets/sfx/SFX_WING.C"

//Graphics FX Includes
//Backgrounds
#include "assets/gfx/bg/bg.pal.c"
#include "assets/gfx/bg/bg_day.raw.c"
#include "assets/gfx/bg/bg_day.map.c"
#include "assets/gfx/bg/ground.raw.c"
#include "assets/gfx/bg/ground.map.c"

//Sprites - Pallet
#include "assets/gfx/sprites/sprites.pal.c"

//Sprites - YellowBird
#include "assets/gfx/sprites/flappy_up.raw.c"
#include "assets/gfx/sprites/flappy_mid.raw.c"
#include "assets/gfx/sprites/flappy_down.raw.c"

//Sprites - Pipes
#include "assets/gfx/sprites/pipe_up.raw.c"
#include "assets/gfx/sprites/pipe_down.raw.c"

//Sprite - Ground
#include "assets/gfx/sprites/ground.raw.c"

//Sprite - Main MENU
#include "assets/gfx/sprites/flappy_name.raw.c"
#include "assets/gfx/sprites/bird_name.raw.c"
#include "assets/gfx/sprites/pointer.raw.c"
#include "assets/gfx/sprites/btn_start.raw.c"

//Sprite HUD
#include "assets/gfx/sprites/play.raw.c"
#include "assets/gfx/sprites/pause.raw.c"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
// Global Variables

//GFX
u8 bird;// Sprite object number
u8 pipeUpSpt;// Sprite object number
u8 pipeDown[3], pipeUp[3];//Pipes sprites
u8 gameStatus;

//SFX
sample_info* sfx_die;
sample_info* sfx_hit;
sample_info* sfx_wing;

int bird_x = 61;  // X position of bird (column)
int bird_y = 0;   // Y position of bird (row)
u32 frames = 0;   // Global frame counter
u32 animcnt = 0;  // Current frame of animation
u32 ground_x = 0;  // Ground X coordinate
u32 scrollx = 64; // X co-ordinate of scrolling center
u32 scrolly = 64; // Y co-ordinate of scrolling center
u32 zoomx = 256; // X co-ordinate of zoom center
u32 zoomy = 256; // Y co-ordinate of zoom center
int rand_pipe_y[3] = {0, -32, -16};
int rand_pipe_x[3] = {240, 240 + 90, 240 + 180};
int paused = 0;//Paused false
int in_menu = 1; //Indicate if in MAIN MENU
int newframe = FALSE;
map_fragment_info_ptr bg_day;
map_fragment_info_ptr bg_ground;

// Function Prototypes
void mainMenu();
int start();        // VBL function
void move_bird();       // Drop the block
void update_bird();     // Apply block's new position
void query_buttons();   // Query for input
void query_menu_buttons();
void setBgDay();
void setBackGrounds();
void setGround();
void moveGround();
void update_bird_gfx();
void pipesGenerator();
void setPipesBeforeGround();
void pipesMover();
int random();
int checkCollisions();
int collided();
int setBirdSprite();
void startGame();
void gameOver();
void renderMenu();
void rendererHud();
void updateGameStatus();
void initSounds();
void VblFunc(void);
void animateBird();

//Program START -----------------------------------------------------------------------------

// Function: main()
int main()
{
    // Initialize HAMlib
    ham_Init();
    //Initialize DirectSound
    ham_InitMixer(MIXER_FREQ);

    //Set soundFX
    initSounds();
    
    //Set Background day
    setBackGrounds();

    //Create bird sprite
    // Initialize the sprites palette
    ham_LoadObjPal((void*)sprites_Palette, 256);

    // Start the VBL interrupt handler to Main Menu
    in_menu = TRUE;
    renderMenu();
    ham_StartIntHandler(INT_TYPE_VBL,(void*)&VblFunc);

    // Infinite loop to keep the program running
    while(TRUE) {
		
		if(newframe)
		{
			if(F_VCNT_CURRENT_SCANLINE==0)
			{
				//update the mixer once per frame
				ham_UpdateMixer();
				
				newframe=FALSE;
			}
		}
    }

} // End of main()



void startGame(){
    //Start the game
    in_menu = FALSE;

    //Clear the sprites in the screen
    ham_ResetObj();

    // Reset the bird sprite
    bird = setBirdSprite();

    //Setup pipes
    pipesGenerator();
    
    //Renderer HUD
    rendererHud();

    start();
}//End startGame() function


//Function RenderMenu drawn the sprites of main menu
void renderMenu(){
    // Create the sprite and store the object number
    bird = setBirdSprite();

    //Menu Sprites
    ham_CreateObj((void*)flappy_name_Bitmap, OBJ_SIZE_64X32 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0,70,20);
    ham_CreateObj((void*)bird_name_Bitmap, OBJ_SIZE_64X32 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0, (70+50),20);
    ham_CreateObj((void*)btn_start_Bitmap, OBJ_SIZE_32X16 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0,104,72);
    ham_CreateObj((void*)pointer_Bitmap, OBJ_SIZE_16X16 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0,112,90);
    ham_CreateObj((void*)pointer_Bitmap, OBJ_SIZE_16X16 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0,112,90);

    // Copy sprites to hardware
    ham_CopyObjToOAM();
    return;
}


void rendererHud(){
    //HUD Sprites
    gameStatus = ham_CreateObj((void*)play_Bitmap, OBJ_SIZE_16X16 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0,1,1);
}


void gameOver(){//Reset the game

    ham_ResetObj();

    //Reset sprite to initial coordinates
    bird_x = 110;
    bird_y = 50;

    rand_pipe_y[0] = 0;
    rand_pipe_y[1] = -32;
    rand_pipe_y[2] = -16;

    rand_pipe_x[0] = 240;
    rand_pipe_x[1] = 240 + 90;
    rand_pipe_x[2] = 240 + 180;


    //Call main menu loop
    in_menu = TRUE;
    renderMenu();
}


int start(){
    while(TRUE){
        if( newframe && !paused){
            
            // Drop the bird from GRAVITY
            move_bird();

            
            pipesMover(); // Move pipe from right to left
            update_bird(); // Apply bird's new position
            moveGround(); //Move Ground

            if( checkCollisions() ){
                //If collided, play hit sound, reset the game and recall main fucntion;
                if(!sfx_die->playing){
                    ham_PlaySample(sfx_die);
                }
                gameOver();
                return FALSE;
            };

            // Copy sprites to hardware
            ham_CopyObjToOAM();
            
            newframe=FALSE;
        }
    }
}


void query_buttons(){

    // Query Pad to see in what direction
    // the map should be scrolled
    if(F_CTRLINPUT_LEFT_PRESSED)
    {
      --bird_x;
    }
    else
    if(F_CTRLINPUT_RIGHT_PRESSED)
    {
      ++bird_x;
    }

    if(F_CTRLINPUT_DOWN_PRESSED)
    {
      ++bird_y;
    }
    else
    if(F_CTRLINPUT_UP_PRESSED)
    {
      --bird_y;
    }
    if (F_CTRLINPUT_START_PRESSED) {
        if(in_menu){
            startGame();
        }else{
            updateGameStatus();
        }
	}

    if(F_CTRLINPUT_A_PRESSED)
    {
        if (bird_y > 0){
            
            if(!sfx_wing->playing){
                ham_PlaySample(sfx_wing);
            }
            
            bird_y = bird_y - 7;
            if (bird_y < 0) bird_y = 0;
        }else{
            bird_y = 0;
        }
    }

    return;
} // End of query_buttons()


int setBirdSprite(){
    // Create the sprite and store the object number
    return ham_CreateObj((void*)flappy_up_Bitmap, OBJ_SIZE_16X16 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0,110,50);
}//End setBirdSprite function



void setBackGrounds(){
    // Setup the background mode
    ham_SetBgMode(1);

    // Initialize the background palettes
    ham_LoadBGPal((void*)bg_Palette,256);

    // Setup the tileset for our image
    ham_bg[0].ti = ham_InitTileSet(
            (void*)bg_day_Tiles,
            SIZEOF_16BIT(bg_day_Tiles),1,1);
    ham_bg[1].ti = ham_InitTileSet(
            (void*)ground_Tiles,
            SIZEOF_16BIT(ground_Tiles),1,1);

    // Setup the map for our image
    ham_bg[0].mi = ham_InitMapEmptySet(3,0);
    ham_bg[1].mi = ham_InitMapEmptySet(1,0);

    bg_day = ham_InitMapFragment(
        (void*)bg_day_Map,30,20,0,0,30,20,0);
    bg_ground = ham_InitMapFragment(
        (void*)ground_Map,64,32,0,0,64,32,0);

    ham_InsertMapFragment(bg_day,0,0,0);
    ham_InsertMapFragment(bg_ground,1,0,0);

    // Display the background
    ham_InitBg(0,1,3,1);
    ham_InitBg(1,1,0,1);
}

void moveGround(){
    ground_x += 1;
    if( frames % SPEED == 0 ) ham_SetBgXY(1,ground_x,0);
    return;
}

void update_bird(){
    ham_SetObjXY(bird,bird_x,bird_y);
    return;
}

void move_bird(){
    // Check if 5 VBLs have passed
    if (frames % GRAVITY == 0) {
        if (bird_y < 115) bird_y = bird_y + 5;// Drop
    }
    update_bird();
    return;
}


void pipesGenerator(){
    // Create the sprite and store the object number
    pipeDown[0] = ham_CreateObj((void*)pipe_down_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL ,1,0,0,0,0,1,0,rand_pipe_x[0], 0);
    pipeDown[1] = ham_CreateObj((void*)pipe_down_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL ,1,0,0,0,0,1,0,rand_pipe_x[1], 0);
    pipeDown[2] = ham_CreateObj((void*)pipe_down_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[2], 0);

    pipeUp[0] = ham_CreateObj((void*)pipe_up_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[0], 110 );
    pipeUp[1] = ham_CreateObj((void*)pipe_up_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[1], 110 );
    pipeUp[2] = ham_CreateObj((void*)pipe_up_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[2], 110 );

    return;
}



void pipesMover(){

    if (frames % SPEED == 0) {

        if(rand_pipe_x[0] <= -32){
            rand_pipe_x[0] = 240;
            rand_pipe_y[0] = random(-37, 0);
        }
        if(rand_pipe_x[1] <= -32){
             rand_pipe_x[1] = 240;
             rand_pipe_y[1] = random(-37, 0);
        }
        if(rand_pipe_x[2] <= -32){
             rand_pipe_x[2] = 240;
             rand_pipe_y[2] = random(-37, 0);
        }

        ham_SetObjXY(pipeDown[0],rand_pipe_x[0], rand_pipe_y[0]);
        ham_SetObjXY(pipeUp[0],rand_pipe_x[0], rand_pipe_y[0] + 110);

        ham_SetObjXY(pipeDown[1], rand_pipe_x[1], rand_pipe_y[1]);
        ham_SetObjXY(pipeUp[1], rand_pipe_x[1], rand_pipe_y[1] + 110 );

        ham_SetObjXY(pipeDown[2], rand_pipe_x[2], rand_pipe_y[2]);
        ham_SetObjXY(pipeUp[2], rand_pipe_x[2],rand_pipe_y[2] + 110 );

        rand_pipe_x[0] -= 2 ;
        rand_pipe_x[1] -= 2 ;
        rand_pipe_x[2] -= 2 ;
    }
    return;
}

void animateBird()
{
    if(frames % 7 == 0){
        // Figure out where to load the image from and update it
        if( animcnt == 0 ) ham_UpdateObjGfx(bird, (void*)&flappy_up_Bitmap[0]);
        if( animcnt == 1 ) ham_UpdateObjGfx(bird, (void*)&flappy_mid_Bitmap[0]);
        if( animcnt == 2 ) ham_UpdateObjGfx(bird, (void*)&flappy_down_Bitmap[0]);
        // Increment the animation counter
        if (animcnt == 2) {
            animcnt = 0;
        } else {
            animcnt++;
        }
        return;
    }
}

int collided(
    int x1,
    int y1,
    int w1,
    int h1,
    int x2,
    int y2,
    int w2,
    int h2
){
    return x1 < ( x2 + w2 ) && x2 < (x1 + w1) && y1 < ( y2 + h2 ) && y2 < ( y1 + h1 );
}

int checkCollisions(){
     if(
        collided( bird_x, bird_y + 2, 16, 12, rand_pipe_x[0] + 3, rand_pipe_y[0], 26, 64) ||
        collided( bird_x, bird_y + 2, 16, 12, rand_pipe_x[1] + 3, rand_pipe_y[1], 26, 64) ||
        collided( bird_x, bird_y + 2, 16, 12, rand_pipe_x[2] + 3, rand_pipe_y[2], 26, 64) ||

        collided( bird_x, bird_y + 2, 16, 12, rand_pipe_x[0] + 3, rand_pipe_y[0] + 110, 26, 64) ||
        collided( bird_x, bird_y + 2, 16, 12, rand_pipe_x[1] + 3, rand_pipe_y[1] + 110, 26, 64) ||
        collided( bird_x, bird_y + 2, 16, 12, rand_pipe_x[2] + 3, rand_pipe_y[2] + 110, 26, 64) ||

        bird_y >= 115
    ){
        return 1;
    }else{
        return 0;
    }
}

int random(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void updateGameStatus(){
    paused = !paused;
    if(paused){
        ham_UpdateObjGfx(gameStatus, (void*)&pause_Bitmap[0]);
    }else{
        ham_UpdateObjGfx(gameStatus, (void*)&play_Bitmap[0]);
    }
}

void initSounds(){
    
    sfx_die = ham_InitSample(
        (u8*)SFX_DIE_DATA,
        SFX_DIE_LENGTH,
        SFX_DIE_BITRATE
    );

    //sfx_hit = ham_InitSample(
    //    (u8*)SFX_HIT_DATA,
    //    SFX_HIT_LENGTH,
    //    SFX_HIT_BITRATE
    //);
    
    sfx_wing = ham_InitSample(
        (u8*)SFX_WING_DATA,
        SFX_WING_LENGTH,
        SFX_WING_BITRATE
    );
}

// this function gets called once / frame
void VblFunc(void)
{
	//Sync mixer, this should always be
	//called right at the VBL Start
	ham_SyncMixer();

    //Animate the bird
    animateBird();
    
    // Query for input
    query_buttons();

	frames++;
	newframe = TRUE;
}


