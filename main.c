#include "warper.h"
#include "battleSystem.h"
#include "mapMaker.h"
#include "warperInterface.h"

int gameLoop(warperTilemap tilemap, cScene* gameScene);
bool battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam);

cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap);

#define TEST_TILEMAP_X 80  //(global.windowW / TILE_SIZE)
#define TEST_TILEMAP_Y 60  //(global.windowH / TILE_SIZE)

#define WARPER_FRAME_LIMIT 60

#define CONFIRM_NONE 0
#define CONFIRM_MOVEMENT 1
#define CONFIRM_TELEPORT 2
#define CONFIRM_ATTACK 3

int main(int argc, char** argv)
{
    if (argc > 1)
        argv = argv;  //useless, but prevents warning. might actually add a debug option on cmd line or something

    const int TILE_SIZE = 32;

    int error = initCoSprite("./assets/cb.bmp", "Warper", 40 * TILE_SIZE, 20 * TILE_SIZE, "assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE, 5, (SDL_Color) {255, 28, 198, 0xFF}, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    initCLogger(&warperLogger, "./logs/log.txt", NULL);

    warperTilemap tilemap;

    cScene menuScene;
    char* optionsArray[] = {"Load Test Map", "Load Created Map", "Create New Map", "Quit"};
    warperTextBox menuBox;
    createMenuTextBox(&menuBox, (cDoubleRect) {TILE_SIZE, TILE_SIZE, global.windowW - 2 * TILE_SIZE, global.windowH - 2 * TILE_SIZE}, optionsArray, (bool[4]) {true, true, true, true}, 4, &(global.mainFont));

    cResource menuBoxResource;
    initCResource(&menuBoxResource, (void*) &menuBox, drawWarperTextBox, destroyWarperTextBox, 5);

    cCamera gameCamera;
    initCCamera(&gameCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0.0);

    initCScene(&menuScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &gameCamera, NULL, 0, NULL, 0, (cResource*[1]) {&menuBoxResource}, 1, NULL, 0);

    cInputState input;
    int fps = 0;
    while(menuBox.selection == -1)
    {
        input = cGetInputState(true);

        if (input.quitInput)
            menuBox.selection = 3;  //quit

        if (input.isClick)
        {
            //if we clicked
            if (menuBoxResource.renderLayer != 0)
                checkWarperTextBoxClick(&menuBox, input.click.x, input.click.y);
        }
        drawCScene(&menuScene, true, true, &fps, WARPER_FRAME_LIMIT);
    }

    if (menuBox.selection == 2)
    {
        //create map
        if (createNewMap(&tilemap, TILE_SIZE)) //if it returns 1, aka if we're force quitting
            menuBox.selection = 3;  //treat it as a quit
    }
    if (menuBox.selection == 1)
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
    if (menuBox.selection == 0)
    {
        //create temp map
        tilemap.width = TEST_TILEMAP_X;
        tilemap.height = TEST_TILEMAP_Y;
        tilemap.tileSize = TILE_SIZE;

        tilemap.spritemap_layer1 = calloc(tilemap.width, sizeof(int*));
        tilemap.spritemap_layer2 = calloc(tilemap.width, sizeof(int*));
        tilemap.collisionmap = calloc(tilemap.width, sizeof(int*));

        for(int x = 0; x < tilemap.width; x++)
        {
            tilemap.spritemap_layer1[x] = calloc(tilemap.height, sizeof(int));
            tilemap.spritemap_layer2[x] = calloc(tilemap.height, sizeof(int));
            tilemap.collisionmap[x] = calloc(tilemap.height, sizeof(int));

            for(int y = 0; y < tilemap.height; y++)
            {
                tilemap.spritemap_layer1[x][y] = 4;  //normal sprite
                tilemap.spritemap_layer2[x][y] = 798;  //invisible sprite?

                if (x == 1)  //left row sprite
                    tilemap.spritemap_layer1[x][y] = 1;

                if (y == 1)  //top row sprite
                    tilemap.spritemap_layer1[x][y] = 3;

                if (x == tilemap.width - 2)  //right row sprite
                    tilemap.spritemap_layer1[x][y] = 7;

                if (y == tilemap.height - 2)  //bottom row sprite
                    tilemap.spritemap_layer1[x][y] = 5;

                if (x == 0 || y == 0 || x + 1 == tilemap.width || y + 1 == tilemap.height || (y == 20 && x > tilemap.width / 2) || (y == 40 && x < tilemap.height / 2))
                {
                    if (x == 0 || y == 0 || x + 1 == tilemap.width || y + 1 == tilemap.height)
                        tilemap.spritemap_layer1[x][y] = 36;  //skyscraper sprite
                    else
                        tilemap.spritemap_layer1[x][y] = 13; //collision sprite

                    tilemap.collisionmap[x][y] = 1;
                }
                else
                    tilemap.collisionmap[x][y] = 0;

                if (x == 1 && y == 1)  //top-left corner sprite
                    tilemap.spritemap_layer1[x][y] = 0;

                if (x == tilemap.width - 2 && y == tilemap.height - 2)  //bottom-right corner sprite
                    tilemap.spritemap_layer1[x][y] = 8;

                if (x == 1 && y == tilemap.height - 2)  //bottom-left corner sprite
                    tilemap.spritemap_layer1[x][y] = 2;

                if (x == tilemap.width - 2 && y == 1)  //top-right corner sprite
                    tilemap.spritemap_layer1[x][y] = 6;
            }
        }
    }

    if (menuBox.selection == 3)
    {
        destroyCScene(&menuScene);
        closeCoSprite();
        return 0;
    }

    destroyCScene(&menuScene);  //have to destroy after to preserve selection

    initCCamera(&gameCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0.0);  //re-init camera

    c2DModel mapModel_layer1, mapModel_layer2;
    loadTilemapModels(tilemap, &mapModel_layer1, &mapModel_layer2);

    cScene gameScene;
    initCScene(&gameScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &gameCamera, NULL, 0, (c2DModel*[2]) {&mapModel_layer1, &mapModel_layer2}, 2, NULL, 0, NULL, 0);

    bool quit = false;
    while (!quit)
    {
        quit = gameLoop(tilemap, &gameScene);
        //pause menu, etc
    }

    destroyCScene(&gameScene);

    /*/
    for(int classType = 0; classType < 4; classType++)
    {
        printf("Class id %d\n", classType);
        for(int i = 1; i <= 100; i++)
        {
            warperUnit testUnit =  (warperUnit) {NULL, i, 0, 0, 0, 0, classType, NULL, (warperStats) {i, i, i, i, i, i, 0}, (warperBattleData) {0, statusNone, 0, 0, 0, false}};
            calculateStats(&testUnit, true);
            printf("Character with stats level %d: HP %d, Stamina %d, Energy %d\n", i, testUnit.maxHp, testUnit.maxStamina, testUnit.maxEnergy);

            warperUnit attackingUnit = (warperUnit) {NULL, i, 0, 0, 0, 0, classTechnomancer, NULL, (warperStats) {i, i, i, i, i, i, 0}, (warperBattleData) {0, statusNone, 0, 0, 0, false}};
            warperAttackCheck checkResult = checkAttack(&attackingUnit, &testUnit, 0);

            printf("            >Attacker character dmg %d. ttk: %d\n", checkResult.damage, (int) round((double) testUnit.maxHp / checkResult.damage + 0.45));

            / *
            int maxDmgRequired = 0, minDmgRequired = 0;

            if (i < 25) //approx. early game
            {
                maxDmgRequired = testUnit.maxHp - 1; //2+ hits
                minDmgRequired = testUnit.maxHp / 2;
            }
            if (i >= 25 && i < 66)  //approx. mid game
            {
                maxDmgRequired = testUnit.maxHp / 2 - 1;  //3+ hits
                minDmgRequired = testUnit.maxHp / 3;
            }
            if (i >= 66)  //approx late or post-game
            {
                maxDmgRequired = testUnit.maxHp / 3 - 1;  //4+ hits
                minDmgRequired = testUnit.maxHp / 4;
            }

            //printf("            >max damage to maintain average time to kill at this lv: %d\n            >min damage to maintain average time to kill at this lv: %d\n", maxDmgRequired, minDmgRequired);
            // * /
        }
        printf("-----------------\n");
    }
    //*/

    destroyWarperTilemap(&tilemap);

    closeCoSprite();

    return error;
}

int gameLoop(warperTilemap tilemap, cScene* gameScene)
{
    cSprite testPlayerSprite;
    cSprite testEnemySprite;

    warperTeam playerTeam;
    warperTeam enemyTeam;

    warperItem testWeapon = (warperItem) {itemMelee, 0, 1};

    warperUnit playerUnit = (warperUnit) {&testPlayerSprite, 1, 0, 150, 35, 12, classNone, &testWeapon, (warperStats) {1, 1, 1, 1, 1, 1, 0}, (warperBattleData) {150, statusNone, 0, 35, 12, false}};
    warperUnit enemyUnit = (warperUnit) {&testEnemySprite, 1, 0, 150, 35, 12, classNone, &testWeapon, (warperStats) {1, 1, 1, 1, 1, 1, 0}, (warperBattleData) {150, statusNone, 0, 35, 12, false}};

    initCSprite(&testPlayerSprite, NULL, "assets/characterTilesheet.png", 0,
                    (cDoubleRect) {tilemap.tileSize, tilemap.tileSize, 2 * tilemap.tileSize, 2 * tilemap.tileSize},
                    (cDoubleRect) {0, 0, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 4);
    initCSprite(&testEnemySprite, NULL, "assets/characterTilesheet.png", 1,
                (cDoubleRect) {(tilemap.width - 3) * tilemap.tileSize, (tilemap.height - 6) * tilemap.tileSize, 44, 96},
                (cDoubleRect) {0, 3 * tilemap.tileSize / 2, 44, 96},
                NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 4);

    //squad init
    cSprite* testSquadSprites = calloc(4, sizeof(cSprite));
    cDoublePt testSquadPts[4] = {(cDoublePt) {0, 0}, (cDoublePt) {2 * tilemap.tileSize, 0}, (cDoublePt) {0, 2 * tilemap.tileSize}, (cDoublePt) {2 * tilemap.tileSize, 4 * tilemap.tileSize}};

    warperUnit testSquadUnits[5];
    testSquadUnits[0] = playerUnit;

    for(int i = 0; i < 4; i++)
    {
        initCSprite(&(testSquadSprites[i]), NULL, "assets/characterTilesheet.png", 1 + i,
                    (cDoubleRect) {testSquadPts[i].x, testSquadPts[i].y, 2 * tilemap.tileSize, 2 * tilemap.tileSize},
                    (cDoubleRect) {0, 0, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 4);

        testSquadUnits[1 + i] = (warperUnit) {&(testSquadSprites[i]), 1, 0, 150, 35 - 2 * i, 12 + 2 * i, classNone, &testWeapon, (warperStats) {1, 1, 1, 1, 1, 1, 0}, (warperBattleData) {150, statusNone, 0, 35 - 2 * i, 12 + 2 * i, false}};

        addSpriteToCScene(gameScene, &(testSquadSprites[i]));
    }

    initWarperTeam(&playerTeam, (warperUnit*[5]) {&playerUnit, &(testSquadUnits[1]), &(testSquadUnits[2]), &(testSquadUnits[3]), &(testSquadUnits[4])}, 5, NULL, 0, 0);
    //end squad init

    initWarperTeam(&enemyTeam, (warperUnit*[1]) {&enemyUnit}, 1, NULL, 0, 0);

    addSpriteToCScene(gameScene, &testPlayerSprite);
    addSpriteToCScene(gameScene, &testEnemySprite);

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
                testPlayerSprite.drawRect.y -= (int) (speed * 60.0 / framerate);

            if (input.keyStates[SDL_SCANCODE_S])
                testPlayerSprite.drawRect.y += (int) (speed * 60.0 / framerate);

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
                testPlayerSprite.drawRect.x -= (int) (speed * 60.0 / framerate);
                testPlayerSprite.flip = SDL_FLIP_HORIZONTAL;
            }

            if (input.keyStates[SDL_SCANCODE_D])
            {
                testPlayerSprite.drawRect.x += (int) (speed * 60.0 / framerate);
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

            gameScene->camera->rect.x = (int) (testPlayerSprite.drawRect.x - gameScene->camera->rect.w / 2);  //set the camera to center on the player
            gameScene->camera->rect.y = (int) (testPlayerSprite.drawRect.y - gameScene->camera->rect.h / 2);

            if (gameScene->camera->rect.y < 0)  //if the camera is set out of bounds in the -y, fix it
                gameScene->camera->rect.y = 0;

            if (gameScene->camera->rect.y > (tilemap.height - gameScene->camera->rect.h / tilemap.tileSize) * tilemap.tileSize)  //if the camera is set out of bounds in the +y, fix it
                gameScene->camera->rect.y = (tilemap.height - gameScene->camera->rect.h / tilemap.tileSize) * tilemap.tileSize;

            if (gameScene->camera->rect.x < 0)  //if the camera is set out of bounds in the -x, fix it
                gameScene->camera->rect.x = 0;

            if (gameScene->camera->rect.x > (tilemap.width - gameScene->camera->rect.w / tilemap.tileSize) * tilemap.tileSize)  //if the camera is set out of bounds in the +x, fix it
                gameScene->camera->rect.x = (tilemap.width - gameScene->camera->rect.w / tilemap.tileSize) * tilemap.tileSize;
        }

        if (input.keyStates[SDL_SCANCODE_B] || getDistance(testPlayerSprite.drawRect.x, testPlayerSprite.drawRect.y, testEnemySprite.drawRect.x, testEnemySprite.drawRect.y) < 6 * tilemap.tileSize)
        {
            //have battle take place in a seperate loop
            quit = battleLoop(tilemap, gameScene, &playerTeam, &enemyTeam);
            input.quitInput = quit;
            //printf("Initiate battle\n");
        }

        /*
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
        //*/

        //camera movement
        if (input.keyStates[SDL_SCANCODE_UP])
            gameScene->camera->rect.y -= 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_DOWN])
            gameScene->camera->rect.y += 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_LEFT])
            gameScene->camera->rect.x -= 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_RIGHT])
            gameScene->camera->rect.x += 10 * 60 / framerate;

        drawCScene(gameScene, true, true, &framerate, 60);
    }

    removeSpriteFromCScene(gameScene, &testPlayerSprite, -1, true);

    for(int i = 0; i < 4; i++)
        removeSpriteFromCScene(gameScene, &(testSquadSprites[i]), -1, true);

    free(testSquadSprites);

    removeSpriteFromCScene(gameScene, &testEnemySprite, -1, true);

    return input.quitInput;
}

