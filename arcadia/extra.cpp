// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER
//
// This file contains extra game-specific functions
//
#include "game.h"
#include "gamedata.h"

int Game::SetupFaction( Faction *pFac )
{
	pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 100;

	if(pFac->noStartLeader)
		return 1;

	//
	// Set up first unit.
	//
	Unit *temp2 = GetNewUnit( pFac );
	temp2->SetMen( I_VIKING, 1 );
	temp2->reveal = REVEAL_FACTION;

	if(!Globals->ARCADIA_MAGIC) {
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
	}

	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
	{
		if (!(ItemDefs[I_FOOD].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_FOOD, 6);
		else if (!(ItemDefs[I_FISH].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_FISH, 6);
		else if (!(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_LIVESTOCK, 6);
		else if (!(ItemDefs[I_GRAIN].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_GRAIN, 2);
		temp2->items.SetNum(I_SILVER, 10);
	}

	ARegion *reg = NULL;
	if(pFac->pStartLoc) {
		reg = pFac->pStartLoc;
	} else if(!Globals->MULTI_HEX_NEXUS) {
		reg = (ARegion *)(regions.First());
	} else {
		ARegionArray *pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
		while(!reg) {
			reg = pArr->GetRegion(pFac->num%4, getrandom(pArr->y));
		}
	}
	temp2->MoveUnit( reg->GetDummy() );

	return( 1 );
}

Faction *Game::CheckVictory(AString *victoryline)
{
	int cities = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		if(r->town && r->town->TownType() == TOWN_CITY) cities++;
	}
	numcities = cities;
	cities = (cities*3+4)/5;  // 3/5 of all cities, rounded up.
	
	Faction *winner = NULL;
	CountGuardedCities();
	forlist_reuse(&factions) {
		Faction *f = (Faction *) elem;
		if(f->IsNPC()) continue; //don't want the Guardsmen winning on day 1.
		
		if(f->guardedcities.Value() >= cities) {
            if(!winner) winner = f;
            else if(f->guardedcities.Value() >= winner->guardedcities.Value()) winner = f;
        }
	}
	if(winner && cities >= 1) {
    	*victoryline += *(winner->name) + " has won the game by guarding " + winner->guardedcities.Value() + " cities!";
        return winner;
    }
    //warning message when faction gets close to victory.
    cities = (numcities*2+4)/5;  // 2/5 of all cities, rounded up.
	forlist_reuse(&factions) {
		Faction *f = (Faction *) elem;
		if(f->IsNPC()) continue;
		if(f->guardedcities.Value() >= cities) {
		    AString message = AString(*f->name) + " nears victory, guarding " + f->guardedcities.Value() + " out of " + numcities + " cities.";
            forlist(&factions) {
                Faction *rfac = (Faction *) elem;
                rfac->Message(message);
            }
        }
	}

	return winner;
}

void Game::ModifyTablesPerRuleset(void)
{
	if(Globals->APPRENTICES_EXIST)
		EnableSkill(S_MANIPULATE);

	if(!Globals->GATES_EXIST)
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
		ModifyAttribMod("stealth", 4, AttribModItem::SKILL,
				"INVI", AttribModItem::UNIT_LEVEL, 1);
	}

	if(Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
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

	if(Globals->ARCADIA_MAGIC) {
	    ModifyAttribMod("stealth", 5, AttribModItem::SKILL,"INVI", AttribModItem::UNIT_LEVEL_HALF, 1);  //make sure 6 slots available - default version has only 5.
	    ModifyAttribMod("stealth", 2, AttribModItem::FLAGGED,"invis", AttribModItem::CONSTANT, 2); //2 down from 3
	    ModifyAttribMod("entertainment", 0, AttribModItem::SKILL,"PHEN", AttribModItem::UNIT_LEVEL, 10);    
	
	
	    //Fundamental Skills
	    EnableSkill(S_BASE_WINDKEY);
	    EnableSkill(S_BASE_SUMMONING);
	    EnableSkill(S_BASE_PATTERNING);
	    EnableSkill(S_BASE_MYSTICISM);
	    EnableSkill(S_BASE_BATTLETRAINING);
	    EnableSkill(S_BASE_CHARISMA);
	    EnableSkill(S_BASE_ARTIFACTLORE);
	    
	    //Other Skills
	    EnableSkill(S_BLIZZARD);
	    EnableSkill(S_FOG);
	    EnableSkill(S_CONCEALMENT);
//	    EnableSkill(S_ILLUSORY_CREATURES);
//	    EnableSkill(S_ILLUSORY_WOUNDS);
	    EnableSkill(S_INSTILL_COURAGE);
	    EnableSkill(S_SUMMON_MEN);
	    EnableSkill(S_RESURRECTION);
	    EnableSkill(S_SPIRIT_OF_DEAD);
	    EnableSkill(S_INNER_STRENGTH);
	    EnableSkill(S_TRANSMUTATION);
//	    EnableSkill(S_TRANSFIGURATION);
	    EnableSkill(S_MODIFICATION);
//	    EnableSkill(S_REJUVENATION);
//	    EnableSkill(S_SEAWARD);
	    EnableSkill(S_DIVERSION);
	    EnableSkill(S_GRYFFIN_LORE);
	    EnableSkill(S_HYPNOSIS);
	    EnableSkill(S_BINDING);
	    EnableSkill(S_CREATE_PORTAL);
	    EnableSkill(S_LIGHT);
	    EnableSkill(S_DARKNESS);
	    EnableSkill(S_DRAGON_TALK);
	    EnableSkill(S_TOUGHNESS);
	    EnableSkill(S_UNITY);
	    EnableSkill(S_FRENZY);
	    EnableSkill(S_SECSIGHT);
	    EnableSkill(S_SWIFTNESS);
	    EnableSkill(S_TRADING);
	    EnableSkill(S_MERCHANTRY);
	    EnableSkill(S_ARCADIA_QUARTERMASTERY);
	    

	    DisableSkill(S_FORCE);
	    DisableSkill(S_PATTERN);
	    DisableSkill(S_SPIRIT);
	    DisableSkill(S_PORTAL_LORE);
	    DisableSkill(S_WEATHER_LORE);
	    DisableSkill(S_SUMMON_SKELETONS);
	    DisableSkill(S_DEMON_LORE);
	    DisableSkill(S_ILLUSION);
	    DisableSkill(S_CREATE_PHANTASMAL_BEASTS);
	    DisableSkill(S_CREATE_PHANTASMAL_UNDEAD);
	    DisableSkill(S_CREATE_PHANTASMAL_DEMONS);
	    DisableSkill(S_DISPEL_ILLUSIONS);
	    DisableSkill(S_CREATE_AMULET_OF_PROTECTION);
	    DisableSkill(S_CREATE_SHIELDSTONE);
	    DisableSkill(S_CONSTRUCT_PORTAL);
	    DisableSkill(S_DRAGON_LORE);
	    DisableSkill(S_ARTIFACT_LORE);
	    DisableSkill(S_EARTHQUAKE);
	    DisableSkill(S_TACTICS);
	    DisableSkill(S_MIND_READING);
	    DisableSkill(S_SUMMON_BLACK_WIND);



        //Skilltree stuff
        ModifySkillName(S_FIRE,"fireball","FIRE");
	    ModifySkillDependancy(S_FIRE, 0, "WKEY", 1);
	    ModifySkillDependancy(S_FORCE_SHIELD, 0, "FIRE", 3);
	    ModifySkillDependancy(S_FARSIGHT, 0, "WKEY", 1);
	    ModifySkillDependancy(S_FARSIGHT, 1, NULL, 0);
	    ModifySkillDependancy(S_SUMMON_WIND, 0, "WKEY", 1);
	    ModifySkillDependancy(S_SUMMON_STORM, 0, "SWIN", 1);
	    ModifySkillDependancy(S_SUMMON_TORNADO, 0, "SWIN", 4);
	    ModifySkillDependancy(S_CALL_LIGHTNING, 0, "SWIN", 6);
	    ModifySkillDependancy(S_CLEAR_SKIES, 0, "WKEY", 1);
	    ModifySkillRange(S_CLEAR_SKIES, "rng_linear");
//	    ModifySkillDependancy(S_BLIZZARD, 0, "WKEY", 4);
//	    ModifySkillDependancy(S_FOG, 0, "WKEY", 1);
	    ModifySkillDependancy(S_FOG, 1, NULL, 0);
	    ModifySkillFlags(S_FOG, SkillType::MAGIC | SkillType::COMBAT | SkillType::CAST | SkillType::MAGEOTHER);
	    
	    ModifySkillDependancy(S_GATE_LORE, 0, "MYST", 1);
	    ModifySkillDependancy(S_GATE_LORE, 1, NULL, 0);
	    ModifySkillDependancy(S_TELEPORTATION, 0, "GATE", 3);
	    ModifySkillDependancy(S_TELEPORTATION, 1, NULL, 0);
	    ModifySkillDependancy(S_CONSTRUCT_GATE, 0, "GATE", 4);
	    ModifySkillDependancy(S_CONSTRUCT_GATE, 1, NULL, 0);
//	    ModifySkillDependancy(S_CREATE_PORTAL, 0, "TELE", 4);
	    ModifySkillDependancy(S_EARTH_LORE, 0, "PTTN", 1);
	    ModifySkillDependancy(S_MAGICAL_HEALING, 0, "PTTN", 1);
	    ModifySkillDependancy(S_RESURRECTION, 0, "NECR", 3);
//	    ModifySkillDependancy(S_RESURRECTION, 1, "MHEA", 3);
//	    ModifySkillDependancy(S_SUMMON_SPIRIT_OF_DEAD, 1, "RESU", 3);
//	    ModifySkillDependancy(S_MODIFICATION, 0, "EART", 4);
//	    ModifySkillDependancy(S_DIVERSION, 0, "EART", 4);
	    
	    ModifySkillDependancy(S_CONCEALMENT, 0, "MYST", 2);
	    ModifySkillDependancy(S_INVISIBILITY, 0, "CONC", 5);
	    ModifySkillDependancy(S_TRUE_SEEING, 0, "CONC", 4);
	    ModifySkillDependancy(S_TRANSMUTATION, 0, "MYST", 1);
	    ModifySkillDependancy(S_ENCHANT_SWORDS, 0, "TRAM", 1);
	    ModifySkillDependancy(S_ENCHANT_ARMOR, 0, "TRAM", 1);
	    ModifySkillDependancy(S_ENERGY_SHIELD, 0, "MYST", 1);
	    //hypnosis
        //binding
        //dragonlore
	    
	    ModifySkillDependancy(S_SPIRIT_SHIELD, 0, "SUMM", 1);
	    ModifySkillDependancy(S_SPIRIT_SHIELD, 1, NULL, 0);
	    ModifySkillDependancy(S_NECROMANCY, 0, "SUMM", 2);
	    ModifySkillDependancy(S_NECROMANCY, 1, NULL, 0);
	    ModifySkillDependancy(S_RAISE_UNDEAD, 0, "NECR", 3);
	    ModifySkillDependancy(S_SUMMON_LICH, 0, "RAIS", 4);
	    ModifySkillDependancy(S_BANISH_UNDEAD, 0, "NECR", 1);
	    ModifySkillDependancy(S_LIGHT, 0, "SUMM", 1);
	    ModifySkillDependancy(S_DARKNESS, 0, "LIGT", 3);
	    ModifySkillDependancy(S_SUMMON_MEN, 0, "SUMM", 1);
        ModifySkillName(S_DEMON_LORE,"blankb","ZZZB");
        ModifySkillName(S_SUMMON_IMPS,"demon lore","DEMO");
        ModifyItemMagicSkill(I_IMP, "DEMO", 1);
	    ModifySkillDependancy(S_SUMMON_IMPS, 0, "SUMM", 1);
	    ModifySkillFlags(S_SUMMON_IMPS, SkillType::MAGIC | SkillType::NOTIFY | SkillType::CAST | SkillType::COSTVARIES);
	    ModifySkillDependancy(S_SUMMON_DEMON, 0, "DEMO", 3);
	    ModifySkillDependancy(S_SUMMON_BALROG, 0, "SUDE", 4);
	    ModifySkillDependancy(S_BANISH_DEMONS, 0, "DEMO", 1);
	    
	    ModifySkillFlags(S_BASE_ARTIFACTLORE, SkillType::MAGIC | SkillType::NOTIFY | SkillType::CAST | SkillType::FOUNDATION);
	    ModifyItemMagicSkill(I_SHIELDSTONE, "ARTL", 3);
	    ModifyItemMagicSkill(I_AMULETOFP, "ARTL", 1);
	    ModifySkillDependancy(S_CREATE_RING_OF_INVISIBILITY, 0, "INVI", 5);
	    ModifySkillDependancy(S_CREATE_RING_OF_INVISIBILITY, 1, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_AMULET_OF_TRUE_SEEING, 0, "TRUE", 4);
	    ModifySkillDependancy(S_CREATE_AMULET_OF_TRUE_SEEING, 1, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_CLOAK_OF_INVULNERABILITY, 0, "SSHI", 3);
	    ModifySkillDependancy(S_CREATE_CLOAK_OF_INVULNERABILITY, 1, "FSHI", 3);
	    ModifySkillDependancy(S_CREATE_CLOAK_OF_INVULNERABILITY, 2, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_FIRE, 0, "FIRE", 3);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_FIRE, 1, "ARTL", 1);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_LIGHTNING, 0, "CALL", 3);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_LIGHTNING, 1, "ARTL", 5);
	    ModifySkillDependancy(S_CREATE_RUNESWORD, 0, "FEAR", 2);
	    ModifySkillDependancy(S_CREATE_RUNESWORD, 1, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_MAGIC_CARPET, 1, "ARTL", 2);
	    ModifySkillDependancy(S_CREATE_MAGIC_CARPET, 1, "SWIN", 3);
	    ModifySkillDependancy(S_ENGRAVE_RUNES_OF_WARDING, 0, "ARTL", 1);
	    ModifySkillDependancy(S_ENGRAVE_RUNES_OF_WARDING, 1, "SSHI", 1);
	    ModifySkillDependancy(S_ENGRAVE_RUNES_OF_WARDING, 2, "ESHI", 1);
	    
	    ModifySkillDependancy(S_INNER_STRENGTH, 0, "BATT", 2);
	    ModifySkillDependancy(S_PHANTASMAL_ENTERTAINMENT, 0, "BATT", 1);
        ModifySkillName(S_PHANTASMAL_ENTERTAINMENT,"gladiator","GLAD");
	    ModifySkillDependancy(S_CREATE_AURA_OF_FEAR, 0, "FREN", 1);
	    ModifySkillDependancy(S_INSTILL_COURAGE, 0, "FREN", 1);
	    
	    
	    
	    ModifySkillDependancy(S_WOLF_LORE, 0, "CHAR", 1);
	    ModifySkillDependancy(S_BIRD_LORE, 0, "CHAR", 1);
	    //gryffin lore
	    //unity
	    //trading
	    //merchantry
	    //quartermaster
	    

	    ModifyItemMagicInput(I_AMULETOFTS, 0, I_IRON, 8);
	    ModifyItemMagicInput(I_RINGOFI, 0, I_MITHRIL, 4);
	    ModifyItemMagicInput(I_CLOAKOFI, 0, I_FUR, 8);
	    ModifyItemMagicInput(I_CLOAKOFI, 1, I_PEARL, 4);
	    ModifyItemMagicInput(I_STAFFOFF, 0, I_WOOD, 8);
	    ModifyItemMagicInput(I_STAFFOFL, 0, I_WOOD, 8);
	    ModifyItemMagicInput(I_STAFFOFL, 0, I_GEMS, 8);
	    ModifyItemMagicInput(I_AMULETOFP, 0, I_IRON, 1);
	    ModifyItemMagicInput(I_SHIELDSTONE, 0, I_IRON, 1);
	    ModifyItemMagicInput(I_RUNESWORD, 0, I_MSWORD, 1);
	    ModifyItemMagicInput(I_MCARPET, 0, I_HERBS, 1);

	    //Illusory Creatures Stuff
   	    EnableItem(I_IGRYFFIN);
   	    DisableItem(I_IWOLF);
   	    DisableItem(I_IIMP);
   	    DisableItem(I_ISKELETON);
   	    DisableItem(I_IEAGLE);
   	    DisableItem(I_IDEMON);
   	    DisableItem(I_IUNDEAD);
   	    DisableItem(I_ILICH);
   	    DisableItem(I_IBALROG);
   	    DisableItem(I_IDRAGON);
/*   	    ModifyItemMagicSkill(I_IWOLF, "ILCR", 1);
   	    ModifyItemMagicSkill(I_IIMP, "ILCR", 1);
   	    ModifyItemMagicSkill(I_ISKELETON, "ILCR", 1);
   	    ModifyItemMagicSkill(I_IEAGLE, "ILCR", 2);
   	    ModifyItemMagicSkill(I_IDEMON, "ILCR", 2);
   	    ModifyItemMagicSkill(I_IUNDEAD, "ILCR", 2);
   	    ModifyItemMagicSkill(I_ILICH, "ILCR", 3);
   	    ModifyItemMagicSkill(I_IBALROG, "ILCR", 3);
   	    ModifyItemMagicSkill(I_IDRAGON, "ILCR", 4);*/
   	    
   	    //new Create Portal stuff
   	    ModifyRangeClass("rng_teleport", RangeType::RNG_LEVEL);
   	    ModifyRangeFlags("rng_teleport", RangeType::RNG_SURFACE_ONLY); //to prevent teleportation in the labryinth
   	    ModifyRangeMultiplier("rng_teleport", 4);
   	    EnableObject(O_ESEAPORTAL);
   	    
   	    //Monster stuff
  	    //dragon cannot be summoned, only captured
        ModifyMonsterAttacksAndHits("DRAG", 150, 150, 0);
        ModifySpecialDamage("firebreath", 0, ATTACK_ENERGY,2,40, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifyItemCapacities(I_DRAGON, 900, 900, 900, 900);
        ModifyItemWeight(I_DRAGON, 800);
        ModifyMonsterSkills("DRAG", 6, 0, 6); //tact, stea, obse
        ModifyMonsterAttackLevel("DRAG", 7);
        ModifyMonsterDefense("DRAG", ATTACK_COMBAT, 7);
        ModifyMonsterDefense("DRAG", ATTACK_ENERGY, 5);
        ModifyMonsterDefense("DRAG", ATTACK_SPIRIT, 5);
        ModifyMonsterDefense("DRAG", ATTACK_WEATHER, 5);
        ModifyMonsterDefense("DRAG", ATTACK_RIDING, 6);
        ModifyMonsterDefense("DRAG", ATTACK_RANGED, 1);
        
        EnableItem(I_BABYDRAGON);
        
        //balrog 40 energy to summon, 4 to maintain, ~2.5% escape chance (average 5 to maintain).
        ModifyMonsterAttacksAndHits("BALR", 50, 50, 0);
        ModifySpecialDamage("hellfire", 0, ATTACK_ENERGY,2,20, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifyMonsterSpecial("BALR", "hellfire", 3);
        ModifyItemCapacities(I_BALROG, 300, 300, 0, 0);
        ModifyItemWeight(I_BALROG, 250);
        ModifyItemFlags(I_BALROG, ItemType::CANTGIVE | ItemType::NOTRANSPORT);
        ModifyMonsterSkills("BALR", 4, 4, 5);
        ModifyMonsterDefense("BALR", ATTACK_ENERGY, 3);
        ModifyMonsterDefense("BALR", ATTACK_SPIRIT, 3);
        ModifyMonsterDefense("BALR", ATTACK_WEATHER, 3);
        ModifyMonsterThreat("BALR", 2, 75);
        //escape changed in gamedata to: escape_lev_square, suba, 10, ((n^2 / 10s^2) chance of escape, ie 1 in 40 turns typical).
        //max_inventory changed in gamedata to 0.

        ModifyItemFlags(I_DEMON, ItemType::CANTGIVE | ItemType::NOTRANSPORT);
        ModifyMonsterDefense("DEMO", ATTACK_ENERGY, 2);
        ModifyMonsterDefense("DEMO", ATTACK_SPIRIT, 3);
        ModifyMonsterDefense("DEMO", ATTACK_WEATHER, 2);
        //escape changed in gamedata to: escape_lev_quad, sude, 20, (n^2 / 20s^4) chance, ie 1 in 20 typical if n=s^2.

        ModifyItemFlags(I_IMP, ItemType::CANTGIVE | ItemType::NOTRANSPORT);
        ModifyMonsterDefense("IMP", ATTACK_ENERGY, 2);
        ModifyMonsterDefense("IMP", ATTACK_SPIRIT, 2);
        ModifyMonsterDefense("IMP", ATTACK_WEATHER, 1);
        //escape changed in gamedata to: escape_lev_quad, suim, 320, (n^2 / 320s^4) chance, ie 1 in 20 typical if n=4s^2.
        
        //lich 30 energy to summon, 0 to maintain, 10% decay chance (average 3 to maintain if count recast cost).
        ModifyMonsterAttacksAndHits("LICH", 40, 40, 0);
        ModifySpecialDamage("fear", 0, ATTACK_SPIRIT,2,40,  WeaponType::RANGED,MAGIC_SPIRIT, "fear");
        ModifyMonsterSpecial("LICH", "fear", 4);
        ModifyItemCapacities(I_LICH, 150, 0, 0, 0);
        ModifyItemWeight(I_LICH, 100);
        ModifyMonsterSkills("LICH", 4, 4, 5);
        ModifyMonsterAttackLevel("LICH", 5);
        ModifyMonsterDefense("LICH", ATTACK_COMBAT, 5);
        ModifyMonsterDefense("LICH", ATTACK_ENERGY, 4);
        ModifyMonsterDefense("LICH", ATTACK_SPIRIT, 5);
        ModifyMonsterDefense("LICH", ATTACK_WEATHER, 3);
        ModifyMonsterDefense("LICH", ATTACK_RIDING, 5);
        ModifyMonsterThreat("LICH", 3, 50);
        
   	    //gryffin free to summon, free to maintain
   	    EnableItem(I_GRYFFIN);
   	    ModifyMonsterAttacksAndHits("GRYF", 33, 33, 0);
        ModifyMonsterSkills("GRYF", 4, 2, 5);
        ModifyMonsterAttackLevel("GRYF", 7);
        ModifyMonsterDefense("GRYF", ATTACK_COMBAT, 7);
        ModifyMonsterDefense("GRYF", ATTACK_ENERGY, 5);
        ModifyMonsterDefense("GRYF", ATTACK_SPIRIT, 5);
        ModifyMonsterDefense("GRYF", ATTACK_WEATHER, 7);
        ModifyMonsterDefense("GRYF", ATTACK_RIDING, 6);
        ModifyMonsterThreat("GRYF", 2, 25);

        ModifyItemCapacities(I_EAGLE, 30, 30, 30, 0);
        ModifyItemWeight(I_EAGLE, 20);
        ModifyMonsterAttacksAndHits("EAGL", 3, 3, 0);
        ModifyMonsterSkills("EAGL", 2, 3, 4);
        ModifyMonsterAttackLevel("EAGL", 3);
        ModifyMonsterDefense("EAGL", ATTACK_COMBAT, 3);
        ModifyMonsterThreat("EAGL", 12, 10);
        
        ModifyMonsterDefense("WOLF", ATTACK_WEATHER, 2);

        //kraken slightly less scary
        EnableItem(I_KRAKEN);
        ModifyMonsterAttacksAndHits("KRAK", 40, 40, 0);
        ModifyMonsterSpecial("KRAK", "fear", 3);
        ModifyMonsterDefense("KRAK", ATTACK_ENERGY, 3);
        ModifyMonsterDefense("KRAK", ATTACK_SPIRIT, 4);
        ModifyMonsterDefense("KRAK", ATTACK_WEATHER, 4);
        ModifyItemCapacities(I_KRAKEN, 0, 0, 0, 250);
        ModifyItemWeight(I_KRAKEN, 200);

        //multi-headed hydra
        EnableItem(I_HYDRA);
        ModifyMonsterAttacksAndHits("HYDR", 60, 30, 5);
        ModifyMonsterDefense("HYDR", ATTACK_SPIRIT, 1);
        ModifyMonsterDefense("HYDR", ATTACK_WEATHER, 2);
        ModifyMonsterSpecial("HYDR", "firebreath", 1);
        ModifyMonsterThreat("HYDR", 2, 50);

        ModifySpecialDamage("fireball", 0, ATTACK_ENERGY,2,10, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifySpecialDamage("lightning", 0, ATTACK_WEATHER,2,50, WeaponType::RANGED,MAGIC_WEATHER,0);
        ModifySpecialDamage("lightning", 1, ATTACK_ENERGY,2,50, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifySpecialDamage("mindblast", 0, ATTACK_SPIRIT,2,125, WeaponType::RANGED,MAGIC_SPIRIT,0);
        ModifySpecialDamage("earthquake", 0, ATTACK_COMBAT,2,50, WeaponType::RANGED,ARMORPIERCING,0); // should this be num_attack_types?
        ModifySpecialDamage("dispel_illusion", 0, NUM_ATTACK_TYPES,2,50, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifySpecialDamage("storm", 0, ATTACK_WEATHER,2,200, WeaponType::RANGED,MAGIC_WEATHER,"storm");
        ModifyEffectFlags("storm", EffectType::EFF_ONESHOT);
        ModifySpecialDamage("tornado", 0, ATTACK_WEATHER,2,50, WeaponType::RANGED,MAGIC_WEATHER, 0);
        ModifySpecialDamage("black_wind", 0, ATTACK_SPIRIT,2,150, WeaponType::RANGED,MAGIC_SPIRIT,0);
        ModifySpecialDamage("banish_undead", 0, NUM_ATTACK_TYPES,2,60, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifySpecialDamage("banish_demon", 0, NUM_ATTACK_TYPES,2,60, WeaponType::RANGED,MAGIC_SPIRIT,0);
        ModifySpecialDamage("icebreath", 0, ATTACK_WEATHER,2,5, WeaponType::RANGED,MAGIC_WEATHER,0);
        ModifySpecialDamage("light", 0, ATTACK_ENERGY,2,10, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifySpecialDamage("wounds", 0, ATTACK_SPIRIT,2,40,  WeaponType::RANGED,MAGIC_SPIRIT, "wounds");
        ModifySpecialDamage("courage", 0, NUM_ATTACK_TYPES,2,40,  WeaponType::RANGED,NUM_WEAPON_CLASSES, "courage");

    //Get rid of dragons from the terrain table. Also, level 3 is Arcadia specific.
        ModifyTerrainWMons(R_CAVERN, 4, I_RAT, I_HYDRA, I_GOBLIN);
        ModifyTerrainWMons(R_UFOREST, 3, I_SPIDER, I_GRYFFIN, I_TROLL);
        ModifyTerrainWMons(R_TUNNELS, 4, I_LIZARD, I_HYDRA, I_ETTIN);
        ModifyTerrainWMons(R_GROTTO, 1, I_RAT, I_IWURM, I_GOBLIN);
        ModifyTerrainWMons(R_DFOREST, 1, I_DEMON, I_GRYFFIN, I_TROLL);
        ModifyTerrainWMons(R_CHASM, 2, I_RAT, I_BALROG, I_ETTIN);
        
        
        //Races for new advancement style
        DisableItem(I_LEADERS);
        ModifyItemName(I_VIKING,"raider","raiders","RAID");
        ModifyItemName(I_PLAINSMAN,"plains dwarf","plains dwarves","PDWA");
        ModifyItemName(I_DARKMAN,"ancient elf","ancient elves","AELF");
	    EnableItem(I_MERMEN);
	    
	    ModifyRaceSkills("BARB", "COMB","MINI","WEAP","OBSE");
	    ModifyRaceSkills("ESKI", "HEAL","BUIL","FISH","HERB");
	    ModifyRaceSkills("NOMA", "RIDI","RANC","HORS","ENTE");
	    ModifyRaceSkills("TMAN", "LUMB","CONS","FARM","OBSE");
	    
	    ModifyRaceSkills("WELF", "LUMB","LBOW","FARM","ENTE");
	    ModifyRaceSkills("SELF", "FISH","SAIL","CONS","RANC");
	    ModifyRaceSkills("HELF", "HORS","ENTE","RIDI","HEAL");
	    ModifyRaceSkills("TELF", "HERB","HEAL","STEA","LBOW");
	    
	    ModifyRaceSkills("IDWA", "COMB","BUIL","HERB","GCUT");
	    ModifyRaceSkills("HDWA", "COMB","MINI","WEAP","ARMO");
	    ModifyRaceSkills("UDWA", "XBOW","MINI","QUAR","GCUT");
	    ModifyRaceSkills("DDWA", "XBOW","QUAR","ARMO","RANC");
	    
	    ModifyRaceSkills("MERM", "STEA","FISH","GCUT","DOLP");
	    ModifyRaceSkills("ORC", "COMB");
	}
	
	if(Globals->REAL_EXPERIENCE) {
//	    ModifyRaceSkillLevels("LEAD",3,3);
	    ModifyRaceSkillLevels("RAID",2,1);
	    ModifyRaceSkillLevels("BARB",2,1);
	    ModifyRaceSkillLevels("ESKI",2,1);
	    ModifyRaceSkillLevels("NOMA",2,1);
	    ModifyRaceSkillLevels("TMAN",2,1);
	    ModifyRaceSkillLevels("AELF",2,1);
	    ModifyRaceSkillLevels("WELF",2,1);
	    ModifyRaceSkillLevels("SELF",2,1);
	    ModifyRaceSkillLevels("HELF",2,1);
	    ModifyRaceSkillLevels("TELF",2,1);
	    ModifyRaceSkillLevels("PDWA",2,1);
	    ModifyRaceSkillLevels("IDWA",2,1);
	    ModifyRaceSkillLevels("HDWA",2,1);
	    ModifyRaceSkillLevels("UDWA",2,1);
	    ModifyRaceSkillLevels("DDWA",2,1);
	    ModifyRaceSkillLevels("ORC",3,0);
/*	    ModifyRaceSkillLevels("MAN",3,3);
	    ModifyRaceSkillLevels("FAIR",3,1);
	    ModifyRaceSkillLevels("LIZA",2,1);
	    ModifyRaceSkillLevels("URUK",3,1);
	    ModifyRaceSkillLevels("GBLN",2,1);
	    ModifyRaceSkillLevels("HOBB",2,1);
	    ModifyRaceSkillLevels("GNOL",2,1);
	    ModifyRaceSkillLevels("DRLF",2,1);
	    ModifyRaceSkillLevels("MERC",1,1);
	    ModifyRaceSkillLevels("TITA",2,1);
	    ModifyRaceSkillLevels("AMAZ",2,1);
	    ModifyRaceSkillLevels("AMAZ",2,1);	    
        ModifyRaceSkillLevels("OGER",3,1);	    	    
	    ModifyRaceSkillLevels("GNOM",2,1);
	    ModifyRaceSkillLevels("HILA",2,1);
	    ModifyRaceSkillLevels("MINO",3,1);
	    ModifyRaceSkillLevels("GELF",2,1);
*/
	    
	    ModifyItemProductionSkill(I_YEW, "LUMB", 6);
	    ModifyItemProductionSkill(I_IRONWOOD, "LUMB", 4);
	    ModifyItemProductionSkill(I_DOUBLEBOW, "WEAP", 6);
	    ModifyItemProductionSkill(I_MITHRIL, "MINI", 4);
	    ModifyItemProductionSkill(I_MSWORD, "WEAP", 4);
	    ModifyItemProductionSkill(I_MPLATE, "ARMO", 6);
	    
	    ModifyItemProductionSkill(I_FLOATER, "FLOA", 4);
	    ModifyItemProductionSkill(I_ROOTSTONE, "QUAR", 3);
	    ModifyItemProductionSkill(I_PPLATE, "GCUT", 4);
	    ModifyItemProductionSkill(I_WHORSE, "HORS", 6);
     	ModifyItemProductionSkill(I_MUSHROOM, "HERB", 4);
     	ModifyItemProductionSkill(I_HEALPOTION, "HEAL", 4);
     	ModifyMountBonuses("WING", 4, 8, 3);
     	ModifyItemType(I_MCARPET, IT_MAGIC|IT_MOUNT);
	    
	    ModifyObjectConstruction(O_BALLOON, I_FLOATER, 50, "SHIP", 5);
	    ModifyObjectConstruction(O_AGALLEON, I_IRONWOOD, 100, "SHIP", 4);
	    // Leave temples, stables, road etc. as they are.
    }



/*  Standard Arcadia Changes.  */
/*  All primary items $30, all secondary items $60
    Introduction of pearls & PPAR
    Armour table changes
    */
/*
    ModifyMonsterAttackLevel("iWOLF", -10);    
    ModifyMonsterDefense("iWOLF", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iWOLF", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iWOLF", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iWOLF", 0, 10, 0);
    ModifyMonsterAttackLevel("iIMP", -10);    
    ModifyMonsterDefense("iIMP", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iIMP", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iIMP", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iIMP", 0, 10, 0);
    ModifyMonsterAttackLevel("iSKEL", -10);    
    ModifyMonsterDefense("iSKEL", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iSKEL", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iSKEL", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iSKEL", 0, 10, 0);
    ModifyMonsterAttackLevel("iEAGL", -10);    
    ModifyMonsterDefense("iEAGL", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iEAGL", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iEAGL", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iEAGL", 0, 10, 0);
    ModifyMonsterAttackLevel("iDEMO", -10);    
    ModifyMonsterDefense("iDEMO", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iDEMO", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iDEMO", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iDEMO", 0, 10, 0);
    ModifyMonsterAttackLevel("iUNDE", -10);    
    ModifyMonsterDefense("iUNDE", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iUNDE", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iUNDE", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iUNDE", 0, 10, 0);
    ModifyMonsterAttackLevel("iGRYF", -10);    
    ModifyMonsterDefense("iGRYF", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iGRYF", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iGRYF", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iGRYF", 0, 10, 0);
    ModifyMonsterAttackLevel("iDRAG", -10);    
    ModifyMonsterDefense("iDRAG", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iDRAG", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iDRAG", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iDRAG", 0, 10, 0);
    ModifyMonsterAttackLevel("iBALR", -10);    
    ModifyMonsterDefense("iBALR", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iBALR", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iBALR", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iBALR", 0, 10, 0);
    ModifyMonsterAttackLevel("iLICH", -10);    
    ModifyMonsterDefense("iLICH", ATTACK_COMBAT, -10);
    ModifyMonsterDefense("iLICH", ATTACK_RIDING, -10);
    ModifyMonsterDefense("iLICH", ATTACK_RANGED, -10);
    ModifyMonsterSkills("iLICH", 0, 10, 0);
    */
    
    
    ModifyMonsterAttacksAndHits("TROL", 4, 4, 2);
    
    ModifyMonsterThreat("ANAC", 8, 10);
    ModifyMonsterThreat("SCOR", 6, 10);
    ModifyMonsterThreat("POLA", 2, 10);
    ModifyMonsterThreat("GRAT", 60, 10);
    ModifyMonsterThreat("ROC", 3, 25);
    ModifyMonsterThreat("ICEW", 12, 25);
    ModifyMonsterSpecial("ICEW", "icebreath", 2);
    ModifyMonsterThreat("LMAN", 16, 10);
    ModifyMonsterThreat("WMAN", 20, 10);
    ModifyMonsterThreat("SAND", 16, 10);
    ModifyMonsterThreat("PIRA", 30, 70);
    ModifyMonsterThreat("MERF", 80, 10);
    ModifyMonsterThreat("UNDE", 16, 25);
    ModifyMonsterThreat("iWOLF", 2000, 10);
    ModifyMonsterThreat("iEAGL", 400, 10);
    ModifyMonsterThreat("iGRYF", 100, 10);
    ModifyMonsterSpoils("LION", 250, -1);
    ModifyMonsterSpoils("WOLF", 150, -1);
    ModifyMonsterSpoils("GRIZ", 500, -1);
    ModifyMonsterSpoils("CROC", 180, -1);
    ModifyMonsterSpoils("ANAC", 180, -1);
    ModifyMonsterSpoils("SCOR", 250, -1);
    ModifyMonsterSpoils("POLA", 600, -1);
    ModifyMonsterSpoils("GRAT", 29, IT_NORMAL);
    ModifyMonsterSpoils("GSPI", 350, -1);
    ModifyMonsterSpoils("GLIZ", 700, -1);
    ModifyMonsterSpoils("TREN", 400, IT_ADVANCED);
    ModifyMonsterSpoils("ROC", 1100, IT_ADVANCED);
    ModifyMonsterSpoils("BOGT", 1000, IT_ADVANCED);
    ModifyMonsterSpoils("KONG", 1200, IT_ADVANCED);
    ModifyMonsterSpoils("SPHI", 3000, IT_ADVANCED);
    ModifyMonsterSpoils("ICEW", 600, IT_ADVANCED);
    ModifyMonsterSpoils("BDRG", 2000, IT_MAGIC);
    ModifyMonsterSpoils("DRAG", 15000, IT_MAGIC);
    ModifyMonsterSpoils("GRYF", 4000, IT_ADVANCED);
    ModifyMonsterSpoils("OGRE", 400, IT_NORMAL);
    ModifyMonsterSpoils("ETTI", 1000, IT_NORMAL);
    ModifyMonsterSpoils("UNDE", 300, IT_ADVANCED);
    ModifyMonsterSpoils("LICH", 3300, IT_ADVANCED);
    ModifyMonsterSpoils("DEMO", 700, IT_ADVANCED);
    ModifyMonsterSpoils("BALR", 5000, IT_ADVANCED);
    ModifyMonsterSpoils("EAGL", 200, IT_ADVANCED);
    ModifyMonsterSpoils("PIRA", 60, IT_NORMAL);
    ModifyMonsterSpoils("KRAK", 3000, IT_ADVANCED);
    ModifyMonsterSpoils("MERF", 150, -1);
    ModifyMonsterSpoils("HYDR", 1600, IT_ADVANCED);

	EnableObject(O_ISLE);
	EnableObject(O_DERELICT);
	EnableObject(O_OCAVE);
	EnableItem(I_PIRATES);
	EnableItem(I_KRAKEN);
	EnableItem(I_MERFOLK);
	ModifyItemCapacities(I_PIRATES, 0, 0, 0, 15);
	ModifyObjectMonster(O_ISLE, I_TRENT);
	ModifyObjectMonster(O_OCAVE, I_CENTAUR);
	ModifyObjectMonster(O_DERELICT, I_CENTAUR);
    
	EnableObject(O_STABLE);
	EnableObject(O_FISHTRAP);
   	EnableObject(O_TEMPLE);
	EnableItem(I_LEATHERARMOR);
	DisableItem(I_WAGON);

	EnableItem(I_PPLATE);
    ModifyItemBasePrice(I_PEARL, 200);
    ModifyItemType(I_PEARL, IT_ADVANCED);
    ModifyItemProductionSkill(I_PEARL, "FISH", 4);
    ModifyItemProductionOutput(I_PEARL, 1, 1);
    
    DisableItem(I_TAROTCARDS);  // total of 19 trade items
    ModifyItemWeight(I_CASHMERE,2);
    //changed roses to orchids
    //changed velvet to goatcheese
    //changed order of items for island placement

	ModifyItemBasePrice(I_DOUBLEBOW,400);
//	ModifyItemBasePrice(I_WAGON,60);
	ModifyItemBasePrice(I_PLATEARMOR,200);
	ModifyItemBasePrice(I_ADPLATE,500);
	ModifyItemBasePrice(I_YEW,200);
	ModifyItemBasePrice(I_HEALPOTION,200);
	ModifyItemBasePrice(I_ROUGHGEM,100);
    ModifyItemType(I_ROUGHGEM, IT_ADVANCED);
	ModifyItemBasePrice(I_WHORSE,200);
	ModifyItemBasePrice(I_LEATHERARMOR,60);
	ModifyItemBasePrice(I_LIVESTOCK,20);
	ModifyItemBasePrice(I_GRAIN,20);
	ModifyItemBasePrice(I_FISH,20);

   	ModifyItemBasePrice(I_MCARPET,600);
  	ModifyItemBasePrice(I_SHIELDSTONE,1500);
  	ModifyItemBasePrice(I_AMULETOFTS,8000);
   	ModifyItemBasePrice(I_RUNESWORD,12000);
	ModifyItemBasePrice(I_RINGOFI,12000);
	ModifyItemBasePrice(I_CLOAKOFI,16000);
 	ModifyItemBasePrice(I_STAFFOFL,32000);
 	ModifyItemCapacities(I_MCARPET,30,30,30,0);
 	
 	EnableSkill(S_CONSTRUCTION);
 	DisableSkill(S_CARPENTER);
 	DisableSkill(S_SHIPBUILDING);
 	
	EnableObject(O_CORACLE); 	
 	ModifyObjectManpower(O_CORACLE,0,20,2,0);
 	ModifyObjectManpower(O_BARGE,0,6000,15,0);
 	ModifyObjectManpower(O_GALLEON,0,2800,15,0);
	ModifyObjectManpower(O_AGALLEON,200,2800,15,1);
	ModifyObjectManpower(O_TRIREME,0,1000,20,0);
	ModifyObjectConstruction(O_CORACLE, I_FUR, 3, "CONS", 1);
	ModifyObjectConstruction(O_LONGBOAT, I_WOOD, 20, "CONS", 1);
	ModifyObjectConstruction(O_TRIREME, I_WOOD, 50, "CONS", 3);
	ModifyObjectConstruction(O_CLIPPER, I_WOOD, 50, "CONS", 4);
	ModifyObjectConstruction(O_BARGE, I_WOOD, 80, "CONS", 2);
	ModifyObjectConstruction(O_GALLEON, I_WOOD, 80, "CONS", 3);
	ModifyObjectConstruction(O_AGALLEON, I_IRONWOOD, 80, "CONS", 5);
	ModifyObjectConstruction(O_BALLOON, I_FLOATER, 50, "CONS", 5);
//	ModifyItemProductionSkill(I_WAGON, "CONS", 1);

	EnableObject(O_MESSAGESTONE);
	
//  Weapon and Armour tables

    ModifyWeaponAttack("DBOW", PIERCING, ATTACK_RANGED, WeaponType::NUM_ATTACKS_SKILL);
    ModifyWeaponAttack("MSWO", ARMORPIERCING, ATTACK_COMBAT, 1);
    ModifyWeaponBonuses("XBOW", 2, 0, 0);
    ModifyWeaponBonuses("DBOW", 2, 0, 0);

	ModifyArmorFlags("LARM", ArmorType::DEFINASSASSINATE);
	ModifyArmorSaveAll("LARM", 6, 2, 0, 3);
	ModifyArmorSaveAll("CARM", 2, 1, 0, 0);
	ModifyArmorSaveAll("PARM", 8, 6, 2, 2);
	ModifyArmorSaveAll("PPAR", 8, 6, 2, 7);
	ModifyArmorSaveAll("MARM", 8, 7, 4, 5);
	ModifyArmorSaveAll("CLOA", 80, 79, 79, 79);	


	ClearTerrainItems(R_FOREST);	
	ModifyTerrainItems(R_FOREST, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_FOREST, 1, I_FUR, 100, 10);
	ModifyTerrainItems(R_FOREST, 2, I_IRONWOOD, 25, 5);
	ModifyTerrainItems(R_FOREST, 3, I_YEW, 25, 5);
	ModifyTerrainEconomy(R_FOREST, 500, 13, 20, 2);
	
	ClearTerrainItems(R_JUNGLE);	
	ModifyTerrainItems(R_JUNGLE, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_JUNGLE, 1, I_HERBS, 100, 10);
	ModifyTerrainItems(R_JUNGLE, 2, I_FUR, 100, 10);
	ModifyTerrainItems(R_JUNGLE, 3, I_YEW, 25, 5);
	ModifyTerrainItems(R_JUNGLE, 4, I_MUSHROOM, 10, 5);
	ModifyTerrainEconomy(R_JUNGLE, 200, 11, 20, 2);
	
	ClearTerrainItems(R_SWAMP);	
	ModifyTerrainItems(R_SWAMP, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_SWAMP, 1, I_HERBS, 100, 10);
	ModifyTerrainItems(R_SWAMP, 2, I_IRONWOOD, 25, 5);
	ModifyTerrainItems(R_SWAMP, 3, I_FLOATER, 33, 10);
	ModifyTerrainItems(R_SWAMP, 4, I_MUSHROOM, 10, 5);
	ModifyTerrainEconomy(R_SWAMP, 300, 12, 20, 2);
	
	ClearTerrainItems(R_DESERT);	
	ModifyTerrainItems(R_DESERT, 0, I_STONE, 100, 20);
	ModifyTerrainItems(R_DESERT, 1, I_IRON, 100, 10);
	ModifyTerrainItems(R_DESERT, 2, I_HORSE, 100, 10);
	ModifyTerrainItems(R_DESERT, 3, I_ROOTSTONE, 25, 10);
	ModifyTerrainItems(R_DESERT, 4, I_MITHRIL, 25, 5);	
	ModifyTerrainEconomy(R_DESERT, 200, 12, 10, 1);
	
	ClearTerrainItems(R_TUNDRA);	
	ModifyTerrainItems(R_TUNDRA, 0, I_HERBS, 100, 20);
	ModifyTerrainItems(R_TUNDRA, 1, I_FUR, 100, 20);
	ModifyTerrainItems(R_TUNDRA, 2, I_WOOD, 75, 5);
	ModifyTerrainItems(R_TUNDRA, 3, I_WHORSE, 25, 5);
	ModifyTerrainEconomy(R_TUNDRA, 400, 11, 10, 2);

	ClearTerrainItems(R_MOUNTAIN);	
	ModifyTerrainItems(R_MOUNTAIN, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_MOUNTAIN, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_MOUNTAIN, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_MOUNTAIN, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainEconomy(R_MOUNTAIN, 400, 13, 10, 2);

	ClearTerrainItems(R_PLAIN);	
	ModifyTerrainItems(R_PLAIN, 0, I_HORSE, 100, 20);
	ModifyTerrainItems(R_PLAIN, 1, I_WHORSE, 25, 5);
	ModifyTerrainEconomy(R_PLAIN, 800, 14, 40, 1);
	ModifyTerrainWMons(R_PLAIN, 2, I_LION, I_EAGLE, I_CENTAUR);

	ClearTerrainItems(R_CAVERN);	
	ModifyTerrainItems(R_CAVERN, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_CAVERN, 1, I_STONE, 100, 20);
	ModifyTerrainItems(R_CAVERN, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_CAVERN, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainEconomy(R_CAVERN, 100, 10, 10, 1);

	ClearTerrainItems(R_TUNNELS);	
	ModifyTerrainItems(R_TUNNELS, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_TUNNELS, 1, I_STONE, 100, 20);
	ModifyTerrainItems(R_TUNNELS, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_TUNNELS, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_TUNNELS, 4, I_MUSHROOM, 10, 5);
	ModifyTerrainEconomy(R_TUNNELS, 0, 0, 0, 1);

	ClearTerrainItems(R_UFOREST);
	//map generated without mushrooms and 30% IRWD chance
	ModifyTerrainItems(R_UFOREST, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_UFOREST, 1, I_STONE, 100, 20);
	ModifyTerrainItems(R_UFOREST, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_UFOREST, 3, I_MUSHROOM, 25, 5);
	ModifyTerrainItems(R_UFOREST, 4, I_IRONWOOD, 25, 7);
	ModifyTerrainItems(R_UFOREST, 5, I_YEW, 10, 5);
	ModifyTerrainEconomy(R_UFOREST, 100, 10, 10, 2);

	ClearTerrainItems(R_GROTTO);	
	ModifyTerrainItems(R_GROTTO, 0, I_STONE, 100, 10);
	ModifyTerrainItems(R_GROTTO, 1, I_ROOTSTONE, 25, 10);
	ModifyTerrainItems(R_GROTTO, 2, I_MITHRIL, 30, 6);
	ModifyTerrainItems(R_GROTTO, 3, I_ROUGHGEM, 35, 10);
	ModifyTerrainItems(R_GROTTO, 4, I_MUSHROOM, 15, 8);
	ModifyTerrainEconomy(R_GROTTO, 0, 11, 0, 1);            //0 economy for Nylandor only

	ClearTerrainItems(R_CHASM);
	ModifyTerrainItems(R_CHASM, 0, I_STONE, 100, 20);
	ModifyTerrainItems(R_CHASM, 1, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CHASM, 2, I_MITHRIL, 30, 6);
	ModifyTerrainItems(R_CHASM, 3, I_ROUGHGEM, 25, 10);
	ModifyTerrainItems(R_CHASM, 4, I_MUSHROOM, 20, 5);
	ModifyTerrainEconomy(R_CHASM, 0, 0, 0, 2);

	ClearTerrainItems(R_DFOREST);
	//map generated with rough gems instead of ironwood
	ModifyTerrainItems(R_DFOREST, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_DFOREST, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_DFOREST, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_DFOREST, 3, I_MUSHROOM, 40, 8);
	ModifyTerrainItems(R_DFOREST, 4, I_IRONWOOD, 30, 7);
	ModifyTerrainEconomy(R_DFOREST, 0, 11, 2, 3);            //0 economy for Nylandor only
	
	ClearTerrainItems(R_OCEAN);	
	ModifyTerrainItems(R_OCEAN, 0, I_FISH, 100, 40);
	ModifyTerrainItems(R_OCEAN, 1, I_DOLPHIN, 50, 10);
	ModifyTerrainItems(R_OCEAN, 1, I_PEARL, 15, 8);
	
	ClearTerrainItems(R_LAKE);	
	ModifyTerrainItems(R_LAKE, 0, I_FISH, 100, 60);
	ModifyTerrainItems(R_LAKE, 1, I_PEARL, 33, 10);

    
    /* End Standard Arcadia Changes */
   	
	if(Globals->HEXSIDE_TERRAIN) {
    	EnableHexside(H_ROAD);
    	EnableHexside(H_BRIDGE);
    	EnableHexside(H_BEACH);
    	EnableHexside(H_HARBOUR);
    	EnableHexside(H_ROCKS);
    	EnableHexside(H_RIVER);
    	EnableHexside(H_RAVINE);
    	EnableHexside(H_CLIFF);
	}

	if((Globals->UNDERDEEP_LEVELS > 0) || (Globals->UNDERWORLD_LEVELS > 1)) {
		EnableItem(I_MUSHROOM);
		EnableItem(I_HEALPOTION);
		EnableItem(I_ROUGHGEM);
		EnableItem(I_GEMS);
		EnableSkill(S_GEMCUTTING);
	}

	// Modify the various spells which are allowed to cross levels
	if(Globals->EASIER_UNDERWORLD) {
//		ModifyRangeFlags("rng_teleport", RangeType::RNG_CROSS_LEVELS);   //Not for Arcadia Labryinth
		ModifyRangeFlags("rng_portal", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_farsight", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_clearsky", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_weather", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_square", RangeType::RNG_CROSS_LEVELS);
	}

	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		EnableSkill(S_QUARTERMASTER);
		EnableObject(O_CARAVANSERAI);
	}

	return;
}
