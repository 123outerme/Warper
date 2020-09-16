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
void initNode(node* nodePtr, int x, int y, node* lastNode, bool visited, double distanceToNext)
{
    nodePtr->x = x;
    nodePtr->y = y;
    nodePtr->lastNode = lastNode;
    nodePtr->visited = visited;
    nodePtr->distanceToNext = distanceToNext;
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
    node* path = calloc((int) (2 * sqrt(pow((double) tilemap.width, 2.0) + pow((double) tilemap.height, 2.0))) + 1, sizeof(node));  //path can be at most a diagonal/zig-zag one
    if (!queue || (startX / tilemap.tileSize == endX / tilemap.tileSize && startY / tilemap.tileSize == endY / tilemap.tileSize))
    {
        *lengthOfPath = -1;
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
