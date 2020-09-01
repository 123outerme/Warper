#ifndef CSIO_H_INCLUDED
#define CSIO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include "SDL2/SDL.h"       //This is included because it's an SDL2 program... duh
#include "SDL2/SDL_image.h" //This is included so we can use PNGs.
#include "SDL2/SDL_ttf.h"   //This is included for text stuff
#include "SDL2/SDL_mixer.h" //This is included for audio
//#define SDL_MAIN_HANDLED 1

//#defines:
#ifndef bool
    #define bool char
    #define false 0
    #define true 1
    #define boolToString(bool) (bool ? "true" : "false")
#endif // bool
#ifndef NULL
    #define NULL ((void*) 0)
#endif //NULL
#define MAX_KEYMAPS 21

typedef struct _cInputState
{
    const Uint8* keyStates;
    SDL_MouseButtonEvent click;
    bool isClick;  /**< If true, then no mouse input */
    bool quitInput;  /**< True if given any sort of quit-like input */
} cInputState;

//function prototypes:
cInputState cGetInputState(bool useMouse);
SDL_Keycode getKey(bool useMouse);
SDL_Keycode waitForKey(bool useMouse);
bool setKey(SDL_Scancode key, int keyslot);
void handleTextInput(char* text, SDL_Keycode key, int maxChar);

//global variable declarations:
SDL_Scancode keymaps[MAX_KEYMAPS];
#define KEY_UP keymaps[0]
#define KEY_DOWN keymaps[1]
#define KEY_LEFT keymaps[2]
#define KEY_RIGHT keymaps[3]
#define KEY_CONFIRM keymaps[4]
#define KEY_BACK keymaps[5]
#define KEY_CUSTOM(x) keymaps[(x < 16) ? (x > 0 ? 5 + x : 6) : (20)]

#endif // CSIO_H_INCLUDED
