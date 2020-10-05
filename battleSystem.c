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

void initFlowNode(flowNode* nodePtr, int x, int y, bool visited, double distance, double fieldLineDegrees)
{
    nodePtr->x = x;
    nodePtr->y = y;
    nodePtr->visited = visited;
    nodePtr->distance = distance;
    nodePtr->fieldLineDegrees = fieldLineDegrees;
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


node* offsetBreadthFirst(warperTilemap tilemap, int startX, int startY, int endX, int endY, int* lengthOfPath, const bool drawDebug, cCamera* camera)
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
    if (!queue || (startX / tilemap.tileSize == endX / tilemap.tileSize && startY / tilemap.tileSize == endY / tilemap.tileSize))
    {
        *lengthOfPath = -1;
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
            initNode(&(searchList[y][x]), x * tilemap.tileSize + deltaEndX, y * tilemap.tileSize + deltaEndY, NULL, false, 0);
    }
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
            for(int y = nextY / tilemap.tileSize; y <= nextY / tilemap.tileSize + 1 * (deltaEndY != 0); y++)
            {
                for(int x = nextX / tilemap.tileSize; x <= nextX / tilemap.tileSize + 1 * (deltaEndX != 0); x++)
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
                if (!searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].visited)
                {
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].visited = true;
                    searchList[nextY / tilemap.tileSize][nextX / tilemap.tileSize].distance = curNode->distance + ((i % 2 == 0) ? 1 : 2 * sqrt(2));
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
                //adjust all surrounding tiles to align to the tilemap.tileSize grid
                bool adjusted[2][2] = {{false, false}, {false, false}};
                int xPos = nextX / tilemap.tileSize;
                int yPos = nextY / tilemap.tileSize;
                int deltaX = nextX - xPos * tilemap.tileSize;
                int deltaY = nextY - yPos * tilemap.tileSize;

                for(int y = 0; y <= 1 * (deltaY != 0); y++)
                {
                    int x = 0;

                    if (tilemap.collisionmap[x + xPos][y + yPos] == 1)
                    {
                        adjusted[x][y] = true;  //flag to say "don't put this in the queue"
                        /*if (deltaY < deltaX && !(adjusted[0][y] || adjusted[1][y]))
                        {
                            searchList[y + yPos][xPos].y = ((int) searchList[y + yPos][x + xPos].y / tilemap.tileSize) * tilemap.tileSize;
                            searchList[y + yPos][xPos + 1].y = ((int) searchList[y + yPos][x + xPos].y / tilemap.tileSize) * tilemap.tileSize;

                            adjusted[0][y] = true;
                            adjusted[1][y] = true;
                        }
                        else
                        {
                            if (!(adjusted[0][y] || adjusted[0][y + 1]))
                            {
                                searchList[yPos][xPos].x = ((int) searchList[y + yPos][x + xPos].x / tilemap.tileSize) * tilemap.tileSize;
                                searchList[yPos + 1][xPos].x = ((int) searchList[y + yPos][x + xPos].x / tilemap.tileSize) * tilemap.tileSize;

                                adjusted[0][y] = true;
                                adjusted[0][y + 1] = true;
                            }
                        }*/
                    }
                    else
                    {
                        x++;

                        adjusted[x][y] = true;  //flag to say "don't put this in the queue"

                        /*if (tilemap.collisionmap[x + xPos][y + yPos] == 1)
                        {
                            if (deltaY < deltaX && (!adjusted[0][y] || !adjusted[1][y]))
                            {
                                searchList[y + yPos][xPos].y = ((int) searchList[y + yPos][x + xPos].y / tilemap.tileSize) * tilemap.tileSize;
                                searchList[y + yPos][xPos + 1].y = ((int) searchList[y + yPos][x + xPos].y / tilemap.tileSize) * tilemap.tileSize;

                                adjusted[0][y] = true;
                                adjusted[1][y] = true;
                            }
                            else
                            {
                                if (!(adjusted[0][y] || adjusted[0][y + 1]))
                                {
                                    searchList[yPos][xPos].x = ((int) searchList[y + yPos][x + xPos].x / tilemap.tileSize) * tilemap.tileSize;
                                    searchList[yPos + 1][xPos].x = ((int) searchList[y + yPos][x + xPos].x / tilemap.tileSize) * tilemap.tileSize;

                                    adjusted[1][y] = true;
                                    adjusted[1][y + 1] = true;
                                }
                            }
                        }*/
                    }
                    for(int enqueueX = 0; enqueueX < 1 * (deltaX != 0); enqueueX++)
                    {
                        if (!searchList[y + yPos][enqueueX + xPos].visited && !adjusted[enqueueX][y])
                        {
                            searchList[y + yPos][enqueueX + xPos].distance = curNode->distance + ((i % 2 == 0) ? 1 : 2 * sqrt(2));
                            searchList[y + yPos][enqueueX + xPos].lastNode = curNode;
                            queue[queueCount] = malloc(sizeof(node));
                            initNode(queue[queueCount], searchList[y + yPos][enqueueX + xPos].x, searchList[y + yPos][enqueueX + xPos].y,
                                     searchList[y + yPos][enqueueX + xPos].lastNode, false, searchList[y + yPos][enqueueX + xPos].distance);
                            queueCount++;
                        }
                    }
                }

                /*
                if (closestX != -1 && closestY != -1 && !searchList[closestY][closestX].visited)
                {
                    searchList[closestY][closestX].visited = true;
                    searchList[closestY][closestX].distance = curNode->distance + ((i % 2 == 0) ? 1 : 2 * sqrt(2));
                    searchList[closestY][closestX].lastNode = curNode;
                    searchList[closestY][closestX].x = closestX * tilemap.tileSize;
                    searchList[closestY][closestX].y = closestY * tilemap.tileSize;
                    queue[queueCount] = malloc(sizeof(node));
                    initNode(queue[queueCount], searchList[closestY][closestX].x, searchList[closestY][closestX].y,
                             searchList[closestY][closestX].lastNode, true, searchList[closestY][closestX].distance);
                    queueCount++;

                    if (drawDebug)
                    {
                        SDL_SetRenderDrawColor(global.mainRenderer, 0x00, 0xFF, 0x00, 0xC0);
                        SDL_RenderFillRect(global.mainRenderer, &((SDL_Rect) {.x = nextX - camera->rect.x, .y = nextY - camera->rect.y, .w = tilemap.tileSize, .h = tilemap.tileSize}));
                        SDL_RenderPresent(global.mainRenderer);
                        //printf("%p\n", searchList[y][x].lastNode);
                        //waitForKey(true);
                    }
                }
                //*/

                /* TODO: I think this is broken
                //pathfind to the closest X and Y node
                int closestTilePathSize = 0;
                node* pathToClosestTile = BreadthFirst(tilemap, curNode->x, curNode->y, endX, endY, &closestTilePathSize, drawDebug, camera);
                if (pathToClosestTile)
                {
                    pathToClosestTile[closestTilePathSize - 1].lastNode = curNode;
                    for(int j = 0; j < closestTilePathSize; j++)
                    {
                        queue[queueCount] = malloc(sizeof(node));
                        initNode(queue[queueCount], pathToClosestTile[j].x, pathToClosestTile[j].y, pathToClosestTile[j].lastNode, true, pathToClosestTile[j].distance);
                        queueCount++;
                    }
                    *lengthOfPath += closestTilePathSize;

                    free(pathToClosestTile);
                }
                //*/
            }

            if (abs(nextX - startX) <= tilemap.tileSize / 2 && abs(nextY - startY) <= tilemap.tileSize / 2)
            {  //check if node is at startX, startY. Stop if is, continue if not
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

flowNode** PseudoFlowField(warperTilemap tilemap, const int startX, const int startY, const int endX, const int endY, int* lengthOfPath, const bool drawDebug, cCamera* camera)
{
    int queueSize = tilemap.width * tilemap.height + 1;

    flowNode** openList = calloc(queueSize, sizeof(flowNode*));

    if (!openList || (startX / tilemap.tileSize == endX / tilemap.tileSize && startY / tilemap.tileSize == endY / tilemap.tileSize))
    {
        *lengthOfPath = -1;
        if (openList)
            free(openList);
        return NULL;
    }
    flowNode* curNode;

    int queueCount = 0;

    flowNode** searchList = calloc(tilemap.height, sizeof(flowNode*));
    for(int y = 0; y < tilemap.height; y++)
    {
        searchList[y] = calloc(tilemap.width, sizeof(flowNode));
        for(int x = 0; x < tilemap.width; x++)
            initFlowNode(&(searchList[y][x]), x * tilemap.tileSize, y * tilemap.tileSize, false, 0, 0);
    }
    curNode = &(searchList[(int) endY / tilemap.tileSize][(int) endX / tilemap.tileSize]);
    curNode->distance = 0;  //distance is obviously zero
    curNode->visited = true;

    const int angleLookup[8] = {0, 180, 270, 90, 315, 225, 45, 135};  //angles for each direction given by the loop below

    curNode->fieldLineDegrees = radToDeg(atan2(endY - ((int) endY / tilemap.tileSize) * tilemap.tileSize, endX - ((int) endX / tilemap.tileSize) * tilemap.tileSize)); //get the angle to the end point

    bool quit = false;
    while(!quit)
    {
        for(int i = 0; i < 8; i++)  //8 directions: 0 = right, 1 = left, 2 = down, 3 = up, 4 = down-right, 5 = down-left, 6 = up-right, 7 = up-left
        {
            int x = (curNode->x / tilemap.tileSize) + (i == 0 || i == 4 || i == 6) - (i == 1 || i == 5 || i == 7);
            int y = (curNode->y / tilemap.tileSize) + (i == 2 || i == 4 || i == 5) - (i == 3 || i == 6 || i == 7);
            if (!(searchList[y][x].visited) && (x >= 0 && y >= 0 && x < tilemap.width && y < tilemap.height) && tilemap.collisionmap[x][y] == 0)
            {
                if (i < 4 || (i >= 4 && tilemap.collisionmap[x - (i == 4 || i == 6) + (i == 5 || i == 7)][y] == 0 && tilemap.collisionmap[x][y - (i == 4 || i == 5) + (i == 6 || i == 7)] == 0))
                {
                    //if we aren't in a diagonal, or if we are, and the two adjacent non-diagonal tiles are free of collision
                    openList[queueCount++] = &(searchList[y][x]);
                    searchList[y][x].visited = true;
                    searchList[y][x].distance = curNode->distance + ((i < 4) ? 1 : 2 * sqrt(2));  //0-3 are cardinal directions, 4-7 are diagonal movements
                    searchList[y][x].fieldLineDegrees = angleLookup[i];
                    if (((int) x == (int) startX / tilemap.tileSize) && ((int) y == (int) startY / tilemap.tileSize))
                    {  //check if flowNode is at startX, startY. Stop if is, continue if not
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

        //enqueue valid adjacent flowNodes to selected flowNode
        if (queueCount == 0)
        {
            *lengthOfPath = 0;
            free(openList);
            for(int y = 0; y < tilemap.height; y++)
                free(searchList[y]);
            free(searchList);
            return NULL;
        }
        //check if no items are enqueued i.e. no path, quits with a NULL path if this is true

        curNode = openList[0];

        for(int i = 0; i < queueCount - 1; i++)
            openList[i] = openList[i + 1];

        queueCount--;
        //select a new adjacent flowNode and delete the last enqueued item
    }
    free(openList);

    return searchList;
}

void doAttack(warperUnit* attackingUnit, warperUnit* defendingUnit, double distance)
{
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
    }
    if (attackingUnit->classType == classShooter)
    {
        //shooter calculations
        //hit chance: bell curve centering around 3-ish tiles away
        //damage: calculated based on attack and status chance maybe?
        //status: based on equipment
    }
    if (attackingUnit->classType == classTechnomancer)
    {
        //technomancer calculations
        //hit chance: bell curve centering around X tiles away, higher plateau on the end of 0 < X, and a lower plateau on the end of X < infinity
        //damage: calculated based on attack and tech affinity
        //status: based on equipment
    }
    //status chance: calculated based on opponent's status resist stat and your status chance
    //               as attacker's status chance stat increases, it overpowers the status resist ever so slightly more, but not to an uncontrollable level

    double randChance = rand() / (double) RAND_MAX;
    if (hitChance - randChance >= 0.001) //if hit chance is greater than or equal to randChance within a 0.1% margin of error (0.001 as a number)
        defendingUnit->battleData.curHp -= damage;

    if (statusChance - randChance >= 0.001)
        defendingUnit->battleData.status = inflictingStatus;
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
        unit->exp -= 100;
        unit->level++;
        //add stat points (either for the player to allocate themselves or automatically)
    }
}
