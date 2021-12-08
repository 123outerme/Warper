#include "warper.h"
#include "battleSystem.h"
#include "mapMaker.h"
#include "cutsceneMaker.h"
#include "warperInterface.h"
#include "warperCutscene.h"

//test/debug functions
int gameDevMenu();
void createTestMap(warperTilemap* tilemap);
void debugPrintStatProgression();
void playTestAnimation();

//game control/state functions
int gameLoop(warperTilemap tilemap, cScene* gameScene, warperTeam* playerTeam, warperTeam* enemyTeam);
int pauseMenu(cScene* gameScene, warperTeam* playerTeam);
bool optionsMenu(bool inGame);
int battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam);

cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap);

void checkWarperUnitHover(SDL_MouseMotionEvent motion, warperTeam* playerTeam, warperTeam* enemyTeam, cCamera* camera);

#define TEST_TILEMAP_X 80  //(global.windowW / TILE_SIZE)
#define TEST_TILEMAP_Y 60  //(global.windowH / TILE_SIZE)

#define WARPER_FRAME_LIMIT 60

#define CONFIRM_NONE 0
#define CONFIRM_MOVEMENT 1
#define CONFIRM_TELEPORT 2
#define CONFIRM_ATTACK 3
#define CONFIRM_ATTACK_RESULT 4
#define CONFIRM_ENEMY_ATTACK 5

#define BATTLE_OPTION_MOVE 0
#define BATTLE_OPTION_TELEPORT 1
#define BATTLE_OPTION_ATTACK 2
#define BATTLE_OPTION_MODS 3
#define BATTLE_OPTION_PASS 4

#define WMENU_PAUSE 0
#define WMENU_PARTY 1
#define WMENU_PARTYMEMBER 4
#define WMENU_INVENTORY 2
#define WMENU_ITEM 5
#define WMENU_OPTIONS 3

bool global_debug;  //global debug flag

