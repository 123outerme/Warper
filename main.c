//#include "csMain.h"
#include "csGraphics.h"
#include "csFile.h"
#include "csIO.h"
#include "csUtility.h"

int main(int argc, char* argv[])
{
    const int TILE_SIZE = 32;
    int error = initCoSprite("", "Warper", 1280, 640, "assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE);
    SDL_Event e;
    bool quit = false;
    SDL_Texture* mouseTexture, * playerTexture;
    loadIMG("assets/cb.bmp", &mouseTexture);
    loadIMG("assets/tilesheet.png", &playerTexture);
    cSprite mouseSprite;
    c2DModel playerModel;
    {
        cSprite playerSprites[7];
        initCSprite(&mouseSprite, mouseTexture, 0, (SDL_Rect) {0, 0, 80, 80}, (SDL_Rect) {15, 0, 120, 120}, 1.0, SDL_FLIP_NONE, 0.0, true, NULL, 1);
        initCSprite(&playerSprites[0], playerTexture, 1, (SDL_Rect) {TILE_SIZE, 0, TILE_SIZE, TILE_SIZE}, (SDL_Rect) {0, 0, TILE_SIZE, TILE_SIZE}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);
        initCSprite(&playerSprites[1], playerTexture, 2, (SDL_Rect) {TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE}, (SDL_Rect) {TILE_SIZE, 0, TILE_SIZE, TILE_SIZE}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);
        initCSprite(&playerSprites[2], playerTexture, 3, (SDL_Rect) {0, TILE_SIZE, TILE_SIZE, 2 * TILE_SIZE}, (SDL_Rect) {2 * TILE_SIZE, 0, TILE_SIZE, 2 * TILE_SIZE}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);
        initCSprite(&playerSprites[3], playerTexture, 4, (SDL_Rect) {2 * TILE_SIZE, TILE_SIZE, TILE_SIZE, 2 * TILE_SIZE}, (SDL_Rect) {2 * TILE_SIZE, 0, TILE_SIZE, 2 * TILE_SIZE}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);
        initCSprite(&playerSprites[4], playerTexture, 5, (SDL_Rect) {TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE}, (SDL_Rect) {0, TILE_SIZE, TILE_SIZE, TILE_SIZE}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);
        initCSprite(&playerSprites[5], playerTexture, 6, (SDL_Rect) {TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (SDL_Rect) {TILE_SIZE, TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);
        initCSprite(&playerSprites[6], playerTexture, 7, (SDL_Rect) {1.5 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, (SDL_Rect) {1.5 * TILE_SIZE, TILE_SIZE, TILE_SIZE / 2, TILE_SIZE}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 3);
        initC2DModel(&playerModel, playerSprites, 7, TILE_SIZE, TILE_SIZE, 3 * TILE_SIZE, 4 * TILE_SIZE, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 1);
    }
    cText versionText;
    initCText(&versionText, COSPRITE_VERSION, (SDL_Rect){0, 0, 150, 50}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, SDL_FLIP_NONE, 0.0, true, 5);
    cCamera testCamera;
    initCCamera(&testCamera, (SDL_Rect) {0, 0, 20, 10}, 1.0, 0.0);
    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, (cSprite*[1]) {&mouseSprite}, 1, (c2DModel*[1]) {&playerModel}, 1, NULL, 0, (cText*[1]) {&versionText}, 1);
    while(!quit)
    {
        while(SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN)
            {
                //mouseSprite.drawRect.x -= (testCamera.rect.x * windowW / testCamera.rect.w);  //subtract out camera offset
                //mouseSprite.drawRect.y -= (testCamera.rect.y * windowH / testCamera.rect.h);

                if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    quit = true;

                if (e.key.keysym.scancode == SDL_SCANCODE_F12)
                    printf("%d, %d x %d/%d [%d, %d]\n", playerModel.rect.x, playerModel.rect.y, playerModel.rect.w, playerModel.rect.h, windowW, windowH);

                if (e.key.keysym.scancode == SDL_SCANCODE_W)
                    playerModel.rect.y -= 6;

                if (e.key.keysym.scancode == SDL_SCANCODE_A)
                    playerModel.rect.x -= 6;

                if (e.key.keysym.scancode == SDL_SCANCODE_S)
                    playerModel.rect.y += 6;

                if (e.key.keysym.scancode == SDL_SCANCODE_D)
                    playerModel.rect.x += 6;


                if (e.key.keysym.scancode == SDL_SCANCODE_UP)
                    testCamera.rect.y--;

                if (e.key.keysym.scancode == SDL_SCANCODE_LEFT)
                    testCamera.rect.x--;

                if (e.key.keysym.scancode == SDL_SCANCODE_DOWN)
                    testCamera.rect.y++;

                if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                    testCamera.rect.x++;

                if (e.key.keysym.scancode == SDL_SCANCODE_Q)  //camera rotation won't be controllable in final game obviously
                    testCamera.degrees -= 5;

                if (e.key.keysym.scancode == SDL_SCANCODE_E)
                    testCamera.degrees += 5;

                //mouseSprite.drawRect.x += (testCamera.rect.x * windowW / testCamera.rect.w);  //add back camera offset
                //mouseSprite.drawRect.y += (testCamera.rect.y * windowH / testCamera.rect.h);

            }
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                //loadIMG("assets/cb1.bmp", &mouseSprite.texture); // you can load new images on the fly and they'll be automatically used next frame
                mouseSprite.degrees = 180.0;  //or just change the rotation
                playerModel.rect.x = e.button.x - (playerModel.rect.w / 2);// + (testCamera.rect.x * windowW / testCamera.rect.w);
                playerModel.rect.y = e.button.y - (playerModel.rect.h / 2);// + (testCamera.rect.y * windowH / testCamera.rect.h);

            }
            if (e.type == SDL_MOUSEBUTTONUP)
            {
                mouseSprite.degrees = 0.0;
                //loadIMG("assets/cb.bmp", &mouseSprite.texture);
            }
            if (e.type == SDL_MOUSEMOTION)
            {
                mouseSprite.drawRect.x = e.motion.x - (mouseSprite.drawRect.w / 2)/* + (testCamera.rect.x * windowW / testCamera.rect.w)*/;
                mouseSprite.drawRect.y = e.motion.y - (mouseSprite.drawRect.h / 2)/* + (testCamera.rect.y * windowH / testCamera.rect.h)*/;
            }
        }
        drawCScene(&testScene, true);
    }
    destroyCScene(&testScene);
    closeCoSprite();
    return error;
}
