#include "crankMain.h"
#include "crankGraphics.h"
#include "crankFile.h"
#include "crankUtility.h"

int main(int argc, char* argv[])
{
    int error = initCrank("", "Warper", 960, 480, "assets/Px437_ITT_BIOS_X.ttf", 24);
    SDL_Event e;
    bool quit = false;
    SDL_Texture* mouseTexture;
    loadIMG("assets/cb.bmp", &mouseTexture);
    cSprite mouseSprite;
    initCSprite(&mouseSprite, mouseTexture, 0, (SDL_Rect) {0, 0, 150, 120}, 1.0, SDL_FLIP_NONE, 0.0, NULL, 5);
    cCamera testCamera;
    initCCamera(&testCamera, (SDL_Rect) {0, 0, 20, 10}, 1.0);
    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, &mouseSprite, 1, NULL, 0, NULL, 0, NULL, 0);
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
                mouseSprite.rect.x -= (testCamera.rect.x * windowW / testCamera.rect.w);
                mouseSprite.rect.y -= (testCamera.rect.y * windowH / testCamera.rect.h);

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

                mouseSprite.rect.x += (testCamera.rect.x * windowW / testCamera.rect.w);
                mouseSprite.rect.y += (testCamera.rect.y * windowH / testCamera.rect.h);

            }
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                loadIMG("assets/cb1.bmp", &mouseSprite.texture);
            }
            else
            {
                loadIMG("assets/cb.bmp", &mouseSprite.texture);
            }
            if (e.type == SDL_MOUSEMOTION)
            {
                mouseSprite.rect.x = e.motion.x - (mouseSprite.rect.w / 2) + (testCamera.rect.x * windowW / testCamera.rect.w);
                mouseSprite.rect.y = e.motion.y - (mouseSprite.rect.h / 2) + (testCamera.rect.y * windowH / testCamera.rect.h);
            }
        }
        drawCScene(&testScene, true);
    }
    closeCrank();
    return error;
}
