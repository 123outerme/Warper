#include "warper.h"

cLogger warperLogger;
warperOptions options;

/** \brief Initializes an animated sprite based on a cSprite.
 *
 * \param animatedSpr warperAnimatedSprite* - animated sprite struct pointer you want filled in
 * \param spr cSprite* - pointer to the base sprite information
 * \param srcRectDiffs cDoubleRect* - array of `cDoubleRect`s that provide coordinate differences between positions of source image frames
 * \param diffsLength int - length of `srcRectDiffs`
 * \param loops int - if != 0, the animation will continually play (MUST provide one extra diff that resets coordinates back to beginning). Negative loops forever
 */
void initWarperAnimatedSprite(warperAnimatedSprite* animatedSpr, cSprite* spr, cDoubleRect* srcRectDiffs, int diffsLength, int loops)
{
    animatedSpr->sprite = spr;
    animatedSpr->srcRectDiffs = calloc(diffsLength, sizeof(cDoubleRect));

    if (!animatedSpr->srcRectDiffs)
    {
        //printf("Warper error: cannot initialize animated sprite");
        cLogEvent(warperLogger, "WARNING", "WARPER: Animated Sprite", "Cannot initialize draw deltas");
        animatedSpr->numDiffs = 0;
        return;
    }
    for(int i = 0; i < diffsLength; i++)
        animatedSpr->srcRectDiffs[i] = srcRectDiffs[i];

    animatedSpr->numDiffs = diffsLength;
    animatedSpr->loops = loops;
    animatedSpr->curFrame = 0;

}

void iterateWarperAnimatedSprite(warperAnimatedSprite* animatedSpr)
{
    if (animatedSpr->curFrame < animatedSpr->numDiffs || animatedSpr->loops != 0)
    {
        animatedSpr->sprite->srcClipRect.x += animatedSpr->srcRectDiffs[animatedSpr->curFrame].x;  //go to next source frame
        animatedSpr->sprite->srcClipRect.y += animatedSpr->srcRectDiffs[animatedSpr->curFrame].y;
        animatedSpr->sprite->srcClipRect.w += animatedSpr->srcRectDiffs[animatedSpr->curFrame].w;
        animatedSpr->sprite->srcClipRect.h += animatedSpr->srcRectDiffs[animatedSpr->curFrame].h;

        animatedSpr->curFrame++;

        if (animatedSpr->curFrame >= animatedSpr->numDiffs && animatedSpr->loops != 0)
        {
            animatedSpr->curFrame = 0;
            if (animatedSpr->loops > 0)
                animatedSpr->loops--;
        }
    }
}

void drawWarperAnimatedSpr(void* spr, cCamera camera)
{
    warperAnimatedSprite* animatedSpr = (warperAnimatedSprite*) spr;

    drawCSprite(*animatedSpr->sprite, camera, false, false);
    iterateWarperAnimatedSprite(animatedSpr);
}

void cleanupWarperAnimatedSpr(void* spr)
{
    warperAnimatedSprite* animatedSpr = (warperAnimatedSprite*) spr;

    destroyWarperAnimatedSprite(animatedSpr, true);
}

void destroyWarperAnimatedSprite(warperAnimatedSprite* animatedSpr, bool destroySprite)
{
    if (destroySprite)
        destroyCSprite(animatedSpr->sprite);

    animatedSpr->sprite = NULL;
    free(animatedSpr->srcRectDiffs);
    animatedSpr->numDiffs = 0;
}

/* Not really used anymore
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
//*/

/** \brief Imports a tilemap from a file
 *
 * \param tilemap warperTilemap* - tilemap pointer to be filled in
 * \param filepath char* - filepath to map data
 */
