#ifndef MAPMAKER_H_INCLUDED
#define MAPMAKER_H_INCLUDED

#include "warper.h"

typedef struct _warperFilter
{
    SDL_Color filterColor;
} warperFilter;

void createNewMap(warperTilemap* tilemap, int tileSize);
warperFilter initWarperFilter(int r, int g, int b, int a);
void drawWarperFilter(void* subclass, cCamera camera);
void destroyWarperFilter(void* subclass);

#endif // MAPMAKER_H_INCLUDED
