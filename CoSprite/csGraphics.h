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
    #define COSPRITE_VERSION_MINOR 12
    #define COSPRITE_VERSION_PATCH 0
    #define COSPRITE_VERSION "0.12.0"
#endif //COSPRITE_VERSION
#define SDL_MAIN_HANDLED 1

//#includes:
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
    #define radToDeg(x) (180.0 * (x) / PI)
    #define degToRad(x) ((x) * PI / 180.0)
#endif // PI
#ifndef MAX_PATH
#define MAX_PATH (260)
#endif  //MAX_PATH

//struct definitions:
typedef struct _cFont
{
    TTF_Font* font;
    int fontSize;
} cFont;

typedef struct _coSprite
{
    SDL_Window* window;
    SDL_Renderer* mainRenderer;
    cFont mainFont;
    int windowW;
    int windowH;
    SDL_Color colorKey;
    bool canDrawText;
    int soundVolume;
    int musicVolume;
    int renderLayers;  /**< default 5 */
} coSprite;

typedef struct _cDoubleRect
{
    double x;
    double y;
    double w;
    double h;
} cDoubleRect;

typedef struct _cDoublePt
{
    double x;
    double y;
} cDoublePt;

typedef struct _cDoubleVector
{
    double magnitude;
    double degrees;
} cDoubleVector;

typedef struct _cSprite
{
    SDL_Texture* texture;
    char textureFilepath[MAX_PATH];
    int id;
    cDoubleRect drawRect;
    cDoubleRect srcClipRect;
    cDoublePt center;
    double scale;
    SDL_RendererFlip flip;
    double degrees;
    int renderLayer;  /**< 0 - not drawn. 1-`renderLayers` - drawn. Lower number = drawn later */
    bool fixed;  /**< if true, won't be affected by camera movement */
    void* subclass;  /**< fill with any extraneous data or pointer to another struct */
} cSprite;

/*typedef struct _cCircle
{
    cDoublePt pt;
    double r;
    cDoublePt center;
    double scale;
    SDL_RendererFlip flip;
    double degrees;
    int renderLayer;  / **< 0 - not drawn. 1-5 - drawn. Lower number = drawn later * /
    bool fixed;  / **< if true, won't be affected by camera movement * /
} cCircle;*/

typedef struct _c2DModel
{  //essentially a 2D version of a wireframe model: A collection of sprites with relative coordinates
    cSprite* sprites;
    int numSprites;
    cDoubleRect rect;
    cDoublePt center;
    double scale;
    SDL_RendererFlip flip;
    double degrees;
    int renderLayer;
    bool fixed;  /**< if true, won't be affected by camera movement */
    void* subclass;
} c2DModel;

typedef struct _cText
{
    char* str;
    SDL_Texture* texture;
    cDoubleRect rect;
    int renderLayer; /**< 0 - not drawn. 1-`renderLayers` - drawn. Lower number = drawn later */
    SDL_Color textColor;
    SDL_Color bgColor;
    cFont* font;
    double scale;
    SDL_RendererFlip flip;
    double degrees;
    bool fixed;  /**< if true, won't be affected by camera movement */
} cText;

typedef struct _cCamera
{
    cDoubleRect rect;
    double zoom;
    double degrees;
} cCamera;

typedef struct _cResource
{
    void* subclass;
    void (*drawingRoutine)(void*, cCamera);
    void (*cleanupRoutine)(void*);
    int renderLayer; /**< 0 - not drawn. 1-`renderLayers` - drawn. Lower number = drawn later */
} cResource;

