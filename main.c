#include "crankMain.h"
#include "crankGraphics.h"
#include "crankFile.h"
#include "crankUtility.h"

int main(int argc, char* argv[])
{
    int error = initCrank("", "Warper", 960, 480, "assets/Px437_ITT_BIOS_X.ttf", 24);
    SDL_Event e;
    bool quit = false;
    SDL_Texture* mouseTexture, * playerTexture;
    loadIMG("assets/cb.bmp", &mouseTexture);
    loadIMG("assets/cb1.bmp", &playerTexture);
    cSprite sprites[2];
    initCSprite(&sprites[0], mouseTexture, 0, (SDL_Rect) {0, 0, 80, 80}, (SDL_Rect) {15, 0, 120, 120}, 1.0, SDL_FLIP_NONE, 0.0, NULL, 1);
    initCSprite(&sprites[1], playerTexture, 1, (SDL_Rect) {0, 0, 120, 120}, (SDL_Rect) {15, 0, 120, 120}, 1.0, SDL_FLIP_NONE, 0.0, NULL, 2);
    cText versionText;
    initCText(&versionText, CRANK_VERSION, (SDL_Rect){0, 0, 150, 50}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, 2);
    cCamera testCamera;
    initCCamera(&testCamera, (SDL_Rect) {0, 0, 20, 10}, 1.0);
    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, sprites, 2, NULL, 0, NULL, 0, &versionText, 1);
    printf("%d\n", testScene.sprites[0]->drawPriority);
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
                sprites[0].drawRect.x -= (testCamera.rect.x * windowW / testCamera.rect.w);  //subtract out camera offset
                sprites[0].drawRect.y -= (testCamera.rect.y * windowH / testCamera.rect.h);

                if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    quit = true;
                if (e.key.keysym.scancode == SDL_SCANCODE_W)
                    testCamera.rect.y--;

                if (e.key.keysym.scancode == SDL_SCANCODE_A)
                    testCamera.rect.x--;

                if (e.key.keysym.scancode == SDL_SCANCODE_S)
                    testCamera.rect.y++;

                if (e.key.keysym.scancode == SDL_SCANCODE_D)
                    testCamera.rect.x++;

                sprites[0].drawRect.x += (testCamera.rect.x * windowW / testCamera.rect.w);  //add back camera offset
                sprites[0].drawRect.y += (testCamera.rect.y * windowH / testCamera.rect.h);

            }
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                loadIMG("assets/cb1.bmp", &sprites[0].texture);
            }
            else
            {
                loadIMG("assets/cb.bmp", &sprites[0].texture);
            }
            if (e.type == SDL_MOUSEMOTION)
            {
                sprites[0].drawRect.x = e.motion.x - (sprites[0].drawRect.w / 2) + (testCamera.rect.x * windowW / testCamera.rect.w);
                sprites[0].drawRect.y = e.motion.y - (sprites[0].drawRect.h / 2) + (testCamera.rect.y * windowH / testCamera.rect.h);
            }
        }
        drawCScene(&testScene, true);
    }
    destroyCScene(&testScene);
    closeCrank();
    return error;
}
