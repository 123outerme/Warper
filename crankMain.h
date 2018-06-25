#ifndef CRANKMAIN_H_INCLUDED
#define CRANKMAIN_H_INCLUDED

/* ++ Crank Engine version 0.2.0 - last update 6/25/2018 ++
  -- initCrank() error codes:  --
  error code 0: No error
  error code 1: SDL systems failed to initialize
  error code 2: Window could not be created
  error code 3: Renderer failed to initialize
*/

#ifndef CRANK_VERSION
    #define CRANK_VERSION_MAJOR 0
    #define CRANK_VERSION_MINOR 2
    #define CRANK_VERSION_PATCH 1
    #define CRANK_VERSION "0.2.1"
#endif //CRANK_VERSION

//#includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include "SDL/SDL.h"       //This is included because it's an SDL2 program... duh
#include "SDL/SDL_image.h" //This is included so we can use PNGs.
#include "SDL/SDL_ttf.h"   //This is included for text stuff
#include "SDL/SDL_mixer.h" //This is included for audio

//#defines:
#ifndef bool
    #define bool char
    #define false 0
    #define true 1
#endif // bool
#ifndef NULL
    #define NULL ((void*) 0)
#endif //NULL

#define boolToString(bool) (bool ? "true" : "false")
//struct definitions:


//function prototypes:
int initCrank();
void closeCrank();
bool loadIMG(char* imgPath, SDL_Texture** dest);
bool loadTTFont(char* filePath, TTF_Font** dest, int sizeInPts);
SDL_Keycode getKey(bool useMouse);
SDL_Keycode waitForKey(bool useMouse);
int* loadTextTexture(char* text, SDL_Texture** dest, int maxW, SDL_Color color, bool isBlended);

//global variable declarations:
SDL_Window* mainWindow;
SDL_Surface* mainScreen;
SDL_Renderer* mainRenderer;
TTF_Font* mainFont;

int windowW, windowH;
bool canDrawText;
int soundVolume, musicVolume;

#endif // CRANKMAIN_H_INCLUDED
