#include "battleSystem.h"

/** \brief Add money,
 *
 * \param unit warperUnit* the units that finished the fight
 */
void finishBattle(warperTeam* team, warperBattle battle)
{

    for(int i = 0; i < team->unitsSize; i++)
    {
        //add exp, check level up
        addExp(&(team->units[i]), 1);
    }
    team->money += 1;  //add money based on battle
    //drop items?
}

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
