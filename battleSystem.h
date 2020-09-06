#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


//#defines
#ifndef bool
    #define bool char
    #define false 0
    #define true 1
    #define boolToString(bool) (bool ? "true" : "false")
#endif // bool
#ifndef NULL
    #define NULL ((void*) 0)
#endif //NULL


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
    warperUnit* units;
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
void finishBattle(warperTeam* team, warperBattle battle);
void addExp(warperUnit* unit, int exp);

#endif // PLAYER_H_INCLUDED
