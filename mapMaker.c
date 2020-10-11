#include "mapMaker.h"

void createNewMap(warperTilemap* tilemap, int tileSize)
{
    bool quit = false;
    SDL_Keycode key;

    int width = 40, height = 40;
    char* dimensionInput = calloc(5, sizeof(char));
    cScene inputScene;
    cCamera inputCamera;
    cText inputText;
    cText promptText;
    initCText(&promptText, "Input width:", (cDoubleRect) {0, global.windowH / 4, 12 * global.mainFont.fontSize, global.mainFont.fontSize}, 13 * global.mainFont.fontSize, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
    initCText(&inputText, " ", (cDoubleRect) {0, global.windowH / 2, 4 * global.mainFont.fontSize, global.mainFont.fontSize}, 5 * global.mainFont.fontSize, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
    initCCamera(&inputCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0);
    initCScene(&inputScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &inputCamera, NULL, 0, NULL, 0, NULL, 0, (cText*[2]) {&promptText, &inputText}, 2);
    drawCScene(&inputScene, true, true, NULL, 0);
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

        drawCScene(&inputScene, true, true, NULL, 60);
    }
    if (key == -1)
    {
        free(dimensionInput);
        destroyCScene(&inputScene);
        return;
    }

    key = SDLK_UNKNOWN;

    tilemap->width = strtol(dimensionInput, NULL, 10);
    //printf("%d\n", tilemap->width);
    quit = false;
    free(dimensionInput);

    dimensionInput = calloc(5, sizeof(char));

    updateCText(&inputText, "   ");
    updateCText(&promptText, "Input height:");
    drawCScene(&inputScene, true, true, NULL, 0);
    while(!quit)
    {
        key = getKey(false);
        if (key == -1 || key == SDLK_ESCAPE || key == SDLK_RETURN)
            quit = true;

        handleTextKeycodeInput(dimensionInput, key, 4);

        if ((key >= SDLK_0 && key <= SDLK_z) || key == SDLK_SPACE || key == SDLK_BACKSPACE)
            updateCText(&inputText, dimensionInput);

        drawCScene(&inputScene, true, true, NULL, 60);
    }

    tilemap->height = strtol(dimensionInput, NULL, 10);
    free(dimensionInput);
    destroyCScene(&inputScene);

    if (key == -1)
        return;

    tilemap->tileSize = tileSize;

    tilemap->spritemap = calloc(width, sizeof(uint8_t*));
    tilemap->collisionmap = calloc(width, sizeof(uint8_t*));
    for(int x = 0; x < width; x++)
    {
        tilemap->spritemap[x] = calloc(height, sizeof(uint8_t));
        tilemap->collisionmap[x] = calloc(height, sizeof(uint8_t));
        for(int y = 0; y < height; y++)
        {
            tilemap->spritemap[x][y] = 4;
            tilemap->collisionmap[x][y] = 0;
        }
    }

    quit = false;
    bool spriteMode = true;

    c2DModel mapModel;
    c2DModel collisionModel;
    cSprite tileSprite;
    {
        SDL_Texture* tilesetTexture;
        loadIMG("./assets/worldTilesheet.png", &tilesetTexture);
        cSprite* tileSprites = calloc(tilemap->width * tilemap->height, sizeof(cSprite));
        cSprite* collisionSprites = calloc(tilemap->width * tilemap->height, sizeof(cSprite));
        for(int x = 0; x < tilemap->width; x++)
        {
            for(int y = 0; y < tilemap->height; y++)
            {
                initCSprite(&tileSprites[x * tilemap->height + y], tilesetTexture, "./assets/worldTilesheet.png", tilemap->spritemap[x][y],
                            (cDoubleRect) {tilemap->tileSize * x, tilemap->tileSize * y, tilemap->tileSize, tilemap->tileSize},
                            (cDoubleRect) {(tilemap->spritemap[x][y] / 20) * tilemap->tileSize / 2, (tilemap->spritemap[x][y] % 20) * tilemap->tileSize / 2, tilemap->tileSize / 2, tilemap->tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

                initCSprite(&collisionSprites[x * tilemap->height + y], tilesetTexture, "./assets/worldTilesheet.png", tilemap->collisionmap[x][y],
                            (cDoubleRect) {tilemap->tileSize * x, tilemap->tileSize * y, tilemap->tileSize, tilemap->tileSize},
                            (cDoubleRect) {39 * tilemap->tileSize / 2, (tilemap->collisionmap[x][y] + 18) * tilemap->tileSize / 2, tilemap->tileSize / 2, tilemap->tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
            }
        }
        initC2DModel(&mapModel, tileSprites, tilemap->width * tilemap->height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
        initC2DModel(&collisionModel, collisionSprites, tilemap->width * tilemap->height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initCSprite(&tileSprite, tilesetTexture, "./assets/worldTilesheet.png", 0,
                    (cDoubleRect) {tilemap->tileSize, tilemap->tileSize, tilemap->tileSize, tilemap->tileSize},
                    (cDoubleRect) {0, 0, tilemap->tileSize / 2, tilemap->tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 4);
    }

    initCCamera(&inputCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0);
    initCScene(&inputScene, (SDL_Color) {0xFF, 0xFF, 0xFF}, &inputCamera, (cSprite*[1]) {&tileSprite}, 1, (c2DModel*[2]) {&mapModel, &collisionModel}, 2, NULL, 0, NULL, 0);

    while(!quit)
    {
        SDL_Event e;  //this type of thing is needed because clicking & dragging doesn't work otherwise

        while(SDL_PollEvent(&e) != 0)
        {
            if(e.type == SDL_QUIT)
            {
                quit = true;
            }
            else
            {
                if(e.type == SDL_KEYDOWN)
                {
                    if (e.key.keysym.sym == SDLK_q && tileSprite.id > 0)
                        tileSprite.id--;

                    if (e.key.keysym.sym == SDLK_e)
                        tileSprite.id++;

                    if (e.key.keysym.sym == SDLK_w)
                        inputCamera.rect.y -= 6;

                    if (e.key.keysym.sym == SDLK_s)
                        inputCamera.rect.y += 6;

                    if (e.key.keysym.sym == SDLK_a)
                        inputCamera.rect.x -= 6;

                    if (e.key.keysym.sym == SDLK_d)
                        inputCamera.rect.x += 6;

                    if (e.key.keysym.sym == SDLK_LSHIFT)
                    {
                        spriteMode = !spriteMode;
                        collisionModel.renderLayer = spriteMode ? 0 : 4;
                    }

                    if (e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_RETURN)
                        quit = true;

                    tileSprite.srcClipRect.x = ((spriteMode) ? (tileSprite.id / 20) : 39) * tilemap->tileSize / 2;
                    tileSprite.srcClipRect.y = ((spriteMode) ? (tileSprite.id % 20) : 19) * tilemap->tileSize / 2;
                }
                if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) || SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) && e.button.button != 0)
                {
                    int tileX = (e.button.x + inputCamera.rect.x) / tilemap->tileSize, tileY = (e.button.y + inputCamera.rect.y) / tilemap->tileSize;
                    if (tileX >= 0 && tileY >= 0)
                    {
                        if (spriteMode)
                        {
                            tilemap->spritemap[tileX][tileY] = (e.button.button == SDL_BUTTON_LEFT) ? tileSprite.id : 0;
                            mapModel.sprites[tileX * tilemap->height + tileY].id = tilemap->spritemap[tileX][tileY];
                            mapModel.sprites[tileX * tilemap->height + tileY].srcClipRect.x = (tilemap->spritemap[tileX][tileY] / 20) * tilemap->tileSize / 2;
                            mapModel.sprites[tileX * tilemap->height + tileY].srcClipRect.y = (tilemap->spritemap[tileX][tileY] % 20) * tilemap->tileSize / 2;
                        }
                        else
                        {
                            tilemap->collisionmap[tileX][tileY] = (e.button.button == SDL_BUTTON_LEFT) ? 1 : 0;
                            collisionModel.sprites[tileX * tilemap->height + tileY].id = 1;
                            collisionModel.sprites[tileX * tilemap->height + tileY].srcClipRect.x = 39 * tilemap->tileSize / 2;
                            collisionModel.sprites[tileX * tilemap->height + tileY].srcClipRect.y = (18 + (e.button.button == SDL_BUTTON_LEFT)) * tilemap->tileSize / 2;
                        }
                    }
                }
                if (e.type == SDL_MOUSEMOTION)
                {
                    tileSprite.drawRect.x = e.motion.x + inputCamera.rect.x - tilemap->tileSize / 2;
                    tileSprite.drawRect.y = e.motion.y + inputCamera.rect.y - tilemap->tileSize / 2;
                }
            }
        }
        drawCScene(&inputScene, true, true, NULL, 60);
    }

    char* tileMapData = calloc(3 * 3 * tilemap->width * tilemap->height + 3 + 3 + 1, sizeof(char));  //3 arrays * 3 digits * width * height + width data + height data + 1 to be safe
    exportTilemap(*tilemap, tileMapData);

    printf("%s\n", tileMapData);
    free(tileMapData);

    destroyCScene(&inputScene);
}