bool battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam)
{
    //TODO: add a battle intro animation

    bool quit = false, quitEverything = false;
    int confirmMode = CONFIRM_NONE;  //used for confirming selections
    //double speed = 6.0;  //just a good speed value, nothing special. Pixels/frame at 60 FPS

    const cDoubleRect textBoxDims = (cDoubleRect) {5 * tilemap.tileSize, 14 * tilemap.tileSize, 30 * tilemap.tileSize, 14 * tilemap.tileSize};

    cResource battleTextBoxRes, movePathRes, circleRes, enemyCircleRes;
    warperTextBox battleTextBox, backupTextBox;
    char* strings[] = {"Choose Unit", "Move", "Teleport", "Attack", "Mods", "End Turn"};
    bool isOptions[] = {true, true, true, true, true, true};
    createBattleTextBox(&battleTextBox, textBoxDims, strings, isOptions, 6, tilemap.tileSize);

    warperPath movePath = {.path = NULL, .pathLength = 0, .pathColor = (SDL_Color) {0, 0, 0, 0xF0}, .pathfinderWidth = 0, .pathfinderHeight = 0};
    warperUnit* pathfinderUnit = NULL;
    //flowNode** movePath = NULL;
    double moveDistance = 0;
    int pathIndex = -1;

    warperCircle circle = {.radius = 0, .deltaDegrees = 10, .center = (cDoublePt) {0, 0}, .circleColor = (SDL_Color) {0, 0, 0, 0x50}, .filled = true};
    warperCircle enemyCircle = {.radius = 0, .deltaDegrees = 10, .center = (cDoublePt) {0, 0}, .circleColor = (SDL_Color) {0xFF, 0, 0, 0x50}, .filled = true};

    initCResource(&battleTextBoxRes, (void*) &battleTextBox, drawWarperTextBox, destroyWarperTextBox, 1);
    initCResource(&movePathRes, (void*) &movePath, drawWarperPath, destroyWarperPath, 0);
    initCResource(&circleRes, (void*) &circle, drawWarperCircle, destroyWarperCircle, 0);
    initCResource(&enemyCircleRes, (void*) &enemyCircle, drawWarperCircle, destroyWarperCircle, 0);

    battleTextBox.selection = 0;

    addResourceToCScene(scene, &battleTextBoxRes);
    addResourceToCScene(scene, &movePathRes);
    addResourceToCScene(scene, &circleRes);
    addResourceToCScene(scene, &enemyCircleRes);

    cSprite confirmPlayerSprite;
    cSprite unitSelectSprite;

    initCSprite(&confirmPlayerSprite, NULL, "assets/characterTilesheet.png", 0,
                    (cDoubleRect) {-3 * tilemap.tileSize, -3 * tilemap.tileSize, 2 * tilemap.tileSize, 2 * tilemap.tileSize},
                    (cDoubleRect) {0, 2 * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 0);

    initCSprite(&unitSelectSprite, NULL, "assets/uiTilesheet.png", 0,
                playerTeam->units[0]->sprite->drawRect,  //put this over top of the first selected unit
                (cDoubleRect) {0, 0, tilemap.tileSize, tilemap.tileSize},
                NULL, 1.0, SDL_FLIP_NONE, 0, false, NULL, 3);

    addSpriteToCScene(scene, &confirmPlayerSprite);
    addSpriteToCScene(scene, &unitSelectSprite);

    warperAttackCheck checkResult;

    cInputState input;
    int framerate = 60;
    int selectedUnit = 0, selectedEnemy = -1;  //used for selecting a unit or an enemy unit
    int turnEnemy = 0;  //used for going through enemy turns

    const int CUSTOM_COLLISIONS_COUNT = playerTeam->unitsSize - 1 + enemyTeam->unitsSize;
    cDoubleRect* customCollisions = calloc(CUSTOM_COLLISIONS_COUNT, sizeof(cDoubleRect));

    bool playerTurn = true, pathToCursor = false;

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
                checkWarperTextBoxClick(&battleTextBox, input.click.x, input.click.y);  //saves the selection to the battleTextBox

                if (battleTextBox.selection == 1 && !confirmMode)
                {  //we clicked the first element (move or DEBUG force enemy pass turn)
                    if (playerTurn)
                    {  //we want to move
                        pathToCursor = true;  //start pathing non-stop to cursor
                    }
                    else
                    {  //DEBUG: force enemy pass turn
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
                        initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                        destroyWarperTextBox((void*) &backupTextBox);
                    }
                }
                else
                {
                    if (!confirmMode)
                    {  //if we are in confirm mode it's likely we're trying to move
                        if (!(battleTextBox.selection == battleTextBox.textsSize - 1 || battleTextBox.selection == battleTextBox.textsSize - 2))  //if we're not maximizing/minimizing
                        {
                            pathToCursor = false;
                            confirmPlayerSprite.renderLayer = 0;

                            if (movePath.path)
                                destroyWarperPath((void*) &movePath);  //free current path
                        }
                    }
                }

                if (battleTextBox.selection == 2 && !confirmMode)
                {
                    //we want to teleport
                    circle.center.x = playerTeam->units[selectedUnit]->sprite->drawRect.x + playerTeam->units[selectedUnit]->sprite->drawRect.w / 2;
                    circle.center.y = playerTeam->units[selectedUnit]->sprite->drawRect.y + playerTeam->units[selectedUnit]->sprite->drawRect.h / 2;
                    circle.radius = playerTeam->units[selectedUnit]->battleData.energyLeft * tilemap.tileSize;
                    circleRes.renderLayer = 2;
                }
                else
                {
                    if (!(battleTextBox.selection == battleTextBox.textsSize - 1 || battleTextBox.selection == battleTextBox.textsSize - 2))  //if we're not maximizing/minimizing
                        circleRes.renderLayer = 0;
                }

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

                if (battleTextBox.selection == 4 && !confirmMode)
                {
                    //open mods menu
                }

                if (battleTextBox.selection == 5 && !confirmMode)
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
                    initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.outlineColor, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                    destroyWarperTextBox((void*) &battleTextBox);
                    char* enemyTurnStrings[] = {"Choose Unit", "End Their Turn (Debug)"};
                    bool enemyTurnIsOptions[] = {true, true};
                    createBattleTextBox(&battleTextBox, textBoxDims, enemyTurnStrings, enemyTurnIsOptions, 2, tilemap.tileSize);
                }

                if (confirmMode)
                {
                    //If we are confirming an action
                    if (battleTextBox.selection == 2 || battleTextBox.selection == 3 || (((battleTextBox.selection == 5 || battleTextBox.selection == 6) && confirmMode == CONFIRM_ATTACK)))
                    {
                        //If we selected "Yes" or "No"
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
                                pathfinderUnit = NULL;
                            }
                        }
                        if (battleTextBox.selection == 5)
                        {
                            //if we selected "yes" but only for attack confirm
                            if (confirmMode == CONFIRM_ATTACK)
                            {
                                //do attack
                                doAttack(playerTeam->units[selectedUnit], enemyTeam->units[selectedEnemy], checkResult);
                                playerTeam->units[selectedUnit]->battleData.teleportedOrAttacked = true;
                                playerTeam->units[selectedUnit]->battleData.energyLeft = 0;

                                if (enemyTeam->units[selectedEnemy]->battleData.curHp <= 0)
                                {
                                    //enemy is dead
                                    enemyTeam->units[selectedEnemy]->sprite->renderLayer = 0;
                                }
                            }
                        }

                        //printf("selection %d\n", battleTextBox.selection);

                        confirmMode = CONFIRM_NONE;
                        confirmPlayerSprite.renderLayer = 0;

                        //restore textbox to regular menu
                        destroyWarperTextBox((void*) &battleTextBox);
                        initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                        destroyWarperTextBox((void*) &backupTextBox);
                    }
                }
            }
            else
            {  //if we didn't click on the text box
                if (!confirmMode)
                {
                    //if we aren't currently trying to confirm an action
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
                            if (worldClickX >= playerTeam->units[i]->sprite->drawRect.x
                                && worldClickX < playerTeam->units[i]->sprite->drawRect.x + playerTeam->units[i]->sprite->drawRect.w
                                && worldClickY >= playerTeam->units[i]->sprite->drawRect.y
                                && worldClickY < playerTeam->units[i]->sprite->drawRect.y + playerTeam->units[i]->sprite->drawRect.h)
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
                            enemyCircleRes.renderLayer = 2;
                        }
                    }

                    //if we want to move
                    if ((battleTextBox.selection == 1 || battleTextBox.selection == 2) && playerTurn)
                    {  //move or teleport
                        /*if (movePath.path == NULL)
                        {*/
                            //printf("move\n");

                            confirmPlayerSprite.drawRect.x = worldClickX  - playerTeam->units[selectedUnit]->sprite->drawRect.w / 2;
                            confirmPlayerSprite.drawRect.y = worldClickY  - playerTeam->units[selectedUnit]->sprite->drawRect.h / 2;

                            cDoubleVector mtv = getTilemapCollision(confirmPlayerSprite, tilemap);  //check if we can move there

                            if (mtv.magnitude)
                            {  //if there was a collision
                                //printf("no\n");
                            }
                            else
                            {
                                //no collision; move is valid
                                moveDistance = 0;
                                char* questionStr = calloc(61, sizeof(char));  //1 line = approx. 30 characters, and we're allowing 2 lines
                                //printf("start moving\n");
                                //we can move there
                                movePath.pathLength = 0;
                                pathIndex = 0;

                                if (battleTextBox.selection == 1)
                                {
                                    pathToCursor = false;

                                    if (movePath.path)
                                        destroyWarperPath((void*) &movePath);  //free current path before we make a new one
                                    //if we're moving, do a search for the correct path
                                    //*
                                    int customArrPos = 0;
                                    for(int i = 0; i < playerTeam->unitsSize; i++)  //copy over all player-team rects (except current)
                                    {
                                        if (i != selectedUnit)
                                            customCollisions[customArrPos++] = playerTeam->units[i]->sprite->drawRect;
                                    }

                                    for(int i = 0; i < enemyTeam->unitsSize; i++)  //copy over all enemy-team rects
                                        customCollisions[customArrPos++] = enemyTeam->units[i]->sprite->drawRect;

                                    movePath.path = offsetBreadthFirst(tilemap, (int) playerTeam->units[selectedUnit]->sprite->drawRect.x, (int) playerTeam->units[selectedUnit]->sprite->drawRect.y,
                                                                        (int) confirmPlayerSprite.drawRect.x, (int) confirmPlayerSprite.drawRect.y,
                                                                        (int) playerTeam->units[selectedUnit]->sprite->drawRect.w, (int) playerTeam->units[selectedUnit]->sprite->drawRect.h,
                                                                        customCollisions, CUSTOM_COLLISIONS_COUNT,
                                                                        &(movePath.pathLength), false, scene->camera);

                                    if (movePath.path)
                                    {
                                        //movePath[lengthOfPath - 1].x = worldClickX;
                                        //movePath[lengthOfPath - 1].y = worldClickY;  //don't need these anymore most likely
                                        moveDistance = (int) round(movePath.path[0].distance);
                                        movePathRes.renderLayer = 2;
                                        movePath.pathfinderWidth = (int) playerTeam->units[selectedUnit]->sprite->drawRect.w;
                                        movePath.pathfinderHeight = (int) playerTeam->units[selectedUnit]->sprite->drawRect.h;
                                        pathfinderUnit = playerTeam->units[selectedUnit];  //set the pathfinder so that we can reference it when the movement is confirmed and the unit starts moving
                                    }
                                    //*/

                                    //playerTeam->units[selectedUnit]->sprite->drawRect = oldRect;
                                    if (moveDistance > 0 && moveDistance <= playerTeam->units[selectedUnit]->battleData.staminaLeft)
                                    {
                                        strncpy(questionStr, "Do you want to move? It will use %d stamina.", 60);
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
                                        strncpy(questionStr, "Do you want to teleport? It will use %d energy.", 60);
                                        confirmMode = CONFIRM_TELEPORT;
                                    }
                                }

                                if (confirmMode)
                                {
                                    snprintf(questionStr, 60, questionStr, (int) moveDistance);
                                    confirmPlayerSprite.renderLayer = 2;
                                    confirmPlayerSprite.drawRect.w = playerTeam->units[selectedUnit]->sprite->drawRect.w;
                                    confirmPlayerSprite.drawRect.h = playerTeam->units[selectedUnit]->sprite->drawRect.h;

                                    //create confirm textbox and backup regular textbox
                                    initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.outlineColor, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                                    destroyWarperTextBox((void*) &battleTextBox);
                                    createBattleTextBox(&battleTextBox, textBoxDims, (char* [4]) {questionStr, " ", "Yes", "No"}, (bool[4]) {false, false, true, true}, 4, tilemap.tileSize);
                                }
                                free(questionStr);
                            }
                        //}
                    }
                    if (battleTextBox.selection == 3 && playerTurn && !playerTeam->units[selectedUnit]->battleData.teleportedOrAttacked)
                    {  //battle
                        //find which enemy we clicked on, if any
                        selectedEnemy = -1;
                        for(int i = 0; i < enemyTeam->unitsSize; i++)
                        {
                            if (worldClickX >= enemyTeam->units[i]->sprite->drawRect.x && worldClickX < enemyTeam->units[i]->sprite->drawRect.x + enemyTeam->units[i]->sprite->drawRect.w
                                && worldClickY >= enemyTeam->units[i]->sprite->drawRect.y && worldClickY < enemyTeam->units[i]->sprite->drawRect.y + enemyTeam->units[i]->sprite->drawRect.h
                                && enemyTeam->units[i]->sprite->renderLayer > 0)
                                selectedEnemy = i;
                        }
                        if (selectedEnemy != -1)
                        {
                            //printf("found enemy %d\n", enemyIndex);
                            if (playerTeam->units[selectedUnit]->sprite->drawRect.x + playerTeam->units[selectedUnit]->sprite->drawRect.w / 2 < enemyTeam->units[selectedEnemy]->sprite->drawRect.x + enemyTeam->units[selectedEnemy]->sprite->drawRect.w / 2)
                                playerTeam->units[selectedUnit]->sprite->flip = SDL_FLIP_NONE;  //if the player is more to the left than the enemy, face the enemy

                            if (playerTeam->units[selectedUnit]->sprite->drawRect.x + playerTeam->units[selectedUnit]->sprite->drawRect.w / 2 > enemyTeam->units[selectedEnemy]->sprite->drawRect.x + enemyTeam->units[selectedEnemy]->sprite->drawRect.w / 2)
                                playerTeam->units[selectedUnit]->sprite->flip = SDL_FLIP_HORIZONTAL;  //if the player is more to the right than the enemy, face the enemy

                            //otherwise, stay facing the same way

                            //calculate if we hit, calculate damage

                            //get distance (in tiles)
                            double distance = getDistance(playerTeam->units[selectedUnit]->sprite->drawRect.x + playerTeam->units[selectedUnit]->sprite->drawRect.w / 2,
                                                          playerTeam->units[selectedUnit]->sprite->drawRect.y + playerTeam->units[selectedUnit]->sprite->drawRect.h / 2,
                                                          enemyTeam->units[selectedEnemy]->sprite->drawRect.x + enemyTeam->units[selectedEnemy]->sprite->drawRect.w / 2,
                                                          enemyTeam->units[selectedEnemy]->sprite->drawRect.y + enemyTeam->units[selectedEnemy]->sprite->drawRect.h / 2) / tilemap.tileSize;

                            //if attacker is not a technomancer, cast a ray or 4 (for all sprite edges) and check collision to see if bullets/sword are blocked
                            bool attackBlocked = false;
                            if (playerTeam->units[selectedUnit]->classType != classTechnomancer)
                            {
                                for(double i = 0; i - distance < 0.0001; /*floating pt accuracy check; equivalent to i < dist*/ i += 0.5)  //for every half-tile step along the line between you and the enemy
                                {
                                    //check for a collision tile; if so, then attack is blocked
                                }
                            }

                            if (!attackBlocked)
                            {
                                checkResult = checkAttack(playerTeam->units[selectedUnit], enemyTeam->units[selectedEnemy], distance);

                                printf("Attack does %d damage (chance to hit: %f%% against enemy %f tiles away)\n", checkResult.damage, checkResult.hitChance * 100.0, distance);

                                //ask for confirmation
                                confirmMode = CONFIRM_ATTACK;

                                char* questionStr = calloc(82, sizeof(char));
                                snprintf(questionStr, 81, "Attack? It will do %d dmg;\n%d%% hit chance, %d%% crit chance,\nand %d%% status chance.", checkResult.damage, (int) (100 * checkResult.hitChance), (int) (100 * checkResult.critChance), (int) (100 * checkResult.statusChance));
                                initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.outlineColor, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                                destroyWarperTextBox((void*) &battleTextBox);
                                //createBattleTextBox(&battleTextBox, textBoxDims, (char* [5]) {questionStr, " ", " ", "Yes", "No"}, (bool[5]) {false, false, false, true, true}, 5, tilemap.tileSize);

                                createBattleTextBox(&battleTextBox, (cDoubleRect) {1 * tilemap.tileSize, 1 * tilemap.tileSize, (38) * tilemap.tileSize, (18) * tilemap.tileSize}, (char* [7]) {questionStr, " ", " ", " ", " ", "Yes", "No"}, (bool[7]) {false, false, false, false, false, true, true}, 7, tilemap.tileSize);

                                /*
                                cText* tempTexts = calloc(7, sizeof(cText));

                                initCText(&(tempTexts[0]), questionStr, (cDoubleRect) {2 * tilemap.tileSize, 2 * tilemap.tileSize, (tilemap.width - 12) * tilemap.tileSize, tilemap.tileSize}, (tilemap.width - 12) * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0.0, true, 5);
                                for(int i = 1; i < 5; i++)
                                {
                                    initCText(&(tempTexts[i]), " ", (cDoubleRect) {2 * tilemap.tileSize, (2 + i) * tilemap.tileSize, (tilemap.width - 12) * tilemap.tileSize, tilemap.tileSize}, (tilemap.width - 12) * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0.0, true, 5);
                                }
                                initCText(&(tempTexts[5]), "Yes", (cDoubleRect) {2 * tilemap.tileSize, 7 * tilemap.tileSize, (tilemap.width - 12) * tilemap.tileSize, tilemap.tileSize}, (tilemap.width - 12) * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0.0, true, 5);
                                initCText(&(tempTexts[6]), "No", (cDoubleRect) {2 * tilemap.tileSize, 8 * tilemap.tileSize, (tilemap.width - 12) * tilemap.tileSize, tilemap.tileSize}, (tilemap.width - 12) * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0.0, true, 5);

                                initWarperTextBox(&battleTextBox, (cDoubleRect) {1 * tilemap.tileSize, 1 * tilemap.tileSize, (38) * tilemap.tileSize, (18) * tilemap.tileSize}, (SDL_Color) {0x00, 0x00, 0x00, 0xFF}, (SDL_Color) {0x00, 0x00, 0x00, 0x58}, (SDL_Color) {0xFF, 0x00, 0x00, 0x20}, tempTexts, (char[7]) {false, false, false, false, false, true, true}, 7, true);
                                for(int i = 0; i < 7; i++)
                                    destroyCText(&(tempTexts[i]));
                                //*/

                                free(questionStr);
                            }
                        }
                    }
                }
            }
        }

        //if we should find a path non-stop to the cursor
        if (pathToCursor)
        {
            {
                confirmPlayerSprite.drawRect.x = input.motion.x + scene->camera->rect.x - confirmPlayerSprite.drawRect.w / 2;
                confirmPlayerSprite.drawRect.y = input.motion.y + scene->camera->rect.y - confirmPlayerSprite.drawRect.h / 2;
            }

            cDoubleVector mtv = getTilemapCollision(confirmPlayerSprite, tilemap);  //check if we can move there

            if (!mtv.magnitude)
            {
                if (movePath.path)  //free it first
                    destroyWarperPath((void*) &movePath);

                int customArrPos = 0;
                for(int i = 0; i < playerTeam->unitsSize; i++)  //copy over all player-team rects (except current)
                {
                    if (i != selectedUnit)
                        customCollisions[customArrPos++] = playerTeam->units[i]->sprite->drawRect;
                }

                for(int i = 0; i < enemyTeam->unitsSize; i++)  //copy over all enemy-team rects
                    customCollisions[customArrPos++] = enemyTeam->units[i]->sprite->drawRect;


                movePath.path = offsetBreadthFirst(tilemap, (int) playerTeam->units[selectedUnit]->sprite->drawRect.x, (int) playerTeam->units[selectedUnit]->sprite->drawRect.y,  //current player position
                                                   (int) confirmPlayerSprite.drawRect.x, (int) confirmPlayerSprite.drawRect.y,  //current mouse position
                                                   (int) playerTeam->units[selectedUnit]->sprite->drawRect.w, (int) playerTeam->units[selectedUnit]->sprite->drawRect.h,
                                                    customCollisions, CUSTOM_COLLISIONS_COUNT,
                                                   &(movePath.pathLength), false, scene->camera);

                if (movePath.path)
                {  //init movePath
                    if ((int) round(movePath.path[0].distance) > playerTeam->units[selectedUnit]->battleData.staminaLeft)
                    {
                        movePathRes.renderLayer = 0; //hide the movePath
                        confirmPlayerSprite.renderLayer = 0;  //hide the confirm sprite
                    }
                    else
                    {
                        movePathRes.renderLayer = 2;
                        confirmPlayerSprite.renderLayer = 2;
                    }
                    movePath.pathfinderWidth = (int) playerTeam->units[selectedUnit]->sprite->drawRect.w;  //set the pathfinder's dimensions to be equal to our selected unit's dims
                    movePath.pathfinderHeight = (int) playerTeam->units[selectedUnit]->sprite->drawRect.h;
                }
            }
            else
            {
                movePathRes.renderLayer = 0; //hide the movePath
                confirmPlayerSprite.renderLayer = 0;  //hide the confirm sprite
            }
        }

        if (!playerTurn)
        {
            //TODO: ADD ENEMY AI HERE
            //calculate best move based on AI type, enemy difficulty, each unit's class, remaining health, etc
            //AI Types: Lone wolves each fight one unit, Pack wolves use 2-3 units to fight one of your units

            //SIMPLE TEST AI - just repositions on top of you (you must teleport to escape)
            if (!movePath.path)  //if we aren't currently following a path
            {
                if (turnEnemy >= enemyTeam->unitsSize)
                {
                    turnEnemy = 0;
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
                    initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                    destroyWarperTextBox((void*) &backupTextBox);
                }
                else
                {
                    if (enemyTeam->units[turnEnemy]->sprite->renderLayer != 0)  //if the unit is alive
                    {
                        //printf("entered enemy pathfinding\n");
                        pathfinderUnit = enemyTeam->units[turnEnemy];

                        movePath.pathfinderWidth = pathfinderUnit->sprite->drawRect.w;
                        movePath.pathfinderHeight = pathfinderUnit->sprite->drawRect.w;

                        //printf("%f, %f\n", pathfinderUnit->sprite->drawRect.x, pathfinderUnit->sprite->drawRect.y);
                        for(int targetUnit = 0; targetUnit < playerTeam->unitsSize; targetUnit++)
                        {
                            int pathLength = 0;
                            double targetUnitX = playerTeam->units[targetUnit]->sprite->drawRect.x, targetUnitY = playerTeam->units[targetUnit]->sprite->drawRect.y;

                            if (targetUnitX - pathfinderUnit->sprite->drawRect.x > pathfinderUnit->sprite->drawRect.w)
                                targetUnitX -= pathfinderUnit->sprite->drawRect.w;  //move to the closest position on the left
                            if (targetUnitX - pathfinderUnit->sprite->drawRect.x < pathfinderUnit->sprite->drawRect.w)
                                targetUnitX += playerTeam->units[targetUnit]->sprite->drawRect.w;  //move to the closest position on the right

                            if (fabs(playerTeam->units[targetUnit]->sprite->drawRect.x - targetUnitX) < 0.001)  //if we didn't adjust on the X direction
                            {
                                if (targetUnitY > pathfinderUnit->sprite->drawRect.y)
                                    targetUnitY -= pathfinderUnit->sprite->drawRect.h;  //move to the closest position above
                                if (targetUnitY < pathfinderUnit->sprite->drawRect.y)
                                    targetUnitY += playerTeam->units[targetUnit]->sprite->drawRect.h;  //move to the closest position below
                            }
                            node* path = offsetBreadthFirst(tilemap, pathfinderUnit->sprite->drawRect.x, pathfinderUnit->sprite->drawRect.y,
                                                            targetUnitX, targetUnitY,
                                                            pathfinderUnit->sprite->drawRect.w, pathfinderUnit->sprite->drawRect.h,
                                                            customCollisions, CUSTOM_COLLISIONS_COUNT, &pathLength, false, scene->camera);

                            if (path && (!movePath.path || path[0].distance < movePath.path[0].distance))  //find the closest enemy and path to them
                            {
                                //printf("found a better path\n");
                                if (movePath.path)
                                    free(movePath.path);

                                movePath.path = path;
                                movePath.pathLength = pathLength;
                            }

                        }
                        //printf("path distance = %f\n", movePath.path[0].distance);
                    }
                    turnEnemy++;
                }
            }
        }

        if (movePath.path != NULL && !confirmMode && pathfinderUnit && !pathToCursor)  //we don't want to continuously move to cursor so we ignore when we try
        {
            //move our unit until there are no more nodes
            if (pathfinderUnit->sprite->drawRect.x > movePath.path[pathIndex].x)  //if we're moving left
                pathfinderUnit->sprite->flip = SDL_FLIP_HORIZONTAL;
            else
                pathfinderUnit->sprite->flip = SDL_FLIP_NONE;


            pathfinderUnit->sprite->drawRect.x = movePath.path[pathIndex].x;  //update position
            pathfinderUnit->sprite->drawRect.y = movePath.path[pathIndex].y;

            pathIndex++;

            if (pathIndex >= movePath.pathLength)
            {
                //set flag to false, reset variables, free movePath
                destroyWarperPath((void*) &movePath);
                pathIndex = -1;
                pathfinderUnit = NULL;
            }
            //*/
        }

        //update unit select sprite position
        unitSelectSprite.drawRect = playerTeam->units[selectedUnit]->sprite->drawRect;

        //camera movement
        if (input.keyStates[SDL_SCANCODE_W])
            scene->camera->rect.y -= 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_S])
            scene->camera->rect.y += 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_A])
            scene->camera->rect.x -= 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_D])
            scene->camera->rect.x += 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_F11])  //DEBUG
            printf("%f, %f\n", playerTeam->units[selectedUnit]->sprite->drawRect.x, playerTeam->units[selectedUnit]->sprite->drawRect.y);

        drawCScene(scene, true, true, &framerate, WARPER_FRAME_LIMIT);
    }

    free(customCollisions);

    //local objects/resources must be removed before quitting
    removeResourceFromCScene(scene, &battleTextBoxRes, -1, true);
    removeResourceFromCScene(scene, &movePathRes, -1, true);
    removeResourceFromCScene(scene, &circleRes, -1, true);
    removeResourceFromCScene(scene, &enemyCircleRes, -1, true);
    removeSpriteFromCScene(scene, &confirmPlayerSprite, -1, true);
    removeSpriteFromCScene(scene, &unitSelectSprite, -1, true);

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
