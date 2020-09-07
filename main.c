#include "CoSprite/csGraphics.h"
#include "CoSprite/csInput.h"
#include "CoSprite/csUtility.h"
#include "battleSystem.h"
#include "mapMaker.h"

typedef struct _warperTextBox
{
    cDoubleRect rect;
    SDL_Color bgColor;
    SDL_Color highlightColor;
    cText* texts;
    int textsSize;
    bool isMenu;
    int selection;
} warperTextBox;

void initWarperTextBox(warperTextBox* textBox, cDoubleRect rect, SDL_Color bgColor, SDL_Color highlightColor, cText* texts, int textsSize, bool isMenu);
void drawWarperTextBox(void* textBoxSubclass, cCamera camera);
void destroyWarperTextBox(void* textBoxSubclass);
int gameLoop(warperTilemap tilemap);
cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap);

#define WARPER_MODE_WALK 0
#define WARPER_MODE_BATTLE 1

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
        //create map
        createNewMap(&tilemap, TILE_SIZE);
    }
    else
    if (temp_k == SDLK_l)
    {
        //load map
        char* importedMap = calloc(7, sizeof(char));
        readLine("maps/testMap.txt", 0, 7, &importedMap);
        //printf("%s\n", importedMap);

        char dimData[4] = "\0";

        strncpy(dimData, importedMap, 3);
        tilemap.width = strtol(dimData, NULL, 16);
        strncpy(dimData, importedMap + 3, 3);
        tilemap.height = strtol(dimData, NULL, 16);

        free(importedMap);

        int importedLength = 3 * 3 * tilemap.width * tilemap.height + 3 + 3;  //3 arrays * 3 digits * width * height + width data + height data
        importedMap = calloc(importedLength + 1, sizeof(char));

        readLine("maps/testMap.txt", 0, importedLength + 1, &importedMap);

        importTilemap(&tilemap, importedMap);

        free(importedMap);

        tilemap.tileSize = TILE_SIZE;
    }
    else
    {
        //create temp map
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
                tilemap.spritemap[x][y] = 4;  //normal sprite

                if (x == 1)  //left row sprite
                    tilemap.spritemap[x][y] = 1;

                if (y == 1)  //top row sprite
                    tilemap.spritemap[x][y] = 3;

                if (x == tilemap.width - 2)  //right row sprite
                    tilemap.spritemap[x][y] = 7;

                if (y == tilemap.height - 2)  //bottom row sprite
                    tilemap.spritemap[x][y] = 5;

                if (x == 0 || y == 0 || x + 1 == tilemap.width || y + 1 == tilemap.height || (y == 20 && x > tilemap.width / 2) || (y == 40 && x < tilemap.height / 2))
                {
                    if (x == 0 || y == 0 || x + 1 == tilemap.width || y + 1 == tilemap.height)
                        tilemap.spritemap[x][y] = 36;  //skyscraper sprite
                    else
                        tilemap.spritemap[x][y] = 13; //collision sprite

                    tilemap.collisionmap[x][y] = 1;
                }
                else
                    tilemap.collisionmap[x][y] = 0;

                if (x == 1 && y == 1)  //top-left corner sprite
                    tilemap.spritemap[x][y] = 0;



                if (x == tilemap.width - 2 && y == tilemap.height - 2)  //bottom-right corner sprite
                    tilemap.spritemap[x][y] = 8;

                if (x == 1 && y == tilemap.height - 2)  //bottom-left corner sprite
                    tilemap.spritemap[x][y] = 2;

                if (x == tilemap.width - 2 && y == 1)  //top-right corner sprite
                    tilemap.spritemap[x][y] = 6;

                tilemap.eventmap[x][y] = 0;
            }
        }
    }

    gameLoop(tilemap);

    destroyWarperTilemap(&tilemap);

    closeCoSprite();

    return error;
}

