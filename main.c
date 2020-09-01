#include "CoSprite/csGraphics.h"
#include "CoSprite/csInput.h"

#define TILEMAP_X 80  //(global.windowW / TILE_SIZE)
#define TILEMAP_Y 60  //(global.windowH / TILE_SIZE)

int main(int argc, char** argv)
{
    if (argc > 1)
        argv = argv;//useless, but prevents warning

    const int TILE_SIZE = 32;

    int error = initCoSprite("./assets/cb.bmp", "Warper", 1280, 640, "assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE, 5, (SDL_Color) {255, 28, 198, 0xFF}, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    int tilemap[TILEMAP_X][TILEMAP_Y];

    for(int x = 0; x < TILEMAP_X; x++)
    {
        for(int y = 0; y < TILEMAP_Y; y++)
        {
            if (x == 0 || y == 0 || x + 1 == TILEMAP_X || y + 1 == TILEMAP_Y || (y == 20 && x > TILEMAP_X / 2) || (y == 40 && x < TILEMAP_X / 2))
                tilemap[x][y] = 1;
            else
                tilemap[x][y] = 0;
        }
    }

    c2DModel mapModel;
    cSprite testPlayerSprite;
    {
        SDL_Texture* tilesetTexture;
        loadIMG("assets/tilesheet.png", &tilesetTexture);
        cSprite* tileSprites = calloc(TILEMAP_X * TILEMAP_Y, sizeof(cSprite));
        for(int x = 0; x < TILEMAP_X; x++)
        {
            for(int y = 0; y < TILEMAP_Y; y++)
            {
                initCSprite(&tileSprites[x * TILEMAP_Y + y], tilesetTexture, "assets/tilesheet.png", tilemap[x][y], (cDoubleRect) {TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE}, (cDoubleRect) {(tilemap[x][y] / 32) * TILE_SIZE, (tilemap[x][y] % 32) * TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
            }
        }
        initC2DModel(&mapModel, tileSprites, TILEMAP_X * TILEMAP_Y, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initCSprite(&testPlayerSprite, tilesetTexture, "assets/tileset.png", 0, (cDoubleRect) {TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE}, (cDoubleRect) {TILE_SIZE, 0, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 5);
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

        if (input.quitInput || input.keyStates[SDL_SCANCODE_RETURN] || input.keyStates[SDL_SCANCODE_ESCAPE])
            quit = true;



        //character movement
        if (input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_S] || input.keyStates[SDL_SCANCODE_D])
        {
            double speed = 6.0;  //just a good speed value, nothing special. Pixels/frame at 60 FPS
            if ((input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_S]) && (input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_D]))
                speed = 6 * sin(degToRad(45));  //diagonal speed component

            if (input.keyStates[SDL_SCANCODE_W])
            {
                testPlayerSprite.drawRect.y -= speed * 60.0 / framerate;

                testCamera.rect.y = testPlayerSprite.drawRect.y - testCamera.rect.h / 2;  //set the camera to center on the player
            }

            if (input.keyStates[SDL_SCANCODE_S])
            {
                testPlayerSprite.drawRect.y += speed * 60.0 / framerate;

                testCamera.rect.y = testPlayerSprite.drawRect.y - testCamera.rect.h / 2;  //set the camera to center on the player
            }

            if (input.keyStates[SDL_SCANCODE_A])
            {
                testPlayerSprite.drawRect.x -= speed * 60.0 / framerate;

                testCamera.rect.x = testPlayerSprite.drawRect.x - testCamera.rect.w / 2;  //set the camera to center on the player
            }

            if (input.keyStates[SDL_SCANCODE_D])
            {
                testPlayerSprite.drawRect.x += speed * 60.0 / framerate;

                testCamera.rect.x = testPlayerSprite.drawRect.x - testCamera.rect.w / 2;  //set the camera to center on the player

            }
            if (testCamera.rect.y < 0)  //if the camera is set out of bounds in the -y, fix it
                testCamera.rect.y = 0;

            if (testCamera.rect.y > (TILEMAP_Y - testCamera.rect.h / TILE_SIZE) * TILE_SIZE)  //if the camera is set out of bounds in the +y, fix it
                testCamera.rect.y = (TILEMAP_Y - testCamera.rect.h / TILE_SIZE) * TILE_SIZE;

            if (testCamera.rect.x < 0)  //if the camera is set out of bounds in the -x, fix it
                testCamera.rect.x = 0;

            if (testCamera.rect.x > (TILEMAP_X - testCamera.rect.w / TILE_SIZE) * TILE_SIZE)  //if the camera is set out of bounds in the +x, fix it
                testCamera.rect.x = (TILEMAP_X - testCamera.rect.w / TILE_SIZE) * TILE_SIZE;
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

    closeCoSprite();

    return error;
}
