#include "warper.h"

void initWarperTilemap(warperTilemap* tilemap, int** spritemap, int** collisionmap, int** eventmap, int width, int height)
{
    tilemap->width = width;
    tilemap->height = height;

    tilemap->spritemap = calloc(width, sizeof(int*));
    tilemap->collisionmap = calloc(width, sizeof(int*));
    tilemap->eventmap = calloc(width, sizeof(int*));
    for(int x = 0; x < width; x++)
    {
        tilemap->spritemap[x] = calloc(height, sizeof(int));
        tilemap->collisionmap[x] = calloc(height, sizeof(int));
        tilemap->eventmap[x] = calloc(height, sizeof(int));
        for(int y = 0; y < height; y++)
        {
            tilemap->spritemap[x][y] = spritemap[x][y];
            tilemap->collisionmap[x][y] = collisionmap[x][y];
            tilemap->eventmap[x][y] = eventmap[x][y];
        }
    }
}

/** \brief Imports a tilemap from text (hex) data
 *
 * \param tilemap warperTilemap* expects tilemap->width and ->height to be filled in
 * \param importedData char* your map data
 */
void importTilemap(warperTilemap* tilemap, char* importedData)
{
    char* tileData = calloc(4, sizeof(char));

    tilemap->spritemap = calloc(tilemap->width, sizeof(int*));
    tilemap->collisionmap = calloc(tilemap->width, sizeof(int*));
    tilemap->eventmap = calloc(tilemap->width, sizeof(int*));

    int x = -1, y = tilemap->height + 1; //triggers if statement

    while(x <= tilemap->width)
    {
        if (y >= tilemap->height)
        {
            y = 0;
            x++;
            tilemap->spritemap[x] = calloc(tilemap->height, sizeof(int));
            tilemap->collisionmap[x] = calloc(tilemap->height, sizeof(int));
            tilemap->eventmap[x] = calloc(tilemap->height, sizeof(int));
        }

        strncpy(tileData, (importedData + 6 + (x * tilemap->height + y) * 3 * 3), 3);  //starts at importedData + 6 + (pos * 3 digits * 3 different maps)
        tilemap->spritemap[x][y] = strtol(tileData, NULL, 16);
        strncpy(tileData, (importedData + 9 + (x * tilemap->height + y) * 3 * 3), 3);
        tilemap->collisionmap[x][y] = strtol(tileData, NULL, 16);
        strncpy(tileData, (importedData + 12 + (x * tilemap->height + y) * 3 * 3), 3);
        tilemap->eventmap[x][y] = strtol(tileData, NULL, 16);
        y++;
    }
    free(tileData);
}

void exportTilemap(warperTilemap tilemap, char* exportedData)
{
    char* tileString = calloc(4, sizeof(char));

    snprintf(tileString, 4, "%.3X", tilemap.width);
    strcat(exportedData, tileString);
    snprintf(tileString, 4, "%.3X", tilemap.height);
    strcat(exportedData, tileString);

    for(int x = 0; x < tilemap.width; x++)
    {
        for(int y = 0; y < tilemap.height; y++)
        {
            snprintf(tileString, 4, "%.3X", tilemap.spritemap[x][y]);
            strcat(exportedData, tileString);
            snprintf(tileString, 4, "%.3X", tilemap.collisionmap[x][y]);
            strcat(exportedData, tileString);
            snprintf(tileString, 4, "%.3X", tilemap.eventmap[x][y]);
            strcat(exportedData, tileString);
        }
    }
    free(tileString);
}

void destroyWarperTilemap(warperTilemap* tilemap)
{
    for(int x = 0; x < tilemap->width; x++)
    {
        free(tilemap->spritemap[x]);
        free(tilemap->collisionmap[x]);
        free(tilemap->eventmap[x]);
    }
    free(tilemap->spritemap);
    free(tilemap->collisionmap);
    free(tilemap->eventmap);

    tilemap->width = 0;
    tilemap->height = 0;
}
