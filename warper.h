#ifndef WARPER_H_INCLUDED
#define WARPER_H_INCLUDED

#include <stdlib.h>

#include "CoSprite/csGraphics.h"
#include "CoSprite/csInput.h"
#include "CoSprite/csUtility.h"

typedef struct _warperTilemap
{
    int** spritemap;
    int** collisionmap;
    int** eventmap;
    int width;  //width of the matrices' elements
    int height;  //height of the matrices' elements
    int tileSize;  //in camera-coord values
} warperTilemap;

void initWarperTilemap(warperTilemap* tilemap, int** spritemap, int** collisionmap, int** eventmap, int width, int height);
void importTilemap(warperTilemap* tilemap, char* importedData);
void exportTilemap(warperTilemap tilemap, char* exportedData);
void destroyWarperTilemap(warperTilemap* tilemap);

cLogger warperLogger;

#endif // WARPER_H_INCLUDED