int main(int argc, char** argv)
{
    if (argc > 1 && (strcmp(argv[1], "--debug") || strcmp(argv[1], "-d")))
        global_debug = true;
    else
        global_debug = false;

    int error = initCoSprite("./assets/cb.bmp", "Warper", SCREEN_PX_WIDTH, SCREEN_PX_HEIGHT, "./assets/Px437_ITT_BIOS_X.ttf", TILE_SIZE, (SDL_Color) {255, 28, 198, 0xFF}, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    initCLogger(&warperLogger, "./logs/log.txt", NULL, global_debug);

    /*
    if (global_debug)
        cLogEvent(warperLogger, "DEBUG", "Warper - Startup", "Started Warper in Debug mode");
    //*/

    loadWarperOptions();

    warperTilemap tilemap;

    initCSprite(&cursorSprite, NULL, "./assets/uiTilesheet.png", 0, (cDoubleRect) {0, 0, 16, 16}, (cDoubleRect) {0, 2 * TILE_SIZE, 16, 16}, NULL, 1.0, SDL_FLIP_NONE, 0.0, true, true, NULL, 1);
    SDL_ShowCursor(false);

    bool quitAll = false;
    while(!quitAll)
    {
        //main menu
        int selection = gameDevMenu();

        if (selection == 5)
        {
            //create new cutscene

            if (createNewCutscene()) //if it returns 1, aka if we're force quitting
                selection = 8;  //treat it as a quit
        }

        if (selection == 4)
        {  //create new map
            if (createNewMap(&tilemap, TILE_SIZE)) //if it returns 1, aka if we're force quitting
                selection = 8;  //treat it as a quit
        }
        if (selection == 3)
        {  //load created test map
            importWarperTilemap(&tilemap, "./maps/testMap.txt", 0);
        }
        if (selection == 2)
        {  //create temp map
            createTestMap(&tilemap);
        }

        if (selection == 8 || selection == 5)  //if we don't want to start playing (completed a cutscene or quitting entirely)
        {
            if (selection == 8)
                quitAll = true;  //we want to immediately close the window and quit, as the user requests
        }
        else
        {
            cCamera gameCamera;
            initCCamera(&gameCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0.0, 10);  //init camera

            c2DModel mapModel_layer1, mapModel_layer2, mapModel_gridLayer;
            loadTilemapModels(tilemap, &mapModel_layer1, &mapModel_layer2);

            loadGridModel(tilemap, &mapModel_gridLayer, options.gridOpacity);

            cScene gameScene;
            initCScene(&gameScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &gameCamera, (cSprite*[1]) {&cursorSprite}, 1, (c2DModel*[3]) {&mapModel_layer1, &mapModel_layer2, &mapModel_gridLayer}, 3, NULL, 0, NULL, 0);

            warperTeam playerTeam, enemyTeam;

            //TEST squads init
            cSprite* enemySquadSprites = calloc(5, sizeof(cSprite));
            warperItem testWeapon = (warperItem) {itemMelee, 0, "Test Weapon", 1, (warperWeaponStats) {1, 1, 0}};

            warperUnit enemySquad[5];

            for(int i = 0; i < 5; i++)
            {
                initCSprite(&(enemySquadSprites[i]), NULL, "./assets/characterTilesheet.png", 6 + i,
                        (cDoubleRect) {(tilemap.width - 3 - 2 * i) * tilemap.tileSize, (tilemap.height - 6 + 2 * (i % 2)) * tilemap.tileSize, 44, 96},
                        (cDoubleRect) {0, 3 * tilemap.tileSize / 2, 44, 96},
                        NULL, 1.0, SDL_FLIP_NONE, 0, false, false, NULL, 4);

                enemySquad[i] = (warperUnit) {&enemySquadSprites[i], "Enemy", 1, 0, 0, 0, 0, classNone, &testWeapon, (warperStats) {1, 1, 1, 1, 1, 1, 0}, (warperBattleData) {0, statusNone, 0, 0, 0, false}};
                calculateStats(&enemySquad[i], true);  //fill in both regular and battle stats
            }

            cSprite* playerSquadSprites = calloc(5, sizeof(cSprite));
            cDoublePt testSquadPts[5] = {(cDoublePt) {3 * tilemap.tileSize, 3 * tilemap.tileSize}, (cDoublePt) {6 * tilemap.tileSize, 6 * tilemap.tileSize}, (cDoublePt) {6 * tilemap.tileSize, 3 * tilemap.tileSize}, (cDoublePt) {3 * tilemap.tileSize, 6 * tilemap.tileSize}, (cDoublePt) {9 * tilemap.tileSize, 4.5 * tilemap.tileSize}};
            warperUnit playerSquad[5];
            char* testSquadNames[5] = {"You", "Alessia", "Marc", "Samael", "Marie"};
            enum warperClass testSquadClasses[5] = {classNone, classAttacker, classAttacker, classShooter, classTechnomancer};

            cDoublePt squadSprLocations[5] = {(cDoublePt) {0, 0}, (cDoublePt) {0, tilemap.tileSize / 2}, (cDoublePt) {tilemap.tileSize / 2, 0}, (cDoublePt) {tilemap.tileSize, 0}, (cDoublePt) {tilemap.tileSize / 2, tilemap.tileSize / 2}};

            const int testUnitsLevel = 10;

            for(int i = 0; i < 5; i++)
            {
                initCSprite(&(playerSquadSprites[i]), NULL, "./assets/characterTilesheet.png", 1 + i,
                            (cDoubleRect) {testSquadPts[i].x, testSquadPts[i].y, 2 * tilemap.tileSize, 2 * tilemap.tileSize},
                            (cDoubleRect) {squadSprLocations[i].x, squadSprLocations[i].y, tilemap.tileSize / 2, tilemap.tileSize / 2},
                            NULL, 1.0, SDL_FLIP_NONE, 0, false, false, NULL, 4);

                playerSquad[i] = (warperUnit) {&(playerSquadSprites[i]), testSquadNames[i], testUnitsLevel, 0, 0, 0, 0, testSquadClasses[i], &testWeapon, (warperStats) {testUnitsLevel, testUnitsLevel, testUnitsLevel, testUnitsLevel, testUnitsLevel, testUnitsLevel, 0}, (warperBattleData) {0, statusNone, 0, 0, 0, false}};
                calculateStats(&playerSquad[i], true);  //fill in regular stats and battle stats
            }

            initWarperTeam(&playerTeam, (warperUnit*[5]) {&(playerSquad[0]), &(playerSquad[1]), &(playerSquad[2]), &(playerSquad[3]), &(playerSquad[4])}, 5, (warperItem[1]) {testWeapon}, 1, 0);
            initWarperTeam(&enemyTeam, (warperUnit*[5]) {&(enemySquad[0]), &(enemySquad[1]), &(enemySquad[2]), &(enemySquad[3]), &(enemySquad[4])}, 5, NULL, 0, 0);
            //end TEST squads init

            addSpriteToCScene(&gameScene, playerTeam.units[0]->sprite);
            addSpriteToCScene(&gameScene, enemyTeam.units[0]->sprite);

            bool quit = false;
            int controlCode = 0;

            while (!quit)
            {
                if (controlCode != 3) //if we did not try to pause from the battle loop
                    controlCode = gameLoop(tilemap, &gameScene, &playerTeam, &enemyTeam);

                if (controlCode == 2 || controlCode == 3)  //if we are going into a battle, or returning to the battle from the pause menu
                {  //WE WANT TO BATTLE
                    if (controlCode == 2)
                    {  //add in the player's squad and the enemy's squad
                        for(int i = 1; i < playerTeam.unitsSize; i++)
                            addSpriteToCScene(&gameScene, playerTeam.units[i]->sprite);

                        for(int i = 1; i < enemyTeam.unitsSize; i++)
                            addSpriteToCScene(&gameScene, enemyTeam.units[i]->sprite);
                    }

                    controlCode = battleLoop(tilemap, &gameScene, &playerTeam, &enemyTeam);

                    if (controlCode != 3)  //if we are ending the battle for good, quitting or just returning to the overworld
                    {  //clean up the sprites for the player's squad and the enemy squad
                        for(int i = 1; i < playerTeam.unitsSize; i++)
                            removeSpriteFromCScene(&gameScene, playerTeam.units[i]->sprite, -1, false);

                        for(int i = 1; i < enemyTeam.unitsSize; i++)
                            removeSpriteFromCScene(&gameScene, enemyTeam.units[i]->sprite, -1, false);

                        //show main character sprite again
                        playerTeam.units[0]->sprite->renderLayer = 4;
                    }
                }

                if (controlCode == 1 || controlCode == 3)
                {  //execute pause menu if we are trying to access it from the game or battle loops
                    int pauseCode = pauseMenu(&gameScene, &playerTeam);
                    if (pauseCode < 0)
                        controlCode = pauseCode;  //if pause menu returns that we want to quit then quit to main menu or entirely
                }

                if (controlCode < 0)
                {
                    quit = true;
                    if (controlCode == -2)
                        quitAll = true;
                }
            }

            destroyCScene(&gameScene);

            destroyWarperTeam(&playerTeam, false);
            destroyWarperTeam(&enemyTeam, false);

            free(enemySquadSprites);
            free(playerSquadSprites);

            destroyWarperTilemap(&tilemap);
        }
    }

    saveWarperOptions();

    destroyCSprite(&cursorSprite); //cursorSprite is set to be global, so it won't get destroyed automatically by destroyCScene

    closeCoSprite();

    return error;
}



int gameDevMenu()
{
    cScene menuScene;
    char* optionsArray[] = {"Alpha Build Menu", " ", "Load Test Map", "Load Created Map", "Create New Map", "Create New Cutscene", "Options", "Print Progression Info", "Quit"};
    warperTextBox menuBox;
    createMenuTextBox(&menuBox, (cDoubleRect) {TILE_SIZE, TILE_SIZE, global.windowW - 2 * TILE_SIZE, global.windowH - 2 * TILE_SIZE}, (cDoublePt) {412, 8}, 4, true, 0xFF, optionsArray, (bool[9]) {false, false, true, true, true, true, true, true, true}, 9, &(global.mainFont));

    cResource menuBoxResource;
    initCResource(&menuBoxResource, (void*) &menuBox, drawWarperTextBox, destroyWarperTextBox, 5);

    cCamera gameCamera;
    initCCamera(&gameCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0.0, 5);

    initCScene(&menuScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &gameCamera, (cSprite*[1]) {&cursorSprite}, 1, NULL, 0, (cResource*[1]) {&menuBoxResource}, 1, NULL, 0);

    cInputState input;
    //int fps = 0;
    while(menuBox.selection == -1)
    {
        input = cGetInputState(true);

        if (input.quitInput)
            menuBox.selection = 8;  //quit

        if (input.motion.x >= 0 && input.motion.x <= SCREEN_PX_WIDTH && input.motion.y >= 0 && input.motion.y <= SCREEN_PX_HEIGHT)
            updateCursorIcon(CURSOR_NORMAL);

        checkWarperTextBoxHover(&menuBox, input.motion);
        if (input.isClick)
        {
            //if we clicked
            if (menuBoxResource.renderLayer != 0)
                checkWarperTextBoxSelection(&menuBox, input.click.x, input.click.y);
        }

        if (menuBox.selection == 6)
        {
            bool optionsQuit = optionsMenu(false);
            menuBox.selection = (optionsQuit) ? 8 : -1;  //if we are trying to force quit, set equal to the quit value, else stay in the menu
        }


        if (menuBox.selection == 7)
        {
            debugPrintStatProgression();
            menuBox.selection = -1;
            playTestAnimation();
        }

        updateCursorPos(input.motion, false);

        drawCScene(&menuScene, true, true, NULL, NULL, WARPER_FRAME_LIMIT);
    }

    int selection = menuBox.selection;

    destroyCScene(&menuScene);
    updateCursorIcon(0);

    return selection;
}

void debugPrintStatProgression()
{
    //* testing stat progression
    for(int classType = 0; classType < 4; classType++)
    {
        printf("Class id %d\n", classType);
        for(int i = 1; i <= 100; i++)
        {
            warperUnit testUnit =  (warperUnit) {NULL, "Test Unit", i, 0, 0, 0, 0, classType, NULL, (warperStats) {i, i, i, i, i, i, 0}, (warperBattleData) {0, statusNone, 0, 0, 0, false}};
            calculateStats(&testUnit, true);
            printf("Character with stats level %d: HP %d, Stamina %d, Energy %d\n", i, testUnit.maxHp, testUnit.maxStamina, testUnit.maxEnergy);

            warperUnit attackingUnit = (warperUnit) {NULL, "Test Attacker", i, 0, 0, 0, 0, classTechnomancer, NULL, (warperStats) {i, i, i, i, i, i, 0}, (warperBattleData) {0, statusNone, 0, 0, 0, false}};
            warperAttackCheck checkResult = checkAttack(&attackingUnit, &testUnit, 0);

            printf("            >Attacker character dmg %d. ttk: %d\n", checkResult.damage, (int) round((double) testUnit.maxHp / checkResult.damage + 0.45));

            /*
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
            //*/
        }
        printf("-----------------\n");
    }
    //*/
}

void playTestAnimation()
{
    cSprite spr;
    warperAnimatedSprite aSpr;
    initCSprite(&spr, NULL, "./assets/characterTilesheet.png", 0, (cDoubleRect) {0, 0, 2 * TILE_SIZE, 2 * TILE_SIZE}, (cDoubleRect) {0, 0, 16, 16}, NULL, 1.0, SDL_FLIP_NONE, 0.0, false, false, &aSpr, 5);

    cDoubleRect animationRects[5] = {(cDoubleRect) {16, 0, 0, 0}, (cDoubleRect) {16, 0, 0, 0}, (cDoubleRect) {-32, 16, 0, 0}, (cDoubleRect) {16, 0, 0, 0}, (cDoubleRect) {-16, -16, 0, 0}};
    cDoubleRect finalAnimations[60];

    double rotations[10] = {15, 15, 15, 15, 15, 15, -30, -30, -30, 0};
    double finalRotations[60];

    int layerSettings[60];

    for(int i = 0; i < 60; i++)
    {
        finalAnimations[i] = (cDoubleRect) {0, 0, 0, 0};
        finalRotations[i] = 0;
        layerSettings[i] = 5;

        if (i % 12 == 0)
            finalAnimations[i] = animationRects[i / 12];

        if (i % 6 == 0)
            finalRotations[i] = rotations[i / 6];
    }

    initWarperAnimatedSprite(&aSpr, &spr, (cDoubleRect*) finalAnimations, (double*) finalRotations, NULL, NULL, NULL, layerSettings, 60, -1);

    warperAnimatedSprite anime;
    cSprite animeSprite;
    {
        char* exportedSpr = exportWarperAnimatedSprite(aSpr, -1);
        char* copiedOver = calloc(strlen(exportedSpr) + 1, sizeof(char));
        strcpy(copiedOver, exportedSpr);
        free(exportedSpr);
        printf("%s\n", copiedOver);

        anime.sprite = &animeSprite;
        int index = -1;
        importWarperAnimatedSprite(&anime, copiedOver, &index);
        free(copiedOver);
        printf("-- %d\n", index);

        //aSpr = anime;
    }

    warperActor actorOneAnimations[5];
    warperAnimation animations[5];
    cDoubleRect animationPos[5] = {(cDoubleRect) {0, 0, 64, 64}, (cDoubleRect) {64, 0, 64, 64}, (cDoubleRect) {64, 64, 64, 64}, (cDoubleRect) {128, 64, 64, 64}, (cDoubleRect) {128, 128, 64, 64}};
    warperCutscene cutscene;

    for(int i = 0; i < 5; i++)
    {
        initWarperActor(&actorOneAnimations[i], animationPos[i], &aSpr, true);
        initWarperAnimation(&animations[i], (warperActor[1]) {actorOneAnimations[i]}, 1, 12);
    }

    warperCutsceneBox emptyBox;
    initWarperCutsceneBox(&emptyBox, NULL, NULL, 0);
    warperCutsceneBox otherBoxes[2];
    warperTextBox box;
    createBattleTextBox(&box, (cDoubleRect) {0, 0, SCREEN_PX_WIDTH, SCREEN_PX_HEIGHT}, (cDoublePt) {0, 0}, 0, true, (char*[2]) {"Test", "Box"}, (bool[2]) {false, false}, 2, TILE_SIZE);
    initWarperCutsceneBox(&otherBoxes[0], (warperTextBox*[1]) {&box}, (int[1]) {6}, 1);
    initWarperCutsceneBox(&otherBoxes[1], (warperTextBox*[1]) {&box}, (int[1]) {6}, 1);
    warperCutsceneBox boxes[5] = {emptyBox, otherBoxes[0], emptyBox, emptyBox, otherBoxes[1]};
    initWarperCutscene(&cutscene, animations, boxes, 5, ".", -1);

    exportWarperCutscene(cutscene, "./assets/testCutscene.txt");

    cCamera camera;
    initCCamera(&camera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0.0, 5);

    cScene scene;
    initCScene(&scene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &camera, (cSprite*[2]) {&cursorSprite, aSpr.sprite}, 2, NULL, 0, (cResource*[2]) {otherBoxes[0].boxResources[0], otherBoxes[1].boxResources[0]}, 2, NULL, 0);

    bool quit = false;
    cInputState input;
    while(!quit)
    {
        input = cGetInputState(true);
        if (input.quitInput || input.keyStates[SDL_SCANCODE_ESCAPE] || input.keyStates[SDL_SCANCODE_RETURN])
            quit = true;

        if (input.isClick)
        {
            if (cutscene.waitingForBox)
                incrementWarperCutsceneBox(&cutscene);
        }

        if (input.isMotion)
        {
            cursorSprite.drawRect.x = input.motion.x;
            cursorSprite.drawRect.y = input.motion.y;
        }

        drawCScene(&scene, true, true, NULL, NULL, WARPER_FRAME_LIMIT);

        if (cutscene.currentAnimation < cutscene.numAnimations)
        {
            for(int i = 0; i < cutscene.animations[cutscene.currentAnimation].numActors; i++)
            {  //iterate through each actor in the current animation step
                if (!(cutscene.waitingForBox && cutscene.animations[cutscene.currentAnimation].actors[i].pauseAnimationWhenWaiting))  //if we aren't supposed to pause animations for this sprite when the textbox is open and the textbox is indeed open
                    iterateWarperAnimatedSprite(cutscene.animations[cutscene.currentAnimation].actors[i].animatedSpr);  //iterate the animation of the sprite
            }
        }

        iterateWarperCutscene(&cutscene);
    }

    destroyWarperCutscene(&cutscene, true, true, false);
    destroyWarperAnimatedSprite(&aSpr, false);
    destroyCScene(&scene);
}

void createTestMap(warperTilemap* tilemap)
{
    tilemap->width = TEST_TILEMAP_X;
    tilemap->height = TEST_TILEMAP_Y;
    tilemap->tileSize = TILE_SIZE;

    tilemap->spritemap_layer1 = calloc(tilemap->width, sizeof(int*));
    tilemap->spritemap_layer2 = calloc(tilemap->width, sizeof(int*));
    tilemap->collisionmap = calloc(tilemap->width, sizeof(int*));

    for(int x = 0; x < tilemap->width; x++)
    {
        tilemap->spritemap_layer1[x] = calloc(tilemap->height, sizeof(int));
        tilemap->spritemap_layer2[x] = calloc(tilemap->height, sizeof(int));
        tilemap->collisionmap[x] = calloc(tilemap->height, sizeof(int));

        for(int y = 0; y < tilemap->height; y++)
        {
            tilemap->spritemap_layer1[x][y] = 4;  //normal sprite
            tilemap->spritemap_layer2[x][y] = 798;  //invisible sprite?

            if (x == 1)  //left row sprite
                tilemap->spritemap_layer1[x][y] = 1;

            if (y == 1)  //top row sprite
                tilemap->spritemap_layer1[x][y] = 3;

            if (x == tilemap->width - 2)  //right row sprite
                tilemap->spritemap_layer1[x][y] = 7;

            if (y == tilemap->height - 2)  //bottom row sprite
                tilemap->spritemap_layer1[x][y] = 5;

            if (x == 0 || y == 0 || x + 1 == tilemap->width || y + 1 == tilemap->height || (y == 20 && x > tilemap->width / 2) || (y == 40 && x < tilemap->height / 2))
            {
                if (x == 0 || y == 0 || x + 1 == tilemap->width || y + 1 == tilemap->height)
                    tilemap->spritemap_layer1[x][y] = 36;  //skyscraper sprite
                else
                    tilemap->spritemap_layer1[x][y] = 13; //collision sprite

                tilemap->collisionmap[x][y] = 1;
            }
            else
                tilemap->collisionmap[x][y] = 0;

            if (x == 1 && y == 1)  //top-left corner sprite
                tilemap->spritemap_layer1[x][y] = 0;

            if (x == tilemap->width - 2 && y == tilemap->height - 2)  //bottom-right corner sprite
                tilemap->spritemap_layer1[x][y] = 8;

            if (x == 1 && y == tilemap->height - 2)  //bottom-left corner sprite
                tilemap->spritemap_layer1[x][y] = 2;

            if (x == tilemap->width - 2 && y == 1)  //top-right corner sprite
                tilemap->spritemap_layer1[x][y] = 6;
        }
    }
}

int gameLoop(warperTilemap tilemap, cScene* gameScene, warperTeam* playerTeam, warperTeam* enemyTeam)
{
    cSprite* overworldSprite = playerTeam->units[0]->sprite;
    cSprite* overworldEnemySprite = enemyTeam->units[0]->sprite;

    int controlCode = 0;

    bool quit = false;

    cInputState input;
    int framerate = 0;

    while(!quit)
    {
        input = cGetInputState(true);

        if (input.quitInput || input.keyStates[SDL_SCANCODE_ESCAPE])
        {
            quit = true;
            if (input.quitInput)
                controlCode = -2;  //quit EVERYTHING
            if (input.keyStates[SDL_SCANCODE_ESCAPE])
                controlCode = 1;
        }

        //character movement
        if (input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_S] || input.keyStates[SDL_SCANCODE_D])
        {
            //double lastX = overworldSprite->drawRect.x, lastY = overworldSprite->drawRect.y;
            double speed = 6.0;  //just a good speed value, nothing special. Pixels/frame at 60 FPS
            if ((input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_S]) && (input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_D]))
                speed *= sin(degToRad(45));  //diagonal speed component

            if (input.keyStates[SDL_SCANCODE_W])
                overworldSprite->drawRect.y -= (int) (speed * 60.0 / framerate);

            if (input.keyStates[SDL_SCANCODE_S])
                overworldSprite->drawRect.y += (int) (speed * 60.0 / framerate);

            cDoubleVector mtv = getTilemapCollision(*overworldSprite, tilemap);

            if (mtv.magnitude)
            {  //apply collision after doing y movements
                //*
                overworldSprite->drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                overworldSprite->drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);
                //*/
                //overworldSprite->drawRect.y = lastY;
            }

            if (input.keyStates[SDL_SCANCODE_A])
            {
                overworldSprite->drawRect.x -= (int) (speed * 60.0 / framerate);
                overworldSprite->flip = SDL_FLIP_HORIZONTAL;
            }

            if (input.keyStates[SDL_SCANCODE_D])
            {
                overworldSprite->drawRect.x += (int) (speed * 60.0 / framerate);
                overworldSprite->flip = SDL_FLIP_NONE;
            }

            mtv = getTilemapCollision(*overworldSprite, tilemap);

            if (mtv.magnitude)
            {  //apply collision again after doing x movements (allows smooth collision sliding. The only way I could figure out how to fix it without 100% hard-coding)
                //*
                overworldSprite->drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                overworldSprite->drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);
                //*/
                //overworldSprite->drawRect.x = lastX;
            }

            gameScene->camera->rect.x = (int) (overworldSprite->drawRect.x - gameScene->camera->rect.w / 2);  //set the camera to center on the player
            gameScene->camera->rect.y = (int) (overworldSprite->drawRect.y - gameScene->camera->rect.h / 2);

            if (gameScene->camera->rect.y < 0)  //if the camera is set out of bounds in the -y, fix it
                gameScene->camera->rect.y = 0;

            if (gameScene->camera->rect.y > (tilemap.height - gameScene->camera->rect.h / tilemap.tileSize) * tilemap.tileSize)  //if the camera is set out of bounds in the +y, fix it
                gameScene->camera->rect.y = (tilemap.height - gameScene->camera->rect.h / tilemap.tileSize) * tilemap.tileSize;

            if (gameScene->camera->rect.x < 0)  //if the camera is set out of bounds in the -x, fix it
                gameScene->camera->rect.x = 0;

            if (gameScene->camera->rect.x > (tilemap.width - gameScene->camera->rect.w / tilemap.tileSize) * tilemap.tileSize)  //if the camera is set out of bounds in the +x, fix it
                gameScene->camera->rect.x = (tilemap.width - gameScene->camera->rect.w / tilemap.tileSize) * tilemap.tileSize;
        }

        if (input.keyStates[SDL_SCANCODE_B] || (getDistance(overworldSprite->drawRect.x, overworldSprite->drawRect.y, overworldEnemySprite->drawRect.x, overworldEnemySprite->drawRect.y) < 9 * tilemap.tileSize && overworldEnemySprite->renderLayer != 0))
        {
            //have battle take place
            quit = true;
            controlCode = 2;  //battle control code
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

        updateCursorPos(input.motion, false);

        drawCScene(gameScene, true, true, NULL, &framerate, 60);
    }

    return controlCode;
}

