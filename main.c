#include "warper.h"
#include "battleSystem.h"
#include "mapMaker.h"
#include "warperInterface.h"

int gameLoop(warperTilemap tilemap);
bool battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam);
cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap);

#define TILEMAP_X 80  //(global.windowW / TILE_SIZE)
#define TILEMAP_Y 60  //(global.windowH / TILE_SIZE)

#define CONFIRM_NONE 0
#define CONFIRM_MOVEMENT 1
#define CONFIRM_TELEPORT 2
#define CONFIRM_ATTACK 3

int main(int argc, char** argv)
{
    if (argc > 1)
        argv = argv;//useless, but prevents warning

    const int TILE_SIZE = 32;

    int error = initCoSprite("./assets/cb.bmp", "Warper", 40 * TILE_SIZE, 20 * TILE_SIZE, "assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE, 5, (SDL_Color) {255, 28, 198, 0xFF}, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    initCLogger(&warperLogger, "./logs/log.txt", NULL);

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

        tilemap.spritemap = calloc(tilemap.width, sizeof(uint8_t*));
        tilemap.collisionmap = calloc(tilemap.width, sizeof(uint8_t*));

        for(int x = 0; x < tilemap.width; x++)
        {
            tilemap.spritemap[x] = calloc(tilemap.height, sizeof(uint8_t));
            tilemap.collisionmap[x] = calloc(tilemap.height, sizeof(uint8_t));

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
            }
        }
    }

    bool quit = false;
    while (!quit)
    {
        quit = gameLoop(tilemap);
        //pause menu, etc
    }

    for(int classType = 0; classType < 4; classType++)
    {
        printf("Class id %d\n", classType);
        for(int i = 1; i <= 100; i++)
        {
            warperUnit testUnit =  (warperUnit) {NULL, i, 0, 0, 0, 0, classType, NULL, (warperStats) {i, i, i, i, i, i}, (warperBattleData) {0, statusNone, 0, 0, 0, false}};
            calculateStats(&testUnit, true);
            printf("Character with stats level %d: HP %d, Stamina %d, Energy %d\n", i, testUnit.maxHp, testUnit.maxStamina, testUnit.maxEnergy);

            int avgDmgRequired = 0;

            if (i < 25) //approx. early game
                avgDmgRequired = testUnit.maxHp / 3;
            if (i >= 25 && i < 66)  //approx. mid game
                avgDmgRequired = testUnit.maxHp / 3;
            if (i >= 66)  //approx late or post-game
                avgDmgRequired = testUnit.maxHp / 3;

            printf("            >damage to maintain average time to kill at this lv: %d\n", avgDmgRequired);
        }
        printf("-----------------\n");
    }

    destroyWarperTilemap(&tilemap);

    closeCoSprite();

    return error;
}

