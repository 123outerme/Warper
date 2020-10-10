#include "battleSystem.h"

void initWarperTeam(warperTeam* team, warperUnit** units, int unitsSize, warperItem* inventory, int inventorySize, int money)
{
    //TODO: copy this mem, don't do shallow copies
    team->units = units;
    team->unitsSize = unitsSize;
    team->inventory = inventory;
    team->inventorySize = inventorySize;
    team->money = money;
}

/** \brief Inits a new node
 *
 * \param nodePtr node*
 * \param x double
 * \param y double
 * \param lastNode node*
 * \param visited bool
 * \param distance double
 */
void initNode(node* nodePtr, int x, int y, node* lastNode, bool visited, double distance)
{
    nodePtr->x = x;
    nodePtr->y = y;
    nodePtr->lastNode = lastNode;
    nodePtr->visited = visited;
    nodePtr->distance = distance;
}

/** \brief Searches for a path between two points on a map with a breadth first search
 *
 * \param tilemap warperTilemap current tilemap
 * \param startX const double
 * \param startY const double
 * \param endX const double
 * \param endY const double
 * \param lengthOfPath double* - pointer to a double you want to hold the path length (in number of nodes)
 * \param drawDebug const bool debug use only, set to false
 * \return node* The fastest path you can travel through (must be manually free()'d after use)
 */
node* BreadthFirst(warperTilemap tilemap, const int startX, const int startY, const int endX, const int endY, int* lengthOfPath, const bool drawDebug, cCamera* camera)
{
    int queueSize = tilemap.width * tilemap.height + 1;
    node** queue = calloc(queueSize, sizeof(node*));
    node* path = calloc(queueSize, sizeof(node));
    if (!queue || (startX / tilemap.tileSize == endX / tilemap.tileSize && startY / tilemap.tileSize == endY / tilemap.tileSize))
    {
        *lengthOfPath = -1;
        free(path);
        if (queue)
            free(queue);
        return NULL;
    }
    node* curNode;
    node searchList[tilemap.height][tilemap.width];
    int queueCount = 0;
    for(int y = 0; y < tilemap.height; y++)
    {
        for(int x = 0; x < tilemap.width; x++)
            initNode(&(searchList[y][x]), x * tilemap.tileSize, y * tilemap.tileSize, NULL, false, 0);
    }
    curNode = &(searchList[(int) endY / tilemap.tileSize][(int) endX / tilemap.tileSize]);
    curNode->lastNode = (void*) 1; //marked as the beginning
    curNode->distance = 0;  //distance is obviously zero
    curNode->visited = true;
    bool quit = false;
    while(!quit)
    {
        //curNode->visited = true;
        for(int i = 0; i < 8; i++)  //8 directions: 0 = right, 1 = left, 2 = down, 3 = up, 4 = down-right, 5 = down-left, 6 = up-right, 7 = up-left
        {
            int x = (curNode->x / tilemap.tileSize) + (i == 0 || i == 4 || i == 6) - (i == 1 || i == 5 || i == 7);
            int y = (curNode->y / tilemap.tileSize) + (i == 2 || i == 4 || i == 5) - (i == 3 || i == 6 || i == 7);
            //printf("%s\n", boolToString(curNode->visited));
            if (!(searchList[y][x].visited) && (x >= 0 && y >= 0 && x < tilemap.width && y < tilemap.height) && tilemap.collisionmap[x][y] == 0)
            {
                if (i < 4 || (i >= 4 && tilemap.collisionmap[x - (i == 4 || i == 6) + (i == 5 || i == 7)][y] == 0 && tilemap.collisionmap[x][y - (i == 4 || i == 5) + (i == 6 || i == 7)] == 0))
                {
                    //if we aren't in a diagonal, or if we are, and the two adjacent non-diagonal tiles are free of collision
                    queue[queueCount++] = &(searchList[y][x]);
                    searchList[y][x].visited = true;
                    searchList[y][x].lastNode = (void*) curNode;
                    searchList[y][x].distance = curNode->distance + ((i < 4) ? 1 : 2 * sqrt(2));  //0-3 are cardinal directions, 4-7 are diagonal movements
                    if (((int) x == (int) startX / tilemap.tileSize) && ((int) y == (int) startY / tilemap.tileSize))
                    {  //check if node is at startX, startY. Stop if is, continue if not
                        quit = true;
                        break;
                    }
                    if (drawDebug)
                    {
                        SDL_SetRenderDrawColor(global.mainRenderer, 0xFF, 0x00, 0x00, 0xF0);
                        SDL_RenderFillRect(global.mainRenderer, &((SDL_Rect) {.x = x * tilemap.tileSize - camera->rect.x, .y = y * tilemap.tileSize - camera->rect.y, .w = tilemap.tileSize, .h = tilemap.tileSize}));
                        SDL_RenderPresent(global.mainRenderer);
                        //printf("%p\n", searchList[y][x].lastNode);
                        //waitForKey(true);
                    }
                }
            }
        }


        //enqueue valid adjacent nodes to selected node
        if (queueCount == 0)
        {
            *lengthOfPath = 0;
            free(queue);
            free(path);
            return NULL;
        }
        //check if no items are enqueued i.e. no path, quits with a NULL path if this is true

        curNode = queue[0];
        //printf("t>>(%d, %d) ... %d\n", curNode->x / tilemap.tileSize, curNode->y / tilemap.tileSize, queueCount);
        //printf("n>>(%d, %d)\n", queue[1].x / tilemap.tileSize, queue[1].y / tilemap.tileSize);

        for(int i = 0; i < queueCount - 1; i++)
            queue[i] = queue[i + 1];

        queueCount--;
        //select a new adjacent node and delete the last enqueued item
    }
    free(queue);
    quit = false;
    int pathCount = 1;
    path[0] = searchList[startY / tilemap.tileSize][startX / tilemap.tileSize];
    path[1] = path[0];
    while(!quit)
    {
        if (path[pathCount - 1].lastNode == (void*) 1)
            quit = true;
        else
        {
            //printf("%d = pathCount\n", pathCount);
            path[pathCount] = *((node*) (path[pathCount - 1].lastNode));
            //printf("%p\n", (void*) path[pathCount].lastNode);
            pathCount++;
        }
    }
    //backtrack through the path found, starting at the start node and following lastNode to the end
    *lengthOfPath = pathCount;
    return path;
}


