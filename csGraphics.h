#ifndef CSGRAPHICS_H_INCLUDED
#define CSGRAPHICS_H_INCLUDED

/* ++ CoSprite 2D Engine ++
  -- initCoSprite() error codes:  --
  error code 0: No error
  error code 1: SDL systems failed to initialize
  error code 2: Window could not be created
  error code 3: Renderer failed to initialize
*/

#ifndef COSPRITE_VERSION
    #define COSPRITE_VERSION_MAJOR 0
    #define COSPRITE_VERSION_MINOR 6
    #define COSPRITE_VERSION_PATCH 1
    #define COSPRITE_VERSION "0.6.1"
#endif //COSPRITE_VERSION
#define SDL_MAIN_HANDLED 1

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
    #define boolToString(bool) (bool ? "true" : "false")
#endif // bool
#ifndef NULL
    #define NULL ((void*) 0)
#endif //NULL
#ifndef PI
    #define PI (3.14159265359879)
    #define radToDeg(x) (180.0 * x / PI)
    #define degToRad(x) (x * PI / 180.0)
#endif // PI



//struct definitions:
typedef struct _cSprite {
    SDL_Texture* texture;
    int id;
    SDL_Rect drawRect;
    SDL_Rect srcClipRect;
    SDL_Point center;
    double scale;
    SDL_RendererFlip flip;
    double degrees;
    int drawPriority;  /**< 0 - not drawn. 1-5 - drawn. Lower number = drawn later */
    bool fixed;  /**< if true, won't be affected by camera movement */
    void* subclass;  /**< fill with any extraneous data or pointer to another struct */
} cSprite;

typedef struct _c2DModel {  //essentially a 2D version of a wireframe model: A collection of sprites with relative coordinates
    cSprite* sprites;
    int numSprites;
    SDL_Rect rect;
    SDL_Point center;
    double scale;
    SDL_RendererFlip flip;
    double degrees;
    int drawPriority;
    bool fixed;  /**< if true, won't be affected by camera movement */
    void* subclass;
} c2DModel;

typedef struct _cText {
    char* string;
    SDL_Texture* texture;
    SDL_Rect rect;
    int drawPriority; /**< 0 - not drawn. 1-5 - drawn. Lower number = drawn later */
    SDL_Color textColor;
    SDL_Color bgColor;
    SDL_RendererFlip flip;
    double degrees;
    bool fixed;  /**< if true, won't be affected by camera movement */
} cText;

typedef struct _cCamera {
    SDL_Rect rect;
    double scale;
    double degrees;
} cCamera;

typedef struct _cResource {
    void* subclass;
    void (*drawingRoutine)(void*);
    int drawPriority; /**< 0 - not drawn. 1-5 - drawn. Lower number = drawn later */
} cResource;

typedef struct _cScene {
    SDL_Color bgColor;
    cCamera* camera;
    cSprite** sprites;
    int spriteCount;
    c2DModel** models;
    int modelCount;
    cResource** resources;
    int resCount;
    cText** strings;
    int stringCount;
} cScene;

//function prototypes:
int initCoSprite();
void closeCoSprite();
bool loadIMG(char* imgPath, SDL_Texture** dest);
bool loadTTFont(char* filePath, TTF_Font** dest, int sizeInPts);
int* loadTextTexture(char* text, SDL_Texture** dest, int maxW, SDL_Color color, bool isBlended);

void initCSprite(cSprite* sprite, SDL_Texture* texture, int id, SDL_Rect drawRect, SDL_Rect srcClipRect, SDL_Point* center, double scale, SDL_RendererFlip flip, double degrees, bool fixed, void* subclass, int drawPriority);
void destroyCSprite(cSprite* sprite);
void drawCSprite(cSprite sprite, cCamera camera, bool update, bool fixedOverride);
void initC2DModel(c2DModel* model, cSprite* sprites, int numSprites, SDL_Point position, SDL_Point* center, double scale, SDL_RendererFlip flip, double degrees, bool fixed, void* subclass, int drawPriority);
void destroyC2DModel(c2DModel* model);
void drawC2DModel(c2DModel model, cCamera camera, bool update);
void initCText(cText* text, char* string, SDL_Rect rect, SDL_Color textColor, SDL_Color bgColor, SDL_RendererFlip flip, double degrees, bool fixed, int drawPriority);
void destroyCText(cText* text);
void drawCText(cText text, cCamera camera, bool update);
void initCResource(cResource* res, void* subclass, void (*drawingRoutine)(void*), int drawPriority);
void drawCResource(cResource* res);
void destroyCResource(cResource* res);
void initCCamera(cCamera* camera, SDL_Rect rect, double scale, double degrees);
void destroyCCamera(cCamera* camera);
void initCScene(cScene* scenePtr, SDL_Color bgColor, cCamera* camera, cSprite* sprites[], int spriteCount, c2DModel* models[], int modelCount, cResource* resources[], int resCount, cText* strings[], int stringCount);
void destroyCScene(cScene* scenePtr);
void drawCScene(cScene* scenePtr, bool redraw);
void drawText(char* input, int x, int y, int maxW, int maxH, SDL_Color color, bool render);
SDL_Point rotatePoint(SDL_Point pt, SDL_Point center, int degrees);

//global variable declarations:
SDL_Window* mainWindow;
SDL_Surface* mainScreen;
SDL_Renderer* mainRenderer;
TTF_Font* mainFont;

int windowW, windowH;
bool canDrawText;
int soundVolume, musicVolume;

#endif // CSGRAPHICS_H_INCLUDED