typedef struct _cScene
{
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

typedef struct _cLogger
{
    char* filepath;
    char* dateTimeFormat;  /**< strftime() compatible time format */
} cLogger;

//function prototypes:

//initialization
int initCoSprite();
void closeCoSprite();
bool loadIMG(char* imgPath, SDL_Texture** dest);
bool loadTTFont(char* filePath, TTF_Font** dest, int sizeInPts);
int* loadTextTexture(char* text, SDL_Texture** dest, int maxW, SDL_Color color, TTF_Font* font, bool isBlended);

//cSprite
void initCSprite(cSprite* sprite, SDL_Texture* texture, char* textureFilepath, int id, cDoubleRect drawRect, cDoubleRect srcClipRect, cDoublePt* center, double scale, SDL_RendererFlip flip, double degrees, bool fixed, void* subclass, int drawPriority);
void destroyCSprite(cSprite* sprite);
void drawCSprite(cSprite sprite, cCamera camera, bool update, bool fixedOverride);

//c2DModel
void initC2DModel(c2DModel* model, cSprite* sprites, int numSprites, cDoublePt position, cDoublePt* center, double scale, SDL_RendererFlip flip, double degrees, bool fixed, void* subclass, int drawPriority);
void destroyC2DModel(c2DModel* model);
void importC2DModel(c2DModel* model, char* filepath);
void exportC2DModel(c2DModel* model, char* filepath);
void sortCSpritesInModel(c2DModel* model);
void drawC2DModel(c2DModel model, cCamera camera, bool update);

//cText
void initCText(cText* text, char* str, cDoubleRect rect, SDL_Color textColor, SDL_Color bgColor, cFont* font, double scale, SDL_RendererFlip flip, double degrees, bool fixed, int drawPriority);
void updateCText(cText* text, char* str);
void destroyCText(cText* text);
void drawCText(cText text, cCamera camera, bool update);

//cResource
void initCResource(cResource* res, void* subclass, void (*drawingRoutine)(void*, cCamera), void (*cleanupRoutine)(void*), int renderLayer);
void drawCResource(cResource* res, cCamera camera);
void destroyCResource(cResource* res);

//cCamera
void initCCamera(cCamera* camera, cDoubleRect rect, double zoom, double degrees);
cDoublePt cWindowCoordToCameraCoord(cDoublePt pt, cCamera camera);
cDoublePt cCameraCoordToWindowCoord(cDoublePt pt, cCamera camera);
void destroyCCamera(cCamera* camera);


//cScene
void initCScene(cScene* scenePtr, SDL_Color bgColor, cCamera* camera, cSprite* sprites[], int spriteCount, c2DModel* models[], int modelCount, cResource* resources[], int resCount, cText* strings[], int stringCount);
int addSpriteToCScene(cScene* scenePtr, cSprite* sprite);
int removeSpriteFromCScene(cScene* scenePtr, cSprite* sprite, int index, bool free);
int add2DModelToCScene(cScene* scenePtr, c2DModel* model);
int remove2DModelFromCScene(cScene* scenePtr, c2DModel* model, int index, bool free);
int addTextToCScene(cScene* scenePtr, cText* text);
int removeTextFromCScene(cScene* scenePtr, cText* text, int index, bool free);
int addResourceToCScene(cScene* scenePtr, cResource* resource);
int removeResourceFromCScene(cScene* scenePtr, cResource* resource, int index, bool free);
void destroyCScene(cScene* scenePtr);
void drawCScene(cScene* scenePtr, bool clearScreen, bool redraw, int* fps, int fpsCap);
void cSceneViewer(cScene* scene);

//cFont
bool initCFont(cFont* font, char* fontFilepath, int fontSize);
void destroyCFont(cFont* font);

//misc
void cSceneViewer(cScene* scene);
void drawText(char* input, int x, int y, int maxW, int maxH, SDL_Color color, bool render);
cDoubleVector checkCSpriteCollision(cSprite sprite1, cSprite sprite2);
cDoubleVector checkC2DModelCollision(c2DModel model1, c2DModel model2, bool fast);
cDoublePt rotatePoint(cDoublePt pt, cDoublePt center, double degrees);

//file I/O
int createFile(char* filePath);
int checkFile(char* filePath);
int appendLine(char* filePath, char* stuff, bool addNewline);
int replaceLine(char* filePath, int lineNum, char* stuff, int maxLength, bool addNewline);
char* readLine(char* filePath, int lineNum, int maxLength, char** output);

//logging
void initCLogger(cLogger* logger, char* outFilepath, char* dateTimeFormat);
void cLogEvent(cLogger logger, char* entryType, char* brief, char* explanation);
void destroyCLogger(cLogger* logger);

//global variable declarations:
coSprite global;
Uint32 startTime;  /**< not set in-engine; if you want to collect a framerate from drawCScene(), set this right before you start your loop */

#endif // CSGRAPHICS_H_INCLUDED