void importWarperTilemap(warperTilemap* tilemap, char* filepath)
{
    char* importedMap = calloc(7, sizeof(char));

    if (!importedMap)
    {
        cLogEvent(warperLogger, "ERROR", "WARPER: Tilemap", "Cannot initialize tilemap's size buffer");
        tilemap->height = -1;
        tilemap->width = -1;
        tilemap->spritemap_layer1 = NULL;
        tilemap->spritemap_layer2 = NULL;
        tilemap->collisionmap = NULL;
        return;
    }

    readLine(filepath, 0, 7, &importedMap);
    //printf("%s\n", importedMap);

    char dimData[4] = "\0";

    strncpy(dimData, importedMap, 3); //the first three characters of the map file indicate its width
    tilemap->width = strtol(dimData, NULL, 16);
    strncpy(dimData, importedMap + 3, 3);  //the next three characters indicate the map's height
    tilemap->height = strtol(dimData, NULL, 16);

    free(importedMap);

    int importedLength = 3 * 3 * tilemap->width * tilemap->height + 3 + 3;
    //this string is long enough to hold data that fills 3 arrays with tiles, using 3-digit tile codes, over the whole width and height of the map
    //plus the width and height 'bytes' which is how we know how big this map is in the first place
    //(3 arrays * 3 digits * width * height + width 'bytes' + height 'bytes')

    importedMap = calloc(importedLength + 1, sizeof(char));

    if (!importedMap)
    {
        cLogEvent(warperLogger, "ERROR", "WARPER: Tilemap", "Cannot initialize tilemap data buffer");
        tilemap->height = -1;
        tilemap->width = -1;
        tilemap->spritemap_layer1 = NULL;
        tilemap->spritemap_layer2 = NULL;
        tilemap->collisionmap = NULL;
        return;
    }

    readLine(filepath, 0, importedLength + 1, &importedMap);

    loadTilemap(tilemap, importedMap);

    free(importedMap);

    tilemap->tileSize = TILE_SIZE;
}

/** \brief Imports a tilemap from text (hex) data
 *
 * \param tilemap warperTilemap* - expects tilemap->width and ->height to be filled in
 * \param importedData char* - your map data
 */
void loadTilemap(warperTilemap* tilemap, char* importedData)
{
    char* tileData = calloc(4, sizeof(char));

    tilemap->spritemap_layer1 = calloc(tilemap->width, sizeof(int*));
    tilemap->spritemap_layer2 = calloc(tilemap->width, sizeof(int*));
    tilemap->collisionmap = calloc(tilemap->width, sizeof(int*));

    if (!tilemap->spritemap_layer1 || !tilemap->spritemap_layer2 || !tilemap->collisionmap)
    {
        cLogEvent(warperLogger, "ERROR", "WARPER: Tilemap", "Cannot initialize tilemap column memory");
        tilemap->height = -1;
        tilemap->width = -1;
        tilemap->spritemap_layer1 = NULL;
        tilemap->spritemap_layer2 = NULL;
        tilemap->collisionmap = NULL;
        return;
    }

    int x = -1, y = tilemap->height + 1; //triggers if statement upon first execution to start the wraparound code (allocating new memory for each horizontal line of tiles)

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

                if (!tilemap->spritemap_layer1[x] || !tilemap->spritemap_layer2[x] || !tilemap->collisionmap[x])
                {
                    cLogEvent(warperLogger, "ERROR", "WARPER: Tilemap", "Cannot initialize tilemap row memory");
                    tilemap->height = -1;
                    tilemap->width = -1;
                    tilemap->spritemap_layer1 = NULL;
                    tilemap->spritemap_layer2 = NULL;
                    tilemap->collisionmap = NULL;
                    return;
                }
            }
            else
                break;  //we have loaded all of the data
        }

        //start importing tilemap data AFTER the width/height 'bytes' (6 characters in total), then iterate through for every x/y position
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

/** \brief turns tilemap data into c2DModels to draw
 *
 * \param tilemap warperTilemap - the tilemap data to draw
 * \param layer1 c2DModel* - the lower layer to draw
 * \param layer2 c2DModel* - the higher layer to draw
 */
