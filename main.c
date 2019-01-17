//#include "csMain.h"
#include "csGraphics.h"
#include "csIO.h"
#include "csUtility.h"

#define MAX_SKILLS 10
typedef struct _player {
    int walkFrame;
    int HP;
    int maxHP;
    int xVeloc;
    int yVeloc;
    int skills[MAX_SKILLS];
    bool grounded;
} player;

typedef struct _spFX {
    int* fxTimers;
    bool* fxTimerOn;
    int numFX;
} spFX;

typedef struct _collisionResult {
    cDoubleVector* mtvs;
    int* tilesCollided;
    int collisions;
} collisionResult;

#define calcWaitTime(x) x == 0 ? 0 : 1000 / x

#define TILEMAP_X 60  //(global.windowW / TILE_SIZE)
#define TILEMAP_Y 30  //(global.windowH / TILE_SIZE)

player initPlayer(int maxHealth);
spFX initSPFX(int numFX);
void startSPFXTimer(spFX* FX, int index, int frames);
void pauseResSPFXTimer(spFX* FX, int index, bool pause);
void stopSPFXTimer(spFX* FX, int index);
void freeSPFX(spFX* FX);
int checkTilemapCollision(collisionResult* result, c2DModel playerModel, c2DModel tilemapModel, int playerSprite, int airID);

const int upperArmRotations[10] = {0, 10, 20, 25, 28, 30, 28, 25, 21, 9};
const int lowerArmRotations[20] = {0, 20, 35, 52, 60, 70, 57, 40, 25, 11, 0, -14, -18, -25, -34, -45, -30, -21, -16, -6};
const int legRotations[10] = {0, -14, -24, -26, -29, -33, -28, -23, -18, -8};
const int footRotations[20] = {0, -14, -27, -41, -55, -63, -51, -36, -23, -12, 0, 14, 24, 26, 29, 33, 28, 23, 18, 8};

