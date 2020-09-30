#include "warper.h"
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
    int storedSelection;
} warperTextBox;

void initWarperTextBox(warperTextBox* textBox, cDoubleRect rect, SDL_Color bgColor, SDL_Color highlightColor, cText* texts, int textsSize, bool isMenu);
void drawWarperTextBox(void* textBoxSubclass, cCamera camera);
void destroyWarperTextBox(void* textBoxSubclass);
int gameLoop(warperTilemap tilemap);
bool battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam);
cDoubleVector getTilemapCollision(cSprite playerSprite, warperTilemap tilemap);

#define TILEMAP_X 80  //(global.windowW / TILE_SIZE)
#define TILEMAP_Y 60  //(global.windowH / TILE_SIZE)

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

    warperUnit playerUnit = (warperUnit) {&testPlayerSprite, 1, 0, 15, noClass, (warperStats) {1, 1, 1, 1, 1, 1}, (warperBattleData) {15, 300, false}};
    warperUnit enemyUnit = (warperUnit) {&testEnemySprite, 1, 0, 15, noClass, (warperStats) {1, 1, 1, 1, 1, 1}, (warperBattleData) {15, 300, false}};
    warperTeam* playerTeam = malloc(sizeof(warperTeam));
    initWarperTeam(playerTeam, (warperUnit*[1]) {&playerUnit}, 1, NULL, 0, 0);
    warperTeam* enemyTeam = malloc(sizeof(warperTeam));
    initWarperTeam(enemyTeam, (warperUnit*[1]) {&enemyUnit}, 1, NULL, 0, 0);
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
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, (void*) playerTeam, 4);
        initCSprite(&testEnemySprite, NULL, "assets/characterTilesheet.png", 1,
                    (cDoubleRect) {(tilemap.width - 2) * tilemap.tileSize, (tilemap.height - 2) * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize},
                    (cDoubleRect) {0, tilemap.tileSize / 2, tilemap.tileSize / 2, tilemap.tileSize / 2},
                    NULL, 1.0, SDL_FLIP_NONE, 0, false, (void*) enemyTeam, 4);
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
                          texts, textCount, true);
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
                testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);

                playerUnit.battleData.remainingDistance += mtv.magnitude;
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
                testPlayerSprite.drawRect.x += mtv.magnitude * cos(degToRad(mtv.degrees));
                testPlayerSprite.drawRect.y += mtv.magnitude * sin(degToRad(mtv.degrees));
                //printf("translating %f at %f\n", mtv.magnitude, mtv.degrees);

                playerUnit.battleData.remainingDistance -= mtv.magnitude;
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

        if (getDistance(testPlayerSprite.drawRect.x, testPlayerSprite.drawRect.y, testEnemySprite.drawRect.x, testEnemySprite.drawRect.y) < 6 * tilemap.tileSize)
        {
            //have battle take place in a seperate loop
            quit = battleLoop(tilemap, &testScene, playerTeam, enemyTeam);
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

    return (input.quitInput) ? 1 : 0;
}

