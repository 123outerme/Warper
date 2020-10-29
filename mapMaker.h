#ifndef MAPMAKER_H_INCLUDED
#define MAPMAKER_H_INCLUDED

#include "warper.h"

typedef struct _warperFilter
{
    SDL_Color filterColor;
} warperFilter;

typedef struct _warperMultiProperties
{
    SDL_Rect tileRect;
    bool resizesW;
    int colToRepeat;
    bool resizesH;
    int rowToRepeat;
} warperMultiProperties;

bool createNewMap(warperTilemap* tilemap, int tileSize);
warperFilter initWarperFilter(int r, int g, int b, int a);
void drawWarperFilter(void* subclass, cCamera camera);
void destroyWarperFilter(void* subclass);

#define WARPER_MULTI_PROPS { (warperMultiProperties) {.tileRect = (SDL_Rect) {2, 0, 4, 5}, .resizesW = false, .colToRepeat = -1, .resizesH = true, .rowToRepeat = 2}, \
                             (warperMultiProperties) {.tileRect = (SDL_Rect) {2, 5, 6, 6}, .resizesW = true, .colToRepeat = 2, .resizesH = true, .rowToRepeat = 2} };

#endif // MAPMAKER_H_INCLUDED
