// main.c BuzzyBee v0.1 for 32bitJam 2021

/*

MIT License

Copyright (c) 2021 NDR008

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

// #include "common/syscalls/syscalls.h"
#include <sys/types.h>
#include <libetc.h>
//#include <libpad.h>
//#include <hardware/pcsxhw.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libspu.h>
#include "graphics/images.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/timerz.h"
#include "engine/audio.h"
#include "sound/sfx/buzz1.h"

// Global system
#define OT_LENGTH 1
#define PACKETMAX 300
#define __ramsize   0x00200000
#define __stacksize 0x00004000


// Global timer
PSXTimer mainTimer;

typedef struct{
    int frame_n;            // this is the current frame
    int index;
    int total_frames;       // this is the total frame
    int y_pos;
    int x_pos;
    int x_vel;
    int y_vel;
    int anim_rate; // how many vsyncs
    Image *img_list;   // make this a list of Image (ok)
} AnimatedObject;

AnimatedObject mainPlayer;
Image mainPlayerL[4];

AnimatedObject pressStart;
Image pressStartL[3];

AnimatedObject Bee;
Image BeeL[1];

AnimatedObject Buzzy;
Image BuzzyL[1];

AnimatedObject jam;
Image jamL[1];


void initialize();
void startScreen();
void gameScreen();
void initPlayer();
void initIntro();
void animate(AnimatedObject *animatedObj);

void initialize() {
	initializeScreen();
    setBackgroundColor(createColor(30, 30, 30));
    audioInit();
    audioTransferVagToSPU(buzz1, buzz1_size, SPU_1CH);

    in_init();  // init inputs
    in_update(); // should not be needed but there is a bug without it
	//initializeDebugFont();
    initPlayer();
    initIntro();
    //load sprites
}

// scaling the graphics (example video width is 320px but "game world" is 320 x factor)
#define factor 1024     
#define GRAVITY factor / 8
#define MAXSPEED factor
#define MAXFLAP 5 * factor
#define SPRITEHEIGHT 30
#define GROUND (SCREEN_HEIGHT-20-30) * factor
#define CEILING (SPRITEHEIGHT)

int gameState = 0; // 0 = start state, 1 = play state, 2 = pause

void updateAnimation(){   
    // if we pressu jump...
    if (input_trig & PAD_CROSS) {
        audioPlay(SPU_1CH, 0x1000);
        mainPlayer.y_vel -= MAXFLAP;
        if (mainPlayer.y_vel < -MAXFLAP) {
            mainPlayer.y_vel = MAXFLAP;
        }
    }
    // constant desire to go down...
    mainPlayer.y_vel += GRAVITY;
    if (mainPlayer.y_vel > MAXSPEED) {
        mainPlayer.y_vel = MAXSPEED;
    }
    
    // update position
    mainPlayer.y_pos += mainPlayer.y_vel;

    // hover on the ground limit
    if (mainPlayer.y_pos > GROUND){
        mainPlayer.y_pos = GROUND;        
    }
    // stay within frame
    else if (mainPlayer.y_pos < CEILING){
        mainPlayer.y_pos = CEILING;
        mainPlayer.y_vel = 0;
    }
    // animate
    animate(&mainPlayer);
}

void gameScreen(){
    animate(&pressStart);
    animate(&Bee);
}

void animate(AnimatedObject *animatedObj){
    Image toDraw;
    if (mainTimer.vsync % animatedObj->anim_rate == 0) {
        animatedObj->frame_n++;
        animatedObj->index = animatedObj->frame_n % animatedObj->total_frames;
    }
    toDraw = moveImage(animatedObj->img_list[animatedObj->index], animatedObj->x_pos / factor, animatedObj->y_pos / factor);
    drawImage(toDraw);
}

void initPlayer(){
    mainPlayer.total_frames = 4;
    mainPlayer.frame_n = 0;
    mainPlayer.index = 0;
    mainPlayer.y_pos = (SCREEN_HEIGHT * 3 / 4) * factor;
    mainPlayer.x_pos = (SCREEN_WIDTH / 4) * factor;
    mainPlayer.y_vel, mainPlayer.x_vel = 0;
    mainPlayer.anim_rate = 4;
    mainPlayerL[0] = createImage(img_bee_0);
    mainPlayerL[1] = createImage(img_bee_1);
    mainPlayerL[2] = createImage(img_bee_2);
    mainPlayerL[3] = mainPlayerL[1];
    mainPlayer.img_list = mainPlayerL;
}

void initIntro(){
    pressStart.total_frames = 2; // third frame is reserved
    pressStart.frame_n = 0;
    pressStart.index = 0;
    pressStart.y_pos = (SCREEN_HEIGHT * 3 / 4) * factor;
    pressStart.x_pos = (SCREEN_WIDTH / 2) * factor;
    pressStart.y_vel, pressStart.x_vel = 0;
    pressStart.anim_rate = 25;
    pressStartL[0] = createImage(img_Press_start1);
    pressStartL[1] = createImage(img_Press_start2);
    pressStartL[2] = createImage(img_Press_start3);
    pressStart.img_list = pressStartL;

    Bee.total_frames = 1; // third frame is reserved
    Bee.frame_n = 0;
    Bee.index = 0;
    Bee.y_pos = (SCREEN_HEIGHT 3 / 4) * factor;
    Bee.x_pos = (SCREEN_WIDTH * 1 / 4) * factor;
    Bee.y_vel, pressStart.x_vel = 0;
    Bee.anim_rate = 25;
    BeeL[0] = createImage(img_Bee);
    Bee.img_list = BeeL;
}

int main() {
    initialize();
    printf("BuzzyBee v0.12 New Animation routine\n");
    mainTimer = createTimer();
    while (1) {
        in_update();
        clearDisplay();
        if (gameState == 1){
            updateAnimation();
        }
        else if (gameState == 0){
            gameScreen();
            if (input_trig & PAD_START) {
                audioPlay(SPU_1CH, 0x1000);
                gameState = 1;
            }
        }
        flushDisplay(); // dump it to the screen
        mainTimer = incTimer(mainTimer);
    }
}
