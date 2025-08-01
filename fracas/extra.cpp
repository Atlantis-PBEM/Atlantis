#include "game.h"
#include "gamedata.h"

int Game::SetupFaction( Faction *pFac )
{
    pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 500;

    if (pFac->noStartLeader)
        return 1;

    //
    // Set up first unit.
    //
    Unit *leader = GetNewUnit(pFac);
    leader->SetMen(I_LEADERS, 1);
    pFac->DiscoverItem(I_LEADERS, 0, 1);
    leader->reveal = REVEAL_FACTION;
    leader->SetFlag(FLAG_BEHIND, 1);
    leader->set_name("House Leader");
    leader->items.SetNum(I_HORSE, 1);
    pFac->DiscoverItem(I_HORSE, 0, 1);

    leader->type = U_MAGE;
    leader->Study(S_TACTICS, 180);
    leader->Study(S_PATTERN, 30);
    leader->Study(S_SPIRIT, 30);
    leader->Study(S_GATE_LORE, 30);
    leader->Study(S_FORCE, 90);
    leader->Study(S_FIRE, 90);
    leader->combat = S_FIRE;

    if (TurnNumber() >= 25) {
        leader->Study(S_PATTERN, 60);
        leader->Study(S_SPIRIT, 60);
        leader->Study(S_FORCE, 90);
        leader->Study(S_COMBAT, 30);
    }

    //
    // Set up other supporting units
    //
    Unit *army = GetNewUnit(pFac);
    army->SetMen(I_LEADERS, 10);
    army->reveal = REVEAL_FACTION;
    army->set_name("House Guard");
    army->items.SetNum(I_SWORD, 10);
    pFac->DiscoverItem(I_SWORD, 0, 1);
    army->items.SetNum(I_HORSE, 10);
    pFac->DiscoverItem(I_HORSE, 0, 1);
    army->items.SetNum(I_CHAINARMOR, 10);
    pFac->DiscoverItem(I_CHAINARMOR, 0, 1);
    army->Study(S_COMBAT, 1800);
    army->Study(S_RIDING, 1800);


    Unit *archers = GetNewUnit(pFac);
    archers->SetMen(I_LEADERS, 10);
    archers->reveal = REVEAL_FACTION;
    archers->SetFlag(FLAG_BEHIND, 1);
    archers->set_name("House Guard");
    archers->items.SetNum(I_CROSSBOW, 10);
    pFac->DiscoverItem(I_CROSSBOW, 0, 1);
    archers->items.SetNum(I_HORSE, 10);
    pFac->DiscoverItem(I_HORSE, 0, 1);
    archers->items.SetNum(I_CHAINARMOR, 10);
    pFac->DiscoverItem(I_CHAINARMOR, 0, 1);
    archers->Study(S_CROSSBOW, 1800);
    archers->Study(S_RIDING, 1800);


    Unit *sneak = GetNewUnit(pFac);
    sneak->SetMen(I_LEADERS, 1);
    sneak->reveal = REVEAL_NONE;
    sneak->SetFlag(FLAG_BEHIND, 1);
    sneak->guard = GUARD_AVOID;
    sneak->set_name("Captain of the Hunt");
    sneak->items.SetNum(I_HORSE, 1);
    pFac->DiscoverItem(I_HORSE, 0, 1);
    sneak->items.SetNum(I_CLOTHARMOR, 1);
    pFac->DiscoverItem(I_CLOTHARMOR, 0, 1);
    sneak->Study(S_STEALTH, 180);
    sneak->Study(S_OBSERVATION, 180);
    sneak->Study(S_HUNTING, 180);


    Unit *quartermaster = GetNewUnit(pFac);
    quartermaster->SetMen(I_LEADERS, 1);
    quartermaster->reveal = REVEAL_FACTION;
    quartermaster->SetFlag(FLAG_BEHIND, 1);
    quartermaster->guard = GUARD_AVOID;
    quartermaster->set_name("Store Master");
    quartermaster->items.SetNum(I_WOOD, 10);
    pFac->DiscoverItem(I_WOOD, 0, 1);
    quartermaster->items.SetNum(I_STONE, 10);
    pFac->DiscoverItem(I_STONE, 0, 1);
    quartermaster->Study(S_QUARTERMASTER, 180);
    quartermaster->Study(S_OBSERVATION, 180);


    Unit *retainer = GetNewUnit(pFac);
    retainer->SetMen(I_LEADERS, 1);
    retainer->reveal = REVEAL_FACTION;
    retainer->SetFlag(FLAG_BEHIND, 1);
    retainer->guard = GUARD_AVOID;
    retainer->set_name("Old Family Retainer");
    retainer->Study(S_FARMING, 180);
    retainer->Study(S_RANCHING, 180);
    retainer->Study(S_FISHING, 180);


    Unit *merchant = GetNewUnit(pFac);
    merchant->SetMen(I_LEADERS, 1);
    merchant->reveal = REVEAL_FACTION;
    merchant->SetFlag(FLAG_BEHIND, 1);
    merchant->guard = GUARD_AVOID;
    merchant->set_name("Trusted City Merchant");
    merchant->items.SetNum(I_HORSE, 1);
    pFac->DiscoverItem(I_HORSE, 0, 1);
    merchant->Study(S_HORSETRAINING, 180);
    merchant->Study(S_LUMBERJACK, 180);
    merchant->Study(S_BUILDING, 180);
    merchant->Study(S_SHIPBUILDING, 180);


    Unit *smith = GetNewUnit(pFac);
    smith->SetMen(I_LEADERS, 1);
    smith->reveal = REVEAL_FACTION;
    smith->SetFlag(FLAG_BEHIND, 1);
    smith->guard = GUARD_AVOID;
    smith->set_name("Blacksmith");
    smith->items.SetNum(I_IRON, 10);
    pFac->DiscoverItem(I_IRON, 0, 1);
    smith->Study(S_MINING, 180);
    smith->Study(S_WEAPONSMITH, 180);
    smith->Study(S_ARMORER, 180);
    smith->Study(S_QUARRYING, 180);


    /* Food is stoopid! (IMHO, of course)
    if (Globals->UPKEEP_MINIMUM_FOOD > 0)
    {
        if (!(ItemDefs[I_FOOD].flags & ItemType::DISABLED))
            leader->items.SetNum(I_FOOD, 6);
        else if (!(ItemDefs[I_FISH].flags & ItemType::DISABLED))
            leader->items.SetNum(I_FISH, 6);
        else if (!(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED))
            leader->items.SetNum(I_LIVESTOCK, 6);
        else if (!(ItemDefs[I_GRAIN].flags & ItemType::DISABLED))
            leader->items.SetNum(I_GRAIN, 2);
        leader->items.SetNum(I_SILVER, 10);
    }
    */

    ARegion *reg = NULL;
    if (pFac->pStartLoc) {
        reg = pFac->pStartLoc;
    } else if (!Globals->MULTI_HEX_NEXUS) {
        if (Globals->NEXUS_EXISTS) {
            reg = regions.front();
        } else {
            // Get the surface
            ARegionArray *pArr = regions.GetRegionArray(1);
            // Let's look for a random city hex
            while (!reg) {
                reg = pArr->GetRegion(rng::get_random(pArr->x), rng::get_random(pArr->y));
                // reg->town will be null if there is no town.
                // reg->town->TownType() returns town size.
                if (reg == NULL || (Globals->TOWNS_EXIST && reg->town == NULL) )
                    reg = NULL;
                //else if (reg->town->TownType() != TOWN_CITY)
                //  reg = NULL;
                // If you just wanted non-ocean, try
                // if (reg->similar_type != R_OCEAN) reg = NULL;
            }
        }
    } else {
        ARegionArray *pArr = regions.GetRegionArray(0);
        while(!reg) {
            reg = pArr->GetRegion(rng::get_random(pArr->x), rng::get_random(pArr->y));
        }
    }

    leader->MoveUnit( reg->GetDummy() );
    army->MoveUnit( reg->GetDummy() );
    archers->MoveUnit( reg->GetDummy() );
    sneak->MoveUnit( reg->GetDummy() );
    quartermaster->MoveUnit( reg->GetDummy() );
    retainer->MoveUnit( reg->GetDummy() );
    merchant->MoveUnit( reg->GetDummy() );
    smith->MoveUnit( reg->GetDummy() );

    return( 1 );
}