node* offsetBreadthFirst(warperTilemap tilemap, int startX, int startY, int endX, int endY, int finderWidth, int finderHeight, int* lengthOfPath, const bool drawDebug, cCamera* camera)
{
    /* TODO: new algorithm

    breadth-first based, except nodes are created, on the fly, so that their x's and y's are (at first) offset by the same amount the start (or end?) x and y are from being aligned to the tile-grid
    except we will:
    -search the neighbor "nodes" that have no collision
    --if there is none for this node: create a neighbor node that correspond to your current node's x and y position plus the tile offset in the direction (so if it's to the right, it's tilemap.tileSize more on the X)
    --if there isn't: find a path from the current node to the nearest node snapped to the tile grid
    -if the end node (or start?) is found in this breadth-first search, then we have finished
    */
    int queueSize = 2 * tilemap.width * tilemap.height + 1;
    node** queue = calloc(queueSize, sizeof(node*));
    node* path = calloc(queueSize, sizeof(node));
    //node* pathFromCollision = NULL;
    //int pathFromCollisionSize = 0;

    if (!queue || (startX / tilemap.tileSize == endX / tilemap.tileSize && startY / tilemap.tileSize == endY / tilemap.tileSize))
    {
        *lengthOfPath = -1;
        free(path);
        if (queue)
            free(queue);
        return NULL;
    }

    *lengthOfPath = 0;
    int queueCount = 0;

    int deltaEndX = endX - ((int) endX / tilemap.tileSize) * tilemap.tileSize;  //getting the distance between the grid-aligned endX, and the requested endX
    int deltaEndY = endY - ((int) endY / tilemap.tileSize) * tilemap.tileSize;

    node* curNode;
    /*
    curNode = malloc(sizeof(node));
    initNode(curNode, endX, endY, (void*) 1, true, 0);
    //*/
    //*
    node searchList[tilemap.height][tilemap.width];

    for(int y = 0; y < tilemap.height; y++)
    {
        for(int x = 0; x < tilemap.width; x++)
            initNode(&(searchList[y][x]), x * tilemap.tileSize + deltaEndX, y * tilemap.tileSize + deltaEndY, NULL, false, -1);
    }

    searchList[startY / tilemap.tileSize][startX / tilemap.tileSize].x = startX;
    searchList[startY / tilemap.tileSize][startX / tilemap.tileSize].y = startY;  //setting our start position to its own offset

    curNode = &(searchList[(int) endY / tilemap.tileSize][(int) endX / tilemap.tileSize]);
    curNode->lastNode = (void*) 1;  //marked as the beginning
    curNode->distance = 0;  //distance is obviously zero
    curNode->visited = true;
    //*/

    int checkStartX = -1, checkStartY = -1;

    bool quit = false;
    while(!quit)
    {
        //loop through all the groups of tiles adjacent to curNode and start creating new nodes
        for(int i = 0; i < 8; i++)
        {
            //angle to next node = i * (360 / 8) = i * 45
            int nextX = curNode->x + tilemap.tileSize * (i == 7 || i == 0 || i == 1) - tilemap.tileSize * (i == 3 || i == 4 || i == 5);
            int nextY = curNode->y + tilemap.tileSize * (i > 4) - tilemap.tileSize * (i == 1 || i == 2 || i == 3);

            //loop through all of the groups of tiles adjacent to curNode and check for collision
            bool isThereCollision = false;
            for(int y = nextY / tilemap.tileSize; y <= nextY / tilemap.tileSize + (finderHeight / tilemap.tileSize) + 1 * (deltaEndY != 0); y++)
            {
                for(int x = nextX / tilemap.tileSize; x <= nextX / tilemap.tileSize + (finderWidth / tilemap.tileSize) + 1 * (deltaEndX != 0); x++)
                {
                    if (x >= 0 && x < tilemap.width && y >= 0 && y < tilemap.height)
                    {

                        if (tilemap.collisionmap[x][y] == 1)
                        {
                            isThereCollision = true;
                        }
                    }
                }
            }

            if (!isThereCollision)
            {
                //we're free to keep moving on
                if (nextX >= 0 && nextX < tilemap.width * tilemap.tileSize && nextY >= 0 && nextY < tilemap.height * tilemap.tileSize && !searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].visited)
                {
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].visited = true;
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance = curNode->distance + ((i % 2 == 0) ? 1 : sqrt(2));

                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].lastNode = curNode;

                    queue[queueCount] = malloc(sizeof(node));
                    initNode(queue[queueCount], searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].x, searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].y,
                             searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].lastNode, true, searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance);
                    queueCount++;
                    if (drawDebug)
                    {
                        SDL_SetRenderDrawColor(global.mainRenderer, 0x00, 0x00, 0xFF, 0xC0);
                        SDL_RenderFillRect(global.mainRenderer, &((SDL_Rect) {.x = nextX - camera->rect.x, .y = nextY - camera->rect.y, .w = tilemap.tileSize, .h = tilemap.tileSize}));
                        SDL_RenderPresent(global.mainRenderer);
                        //printf("%p\n", searchList[y][x].lastNode);
                        //waitForKey(true);
                    }
                }

            }
            else
            {
                //adjust the tile we're checking to align to the tilemap.tileSize grid, but do not add it to the queue
                if (nextX >= 0 && nextX < tilemap.width * tilemap.tileSize && nextY >= 0 && nextY < tilemap.height * tilemap.tileSize && !searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].visited)
                {
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].x = ((int) searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].x / tilemap.tileSize) * tilemap.tileSize;
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].y = ((int) searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].y / tilemap.tileSize) * tilemap.tileSize;
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].visited = true;
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance = curNode->distance + ((i % 2 == 0) ? 1 : sqrt(2));

                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].lastNode = curNode;

                    bool collision = false;
                    for(int y = 0; y <= (finderHeight - 1) / tilemap.tileSize; y++) //from 1 pixel to 1 tile, run once. When greater than 1 tile up to 2 tiles, run twice, from 2-3, run 3 times, etc
                    {
                        for(int x = 0; x <= (finderWidth - 1) / tilemap.tileSize; x++)  //that's why I subtract one from the width and height
                        {
                            //if there is a collision within the bounds of the pathfinder, then we do not want to add this node to the queue
                            if (tilemap.collisionmap[nextX / tilemap.tileSize + x][nextY / tilemap.tileSize + y] == 1)
                                collision = true;
                        }
                    }

                    if (!collision)
                    {
                        queue[queueCount] = malloc(sizeof(node));
                        initNode(queue[queueCount], searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].x, searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].y,
                                 searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].lastNode, true, searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance);
                        queueCount++;
                        if (drawDebug)
                        {
                            SDL_SetRenderDrawColor(global.mainRenderer, 0x00, 0xFF, 0x00, 0xC0);
                            SDL_RenderFillRect(global.mainRenderer, &((SDL_Rect) {.x = searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].x - camera->rect.x, .y = searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].y - camera->rect.y,
                                                                                  .w = tilemap.tileSize, .h = tilemap.tileSize}));
                            SDL_RenderPresent(global.mainRenderer);
                            //printf("%p\n", searchList[y][x].lastNode);
                            //waitForKey(true);
                        }
                    }
                    else
                        searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance = -1;


                }
            }

            if (abs(nextX - startX) <= tilemap.tileSize / 2 && abs(nextY - startY) <= tilemap.tileSize / 2)
            {  //check if node is at startX, startY plus or minus half a tile. Stop if is, continue if not
                checkStartX = nextX / tilemap.tileSize;
                checkStartY = nextY / tilemap.tileSize;
                quit = true;
                break;
            }
        }

        //enqueue valid adjacent nodes to selected node
        if (queueCount == 0)
        {
            *lengthOfPath = 0;
            free(queue);
            free(path);
            return NULL;
        }
        //check if no items are enqueued i.e. no path, quits with a NULL path if this is true

        curNode = queue[0];
        //printf("t>>(%d, %d) ... %d\n", curNode->x / tilemap.tileSize, curNode->y / tilemap.tileSize, queueCount);
        //printf("n>>(%d, %d)\n", queue[1].x / tilemap.tileSize, queue[1].y / tilemap.tileSize);

        for(int i = 0; i < queueCount - 1; i++)
            queue[i] = queue[i + 1];

        queueCount--;
        //select a new adjacent node and delete the last enqueued item
    }

    for(int i = 0; i < queueCount; i++)
    {
        free(queue[i]);
    }
    free(queue);

    /*Backtrack through the found path(s) and see if there are better nodes to travel through
    curNode = &(searchList[checkStartY][checkStartX]);
    quit = false;

    while(!quit)
    {
        node* nextNode = NULL;
        double minDistance = -1;
        int foundIVal = -1;
        for(int i = 0; i < 8; i++)
        {
            //angle to next node = i * (360 / 8) = i * 45
            int nextX = curNode->x + tilemap.tileSize * (i == 7 || i == 0 || i == 1) - tilemap.tileSize * (i == 3 || i == 4 || i == 5);
            int nextY = curNode->y + tilemap.tileSize * (i > 4) - tilemap.tileSize * (i == 1 || i == 2 || i == 3);

            if (minDistance == -1 || (nextX >= 0 && nextX < tilemap.width * tilemap.tileSize && nextY >= 0 && nextY < tilemap.height * tilemap.tileSize && searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance < minDistance && searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance > -1))
            {  //find the node next to our current node with the shortest path
                nextNode = &(searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize]);
                foundIVal = i;
                minDistance = searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance;
            }
        }

        curNode->lastNode = nextNode;
        curNode->distance = nextNode->distance + ((foundIVal % 2 == 0) ? 1 : sqrt(2)); //recalculate distance

        if (nextNode->lastNode == (void*) 1)
            quit = true;

        curNode = curNode->lastNode;
    }
    //*/

    quit = false;
    int pathCount = 1;
    path[0] = searchList[checkStartY][checkStartX];
    path[1] = path[0];
    while(!quit)
    {
        if (path[pathCount - 1].lastNode == (void*) 1)
            quit = true;
        else
        {
            //printf("%d = pathCount\n", pathCount);
            path[pathCount] = *((node*) (path[pathCount - 1].lastNode));
            //printf("%p\n", (void*) path[pathCount].lastNode);
            pathCount++;
        }
    }

    //backtrack through the path found, starting at the start node and following lastNode to the end
    *lengthOfPath += pathCount;
    //get pathSize
    return path;
}