bool battleLoop(warperTilemap tilemap, cScene* scene, warperTeam* playerTeam, warperTeam* enemyTeam)
{
    bool quit = false, quitEverything = false;
    int confirmMode = 0;  //used for confirming selections

    cResource battleTextBoxRes;
    warperTextBox battleTextBox;
    {
        int textCount = 4 + 2;  //3 options + the +/- buttons
        char* strings[] = {"Choose Unit", "Move", "Teleport", "Attack"};
        cText* texts = calloc(textCount, sizeof(cText));
        for(int i = 0; i < textCount - 2; i++)
        {
            initCText(&(texts[i]), strings[i], (cDoubleRect) {5 * tilemap.tileSize, (14 + i) * tilemap.tileSize, 30 * tilemap.tileSize, (14 - i) * tilemap.tileSize}, 30 * tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        }
        initCText(&(texts[textCount - 2]), "-", (cDoubleRect) {34 * tilemap.tileSize, 14 * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize}, tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 5);
        initCText(&(texts[textCount - 1]), "+", (cDoubleRect) {34 * tilemap.tileSize, 19 * tilemap.tileSize, tilemap.tileSize, tilemap.tileSize}, tilemap.tileSize, (SDL_Color) {0x00, 0x00, 0x00, 0xCF}, (SDL_Color) {0xFF, 0xFF, 0xFF, 0xFF}, NULL, 1.0, SDL_FLIP_NONE, 0, true, 0);

        initWarperTextBox(&battleTextBox, (cDoubleRect) {5 * tilemap.tileSize, 14 * tilemap.tileSize, 30 * tilemap.tileSize, 14 * tilemap.tileSize},
                          (SDL_Color) {0xFF, 0xFF, 0xFF, 0xC0}, (SDL_Color) {0xFF, 0x00, 0x00, 0xC0},
                          texts, textCount, true);
    }
    initCResource(&battleTextBoxRes, (void*) &battleTextBox, &drawWarperTextBox, &destroyWarperTextBox, 3);
    battleTextBox.selection = 0;

    addResourceToCScene(scene, &battleTextBoxRes);

    cInputState input;
    int framerate = 0;
    int selectedUnit = 0;
    node* movePath = NULL;
    int lengthOfPath = 0;
    int pathIndex = -1;

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
            if (confirmMode)
            {
                //
            }
            else
            {
                if (battleTextBoxRes.renderLayer != 0 && (input.click.x > battleTextBox.rect.x && input.click.x < battleTextBox.rect.x + battleTextBox.rect.w &&
                                                          input.click.y > battleTextBox.rect.y && input.click.y < battleTextBox.rect.y + battleTextBox.rect.h))
                {  // if we clicked the text box
                    battleTextBox.storedSelection = battleTextBox.selection;

                    for(int i = 0; i < battleTextBox.textsSize; i++)
                    {
                        if (input.click.x > battleTextBox.texts[i].rect.x && input.click.x < battleTextBox.texts[i].rect.x + battleTextBox.texts[i].rect.w &&
                            input.click.y > battleTextBox.texts[i].rect.y && input.click.y < battleTextBox.texts[i].rect.y + battleTextBox.texts[i].rect.h)
                        {
                            //we clicked on an element
                            battleTextBox.selection = i;
                        }
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
                }
                else
                {  //if we didn't click on the text box

                    double worldClickX = input.click.x + scene->camera->rect.x - playerTeam->units[selectedUnit]->sprite->drawRect.w / 2, worldClickY = input.click.y + scene->camera->rect.y - playerTeam->units[selectedUnit]->sprite->drawRect.h / 2;  //where we clicked on in the world
                    //if we want to select our unit
                    if (battleTextBox.selection == 0)
                    {
                        for(int i = 0; i < playerTeam->unitsSize; i++)
                        {
                            if (worldClickX > playerTeam->units[i]->sprite->drawRect.x && worldClickX < playerTeam->units[i]->sprite->drawRect.x + playerTeam->units[i]->sprite->drawRect.w &&
                                worldClickY > playerTeam->units[i]->sprite->drawRect.y && worldClickY < playerTeam->units[i]->sprite->drawRect.y + playerTeam->units[i]->sprite->drawRect.h)
                            {
                                selectedUnit = i;
                                //printf("found unit %d\n", i);
                            }
                        }
                    }

                    //if we want to move
                    if (battleTextBox.selection == 1 || battleTextBox.selection == 2)
                    {  //move or teleport
                        if (movePath == NULL)
                        {
                            //printf("move\n");

                            cDoubleRect oldRect = playerTeam->units[selectedUnit]->sprite->drawRect;  //save our current location rect
                            playerTeam->units[selectedUnit]->sprite->drawRect.x = worldClickX;  //move the unit there pre-maturely
                            playerTeam->units[selectedUnit]->sprite->drawRect.y = worldClickY;

                            cDoubleVector mtv = getTilemapCollision(*(playerTeam->units[selectedUnit]->sprite), tilemap);  //check if we can move there
                            //Check to see if we have enough stamina

                            if (mtv.magnitude)
                            {  //if there was a collision
                                //reset movement
                                //printf("no\n");
                                playerTeam->units[selectedUnit]->sprite->drawRect = oldRect;
                            }
                            else
                            {
                                //printf("start moving\n");
                                //we can move there
                                lengthOfPath = 0;
                                pathIndex = 0;
                                //maybe change pathing algorithm to checking in the exact direction we want to go, then navigating around walls?

                                if (battleTextBox.selection < 2)
                                {
                                    //if we're moving, do a search for the correct path
                                    movePath = BreadthFirst(tilemap, oldRect.x, oldRect.y, playerTeam->units[selectedUnit]->sprite->drawRect.x, playerTeam->units[selectedUnit]->sprite->drawRect.y, &lengthOfPath, false, NULL);
                                    if (movePath)
                                    {
                                        movePath[lengthOfPath - 1].x = worldClickX;
                                        movePath[lengthOfPath - 1].y = worldClickY;
                                    }
                                    //TODO: show the movement path and cost, and ask for confirmation
                                    playerTeam->units[selectedUnit]->sprite->drawRect = oldRect;
                                }
                            }
                        }
                    }
                    if (battleTextBox.selection == 3)
                    {  //battle
                        //find which enemy we clicked on, if any
                        int enemyIndex = -1;
                        for(int i = 0; i < enemyTeam->unitsSize; i++)
                        {
                            if (worldClickX > enemyTeam->units[i]->sprite->drawRect.x && worldClickX < enemyTeam->units[i]->sprite->drawRect.x + enemyTeam->units[i]->sprite->drawRect.w &&
                                worldClickY > enemyTeam->units[i]->sprite->drawRect.y && worldClickY < enemyTeam->units[i]->sprite->drawRect.y + enemyTeam->units[i]->sprite->drawRect.h)
                                enemyIndex = i;
                        }
                        if (enemyIndex != -1)
                        {
                            //printf("found enemy %d\n", enemyIndex);
                            //calculate if we hit, calculate damage
                            double distance = getDistance(playerTeam->units[selectedUnit]->sprite->drawRect.x, playerTeam->units[selectedUnit]->sprite->drawRect.y, enemyTeam->units[enemyIndex]->sprite->drawRect.x, enemyTeam->units[enemyIndex]->sprite->drawRect.y);
                            //...
                        }
                    }
                }
            }
        }

        if (movePath != NULL)
        {
            //move our unit until there are no more nodes
            playerTeam->units[selectedUnit]->sprite->drawRect.x = movePath[pathIndex].x;
            playerTeam->units[selectedUnit]->sprite->drawRect.y = movePath[pathIndex].y;
            //for testing, no movement limits
            //playerTeam->units[chosenUnit]->battleData.remainingDistance -= sqrt(pow(movePath[pathIndex].x, 2.0) + pow(movePath[pathIndex].x, 2.0));  //subtract out the magnitude of our movements
            pathIndex++;
            if (pathIndex >= lengthOfPath)
            {
                //set flag to false, reset variables, free movePath
                free(movePath);
                movePath = NULL;
                pathIndex = -1;
                lengthOfPath = 0;
            }
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

    return quitEverything;
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
            initCText(&(textBox->texts[i]), texts[i].str, texts[i].rect, texts[i].maxW, texts[i].textColor, texts[i].bgColor, texts[i].font, texts[i].scale, texts[i].flip, texts[i].degrees, true, texts[i].renderLayer);
        }
    }
    else
    {
        printf("ERROR: not enough mem to alloc text box\n");
    }
    textBox->isMenu = isMenu;
    textBox->selection = -1;
    textBox->storedSelection = -1;
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
    if (textBox->isMenu && textBox->selection != -1 && textBox->texts[textBox->selection].renderLayer > 0)
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