int main(int argc, char* argv[])
{
    if (argc)
        argv = argv;
    const int TILE_SIZE = 32;
    int range = 7 * TILE_SIZE;  //10 * TILE_SIZE was a good range
    int error = initCoSprite("assets/cb.bmp", "Warper", 1280, 640, "assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE, (SDL_Color) {255, 28, 198, 0xFF});
    int tilemap[TILEMAP_X][TILEMAP_Y];

    for(int x = 0; x < TILEMAP_X; x++)
    {
        for(int y = 0; y < TILEMAP_Y; y++)
        {
            if (x == 0 || y == 0 || x + 1 == TILEMAP_X || y + 1 == TILEMAP_Y || y == 7)
                tilemap[x][y] = 1;
            else
                tilemap[x][y] = 0;
        }
    }
    int frame = 0, framerate = 0, targetTime = calcWaitTime(0), sleepFor = 0;
    cSprite mouseSprite;
    c2DModel playerModel, spFXModel;
    {
        SDL_Texture* playerTexture;
        loadIMG("assets/tilesheet.png", &playerTexture);
        player thisPlayer = initPlayer(10);
        spFX theseFX = initSPFX(1);
        cSprite playerSprites[14];
        cSprite spFXSprites[1];
        initCSprite(&mouseSprite, playerTexture, "assets/tileset.bmp", 0, (cDoubleRect) {0, 0, TILE_SIZE, TILE_SIZE}, (cDoubleRect) {TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, true, NULL, 1);
        initCSprite(&spFXSprites[0], playerTexture, "assets/tileset.png", 0, (cDoubleRect) {0, 0, 2 * TILE_SIZE, 2 * TILE_SIZE}, (cDoubleRect) {5 * TILE_SIZE, 0, 2 * TILE_SIZE, 2 * TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 0);  //teleport explosion
        initCSprite(&playerSprites[0], playerTexture, "assets/tilesheet.png", 1, (cDoubleRect) {0.5 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE}, (cDoubleRect) {TILE_SIZE, 0, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2); //head
        initCSprite(&playerSprites[1], playerTexture, "assets/tilesheet.png", 2, (cDoubleRect) {0.5 * TILE_SIZE, TILE_SIZE, TILE_SIZE, 2 * TILE_SIZE}, (cDoubleRect) {2 * TILE_SIZE, 0, TILE_SIZE, 2 * TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3); //torso
        initCSprite(&playerSprites[2], playerTexture, "assets/tilesheet.png", 3, (cDoubleRect) {0, TILE_SIZE, TILE_SIZE / 2, 1.25 * TILE_SIZE}, (cDoubleRect) {3 * TILE_SIZE, 0, TILE_SIZE / 2, 1.25 * TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / 2}), 1.0, SDL_FLIP_HORIZONTAL, 0.0, false, NULL, 1);  //upper left arm
        initCSprite(&playerSprites[3], playerTexture, "assets/tilesheet.png", 4, (cDoubleRect) {1.5 * TILE_SIZE, TILE_SIZE, TILE_SIZE / 2, 1.25 * TILE_SIZE}, (cDoubleRect) {3 * TILE_SIZE, 0, TILE_SIZE / 2, 1.25 * TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / 2}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 4);  //upper right arm
        initCSprite(&playerSprites[4], playerTexture, "assets/tilesheet.png", 5, (cDoubleRect) {0, 2.25 * TILE_SIZE, TILE_SIZE / 2, 1.25 * TILE_SIZE}, (cDoubleRect) {3 * TILE_SIZE, 1.25 * TILE_SIZE, TILE_SIZE / 2, 1.25 * TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / -2}), 1.0, SDL_FLIP_HORIZONTAL, 0.0, false, NULL, 1);  //lower left arm
        initCSprite(&playerSprites[5], playerTexture, "assets/tilesheet.png", 6, (cDoubleRect) {1.5 * TILE_SIZE, 2.25 * TILE_SIZE, TILE_SIZE / 2, 1.25 * TILE_SIZE}, (cDoubleRect) {3 * TILE_SIZE, 1.25 * TILE_SIZE, TILE_SIZE / 2, 1.25 * TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / -2}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 4);  //lower right arm
        initCSprite(&playerSprites[6], playerTexture, "assets/tilesheet.png", 7, (cDoubleRect) {0.5 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (cDoubleRect) {3.5 * TILE_SIZE, 0, TILE_SIZE / 2, TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / 4}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);  //left leg
        initCSprite(&playerSprites[7], playerTexture, "assets/tilesheet.png", 8, (cDoubleRect) {TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (cDoubleRect) {4 * TILE_SIZE, 0, TILE_SIZE / 2, TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / 4}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 4);  //right leg
        initCSprite(&playerSprites[8], playerTexture, "assets/tilesheet.png", 9, (cDoubleRect) {0.5 * TILE_SIZE, 4 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (cDoubleRect) {3.5 * TILE_SIZE, TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / -2}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);  //left foot
        initCSprite(&playerSprites[9], playerTexture, "assets/tilesheet.png", 10, (cDoubleRect) {TILE_SIZE, 4 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (cDoubleRect) {4 * TILE_SIZE, TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, &((cDoublePt) {TILE_SIZE / 4, TILE_SIZE / -2}), 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 4);  //right foot
        initCSprite(&playerSprites[10], playerTexture, "assets/tilesheet.png", 11, (cDoubleRect) {TILE_SIZE / 2, 0, TILE_SIZE, 5 * TILE_SIZE}, (cDoubleRect) {0, TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 0);  //floor hurtbox
        initCSprite(&playerSprites[11], playerTexture, "assets/tilesheet.png", 12, (cDoubleRect) {0, 0, 2 * TILE_SIZE, 4 * TILE_SIZE}, (cDoubleRect) {0, TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 0);  //wall/ceiling hurtbox
        initCSprite(&playerSprites[12], playerTexture, "assets/tilesheet.png", 13, (cDoubleRect) {0, 0, 2 * TILE_SIZE, 5 * TILE_SIZE}, (cDoubleRect) {0, TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 0);  //dmg hurtbox
        initCSprite(&playerSprites[13], playerTexture, "assets/tilesheet.png", 14, (cDoubleRect) {TILE_SIZE, 4 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (cDoubleRect) {0, TILE_SIZE, TILE_SIZE, TILE_SIZE}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 0);  //hitbox

        initC2DModel(&playerModel, playerSprites, 14, (cDoublePt) {4 * TILE_SIZE, 4 * TILE_SIZE}, NULL, 0.75, SDL_FLIP_NONE, 0.0, false, &thisPlayer, 2);
        initC2DModel(&spFXModel, spFXSprites, 1, (cDoublePt) {0, 0}, NULL, 0.75, SDL_FLIP_NONE, 0.0, false, &theseFX, 1);
    }
    c2DModel mapModel;
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
    }
    cText FPStext;
    cText versionText;
    char FPSstring[3] = "   ";
    initCText(&FPStext, "0", (cDoubleRect) {global.windowW - 3 * TILE_SIZE, 0, 3 * TILE_SIZE, TILE_SIZE}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, SDL_FLIP_NONE, 0.0, true, 0);
    initCText(&versionText, COSPRITE_VERSION, (cDoubleRect) {0, 0, 150, 50}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, SDL_FLIP_NONE, 0.0, true, 5);
    cCamera testCamera;
    initCCamera(&testCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0.0);
    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, (cSprite*[1]) {&mouseSprite}, 1, (c2DModel*[3]) {&playerModel, &spFXModel, &mapModel}, 3, NULL, 0, (cText*[2]) {&versionText, &FPStext}, 2);
    player* playerSubclass = (player*) playerModel.subclass;
    spFX* specialFX = (spFX*) spFXModel.subclass;
    SDL_Event e;
    bool quit = false;
    int startTime = SDL_GetTicks(), lastFrame = startTime;
    int playerFlip = -1;
    bool walkBypass = false;
    while(!quit)
    {
        double previousX = playerModel.rect.x;
        double previousY = playerModel.rect.y;
        while(SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;

            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                //loadIMG("assets/cb1.bmp", &mouseSprite.texture); // you can load new images on the fly and they'll be automatically used next frame
                //mouseSprite.degrees = 180.0;  //or just change the rotation

                //SDL_SetTextureColorMod(mouseSprite.texture, 0x80, 0x00, 0x00);
                SDL_SetTextureAlphaMod(mouseSprite.texture, 0x80);
            }
            if (e.type == SDL_MOUSEBUTTONUP)
            {
                //loadIMG("assets/cb.bmp", &mouseSprite.texture);
                //mouseSprite.degrees = 0.0;
                walkBypass = true;
                playerSubclass->walkFrame = 0;

                //SDL_SetTextureColorMod(mouseSprite.texture, 0xFF, 0xFF, 0xFF);
                SDL_SetTextureAlphaMod(mouseSprite.texture, 0xFF);

                int newX = (e.button.x + (testCamera.rect.x * global.windowW / testCamera.rect.w)) / testCamera.scale - (playerModel.rect.w * playerModel.scale) / 2;
                int newY = (e.button.y + (testCamera.rect.y * global.windowH / testCamera.rect.h)) / testCamera.scale - (playerModel.rect.h * playerModel.scale) / 2;
                if (getDistance(playerModel.rect.x * playerModel.scale, playerModel.rect.y * playerModel.scale, newX, newY) > range)
                {
                    double angle = atan2((double) (newY - playerModel.rect.y * playerModel.scale), (newX - playerModel.rect.x * playerModel.scale));
                    playerModel.rect.x += range * cos(angle)/* * (1 - 2 * (newX - playerModel.rect.x * playerModel.scale < 0))*/;
                    playerModel.rect.y += range * sin(angle)/* * (1 - 2 * (newX - playerModel.rect.x * playerModel.scale < 0))*/;  //remember, bounds of atan() require this
                }
                else
                {
                    playerModel.rect.x = ((newX * 6) / 6) / playerModel.scale;
                    playerModel.rect.y = ((newY * 6) / 6) / playerModel.scale;
                }
                collisionResult result;
                if (checkTilemapCollision(&result, playerModel, mapModel, 12, 0))
                {
                    playerModel.rect.x = previousX;
                    playerModel.rect.y = previousY;
                }
                else
                {
                    spFXModel.sprites[0].drawPriority = 1;
                    spFXModel.sprites[0].drawRect.x = previousX;
                    spFXModel.sprites[0].drawRect.y = previousY + TILE_SIZE;
                    startSPFXTimer(specialFX, 0, 12); //turn on a timer and display for 12 frames
                }
                if (playerModel.rect.x > previousX)
                    playerFlip = SDL_FLIP_NONE;
                if (playerModel.rect.x < previousX)
                    playerFlip = SDL_FLIP_HORIZONTAL;
            }
            if (e.type == SDL_MOUSEMOTION)
            {
                mouseSprite.drawRect.x = e.motion.x - (mouseSprite.drawRect.w / 2)/* + (testCamera.rect.x * global.windowW / testCamera.rect.w)*/;
                mouseSprite.drawRect.y = e.motion.y - (mouseSprite.drawRect.h / 2)/* + (testCamera.rect.y * global.windowH / testCamera.rect.h)*/;
            }
        }

        const Uint8* keyStates = SDL_GetKeyboardState(NULL);
        //mouseSprite.drawRect.x -= (testCamera.rect.x * global.windowW / testCamera.rect.w);  //subtract out camera offset
        //mouseSprite.drawRect.y -= (testCamera.rect.y * global.windowH / testCamera.rect.h);

        if (keyStates[SDL_SCANCODE_ESCAPE])
            quit = true;

        if (keyStates[SDL_SCANCODE_F1])
            printf("%.0f, %.0f x %.0f/%.0f [%d, %d]\n", playerModel.rect.x, playerModel.rect.y, playerModel.rect.w, playerModel.rect.h, global.windowW, global.windowH);

        if (keyStates[SDL_SCANCODE_F12])
            FPStext.drawPriority = 5;
        else
            FPStext.drawPriority = 0;

        /*cDoubleVector translation = checkCSpriteCollision(mouseSprite, testSprite);  //debugging checkCSpriteCollision()
        if (keyStates[SDL_SCANCODE_G])
        {
            mouseSprite.drawRect.x += translation.magnitude * cos(degToRad(translation.degrees));
            mouseSprite.drawRect.y += translation.magnitude * sin(degToRad(translation.degrees));
        }*/

        if (keyStates[SDL_SCANCODE_F2])  //testing
        {
            playerModel.degrees = 0;
            testCamera.degrees = 0;
            testCamera.rect.x = 0;
            testCamera.rect.y = 0;
            testCamera.scale = 1.0;
        }

        if (keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_A] ||
            keyStates[SDL_SCANCODE_S] || keyStates[SDL_SCANCODE_D])
        {

            if (keyStates[SDL_SCANCODE_W])  //jumping will not be in the final game
            {
                playerSubclass->yVeloc = -14;
            }

            if (keyStates[SDL_SCANCODE_A])
            {
                playerSubclass->xVeloc -= 6;
                playerFlip = SDL_FLIP_HORIZONTAL;
                //playerModel.flip = SDL_FLIP_HORIZONTAL;
            }

            if (playerModel.flip == SDL_FLIP_NONE)
            {

                playerModel.sprites[2].drawPriority = 1;
                playerModel.sprites[3].drawPriority = 4;  //upper arm priority
                playerModel.sprites[4].drawPriority = 1;
                playerModel.sprites[5].drawPriority = 4;  //lower arm priority

                playerModel.sprites[6].drawPriority = 2;
                playerModel.sprites[7].drawPriority = 4;  //leg priority
                playerModel.sprites[8].drawPriority = 2;
                playerModel.sprites[9].drawPriority = 4;  //foot priority
            }
            else
            {
                playerModel.sprites[2].drawPriority = 4;
                playerModel.sprites[3].drawPriority = 1;  //upper arm priority
                playerModel.sprites[4].drawPriority = 4;
                playerModel.sprites[5].drawPriority = 1;  //lower arm priority

                playerModel.sprites[6].drawPriority = 4;
                playerModel.sprites[7].drawPriority = 2;  //leg priority
                playerModel.sprites[8].drawPriority = 4;
                playerModel.sprites[9].drawPriority = 2;  //foot priority
            }

            if (keyStates[SDL_SCANCODE_S])
                playerSubclass->yVeloc += 6;

            if (keyStates[SDL_SCANCODE_D])
            {
                playerSubclass->xVeloc += 6;
                playerFlip = SDL_FLIP_NONE;
                //playerModel.flip = SDL_FLIP_NONE;
            }

            //printf("%d\n", playerSubclass->walkFrame % 20);
        }


        collisionResult result;
        if (!checkTilemapCollision(&result, playerModel, mapModel, 10, 0))
        {
            playerSubclass->yVeloc += 8;
            if (playerSubclass->yVeloc > 48)
                playerSubclass->yVeloc = 48;
            playerSubclass->grounded = false;
        }
        if (!playerSubclass->grounded)
        {
            for(int i = 0; i < result.collisions; i++)
            {
                playerModel.rect.x += result.mtvs[i].magnitude * cos(degToRad(result.mtvs[i].degrees));
                playerModel.rect.y += result.mtvs[i].magnitude * sin(degToRad(result.mtvs[i].degrees));
            }
            playerSubclass->grounded = true;
            free(result.mtvs);
            free(result.tilesCollided);
        }
        else
            walkBypass = false;

        if (checkTilemapCollision(&result, playerModel, mapModel, 11, 0))
        {
            double totalX = 0, totalY = 0;
            for(int i = 0; i < result.collisions; i++)
            {
                totalX += result.mtvs[i].magnitude * cos(degToRad(result.mtvs[i].degrees));
                totalY += result.mtvs[i].magnitude * sin(degToRad(result.mtvs[i].degrees));
            }
            playerModel.rect.x += totalX / result.collisions;
            playerModel.rect.y += totalY / result.collisions;
            int angle = abs(radToDeg(atan2(totalY / result.collisions, totalX / result.collisions)));

            /*
            if (playerSubclass->yVeloc && playerSubclass->xVeloc)
                printf("%d\n", angle);//*/

            if (angle % 180 >= 45 && angle % 180 <= 135)
            {
                playerSubclass->yVeloc = 1;
            }
            if (playerSubclass->yVeloc && playerSubclass->xVeloc)
            {
                playerSubclass->xVeloc = 0;
            }
            free(result.mtvs);
            free(result.tilesCollided);
        }

        if (playerSubclass->xVeloc)
        {
            playerModel.rect.x += playerSubclass->xVeloc;
            bool xVelocPos = (playerSubclass->xVeloc > 0);
            playerSubclass->xVeloc -= 6 - 12 * (playerSubclass->xVeloc < 0);
            if (playerSubclass->xVeloc * (xVelocPos - 2) < 0)
            {
                playerSubclass->xVeloc = 0;
            }

        }
        if (playerSubclass->yVeloc)
        {
            playerModel.rect.y += playerSubclass->yVeloc;
            bool yVelocPos = (playerSubclass->yVeloc > 0);
            playerSubclass->yVeloc -= 6 - 12 * (playerSubclass->yVeloc < 0);
            if (playerSubclass->yVeloc * (yVelocPos - 2) < 0)
            {
                playerSubclass->yVeloc = 0;
            }
        }
        if ((playerSubclass->walkFrame % 20 > 0 || previousX != playerModel.rect.x || previousY != playerModel.rect.y) && !walkBypass)
            playerSubclass->walkFrame = (playerSubclass->walkFrame + 1) % 40;

        playerModel.sprites[2].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 < 11)) * upperArmRotations[(playerSubclass->walkFrame / 2) % 10];
        playerModel.sprites[3].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 > 10)) * upperArmRotations[(playerSubclass->walkFrame / 2) % 10];

        playerModel.sprites[4].degrees = (1 - 2 * (playerModel.flip == SDL_FLIP_NONE)) * lowerArmRotations[((10 * (playerModel.sprites[4].drawPriority == 4)) + playerSubclass->walkFrame / 2) % 20];
        playerModel.sprites[5].degrees = (1 - 2 * (playerModel.flip == SDL_FLIP_NONE)) * lowerArmRotations[((10 * (playerModel.sprites[5].drawPriority == 4)) + playerSubclass->walkFrame / 2) % 20];

        playerModel.sprites[6].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 < 11)) * legRotations[(playerSubclass->walkFrame / 2) % 10];
        playerModel.sprites[7].degrees = (1 - 2 * (playerSubclass->walkFrame / 2 > 10)) * legRotations[(playerSubclass->walkFrame / 2) % 10];

        playerModel.sprites[8].degrees = (1 - 2 * (playerModel.flip == SDL_FLIP_NONE)) * footRotations[((10 * (playerModel.sprites[8].drawPriority == 4)) + playerSubclass->walkFrame / 2) % 20];
        playerModel.sprites[9].degrees = (1 - 2 * (playerModel.flip == SDL_FLIP_NONE)) * footRotations[((10 * (playerModel.sprites[9].drawPriority == 4)) + playerSubclass->walkFrame / 2) % 20];

        if (playerModel.rect.y * playerModel.scale - (playerModel.rect.h * playerModel.scale) / 8 * playerModel.scale < testCamera.rect.y * global.windowH / testCamera.rect.h / testCamera.scale)
            testCamera.rect.y -= testCamera.rect.h / 4;

        if (playerModel.rect.x * playerModel.scale - (playerModel.rect.w * playerModel.scale) / 2  < testCamera.rect.x * global.windowW / testCamera.rect.w / testCamera.scale)
            testCamera.rect.x -= testCamera.rect.w / 4;

        if ((playerModel.rect.y + playerModel.rect.h) * playerModel.scale > (testCamera.rect.y + testCamera.rect.h) * global.windowH / testCamera.rect.h / testCamera.scale)
            testCamera.rect.y += testCamera.rect.h / 4;

        if ((playerModel.rect.x + playerModel.rect.w * 1.5) * playerModel.scale > (testCamera.rect.x + testCamera.rect.w) * global.windowW / testCamera.rect.w / testCamera.scale)
            testCamera.rect.x += testCamera.rect.w / 4;  //later, introduce a screen scrolling var that gets set here instead


        if (keyStates[SDL_SCANCODE_Q])
        {  //punch
            //mouseSprite.degrees -= 5;
        }


        if (keyStates[SDL_SCANCODE_E])
        {  //kick
            //mouseSprite.degrees += 5;
        }

        if (keyStates[SDL_SCANCODE_X])  //camera rotation won't be controllable in final game obviously
            testCamera.degrees -= 5;

        if (keyStates[SDL_SCANCODE_V])
            testCamera.degrees += 5;

        if (keyStates[SDL_SCANCODE_Z])
            playerModel.degrees -= 5;

        if (keyStates[SDL_SCANCODE_C])
            playerModel.degrees += 5;

        if (keyStates[SDL_SCANCODE_MINUS])
            testCamera.scale -= .05;

        if (keyStates[SDL_SCANCODE_EQUALS])
            testCamera.scale += .05;

        frame++;
        //if ((SDL_GetTicks() - startTime) % 250 == 0)
        framerate = (int) (frame * 1000.0 / (SDL_GetTicks() - startTime));  //multiplied by 1000 on both sides since 1000f / ms == 1f / s
                snprintf(FPSstring, 3, "%d", framerate);
        initCText(&FPStext, FPSstring, (cDoubleRect) {global.windowW - 3 * TILE_SIZE, 0, 3 * TILE_SIZE, TILE_SIZE}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, SDL_FLIP_NONE, 0.0, true, FPStext.drawPriority);

        if ((sleepFor = targetTime - (SDL_GetTicks() - lastFrame)) > 0)
            SDL_Delay(sleepFor);  //FPS limiter; rests for (16 - time spent) ms per frame, effectively making each frame run for ~16 ms, or 60 FPS
        lastFrame = SDL_GetTicks();

        for(int i = 0; i < specialFX->numFX; i++)
        {
            if (specialFX->fxTimerOn[i])
            {
                specialFX->fxTimers[i]--;
                if (specialFX->fxTimers[i] == 0)
                {
                    spFXModel.sprites[i].drawPriority = 0;
                    stopSPFXTimer(specialFX, 0);
                }
            }
        }
        drawCScene(&testScene, true, true);
        if (keyStates[SDL_SCANCODE_P])
            waitForKey(false);
        if (playerFlip != -1)
        {  //delays player model from flipping, eliminating visual glitch with rapidly flipping while in walking animation
            playerModel.flip = playerFlip;
            playerFlip = -1;
        }
        /*SDL_SetRenderDrawColor(mainRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderDrawLine(mainRenderer, playerModel.rect.x + playerModel.rect.w / 2, playerModel.rect.y + playerModel.rect.h / 2, mouseSprite.drawRect.x + mouseSprite.drawRect.w / 2, mouseSprite.drawRect.y + mouseSprite.drawRect.h / 2);
        SDL_RenderPresent(mainRenderer);*/
    }

    destroyCScene(&testScene);
    freeSPFX(specialFX);
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
    inittedPlayer.xVeloc = 0;
    inittedPlayer.yVeloc = 0;
    inittedPlayer.grounded = false;
    return inittedPlayer;
}

spFX initSPFX(int numFX)
{
    spFX inittedSPFX;
    inittedSPFX.numFX = numFX;
    inittedSPFX.fxTimers = calloc(numFX, sizeof(int));
    inittedSPFX.fxTimerOn = calloc(numFX, sizeof(int));
    return inittedSPFX;
}

void startSPFXTimer(spFX* FX, int index, int frames)
{
    FX->fxTimers[index] = frames;
    FX->fxTimerOn[index] = true;

}

void pauseResSPFXTimer(spFX* FX, int index, bool pause)
{
    FX->fxTimerOn[index] = !pause;
}

void stopSPFXTimer(spFX* FX, int index)
{
    FX->fxTimerOn[index] = false;
    FX->fxTimers[index] = 0;
}

void freeSPFX(spFX* FX)
{
    FX->numFX = 0;
    free(FX->fxTimers);
    FX->fxTimers = NULL;
    free(FX->fxTimerOn);
    FX->fxTimerOn = NULL;
}

int checkTilemapCollision(collisionResult* result, c2DModel playerModel, c2DModel tilemapModel, int playerSprite, int airID)
{
    cSprite sprite1 = playerModel.sprites[playerSprite];
    sprite1.drawRect.x = ((sprite1.drawRect.x * sprite1.scale) + (playerModel.rect.x * playerModel.scale)) / sprite1.scale;  //divide by scale?
    sprite1.drawRect.y = ((sprite1.drawRect.y * sprite1.scale) + (playerModel.rect.y * playerModel.scale)) / sprite1.scale;  //because it's just multiplied right back in, right?
    sprite1.drawRect.w *= sprite1.scale * playerModel.scale;
    sprite1.drawRect.h *= sprite1.scale * playerModel.scale;
    sprite1.degrees += playerModel.degrees;
    cSprite sprite2;
    result->mtvs = calloc(tilemapModel.numSprites, sizeof(cDoubleVector));
    result->tilesCollided = calloc(tilemapModel.numSprites, sizeof(int));
    result->collisions = 0;
    for(int i = 0; i < tilemapModel.numSprites; i++)
    {
        sprite2 = tilemapModel.sprites[i];
        cDoubleVector collisionV = checkCSpriteCollision(sprite1, sprite2);
        if (sprite2.id != airID && collisionV.magnitude)
        {
            if (collisionV.magnitude)
            {
                result->mtvs[result->collisions] = collisionV;
                result->tilesCollided[result->collisions++] = i;
                /*
                SDL_SetRenderDrawColor(global.mainRenderer, 0x00, 0x00, 0xFF, 0xFF);
                SDL_RenderDrawLine(global.mainRenderer, playerModel.rect.x + playerModel.sprites[playerSprite].drawRect.x, playerModel.rect.y + playerModel.sprites[playerSprite].drawRect.y, playerModel.rect.x + playerModel.sprites[playerSprite].drawRect.x + collisionV.magnitude * cos(degToRad(collisionV.degrees)), playerModel.rect.y + playerModel.sprites[playerSprite].drawRect.y + collisionV.magnitude * sin(degToRad(collisionV.degrees)));
                SDL_SetRenderDrawColor(global.mainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderPresent(global.mainRenderer);
                //*/
            }
        }
    }
    return result->collisions;
}