void calculateStats(warperUnit* unit)
{
    //for shooters and no-classes
    int staminaBase = 13, energyBase = 5, hpBase = 40;  //base stat values at stat level 1
    double staminaAmplitude = 35, energyAmplitude = 38;  //how high the sigmoid functions get
    double staminaGrowth = 0.031, energyGrowth = 0.045, hpGrowth = 0.13; //how fast the stat functions grow
    double hpShiftPoint = 0.52;  //where the polynomial function for HP intersects with the base (y-intercept, or more specifically "stat level 1"-intercept)
    //stat lv 50: 24 stamina, 20 energy, 107 HP
    //max: 29 stamina, 24 energy, 257 HP
    if (unit->classType == classAttacker)
    {
        staminaBase = 22;
        energyBase = 3;
        hpBase = 30;
        staminaAmplitude = 64;
        energyAmplitude = 25;
        staminaGrowth = 0.027;
        energyGrowth = 0.038;
        hpGrowth = 0.078;
        hpShiftPoint = 0.62;
        //stat lv 50: 33 stamina, 13 energy, 75 HP
        //max: 50 stamina, 15 energy, 150 HP
    }
    if (unit->classType == classTechnomancer)
    {
        staminaBase = 8;
        energyBase = 12;
        hpBase = 20;
        staminaAmplitude = 25;
        energyAmplitude = 61;
        staminaGrowth = 0.04;
        energyGrowth = 0.034;
        hpGrowth = 0.078;
        hpShiftPoint = 0.62;
        //stat lv 50: 17 stamina, 33 energy, 65 HP
        //max: 20 stamina, 40 energy, 140 HP
    }

    unit->maxStamina = (int) round(staminaAmplitude / (1 + pow(M_E, -1 * staminaGrowth * (unit->stats.speed - 1))) + (staminaBase - staminaAmplitude / 2.0));  //sigmoid function
    unit->maxEnergy = (int) round(energyAmplitude / (1 + pow(M_E, -1 * energyGrowth * (unit->stats.tp - 1))) + (energyBase - energyAmplitude / 2.0)); //sigmoid function
    unit->maxHp = (int) round(pow(hpGrowth * (unit->stats.hp - 1), 2) + hpShiftPoint * (unit->stats.hp - 1) + hpBase);  //linear function
}

