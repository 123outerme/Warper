#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "warper.h"


//enum definitions
enum warperClass
{
    noClass,
    attacker,
    shooter,
    technomancer
};

enum warperObjective
{
    noObjective,
    defeat,
    retreat,
    rescue
};


//struct definitions
typedef struct _node {
    int x;
    int y;
    void* lastNode;
    bool visited;
    double distance;
} node;

typedef struct _warperStats
{
    int hp;
    int attack;
    int speed;
    int tp;
    int statusResistance;
    int techAffinity;
} warperStats;

typedef struct _warperBattleData
{
    int curHp;
    int remainingDistance;
    bool teleportedOrAttacked;
} warperBattleData;

typedef struct _warperUnit
{
    cSprite* sprite;
    int level;
    int exp;
    int maxHp;
    enum warperClass classType;
    warperStats stats;
    warperBattleData battleData;
} warperUnit;

typedef struct _warperItem
{
    int id;
    int number;
} warperItem;

typedef struct _warperTeam
{
    warperUnit** units;
    int unitsSize;
    warperItem* inventory;
    int inventorySize;
    int money;
} warperTeam;

typedef struct _warperBattle
{
    enum warperObjective objective;
    bool isPlayerTurn;
} warperBattle;

//function prototypes
void initWarperTeam(warperTeam* team, warperUnit** units, int unitsSize, warperItem* inventory, int inventorySize, int money);
void destroyWarperTeam(warperTeam* team);
void initNode(node* nodePtr, int x, int y, node* lastNode, bool visited, double distance);
node* BreadthFirst(warperTilemap tilemap, const int startX, const int startY, const int endX, const int endY, int* lengthOfPath, const bool drawDebug, cCamera* camera);
void finishBattle(warperTeam* team, warperBattle battle);
void addExp(warperUnit* unit, int exp);

#endif // PLAYER_H_INCLUDED
