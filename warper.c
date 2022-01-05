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
void initWarperAnimatedSprite(warperAnimatedSprite* animatedSpr, cSprite* spr, cDoubleRect* srcRectDiffs, double* rotationDiffs, double* scaleDiffs, SDL_RendererFlip* flipSettings, cDoublePt* centerDiffs, int* layerSettings, int diffsLength, int loops)
{
    animatedSpr->sprite = spr;
    animatedSpr->srcRectDiffs = calloc(diffsLength, sizeof(cDoubleRect));
    animatedSpr->rotationDiffs = calloc(diffsLength, sizeof(double));
    animatedSpr->scaleDiffs = calloc(diffsLength, sizeof(double));
    animatedSpr->flipSettings = calloc(diffsLength, sizeof(SDL_RendererFlip));
    animatedSpr->centerDiffs = calloc(diffsLength, sizeof(cDoublePt));
    animatedSpr->layerSettings = calloc(diffsLength, sizeof(int));

    if (!animatedSpr->srcRectDiffs || !animatedSpr->rotationDiffs || !animatedSpr->scaleDiffs || !animatedSpr->flipSettings || !animatedSpr->centerDiffs || !animatedSpr->layerSettings)
    {
        //printf("Warper error: cannot initialize animated sprite");
        cLogEvent(warperLogger, "WARNING", "WARPER: Animated Sprite", "Cannot initialize draw deltas");
        animatedSpr->numDiffs = 0;
        return;
    }
    for(int i = 0; i < diffsLength; i++)
    {
        if (srcRectDiffs)
            animatedSpr->srcRectDiffs[i] = srcRectDiffs[i];

        if (rotationDiffs)
            animatedSpr->rotationDiffs[i] = rotationDiffs[i];

        if (scaleDiffs)
            animatedSpr->scaleDiffs[i] = scaleDiffs[i];

        if (flipSettings)
            animatedSpr->flipSettings[i] = flipSettings[i];
        else
            animatedSpr->flipSettings[i] = SDL_FLIP_NONE;  //not sure if this is default after calloc()'ing, so I just do this in case

        if (centerDiffs)
            animatedSpr->centerDiffs[i] = centerDiffs[i];

        if (layerSettings)
            animatedSpr->layerSettings[i] = layerSettings[i];
        else
            animatedSpr->layerSettings[i] = 1;
    }

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

        animatedSpr->sprite->degrees += animatedSpr->rotationDiffs[animatedSpr->curFrame];
        animatedSpr->sprite->scale += animatedSpr->scaleDiffs[animatedSpr->curFrame];
        animatedSpr->sprite->flip = animatedSpr->flipSettings[animatedSpr->curFrame];

        animatedSpr->sprite->center.x += animatedSpr->centerDiffs[animatedSpr->curFrame].x;
        animatedSpr->sprite->center.y += animatedSpr->centerDiffs[animatedSpr->curFrame].y;

        animatedSpr->sprite->renderLayer = animatedSpr->layerSettings[animatedSpr->curFrame];

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
    free(animatedSpr->rotationDiffs);
    free(animatedSpr->scaleDiffs);
    free(animatedSpr->flipSettings);
    free(animatedSpr->centerDiffs);
    free(animatedSpr->layerSettings);
    animatedSpr->numDiffs = 0;
}

/** \brief Imports an animated sprite from string data
 *
 * \param aSpr warperAnimatedSprite* - expects aSpr->spr to be filled with an allocated cSprite*, if sprIndex != NULL
 * \param spriteIndex int* - if not NULL, will be filled in with the index that this sprite occurs at (only for if sprite data wasn't exported and the index was instead)
 * \param data char* - the data you want to import
 */