warperAttackResult doAttack(warperUnit* attackingUnit, warperUnit* defendingUnit, double distance)
{
    warperAttackResult result = {.damage = 0, .status = statusNone};
    //attack calculations
    int damage = 0;
    double hitChance = 0;
    double statusChance = 0;
    enum warperStatus inflictingStatus = statusNone;

    if (attackingUnit->classType == classNone)
    {
        //no-class calculations
        //pretty much just debug calculations, although maybe this is like the 1st act/tutorial case?
        //status: none ever
    }
    if (attackingUnit->classType == classAttacker)
    {
        //attacker calculations
        //hit chance: 0 tiles < x < 2 tiles: linearly decreasing from 100% to 85%
        //            2 < x < infinity: inverse square law
        //damage: calculated based on attack and speed maybe?
        //status: based on equipment
        //luck: only slightly affects the hit chance outcome
    }
    if (attackingUnit->classType == classShooter)
    {
        //shooter calculations
        //hit chance: bell curve centering around 3-ish tiles away
        //damage: calculated based on attack and range maybe?
        //status: based on equipment
        //luck: somewhat affects the hit chance outcome, slightly affects damage
    }
    if (attackingUnit->classType == classTechnomancer)
    {
        //technomancer calculations
        //hit chance: bell curve centering around X tiles away, lower plateau on the end of 0 < X, and a higher plateau on the end of X < infinity
        //damage: calculated based on attack and tech affinity
        //status: based on equipment
        //luck: affects status proc chance and damage?
    }
    //status chance: calculated based on opponent's status resist stat and your status chance
    //               as attacker's status chance stat increases, it overpowers the status resist ever so slightly more, but not to an uncontrollable level

    double randChance = rand() / (double) RAND_MAX;
    if (hitChance - randChance >= 0.001) //if hit chance + luck modifier is greater than or equal to randChance within a 0.1% margin of error (0.001 as a number)
    {
        defendingUnit->battleData.curHp -= damage;
        result.damage = damage;
    }

    if (statusChance - randChance >= 0.001)
    {
        defendingUnit->battleData.status = inflictingStatus;
        result.status = inflictingStatus;
    }
    return result;
}