int pauseMenu(cScene* gameScene, warperTeam* playerTeam)
{
    int menuLevel = 0, menuId = WMENU_PAUSE, subMenuSelection = -1, menuOptsLength = 7;
    warperTextBox menuLayers[3];
    warperTextBox* pauseBox;
    char* optionsArray[] = {"PAUSE", " ", "Resume", "Party", "Items", "Options", "Quit"};
    bool isOptions[] = {false, false, true, true, true, true, true};
    createMenuTextBox(&menuLayers[0], (cDoubleRect) {0, 0, global.windowW, global.windowH}, (cDoublePt) {0, 8}, 6, true, 0xB0, optionsArray, isOptions, menuOptsLength, &global.mainFont);

    pauseBox = &menuLayers[0]; //start keeping track of menu layers

    cResource pauseBoxResource;
    initCResource(&pauseBoxResource, (void*) pauseBox, drawWarperTextBox, destroyWarperTextBox, 2);

    addResourceToCScene(gameScene, &pauseBoxResource);

    cInputState input;
    //int fps = 0;
    while(pauseBox->selection == -1)
    {
        input = cGetInputState(true);

        if (input.quitInput)
            pauseBox->selection = -3;  //quit

        if (input.motion.x >= 0 && input.motion.x <= SCREEN_PX_WIDTH && input.motion.y >= 0 && input.motion.y <= SCREEN_PX_HEIGHT)
            updateCursorIcon(CURSOR_NORMAL);

        checkWarperTextBoxHover(pauseBox, input.motion);
        if (input.isClick)
        {
            //if we clicked
            if (pauseBoxResource.renderLayer != 0)
                checkWarperTextBoxSelection(pauseBox, input.click.x, input.click.y);
        }

        updateCursorPos(input.motion, false);

        drawCScene(gameScene, true, true, NULL, NULL, WARPER_FRAME_LIMIT);

        if (menuId == WMENU_PAUSE)
        {
            if (pauseBox->selection > 2 && pauseBox->selection < 6)
            {
                menuLevel++;
                if (pauseBox->selection == 3)
                {
                    //party menu
                    menuId = WMENU_PARTY;
                    menuOptsLength = 4 + playerTeam->unitsSize;
                    char** partyArray = calloc(menuOptsLength, sizeof(char*));
                    partyArray[0] = "PARTY";
                    partyArray[1] = calloc(9 + digits(playerTeam->money), sizeof(char));
                    snprintf(partyArray[1], 8 + digits(playerTeam->money), "Money: %d", playerTeam->money);
                    partyArray[2] = " ";
                    for(int i = 0; i < playerTeam->unitsSize; i++)
                    {
                        partyArray[3 + i] = calloc(strlen(playerTeam->units[i]->name) + 1, sizeof(char));
                        strcpy(partyArray[3 + i], playerTeam->units[i]->name);
                    }
                    partyArray[menuOptsLength - 1] = "Back";

                    bool* partyIsOptions = calloc(menuOptsLength, sizeof(bool));
                    partyIsOptions[0] = false;
                    partyIsOptions[1] = false;
                    partyIsOptions[2] = false;
                    for(int i = 0; i < playerTeam->unitsSize; i++)
                    {
                        partyIsOptions[3 + i] = true;
                    }
                    partyIsOptions[menuOptsLength - 1] = true;

                    createMenuTextBox(&menuLayers[menuLevel], (cDoubleRect) {0, 0, global.windowW, global.windowH}, (cDoublePt) {0, 8}, 6, true, 0xB0, partyArray, partyIsOptions, menuOptsLength, &global.mainFont);
                    for(int i = 0; i < menuOptsLength; i++)
                    {
                        if (i != 0 && i != 2 && i != 3 + playerTeam->unitsSize)
                            free(partyArray[i]);
                    }
                    free(partyArray);
                    free(partyIsOptions);

                    pauseBox = &menuLayers[menuLevel];  //set the new menu to be what the loop accesses
                    pauseBoxResource.subclass = pauseBox;  //set the new menu to be displayed
                }

                if (pauseBox->selection == 4)
                {
                    //inventory menu
                    menuId = WMENU_INVENTORY;
                    menuOptsLength = (3 + playerTeam->inventorySize > 20) ? 20 : 3 + playerTeam->inventorySize;
                    //bool menuOverflow = 3 + playerTeam->inventorySize > 20;
                    char** inventoryArray = calloc(menuOptsLength, sizeof(char*));

                    inventoryArray[0] = "INVENTORY";
                    inventoryArray[1] = " ";

                    for(int i = 0; i < menuOptsLength - 3; i++)
                    {
                        inventoryArray[i + 2] = calloc(strlen(playerTeam->inventory[i].name), sizeof(char));
                        strcpy(inventoryArray[i + 2], playerTeam->inventory[i].name);
                    }

                    /* add pagination options
                    if (menuOverflow)
                        inventoryArray[menuOptsLength - 1] = "Next;"
                    //*/
                    inventoryArray[menuOptsLength - 1] = "Back";

                    bool* inventoryIsOptions = calloc(menuOptsLength, sizeof(bool));

                    inventoryIsOptions[0] = false;
                    inventoryIsOptions[1] = false;

                    for(int i = 0; i < menuOptsLength - 3; i++)
                        inventoryIsOptions[i + 2] = true;

                    inventoryIsOptions[menuOptsLength - 1] = true;

                    createMenuTextBox(&menuLayers[menuLevel], (cDoubleRect) {0, 0, global.windowW, global.windowH}, (cDoublePt) {512, 8}, 0, false, 0xB0, inventoryArray, inventoryIsOptions, menuOptsLength, &global.mainFont);

                    for(int i = 0; i < menuOptsLength - 3; i++)
                    {
                        free(inventoryArray[i + 2]);
                    }

                    free(inventoryArray);
                    free(inventoryIsOptions);

                    pauseBox = &menuLayers[menuLevel];  //set the new menu to be what the loop accesses
                    pauseBoxResource.subclass = pauseBox;  //set the new menu to be displayed
                }

                if (pauseBox->selection == 5)
                {
                    //options menu
                    if(optionsMenu(true))  //run the options menu and if it returns true (a force quit)
                        pauseBox->selection = -2;  //quit the game
                }
                if (pauseBox->selection > -1)  //if we aren't trying to force quit (or, as implied by the if statement encapsulating, leave the pause menu)
                    pauseBox->selection = -1;  //do not leave the loop
            }
            if (pauseBox->selection == 6)
                pauseBox->selection = -2; //quit the game
        }

        if (menuId == WMENU_PARTY && pauseBox->selection > 2)
        {
            if (pauseBox->selection == menuOptsLength - 1)
            {
                //back to pause menu
                destroyWarperTextBox((void*) &menuLayers[menuLevel]);
                menuLevel--;
                menuId = WMENU_PAUSE;
                pauseBox = &menuLayers[menuLevel];  //set the previous menu to be what the loop accesses
                pauseBoxResource.subclass = pauseBox;  //set the previous menu to be displayed

            }
            else
            {
                //go into a party member's menu
                subMenuSelection = pauseBox->selection - 3;
                menuId = WMENU_PARTYMEMBER;
                menuLevel++;
                char* memberArray[12] = {"Name", " ", "Class", "HP", "Attack", "Speed", "TP Range", "Tech Affinity", "Luck", "Stat Pts", "Gear", "Back"};
                memberArray[0] = calloc(strlen(playerTeam->units[subMenuSelection]->name), sizeof(char));
                strcpy(memberArray[0], playerTeam->units[subMenuSelection]->name);

                bool memberIsOptions[12] = {false, false, false, false, false, false, false, false, false, false, false, true};
                createMenuTextBox(&menuLayers[menuLevel], (cDoubleRect) {0, 0, global.windowW, global.windowH}, (cDoublePt) {0, 8}, 6, true, 0xB0, memberArray, memberIsOptions, 12, &global.mainFont);

                free(memberArray[0]);

                pauseBox = &menuLayers[menuLevel];  //set the new menu to be what the loop accesses
                pauseBoxResource.subclass = pauseBox;  //set the new menu to be displayed
            }
            pauseBox->selection = -1; //do not leave the loop
        }

        if (menuId == WMENU_PARTYMEMBER && pauseBox->selection == 11)
        {
            //back to party menu
            destroyWarperTextBox((void*) &menuLayers[menuLevel]);
            menuLevel--;
            menuId = WMENU_PARTY;
            pauseBox = &menuLayers[menuLevel];  //set the previous menu to be what the loop accesses
            pauseBoxResource.subclass = pauseBox;  //set the previous menu to be displayed
            pauseBox->selection = -1;
        }

        if (menuId == WMENU_INVENTORY && pauseBox->selection > 2)
        {
            if (pauseBox->selection == menuOptsLength - 1)
            {
                //back to pause menu
                destroyWarperTextBox((void*) &menuLayers[menuLevel]);
                menuLevel--;
                menuId = WMENU_PAUSE;
                pauseBox = &menuLayers[menuLevel];
                pauseBoxResource.subclass = pauseBox;
                pauseBox->selection = -1;
            }
            else
            {
                //choose the item
            }
        }
    }

    int selection = pauseBox->selection;
    if (selection < -1)
        selection++;  //-2 -> -1 (return to menu), -3 -> -2 (exit entirely)
    else
        selection = 0;

    removeResourceFromCScene(gameScene, &pauseBoxResource, -1, true);  //this will destroy pauseBox so we need to save its selection value
    updateCursorIcon(0);

    return selection;
}

