#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "warper.h"

//#defines
#define WARPER_MAX_LEVEL 50
#define WARPER_MAX_STAT_LEVEL 100

//enum definitions
enum warperClass
{
    classNone,
    classAttacker,
    classShooter,  //maybe rename this one
    classTechnomancer
};

enum warperStatus
{
    statusNone,
    statusHacked,  /**< unable to use attack with anything other than basic attacks, and movement is restricted to walking */
    statusBleed,  /**< damage over time (damage at the start of each turn) */
    statusSlow,  /**< reduces stamina by 25-ish % */
    statusExposed  /**< crits are easier (maybe use this, maybe not) */
};

enum warperObjective
{
    objectiveNone,
    objectiveDefeat,
    objectiveRetreat,
    objectiveRescue,
    objectiveCapture
};

enum warperItemType
{
    itemMisc,
    itemMelee,
    itemRanged,
    itemMagic,
    itemConsumable,
    itemStory
};

enum warperWeaponAttribute
{
    attrNone,
    attrTough,
    attrOptimized,
    attrLucky
};

//struct definitions
typedef struct _node
{
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
    int techAffinity;
    int luck;
    int statPts;
} warperStats;

typedef struct _warperBattleData
{
    int curHp;
    enum warperStatus status;
    int statusStrength;
    int staminaLeft;
    int energyLeft;
    bool teleportedOrAttacked;
} warperBattleData;

typedef struct _warperWeaponStats
{
    int power;
    int weight;
    int reach;
    enum warperWeaponAttribute attribute;
} warperWeaponStats;

typedef struct _warperItem
{
    enum warperItemType itemType;
    int id;
    char* name;
    int count;
    warperWeaponStats weaponStats;
} warperItem;

typedef struct _warperUnit
{
    cSprite* sprite;
    char* name;
    int level;
    int exp;
    int maxHp;
    int maxStamina;
    int maxEnergy;
    enum warperClass classType;
    warperItem* weapon;
    warperStats stats;
    warperBattleData battleData;
} warperUnit;

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

typedef struct _warperAttackCheck
{
    int damage;
    enum warperStatus status;
    double hitChance;
    double critChance;
    double statusChance;
} warperAttackCheck;

typedef struct _warperAttackResult
{
    int damage;
    enum warperStatus status;
    bool crit;
    bool miss;
} warperAttackResult;

//function prototypes
void initWarperTeam(warperTeam* team, warperUnit** units, int unitsSize, warperItem* inventory, int inventorySize, int money);
void destroyWarperTeam(warperTeam* team, bool freeUnits);
void initNode(node* nodePtr, int x, int y, node* lastNode, bool visited, double distance);
node* BreadthFirst(warperTilemap tilemap, const int startX, const int startY, const int endX, const int endY, int* lengthOfPath, const bool drawDebug, cCamera* camera);
node* offsetBreadthFirst(warperTilemap tilemap, int startX, int startY, int endX, int endY, int finderWidth, int finderHeight, cDoubleRect* customCollisions, int customCollisionLength, int* lengthOfPath, const bool drawDebug, cCamera* camera);
void calculateStats(warperUnit* unit, bool setBattleStats);
warperAttackCheck checkAttack(warperUnit* attackingUnit, warperUnit* defendingUnit, double distance);
warperAttackResult doAttack(warperUnit* attackingUnit, warperUnit* defendingUnit, warperAttackCheck checkResult);
void finishBattle(warperTeam* team, warperTeam* enemyTeam, warperBattle battle);
void addExp(warperUnit* unit, int exp);

void initTestWarperTeams(warperTilemap tilemap, warperTeam* playerTeam, warperTeam* enemyTeam);

#define STATUS_NAME_ARR {"None", "Hacked", "Wounded", "Slowed", "Exposed"}

#define MISC_ITEM_NAME_ARR {"Misc Item 1"}
#define MELEE_ITEM_NAME_ARR {"Training Sword"}
#define RANGED_ITEM_NAME_ARR {"Training Blaster"}
#define MAGIC_ITEM_NAME_ARR {"Training Crypto- + Grimoire/Tome = Cryptoire/Cryptome"}
#define CONSUMABLE_ITEM_NAME_ARR {"Consumable 1"}
#define STORY_ITEM_NAME_ARR {"Story Item 1"}

#define MELEE_WEAPON_STATS_ARR {}
#define RANGED_WEAPON_STATS_ARR {}
#define MAGIC_WEAPON_STATS_ARR {}

#endif // PLAYER_H_INCLUDED