void importWarperAnimatedSprite(warperAnimatedSprite* aSpr, char* data, int* spriteIndex)
{
    char* savePtr = data;  //setup for strtok_r()
    char* nextToken = strtok_r(data, "{}", &savePtr);  //get the cSprite data

    if (nextToken[0] == '\"')
    {  //if our exported sprite data was actual data, and not a referenced index
        char* tempData = calloc(strlen(nextToken) + 3, sizeof(char));
        snprintf(tempData, strlen(nextToken) + 3, "{%s}", nextToken);  //put it back in the {} (which strtok_r() gets rid of)
        //printf("> %s\n", tempData);
        importCSprite(aSpr->sprite, tempData);  //import the sprite to the (already allocated) cSprite memory space
        free(tempData);  //free the data surrounded by {} since we have the sprite data imported now
        //printf("> %s\n", aSpr->sprite->textureFilepath);
    }
    else
    {  //if it was an index
        if (spriteIndex != NULL)
        {  //and the caller provided an address for spriteIndex
            *spriteIndex = strtol(nextToken, NULL, 10);  //write the sprite index
        }
        else
        {   //error occurred
            cLogEvent(warperLogger, "ERROR", "WARPER: Animated Sprite", "Sprite data stored as ref index not expected");
        }
    }

    aSpr->numDiffs = strtol(strtok_r(savePtr, ";", &savePtr), NULL, 10);  //get the number of different frames
    aSpr->loops = strtol(strtok_r(savePtr, ";", &savePtr), NULL, 10);  //get the number of loops we will do
    //printf("> %d, %d\n", aSpr->numDiffs, aSpr->loops);

    aSpr->srcRectDiffs = calloc(aSpr->numDiffs, sizeof(cDoubleRect));  //allocate each array according to how many frames there will be
    aSpr->rotationDiffs = calloc(aSpr->numDiffs, sizeof(double));
    aSpr->scaleDiffs = calloc(aSpr->numDiffs, sizeof(double));
    aSpr->flipSettings = calloc(aSpr->numDiffs, sizeof(SDL_RendererFlip));
    aSpr->centerDiffs = calloc(aSpr->numDiffs, sizeof(cDoublePt));
    aSpr->layerSettings = calloc(aSpr->numDiffs, sizeof(int));

    //src rect diffs
    nextToken = strtok_r(savePtr, "[]", &savePtr);  //get the first array (src rects)
    //strcpy(tempData, nextToken);
    char* saveArr = nextToken;  //start at the next gotten token (without updating nextToken's address)
    //printf("> %s\n", nextToken);
    for(int i = 0; i < aSpr->numDiffs; i++)
    {
        aSpr->srcRectDiffs[i].x = strtod(strtok_r(saveArr, "(,)", &saveArr), NULL);  //get the x, then y, then w, then h values
        aSpr->srcRectDiffs[i].y = strtod(strtok_r(saveArr, "(,)", &saveArr), NULL);
        aSpr->srcRectDiffs[i].w = strtod(strtok_r(saveArr, "(,)", &saveArr), NULL);
        aSpr->srcRectDiffs[i].h = strtod(strtok_r(saveArr, "(,)", &saveArr), NULL);
    }

    //rotation diffs
    nextToken = strtok_r(savePtr, "[;]", &savePtr);  //do the same for rotation
    saveArr = nextToken;
    //printf("> %s\n", nextToken);
    for(int i = 0; i < aSpr->numDiffs; i++)
        aSpr->rotationDiffs[i] = strtod(strtok_r(saveArr, ",", &saveArr), NULL);

    //scale diffs
    nextToken = strtok_r(savePtr, "[;]", &savePtr);  //and scale
    saveArr = nextToken;
    //printf("> %s\n", nextToken);
    for(int i = 0; i < aSpr->numDiffs; i++)
        aSpr->scaleDiffs[i] = strtod(strtok_r(saveArr, ",", &saveArr), NULL);

    //flip settings
    nextToken = strtok_r(savePtr, "[;]", &savePtr);  //and flip
    saveArr = nextToken;
    //printf("> %s\n", nextToken);
    for(int i = 0; i < aSpr->numDiffs; i++)
        aSpr->flipSettings[i] = (SDL_RendererFlip) strtol(strtok_r(saveArr, ",", &saveArr), NULL, 10);

    //center diffs
    nextToken = strtok_r(savePtr, "[;]", &savePtr);  //and center
    saveArr = nextToken;
    //printf("> %s\n", nextToken);
    for(int i = 0; i < aSpr->numDiffs; i++)
    {
        aSpr->centerDiffs[i].x = strtod(strtok_r(saveArr, "(,)", &saveArr), NULL);
        aSpr->centerDiffs[i].y = strtod(strtok_r(saveArr, "(,)", &saveArr), NULL);
    }

    //render layer settings
    nextToken = strtok_r(savePtr, "[;]", &savePtr);  //and layers
    saveArr = nextToken;
    //printf("> %s\n", nextToken);
    for(int i = 0; i < aSpr->numDiffs; i++)
        aSpr->layerSettings[i] = (int) strtol(strtok_r(saveArr, ",", &saveArr), NULL, 10);
}

