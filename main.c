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
    cSprite mouseSprite, playerSprite;
    initCSprite(&mouseSprite, mouseTexture, 0, (SDL_Rect) {0, 0, 80, 80}, (SDL_Rect) {15, 0, 120, 120}, 1.0, SDL_FLIP_NONE, 0.0, true, NULL, 1);
    initCSprite(&playerSprite, playerTexture, 1, (SDL_Rect) {0, 24, 120, 120}, (SDL_Rect) {15, 0, 120, 120}, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 2);
    cText versionText;
    initCText(&versionText, CRANK_VERSION, (SDL_Rect){0, 0, 150, 50}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, true, 5);
    cCamera testCamera;
    initCCamera(&testCamera, (SDL_Rect) {0, 0, 20, 10}, 1.0, 0.0);
    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, (cSprite*[2]) {&mouseSprite, &playerSprite}, 2, NULL, 0, NULL, 0, (cText*[1]) {&versionText}, 1);
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


                if (e.key.keysym.scancode == SDL_SCANCODE_W)
                    playerSprite.drawRect.y -= 6;

                if (e.key.keysym.scancode == SDL_SCANCODE_A)
                    playerSprite.drawRect.x -= 6;

                if (e.key.keysym.scancode == SDL_SCANCODE_S)
                    playerSprite.drawRect.y += 6;

                if (e.key.keysym.scancode == SDL_SCANCODE_D)
                    playerSprite.drawRect.x += 6;


                if (e.key.keysym.scancode == SDL_SCANCODE_UP)
                    testCamera.rect.y--;

                if (e.key.keysym.scancode == SDL_SCANCODE_LEFT)
                    testCamera.rect.x--;

                if (e.key.keysym.scancode == SDL_SCANCODE_DOWN)
                    testCamera.rect.y++;

                if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                    testCamera.rect.x++;

                //mouseSprite.drawRect.x += (testCamera.rect.x * windowW / testCamera.rect.w);  //add back camera offset
                //mouseSprite.drawRect.y += (testCamera.rect.y * windowH / testCamera.rect.h);

            }
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                loadIMG("assets/cb1.bmp", &mouseSprite.texture);
                playerSprite.drawRect.x = e.button.x - (playerSprite.drawRect.w / 2) + (testCamera.rect.x * windowW / testCamera.rect.w);
                playerSprite.drawRect.y = e.button.y - (playerSprite.drawRect.h / 2) + (testCamera.rect.y * windowH / testCamera.rect.h);
            }
            if (e.type == SDL_MOUSEBUTTONUP)
            {
                loadIMG("assets/cb.bmp", &mouseSprite.texture);
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
    closeCrank();
    return error;
}
