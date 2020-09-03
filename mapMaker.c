#include "mapMaker.h"

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

void createNewMap(warperTilemap* tilemap, int tileSize)
{
    bool quit = false;

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
        SDL_Keycode key = getKey(false);

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

    tilemap->width = strtol(dimensionInput, NULL, 10);

    quit = false;
    free(dimensionInput);

    dimensionInput = calloc(5, sizeof(char));

    updateCText(&inputText, "   ");
    updateCText(&promptText, "Input height:");
    drawCScene(&inputScene, true, true, NULL, 0);
    while(!quit)
    {
        SDL_Keycode key = getKey(false);
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

    tilemap->tileSize = tileSize;

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
            tilemap->spritemap[x][y] = 1;
            tilemap->collisionmap[x][y] = 0;
            tilemap->eventmap[x][y] = -1;
        }
    }

    quit = false;
    bool spriteMode = true;

    c2DModel mapModel;
    c2DModel collisionModel;
    cSprite tileSprite;
    {
        SDL_Texture* tilesetTexture;
        loadIMG("./assets/tilesheet.png", &tilesetTexture);
        cSprite* tileSprites = calloc(tilemap->width * tilemap->height, sizeof(cSprite));
        cSprite* collisionSprites = calloc(tilemap->width * tilemap->height, sizeof(cSprite));
        for(int x = 0; x < tilemap->width; x++)
        {
            for(int y = 0; y < tilemap->height; y++)
            {
                initCSprite(&tileSprites[x * tilemap->height + y], tilesetTexture, "./assets/tilesheet.png", tilemap->spritemap[x][y], (cDoubleRect) {tilemap->tileSize * x, tilemap->tileSize * y, tilemap->tileSize, tilemap->tileSize}, (cDoubleRect) {(tilemap->spritemap[x][y] / 32) * tilemap->tileSize, (tilemap->spritemap[x][y] % 32) * tilemap->tileSize, tilemap->tileSize, tilemap->tileSize}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
                initCSprite(&collisionSprites[x * tilemap->height + y], tilesetTexture, "./assets/tilesheet.png", tilemap->collisionmap[x][y], (cDoubleRect) {tilemap->tileSize * x, tilemap->tileSize * y, tilemap->tileSize, tilemap->tileSize}, (cDoubleRect) {40 * tilemap->tileSize, (tilemap->collisionmap[x][y] + 19) * tilemap->tileSize, tilemap->tileSize, tilemap->tileSize}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
            }
        }
        initC2DModel(&mapModel, tileSprites, tilemap->width * tilemap->height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
        initC2DModel(&collisionModel, collisionSprites, tilemap->width * tilemap->height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initCSprite(&tileSprite, tilesetTexture, "./assets/tilesheet.png", 1, (cDoubleRect) {tilemap->tileSize, tilemap->tileSize, tilemap->tileSize, tilemap->tileSize}, (cDoubleRect) {0, tilemap->tileSize, tilemap->tileSize, tilemap->tileSize}, NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 4);
    }

    initCCamera(&inputCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0);
    initCScene(&inputScene, (SDL_Color) {0xFF, 0xFF, 0xFF}, &inputCamera, (cSprite*[1]) {&tileSprite}, 1, (c2DModel*[2]) {&mapModel, &collisionModel}, 2, NULL, 0, NULL, 0);

    while(!quit)
    {
        cInputState input = cGetInputState(true);

        if (input.quitInput || input.keysym.sym == SDLK_SPACE)
            quit = true;

        if (input.isClick)
        {
            int tileX = input.click.x / tilemap->tileSize, tileY = input.click.y / tilemap->tileSize;

            if (spriteMode)
            {
                tilemap->spritemap[tileX][tileY] = (input.click.button == SDL_BUTTON_LEFT) ? tileSprite.id : 0;
                mapModel.sprites[tileX * tilemap->height + tileY].id = tilemap->spritemap[tileX][tileY];
                mapModel.sprites[tileX * tilemap->height + tileY].srcClipRect.x = (tilemap->spritemap[tileX][tileY] / 32) * tilemap->tileSize;
                mapModel.sprites[tileX * tilemap->height + tileY].srcClipRect.y = (tilemap->spritemap[tileX][tileY] % 32) * tilemap->tileSize;
            }
            else
            {
                tilemap->collisionmap[tileX][tileY] = (input.click.button == SDL_BUTTON_LEFT) ? 1 : 0;
                collisionModel.sprites[tileX * tilemap->height + tileY].id = 1;
                collisionModel.sprites[tileX * tilemap->height + tileY].srcClipRect.x = 40;
                collisionModel.sprites[tileX * tilemap->height + tileY].srcClipRect.y = 19 + (input.click.button == SDL_BUTTON_LEFT);
            }
        }
        else
        {
            if (input.keyStates[SDL_SCANCODE_Q] && tileSprite.id > 0)
                tileSprite.id--;

            if (input.keyStates[SDL_SCANCODE_E])
                tileSprite.id++;

            if (input.keyStates[SDL_SCANCODE_LSHIFT])
            {
                spriteMode = !spriteMode;
                collisionModel.renderLayer = spriteMode ? 0 : 4;
            }

            tileSprite.srcClipRect.x = ((spriteMode) ? (tileSprite.id / 32) : 40) * tilemap->tileSize;
            tileSprite.srcClipRect.y = ((spriteMode) ? (tileSprite.id % 32) : 20) * tilemap->tileSize;
        }

        if (input.motion.x >= 0)
        {
            tileSprite.drawRect.x = input.motion.x - tilemap->tileSize / 2;
            tileSprite.drawRect.y = input.motion.y - tilemap->tileSize / 2;
        }

        drawCScene(&inputScene, true, true, NULL, 60);
    }

    destroyCScene(&inputScene);
}