/** \brief Add money, exp, and all other win-related stuff
 *
 * \param unit warperUnit* the units that finished the fight
 */
void finishBattle(warperTeam* team, warperBattle battle)
{
    for(int i = 0; i < team->unitsSize; i++)
    {
        //add exp, check level up
        addExp(team->units[i], 1);
    }
    team->money += 1;  //add money based on battle
    //drop items?
}

/** \brief Adds exp to a unit
 *
 * \param unit warperUnit*
 * \param exp int
 */
void addExp(warperUnit* unit, int exp)
{
   unit->exp += exp;
    if (unit->exp >= 100)  //if exp goes over a certain amount
    {
        //level up
        if (unit->level < WARPER_MAX_LEVEL)
        {
            unit->exp -= 100;
            unit->level++;
            if (unit->stats.attack < WARPER_MAX_STAT_LEVEL)
                unit->stats.attack++;
            if (unit->stats.hp < WARPER_MAX_STAT_LEVEL)
                unit->stats.hp++;
            if (unit->stats.luck < WARPER_MAX_STAT_LEVEL)
                unit->stats.luck++;
            if (unit->stats.speed < WARPER_MAX_STAT_LEVEL)
                unit->stats.speed++;
            if (unit->stats.techAffinity < WARPER_MAX_STAT_LEVEL)
                unit->stats.techAffinity++;
            if (unit->stats.tp < WARPER_MAX_STAT_LEVEL)
                unit->stats.tp++;
            calculateStats(unit);
        }
        else
        {
            unit->exp = 100;
            //reset exp
        }
        //add stat points (either for the player to allocate themselves or automatically)
    }
}
