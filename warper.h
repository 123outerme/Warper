#ifndef WARPER_H_INCLUDED
#define WARPER_H_INCLUDED

#include <stdlib.h>

#include "CoSprite/csGraphics.h"
#include "CoSprite/csInput.h"
#include "CoSprite/csUtility.h"

#define TILE_SIZE 32
#define GRID_MAX_OPACITY 0x40
#define WARPER_OPTIONS_FILE "assets/options.cfg"

typedef struct _warperTilemap
{
    int** spritemap_layer1;
    int** spritemap_layer2;
    int** collisionmap;
    int width;  //width of the matrices' elements
    int height;  //height of the matrices' elements
    int tileSize;  //in camera-coord values
} warperTilemap;

typedef struct _warperOptions
{
    Uint8 gridOpacity;
    int difficulty;
    int musicVolume;
    int soundFxVolume;
} warperOptions;

//tilemap
void initWarperTilemap(warperTilemap* tilemap, int** spritemap, int** collisionmap, int width, int height);
void importWarperTilemap(warperTilemap* tilemap, char* filepath);
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
