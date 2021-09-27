#include "mapMaker.h"

bool createNewMap(warperTilemap* tilemap, int tileSize)
{
    bool quit = false;
    bool fullQuit = false;
    SDL_Keycode key;

    bool loadMap = false;
    cScene inputScene;
    cCamera inputCamera;
    initCCamera(&inputCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0);
    {
        warperTextBox loadChoiceBox;
        cResource loadChoiceBoxRes;
        createMenuTextBox(&loadChoiceBox, (cDoubleRect) {tileSize, tileSize, global.windowW - 2 * tileSize, global.windowH - 2 * tileSize}, (cDoublePt) {412, 8}, 4, true, 0xFF, (char*[4]) {"Load Map?", " ", "Yes", "No"}, (bool[4]) {false, false, true, true}, 4, &global.mainFont);
        initCResource(&loadChoiceBoxRes, (void*) &loadChoiceBox, drawWarperTextBox, destroyWarperTextBox, 5);
        initCScene(&inputScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &inputCamera, NULL, 0, NULL, 0, (cResource*[1]) {&loadChoiceBoxRes}, 1, NULL, 0);

        loadChoiceBox.selection = -1;

        cInputState input;
        while(loadChoiceBox.selection < 0)
        {
            input = cGetInputState(true);

            if (input.quitInput)
                return true; //we are quitting

            if (input.isClick)
            {
                if (loadChoiceBoxRes.renderLayer != 0)
                    checkWarperTextBoxClick(&loadChoiceBox, input.click.x, input.click.y);
            }
            drawCScene(&inputScene, true, true, NULL, NULL, 60);
        }
        loadMap = (loadChoiceBox.selection == 2);
    }

    destroyCScene(&inputScene);
    quit = false;

    if (loadMap)
    {
        importWarperTilemap(tilemap, "maps/testMap.txt");
    }
    else
    {
        char* dimensionInput = calloc(5, sizeof(char));

        cText promptText;
        cText inputText;
        initCCamera(&inputCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0);
        initCText(&promptText, "Input width:", (cDoubleRect) {0, global.windowH / 4, 12 * global.mainFont.fontSize, global.mainFont.fontSize}, 13 * global.mainFont.fontSize, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        initCText(&inputText, " ", (cDoubleRect) {0, global.windowH / 2, 4 * global.mainFont.fontSize, global.mainFont.fontSize}, 5 * global.mainFont.fontSize, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        initCScene(&inputScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &inputCamera, NULL, 0, NULL, 0, NULL, 0, (cText*[2]) {&promptText, &inputText}, 2);
        drawCScene(&inputScene, true, true, NULL, NULL, 0);
        while(!quit)
        {
            //cInputState keyboardState = cGetInputState(false);
            key = getKey(false);

            /*
            if (keyboardState.quitInput || keyboardState.keyStates[SDL_SCANCODE_ESCAPE] || keyboardState.keyStates[SDL_SCANCODE_RETURN])
                quit = true;
            //*/

            //*
            if (key == -1 || key == SDLK_ESCAPE || key == SDLK_RETURN)
                quit = true;
            //*/

            //handleTextInput(widthInput, keyboardState, 3);
            handleTextKeycodeInput(dimensionInput, key, 4);
            if ((key >= SDLK_0 && key <= SDLK_z) || key == SDLK_SPACE || key == SDLK_BACKSPACE)
                updateCText(&inputText, dimensionInput);

            drawCScene(&inputScene, true, true, NULL, NULL, 60);
        }
        if (key == -1)
        {
            free(dimensionInput);
            destroyCScene(&inputScene);
            return true;
        }

        key = SDLK_UNKNOWN;

        tilemap->width = strtol(dimensionInput, NULL, 10);
        //printf("%d\n", tilemap->width);
        quit = false;
        free(dimensionInput);

        dimensionInput = calloc(5, sizeof(char));

        updateCText(&inputText, "   ");
        updateCText(&promptText, "Input height:");
        drawCScene(&inputScene, true, true, NULL, NULL, 60);
        while(!quit)
        {
            key = getKey(false);
            if (key == -1 || key == SDLK_ESCAPE || key == SDLK_RETURN)
                quit = true;

            handleTextKeycodeInput(dimensionInput, key, 4);

            if ((key >= SDLK_0 && key <= SDLK_z) || key == SDLK_SPACE || key == SDLK_BACKSPACE)
                updateCText(&inputText, dimensionInput);

            drawCScene(&inputScene, true, true, NULL, NULL, 60);
        }

        tilemap->height = strtol(dimensionInput, NULL, 10);
        free(dimensionInput);
        destroyCScene(&inputScene);

        if (key == -1)
            return true;

        tilemap->tileSize = tileSize;

        tilemap->spritemap_layer1 = calloc(tilemap->width, sizeof(int*));
        tilemap->spritemap_layer2 = calloc(tilemap->width, sizeof(int*));
        tilemap->collisionmap = calloc(tilemap->width, sizeof(int*));
        tilemap->collisionmap = calloc(tilemap->width, sizeof(int*));
        for(int x = 0; x < tilemap->width; x++)
        {
            tilemap->spritemap_layer1[x] = calloc(tilemap->height, sizeof(int));
            tilemap->spritemap_layer2[x] = calloc(tilemap->height, sizeof(int));
            tilemap->collisionmap[x] = calloc(tilemap->height, sizeof(int));
            for(int y = 0; y < tilemap->height; y++)
            {
                tilemap->spritemap_layer1[x][y] = 4;
                tilemap->spritemap_layer2[x][y] = 798;  //should be invisible tile
                tilemap->collisionmap[x][y] = 0;
            }
        }

        quit = false;
    }
    bool spriteMode = true, drawLayer1 = true, drawMulti = false, pickMode = false;

    c2DModel mapModel_layer1, mapModel_layer2;
    c2DModel collisionModel;
    cSprite leftTileSprite, rightTileSprite;
    cSprite tilesetSprite;

    loadTilemapModels(*tilemap, &mapModel_layer1, &mapModel_layer2);
    mapModel_layer2.renderLayer = 4;  //we need to compress this for proper visual stuff

    {
        SDL_Texture* tilesetTexture;
        loadIMG("./assets/worldTilesheet.png", &tilesetTexture);
        cSprite* collisionSprites = calloc(tilemap->width * tilemap->height, sizeof(cSprite));
        for(int x = 0; x < tilemap->width; x++)
        {
            for(int y = 0; y < tilemap->height; y++)
            {
                initCSprite(&collisionSprites[x * tilemap->height + y], tilesetTexture, "./assets/worldTilesheet.png", tilemap->collisionmap[x][y],
                            (cDoubleRect) {tilemap->tileSize * x, tilemap->tileSize * y, tilemap->tileSize, tilemap->tileSize},
                            (cDoubleRect) {39 * tilemap->tileSize / 2, (tilemap->collisionmap[x][y] + 18) * tilemap->tileSize / 2, tilemap->tileSize / 2, tilemap->tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);
            }
        }
        initC2DModel(&collisionModel, collisionSprites, tilemap->width * tilemap->height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initCSprite(&leftTileSprite, tilesetTexture, "./assets/worldTilesheet.png", 0,
                    (cDoubleRect) {tilemap->tileSize, tilemap->tileSize, tilemap->tileSize, tilemap->tileSize},
                    (cDoubleRect) {0, 0, tilemap->tileSize / 2, tilemap->tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 2);

        initCSprite(&rightTileSprite, tilesetTexture, "./assets/worldTilesheet.png", 0,
                    (cDoubleRect) {tilemap->tileSize, tilemap->tileSize, tilemap->tileSize, tilemap->tileSize},
                    (cDoubleRect) {0, 0, tilemap->tileSize / 2, tilemap->tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 0);

        initCSprite(&tilesetSprite, tilesetTexture, "./assets/worldTileset.png", 0,
                    (cDoubleRect) {0, 0, 640, 320},
                    (cDoubleRect) {0, 0, 640, 320},
                    NULL, 2.0, SDL_FLIP_NONE, 0, true, NULL, 0);
    }

    cResource shadeResource;
    warperFilter filter = initWarperFilter(0x00, 0x00, 0x00, 0x58);
    initCResource(&shadeResource, &filter, drawWarperFilter, destroyWarperFilter, 0);

    initCCamera(&inputCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0);
    initCScene(&inputScene, (SDL_Color) {0xFF, 0xFF, 0xFF}, &inputCamera, (cSprite*[3]) {&tilesetSprite, &leftTileSprite, &rightTileSprite}, 3, (c2DModel*[3]) {&mapModel_layer1, &mapModel_layer2, &collisionModel}, 3, (cResource*[1]) {&shadeResource}, 1, NULL, 0);


    int previousIndex = 0;

    const warperMultiProperties multiProperties[WARPER_MULTI_PROPS_LEN] = WARPER_MULTI_PROPS;

    while(!quit)
    {
        SDL_Event e;  //this type of thing is needed because clicking & dragging doesn't work otherwise

        while(SDL_PollEvent(&e) != 0)
        {
            if(e.type == SDL_QUIT)
            {
                quit = true;
                fullQuit = true;
            }
            else
            {
                if(e.type == SDL_KEYDOWN)
                {
                    if (e.key.keysym.sym == SDLK_1)
                    {
                        drawMulti = false;

                        int temp = previousIndex;  //swap previousIndex and tile id
                        previousIndex = leftTileSprite.id;
                        leftTileSprite.id = temp;

                        leftTileSprite.srcClipRect.x = leftTileSprite.id / 20 * tileSize / 2;
                        leftTileSprite.srcClipRect.y = leftTileSprite.id % 20 * tileSize / 2;
                        leftTileSprite.srcClipRect.w = tileSize / 2;
                        leftTileSprite.srcClipRect.h = tileSize / 2;
                        leftTileSprite.drawRect.w = tileSize;
                        leftTileSprite.drawRect.h = tileSize;
                    }

                    if (e.key.keysym.sym == SDLK_2)
                    {
                        //multi (building) placing
                        drawMulti = true;

                        leftTileSprite.renderLayer = 2;
                        rightTileSprite.renderLayer = 0;

                        int temp = previousIndex;  //swap previousIndex and tile id
                        previousIndex = leftTileSprite.id;
                        leftTileSprite.id = temp;

                        leftTileSprite.srcClipRect.x = multiProperties[leftTileSprite.id].tileRect.x * tileSize / 2;
                        leftTileSprite.srcClipRect.y = multiProperties[leftTileSprite.id].tileRect.y * tileSize / 2;
                        leftTileSprite.srcClipRect.w = multiProperties[leftTileSprite.id].tileRect.w * tileSize / 2;
                        leftTileSprite.srcClipRect.h = multiProperties[leftTileSprite.id].tileRect.h * tileSize / 2;
                        leftTileSprite.drawRect.w = multiProperties[leftTileSprite.id].tileRect.w * tileSize;
                        leftTileSprite.drawRect.h = multiProperties[leftTileSprite.id].tileRect.h * tileSize;
                    }

                    if (e.key.keysym.sym == SDLK_q && leftTileSprite.id > 0 && drawMulti)
                    {
                        leftTileSprite.id--;

                        leftTileSprite.srcClipRect.x = multiProperties[leftTileSprite.id].tileRect.x * tileSize / 2;
                        leftTileSprite.srcClipRect.y = multiProperties[leftTileSprite.id].tileRect.y * tileSize / 2;
                        leftTileSprite.srcClipRect.w = multiProperties[leftTileSprite.id].tileRect.w * tileSize / 2;
                        leftTileSprite.srcClipRect.h = multiProperties[leftTileSprite.id].tileRect.h * tileSize / 2;
                        leftTileSprite.drawRect.w = multiProperties[leftTileSprite.id].tileRect.w * tileSize;
                        leftTileSprite.drawRect.h = multiProperties[leftTileSprite.id].tileRect.h * tileSize;

                    }

                    if (e.key.keysym.sym == SDLK_e && leftTileSprite.id < WARPER_MULTI_PROPS_LEN && drawMulti)
                    {
                        leftTileSprite.id++;

                        leftTileSprite.srcClipRect.x = multiProperties[leftTileSprite.id].tileRect.x * tileSize / 2;
                        leftTileSprite.srcClipRect.y = multiProperties[leftTileSprite.id].tileRect.y * tileSize / 2;
                        leftTileSprite.srcClipRect.w = multiProperties[leftTileSprite.id].tileRect.w * tileSize / 2;
                        leftTileSprite.srcClipRect.h = multiProperties[leftTileSprite.id].tileRect.h * tileSize / 2;
                        leftTileSprite.drawRect.w = multiProperties[leftTileSprite.id].tileRect.w * tileSize;
                        leftTileSprite.drawRect.h = multiProperties[leftTileSprite.id].tileRect.h * tileSize;
                    }

                    if (drawMulti && multiProperties[leftTileSprite.id].colToRepeat != -1 && e.key.keysym.sym == SDLK_MINUS)
                    {
                        //x-
                        if (leftTileSprite.drawRect.w > multiProperties[leftTileSprite.id].tileRect.w * tileSize)
                            leftTileSprite.drawRect.w -= tileSize;
                    }

                    if (drawMulti && multiProperties[leftTileSprite.id].colToRepeat != -1 && e.key.keysym.sym == SDLK_EQUALS)
                    {
                        //x+
                        leftTileSprite.drawRect.w += tileSize;
                    }

                    if (drawMulti && multiProperties[leftTileSprite.id].rowToRepeat != -1 && e.key.keysym.sym == SDLK_LEFTBRACKET)
                    {
                        if (leftTileSprite.drawRect.h > multiProperties[leftTileSprite.id].tileRect.h * tileSize)
                            leftTileSprite.drawRect.h -= tileSize;
                    }

                    if (drawMulti && multiProperties[leftTileSprite.id].rowToRepeat != -1 && e.key.keysym.sym == SDLK_RIGHTBRACKET)
                    {
                        //y+
                        leftTileSprite.drawRect.h += tileSize;
                    }

                    if (e.key.keysym.sym == SDLK_w)
                        inputCamera.rect.y -= 12;

                    if (e.key.keysym.sym == SDLK_s)
                        inputCamera.rect.y += 12;

                    if (e.key.keysym.sym == SDLK_a)
                        inputCamera.rect.x -= 12;

                    if (e.key.keysym.sym == SDLK_d)
                        inputCamera.rect.x += 12;

                    if (e.key.keysym.sym == SDLK_LSHIFT)
                    {  //show collision mode
                        spriteMode = !spriteMode;
                        pickMode = false;

                        collisionModel.renderLayer = spriteMode ? 0 : 4;

                        if (drawMulti)
                        {
                            int temp = previousIndex;  //swap previousIndex and tile id
                            previousIndex = leftTileSprite.id;
                            leftTileSprite.id = temp;
                        }
                        drawMulti = false;

                        leftTileSprite.srcClipRect.x = ((spriteMode) ? (leftTileSprite.id / 20) : 39) * tilemap->tileSize / 2;
                        leftTileSprite.srcClipRect.y = ((spriteMode) ? (leftTileSprite.id % 20) : 19) * tilemap->tileSize / 2;
                        leftTileSprite.srcClipRect.w = tileSize / 2;
                        leftTileSprite.srcClipRect.h = tileSize / 2;
                        leftTileSprite.drawRect.w = tileSize;
                        leftTileSprite.drawRect.h = tileSize;
                    }

                    if (e.key.keysym.sym == SDLK_RETURN)
                        quit = true;

                    if (e.key.keysym.sym == SDLK_SPACE)
                    {  //toggle upper and lower layers
                        drawLayer1 = !drawLayer1;
                        if (drawLayer1)
                            shadeResource.renderLayer = 0;
                        else
                            shadeResource.renderLayer = 5;
                    }

                    if (e.key.keysym.sym == SDLK_TAB)
                    {  //toggle pick sprite mode
                        spriteMode = !spriteMode;
                        pickMode = !pickMode;
                        drawMulti = false;

                        if (pickMode)
                        {
                            tilesetSprite.renderLayer = 2;  //display tileset
                            shadeResource.renderLayer = 3;  //put a shade over everything

                        }
                        else
                        {
                            tilesetSprite.renderLayer = 0;  //hide tileset
                            shadeResource.renderLayer = 0;  //hide shade
                        }
                    }
                }
                if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) || SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) && e.button.button != 0)
                {
                    int tileX = (e.button.x + inputCamera.rect.x) / tilemap->tileSize, tileY = (e.button.y + inputCamera.rect.y) / tilemap->tileSize;

                    if (tileX >= 0 && tileX < tilemap->width && tileY >= 0 && tileY < tilemap->height)
                    {
                        if (spriteMode)
                        {
                            int** layer = tilemap->spritemap_layer1;
                            c2DModel* layerModel = &mapModel_layer1;
                            if (!drawLayer1)
                            {
                                layer = tilemap->spritemap_layer2;
                                layerModel = &mapModel_layer2;
                            }

                            if (!drawMulti)  //shift draw to layer 2
                            {
                                //draw single
                                if (!(tileX < 0 || tileX >= tilemap->width || tileY < 0 || tileY >= tilemap->height))  //if we are within bounds
                                {
                                    layer[tileX][tileY] = (e.button.button == SDL_BUTTON_LEFT) ? leftTileSprite.id : rightTileSprite.id;
                                    layerModel->sprites[tileX * tilemap->height + tileY].id = layer[tileX][tileY];
                                    layerModel->sprites[tileX * tilemap->height + tileY].srcClipRect.x = (layer[tileX][tileY] / 20) * tilemap->tileSize / 2;
                                    layerModel->sprites[tileX * tilemap->height + tileY].srcClipRect.y = (layer[tileX][tileY] % 20) * tilemap->tileSize / 2;

                                    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
                                    {  //left click
                                        leftTileSprite.renderLayer = 2;
                                        rightTileSprite.renderLayer = 0;
                                    }
                                    else
                                    {  //right click
                                        leftTileSprite.renderLayer = 0;
                                        rightTileSprite.renderLayer = 2;
                                    }
                                }
                            }
                            else
                            {
                                //draw multi
                                int difW = leftTileSprite.drawRect.w / tileSize - multiProperties[leftTileSprite.id].tileRect.w,
                                    difH = leftTileSprite.drawRect.h / tileSize - multiProperties[leftTileSprite.id].tileRect.h;

                                int xOffset = 0;
                                for(int x = tileX; x < tileX + leftTileSprite.drawRect.w / tileSize; x++)
                                {
                                    if (difW > 0 && x - tileX > multiProperties[leftTileSprite.id].colToRepeat && x - tileX <= multiProperties[leftTileSprite.id].colToRepeat + difW)
                                        xOffset++;

                                    int yOffset = 0;
                                    for(int y = tileY; y < tileY + leftTileSprite.drawRect.h / tileSize; y++)
                                    {
                                        if (difH > 0 && y - tileY > multiProperties[leftTileSprite.id].rowToRepeat && y - tileY <= multiProperties[leftTileSprite.id].rowToRepeat + difH)
                                            yOffset++;

                                        int drawY = multiProperties[leftTileSprite.id].tileRect.y + y - tileY - yOffset,
                                            drawX = multiProperties[leftTileSprite.id].tileRect.x + x - tileX - xOffset;

                                        if (!(x < 0 || x >= tilemap->width || y < 0 || y >= tilemap->height))  //if we're drawing within bounds
                                        {
                                            layer[x][y] = drawY % 20 + drawX * 20;
                                            layerModel->sprites[x * tilemap->height + y].id = layer[x][y];
                                            layerModel->sprites[x * tilemap->height + y].srcClipRect.x = (layer[x][y] / 20) * tilemap->tileSize / 2;
                                            layerModel->sprites[x * tilemap->height + y].srcClipRect.y = (layer[x][y] % 20) * tilemap->tileSize / 2;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (pickMode)
                            {
                                //visual tile picker
                                int pickX = e.button.x / tileSize, pickY = e.button.y / tileSize;

                                //pick sprite
                                cSprite* tileSprite = &rightTileSprite;
                                rightTileSprite.renderLayer = 0;
                                leftTileSprite.renderLayer = 0;

                                if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
                                    tileSprite = &leftTileSprite;

                                tileSprite->srcClipRect.x = pickX * tileSize / tilesetSprite.scale;
                                tileSprite->srcClipRect.y = pickY * tileSize / tilesetSprite.scale;
                                tileSprite->srcClipRect.w = tileSize / 2;
                                tileSprite->srcClipRect.h = tileSize / 2;
                                tileSprite->id = pickY + pickX * tilesetSprite.srcClipRect.w / tileSize;
                                tileSprite->drawRect.w = tileSize;
                                tileSprite->drawRect.h = tileSize;
                                tileSprite->renderLayer = 2;
                                //printf("pick %f, %f, %d\n", tileSprite->srcClipRect.x, tileSprite->srcClipRect.y, tileSprite->id);
                            }
                            else
                            {
                                //draw collision
                                tilemap->collisionmap[tileX][tileY] = (e.button.button == SDL_BUTTON_LEFT) ? 1 : 0;
                                collisionModel.sprites[tileX * tilemap->height + tileY].id = 1;
                                collisionModel.sprites[tileX * tilemap->height + tileY].srcClipRect.x = 39 * tilemap->tileSize / 2;
                                collisionModel.sprites[tileX * tilemap->height + tileY].srcClipRect.y = (18 + (e.button.button == SDL_BUTTON_LEFT)) * tilemap->tileSize / 2;
                            }
                        }
                    }
                }
                if (e.type == SDL_MOUSEMOTION)
                {
                    leftTileSprite.drawRect.x = e.motion.x + inputCamera.rect.x - tilemap->tileSize / 2;
                    leftTileSprite.drawRect.y = e.motion.y + inputCamera.rect.y - tilemap->tileSize / 2;

                    rightTileSprite.drawRect.x = e.motion.x + inputCamera.rect.x - tilemap->tileSize / 2;
                    rightTileSprite.drawRect.y = e.motion.y + inputCamera.rect.y - tilemap->tileSize / 2;
                }
            }
        }
        drawCScene(&inputScene, true, true, NULL, NULL, 60);
    }

    char* tileMapData = calloc(3 * 3 * tilemap->width * tilemap->height + 3 + 3 + 1, sizeof(char));  //3 arrays * 3 digits * width * height + width data + height data + 1 to be safe
    exportTilemap(*tilemap, tileMapData);

    printf("%s\n", tileMapData);
    free(tileMapData);

    destroyCScene(&inputScene);
    return fullQuit;
}

warperFilter initWarperFilter(int r, int g, int b, int a)
{
    return (warperFilter) {.filterColor = (SDL_Color) {r, g, b, a}};
}

void drawWarperFilter(void* subclass, cCamera camera)
{
    warperFilter* filter = (warperFilter*) subclass;
    Uint8 prevR, prevG, prevB, prevA;
    SDL_GetRenderDrawColor(global.mainRenderer, &prevR, &prevG, &prevB, &prevA);
    SDL_SetRenderDrawColor(global.mainRenderer, filter->filterColor.r, filter->filterColor.g, filter->filterColor.b, filter->filterColor.a);
    SDL_RenderFillRect(global.mainRenderer, NULL);
    SDL_SetRenderDrawColor(global.mainRenderer, prevR, prevG, prevB, prevA);
}

void destroyWarperFilter(void* subclass)
{
    warperFilter* filter = (warperFilter*) subclass;
    filter->filterColor = (SDL_Color) {0,0,0,0};
}
