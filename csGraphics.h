#ifndef CSGRAPHICS_H_INCLUDED
#define CSGRAPHICS_H_INCLUDED

#include "csMain.h"

#ifndef bool
    #define bool char
    #define false 0
    #define true 1
#endif // bool

//struct definitions:
typedef struct _cSprite {
    SDL_Texture* texture;
    int id;
    SDL_Rect drawRect;
    SDL_Rect srcClipRect;
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
    double scale;
    SDL_RendererFlip flip;
    double degrees;
    int drawPriority;
    bool fixed;  /**< if true, won't be affected by camera movement */
    void* subclass;
} c2DModel;

typedef struct _cText {
    char* string;
    SDL_Rect rect;
    int drawPriority; /**< 0 - not drawn. 1-5 - drawn. Lower number = drawn later */
    SDL_Color textColor;
    SDL_Color bgColor;
    bool fixed;  /**< if true, won't be affected by camera movement */
} cText;

typedef struct _cCamera {
    SDL_Rect rect;
    double zoom;
    double degrees;
} cCamera;

typedef struct _cResource {
    char* filepath;
    SDL_Texture* texture;
    SDL_Rect rect;
    void* subclass;
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
void initCSprite(cSprite* sprite, SDL_Texture* texture, int id, SDL_Rect drawRect, SDL_Rect srcClipRect, double scale, SDL_RendererFlip flip, double degrees, bool fixed, void* subclass, int drawPriority);
void destroyCSprite(cSprite* sprite);
void drawCSprite(cSprite sprite, cCamera camera, bool update);
void initC2DModel(c2DModel* model, cSprite* sprites, int numSprites, int x, int y, int w, int h, double scale, SDL_RendererFlip flip, double degrees, bool fixed, void* subclass, int drawPriority);
void destroyC2DModel(c2DModel* model);
void drawC2DModel(c2DModel model, cCamera camera, bool update);
void initCText(cText* text, char* string, SDL_Rect rect, SDL_Color textColor, SDL_Color bgColor, bool fixed, int drawPriority);
void destroyCText(cText* text);
void drawCText(cText text, cCamera camera, bool update);
void initCResource(cResource* res, char* filepath);
void destroyCResource(cResource* res);
void initCCamera(cCamera* camera, SDL_Rect rect, double zoom, double degrees);
void destroyCCamera(cCamera* camera);
void initCScene(cScene* scenePtr, SDL_Color bgColor, cCamera* camera, cSprite* sprites[], int spriteCount, c2DModel* models[], int modelCount, cResource* resources[], int resCount, cText* strings[], int stringCount);
void destroyCScene(cScene* scenePtr);
void drawCScene(cScene* scenePtr, bool redraw);
void drawText(char* input, int x, int y, int maxW, int maxH, SDL_Color color, bool render);

#endif // CSGRAPHICS_H_INCLUDED