int gameLoop(warperTilemap tilemap)
{
    c2DModel mapModel;
    cSprite testPlayerSprite;
    cSprite testEnemySprite;

    warperItem testWeapon = (warperItem) {itemMelee, 0, 1};

    warperUnit playerUnit = (warperUnit) {&testPlayerSprite, 1, 0, 15, 35, 12, classNone, &testWeapon, (warperStats) {1, 1, 1, 1, 1, 1}, (warperBattleData) {15, statusNone, 0, 35, 25, false}};
    warperUnit enemyUnit = (warperUnit) {&testEnemySprite, 1, 0, 15, 35, 12, classNone, &testWeapon, (warperStats) {1, 1, 1, 1, 1, 1}, (warperBattleData) {15, statusNone, 0, 35, 12, false}};
    warperTeam playerTeam;
    initWarperTeam(&playerTeam, (warperUnit*[1]) {&playerUnit}, 1, NULL, 0, 0);
    warperTeam enemyTeam;
    initWarperTeam(&enemyTeam, (warperUnit*[1]) {&enemyUnit}, 1, NULL, 0, 0);
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
                    (cDoubleRect) {tilemap.tileSize, tilemap.tileSize, 4 * tilemap.tileSize, 2 * tilemap.tileSize},
                    (cDoubleRect) {0, 0, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, (void*) &playerTeam, 4);
        initCSprite(&testEnemySprite, NULL, "assets/characterTilesheet.png", 1,
                    (cDoubleRect) {(tilemap.width - 3) * tilemap.tileSize, (tilemap.height - 6) * tilemap.tileSize, 44, 96},
                    (cDoubleRect) {0, 3 * tilemap.tileSize / 2, 44, 96},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, (void*) &enemyTeam, 4);
    }

    cResource textBoxResource;
    warperTextBox textBox;
    {
        int textCount = 2;
        char* strings[] = {"Test", "Text box"};
        cText* texts = calloc(textCount, sizeof(cText));
        for(int i = 0; i < textCount; i++)
        {
            initCText(&(texts[i]), strings[i], (cDoubleRect) {5 * tilemap.tileSize, (14 + i) * tilemap.tileSize, 30 * tilemap.tileSize, (14 - i) * tilemap.tileSize}, 30 * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        }
        initWarperTextBox(&textBox, (cDoubleRect) {5 * tilemap.tileSize, 14 * tilemap.tileSize, 30 * tilemap.tileSize, 14 * tilemap.tileSize},
                          (SDL_Color) {0xFF, 0xFF, 0xFF, 0xC0}, (SDL_Color) {0xFF, 0x00, 0x00, 0xC0},
                          texts, (bool[2]) {false, false}, textCount, true);
    }
    initCResource(&textBoxResource, (void*) &textBox, &drawWarperTextBox, &destroyWarperTextBox, 0);

    cCamera testCamera;
    initCCamera(&testCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1, 0.0);

    cScene testScene;
    initCScene(&testScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &testCamera, (cSprite*[2]) {&testPlayerSprite, &testEnemySprite}, 2, (c2DModel*[1]) {&mapModel}, 1, /*(cResource*[1]) {&textBoxResource}, 1,*/ NULL, 0, NULL, 0);

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
            //double lastX = testPlayerSprite.drawRect.x, lastY = testPlayerSprite.drawRect.y;
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
                //*
                testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);
                //*/
                //testPlayerSprite.drawRect.y = lastY;
            }

            if (input.keyStates[SDL_SCANCODE_A])
            {
                testPlayerSprite.drawRect.x -= speed * 60.0 / framerate;
                testPlayerSprite.flip = SDL_FLIP_HORIZONTAL;
            }

            if (input.keyStates[SDL_SCANCODE_D])
            {
                testPlayerSprite.drawRect.x += speed * 60.0 / framerate;
                testPlayerSprite.flip = SDL_FLIP_NONE;
            }

            mtv = getTilemapCollision(testPlayerSprite, tilemap);

            if (mtv.magnitude)
            {  //apply collision again after doing x movements (allows smooth collision sliding. The only way I could figure out how to fix it without 100% hard-coding)
                //*
                testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);
                //*/
                //testPlayerSprite.drawRect.x = lastX;
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

        if (input.keyStates[SDL_SCANCODE_B] || getDistance(testPlayerSprite.drawRect.x, testPlayerSprite.drawRect.y, testEnemySprite.drawRect.x, testEnemySprite.drawRect.y) < 6 * tilemap.tileSize)
        {
            //have battle take place in a seperate loop
            quit = battleLoop(tilemap, &testScene, &playerTeam, &enemyTeam);
            input.quitInput = quit;
            textBoxResource.renderLayer = 1;
            //printf("Initiate battle\n");
        }

        if (input.isClick)
        {
            //if we clicked
            //if we clicked the text box
            if (textBoxResource.renderLayer != 0 && (input.click.x > textBox.rect.x && input.click.x < textBox.rect.x + textBox.rect.w && input.click.y > textBox.rect.y && input.click.y < textBox.rect.y + textBox.rect.h))
            {
                for(int i = 0; i < textBox.textsSize; i++)
                {
                    if (input.click.x > textBox.texts[i].rect.x && input.click.x < textBox.texts[i].rect.x + textBox.texts[i].rect.w &&
                        input.click.y > textBox.texts[i].rect.y && input.click.y < textBox.texts[i].rect.y + textBox.texts[i].rect.h)
                    {
                        //we clicked on an element
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

    return input.quitInput;
}

bool battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam)
{
    bool quit = false, quitEverything = false;
    int confirmMode = 0;  //used for confirming selections
    //double speed = 6.0;  //just a good speed value, nothing special. Pixels/frame at 60 FPS

    const cDoubleRect textBoxDims = (cDoubleRect) {5 * tilemap.tileSize, 14 * tilemap.tileSize, 30 * tilemap.tileSize, 14 * tilemap.tileSize};

    cResource battleTextBoxRes, movePathRes, circleRes, enemyCircleRes;
    warperTextBox battleTextBox, backupTextBox;
    char* strings[] = {"Choose Unit", "Move", "Teleport", "Attack", "Skills", "End Turn"};
    bool isOptions[] = {true, true, true, true, true, true};
    createBattleTextBox(&battleTextBox, textBoxDims, strings, isOptions, 6, tilemap);

    warperPath movePath = {.path = NULL, .pathLength = 0, .pathColor = (SDL_Color) {0, 0, 0, 0xF0}, .pathfinderWidth = 0, .pathfinderHeight = 0};
    //flowNode** movePath = NULL;
    double moveDistance = 0;
    int pathIndex = -1;

    warperCircle circle = {.radius = 0, .deltaDegrees = 10, .center = (cDoublePt) {0, 0}, .circleColor = (SDL_Color) {0, 0, 0, 0xF0}};
    warperCircle enemyCircle = {.radius = 0, .deltaDegrees = 10, .center = (cDoublePt) {0, 0}, .circleColor = (SDL_Color) {0xFF, 0, 0, 0xF0}};

    initCResource(&battleTextBoxRes, (void*) &battleTextBox, &drawWarperTextBox, &destroyWarperTextBox, 2);
    initCResource(&movePathRes, (void*) &movePath, &drawWarperPath, &destroyWarperPath, 0);
    initCResource(&circleRes, (void*) &circle, &drawWarperCircle, &destroyWarperCircle, 0);
    initCResource(&enemyCircleRes, (void*) &enemyCircle, &drawWarperCircle, &destroyWarperCircle, 0);

    battleTextBox.selection = 0;

    addResourceToCScene(scene, &battleTextBoxRes);
    addResourceToCScene(scene, &movePathRes);
    addResourceToCScene(scene, &circleRes);
    addResourceToCScene(scene, &enemyCircleRes);

    cSprite confirmPlayerSprite;
    initCSprite(&confirmPlayerSprite, NULL, "assets/characterTilesheet.png", 0,
                    (cDoubleRect) {-1 * tilemap.tileSize, -1 * tilemap.tileSize, 2 * tilemap.tileSize, 2 * tilemap.tileSize},
                    (cDoubleRect) {0, 2 * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 0);

    addSpriteToCScene(scene, &confirmPlayerSprite);

    cInputState input;
    int framerate = 0;
    int selectedUnit = 0;

    bool playerTurn = true;

    while(!quit)
    {
        input = cGetInputState(true);

        if (input.quitInput || input.keyStates[SDL_SCANCODE_ESCAPE] || input.keyStates[SDL_SCANCODE_RETURN])
        {
            quit = true;
            quitEverything = input.quitInput;
        }

        if (input.isClick)
        {
            //if we clicked

            if (battleTextBoxRes.renderLayer != 0 && (input.click.x > battleTextBox.rect.x && input.click.x < battleTextBox.rect.x + battleTextBox.rect.w &&
                                                      input.click.y > battleTextBox.rect.y && input.click.y < battleTextBox.rect.y + battleTextBox.rect.h))
            {  // if we clicked the text box
                battleTextBox.storedSelection = battleTextBox.selection;

                for(int i = 0; i < battleTextBox.textsSize; i++)
                {
                    if (battleTextBox.isOption[i] && (input.click.x > battleTextBox.texts[i].rect.x && input.click.x < battleTextBox.texts[i].rect.x + battleTextBox.texts[i].rect.w &&
                        input.click.y > battleTextBox.texts[i].rect.y && input.click.y < battleTextBox.texts[i].rect.y + battleTextBox.texts[i].rect.h))
                    {
                        //we clicked on an element
                        battleTextBox.selection = i;
                    }
                }

                if (battleTextBox.selection == 2 && !confirmMode)
                {
                    //we want to teleport
                    circle.center.x = playerTeam->units[selectedUnit]->sprite->drawRect.x + playerTeam->units[selectedUnit]->sprite->drawRect.w / 2;
                    circle.center.y = playerTeam->units[selectedUnit]->sprite->drawRect.y + playerTeam->units[selectedUnit]->sprite->drawRect.h / 2;
                    circle.radius = playerTeam->units[selectedUnit]->battleData.energyLeft * tilemap.tileSize;
                    circleRes.renderLayer = 5;
                }
                else
                    circleRes.renderLayer = 0;

                if (battleTextBox.selection == battleTextBox.textsSize - 1 || battleTextBox.selection == battleTextBox.textsSize - 2)
                {
                    //we clicked on the minimize/maximize button
                    if (battleTextBox.selection == battleTextBox.textsSize - 2 && battleTextBox.texts[battleTextBox.textsSize - 1].renderLayer == 0)
                    {
                        //minimize
                        battleTextBox.rect.y = 19 * tilemap.tileSize;
                        battleTextBox.rect.h = tilemap.tileSize;
                        for(int i = 0; i < battleTextBox.textsSize - 1; i++)
                        {
                            //hide each regular text
                            battleTextBox.texts[i].renderLayer = 0;
                        }
                        battleTextBox.texts[battleTextBox.textsSize - 1].renderLayer = 5;
                        battleTextBox.selection = battleTextBox.storedSelection; //reset selection
                    }
                    if (battleTextBox.selection == battleTextBox.textsSize - 1)
                    {
                        if (battleTextBox.texts[battleTextBox.textsSize - 1].renderLayer == 5)
                        {
                            //maximize
                            battleTextBox.rect.y = 14 * tilemap.tileSize;
                            battleTextBox.rect.h = 14 * tilemap.tileSize;
                            for(int i = 0; i < battleTextBox.textsSize - 1; i++)
                            {
                                //hide each regular text
                                battleTextBox.texts[i].renderLayer = 5;
                            }
                            battleTextBox.texts[battleTextBox.textsSize - 1].renderLayer = 0;
                        }
                        battleTextBox.selection = battleTextBox.storedSelection; //reset selection to what it was before we minimized either way
                    }
                }

                if (battleTextBox.selection == 4)
                {
                    //open skills menu
                }

                if (battleTextBox.selection == 5)
                {  //end turn
                    playerTurn = false;

                    //restore each enemy unit's battle stats (stamina, etc)
                    for(int i = 0; i < enemyTeam->unitsSize; i++)
                    {
                        enemyTeam->units[i]->battleData.staminaLeft = enemyTeam->units[i]->maxStamina;
                        enemyTeam->units[i]->battleData.energyLeft = enemyTeam->units[i]->maxEnergy;
                        enemyTeam->units[i]->battleData.teleportedOrAttacked = false;
                        //iterate on status effects
                    }

                    //set text box to be enemy turn textbox and back up your turns' textbox
                    initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                    destroyWarperTextBox((void*) &battleTextBox);
                    char* enemyTurnStrings[] = {"Choose Unit", "End Their Turn (Debug)"};
                    bool enemyTurnIsOptions[] = {true, true};
                    createBattleTextBox(&battleTextBox, textBoxDims, enemyTurnStrings, enemyTurnIsOptions, 2, tilemap);
                }
                if (battleTextBox.selection == 1 && playerTurn == false)
                {
                    //DEBUG: force enemy pass turn
                    playerTurn = true;

                    //restore each player unit's battle stats (stamina, etc)
                    for(int i = 0; i < playerTeam->unitsSize; i++)
                    {
                        playerTeam->units[i]->battleData.staminaLeft = playerTeam->units[i]->maxStamina;
                        playerTeam->units[i]->battleData.energyLeft = playerTeam->units[i]->maxEnergy;
                        playerTeam->units[i]->battleData.teleportedOrAttacked = false;
                        //iterate on status effects
                    }

                    //restore textbox to regular menu
                    destroyWarperTextBox((void*) &battleTextBox);
                    initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                    destroyWarperTextBox((void*) &backupTextBox);
                }

                if (confirmMode)
                {
                    if (battleTextBox.selection == 2 || battleTextBox.selection == 3)
                    {
                        if (battleTextBox.selection == 2)
                        {
                            //if we selected "yes"
                            if (confirmMode == CONFIRM_MOVEMENT)
                            {
                                playerTeam->units[selectedUnit]->battleData.staminaLeft -= moveDistance;
                                movePathRes.renderLayer = 0;
                            }
                            if (confirmMode == CONFIRM_TELEPORT)
                            {
                                //teleport player right away
                                //NOTE: In future make this an animation as well
                                playerTeam->units[selectedUnit]->sprite->drawRect = confirmPlayerSprite.drawRect;
                                playerTeam->units[selectedUnit]->battleData.energyLeft -= moveDistance;
                                playerTeam->units[selectedUnit]->battleData.teleportedOrAttacked = true;
                            }
                        }
                        if (battleTextBox.selection == 3)
                        {
                            //if we selected "no"
                            if (confirmMode == CONFIRM_MOVEMENT)
                            {
                                //free movePath and reset all variables that go along with it
                                destroyWarperPath((void*) &movePath);
                                movePathRes.renderLayer = 0;
                                pathIndex = -1;
                            }
                        }
                        confirmMode = CONFIRM_NONE;
                        confirmPlayerSprite.renderLayer = 0;

                        //restore textbox to regular menu
                        destroyWarperTextBox((void*) &battleTextBox);
                        initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                        destroyWarperTextBox((void*) &backupTextBox);
                    }
                }
            }
            else
            {  //if we didn't click on the text box
                if (!confirmMode)
                {
                    double worldClickX = input.click.x + scene->camera->rect.x, worldClickY = input.click.y + scene->camera->rect.y;  //where we clicked on in the world

                    /*
                    double curSpeed = speed * 60.0 / framerate;
                    worldClickX = ((int)(worldClickX / curSpeed)) * curSpeed;
                    worldClickY = ((int)(worldClickY / curSpeed)) * curSpeed;  //bounding them each distance unit covered by 1 frame
                    //*/
                    //if we want to select a unit
                    if (battleTextBox.selection == 0)
                    {
                        for(int i = 0; i < playerTeam->unitsSize; i++)
                        {
                            if (worldClickX >= playerTeam->units[i]->sprite->drawRect.x && worldClickX < playerTeam->units[i]->sprite->drawRect.x + playerTeam->units[i]->sprite->drawRect.w &&
                                worldClickY >= playerTeam->units[i]->sprite->drawRect.y && worldClickY < playerTeam->units[i]->sprite->drawRect.y + playerTeam->units[i]->sprite->drawRect.h)
                            {
                                selectedUnit = i;
                                //printf("found unit %d\n", i);
                            }
                        }

                        enemyCircleRes.renderLayer = 0;
                        int enemyIndex = -1;
                        for(int i = 0; i < enemyTeam->unitsSize; i++)
                        {
                            if (worldClickX >= enemyTeam->units[i]->sprite->drawRect.x && worldClickX < enemyTeam->units[i]->sprite->drawRect.x + enemyTeam->units[i]->sprite->drawRect.w &&
                                worldClickY >= enemyTeam->units[i]->sprite->drawRect.y && worldClickY < enemyTeam->units[i]->sprite->drawRect.y + enemyTeam->units[i]->sprite->drawRect.h)
                                enemyIndex = i;
                        }

                        if (enemyIndex > -1)
                        {
                            //printf("found\n");
                            enemyCircle.center.x = enemyTeam->units[enemyIndex]->sprite->drawRect.x + enemyTeam->units[enemyIndex]->sprite->drawRect.w / 2;
                            enemyCircle.center.y = enemyTeam->units[enemyIndex]->sprite->drawRect.y + enemyTeam->units[enemyIndex]->sprite->drawRect.h / 2;
                            enemyCircle.radius = enemyTeam->units[enemyIndex]->battleData.energyLeft * tilemap.tileSize;
                            enemyCircleRes.renderLayer = 5;
                        }
                    }

                    //if we want to move
                    if ((battleTextBox.selection == 1 || battleTextBox.selection == 2) && playerTurn)
                    {  //move or teleport
                        if (movePath.path == NULL)
                        {
                            //printf("move\n");

                            confirmPlayerSprite.drawRect.x = worldClickX  - playerTeam->units[selectedUnit]->sprite->drawRect.w / 2;
                            confirmPlayerSprite.drawRect.y = worldClickY  - playerTeam->units[selectedUnit]->sprite->drawRect.h / 2;

                            cDoubleVector mtv = getTilemapCollision(confirmPlayerSprite, tilemap);  //check if we can move there

                            //Check to see if we have enough stamina

                            if (mtv.magnitude)
                            {  //if there was a collision
                                //printf("no\n");
                            }
                            else
                            {
                                //no collision; move is valid
                                moveDistance = 0;
                                char* questionStr = calloc(61, sizeof(char));  //1 line = approx. 30 characters, and we're allowing 2 lines
                                char* templateStr = calloc(61, sizeof(char));
                                //printf("start moving\n");
                                //we can move there
                                movePath.pathLength = 0;
                                pathIndex = 0;

                                if (battleTextBox.selection == 1)
                                {
                                    //if we're moving, do a search for the correct path
                                    //*
                                    movePath.path = offsetBreadthFirst(tilemap, (int) playerTeam->units[selectedUnit]->sprite->drawRect.x, (int) playerTeam->units[selectedUnit]->sprite->drawRect.y,
                                                                  (int) confirmPlayerSprite.drawRect.x, (int) confirmPlayerSprite.drawRect.y,
                                                                  (int) playerTeam->units[selectedUnit]->sprite->drawRect.w, (int) playerTeam->units[selectedUnit]->sprite->drawRect.h,
                                                                   &(movePath.pathLength), false, scene->camera);

                                    if (movePath.path)
                                    {
                                        //movePath[lengthOfPath - 1].x = worldClickX;
                                        //movePath[lengthOfPath - 1].y = worldClickY;  //don't need these anymore most likely
                                        moveDistance = (int) round(movePath.path[0].distance);
                                        movePathRes.renderLayer = 5;
                                        movePath.pathfinderWidth = (int) playerTeam->units[selectedUnit]->sprite->drawRect.w;
                                        movePath.pathfinderHeight = (int) playerTeam->units[selectedUnit]->sprite->drawRect.h;
                                    }
                                    //*/

                                    //playerTeam->units[selectedUnit]->sprite->drawRect = oldRect;
                                    if (moveDistance > 0 && moveDistance <= playerTeam->units[selectedUnit]->battleData.staminaLeft)
                                    {
                                        strncpy(templateStr, "Do you want to move? It will use %d stamina.", 60);
                                        confirmMode = CONFIRM_MOVEMENT;
                                    }
                                    else
                                    {
                                        //set flag to false, reset variables, free movePath
                                        destroyWarperPath((void*) &movePath);
                                        pathIndex = -1;
                                    }
                                }
                                else
                                {
                                    moveDistance = getDistance(playerTeam->units[selectedUnit]->sprite->drawRect.x, playerTeam->units[selectedUnit]->sprite->drawRect.y,
                                                           confirmPlayerSprite.drawRect.x, confirmPlayerSprite.drawRect.y) / tilemap.tileSize;
                                    //show the energy cost and ask for confirmation

                                    if (worldClickX >= 0 && worldClickY >= 0 &&  //not going out of bounds up or left
                                        worldClickX <= tilemap.width * tilemap.tileSize - playerTeam->units[selectedUnit]->sprite->drawRect.w &&  //not going out of bounds right
                                        worldClickY <= tilemap.height * tilemap.tileSize - playerTeam->units[selectedUnit]->sprite->drawRect.h &&  //not going out of bounds down
                                        moveDistance > 0 && moveDistance <= playerTeam->units[selectedUnit]->battleData.energyLeft &&  //are moving somewhere and have the energy to do so
                                        !playerTeam->units[selectedUnit]->battleData.teleportedOrAttacked)  //haven't teleported or attacked already
                                    {
                                        strncpy(templateStr, "Do you want to teleport? It will use %d energy.", 60);
                                        confirmMode = CONFIRM_TELEPORT;
                                    }
                                }

                                if (confirmMode)
                                {
                                    snprintf(questionStr, 60, templateStr, (int) moveDistance);
                                    confirmPlayerSprite.renderLayer = 3;
                                    confirmPlayerSprite.drawRect.w = playerTeam->units[selectedUnit]->sprite->drawRect.w;
                                    confirmPlayerSprite.drawRect.h = playerTeam->units[selectedUnit]->sprite->drawRect.h;

                                    //create confirm textbox and backup regular textbox
                                    initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                                    destroyWarperTextBox((void*) &battleTextBox);
                                    createBattleTextBox(&battleTextBox, textBoxDims, (char* [4]) {questionStr, " ", "Yes", "No"}, (bool[4]) {false, false, true, true}, 4, tilemap);
                                }
                                free(questionStr);
                                free(templateStr);
                            }
                        }
                    }
                    if (battleTextBox.selection == 3 && playerTurn)
                    {  //battle
                        //find which enemy we clicked on, if any
                        int enemyIndex = -1;
                        for(int i = 0; i < enemyTeam->unitsSize; i++)
                        {
                            if (worldClickX >= enemyTeam->units[i]->sprite->drawRect.x && worldClickX < enemyTeam->units[i]->sprite->drawRect.x + enemyTeam->units[i]->sprite->drawRect.w &&
                                worldClickY >= enemyTeam->units[i]->sprite->drawRect.y && worldClickY < enemyTeam->units[i]->sprite->drawRect.y + enemyTeam->units[i]->sprite->drawRect.h)
                                enemyIndex = i;
                        }
                        if (enemyIndex != -1)
                        {
                            //printf("found enemy %d\n", enemyIndex);
                            //calculate if we hit, calculate damage
                            double distance = getDistance(playerTeam->units[selectedUnit]->sprite->drawRect.x, playerTeam->units[selectedUnit]->sprite->drawRect.y, enemyTeam->units[enemyIndex]->sprite->drawRect.x, enemyTeam->units[enemyIndex]->sprite->drawRect.y);

                            //if attacker is not a technomancer, check collision to see if bullets/sword are blocked

                            warperAttackResult attackResult = doAttack(playerTeam->units[selectedUnit], enemyTeam->units[enemyIndex], distance);

                            playerTeam->units[selectedUnit]->battleData.teleportedOrAttacked = true;
                            //do something with the result?

                            printf("Attack did %d damage (was miss: %s)\n", attackResult.damage, boolToString(attackResult.miss));
                        }
                    }
                }
            }
        }


        if (movePath.path != NULL && !confirmMode)
        {
            //move our unit until there are no more nodes
            if (playerTeam->units[selectedUnit]->sprite->drawRect.x > movePath.path[pathIndex].x)  //if we're moving left
                playerTeam->units[selectedUnit]->sprite->flip = SDL_FLIP_HORIZONTAL;
            else
                playerTeam->units[selectedUnit]->sprite->flip = SDL_FLIP_NONE;


            playerTeam->units[selectedUnit]->sprite->drawRect.x = movePath.path[pathIndex].x;
            playerTeam->units[selectedUnit]->sprite->drawRect.y = movePath.path[pathIndex].y;

            pathIndex++;

            if (pathIndex >= movePath.pathLength)
            {
                //set flag to false, reset variables, free movePath
                destroyWarperPath((void*) &movePath);
                pathIndex = -1;
            }
            //*/
        }


        //camera movement
        if (input.keyStates[SDL_SCANCODE_UP])
            scene->camera->rect.y -= 10 * 60.0 / framerate;

        if (input.keyStates[SDL_SCANCODE_DOWN])
            scene->camera->rect.y += 10 * 60.0 / framerate;

        if (input.keyStates[SDL_SCANCODE_LEFT])
            scene->camera->rect.x -= 10 * 60.0 / framerate;

        if (input.keyStates[SDL_SCANCODE_RIGHT])
            scene->camera->rect.x += 10 * 60.0 / framerate;

        if (input.keyStates[SDL_SCANCODE_F11])
            printf("%f, %f\n", playerTeam->units[selectedUnit]->sprite->drawRect.x, playerTeam->units[selectedUnit]->sprite->drawRect.y);

        drawCScene(scene, true, true, &framerate, 60);
    }

    //local resources must be removed before quitting
    removeResourceFromCScene(scene, &battleTextBoxRes, -1, true);
    removeResourceFromCScene(scene, &movePathRes, -1, true);
    removeResourceFromCScene(scene, &circleRes, -1, true);
    removeResourceFromCScene(scene, &enemyCircleRes, -1, true);
    removeSpriteFromCScene(scene, &confirmPlayerSprite, -1, true);

    return quitEverything;
}

cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap)
{
    cDoubleVector mtv = {0, 0};

    int playerX = round(-0.45 + playerSprite.drawRect.x / tilemap.tileSize), playerY = round(-0.45 + playerSprite.drawRect.y / tilemap.tileSize);
    int playerW = round(0.45 + playerSprite.drawRect.w / tilemap.tileSize), playerH = round(0.45 + playerSprite.drawRect.h / tilemap.tileSize);

    for(int x = playerX; x <= playerX + playerW; x++)
    {
        for(int y = playerY - 1; y <= playerY + playerH; y++)
        {
            //printf("%d, %d", x, y);
            if (x >= 0 && x < tilemap.width && y >= 0 && y < tilemap.height && tilemap.collisionmap[x][y])
            {
                cDoubleVector newMtv = checkCDoubleRectCollision(playerSprite.drawRect, (cDoubleRect) {x * tilemap.tileSize, y * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize});  //get collision result
                if ((mtv.magnitude < 0.001 || ((int) newMtv.degrees % 180 == 90 && (int) mtv.degrees % 180 == 0) || ((int) newMtv.degrees % 180 == 0 && (int) mtv.degrees % 180 == 90)) && (int) mtv.degrees % 90 != 45)
                {
                    mtv = addCDoubleVectors(mtv, newMtv);  //if we don't have a partial mtv on the same axis, add these partials together
                    //printf("- found %f at %f deg", newMtv.magnitude, newMtv.degrees);
                }
            }
            //printf("\n");
        }
    }
    //printf("--------\n");

    /*
    if (mtv.magnitude)
        printf("final mtv == %f @ %f deg\n", mtv.magnitude, mtv.degrees);
    //*/

    return mtv;
}
