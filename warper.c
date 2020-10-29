#include "warper.h"

void initWarperTilemap(warperTilemap* tilemap, int** spritemap, int** collisionmap, int width, int height)
{
    tilemap->width = width;
    tilemap->height = height;

    tilemap->spritemap_layer1 = calloc(width, sizeof(int*));
    tilemap->collisionmap = calloc(width, sizeof(int*));
    for(int x = 0; x < width; x++)
    {
        tilemap->spritemap_layer1[x] = calloc(height, sizeof(int));
        tilemap->collisionmap[x] = calloc(height, sizeof(int));
        for(int y = 0; y < height; y++)
        {
            tilemap->spritemap_layer1[x][y] = spritemap[x][y];
            tilemap->collisionmap[x][y] = collisionmap[x][y];
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

    tilemap->spritemap_layer1 = calloc(tilemap->width, sizeof(int*));
    tilemap->spritemap_layer2 = calloc(tilemap->width, sizeof(int*));
    tilemap->collisionmap = calloc(tilemap->width, sizeof(int*));

    int x = -1, y = tilemap->height + 1; //triggers if statement

    while(x <= tilemap->width)
    {
        if (y >= tilemap->height)
        {
            y = 0;
            x++;
            if (x < tilemap->width)
            {
                tilemap->spritemap_layer1[x] = calloc(tilemap->height, sizeof(int));
                tilemap->spritemap_layer2[x] = calloc(tilemap->height, sizeof(int));
                tilemap->collisionmap[x] = calloc(tilemap->height, sizeof(int));
            }
            else
                break;
        }

        strncpy(tileData, (importedData + 6 + (x * tilemap->height + y) * 3 * 3), 3);  //starts at importedData + 6 + (pos * 3 digits * 3 different maps)
        tilemap->spritemap_layer1[x][y] = strtol(tileData, NULL, 16);
        strncpy(tileData, (importedData + 9 + (x * tilemap->height + y) * 3 * 3), 3);
        tilemap->spritemap_layer2[x][y] = strtol(tileData, NULL, 16);
        strncpy(tileData, (importedData + 12 + (x * tilemap->height + y) * 3 * 3), 3);
        tilemap->collisionmap[x][y] = strtol(tileData, NULL, 16);
        y++;
    }
    free(tileData);
}

void loadTilemapModels(warperTilemap tilemap, c2DModel* layer1, c2DModel* layer2)
{
        SDL_Texture* tilesetTexture;
        loadIMG("assets/worldTilesheet.png", &tilesetTexture);
        cSprite* tileSprites_layer1 = calloc(tilemap.width * tilemap.height, sizeof(cSprite)), * tileSprites_layer2 = calloc(tilemap.width * tilemap.height, sizeof(cSprite));
        for(int x = 0; x < tilemap.width; x++)
        {
            for(int y = 0; y < tilemap.height; y++)
            {
                initCSprite(&tileSprites_layer1[x * tilemap.height + y], tilesetTexture, "assets/worldTilesheet.png", tilemap.spritemap_layer1[x][y],
                            (cDoubleRect) {tilemap.tileSize * x, tilemap.tileSize * y, tilemap.tileSize, tilemap.tileSize},
                            (cDoubleRect) {(tilemap.spritemap_layer1[x][y] / 20) * tilemap.tileSize / 2, (tilemap.spritemap_layer1[x][y] % 20) * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

                initCSprite(&tileSprites_layer2[x * tilemap.height + y], tilesetTexture, "assets/worldTilesheet.png", tilemap.spritemap_layer2[x][y],
                            (cDoubleRect) {tilemap.tileSize * x, tilemap.tileSize * y, tilemap.tileSize, tilemap.tileSize},
                            (cDoubleRect) {(tilemap.spritemap_layer2[x][y] / 20) * tilemap.tileSize / 2, (tilemap.spritemap_layer2[x][y] % 20) * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
            }
        }
        initC2DModel(layer1, tileSprites_layer1, tilemap.width * tilemap.height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initC2DModel(layer2, tileSprites_layer2, tilemap.width * tilemap.height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);

        free(tileSprites_layer1);
        free(tileSprites_layer2);
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
            snprintf(tileString, 4, "%.3X", tilemap.spritemap_layer1[x][y]);
            strcat(exportedData, tileString);
            snprintf(tileString, 4, "%.3X", tilemap.spritemap_layer2[x][y]);
            strcat(exportedData, tileString);
            snprintf(tileString, 4, "%.3X", tilemap.collisionmap[x][y]);
            strcat(exportedData, tileString);
        }
    }
    free(tileString);
}

void destroyWarperTilemap(warperTilemap* tilemap)
{
    for(int x = 0; x < tilemap->width; x++)
    {
        free(tilemap->spritemap_layer1[x]);
        free(tilemap->spritemap_layer2[x]);
        free(tilemap->collisionmap[x]);
    }
    free(tilemap->spritemap_layer1);
    free(tilemap->spritemap_layer2);
    free(tilemap->collisionmap);

    tilemap->width = 0;
    tilemap->height = 0;
}