void loadTilemapModels(warperTilemap tilemap, c2DModel* layer1, c2DModel* layer2)
{
        SDL_Texture* tilesetTexture;
        loadIMG("assets/worldTilesheet.png", &tilesetTexture);
        cSprite* tileSprites_layer1 = calloc(tilemap.width * tilemap.height, sizeof(cSprite)), * tileSprites_layer2 = calloc(tilemap.width * tilemap.height, sizeof(cSprite));

        if (!tileSprites_layer1 || !tileSprites_layer2)
        {
            cLogEvent(warperLogger, "ERROR", "WARPER: Tilemap", "Cannot initialize tilemap sprites");
            layer1->numSprites = 0;
            layer2->numSprites = 0;
            return;
        }

        for(int x = 0; x < tilemap.width; x++)
        {
            for(int y = 0; y < tilemap.height; y++)
            {
                initCSprite(&tileSprites_layer1[x * tilemap.height + y], tilesetTexture, "assets/worldTilesheet.png", tilemap.spritemap_layer1[x][y],
                            (cDoubleRect) {tilemap.tileSize * x, tilemap.tileSize * y, tilemap.tileSize, tilemap.tileSize},
                            (cDoubleRect) {(tilemap.spritemap_layer1[x][y] / 20) * tilemap.tileSize / 2, (tilemap.spritemap_layer1[x][y] % 20) * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, false, NULL, 5);

                initCSprite(&tileSprites_layer2[x * tilemap.height + y], tilesetTexture, "assets/worldTilesheet.png", tilemap.spritemap_layer2[x][y],
                            (cDoubleRect) {tilemap.tileSize * x, tilemap.tileSize * y, tilemap.tileSize, tilemap.tileSize},
                            (cDoubleRect) {(tilemap.spritemap_layer2[x][y] / 20) * tilemap.tileSize / 2, (tilemap.spritemap_layer2[x][y] % 20) * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, false, NULL, 5);
            }
        }
        initC2DModel(layer1, tileSprites_layer1, tilemap.width * tilemap.height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initC2DModel(layer2, tileSprites_layer2, tilemap.width * tilemap.height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);

        free(tileSprites_layer1);
        free(tileSprites_layer2);
}

/** \brief Prints the exported map data into an allocated exportedData
 *
 * \param tilemap warperTilemap - tilemap data to draw
 * \param exportedData char* - an alloc'd string where the data will go
 */
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

/** \brief Loads the Warper options into the global struct. If the options file exist, creates it first
 */
void loadWarperOptions()
{
    if (checkFile(WARPER_OPTIONS_FILE) <= 0)
    {
        //default options
        options.difficulty = 1; //Beginner. Should it be something more like Medium?
        options.gridOpacity = 0x00;  //what should the default be here?
        options.musicVolume = MIX_MAX_VOLUME;
        options.soundFxVolume = MIX_MAX_VOLUME;
        saveWarperOptions();
    }
    else
    {
        char* optionsText = calloc(80, sizeof(char));

        readLine(WARPER_OPTIONS_FILE, 0, 80, &optionsText);  //difficulty
        char* savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.difficulty = strtol(savePtr, NULL, 10);

        readLine(WARPER_OPTIONS_FILE, 1, 80, &optionsText);  //grid opacity
        savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.gridOpacity = (Uint8) strtol(savePtr, NULL, 10) / 100.0 * GRID_MAX_OPACITY;

        readLine(WARPER_OPTIONS_FILE, 2, 80, &optionsText);  //music volume
        savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.musicVolume = (int) strtol(savePtr, NULL, 10) / 100.0 * MIX_MAX_VOLUME;

        readLine(WARPER_OPTIONS_FILE, 3, 80, &optionsText);  //sfx volume
        savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.soundFxVolume = (int) strtol(savePtr, NULL, 10) / 100.0 * MIX_MAX_VOLUME;

        //TODO - complete

        free(optionsText);
    }
}

/** \brief Writes the Warper options that are in the global struct to the file
 */
void saveWarperOptions()
{
    createFile(WARPER_OPTIONS_FILE);  //erase existing file
    char* optionsText = calloc(80, sizeof(char));

    snprintf(optionsText, 80, "difficulty:%d", options.difficulty);
    appendLine(WARPER_OPTIONS_FILE, optionsText, true);

    snprintf(optionsText, 80, "gridOpacity:%d", (int) (options.gridOpacity / ((double) GRID_MAX_OPACITY) * 100));
    appendLine(WARPER_OPTIONS_FILE, optionsText, true);

    snprintf(optionsText, 80, "musicVolume:%d", (int) (options.musicVolume / ((double) MIX_MAX_VOLUME) * 100));
    appendLine(WARPER_OPTIONS_FILE, optionsText, true);

    snprintf(optionsText, 80, "soundFxVolume:%d", (int) (options.soundFxVolume / ((double) MIX_MAX_VOLUME) * 100));
    appendLine(WARPER_OPTIONS_FILE, optionsText, true);

    //TODO - complete

    free(optionsText);
}
