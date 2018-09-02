//#include "csMain.h"
#include "csGraphics.h"
#include "csFile.h"
#include "csIO.h"
#include "csUtility.h"

#define MAX_SKILLS 10
typedef struct _player {
    int walkFrame;
    int HP;
    int maxHP;
    int skills[MAX_SKILLS];
} player;

#define calcWaitTime(x) x == 0 ? 0 : 1000 / x

player initPlayer(int maxHealth);

const int armRotations[10] = {0, 10, 20, 25, 28, 30, 28, 25, 21, 9};
const int legRotations[10] = {0, -14, -24, -26, -29, -33, -28, -23, -18, -8};

int main(int argc, char* argv[])
{
    const int TILE_SIZE = 32;
    int range = 7 * TILE_SIZE;  //10 * TILE_SIZE was a good range
    int error = initCoSprite("", "Warper", 1280, 640, "assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE);
    int tilemap[(windowW / TILE_SIZE)][(windowH / TILE_SIZE)];

    for(int x = 0; x < windowW / TILE_SIZE; x++)
    {
        for(int y = 0; y < windowH / TILE_SIZE; y++)
        {
            if (x == 0 || y == 0 || x + 1 == windowW / TILE_SIZE || y + 1 == windowH / TILE_SIZE)
                tilemap[x][y] = 1;
            else
                tilemap[x][y] = 0;
        }
    }
    int frame = 0, framerate = 0, targetTime = calcWaitTime(60), sleepFor = 0;
    SDL_Texture* mouseTexture;
    loadIMG("assets/cb.bmp", &mouseTexture);
    cSprite mouseSprite;
    c2DModel playerModel;
    {
        SDL_Texture* playerTexture;
        loadIMG("assets/tilesheet.png", &playerTexture);
        player thisPlayer = initPlayer(10);
        cSprite playerSprites[7];
        initCSprite(&mouseSprite, mouseTexture, 0, (SDL_Rect) {0, 0, 80, 80}, (SDL_Rect) {15, 0, 120, 120}, NULL, 1.0, SDL_FLIP_NONE, 0.0, true, NULL, 1);
        initCSprite(&playerSprites[0], playerTexture, 1, (SDL_Rect) {TILE_SIZE, 0, TILE_SIZE, TILE_SIZE}, (SDL_Rect) {0, 0, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2); //head
        initCSprite(&playerSprites[1], playerTexture, 2, (SDL_Rect) {TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE}, (SDL_Rect) {TILE_SIZE, 0, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3); //torso
        initCSprite(&playerSprites[2], playerTexture, 3, (SDL_Rect) {0, TILE_SIZE, TILE_SIZE, 2 * TILE_SIZE}, (SDL_Rect) {2 * TILE_SIZE, 0, TILE_SIZE, 2 * TILE_SIZE}, &((SDL_Point) {TILE_SIZE / 2, TILE_SIZE / 2}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);  //left arm
        initCSprite(&playerSprites[3], playerTexture, 4, (SDL_Rect) {2 * TILE_SIZE, TILE_SIZE, TILE_SIZE, 2 * TILE_SIZE}, (SDL_Rect) {2 * TILE_SIZE, 0, TILE_SIZE, 2 * TILE_SIZE}, &((SDL_Point) {TILE_SIZE / 2, TILE_SIZE / 2}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);  //right arm
        initCSprite(&playerSprites[4], playerTexture, 5, (SDL_Rect) {TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE}, (SDL_Rect) {0, TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);  //legs
        initCSprite(&playerSprites[5], playerTexture, 6, (SDL_Rect) {TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (SDL_Rect) {TILE_SIZE, TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, &((SDL_Point) {TILE_SIZE / 4, TILE_SIZE / 4}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);  //left foot
        initCSprite(&playerSprites[6], playerTexture, 7, (SDL_Rect) {1.5 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (SDL_Rect) {1.5 * TILE_SIZE, TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, &((SDL_Point) {TILE_SIZE / 4, TILE_SIZE / 4}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);  //right foot
        initC2DModel(&playerModel, playerSprites, 7, (SDL_Point) {TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, &thisPlayer, 1);
    }
    c2DModel mapModel;
    {
        SDL_Texture* tilesetTexture;
        loadIMG("assets/tilesheet.png", &tilesetTexture);
        cSprite* tileSprites = calloc(windowW / TILE_SIZE * windowH / TILE_SIZE, sizeof(cSprite));
        for(int x = 0; x < windowW / TILE_SIZE; x++)
        {
            for(int y = 0; y < windowH / TILE_SIZE; y++)
            {
                initCSprite(&tileSprites[x * windowH / TILE_SIZE + y], tilesetTexture, tilemap[x][y], (SDL_Rect) {TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE}, (SDL_Rect) {(tilemap[x][y] / 32) * TILE_SIZE, (tilemap[x][y] % 32) * TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
            }
        }
        initC2DModel(&mapModel, tileSprites, windowW / TILE_SIZE * windowH / TILE_SIZE, (SDL_Point) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
    }
    cText FPStext;
    cText versionText;
    char FPSstring[3] = "   ";
    initCText(&FPStext, "0", (SDL_Rect) {windowW - 3 * TILE_SIZE, 0, 3 * TILE_SIZE, TILE_SIZE}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, SDL_FLIP_NONE, 0.0, true, 0);
    initCText(&versionText, COSPRITE_VERSION, (SDL_Rect){0, 0, 150, 50}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, SDL_FLIP_NONE, 0.0, true, 5);
    cCamera testCamera;
    initCCamera(&testCamera, (SDL_Rect) {0, 0, 20, 10}, 1.0, 0.0);
    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, (cSprite*[1]) {&mouseSprite}, 1, (c2DModel*[2]) {&playerModel, &mapModel}, 2, NULL, 0, (cText*[2]) {&versionText, &FPStext}, 2);
    player* playerSubclass = (player*) playerModel.subclass;
    SDL_Event e;
    bool quit = false;
    int startTime = SDL_GetTicks(), lastFrame = startTime;
    while(!quit)
    {
        while(SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;

            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                //loadIMG("assets/cb1.bmp", &mouseSprite.texture); // you can load new images on the fly and they'll be automatically used next frame
                mouseSprite.degrees = 180.0;  //or just change the rotation

                SDL_SetTextureColorMod(mouseSprite.texture, 0x80, 0x00, 0x00);
                SDL_SetTextureAlphaMod(mouseSprite.texture, 0x80);
            }
            if (e.type == SDL_MOUSEBUTTONUP)
            {
                //loadIMG("assets/cb.bmp", &mouseSprite.texture);
                mouseSprite.degrees = 0.0;

                SDL_SetTextureColorMod(mouseSprite.texture, 0xFF, 0xFF, 0xFF);
                SDL_SetTextureAlphaMod(mouseSprite.texture, 0xFF);

                int newX = e.button.x - (playerModel.rect.w / 2);// + (testCamera.rect.x * windowW / testCamera.rect.w);
                int newY = e.button.y - (playerModel.rect.h / 2);// + (testCamera.rect.y * windowH / testCamera.rect.h);
                if (getDistance(playerModel.rect.x, playerModel.rect.y, newX, newY) > range)
                {
                    double angle = atan((double) (newY - playerModel.rect.y) / (newX - playerModel.rect.x));
                    playerModel.rect.x += range * cos(angle) * (1 - 2 * (newX - playerModel.rect.x < 0));
                    playerModel.rect.y += range * sin(angle) * (1 - 2 * (newX - playerModel.rect.x < 0));  //remember, bounds of inverse tan
                }
                else
                {
                    playerModel.rect.x = newX;
                    playerModel.rect.y = newY;
                }
            }
            if (e.type == SDL_MOUSEMOTION)
            {
                mouseSprite.drawRect.x = e.motion.x - (mouseSprite.drawRect.w / 2)/* + (testCamera.rect.x * windowW / testCamera.rect.w)*/;
                mouseSprite.drawRect.y = e.motion.y - (mouseSprite.drawRect.h / 2)/* + (testCamera.rect.y * windowH / testCamera.rect.h)*/;
            }
        }

        const Uint8* keyStates = SDL_GetKeyboardState(NULL);
        //mouseSprite.drawRect.x -= (testCamera.rect.x * windowW / testCamera.rect.w);  //subtract out camera offset
        //mouseSprite.drawRect.y -= (testCamera.rect.y * windowH / testCamera.rect.h);

        if (keyStates[SDL_SCANCODE_ESCAPE])
            quit = true;

        if (keyStates[SDL_SCANCODE_F1])
            printf("%d, %d x %d/%d [%d, %d]\n", playerModel.rect.x, playerModel.rect.y, playerModel.rect.w, playerModel.rect.h, windowW, windowH);

        if (keyStates[SDL_SCANCODE_F12])
            FPStext.drawPriority = 5;
        else
            FPStext.drawPriority = 0;

        if (keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_A] ||
            keyStates[SDL_SCANCODE_S] || keyStates[SDL_SCANCODE_D] ||
            playerSubclass->walkFrame % 20 > 0)
        {
            int previousX = playerModel.rect.x;
            int previousY = playerModel.rect.y;

            if (keyStates[SDL_SCANCODE_W])
                playerModel.rect.y -= 6;

            if (keyStates[SDL_SCANCODE_A])
            {
                playerModel.rect.x -= 6;
                playerModel.flip = SDL_FLIP_HORIZONTAL;
            }
            else
                playerModel.flip = SDL_FLIP_NONE;

            if (keyStates[SDL_SCANCODE_S])
                playerModel.rect.y += 6;

            if (keyStates[SDL_SCANCODE_D])
                playerModel.rect.x += 6;

            //printf("%d\n", playerSubclass->walkFrame % 20);
            if (playerSubclass->walkFrame % 20 > 0 || previousX != playerModel.rect.x || previousY != playerModel.rect.y)
                playerSubclass->walkFrame = (playerSubclass->walkFrame + 1) % 40;

            playerModel.sprites[2].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 < 11)) * armRotations[(playerSubclass->walkFrame / 2) % 10];
            playerModel.sprites[3].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 > 10)) * armRotations[(playerSubclass->walkFrame / 2) % 10];

            playerModel.sprites[5].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 < 11)) * legRotations[(playerSubclass->walkFrame / 2) % 10];
            playerModel.sprites[6].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 > 10)) * legRotations[(playerSubclass->walkFrame / 2) % 10];
        }

        if (keyStates[SDL_SCANCODE_UP])
                testCamera.rect.y--;

        if (keyStates[SDL_SCANCODE_LEFT])
            testCamera.rect.x--;

        if (keyStates[SDL_SCANCODE_DOWN])
            testCamera.rect.y++;

        if (keyStates[SDL_SCANCODE_RIGHT])
            testCamera.rect.x++;

        if (keyStates[SDL_SCANCODE_Q])  //camera rotation won't be controllable in final game obviously
            testCamera.degrees -= 5;

        if (keyStates[SDL_SCANCODE_E])
            testCamera.degrees += 5;

        if (keyStates[SDL_SCANCODE_Z])
            playerModel.degrees -= 5;

        if (keyStates[SDL_SCANCODE_C])
            playerModel.degrees += 5;

        frame++;
        //if ((SDL_GetTicks() - startTime) % 250 == 0)
        framerate = (int) (frame * 1000.0 / (SDL_GetTicks() - startTime));  //multiplied by 1000 on both sides since 1000f / ms == 1f / s
                snprintf(FPSstring, 3, "%d", framerate);
        initCText(&FPStext, FPSstring, (SDL_Rect) {windowW - 3 * TILE_SIZE, 0, 3 * TILE_SIZE, TILE_SIZE}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, SDL_FLIP_NONE, 0.0, true, FPStext.drawPriority);

        if ((sleepFor = targetTime - (SDL_GetTicks() - lastFrame)) > 0)
            SDL_Delay(sleepFor);  //FPS limiter; rests for (16 - time spent) ms per frame, effectively making each frame run for ~16 ms, or 60 FPS
        lastFrame = SDL_GetTicks();


        //mouseSprite.drawRect.x += (testCamera.rect.x * windowW / testCamera.rect.w);  //add back camera offset
        //mouseSprite.drawRect.y += (testCamera.rect.y * windowH / testCamera.rect.h);
        drawCScene(&testScene, false);
        SDL_SetRenderDrawColor(mainRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderDrawLine(mainRenderer, playerModel.rect.x + playerModel.rect.w / 2, playerModel.rect.y + playerModel.rect.h / 2, mouseSprite.drawRect.x + mouseSprite.drawRect.w / 2, mouseSprite.drawRect.y + mouseSprite.drawRect.h / 2);
        SDL_RenderPresent(mainRenderer);
    }

    destroyCScene(&testScene);
    closeCoSprite();
    return error;
}


player initPlayer(int maxHealth)
{
    player inittedPlayer;
    for(int i = 0; i < MAX_SKILLS; i++)
        inittedPlayer.skills[i] = 0;
    inittedPlayer.walkFrame = 0;
    inittedPlayer.HP = maxHealth;
    inittedPlayer.maxHP = maxHealth;
    return inittedPlayer;
}