int gameLoop(warperTilemap tilemap)
{
    c2DModel mapModel;
    cSprite testPlayerSprite;
    cSprite testEnemySprite;

    warperUnit playerUnit = (warperUnit) {1, 0, 15, noClass, (warperStats) {1, 1, 1, 1, 1, 1}, (warperBattleData) {15, 300, false}};
    warperUnit enemyUnit = (warperUnit) {1, 0, 15, noClass, (warperStats) {1, 1, 1, 1, 1, 1}, (warperBattleData) {15, 300, false}};;
    {
        SDL_Texture* tilesetTexture;
        loadIMG("assets/worldTilesheet.png", &tilesetTexture);
        cSprite* tileSprites = calloc(tilemap.width * tilemap.height, sizeof(cSprite));
        for(int x = 0; x < tilemap.width; x++)
        {
            for(int y = 0; y < tilemap.height; y++)
            {
                initCSprite(&tileSprites[x * tilemap.height + y], tilesetTexture, "assets/worldTilesheet.png", tilemap.spritemap[x][y],
                            (cDoubleRect) {tilemap.tileSize * x, tilemap.tileSize * y, tilemap.tileSize, tilemap.tileSize},
                            (cDoubleRect) {(tilemap.spritemap[x][y] / 20) * tilemap.tileSize / 2, (tilemap.spritemap[x][y] % 20) * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);
            }
        }
        initC2DModel(&mapModel, tileSprites, tilemap.width * tilemap.height, (cDoublePt) {0, 0}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, NULL, 5);

        initCSprite(&testPlayerSprite, NULL, "assets/characterTilesheet.png", 0,
                    (cDoubleRect) {tilemap.tileSize, tilemap.tileSize, tilemap.tileSize, tilemap.tileSize},
                    (cDoubleRect) {0, 0, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, (void*) &playerUnit, 4);
        initCSprite(&testEnemySprite, NULL, "assets/characterTilesheet.png", 1,
                    (cDoubleRect) {(tilemap.width - 2) * tilemap.tileSize, (tilemap.height - 2) * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize},
                    (cDoubleRect) {0, tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, (void*) &enemyUnit, 4);
    }

    //note: convert warperTextBox from cTexts to char*s
    //*
    cResource textBoxResource;
    warperTextBox textBox;
    int lastSelection = -1;
    {
        int textCount = 2;
        char* strings[2] = {"Test", "Text box"};
        cText* texts = calloc(textCount, sizeof(cText));
        for(int i = 0; i < textCount; i++)
        {
            initCText(&(texts[i]), strings[i], (cDoubleRect) {5 * tilemap.tileSize, (14 + i) * tilemap.tileSize, 30 * tilemap.tileSize, (14 - i) * tilemap.tileSize}, 30 * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        }
        initWarperTextBox(&textBox, (cDoubleRect) {5 * tilemap.tileSize, 14 * tilemap.tileSize, 30 * tilemap.tileSize, 14 * tilemap.tileSize},
                          (SDL_Color) {0xFF, 0xFF, 0xFF, 0xC0}, (SDL_Color) {0xFF, 0x00, 0x00, 0xC0},
                          texts, textCount, true);
    }
    initCResource(&textBoxResource, (void*) &textBox, &drawWarperTextBox, &destroyWarperTextBox, 0);
    //*/

    cCamera testCamera;
    initCCamera(&testCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1, 0.0);

    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, (cSprite*[2]) {&testPlayerSprite, &testEnemySprite}, 2, (c2DModel*[1]) {&mapModel}, 1, (cResource*[1]) {&textBoxResource}, 1, /*NULL, 0,*/ NULL, 0);

    bool quit = false;

    int gameplayMode = WARPER_MODE_WALK;
    warperBattle battle = (warperBattle) {noObjective, true};

    cInputState input;
    int framerate = 0;

    while(!quit)
    {
        input = cGetInputState(true);

        if (input.quitInput || input.keyStates[SDL_SCANCODE_ESCAPE] || input.keyStates[SDL_SCANCODE_RETURN])
            quit = true;

        //if your turn (or out of battle)
        if (battle.isPlayerTurn)
        {
            //character movement
            if (input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_S] || input.keyStates[SDL_SCANCODE_D])
            {
                double speed = 6.0;  //just a good speed value, nothing special. Pixels/frame at 60 FPS
                if ((input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_S]) && (input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_D]))
                    speed *= sin(degToRad(45));  //diagonal speed component

                if (input.keyStates[SDL_SCANCODE_W])
                {
                    testPlayerSprite.drawRect.y -= speed * 60.0 / framerate;
                    if (gameplayMode == WARPER_MODE_BATTLE)
                        playerUnit.battleData.remainingDistance -= speed * 60.0 / framerate;
                }

                if (input.keyStates[SDL_SCANCODE_S])
                {
                    testPlayerSprite.drawRect.y += speed * 60.0 / framerate;

                    if (gameplayMode == WARPER_MODE_BATTLE)
                        playerUnit.battleData.remainingDistance -= speed * 60.0 / framerate;
                }

                cDoubleVector mtv = getTilemapCollision(testPlayerSprite, tilemap);

                if (mtv.magnitude)
                {  //apply collision after doing y movements
                    testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                    testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                    //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);

                    playerUnit.battleData.remainingDistance += mtv.magnitude;
                }

                if (input.keyStates[SDL_SCANCODE_A])
                {
                    testPlayerSprite.drawRect.x -= speed * 60.0 / framerate;
                    testPlayerSprite.flip = SDL_FLIP_HORIZONTAL;

                    if (gameplayMode == WARPER_MODE_BATTLE)
                        playerUnit.battleData.remainingDistance -= speed * 60.0 / framerate;
                }

                if (input.keyStates[SDL_SCANCODE_D])
                {
                    testPlayerSprite.drawRect.x += speed * 60.0 / framerate;
                    testPlayerSprite.flip = SDL_FLIP_NONE;

                    if (gameplayMode == WARPER_MODE_BATTLE)
                        playerUnit.battleData.remainingDistance -= speed * 60.0 / framerate;
                }

                mtv = getTilemapCollision(testPlayerSprite, tilemap);

                if (mtv.magnitude)
                {  //apply collision again after doing x movements (allows smooth collision sliding. The only way I could figure out how to fix it without 100% hard-coding)
                    testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                    testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                    //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);

                    playerUnit.battleData.remainingDistance += mtv.magnitude;
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

                if (playerUnit.battleData.remainingDistance <= 0)
                {
                    battle.isPlayerTurn = false;
                    printf("no longer your turn!\n");
                }
            }

            if (input.keyStates[SDL_SCANCODE_SPACE])
            {
                //attack
            }
        }
        else
        {
            if (!battle.isPlayerTurn && input.keyStates[SDL_SCANCODE_P])  //debug pass enemy turn
            {
                battle.isPlayerTurn = true;
                playerUnit.battleData.remainingDistance = 300;
                printf("your turn!\n");
            }
        }

        //if we're in walk mode
        if (gameplayMode == WARPER_MODE_WALK)
        {
            if (getDistance(testPlayerSprite.drawRect.x, testPlayerSprite.drawRect.y, testEnemySprite.drawRect.x, testEnemySprite.drawRect.y) < 6 * tilemap.tileSize)
            {
                gameplayMode = WARPER_MODE_BATTLE;
                textBoxResource.renderLayer = 1;
                printf("Initiate battle\n");
            }
        }
        else
        {
            //we're in battle mode (?)
        }

        if (input.isClick)
        {
            //if we clicked
            //if we clicked the text box
            if (input.click.x > textBox.rect.x && input.click.x < textBox.rect.x + textBox.rect.w && input.click.y > textBox.rect.y && input.click.y < textBox.rect.y + textBox.rect.h)
            {
                for(int i = 0; i < textBox.textsSize; i++)
                {
                    if (input.click.x > textBox.texts[i].rect.x && input.click.x < textBox.texts[i].rect.x + textBox.texts[i].rect.w &&
                        input.click.y > textBox.texts[i].rect.y && input.click.y < textBox.texts[i].rect.y + textBox.texts[i].rect.h)
                    {
                        //we clicked on an element
                        lastSelection = textBox.selection;
                        textBox.selection = i;
                    }
                }
            }
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
    destroyCScene(&testScene);

    return (input.quitInput) ? 1 : 0;
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

void initWarperTextBox(warperTextBox* textBox, cDoubleRect rect, SDL_Color bgColor, SDL_Color highlightColor, cText* texts, int textsSize, bool isMenu)
{
    textBox->rect = rect;
    textBox->bgColor = bgColor;
    textBox->highlightColor = highlightColor;
    textBox->textsSize = textsSize;
    textBox->texts = calloc(textsSize, sizeof(cText));
    if (textBox->texts != NULL)
    {
        for(int i = 0; i < textsSize; i++)
        {
            initCText(&(textBox->texts[i]), texts[i].str, texts[i].rect, texts[i].maxW, texts[i].textColor, texts[i].bgColor, texts[i].font, texts[i].scale, texts[i].flip, texts[i].degrees, true, 1);
        }
    }
    else
    {
        printf("ERROR: not enough mem to alloc text box\n");
    }
    textBox->isMenu = isMenu;
    textBox->selection = -1;
}

void drawWarperTextBox(void* textBoxSubclass, cCamera camera)
{
    warperTextBox* textBox = (warperTextBox*) textBoxSubclass;

    SDL_Rect boxRect = (SDL_Rect) {textBox->rect.x, textBox->rect.y, textBox->rect.w, textBox->rect.h};

    Uint8 prevR = 0, prevG = 0, prevB = 0, prevA = 0;
    SDL_GetRenderDrawColor(global.mainRenderer, &prevR, &prevG, &prevB, &prevA);

    //draw text box
    SDL_SetRenderDrawColor(global.mainRenderer, textBox->bgColor.r, textBox->bgColor.g, textBox->bgColor.b, textBox->bgColor.a);
    SDL_RenderFillRect(global.mainRenderer, &boxRect);

    //draw cTexts
    for(int i = 0; i < textBox->textsSize; i++)
    {
        if (textBox->texts[i].renderLayer > 0)
            drawCText(textBox->texts[i], camera, false);
    }

    //draw selection highlight
    if (textBox->isMenu && textBox->selection != -1)
    {
        SDL_SetRenderDrawColor(global.mainRenderer, textBox->highlightColor.r, textBox->highlightColor.g, textBox->highlightColor.b, textBox->highlightColor.a);
        SDL_Rect selectionRect = (SDL_Rect) {textBox->rect.x, boxRect.y + textBox->selection * textBox->texts[textBox->selection].font->fontSize, textBox->texts[textBox->selection].rect.w, textBox->texts[textBox->selection].rect.h};
        SDL_RenderDrawRect(global.mainRenderer, &selectionRect);
    }

    SDL_SetRenderDrawColor(global.mainRenderer, prevR, prevG, prevB, prevA);
}

void destroyWarperTextBox(void* textBoxSubclass)
{
    warperTextBox* textBox = (warperTextBox*) textBoxSubclass;

    textBox->rect = (cDoubleRect) {0,0,0,0};
    textBox->bgColor = (SDL_Color) {0,0,0,0};
    textBox->highlightColor = (SDL_Color) {0,0,0,0};

    for(int i = 0; i < textBox->textsSize; i++)
    {
        destroyCText(&(textBox->texts[i]));
    }
    textBox->textsSize = 0;
    textBox->isMenu = false;
    textBox->selection = 0;
}
