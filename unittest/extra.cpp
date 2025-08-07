#include "game.h"
#include "gamedata.h"

/// Run the initial setup for a faction
/// For unit tests, factions get 1 leader with combat 3.
int Game::SetupFaction( Faction *pFac ) {
    pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 50;

    if (pFac->noStartLeader)
        return 1;

    //
    // Set up first unit.
    //
    Unit *temp2 = GetNewUnit( pFac );
    temp2->SetMen(I_LEADERS, 1);
    pFac->DiscoverItem(I_LEADERS, 0, 1);
    temp2->reveal = REVEAL_FACTION;
    // combat skill 3
    temp2->Study(S_COMBAT, 180);
    // Set up flags
    temp2->SetFlag(FLAG_BEHIND, 1);

    // Put the unit in the first region (which will be the one city for the test game)
    ARegion *reg = regions.front();
    temp2->MoveUnit(reg->GetDummy());

    return 1;
}

/// Check to see whether a player has won the game.
/// This is NULL in the unittests.
Faction *Game::CheckVictory() { return NULL; }

/// Modify certain starting statistics of the world's data structures.
void Game::ModifyTablesPerRuleset(void) {
    if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
        EnableSkill(S_QUARTERMASTER);
        EnableObject(O_CARAVANSERAI);
    }
}

const std::optional<std::string> ARegion::movement_forbidden_by_ruleset(Unit *, ARegion *, ARegionList&) {
    return std::nullopt;
}