/** \brief Export an animated sprite into string form
 *
 * \param animatedSpr warperAnimatedSprite - animated sprite to be converted
 * \param cSprIndex int - if > -1, will replace the cSprite data to be exported with the given value
 * \return char* - allocated string holding the animated sprite data.
 */
char* exportWarperAnimatedSprite(warperAnimatedSprite animatedSpr, int cSprIndex)
{
    //I've kinda just given up on calculating the expected size of this string
    int spriteSize = 0;
    const int rectsSize = ((8 * 4) + 3) * animatedSpr.numDiffs + 3;  //4 nums per rect, 7 characters plus one separator per num. 3 characters for parentheses and separator, and this times how ever many diffs we have, plus 2 characters for the containing brackets, plus 1 for good measure
    const int rotationsSize = 12 * animatedSpr.numDiffs + 3;  //11 digits + 1 separator character per diff, plus 2 characters for the containing brackets, plus 1 for good measure
    const int scalesSize = 12 * animatedSpr.numDiffs + 3;  //11 digits + 1 separator character per diff, plus 2 characters for the containing brackets, plus 1 for good measure
    const int flipsSize = 2 * animatedSpr.numDiffs + 3;  //1 digit + 1 separator per diff, plus 2 for containing brackets, plus 1 for good measure
    const int centersSize = ((2 * 12) + 3) * animatedSpr.numDiffs + 3; //2 nums per pt, 11 digits + 1 separator character per num, 2 characters for parentheses and 1 for separator, times num diffs, plus 2 for containing brackets, plus 1 for good measure
    const int layersSize = 4 * animatedSpr.numDiffs + 3;  //3 digits max per item + 1 separator character, times num diffs, plus 2 for parentheses and 1 for good measure


    char* spriteData;
    if (cSprIndex < 0)
    {
        spriteData = exportCSprite(*animatedSpr.sprite);
        spriteSize = strlen(spriteData) + 1;  //plus 1 for good measure
    }
    else
    {
        spriteSize = 2 + digits(cSprIndex) + 1;  //2 chars for brackets, n for digits, 1 for good measure
        spriteData = calloc(spriteSize, sizeof(char));
        snprintf(spriteData, spriteSize, "{%d}", cSprIndex);

    }

    const int dataSize = spriteSize + rectsSize + rotationsSize + scalesSize + flipsSize + centersSize + layersSize + 9;  //8 separators plus 1 for good measure
    char* data = calloc(dataSize, sizeof(char));

    char* srcData = calloc(rectsSize, sizeof(char));  //phoning it in at its finest
    char* rotationData = calloc(rotationsSize, sizeof(char));
    char* scaleData = calloc(scalesSize, sizeof(char));
    char* flipData = calloc(flipsSize, sizeof(char));
    char* centerData = calloc(centersSize, sizeof(char));
    char* layerData = calloc(layersSize, sizeof(char));
    char* temp = calloc(512, sizeof(char));

    strcat(srcData, "[");
    strcat(rotationData, "[");
    strcat(scaleData, "[");
    strcat(flipData, "[");
    strcat(centerData, "[");
    strcat(layerData, "[");

    for(int i = 0; i < animatedSpr.numDiffs; i++)
    {
        snprintf(temp, 512, "(%.2f,%.2f,%.2f,%.2f)", animatedSpr.srcRectDiffs[i].x, animatedSpr.srcRectDiffs[i].y, animatedSpr.srcRectDiffs[i].w, animatedSpr.srcRectDiffs[i].h);
        strcat(srcData, temp);

        snprintf(temp, 512, "%f", animatedSpr.rotationDiffs[i]);
        strcat(rotationData, temp);

        snprintf(temp, 512, "%f", animatedSpr.scaleDiffs[i]);
        strcat(scaleData, temp);

        snprintf(temp, 512, "%d", (int) animatedSpr.flipSettings[i]);
        strcat(flipData, temp);

        snprintf(temp, 512, "(%f,%f)", animatedSpr.centerDiffs[i].x, animatedSpr.centerDiffs[i].y);
        strcat(centerData, temp);

        snprintf(temp, 512, "%d", animatedSpr.layerSettings[i]);
        strcat(layerData, temp);

        if (i < animatedSpr.numDiffs - 1)
        {
            strcat(srcData, ",");
            strcat(rotationData, ",");
            strcat(scaleData, ",");
            strcat(flipData, ",");
            strcat(centerData, ",");
            strcat(layerData, ",");
        }
    }
    free(temp);

    strcat(srcData, "]");
    strcat(rotationData, "]");
    strcat(scaleData, "]");
    strcat(flipData, "]");
    strcat(centerData, "]");
    strcat(layerData, "]");

    snprintf(data, dataSize, "%s;%d;%d;%s;%s;%s;%s;%s;%s", spriteData, animatedSpr.numDiffs, animatedSpr.loops, srcData, rotationData, scaleData, flipData, centerData, layerData);

    free(spriteData);
    free(srcData);
    free(rotationData);
    free(scaleData);
    free(flipData);
    free(centerData);
    free(layerData);

    return data;
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
void importWarperTilemap(warperTilemap* tilemap, char* filepath, int line)
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

    readLine(filepath, line, 7, &importedMap);  //read the first 6 characters of the line for the width/height information
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

    readLine(filepath, line, importedLength + 1, &importedMap);

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
        options.framerate = WARPER_FRAME_LIMIT;
        options.difficulty = 1; //Beginner. Should it be something more like Medium?
        options.gridOpacity = 0x00;  //what should the default be here?
        options.musicVolume = MIX_MAX_VOLUME;
        options.soundFxVolume = MIX_MAX_VOLUME;
        saveWarperOptions();
    }
    else
    {
        char* optionsText = calloc(80, sizeof(char));

        readLine(WARPER_OPTIONS_FILE, 0, 80, &optionsText);  //framerate
        char* savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.framerate = strtol(savePtr, NULL, 10);

        readLine(WARPER_OPTIONS_FILE, 1, 80, &optionsText);  //difficulty
        savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.difficulty = strtol(savePtr, NULL, 10);

        readLine(WARPER_OPTIONS_FILE, 2, 80, &optionsText);  //grid opacity
        savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.gridOpacity = (Uint8) strtol(savePtr, NULL, 10) / 100.0 * GRID_MAX_OPACITY;

        readLine(WARPER_OPTIONS_FILE, 3, 80, &optionsText);  //music volume
        savePtr = optionsText;
        strtok_r(savePtr, ":", &savePtr);
        options.musicVolume = (int) strtol(savePtr, NULL, 10) / 100.0 * MIX_MAX_VOLUME;

        readLine(WARPER_OPTIONS_FILE, 4, 80, &optionsText);  //sfx volume
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

    snprintf(optionsText, 80, "framerate:%d", options.framerate);
    appendLine(WARPER_OPTIONS_FILE, optionsText, true);

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