bool optionsMenu(bool inGame)
{
    bool quitGame = false;

    const int menuOptsLength = 7;
    warperTextBox optionsBox;
    char* optionsArray[] = {"OPTIONS", " ", NULL, NULL, NULL, NULL, "Back"};

    optionsArray[2] = calloc(17, sizeof(char));
    snprintf(optionsArray[2], 17, "Difficulty: %s", "TEST");

    optionsArray[3] = calloc(18, sizeof(char));
    snprintf(optionsArray[3], 18, "Grid Opacity: %d", (int) (options.gridOpacity / (double) GRID_MAX_OPACITY * 100.0));

    optionsArray[4] = calloc(18, sizeof(char));
    snprintf(optionsArray[4], 18, "Music Volume: %d", (int) (options.musicVolume / (double) MIX_MAX_VOLUME * 100.0));

    optionsArray[5] = calloc(21, sizeof(char));
    snprintf(optionsArray[5], 21, "Sound FX Volume: %d", (int) (options.soundFxVolume / (double) MIX_MAX_VOLUME * 100.0));

    bool isOptions[] = {false, false, false, false, false, false, true};
    createMenuTextBox(&optionsBox, (cDoubleRect) {0, 0, global.windowW, global.windowH}, (cDoublePt) {0, 8}, 6, true, 0xB0, optionsArray, isOptions, menuOptsLength, &global.mainFont);

    for(int i = 2; i < 6; i++)
        free(optionsArray[i]);

    cResource optionsBoxRes;
    initCResource(&optionsBoxRes, (void*) &optionsBox, drawWarperTextBox, destroyWarperTextBox, 3);

    cCamera optionsCamera;
    initCCamera(&optionsCamera, (cDoubleRect) {0, 0, global.windowW, global.windowH}, 1.0, 0.0, 5);

    cScene optionsScene;
    initCScene(&optionsScene, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, &optionsCamera, (cSprite*[1]) {&cursorSprite}, 1, NULL, 0, (cResource*[1]) {&optionsBoxRes}, 1, NULL, 0);

    optionsBox.selection = -1;

    cInputState input;
    //int fps = 0;
    while(optionsBox.selection == -1)
    {
        input = cGetInputState(true);

        if (input.quitInput)
            optionsBox.selection = -2;  //quit

        if (input.motion.x >= 0 && input.motion.x <= SCREEN_PX_WIDTH && input.motion.y >= 0 && input.motion.y <= SCREEN_PX_HEIGHT)
            updateCursorIcon(CURSOR_NORMAL);

        checkWarperTextBoxHover(&optionsBox, input.motion);
        if (input.isClick)
        {
            //if we clicked
            if (optionsBoxRes.renderLayer != 0)
                checkWarperTextBoxSelection(&optionsBox, input.click.x, input.click.y);
        }

        updateCursorPos(input.motion, false);

        drawCScene(&optionsScene, true, true, NULL, NULL, WARPER_FRAME_LIMIT);
    }

    quitGame = (optionsBox.selection == -2);

    destroyCScene(&optionsScene);
    updateCursorIcon(0);

    saveWarperOptions();
    return quitGame;
}

int battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam)
{
    //TODO: add a battle intro animation

    bool quit = false;
    int controlCode = 0;
    int confirmMode = CONFIRM_NONE;  //used for confirming selections
    //double speed = 6.0;  //just a good speed value, nothing special. Pixels/frame at 60 FPS

    int turnCount = 1;

    const cDoubleRect textBoxDims = (cDoubleRect) {5 * tilemap.tileSize, 14 * tilemap.tileSize, 30 * tilemap.tileSize, 14 * tilemap.tileSize};

    cResource battleTextBoxRes, movePathRes, enemyMoveCircleRes, teleportCircleRes, enemyTeleportCircleRes;
    warperTextBox battleTextBox, backupTextBox;
    cDoubleRect minimizeBoxSave = {0,0,0,0};

    const int ROOT_TEXTBOX_LENGTH = 5;
    char* strings[] = {"Move", "Teleport", "Attack", "Mods", "End Turn"};
    bool isOptions[] = {true, true, true, true, true};
    createBattleTextBox(&battleTextBox, textBoxDims, (cDoublePt) {0, 0}, 0, true, strings, isOptions, ROOT_TEXTBOX_LENGTH, tilemap.tileSize);

    warperPath movePath = {.path = NULL, .pathLength = 0, .pathColor = (SDL_Color) {0, 0, 0, 0xF0}, .pathfinderWidth = 0, .pathfinderHeight = 0};
    warperUnit* pathfinderUnit = NULL;
    //flowNode** movePath = NULL;
    double moveDistance = 0;
    int pathIndex = -1;

    warperCircle enemyMoveCircle = {.radius = 0, .deltaDegrees = 10, .center = (cDoublePt) {0, 0}, .circleColor = (SDL_Color) {0, 0, 0, 0x50}, .filled = true};

    warperCircle teleportCircle = {.radius = 0, .deltaDegrees = 10, .center = (cDoublePt) {0, 0}, .circleColor = (SDL_Color) {0, 0, 0, 0x50}, .filled = true};
    warperCircle enemyTeleportCircle = {.radius = 0, .deltaDegrees = 10, .center = (cDoublePt) {0, 0}, .circleColor = (SDL_Color) {0xFF, 0, 0, 0x50}, .filled = true};
    int selectedEnemyIndex = -1;  //keeps track of which enemy we are checking movement radius for

    initCResource(&battleTextBoxRes, (void*) &battleTextBox, drawWarperTextBox, destroyWarperTextBox, 0);
    initCResource(&movePathRes, (void*) &movePath, drawWarperPath, destroyWarperPath, 0);
    initCResource(&enemyMoveCircleRes, (void*) &enemyMoveCircle, drawWarperCircle, destroyWarperCircle, 0);
    initCResource(&teleportCircleRes, (void*) &teleportCircle, drawWarperCircle, destroyWarperCircle, 0);
    initCResource(&enemyTeleportCircleRes, (void*) &enemyTeleportCircle, drawWarperCircle, destroyWarperCircle, 0);

    battleTextBox.selection = -1;

    addResourceToCScene(scene, &battleTextBoxRes);
    addResourceToCScene(scene, &movePathRes);
    addResourceToCScene(scene, &enemyMoveCircleRes);
    addResourceToCScene(scene, &teleportCircleRes);
    addResourceToCScene(scene, &enemyTeleportCircleRes);

    cSprite confirmPlayerSprite;
    cSprite unitSelectSprite;

    initCSprite(&confirmPlayerSprite, NULL, "./assets/characterTilesheet.png", 0,
                    (cDoubleRect) {-3 * tilemap.tileSize, -3 * tilemap.tileSize, 2 * tilemap.tileSize, 2 * tilemap.tileSize},
                    (cDoubleRect) {0, 2 * tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, false, NULL, 0);

    initCSprite(&unitSelectSprite, NULL, "./assets/uiTilesheet.png", 0,
                playerTeam->units[0]->sprite->drawRect,  //put this over top of the first selected unit
                (cDoubleRect) {0, 0, tilemap.tileSize, tilemap.tileSize},
                NULL, 1.0, SDL_FLIP_NONE, 0, false, false, NULL, 0);

    addSpriteToCScene(scene, &confirmPlayerSprite);
    addSpriteToCScene(scene, &unitSelectSprite);

    warperAttackCheck checkResult;

    cInputState input;
    int framerate = 60, frameCount = 0;
    int selectedUnit = -1, selectedEnemy = -1;  //used for selecting a unit or an enemy unit
    int turnEnemy = 0, enemyTarget = -1;  //used for going through enemy turns

    int nextTabFrame = 0;  //used to put a slowdown on the number of tab presses collected per frame

    const int MAX_CUSTOM_COLLISIONS_COUNT = playerTeam->unitsSize - 1 + enemyTeam->unitsSize;
    int customCollisionsCount = MAX_CUSTOM_COLLISIONS_COUNT;
    cDoubleRect* customCollisions = calloc(MAX_CUSTOM_COLLISIONS_COUNT, sizeof(cDoubleRect));

    bool playerTurn = true, pathToCursor = false;

    while(!quit)
    {
        input = cGetInputState(true);

        if (input.quitInput || input.keyStates[SDL_SCANCODE_ESCAPE])
        {
            quit = true;
            if (input.quitInput)
                controlCode = -2;
            else
                controlCode = 3;  //go to pause menu and come back here when finished
        }

        updateCursorPos(input.motion, false);
        if (input.motion.x >= 0 && input.motion.x <= SCREEN_PX_WIDTH && input.motion.y >= 0 && input.motion.y <= SCREEN_PX_HEIGHT)
            updateCursorIcon(CURSOR_NORMAL);

        if (battleTextBoxRes.renderLayer != 0)
            checkWarperTextBoxHover(&battleTextBox, input.motion);
        if (input.isClick)
        {
            //if we clicked

            if (battleTextBoxRes.renderLayer != 0 && (input.click.x > battleTextBox.rect.x && input.click.x < battleTextBox.rect.x + battleTextBox.rect.w &&
                                                      input.click.y > battleTextBox.rect.y && input.click.y < battleTextBox.rect.y + battleTextBox.rect.h))
            {  // if we clicked the text box
                checkWarperTextBoxSelection(&battleTextBox, input.click.x, input.click.y);  //saves the selection to the battleTextBox

                pathToCursor = false;

                if (input.click.button == SDL_BUTTON_RIGHT)
                {
                    battleTextBoxRes.renderLayer = 0;
                    if (confirmMode == CONFIRM_MOVEMENT || battleTextBox.selection == BATTLE_OPTION_MOVE)
                    {
                        destroyWarperPath((void*) &movePath);
                        movePathRes.renderLayer = 0;
                        pathIndex = -1;
                        pathfinderUnit = NULL;
                        pathToCursor = false;
                        confirmPlayerSprite.renderLayer = 0;
                        if (confirmMode == CONFIRM_MOVEMENT)
                        {
                            //restore previous text box
                            destroyWarperTextBox((void*) &battleTextBox);
                            initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                            destroyWarperTextBox((void*) &backupTextBox);
                        }
                    }

                    if (confirmMode == CONFIRM_TELEPORT || battleTextBox.selection == BATTLE_OPTION_TELEPORT)
                    {
                        confirmPlayerSprite.renderLayer = 0;
                        teleportCircleRes.renderLayer = 0;
                        if (confirmMode == CONFIRM_TELEPORT)
                            {
                                //restore previous text box
                                destroyWarperTextBox((void*) &battleTextBox);
                                initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                                destroyWarperTextBox((void*) &backupTextBox);
                            }
                    }

                    battleTextBox.selection = -1;
                    confirmMode = CONFIRM_NONE;
                    selectedUnit = -1;
                    unitSelectSprite.renderLayer = 0;
                }

                 if (battleTextBox.selection == BATTLE_OPTION_MOVE && !confirmMode)
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
                        createBattleTextBox(&battleTextBox, textBoxDims, (cDoublePt) {0, 0}, 0, true, strings, isOptions, ROOT_TEXTBOX_LENGTH, tilemap.tileSize);

                        turnCount++;
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
                            movePathRes.renderLayer = 0;

                            if (movePath.path)
                                destroyWarperPath((void*) &movePath);  //free current path
                        }
                    }
                }

                if (battleTextBox.selection == BATTLE_OPTION_TELEPORT && !confirmMode)
                {
                    //we want to teleport
                    teleportCircle.center.x = playerTeam->units[selectedUnit]->sprite->drawRect.x + playerTeam->units[selectedUnit]->sprite->drawRect.w / 2;
                    teleportCircle.center.y = playerTeam->units[selectedUnit]->sprite->drawRect.y + playerTeam->units[selectedUnit]->sprite->drawRect.h / 2;
                    teleportCircle.radius = playerTeam->units[selectedUnit]->battleData.energyLeft * tilemap.tileSize;
                    teleportCircleRes.renderLayer = 2;
                }
                else
                {
                    if (!(battleTextBox.selection == battleTextBox.textsSize - 1 || battleTextBox.selection == battleTextBox.textsSize - 2))  //if we're not maximizing/minimizing
                        teleportCircleRes.renderLayer = 0;
                }

                if (battleTextBox.selection == battleTextBox.textsSize - 1 || battleTextBox.selection == battleTextBox.textsSize - 2)
                {
                    //we clicked on the minimize/maximize button
                    if (battleTextBox.selection == battleTextBox.textsSize - 2 && battleTextBox.texts[battleTextBox.textsSize - 1].renderLayer == 0)
                    {
                        //save previous size and minimize
                        minimizeBoxSave = battleTextBox.rect;
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
                            //restore previous size
                            battleTextBox.rect = minimizeBoxSave;
                            for(int i = 0; i < battleTextBox.textsSize - 1; i++)
                            {
                                //show each regular text
                                battleTextBox.texts[i].renderLayer = 5;
                            }
                            battleTextBox.texts[battleTextBox.textsSize - 1].renderLayer = 0;
                        }
                        battleTextBox.selection = battleTextBox.storedSelection; //reset selection to what it was before we minimized either way
                    }
                }

                if (battleTextBox.selection == BATTLE_OPTION_MODS && !confirmMode)
                {
                    //open mods menu
                }

                if (battleTextBox.selection == BATTLE_OPTION_PASS && !confirmMode)
                {  //end turn
                    playerTurn = false;

                    //clean up movePath so the enemy can path correctly
                    destroyWarperPath((void*) &movePath);
                    movePathRes.renderLayer = 0;
                    pathIndex = -1;
                    pathfinderUnit = NULL;

                    //restore each enemy unit's battle stats (stamina, etc)
                    for(int i = 0; i < enemyTeam->unitsSize; i++)
                    {
                        enemyTeam->units[i]->battleData.staminaLeft = enemyTeam->units[i]->maxStamina;
                        enemyTeam->units[i]->battleData.energyLeft = enemyTeam->units[i]->maxEnergy;
                        enemyTeam->units[i]->battleData.teleportedOrAttacked = false;
                        //iterate on status effects
                    }

                    for(int i = 0; i < playerTeam->unitsSize; i++)
                    {
                        //what is this?
                    }

                    //set text box to be enemy turn textbox (player turn textbox will be re-instantiated from scratch)
                    destroyWarperTextBox((void*) &battleTextBox);
                    char* enemyTurnStrings[] = {"End Their Turn (Debug)"};
                    bool enemyTurnIsOptions[] = {true};
                    createBattleTextBox(&battleTextBox, textBoxDims, (cDoublePt) {0, 0}, 0, true, enemyTurnStrings, enemyTurnIsOptions, 1, tilemap.tileSize);
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

                if (confirmMode)
                {
                    bool keepConfirm = false;
                    //If we are confirming an action
                    if (battleTextBox.selection == 2 || battleTextBox.selection == 3 || (((battleTextBox.selection == 5 || battleTextBox.selection == 6) && confirmMode == CONFIRM_ATTACK)) || (battleTextBox.selection ==  4 && confirmMode == CONFIRM_ATTACK_RESULT))
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

                            if (confirmMode == CONFIRM_ENEMY_ATTACK)
                            {
                                turnEnemy++;
                                //printf("turn enemy on confirm: %d\n", turnEnemy);
                            }
                        }

                        if (battleTextBox.selection == 5)
                        {
                            //if we selected "yes" but only for attack confirm or result confirm
                            if (confirmMode == CONFIRM_ATTACK_RESULT)
                            {
                                //idk if anything goes here
                            }

                            if (confirmMode == CONFIRM_ATTACK)
                            {
                                //do attack
                                warperAttackResult result = doAttack(playerTeam->units[selectedUnit], enemyTeam->units[selectedEnemy], checkResult);
                                playerTeam->units[selectedUnit]->battleData.teleportedOrAttacked = true;
                                playerTeam->units[selectedUnit]->battleData.energyLeft = 0;

                                if (enemyTeam->units[selectedEnemy]->battleData.curHp <= 0)
                                {
                                    //enemy is dead
                                    enemyTeam->units[selectedEnemy]->sprite->renderLayer = 0;
                                    int deadEnemies = 0;
                                    for(int i = 0; i < enemyTeam->unitsSize; i++)
                                    {
                                        if (enemyTeam->units[i]->sprite->renderLayer == 0)
                                            deadEnemies++;
                                    }
                                    if (deadEnemies == enemyTeam->unitsSize)
                                        quit = true;
                                }

                                destroyWarperTextBox((void*) &battleTextBox); //backup the battle text box

                                char* attackNoticeOptions[5] = {NULL, " ", NULL, (enemyTeam->units[selectedEnemy]->sprite->renderLayer == 0) ? "(Mortally Wounded)" : " ", "OK"};
                                attackNoticeOptions[0] = calloc(50, sizeof(char));
                                snprintf(attackNoticeOptions[0], 50, "%s hit for %d damage.", playerTeam->units[selectedUnit]->name, result.damage);

                                attackNoticeOptions[2] = calloc(20, sizeof(char));  //crit message length + status message length + 1
                                strncpy(attackNoticeOptions[2], "", 20);
                                if (result.miss)
                                    strncpy(attackNoticeOptions[2], "(Missed)", 9);
                                else
                                {
                                    if (result.crit)
                                        strncat(attackNoticeOptions[2], "(Crit) ", 20);

                                    if (result.status != statusNone)
                                    {
                                        char* statusStr = calloc(11, sizeof(char));
                                        char* statusNameArr[] = STATUS_NAME_ARR;
                                        snprintf(statusStr, 11, "(%s) ", statusNameArr[result.status]);
                                        strncat(attackNoticeOptions[2], statusStr, 20);
                                    }
                                }

                                createBattleTextBox(&battleTextBox, textBoxDims, (cDoublePt) {0, 0}, 0, true, attackNoticeOptions, (bool[5]) {false, false, false, false, true}, 5, tilemap.tileSize);
                                confirmMode = CONFIRM_ATTACK_RESULT;
                                keepConfirm = true;
                                free(attackNoticeOptions[0]);
                                free(attackNoticeOptions[2]);
                            }
                        }

                        //printf("selection %d\n", battleTextBox.selection);

                        if (!keepConfirm)
                            confirmMode = CONFIRM_NONE;
                        confirmPlayerSprite.renderLayer = 0;

                        //restore textbox to regular menu
                        if (!keepConfirm)
                        {
                            destroyWarperTextBox((void*) &battleTextBox);
                            initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                            destroyWarperTextBox((void*) &backupTextBox);
                        }
                    }
                }
            }
            else
            {  //if we didn't click on the text box

                if (input.click.button == SDL_BUTTON_RIGHT)
                {  //if we're trying to close the textbox by right-clicking
                    //go ahead and reset all the textbox information and close it
                    battleTextBoxRes.renderLayer = 0;
                    selectedUnit = -1;
                    unitSelectSprite.renderLayer = 0;

                    if (confirmMode == CONFIRM_MOVEMENT || battleTextBox.selection == BATTLE_OPTION_MOVE)
                    {
                        destroyWarperPath((void*) &movePath);
                        movePathRes.renderLayer = 0;
                        pathIndex = -1;
                        pathfinderUnit = NULL;
                        pathToCursor = false;
                        confirmPlayerSprite.renderLayer = 0;
                        if (confirmMode == CONFIRM_MOVEMENT)
                        {
                            //restore previous text box
                            destroyWarperTextBox((void*) &battleTextBox);
                            initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                            destroyWarperTextBox((void*) &backupTextBox);
                        }
                    }

                    if (confirmMode == CONFIRM_TELEPORT || battleTextBox.selection == BATTLE_OPTION_TELEPORT)
                    {
                        teleportCircleRes.renderLayer = 0;
                        confirmPlayerSprite.renderLayer = 0;
                        if (confirmMode == CONFIRM_TELEPORT)
                        {
                            //restore previous text box
                            destroyWarperTextBox((void*) &battleTextBox);
                            initWarperTextBox(&battleTextBox, backupTextBox.rect, backupTextBox.outlineColor, backupTextBox.bgColor, backupTextBox.highlightColor, backupTextBox.texts, backupTextBox.isOption, backupTextBox.textsSize, true);
                            destroyWarperTextBox((void*) &backupTextBox);
                        }
                    }

                    confirmMode = CONFIRM_NONE;
                    battleTextBox.selection = -1;
                }

                if (!confirmMode)
                {
                    //if we aren't currently trying to confirm an action
                    double worldClickX = input.click.x + scene->camera->rect.x, worldClickY = input.click.y + scene->camera->rect.y;  //where we clicked on in the world
                    bool clickHadResult = false;

                    /*
                    double curSpeed = speed * 60.0 / framerate;
                    worldClickX = ((int)(worldClickX / curSpeed)) * curSpeed;
                    worldClickY = ((int)(worldClickY / curSpeed)) * curSpeed;  //bounding them each distance unit covered by 1 frame
                    //*/

                    //if we want to select a unit
                    if (input.click.button == SDL_BUTTON_RIGHT || (battleTextBoxRes.renderLayer == 0 && input.click.clicks == 2))
                    {
                        for(int i = 0; i < playerTeam->unitsSize; i++)
                        {
                            if (playerTeam->units[i]->sprite->renderLayer != 0)
                            {
                                if (worldClickX >= playerTeam->units[i]->sprite->drawRect.x
                                    && worldClickX < playerTeam->units[i]->sprite->drawRect.x + playerTeam->units[i]->sprite->drawRect.w
                                    && worldClickY >= playerTeam->units[i]->sprite->drawRect.y
                                    && worldClickY < playerTeam->units[i]->sprite->drawRect.y + playerTeam->units[i]->sprite->drawRect.h)
                                {
                                    selectedUnit = i;
                                    battleTextBoxRes.renderLayer = 2;
                                    clickHadResult = true;
                                    unitSelectSprite.renderLayer = 3;
                                    //printf("found unit %d\n", i);
                                }
                            }
                        }

                        enemyTeleportCircleRes.renderLayer = 0;
                        enemyMoveCircleRes.renderLayer = 0;
                        int enemyIndex = -1;
                        for(int i = 0; i < enemyTeam->unitsSize; i++)
                        {
                            if (enemyTeam->units[i]->sprite->renderLayer != 0)
                            {
                                if (worldClickX >= enemyTeam->units[i]->sprite->drawRect.x && worldClickX < enemyTeam->units[i]->sprite->drawRect.x + enemyTeam->units[i]->sprite->drawRect.w &&
                                    worldClickY >= enemyTeam->units[i]->sprite->drawRect.y && worldClickY < enemyTeam->units[i]->sprite->drawRect.y + enemyTeam->units[i]->sprite->drawRect.h)
                                {
                                    enemyIndex = i;
                                    clickHadResult = true;
                                }
                            }
                        }

                        if (enemyIndex > -1)
                        {
                            //printf("found\n");
                            if (turnCount > 1) //turn 2 (one of their turns if you're going first) or later
                            {
                                enemyTeleportCircle.center.x = enemyTeam->units[enemyIndex]->sprite->drawRect.x + enemyTeam->units[enemyIndex]->sprite->drawRect.w / 2;
                                enemyTeleportCircle.center.y = enemyTeam->units[enemyIndex]->sprite->drawRect.y + enemyTeam->units[enemyIndex]->sprite->drawRect.h / 2;
                                enemyTeleportCircle.radius = enemyTeam->units[enemyIndex]->maxEnergy * tilemap.tileSize;
                                enemyTeleportCircleRes.renderLayer = 2;
                            }
                            enemyMoveCircle.center.x = enemyTeam->units[enemyIndex]->sprite->drawRect.x + enemyTeam->units[enemyIndex]->sprite->drawRect.w / 2;
                            enemyMoveCircle.center.y = enemyTeam->units[enemyIndex]->sprite->drawRect.y + enemyTeam->units[enemyIndex]->sprite->drawRect.h / 2;
                            enemyMoveCircle.radius = enemyTeam->units[enemyIndex]->maxStamina * tilemap.tileSize;
                            enemyMoveCircleRes.renderLayer = 2;
                        }
                        selectedEnemyIndex = enemyIndex;  //reset if not found, otherwise will set to the enemy we clicked on
                    }

                    //if we want to move
                    if ((battleTextBox.selection == BATTLE_OPTION_MOVE || battleTextBox.selection == BATTLE_OPTION_TELEPORT) && playerTurn)
                    {  //move or teleport
                        /*if (movePath.path == NULL)
                        {*/
                            //printf("move\n");
                            clickHadResult = true;
                            char* questionStr = NULL;
                            double prevConfirmX = confirmPlayerSprite.drawRect.x, prevConfirmY = confirmPlayerSprite.drawRect.y;

                            if (battleTextBox.selection == BATTLE_OPTION_TELEPORT)
                            {
                                confirmPlayerSprite.drawRect.x = worldClickX - playerTeam->units[selectedUnit]->sprite->drawRect.w / 2;
                                confirmPlayerSprite.drawRect.y = worldClickY - playerTeam->units[selectedUnit]->sprite->drawRect.h / 2;
                            }

                            cDoubleVector mtv = getTilemapCollision(confirmPlayerSprite, tilemap);  //check if we can move there

                            if (mtv.magnitude)
                            {  //if there was a collision
                                //printf("no\n");
                                confirmPlayerSprite.drawRect.x = prevConfirmX;
                                confirmPlayerSprite.drawRect.y = prevConfirmY;

                                if (battleTextBox.selection == BATTLE_OPTION_MOVE)
                                {  //show the previous valid path if we are walking
                                    pathToCursor = false;

                                    if (movePath.path)
                                    {
                                        moveDistance = (int) round(movePath.path[0].distance);
                                        movePathRes.renderLayer = 2;
                                        movePath.pathfinderWidth = (int) playerTeam->units[selectedUnit]->sprite->drawRect.w;
                                        movePath.pathfinderHeight = (int) playerTeam->units[selectedUnit]->sprite->drawRect.h;
                                        pathfinderUnit = playerTeam->units[selectedUnit];  //set the pathfinder so that we can reference it when the movement is confirmed and the unit starts moving
                                    }

                                    if (moveDistance > 0 && moveDistance <= playerTeam->units[selectedUnit]->battleData.staminaLeft)
                                    {
                                        questionStr = calloc(61, sizeof(char));  //1 line = approx. 30 characters, and we're allowing 2 lines
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
                            }
                            else
                            {
                                //no collision; move is valid
                                moveDistance = 0;
                                //printf("start moving\n");
                                //we can move there
                                movePath.pathLength = 0;
                                pathIndex = 0;

                                if (battleTextBox.selection == BATTLE_OPTION_MOVE)
                                {
                                    pathToCursor = false;

                                    if (movePath.path)
                                        destroyWarperPath((void*) &movePath);  //free current path before we make a new one
                                    //if we're moving, do a search for the correct path
                                    //*
                                    int customArrPos = 0;
                                    for(int i = 0; i < playerTeam->unitsSize; i++)  //copy over all living player-team rects (except current)
                                    {
                                        if (i != selectedUnit && playerTeam->units[i]->sprite->renderLayer != 0)
                                            customCollisions[customArrPos++] = playerTeam->units[i]->sprite->drawRect;
                                    }

                                    for(int i = 0; i < enemyTeam->unitsSize; i++)  //copy over all living enemy-team rects
                                    {
                                        if (enemyTeam->units[i]->sprite->renderLayer != 0)
                                            customCollisions[customArrPos++] = enemyTeam->units[i]->sprite->drawRect;
                                    }

                                    customCollisionsCount = customArrPos;

                                    movePath.path = offsetBreadthFirst(tilemap, (int) playerTeam->units[selectedUnit]->sprite->drawRect.x, (int) playerTeam->units[selectedUnit]->sprite->drawRect.y,
                                                                        (int) confirmPlayerSprite.drawRect.x, (int) confirmPlayerSprite.drawRect.y,
                                                                        (int) playerTeam->units[selectedUnit]->sprite->drawRect.w, (int) playerTeam->units[selectedUnit]->sprite->drawRect.h,
                                                                        customCollisions, customCollisionsCount,
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
                                    else
                                    {
                                        //movePathRes.renderLayer = 0;
                                        //confirmPlayerSprite.renderLayer = 0;
                                        pathToCursor = true;
                                        printf("failed to find path for movement\n");
                                    }
                                    //*/

                                    //playerTeam->units[selectedUnit]->sprite->drawRect = oldRect;
                                    if (moveDistance > 0 && moveDistance <= playerTeam->units[selectedUnit]->battleData.staminaLeft)
                                    {
                                        questionStr = calloc(61, sizeof(char));  //1 line = approx. 30 characters, and we're allowing 2 lines
                                        strncpy(questionStr, "Do you want to move? It will use %d stamina.", 60);
                                        confirmMode = CONFIRM_MOVEMENT;
                                    }
                                    else
                                    {
                                        //set flag to false, reset variables, free movePath
                                        destroyWarperPath((void*) &movePath);
                                        pathIndex = -1;
                                        //confirmPlayerSprite.renderLayer = 0;
                                        //movePathRes.renderLayer = 0;
                                        pathToCursor = true;
                                        printf("failed movement along valid path: %f moveDistance\n", moveDistance);
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
                                        questionStr = calloc(61, sizeof(char));  //1 line = approx. 30 characters, and we're allowing 2 lines
                                        strncpy(questionStr, "Do you want to teleport? It will use %d energy.", 60);
                                        confirmMode = CONFIRM_TELEPORT;
                                    }
                                    else
                                    {
                                        printf("failed teleport\n");
                                    }
                                }
                            }
                            if (confirmMode)  //if we have now entered confirm mode
                            {
                                snprintf(questionStr, 60, questionStr, (int) moveDistance);
                                confirmPlayerSprite.renderLayer = 2;
                                confirmPlayerSprite.drawRect.w = playerTeam->units[selectedUnit]->sprite->drawRect.w;
                                confirmPlayerSprite.drawRect.h = playerTeam->units[selectedUnit]->sprite->drawRect.h;

                                //create confirm textbox and backup regular textbox
                                initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.outlineColor, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                                destroyWarperTextBox((void*) &battleTextBox);
                                createBattleTextBox(&battleTextBox, textBoxDims, (cDoublePt) {0, 0}, 0, false, (char* [4]) {questionStr, " ", "Yes", "No"}, (bool[4]) {false, false, true, true}, 4, tilemap.tileSize);
                                free(questionStr);
                                questionStr = NULL;
                            }

                        //}
                    }
                    if (battleTextBox.selection == BATTLE_OPTION_ATTACK && playerTurn && !playerTeam->units[selectedUnit]->battleData.teleportedOrAttacked)
                    {  //attack
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
                            clickHadResult = true;
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

                                char* questionStr = calloc(84, sizeof(char));
                                snprintf(questionStr, 83, "Attack? It will do %d dmg;\n%d%% hit chance, %d%% crit chance,\nand %d%% status chance.", checkResult.damage, (int) (100 * checkResult.hitChance), (int) (100 * checkResult.critChance), (int) (100 * checkResult.statusChance));
                                initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.outlineColor, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                                destroyWarperTextBox((void*) &battleTextBox);
                                //createBattleTextBox(&battleTextBox, textBoxDims, (char* [5]) {questionStr, " ", " ", "Yes", "No"}, (bool[5]) {false, false, false, true, true}, 5, tilemap.tileSize);

                                createBattleTextBox(&battleTextBox, (cDoubleRect) {1 * tilemap.tileSize, 1 * tilemap.tileSize, (38) * tilemap.tileSize, (18) * tilemap.tileSize}, (cDoublePt) {0, 0}, 0, false, (char* [7]) {questionStr, " ", " ", " ", " ", "Yes", "No"}, (bool[7]) {false, false, false, false, false, true, true}, 7, tilemap.tileSize);

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
                    if (!clickHadResult)
                    {
                        battleTextBoxRes.renderLayer = 0;
                        battleTextBox.selection = -1;
                    }
                }
            }
        }

        //if we should find a path non-stop to the cursor
        if (pathToCursor)
        {
            double prevConfirmX = confirmPlayerSprite.drawRect.x, prevConfirmY = confirmPlayerSprite.drawRect.y;
            if (!(input.motion.x < 0 && input.motion.y < 0))
            {
                confirmPlayerSprite.drawRect.x = input.motion.x + scene->camera->rect.x - confirmPlayerSprite.drawRect.w / 2;
                confirmPlayerSprite.drawRect.y = input.motion.y + scene->camera->rect.y - confirmPlayerSprite.drawRect.h / 2;
            }

            cDoubleVector mtv = getTilemapCollision(confirmPlayerSprite, tilemap);  //check if we can move there

            if (!mtv.magnitude)
            {
                int customArrPos = 0;
                for(int i = 0; i < playerTeam->unitsSize; i++)  //copy over all living player-team rects (except current)
                {
                    if (i != selectedUnit && playerTeam->units[i]->sprite->renderLayer != 0)
                        customCollisions[customArrPos++] = playerTeam->units[i]->sprite->drawRect;
                }

                for(int i = 0; i < enemyTeam->unitsSize; i++)  //copy over all living enemy-team rects
                {
                    if (enemyTeam->units[i]->sprite->renderLayer != 0)
                        customCollisions[customArrPos++] = enemyTeam->units[i]->sprite->drawRect;
                }

                //customArrPos = customCollisionsCount;  //this was here before and didn't cause any issues but
                customCollisionsCount = customArrPos;  //I think this is what this is supposed to be

                int pathLength = 0;

                node* path = offsetBreadthFirst(tilemap, (int) playerTeam->units[selectedUnit]->sprite->drawRect.x, (int) playerTeam->units[selectedUnit]->sprite->drawRect.y,  //current player position
                                                (int) confirmPlayerSprite.drawRect.x, (int) confirmPlayerSprite.drawRect.y,  //current mouse position
                                                (int) playerTeam->units[selectedUnit]->sprite->drawRect.w, (int) playerTeam->units[selectedUnit]->sprite->drawRect.h,
                                                customCollisions, customCollisionsCount,
                                                &pathLength, false, scene->camera);

                if (path)
                {  //init movePath
                    if ((int) round(path[0].distance) > playerTeam->units[selectedUnit]->battleData.staminaLeft)
                    {  //leave the old path and confirm sprite displayed

                        //movePathRes.renderLayer = 0; //hide the movePath
                        //confirmPlayerSprite.renderLayer = 0;  //hide the confirm sprite

                        confirmPlayerSprite.drawRect.x = prevConfirmX;
                        confirmPlayerSprite.drawRect.y = prevConfirmY;
                    }
                    else
                    {
                        if (movePath.path)  //free it if we already have one
                            destroyWarperPath((void*) &movePath);

                        movePath.path = path;
                        movePath.pathLength = pathLength;
                        movePath.pathfinderWidth = (int) playerTeam->units[selectedUnit]->sprite->drawRect.w;  //set the pathfinder's dimensions to be equal to our selected unit's dims
                        movePath.pathfinderHeight = (int) playerTeam->units[selectedUnit]->sprite->drawRect.h;

                        movePathRes.renderLayer = 2;  //ensure the path is shown
                        confirmPlayerSprite.renderLayer = 2;
                    }
                }
                else
                {
                    confirmPlayerSprite.drawRect.x = prevConfirmX;
                    confirmPlayerSprite.drawRect.y = prevConfirmY;
                }
            }
            else
            {
                //movePathRes.renderLayer = 0; //hide the movePath
                //confirmPlayerSprite.renderLayer = 0;  //hide the confirm sprite
                confirmPlayerSprite.drawRect.x = prevConfirmX;
                confirmPlayerSprite.drawRect.y = prevConfirmY;
            }
        }

        if (!playerTurn)
        {
            //TODO: ADD ENEMY AI HERE
            //calculate best move based on AI type, enemy difficulty, each unit's class, remaining health, etc
            //AI Types: Lone wolves each fight one unit, Pack wolves use 2-3 units to fight one of your units

            //SIMPLE TEST AI - just repositions next to you and attacks
            if (!movePath.path && !confirmMode)  //if we aren't currently following a path or waiting for the player to check the attack info
            {
                if (turnEnemy >= enemyTeam->unitsSize)
                {  //we have already iterated through every enemy
                    turnEnemy = 0;
                    playerTurn = true;

                    //clean up movePath pathfinder unit
                    pathfinderUnit = NULL;

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
                    createBattleTextBox(&battleTextBox, textBoxDims, (cDoublePt) {0, 0}, 0, true, strings, isOptions, ROOT_TEXTBOX_LENGTH, tilemap.tileSize);

                    turnCount++;
                }
                else
                {
                    if (enemyTeam->units[turnEnemy]->sprite->renderLayer == 0)
                        turnEnemy++;  //if this unit is dead, go to the next
                    else
                    {
                        //we still have to process the enemy turn
                        if (enemyTeam->units[turnEnemy]->sprite->renderLayer != 0)  //if the enemy unit is alive
                        {
                            //initialize customCollisions array for the turn enemy to use in finding a path
                            int customArrPos = 0;
                            for(int i = 0; i < playerTeam->unitsSize; i++)  //copy over all living player-team rects
                            {
                                if (playerTeam->units[i]->sprite->renderLayer != 0)
                                    customCollisions[customArrPos++] = playerTeam->units[i]->sprite->drawRect;
                            }

                            for(int i = 0; i < enemyTeam->unitsSize; i++)  //copy over all living enemy-team rects (except current)
                            {
                                if (i != turnEnemy && enemyTeam->units[i]->sprite->renderLayer != 0)
                                    customCollisions[customArrPos++] = enemyTeam->units[i]->sprite->drawRect;
                            }

                            customCollisionsCount = customArrPos;

                            //printf("entered enemy pathfinding\n");
                            pathfinderUnit = enemyTeam->units[turnEnemy];

                            movePath.pathfinderWidth = pathfinderUnit->sprite->drawRect.w;
                            movePath.pathfinderHeight = pathfinderUnit->sprite->drawRect.h;

                            //printf("%f, %f\n", pathfinderUnit->sprite->drawRect.x, pathfinderUnit->sprite->drawRect.y);
                            for(int targetUnit = 0; targetUnit < playerTeam->unitsSize; targetUnit++)
                            {
                                if (playerTeam->units[targetUnit]->sprite->renderLayer != 0)  //only target a living unit
                                {
                                    int pathLength = 0;
                                    double targetUnitX = playerTeam->units[targetUnit]->sprite->drawRect.x, targetUnitY = playerTeam->units[targetUnit]->sprite->drawRect.y;

                                    if (targetUnitX - pathfinderUnit->sprite->drawRect.x > pathfinderUnit->sprite->drawRect.w)
                                        targetUnitX -= pathfinderUnit->sprite->drawRect.w;  //aim to move to the closest position on the left
                                    if (targetUnitX - pathfinderUnit->sprite->drawRect.x < pathfinderUnit->sprite->drawRect.w)
                                        targetUnitX += playerTeam->units[targetUnit]->sprite->drawRect.w;  //aim to move to the closest position on the right

                                    if (fabs(playerTeam->units[targetUnit]->sprite->drawRect.x - targetUnitX) < 0.001)  //if we didn't adjust on the X direction
                                    {
                                        if (targetUnitY > pathfinderUnit->sprite->drawRect.y)
                                            targetUnitY -= pathfinderUnit->sprite->drawRect.h;  //aim to move to the closest position above
                                        if (targetUnitY < pathfinderUnit->sprite->drawRect.y)
                                            targetUnitY += playerTeam->units[targetUnit]->sprite->drawRect.h;  //aim to move to the closest position below
                                    }
                                    node* path = NULL;
                                    //if (targetUnitX > 0 && targetUnitX < tilemap.width * tilemap.tileSize && targetUnitY > 0 && targetUnitY < tilemap.height * tilemap.tileSize)
                                        path = offsetBreadthFirst(tilemap, pathfinderUnit->sprite->drawRect.x, pathfinderUnit->sprite->drawRect.y,
                                                                    targetUnitX, targetUnitY,  //use our adjusted target position here
                                                                    movePath.pathfinderWidth, movePath.pathfinderHeight,
                                                                    customCollisions, customCollisionsCount, &pathLength, false, scene->camera);
                                    /*
                                    if (path)
                                        printf("path distance = %f", path[0].distance);
                                    //*/

                                    if (path && (!movePath.path || path[0].distance < movePath.path[0].distance) && round(path[0].distance) <= enemyTeam->units[turnEnemy]->battleData.staminaLeft)  //find the closest opposing unit and path to them. Also be sure its within enemy's (max, for now) range
                                    {
                                        printf(" (this a is better path %f units vs %d stamina)\n", path[0].distance, enemyTeam->units[turnEnemy]->battleData.staminaLeft);
                                        if (movePath.path)
                                            free(movePath.path);

                                        movePath.path = path;
                                        movePath.pathLength = pathLength;
                                        enemyTarget = targetUnit;
                                        enemyTeam->units[turnEnemy]->battleData.staminaLeft -= (int) round(path[0].distance);
                                        printf("\n");
                                    }
                                    //printf(".\n");
                                }
                            }
                            if (!movePath.path)
                                turnEnemy++;  //this enemy can't move so skip it
                        }
                        //turnEnemy++;  //start working on the next enemy (no attack)
                        //printf("turn enemy after move: %d\n", turnEnemy);
                    }
                }
            }

            if (selectedEnemyIndex > -1)
            {  //update circle after movement proceeds
                enemyTeleportCircle.center.x = enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.x + enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.w / 2;
                enemyTeleportCircle.center.y = enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.y + enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.h / 2;

                enemyMoveCircle.center.x = enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.x + enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.w / 2;
                enemyMoveCircle.center.y = enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.y + enemyTeam->units[selectedEnemyIndex]->sprite->drawRect.h / 2;
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

                if (!playerTurn && turnEnemy < enemyTeam->unitsSize) //an enemy just moved (and presumably attacked)
                {  //do enemy attack
                    //printf("turn enemy on end of path: %d\n", turnEnemy);
                    double distance = getDistance(playerTeam->units[enemyTarget]->sprite->drawRect.x + playerTeam->units[enemyTarget]->sprite->drawRect.w / 2,
                                                  playerTeam->units[enemyTarget]->sprite->drawRect.y + playerTeam->units[enemyTarget]->sprite->drawRect.h / 2,
                                                  enemyTeam->units[turnEnemy]->sprite->drawRect.x + enemyTeam->units[turnEnemy]->sprite->drawRect.w / 2,
                                                  enemyTeam->units[turnEnemy]->sprite->drawRect.y + enemyTeam->units[turnEnemy]->sprite->drawRect.h / 2) / tilemap.tileSize;  //get the distance in tiles

                    warperAttackCheck enemyCheck = checkAttack(enemyTeam->units[turnEnemy], playerTeam->units[enemyTarget], distance);
                    warperAttackResult enemyResult = doAttack(enemyTeam->units[turnEnemy], playerTeam->units[enemyTarget], enemyCheck);

                    if (playerTeam->units[enemyTarget]->battleData.curHp <= 0)
                    {
                        playerTeam->units[enemyTarget]->sprite->renderLayer = 0;  //your unit has died
                        if (enemyTarget == selectedUnit)
                        {  //we must re-select a unit
                            bool stop = false;
                            while(!stop)
                            {
                                selectedUnit = (selectedUnit + 1) % playerTeam->unitsSize;
                                if (playerTeam->units[selectedUnit]->sprite->renderLayer != 0 || selectedUnit == enemyTarget)  //if we found a valid selection or we've iterated through all units already
                                    stop = true;  //stop
                            }
                        }

                        int deadUnits = 0;
                        for(int i = 0; i < playerTeam->unitsSize; i++)
                        {
                            if (playerTeam->units[i]->sprite->renderLayer == 0)
                                deadUnits++;
                        }
                        if (deadUnits == playerTeam->unitsSize)
                            quit = true;  //if all units are dead, you lost the battle
                    }

                    initWarperTextBox(&backupTextBox, battleTextBox.rect, battleTextBox.outlineColor, battleTextBox.bgColor, battleTextBox.highlightColor, battleTextBox.texts, battleTextBox.isOption, battleTextBox.textsSize, true);
                    destroyWarperTextBox((void*) &battleTextBox); //backup the battle text box

                    char* attackNoticeOptions[4] = {NULL, NULL, (playerTeam->units[enemyTarget]->sprite->renderLayer == 0) ? "(Mortally Wounded)" : " ", "OK"};
                    attackNoticeOptions[0] = calloc(31, sizeof(char));
                    snprintf(attackNoticeOptions[0], 31, "Enemy hit for %d damage.", enemyResult.damage);

                    attackNoticeOptions[1] = calloc(20, sizeof(char));  //crit message length + status message length + 1
                    strncpy(attackNoticeOptions[1], "", 20);
                    if (enemyResult.miss)
                        strncpy(attackNoticeOptions[1], "(Missed)", 9);
                    else
                    {
                        if (enemyResult.crit)
                            strncat(attackNoticeOptions[1], "(Crit) ", 20);

                        if (enemyResult.status != statusNone)
                        {
                            char* statusStr = calloc(11, sizeof(char));
                            char* statusNameArr[] = STATUS_NAME_ARR;
                            snprintf(statusStr, 11, "(%s) ", statusNameArr[enemyResult.status]);
                            strncat(attackNoticeOptions[1], statusStr, 20);
                        }
                    }

                    createBattleTextBox(&battleTextBox, textBoxDims, (cDoublePt) {0, 0}, 0, true, attackNoticeOptions, (bool[4]) {false, false, false, true}, 4, tilemap.tileSize);
                    confirmMode = CONFIRM_ENEMY_ATTACK;
                    free(attackNoticeOptions[0]);
                    free(attackNoticeOptions[1]);
                }
            }
            //*/
        }

        //update unit select sprite position
        if (selectedUnit > -1)
            unitSelectSprite.drawRect = playerTeam->units[selectedUnit]->sprite->drawRect;

        //camera movement
        if (input.keyStates[SDL_SCANCODE_W] || input.keyStates[SDL_SCANCODE_UP])
            scene->camera->rect.y -= 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_S] || input.keyStates[SDL_SCANCODE_DOWN])
            scene->camera->rect.y += 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_A] || input.keyStates[SDL_SCANCODE_LEFT])
            scene->camera->rect.x -= 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_D] || input.keyStates[SDL_SCANCODE_RIGHT])
            scene->camera->rect.x += 10 * 60 / framerate;

        if (input.keyStates[SDL_SCANCODE_TAB] && playerTurn && nextTabFrame < frameCount)
        {  //iterate through all valid units
            bool stop = false;
            while(!stop)
            {
                selectedUnit = (selectedUnit + 1) % playerTeam->unitsSize;
                if (playerTeam->units[selectedUnit]->sprite->renderLayer != 0 || selectedUnit == enemyTarget)  //if we found a valid selection or we've iterated through all units already
                    stop = true;  //stop
            }
            nextTabFrame = frameCount + 5;
            scene->camera->rect.x = playerTeam->units[selectedUnit]->sprite->drawRect.x + playerTeam->units[selectedUnit]->sprite->drawRect.w / 2 - scene->camera->rect.w / 2;
            scene->camera->rect.y = playerTeam->units[selectedUnit]->sprite->drawRect.y + playerTeam->units[selectedUnit]->sprite->drawRect.h / 2 - scene->camera->rect.h / 2;
        }

        //camera bounds correction
        if (scene->camera->rect.x < 0)
            scene->camera->rect.x = 0;
        if (scene->camera->rect.x + scene->camera->rect.w > tilemap.width * tilemap.tileSize)
            scene->camera->rect.x = tilemap.width * tilemap.tileSize - scene->camera->rect.w;

        if (scene->camera->rect.y < 0)
            scene->camera->rect.y = 0;
        if (scene->camera->rect.y + scene->camera->rect.h > tilemap.height * tilemap.tileSize)
            scene->camera->rect.y = tilemap.height * tilemap.tileSize - scene->camera->rect.h;
        //end camera bounds correction

        checkWarperUnitHover(input.motion, playerTeam, enemyTeam, scene->camera);

        if (input.keyStates[SDL_SCANCODE_F11] && selectedUnit > -1)  //DEBUG
            printf("%f, %f\n", playerTeam->units[selectedUnit]->sprite->drawRect.x, playerTeam->units[selectedUnit]->sprite->drawRect.y);

        drawCScene(scene, true, true, &frameCount, &framerate, WARPER_FRAME_LIMIT);
    }

    free(customCollisions);

    //local objects/resources must be removed before quitting
    removeResourceFromCScene(scene, &battleTextBoxRes, -1, true);
    removeResourceFromCScene(scene, &movePathRes, -1, true);
    removeResourceFromCScene(scene, &enemyMoveCircleRes, -1, true);
    removeResourceFromCScene(scene, &teleportCircleRes, -1, true);
    removeResourceFromCScene(scene, &enemyTeleportCircleRes, -1, true);
    removeSpriteFromCScene(scene, &confirmPlayerSprite, -1, true);
    removeSpriteFromCScene(scene, &unitSelectSprite, -1, true);

    return controlCode;
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