Faction *Game::CheckVictory()
{
    return NULL;
}

void Game::ModifyTablesPerRuleset(void)
{
    if (Globals->APPRENTICES_EXIST)
        EnableSkill(S_MANIPULATE);

    if (!Globals->GATES_EXIST)
        DisableSkill(S_GATE_LORE);

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

    //EnableItem(I_PICK);
    //EnableItem(I_SPEAR);
    //EnableItem(I_AXE);
    //EnableItem(I_HAMMER);
    EnableItem(I_MCROSSBOW);
    EnableItem(I_MWAGON);
    //EnableItem(I_GLIDER);
    //EnableItem(I_NET);
    //EnableItem(I_LASSO);
    //EnableItem(I_BAG);
    //EnableItem(I_SPINNING);
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
    // Hmm, shouldn't LBOW be armour piercing?
    modify_weapon_attack("LBOW", ARMORPIERCING, ATTACK_RANGED, 1, 1);

    //EnableItem(I_GREYELF);
    //EnableItem(I_MINOTAUR);
    //EnableItem(I_DROWMAN);

    // Racial skills are modified too - but that
    // bit's at the bottom
    DisableItem(I_GNOME);
    DisableItem(I_ESKIMO);
    DisableItem(I_TRIBESMAN);
    DisableItem(I_NOMAD);
    DisableItem(I_WOODELF);
    DisableItem(I_TRIBALELF);
    DisableItem(I_ICEDWARF);
    DisableItem(I_UNDERDWARF);
    DisableItem(I_SEAELF);

    EnableItem(I_CENTAURMAN);
    EnableItem(I_GOBLINMAN);

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
    DisableObject(O_BKEEP);
    DisableObject(O_PALACE);

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
    //ModifyItemProductionBooster(I_AXE, I_HAMMER, 1);

    // I reckon you should be able to transport LIVE and HORSES
    ModifyItemFlags(I_LIVESTOCK, 0);
    ModifyItemFlags(I_HORSE, 0);
    ModifyItemFlags(I_WHORSE, 0);

    // Allow YEW and MITH in other hexes (but rarely)
    // ModifyTerrainItems(int terrain, int i, int p, int c, int a)
    // stand for product, chance and amount
    ModifyTerrainItems(R_JUNGLE, 3, I_YEW, 10, 3);
    ModifyTerrainItems(R_JUNGLE, 4, I_IRONWOOD, 10, 3);

    ModifyTerrainItems(R_DESERT, 5, I_MITHRIL, 10, 5);

    ModifyTerrainItems(R_UFOREST, 3, I_MITHRIL, 10, 3);
    ModifyTerrainItems(R_UFOREST, 4, I_ROOTSTONE, 10, 3);
    ModifyTerrainItems(R_UFOREST, 5, I_YEW, 10, 3);
    ModifyTerrainItems(R_UFOREST, 6, I_IRONWOOD, 10, 3);

    ModifyTerrainItems(R_SWAMP, 4, I_IRONWOOD, 5, 3);
    ModifyTerrainItems(R_SWAMP, 5, I_YEW, 5, 3);

    // Reduce the cost of roads...
    ModifyObjectConstruction(O_ROADN, I_STONE, 40, "BUIL", 3);
    ModifyObjectConstruction(O_ROADNE, I_STONE, 40, "BUIL", 3);
    ModifyObjectConstruction(O_ROADNW, I_STONE, 40, "BUIL", 3);
    ModifyObjectConstruction(O_ROADS, I_STONE, 40, "BUIL", 3);
    ModifyObjectConstruction(O_ROADSE, I_STONE, 40, "BUIL", 3);
    ModifyObjectConstruction(O_ROADSW, I_STONE, 40, "BUIL", 3);

    // And modify some of the monsters...
    // Balrogs - weaker, less loot
    modify_monster_attacks_and_hits("BALR", 100, 100, 0, 1);
    modify_monster_skills("BALR", 4, 4, 4); // tact, obse, stea
    modify_monster_spoils("BALR", 20000, IT_MAGIC);

    // Dragons - tougher, more loot
    modify_monster_spoils("DRAG", 20000, IT_MAGIC);
    modify_monster_skills("DRAG", 4, 1, 4); // tact, obse, stea

    // Liches - tougher, more loot
    modify_monster_spoils("LICH", 20000, IT_MAGIC);
    modify_monster_skills("LICH", 4, 4, 5); // tact, obse, stea
    modify_monster_special("LICH", "black_wind", 1);
    modify_monster_defense("LICH", 5, 4);
    modify_monster_defense("LICH", 0, 5);

    // Beef up Undead and Skeletons slightly, too...
    modify_monster_special("SKEL", "fear", 1);
    modify_monster_attacks_and_hits("SKEL", 1, 2, 0, 1);
    modify_monster_defense("SKEL", 5, 2); // skel resist missile attacks
    modify_monster_special("UNDE", "fear", 3);
    modify_monster_attacks_and_hits("UNDE", 5, 10, 0, 1);
    modify_monster_defense("UNDE", 5, 1); // unde resist missile attacks slightly

    // Reduce spoils for mermen, pirates,
    modify_monster_spoils("MERF", 100, IT_NORMAL);
    modify_monster_spoils("PIRA", 200, IT_NORMAL);

    // Trolls should regenerate!
    modify_monster_attacks_and_hits("TROL", 32, 32, 4, 1);

    // Living Trees (ie Ents) should resist missile attacks
    modify_monster_defense("TREN", 5, 3);
    modify_monster_defense("TREN", 2, 5);

    // Why doesn't the sphinx have tact?
    modify_monster_skills("SPHI", 4, 3, 0); // tact, obse, stea

    // Ogres aren't very tough at all!
    modify_monster_attacks_and_hits("OGRE", 10, 20, 0, 1);
    modify_monster_skills("OGRE", 1, 0, 0); // tact, obse, stea

    // Cut out some of the crappier trade items.
    // With fewer trade items, the chance of good trade routes goes up.
    //DisableItem(I_IVORY);
    DisableItem(I_PEARL);
    DisableItem(I_JEWELRY);
    DisableItem(I_FIGURINES);
    DisableItem(I_TAROTCARDS);
    DisableItem(I_CAVIAR);
    //DisableItem(I_WINE);
    //DisableItem(I_SPICES);
    DisableItem(I_CHOCOLATE);
    DisableItem(I_TRUFFLES);
    DisableItem(I_VODKA);
    DisableItem(I_ROSES);
    DisableItem(I_PERFUME);
    //DisableItem(I_SILK);
    DisableItem(I_VELVET);
    //DisableItem(I_MINK);
    DisableItem(I_CASHMERE);
    DisableItem(I_COTTON);
    //DisableItem(I_DYES);
    DisableItem(I_WOOL);

    // Now modify the building defences
    // ModifyObjectDefence(int ob, int co, int en, int sp, int we, int ri, int ra)
    ModifyObjectDefence(O_TOWER, 1,0,0,0,1,1);
    ModifyObjectDefence(O_FORT, 2,1,0,1,2,2);
    ModifyObjectDefence(O_CASTLE, 3,2,1,2,3,3);
    ModifyObjectDefence(O_CITADEL, 4,3,2,3,4,4);
    ModifyObjectDefence(O_MFORTRESS, 2,3,3,3,2,2);

    // Make IMTH (improved mithril armor) better.
    modify_armor_save_value("IMTH", 0, 98);
    modify_armor_save_value("IMTH", 1, 98);
    modify_armor_save_value("IMTH", 2, 98);
    modify_armor_save_value("IMTH", 3, 98);
    modify_armor_save_value("IMTH", 4, 95);
    modify_armor_save_value("IMTH", 5, 80);
    modify_armor_save_value("IMTH", 6, 80);
    modify_armor_save_value("IMTH", 7, 80);

    // Redo all of the races for fracas!
    // Tundra has been disabled too!
    ClearTerrainRaces(R_PLAIN);
    ModifyTerrainRace(R_PLAIN, 0, I_PLAINSMAN);
    ModifyTerrainRace(R_PLAIN, 1, I_HIGHELF);
    ModifyTerrainRace(R_PLAIN, 2, I_CENTAURMAN);
    ModifyTerrainCoastRace(R_PLAIN, 0, I_VIKING);
    ModifyTerrainCoastRace(R_PLAIN, 1, I_HIGHELF);
    //ModifyTerrainCoastRace(R_PLAIN, 2, I_SEAELF);

    ClearTerrainRaces(R_FOREST);
    ModifyTerrainRace(R_FOREST, 0, I_VIKING);
    ModifyTerrainRace(R_FOREST, 1, I_HIGHELF);
    ModifyTerrainRace(R_FOREST, 2, I_CENTAURMAN);
    ModifyTerrainCoastRace(R_FOREST, 0, I_VIKING);
    ModifyTerrainCoastRace(R_FOREST, 1, I_HIGHELF);
    //ModifyTerrainCoastRace(R_FOREST, 2, I_SEAELF);

    ClearTerrainRaces(R_MOUNTAIN);
    ModifyTerrainRace(R_MOUNTAIN, 0, I_HILLDWARF);
    ModifyTerrainRace(R_MOUNTAIN, 1, I_ORC);
    ModifyTerrainRace(R_MOUNTAIN, 2, I_BARBARIAN);
    ModifyTerrainCoastRace(R_MOUNTAIN, 0, I_VIKING);
    ModifyTerrainCoastRace(R_MOUNTAIN, 1, I_HIGHELF);
    //ModifyTerrainCoastRace(R_MOUNTAIN, 2, I_SEAELF);

    ClearTerrainRaces(R_SWAMP);
    ModifyTerrainRace(R_SWAMP, 0, I_GOBLINMAN);
    ModifyTerrainRace(R_SWAMP, 1, I_ORC);
    ModifyTerrainRace(R_SWAMP, 2, I_DARKMAN);
    ModifyTerrainCoastRace(R_SWAMP, 0, I_GOBLINMAN);
    ModifyTerrainCoastRace(R_SWAMP, 1, I_ORC);
    //ModifyTerrainCoastRace(R_SWAMP, 2, I_SEAELF);

    ClearTerrainRaces(R_JUNGLE);
    ModifyTerrainRace(R_JUNGLE, 0, I_GOBLINMAN);
    ModifyTerrainRace(R_JUNGLE, 1, I_ORC);
    ModifyTerrainRace(R_JUNGLE, 2, I_DARKMAN);
    ModifyTerrainCoastRace(R_JUNGLE, 0, I_GOBLINMAN);
    ModifyTerrainCoastRace(R_JUNGLE, 1, I_ORC);
    ModifyTerrainCoastRace(R_JUNGLE, 2, I_VIKING);

    ClearTerrainRaces(R_DESERT);
    ModifyTerrainRace(R_DESERT, 0, I_CENTAURMAN);
    ModifyTerrainRace(R_DESERT, 1, I_DARKMAN);
    ModifyTerrainRace(R_DESERT, 2, I_PLAINSMAN);
    ModifyTerrainCoastRace(R_DESERT, 0, I_CENTAURMAN);
    ModifyTerrainCoastRace(R_DESERT, 1, I_PLAINSMAN);
    ModifyTerrainCoastRace(R_DESERT, 2, I_VIKING);

    // Anything else? What about enabling Hills?
    // Looks like we need to enable it in world.cpp too
    ClearTerrainRaces(R_CERAN_HILL);
    ModifyTerrainRace(R_CERAN_HILL, 0, I_HILLDWARF);
    ModifyTerrainRace(R_CERAN_HILL, 1, I_BARBARIAN);
    ModifyTerrainRace(R_CERAN_HILL, 2, I_ORC);
    ModifyTerrainCoastRace(R_CERAN_HILL, 0, I_VIKING);
    ModifyTerrainCoastRace(R_CERAN_HILL, 1, I_HILLDWARF);
    ModifyTerrainCoastRace(R_CERAN_HILL, 2, I_BARBARIAN);

    ClearTerrainItems(R_CERAN_HILL);
    ModifyTerrainItems(R_CERAN_HILL, 0, I_IRON, 80, 15);
    ModifyTerrainItems(R_CERAN_HILL, 1, I_STONE, 100, 20);
    ModifyTerrainItems(R_CERAN_HILL, 2, I_MITHRIL, 10, 5);
    ModifyTerrainItems(R_CERAN_HILL, 3, I_ROOTSTONE, 25, 5);
    ModifyTerrainItems(R_CERAN_HILL, 4, I_HERBS, 10, 10);

    // Ok, now for the rest of the racial stuff...
    ModifyItemBasePrice(I_GOBLINMAN, 40);
    ModifyItemBasePrice(I_HIGHELF, 75);
    ModifyItemBasePrice(I_CENTAURMAN, 75);
    ModifyItemBasePrice(I_HILLDWARF, 75);

    //modify_race_skill_levels(char *r, int spec, int def);
    //modify_race_skills(char *r, int i, char *sk);

    modify_race_skills("GBLN", 0, "XBOW");

    modify_race_skills("HELF", 0, "COMB");
    modify_race_skills("HELF", 1, "LBOW");
    modify_race_skills("HELF", 2, "LUMB");
    modify_race_skills("HELF", 3, "WEAP");
    modify_race_skills("HELF", 4, "HORS");
    modify_race_skills("HELF", 5, "ARMO");

    modify_race_skills("HDWA", 0, "COMB");
    modify_race_skills("HDWA", 1, "MINI");
    modify_race_skills("HDWA", 2, "ARMO");
    modify_race_skills("HDWA", 3, "WEAP");
    modify_race_skills("HDWA", 4, "QUAR");
    modify_race_skills("HDWA", 5, "BUIL");

    modify_race_skills("CTAU", 0, "HORS");
    modify_race_skills("CTAU", 1, "RIDI");
    modify_race_skills("CTAU", 2, "RANC");

    modify_race_skills("PLAI", 0, "FARM");
    modify_race_skills("PLAI", 1, "RANC");
    modify_race_skills("PLAI", 2, "HORS");
    modify_race_skills("PLAI", 3, "RIDI");

    modify_race_skills("VIKI", 0, "COMB");
    modify_race_skills("VIKI", 1, "SHIP");
    modify_race_skills("VIKI", 2, "SAIL");
    modify_race_skills("VIKI", 3, "LUMB");
    modify_race_skills("VIKI", 4, "CARP");

    return;
}

const std::optional<std::string> ARegion::movement_forbidden_by_ruleset(Unit *u, ARegion *origin, ARegionList& regs) {
    return std::nullopt;
}
