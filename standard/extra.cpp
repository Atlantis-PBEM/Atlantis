#include "game.h"
#include "gamedata.h"

int Game::SetupFaction( Faction *pFac )
{
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

    temp2->type = U_MAGE;
    temp2->Study(S_PATTERN, 30);
    temp2->Study(S_SPIRIT, 30);
    temp2->Study(S_GATE_LORE, 30);

    if (TurnNumber() >= 25) {
        temp2->Study(S_PATTERN, 60);
        temp2->Study(S_SPIRIT, 60);
        temp2->Study(S_FORCE, 90);
        temp2->Study(S_COMBAT, 30);
    }

    if (Globals->UPKEEP_MINIMUM_FOOD > 0)
    {
        if (!(ItemDefs[I_FOOD].flags & ItemType::DISABLED)) {
            temp2->items.SetNum(I_FOOD, 6);
            pFac->DiscoverItem(I_FOOD, 0, 1);
        } else if (!(ItemDefs[I_FISH].flags & ItemType::DISABLED)) {
            temp2->items.SetNum(I_FISH, 6);
            pFac->DiscoverItem(I_FISH, 0, 1);
        } else if (!(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED)) {
            temp2->items.SetNum(I_LIVESTOCK, 6);
            pFac->DiscoverItem(I_LIVESTOCK, 0, 1);
        } else if (!(ItemDefs[I_GRAIN].flags & ItemType::DISABLED)) {
            temp2->items.SetNum(I_GRAIN, 2);
            pFac->DiscoverItem(I_GRAIN, 0, 1);
        }
        temp2->items.SetNum(I_SILVER, 10);
    }

    ARegion *reg = NULL;
    if (pFac->pStartLoc) {
        reg = pFac->pStartLoc;
    } else if (!Globals->MULTI_HEX_NEXUS) {
        reg = regions.front();
    } else {
        ARegionArray *pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
        while(!reg) {
            reg = pArr->GetRegion(rng::get_random(pArr->x), rng::get_random(pArr->y));
        }
    }
    temp2->MoveUnit( reg->GetDummy() );

    if (Globals->LAIR_MONSTERS_EXIST || Globals->WANDERING_MONSTERS_EXIST) {
        // Try to auto-declare all player factions unfriendly
        // to Creatures, since all they do is attack you.
        pFac->set_attitude(monfaction, AttitudeType::UNFRIENDLY);
    }

    return( 1 );
}

Faction *Game::CheckVictory()
{
    for(const auto region : regions) {
        for(const auto obj : region->objects) {
            if (obj->type != O_BKEEP) continue;
            if (obj->units.size()) {
                return nullptr;
            }

            // Now see find the first faction guarding the region
            Object *o = region->GetDummy();
            for(const auto u : o->units) {
                if (u->guard == GUARD_GUARD) return u->faction;
            }
            break;
        }
    }
    return nullptr;
}

void Game::ModifyTablesPerRuleset(void)
{
    if (Globals->APPRENTICES_EXIST)
        EnableSkill(S_MANIPULATE);

    if (!Globals->GATES_EXIST)
        DisableSkill(S_GATE_LORE);

    if (Globals->FULL_TRUESEEING_BONUS) {
        ModifyAttribMod("observation", 1, AttribModItem::SKILL,
                "TRUE", AttribModItem::UNIT_LEVEL, 1);
    }
    if (Globals->IMPROVED_AMTS) {
        ModifyAttribMod("observation", 2, AttribModItem::ITEM,
                "AMTS", AttribModItem::CONSTANT, 3);
    }
    if (Globals->FULL_INVIS_ON_SELF) {
        ModifyAttribMod("stealth", 3, AttribModItem::SKILL,
                "INVI", AttribModItem::UNIT_LEVEL, 1);
    }

    if (Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
        ClearTerrainRaces(R_NEXUS);
        ModifyTerrainRace(R_NEXUS, 0, I_HIGHELF);
        ModifyTerrainRace(R_NEXUS, 1, I_VIKING);
        ModifyTerrainRace(R_NEXUS, 2, I_PLAINSMAN);
        ClearTerrainItems(R_NEXUS);
        ModifyTerrainItems(R_NEXUS, 0, I_IRON, 100, 10);
        ModifyTerrainItems(R_NEXUS, 1, I_WOOD, 100, 10);
        ModifyTerrainItems(R_NEXUS, 2, I_STONE, 100, 10);
        ModifyTerrainEconomy(R_NEXUS, 1000, 15, 50, 2);
    }

    EnableItem(I_PICK);
    EnableItem(I_SPEAR);
    EnableItem(I_AXE);
    EnableItem(I_HAMMER);
    EnableItem(I_MCROSSBOW);
    EnableItem(I_MWAGON);
    EnableItem(I_GLIDER);
    EnableItem(I_NET);
    EnableItem(I_LASSO);
    EnableItem(I_BAG);
    EnableItem(I_SPINNING);
    EnableItem(I_LEATHERARMOR);
    EnableItem(I_CLOTHARMOR);
    EnableItem(I_BOOTS);
    EnableItem(I_BAXE);
    EnableItem(I_MBAXE);
    EnableItem(I_IMARM);
    EnableItem(I_SUPERBOW);
    EnableItem(I_LANCE);
    EnableItem(I_JAVELIN);
    EnableItem(I_PIKE);

    EnableSkill(S_ARMORCRAFT);
    EnableSkill(S_WEAPONCRAFT);

    EnableObject(O_ROADN);
    EnableObject(O_ROADNE);
    EnableObject(O_ROADNW);
    EnableObject(O_ROADS);
    EnableObject(O_ROADSE);
    EnableObject(O_ROADSW);
    EnableObject(O_TEMPLE);
    EnableObject(O_MQUARRY);
    EnableObject(O_AMINE);
    EnableObject(O_PRESERVE);
    EnableObject(O_SACGROVE);

    EnableObject(O_ISLE);
    EnableObject(O_DERELICT);
    EnableObject(O_OCAVE);
    EnableObject(O_WHIRL);
    EnableItem(I_PIRATES);
    EnableItem(I_KRAKEN);
    EnableItem(I_MERFOLK);
    EnableItem(I_ELEMENTAL);

    if ((Globals->UNDERDEEP_LEVELS > 0) || (Globals->UNDERWORLD_LEVELS > 1)) {
        EnableItem(I_MUSHROOM);
        EnableItem(I_HEALPOTION);
        EnableItem(I_ROUGHGEM);
        EnableItem(I_GEMS);
        EnableSkill(S_GEMCUTTING);
    }

    // Modify the various spells which are allowed to cross levels
    if (Globals->EASIER_UNDERWORLD) {
        modify_range_flags("rng_teleport", RangeType::RNG_CROSS_LEVELS);
        modify_range_flags("rng_portal", RangeType::RNG_CROSS_LEVELS);
        modify_range_flags("rng_farsight", RangeType::RNG_CROSS_LEVELS);
        modify_range_flags("rng_clearsky", RangeType::RNG_CROSS_LEVELS);
        modify_range_flags("rng_weather", RangeType::RNG_CROSS_LEVELS);
    }

    if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
        EnableSkill(S_QUARTERMASTER);
        EnableObject(O_CARAVANSERAI);
    }
    // XXX -- This is just here to preserve existing behavior
    ModifyItemProductionBooster(I_AXE, I_HAMMER, 1);
    return;
}

const std::optional<std::string> ARegion::movement_forbidden_by_ruleset(Unit *u, ARegion *origin, ARegionList& regs) {
    return std::nullopt;
}
