#ifndef WARPER_H_INCLUDED
#define WARPER_H_INCLUDED

#include <stdlib.h>

#include "CoSprite/csGraphics.h"
#include "CoSprite/csInput.h"
#include "CoSprite/csUtility.h"

#define WARPER_FRAME_LIMIT 60
#define TILE_SIZE 32
#define GRID_MAX_OPACITY 0x40
#define WARPER_OPTIONS_FILE "assets/options.cfg"

#define SCREEN_PX_WIDTH 40 * TILE_SIZE
#define SCREEN_PX_HEIGHT 20 * TILE_SIZE

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) > (b)) ? (b) : (a))

typedef struct _warperAnimatedSprite
{
    cSprite* sprite;
    cDoubleRect* srcRectDiffs; /**< An array of deltas from the base srcClipRect */
    double* rotationDiffs;  /**< Array of deltas from the base rotation value (degrees) */
    double* scaleDiffs;  /**< Array of diffs from the base scale value */
    SDL_RendererFlip* flipSettings;  /**< Array of settings (not diffs) to set the flip to */
    cDoublePt* centerDiffs;  /**< Array of diffs from the base center */
    int* layerSettings;  /**< Array of settings (not diffs) to set the render layer to */
    int numDiffs;
    int curFrame;
    int loops;  /**< if != 0, then the animation will loop (the last diff should offset back to the starting value). Negative value loops forever */
} warperAnimatedSprite;

typedef struct _warperTilemap
{
    int** spritemap_layer1;
    int** spritemap_layer2;
    int** collisionmap;
    int width;  /**< width of the matrices' elements */
    int height;  /**< height of the matrices' elements */
    int tileSize;  /**< in camera-coord values */
} warperTilemap;

typedef struct _warperOptions
{
    int framerate;
    Uint8 gridOpacity;
    int difficulty;
    int musicVolume;
    int soundFxVolume;
} warperOptions;

//animated sprite
void initWarperAnimatedSprite(warperAnimatedSprite* animatedSpr, cSprite* spr, cDoubleRect* srcRectDiffs, double* rotationDiffs, double* scaleDiffs, SDL_RendererFlip* flipSettings, cDoublePt* centerDiffs, int* layerSettings, int diffsLength, int loops);
//>CoSprite helper functions (for cSprite)
void iterateWarperAnimatedSprite(warperAnimatedSprite* animatedSpr);
//>CoSprite helper functions (for cResource)
void drawWarperAnimatedSpr(void* spr, cCamera camera);
void cleanupWarperAnimatedSpr(void* spr);
//>end helper functions
void destroyWarperAnimatedSprite(warperAnimatedSprite* animatedSpr, bool destroySprite);
void importWarperAnimatedSprite(warperAnimatedSprite* aSpr, char* data, int* spriteIndex);
char* exportWarperAnimatedSprite(warperAnimatedSprite animatedSpr, int cSprIndex);

//tilemap
//void initWarperTilemap(warperTilemap* tilemap, int** spritemap, int** collisionmap, int width, int height);  //not really used anymore
void importWarperTilemap(warperTilemap* tilemap, char* filepath, int line);
void loadTilemap(warperTilemap* tilemap, char* importedData);
void exportTilemap(warperTilemap tilemap, char* exportedData);
void loadTilemapModels(warperTilemap tilemap, c2DModel* layer1, c2DModel* layer2);
void destroyWarperTilemap(warperTilemap* tilemap);

//options
void loadWarperOptions();
void saveWarperOptions();

extern warperOptions options;
extern cLogger warperLogger;

#endif // WARPER_H_INCLUDED