void checkWarperUnitHover(SDL_MouseMotionEvent motion, warperTeam* playerTeam, warperTeam* enemyTeam, cCamera* camera)
{
    if (motion.x >= 0 && motion.y >= 0 && motion.x <= SCREEN_PX_WIDTH && motion.y <= SCREEN_PX_HEIGHT)
    {
        for(int i = 0; i < playerTeam->unitsSize; i++)
        {
            if (playerTeam->units[i]->sprite->renderLayer != 0 && (motion.x > playerTeam->units[i]->sprite->drawRect.x - camera->rect.x && motion.x < playerTeam->units[i]->sprite->drawRect.x + playerTeam->units[i]->sprite->drawRect.w - camera->rect.x &&
                motion.y > playerTeam->units[i]->sprite->drawRect.y - camera->rect.y && motion.y < playerTeam->units[i]->sprite->drawRect.y + playerTeam->units[i]->sprite->drawRect.h - camera->rect.y))
            {
                updateCursorIcon(CURSOR_ALLY);
            }
        }

        for(int i = 0; i < enemyTeam->unitsSize; i++)
        {
            if (enemyTeam->units[i]->sprite->renderLayer != 0 && (motion.x > enemyTeam->units[i]->sprite->drawRect.x - camera->rect.x && motion.x < enemyTeam->units[i]->sprite->drawRect.x + enemyTeam->units[i]->sprite->drawRect.w - camera->rect.x &&
                motion.y > enemyTeam->units[i]->sprite->drawRect.y - camera->rect.y && motion.y < enemyTeam->units[i]->sprite->drawRect.y + enemyTeam->units[i]->sprite->drawRect.h - camera->rect.y))
            {
                updateCursorIcon(CURSOR_ENEMY);
            }
        }
    }
}
