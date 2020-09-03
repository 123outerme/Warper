#include "CoSprite/csGraphics.h"
#include "CoSprite/csInput.h"
#include "mapMaker.h"

cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap);

#define TILEMAP_X 80  //(global.windowW / TILE_SIZE)
#define TILEMAP_Y 60  //(global.windowH / TILE_SIZE)

int main(int argc, char** argv)
{
    if (argc > 1)
        argv = argv;//useless, but prevents warning

    const int TILE_SIZE = 32;

    int error = initCoSprite("./assets/cb.bmp", "Warper", 1280, 640, "assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE, 5, (SDL_Color) {255, 28, 198, 0xFF}, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    //choose between map-maker or not
    SDL_Keycode temp_k = waitForKey(true);

    warperTilemap tilemap;

    if (temp_k == SDLK_m)
    {
        createNewMap(&tilemap, TILE_SIZE);
    }
    else
    {
        tilemap.width = TILEMAP_X;
        tilemap.height = TILEMAP_Y;
        tilemap.tileSize = TILE_SIZE;

        tilemap.spritemap = calloc(tilemap.width, sizeof(int*));
        tilemap.collisionmap = calloc(tilemap.width, sizeof(int*));
        tilemap.eventmap = calloc(tilemap.width, sizeof(int*));

        for(int x = 0; x < tilemap.width; x++)
        {
            tilemap.spritemap[x] = calloc(tilemap.height, sizeof(int*));
            tilemap.collisionmap[x] = calloc(tilemap.height, sizeof(int*));
            tilemap.eventmap[x] = calloc(tilemap.height, sizeof(int*));

            for(int y = 0; y < tilemap.height; y++)
            {
                if (x == 0 || y == 0 || x + 1 == tilemap.width || y + 1 == tilemap.height || (y == 20 && x > tilemap.width / 2) || (y == 40 && x < tilemap.height / 2))
                {
                    tilemap.spritemap[x][y] = 1;
                    tilemap.collisionmap[x][y] = 1;
                }
                else
                {
                    tilemap.spritemap[x][y] = 0;
                    tilemap.collisionmap[x][y] = 0;
                }
                tilemap.eventmap[x][y] = 0;
            }
        }
    }

    c2DModel mapModel;
    cSprite testPlayerSprite;
    {
        SDL_Texture* tilesetTexture;
        loadIMG("assets/tilesheet.png", &tilesetTexture);
        cSprite* tileSprites = calloc(tilemap.width * tilemap.height, sizeof(cSprite));
        for(int x = 0; x < tilemap.width; x++)
        {
            for(int y = 0; y < tilemap.height; y++)
            {
                initCSprite(&tileSprites[x * tilemap.height + y], tilesetTexture, "assets/tilesheet.png", tilemap.spritemap[x][y], (cDoubleRect) {tilemap.tileSize * x, tilemap.tileSize * y, tilemap.tileSize, tilemap.tileSize}, (cDoubleRect) {(tilemap.spritemap[x][y] / 32) * tilemap.tileSize, (tilemap.spritemap[x][y] % 32) * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
            }
        }
        initC2DModel(&mapModel, tileSprites, tilemap.width * tilemap.height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initCSprite(&testPlayerSprite, tilesetTexture, "assets/tileset.png", 0, (cDoubleRect) {tilemap.tileSize, tilemap.tileSize, tilemap.tileSize, tilemap.tileSize}, (cDoubleRect) {tilemap.tileSize, 0, tilemap.tileSize, tilemap.tileSize}, NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 4);
   }

    cCamera testCamera;
    initCCamera(&testCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1, 0.0);

    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, (cSprite*[1]) {&testPlayerSprite}, 1, (c2DModel*[1]) {&mapModel}, 1, NULL, 0, NULL, 0);

    bool quit = false;
    cInputState input;
    int framerate = 0;

    while(!quit)
    {
        input = cGetInputState(true);

        if (input.quitInput || input.keyStates[SDL_SCANCODE_ESCAPE] || input.keyStates[SDL_SCANCODE_RETURN])
            quit = true;



        //character movement
        if (input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_S] || input.keyStates[SDL_SCANCODE_D])
        {
            double speed = 6.0;  //just a good speed value, nothing special. Pixels/frame at 60 FPS
            if ((input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_S]) && (input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_D]))
                speed *= sin(degToRad(45));  //diagonal speed component

            if (input.keyStates[SDL_SCANCODE_W])
                testPlayerSprite.drawRect.y -= speed * 60.0 / framerate;

            if (input.keyStates[SDL_SCANCODE_S])
                testPlayerSprite.drawRect.y += speed * 60.0 / framerate;

            cDoubleVector mtv = getTilemapCollision(testPlayerSprite, tilemap);

            if (mtv.magnitude)
            {  //apply collision after doing y movements
                testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);
            }

            if (input.keyStates[SDL_SCANCODE_A])
                testPlayerSprite.drawRect.x -= speed * 60.0 / framerate;

            if (input.keyStates[SDL_SCANCODE_D])
                testPlayerSprite.drawRect.x += speed * 60.0 / framerate;

            mtv = getTilemapCollision(testPlayerSprite, tilemap);

            if (mtv.magnitude)
            {  //apply collision again after doing x movements (allows smooth collision sliding. The only way I could figure out how to fix it without 100% hard-coding)
                testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);
            }

            testCamera.rect.x = testPlayerSprite.drawRect.x - testCamera.rect.w / 2;  //set the camera to center on the player
            testCamera.rect.y = testPlayerSprite.drawRect.y - testCamera.rect.h / 2;

            if (testCamera.rect.y < 0)  //if the camera is set out of bounds in the -y, fix it
                testCamera.rect.y = 0;

            if (testCamera.rect.y > (tilemap.height - testCamera.rect.h / tilemap.tileSize) * tilemap.tileSize)  //if the camera is set out of bounds in the +y, fix it
                testCamera.rect.y = (tilemap.height - testCamera.rect.h / tilemap.tileSize) * tilemap.tileSize;

            if (testCamera.rect.x < 0)  //if the camera is set out of bounds in the -x, fix it
                testCamera.rect.x = 0;

            if (testCamera.rect.x > (tilemap.width - testCamera.rect.w / tilemap.tileSize) * tilemap.tileSize)  //if the camera is set out of bounds in the +x, fix it
                testCamera.rect.x = (tilemap.width - testCamera.rect.w / tilemap.tileSize) * tilemap.tileSize;
        }

        //camera movement
        if (input.keyStates[SDL_SCANCODE_UP])
            testCamera.rect.y -= 10 * 60.0 / framerate;

        if (input.keyStates[SDL_SCANCODE_DOWN])
            testCamera.rect.y += 10 * 60.0 / framerate;

        if (input.keyStates[SDL_SCANCODE_LEFT])
            testCamera.rect.x -= 10 * 60.0 / framerate;

        if (input.keyStates[SDL_SCANCODE_RIGHT])
            testCamera.rect.x += 10 * 60.0 / framerate;

        drawCScene(&testScene, true, true, &framerate, 60);
    }
    destroyWarperTilemap(&tilemap);

    closeCoSprite();

    return error;
}

cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap)
{
    cDoubleVector mtv = {0, 0};

    int playerX = round(playerSprite.drawRect.x / tilemap.tileSize), playerY = round(playerSprite.drawRect.y / tilemap.tileSize);

    for(int x = playerX - 1; x <= playerX + 1; x++)
    {
        for(int y = playerY - 1; y <= playerY + 1; y++)
        {
            //printf("%d, %d", x, y);
            if (x >= 0 && x < tilemap.width && y >= 0 && y < tilemap.height && tilemap.collisionmap[x][y])
            {
                cDoubleVector newMtv = checkCDoubleRectCollision(playerSprite.drawRect, (cDoubleRect) {x * tilemap.tileSize, y * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize});  //get collision result
                if ((mtv.magnitude == 0 || ((int) newMtv.degrees % 180 == 90 && (int) mtv.degrees % 180 == 0) || ((int) newMtv.degrees % 180 == 0 && (int) mtv.degrees % 180 == 90)) && (int) mtv.degrees % 90 != 45)
                {
                    mtv = addCDoubleVectors(mtv, newMtv);  //if we don't have a partial mtv on the same axis, add these partials together
                    //printf("- found %f at %f deg", newMtv.magnitude, newMtv.degrees);
                }
            }
            //printf("\n");
        }
    }
    //printf("--------\n");

    return mtv;
}
