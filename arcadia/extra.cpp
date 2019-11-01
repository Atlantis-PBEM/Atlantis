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
	int race = -1;
	Unit *temp2 = GetNewUnit( pFac );
	if(pFac->pStartLoc) pFac->ethnicity = pFac->pStartLoc->GetEthnicity();
	else pFac->ethnicity = getrandom(RA_OTHER);
	switch(pFac->ethnicity) {
	    case RA_HUMAN:
	    race = I_BARBARIAN;
	    break;
	    case RA_ELF:
	    race = I_DARKMAN;
	    break;
	    default:
	    race = I_HILLDWARF;
	    break;
    }
	temp2->SetMen( race, 1 );
	pFac->DiscoverItem(race, 0, 1);
	temp2->items.SetNum( I_STAFFOFY, 1 );
	pFac->DiscoverItem(I_STAFFOFY, 0, 1);
	temp2->reveal = REVEAL_FACTION;
	temp2->SetFlag(FLAG_COMMANDER,1);
   	temp2->type = U_MAGE;
   	
   	FindOrder *ord = new FindOrder;
	ord->find = 0;
	ord->quiet = 0;
	temp2->findorders.Add(ord);

   	//Xanaxor mod: see neighbours first turn!
	if(pFac->pStartLoc) {
   	    for(int i=0; i<6; i++) {
	        if(!pFac->pStartLoc->neighbors[i]) continue;
   	    	Farsight *f = new Farsight;
    		f->faction = pFac;
    		f->level = 1;
    		pFac->pStartLoc->neighbors[i]->farsees.Add(f);
  		}
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
	reg->race = race;
/*
    AString message = "Welcome to Xanaxor, the fourth game in the Arcadia series. This game is based on "
        "Atlantis 5.0, and the source code is available from the Atlantis CVS repository "
        "or by request from the GMs.";
    AString message2 = "Up to 12 factions may be present in Xanaxor simultaneously. "
        "4 factions will begin aligned with each of "
        "three major ethnicities - dwarf, human and elf. None begin representing the "
        "fourth alignment of orcs and merfolk. You may freely change your alignment "
        "through the game, and may work with or against other factions regardless of their "
        "alignment. However, the game will end when there are no surviving player factions aligned "
        "with three of the major ethnicities (the fourth ethnicity will be declared victorious "
        "across Xanaxor), or earlier by agreement of the surviving players.";
    AString message3 = "Your hero has begun their reign in their home village. As a bonus to help you "
        "along you have recieved details of all regions adjacent to your hero's "
        "home village. This bonus will not be repeated in future turns. Good Luck!";

    pFac->Message("");
    pFac->Message(message);
    pFac->Message("");
    pFac->Message(message2);
    pFac->Message("");
    pFac->Message(message3);
*/
    AString message1 = "Welcome to Xanaxor, the fourth game in the Arcadia series based on the Xanaxor code. This game is based on "
        "Atlantis 5.0, and the source code is available from the Atlantis CVS repository "
        "or by request from the GM. Many changes have been made both from Atlantis 5.0, and "
        "Arcadia 3 (Nylandor). If you are not aware of the changes, you should read about them "
        "as soon as possible.";
    AString message2 = "Up to 12 player factions may be present in the world of Xanaxor, as well as "
        "the seven non-player factions. "
        "4 player factions begin aligned with each of the three major ethnicities across the land - "
        "humans, dwarves and elves. Yet there is nothing to prevent leaders changing their allegiances "
        "between these three races, or even for some to lead the outcasts of Xanaxor, the orcs and merfolk. "
        "And nor is there anything preventing one race co-operating with another, if only for a short while. ";
    AString message3 = "Xanaxor will end when there are no surviving player factions supporting three of the "
        "four possible alignments - that is, all factions belong either to a particular alignment, or are "
        "in chaos following the death of their commanding hero. Xanaxor may also finish earlier "
        "by agreement of the surviving players.";
    AString message4 = "Your hero has begun their reign in their home village. As a bonus to help you "
        "along you have recieved details of all regions adjacent to your hero's "
        "home village. This bonus will not be repeated in future turns. Good Luck!";

    pFac->Message("");
    pFac->Message(message1);
    pFac->Message("");
    pFac->Message(message2);
    pFac->Message("");
    pFac->Message(message3);
    pFac->Message("");
    pFac->Message(message4);

	return( 1 );
}

Faction *Game::CheckVictory(AString *victoryline)
{
	int faction_present[RA_NA];
	for(int i=0; i<RA_NA; i++) faction_present[i] = 0;
	
	forlist(&factions) {
		Faction *f = (Faction *) elem;
		if(f->IsNPC()) continue;
		if(f->ethnicity < RA_NA) faction_present[f->ethnicity]++;
	}
	
	int race_present = -1;
	for(int i=0; i<RA_NA; i++) {
	    if(race_present != -1) {
	        if(faction_present[i]) return NULL;  //two ethnicities, no winner.
	    } else if(faction_present[i]) race_present = i;
	}
	
	if(race_present == -1) return GetFaction(&factions,monfaction); //there appear to be no player factions left :(
	
	forlist_reuse(&factions) {
		Faction *f = (Faction *) elem;
		if(f->IsNPC()) continue;
		*victoryline += EthnicityString(f->ethnicity) + " factions have taken over Xanaxor!";
		return f; //we have one, let's return it.
	}
	
	//huh? Oh well ...
	return NULL;
	
	
	/*
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
	*/
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
	    ModifyAttribMod("entertainment", 0, AttribModItem::SKILL,"GLAD", AttribModItem::UNIT_LEVEL, 10);    
	
	
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
	    ModifySkillDependancy(S_FORCE_SHIELD, 0, "FIRE", 4);
	    ModifySkillDependancy(S_FARSIGHT, 0, "WKEY", 1);
	    ModifySkillDependancy(S_FARSIGHT, 1, NULL, 0);
	    ModifySkillDependancy(S_SUMMON_WIND, 0, "WKEY", 1);
	    ModifySkillDependancy(S_SUMMON_STORM, 0, "SWIN", 1);
	    ModifySkillDependancy(S_SUMMON_TORNADO, 0, "SWIN", 4);
	    ModifySkillDependancy(S_CALL_LIGHTNING, 0, "SWIN", 6);
	    ModifySkillDependancy(S_CLEAR_SKIES, 0, "WKEY", 1);
	    ModifySkillRange(S_CLEAR_SKIES, "rng_linear");
//	    ModifySkillDependancy(S_BLIZZARD, 0, "WKEY", 4);
	    ModifySkillDependancy(S_FOG, 0, "WKEY", 3);
	    ModifySkillDependancy(S_FOG, 1, NULL, 0);
	    ModifySkillFlags(S_FOG, SkillType::MAGIC | SkillType::COMBAT | SkillType::CAST | SkillType::MAGEOTHER);
	    ModifyBaseSkills(S_BASE_WINDKEY, S_FIRE, S_FORCE_SHIELD, S_FARSIGHT,S_BLIZZARD, S_CLEAR_SKIES);
	    ModifyBaseSkills(S_BASE_WINDKEY, S_SUMMON_WIND, S_SUMMON_STORM, S_SUMMON_TORNADO,S_CALL_LIGHTNING, S_FOG);
	    
	    ModifySkillDependancy(S_GATE_LORE, 0, "PTTN", 1);
	    ModifySkillDependancy(S_GATE_LORE, 1, NULL, 0);
	    ModifySkillDependancy(S_TELEPORTATION, 0, "GATE", 3);
	    ModifySkillDependancy(S_TELEPORTATION, 1, NULL, 0);
	    ModifySkillDependancy(S_CONSTRUCT_GATE, 0, "GATE", 4);
	    ModifySkillDependancy(S_CONSTRUCT_GATE, 1, NULL, 0);
	    ModifySkillDependancy(S_CREATE_PORTAL, 0, "TELE", 3);
	    ModifySkillDependancy(S_EARTH_LORE, 0, "PTTN", 1);
	    ModifySkillDependancy(S_MAGICAL_HEALING, 0, "PTTN", 1);
	    ModifySkillDependancy(S_RESURRECTION, 0, "NECR", 3);
	    ModifySkillDependancy(S_ENERGY_SHIELD, 0, "PTTN", 1);
//	    ModifySkillDependancy(S_RESURRECTION, 1, "MHEA", 3);
//	    ModifySkillDependancy(S_SPIRIT_OF_DEAD, 0, "RESU", 3);
//	    ModifySkillDependancy(S_MODIFICATION, 0, "EART", 4);
//	    ModifySkillDependancy(S_DIVERSION, 0, "EART", 4);
	    ModifyBaseSkills(S_BASE_PATTERNING, S_GATE_LORE, S_TELEPORTATION, S_CONSTRUCT_GATE, S_CREATE_PORTAL);
	    ModifyBaseSkills(S_BASE_PATTERNING, S_EARTH_LORE, S_DIVERSION, S_MODIFICATION, S_ENERGY_SHIELD);
	    ModifyBaseSkills(S_BASE_PATTERNING, S_MAGICAL_HEALING, S_RESURRECTION, S_SPIRIT_OF_DEAD);
	    
	    ModifySkillDependancy(S_CONCEALMENT, 0, "MYST", 2);
	    ModifySkillDependancy(S_INVISIBILITY, 0, "CONC", 5);
	    ModifySkillDependancy(S_TRUE_SEEING, 0, "CONC", 4);
	    ModifySkillDependancy(S_TRANSMUTATION, 0, "MYST", 1);
	    ModifySkillDependancy(S_ENCHANT_SWORDS, 0, "TRAM", 2);
	    ModifySkillDependancy(S_ENCHANT_ARMOR, 0, "TRAM", 2);
	    ModifySkillDependancy(S_HYPNOSIS, 0, "MYST", 1);
	    ModifySkillDependancy(S_BINDING, 0, "HYPN", 3);
	    ModifySkillDependancy(S_DRAGON_TALK, 0, "BIND", 3);
	    ModifyBaseSkills(S_BASE_MYSTICISM, S_CONCEALMENT, S_INVISIBILITY, S_TRUE_SEEING);
	    ModifyBaseSkills(S_BASE_MYSTICISM, S_TRANSMUTATION, S_ENCHANT_SWORDS, S_ENCHANT_ARMOR);
	    ModifyBaseSkills(S_BASE_MYSTICISM, S_HYPNOSIS, S_BINDING, S_DRAGON_TALK);
	    
	    ModifySkillDependancy(S_SPIRIT_SHIELD, 0, "SUMM", 1);
	    ModifySkillDependancy(S_SPIRIT_SHIELD, 1, NULL, 0);
	    ModifySkillDependancy(S_NECROMANCY, 0, "SUMM", 2);
	    ModifySkillDependancy(S_NECROMANCY, 1, NULL, 0);
	    ModifySkillDependancy(S_RAISE_UNDEAD, 0, "NECR", 3);
	    ModifySkillDependancy(S_SUMMON_LICH, 0, "RAIS", 4);
	    ModifySkillDependancy(S_BANISH_UNDEAD, 0, "NECR", 1);
	    ModifySkillDependancy(S_LIGHT, 0, "SUMM", 1);
	    ModifySkillDependancy(S_DARKNESS, 0, "LIGT", 4);
	    ModifySkillDependancy(S_SUMMON_MEN, 0, "SUMM", 1);
        ModifySkillName(S_DEMON_LORE,"blankb","ZZZB");
        ModifySkillName(S_SUMMON_IMPS,"demon lore","DEMO");
        ModifyItemMagicSkill(I_IMP, "DEMO", 1);
        ModifyItemEscapeSkill(I_IMP, "DEMO", 320);
	    ModifySkillDependancy(S_SUMMON_IMPS, 0, "SUMM", 1);
	    ModifySkillFlags(S_SUMMON_IMPS, SkillType::MAGIC | SkillType::NOTIFY | SkillType::CAST | SkillType::COSTVARIES);
	    ModifySkillDependancy(S_SUMMON_DEMON, 0, "DEMO", 3);
	    ModifySkillDependancy(S_SUMMON_BALROG, 0, "SUDE", 4);
	    ModifySkillDependancy(S_BANISH_DEMONS, 0, "DEMO", 1);
	    ModifyBaseSkills(S_BASE_SUMMONING, S_SPIRIT_SHIELD, S_LIGHT, S_DARKNESS, S_SUMMON_MEN);
	    ModifyBaseSkills(S_BASE_SUMMONING, S_NECROMANCY, S_RAISE_UNDEAD, S_SUMMON_LICH, S_BANISH_UNDEAD);
	    ModifyBaseSkills(S_BASE_SUMMONING, S_SUMMON_IMPS, S_SUMMON_DEMON, S_SUMMON_BALROG, S_BANISH_DEMONS);
	    
	    ModifySkillFlags(S_BASE_ARTIFACTLORE, SkillType::MAGIC | SkillType::NOTIFY | SkillType::CAST | SkillType::FOUNDATION);
	    ModifyItemMagicSkill(I_SHIELDSTONE, "ARTL", 3);
	    ModifyItemMagicSkill(I_AMULETOFP, "ARTL", 1);
        ModifySkillName(S_ARTIFACT_LORE,"blankc","ZZZC");
	    ModifySkillDependancy(S_CREATE_RING_OF_INVISIBILITY, 0, "INVI", 4);
	    ModifySkillDependancy(S_CREATE_RING_OF_INVISIBILITY, 1, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_AMULET_OF_TRUE_SEEING, 0, "TRUE", 4);
	    ModifySkillDependancy(S_CREATE_AMULET_OF_TRUE_SEEING, 1, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_CLOAK_OF_INVULNERABILITY, 0, "FSHI", 4);
	    ModifySkillDependancy(S_CREATE_CLOAK_OF_INVULNERABILITY, 1, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_CLOAK_OF_INVULNERABILITY, 2, NULL, 0);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_FIRE, 0, "FIRE", 3);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_FIRE, 1, "ARTL", 1);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_LIGHTNING, 0, "CALL", 3);
	    ModifySkillDependancy(S_CREATE_STAFF_OF_LIGHTNING, 1, "ARTL", 5);
	    ModifySkillDependancy(S_CREATE_RUNESWORD, 0, "FEAR", 2);
	    ModifySkillDependancy(S_CREATE_RUNESWORD, 1, "ARTL", 4);
	    ModifySkillDependancy(S_CREATE_MAGIC_CARPET, 0, "ARTL", 1);
	    ModifySkillDependancy(S_CREATE_MAGIC_CARPET, 1, "SWIN", 3);
	    ModifySkillDependancy(S_ENGRAVE_RUNES_OF_WARDING, 0, "ARTL", 1);
	    ModifySkillDependancy(S_ENGRAVE_RUNES_OF_WARDING, 1, "ESHI", 1);
	    ModifySkillDependancy(S_ENGRAVE_RUNES_OF_WARDING, 2, NULL, 0);
	    ModifyBaseSkills(S_BASE_ARTIFACTLORE, S_CREATE_RING_OF_INVISIBILITY, S_CREATE_AMULET_OF_TRUE_SEEING, S_CREATE_CLOAK_OF_INVULNERABILITY);
	    ModifyBaseSkills(S_BASE_ARTIFACTLORE, S_CREATE_STAFF_OF_FIRE, S_CREATE_STAFF_OF_LIGHTNING, S_CREATE_RUNESWORD);
	    ModifyBaseSkills(S_BASE_ARTIFACTLORE, S_CREATE_MAGIC_CARPET, S_ENGRAVE_RUNES_OF_WARDING);
	    
	    ModifySkillDependancy(S_INNER_STRENGTH, 0, "BATT", 3);
	    ModifySkillDependancy(S_PHANTASMAL_ENTERTAINMENT, 0, "BATT", 1);
        ModifySkillName(S_PHANTASMAL_ENTERTAINMENT,"gladiator","GLAD");
	    ModifySkillDependancy(S_CREATE_AURA_OF_FEAR, 0, "FREN", 1);
	    ModifySkillDependancy(S_INSTILL_COURAGE, 0, "FREN", 5);
	    //swiftness (2)
	    //toughness (3)
	    //unity (2,1)
	    //frenzy (1)
	    ModifyBaseSkills(S_BASE_BATTLETRAINING, S_INNER_STRENGTH, S_PHANTASMAL_ENTERTAINMENT, S_CREATE_AURA_OF_FEAR, S_INSTILL_COURAGE);
	    ModifyBaseSkills(S_BASE_BATTLETRAINING, S_SWIFTNESS, S_TOUGHNESS, S_UNITY, S_FRENZY);
	    
	    ModifySkillDependancy(S_SECSIGHT, 0, "BATT", 1);
	    
	    
	    ModifySkillDependancy(S_WOLF_LORE, 0, "CHAR", 1);
	    ModifySkillDependancy(S_BIRD_LORE, 0, "CHAR", 1);
	    //gryffin lore (4,4)
	    //unity (1,2)
	    //trading (1)
	    //merchantry (3)
	    //quartermastery (4)
	    ModifyBaseSkills(S_BASE_CHARISMA, S_WOLF_LORE, S_BIRD_LORE, S_GRYFFIN_LORE, S_UNITY);
	    ModifyBaseSkills(S_BASE_CHARISMA, S_TRADING, S_MERCHANTRY, S_ARCADIA_QUARTERMASTERY);
	    

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
	    
	    EnableItem(I_STAFFOFY);

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
        ModifySpecialDamage("fear", 0, ATTACK_SPIRIT,2,60,  WeaponType::RANGED,MAGIC_SPIRIT, "fear");
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
        ModifySpecialDamage("storm", 0, ATTACK_WEATHER,2,300, WeaponType::RANGED,MAGIC_WEATHER,"storm");
        ModifyEffectFlags("storm", EffectType::EFF_ONESHOT);
        ModifySpecialDamage("tornado", 0, ATTACK_WEATHER,2,50, WeaponType::RANGED,MAGIC_WEATHER, 0);
        ModifySpecialDamage("black_wind", 0, ATTACK_SPIRIT,2,150, WeaponType::RANGED,MAGIC_SPIRIT,0);
        ModifySpecialDamage("banish_undead", 0, NUM_ATTACK_TYPES,2,60, WeaponType::RANGED|WeaponType::RESTINPEACE,MAGIC_ENERGY,0);
        ModifySpecialDamage("banish_demon", 0, NUM_ATTACK_TYPES,2,60, WeaponType::RANGED,MAGIC_SPIRIT,0);
        ModifySpecialDamage("icebreath", 0, ATTACK_WEATHER,2,5, WeaponType::RANGED,MAGIC_WEATHER,0);
        ModifySpecialDamage("light", 0, ATTACK_ENERGY,2,10, WeaponType::RANGED,MAGIC_ENERGY,0);
        ModifySpecialDamage("wounds", 0, ATTACK_SPIRIT,2,40,  WeaponType::RANGED,MAGIC_SPIRIT, "wounds");
        ModifySpecialDamage("courage", 0, NUM_ATTACK_TYPES,2,100,  WeaponType::RANGED,NUM_WEAPON_CLASSES, "courage");

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
	    ModifyRaceSkills("NOMA", "RIDI","RANC","HORS","BUIL");
	    ModifyRaceSkills("TMAN", "LUMB","CONS","FARM","OBSE");
	    
	    ModifyRaceSkills("WELF", "LUMB","LBOW","FARM","ENTE","BUIL");
	    ModifyRaceSkills("SELF", "FISH","SAIL","CONS","RANC");
	    ModifyRaceSkills("HELF", "HORS","ENTE","RIDI","HEAL");
	    ModifyRaceSkills("TELF", "HERB","HEAL","STEA","LBOW","HUNT");

	    ModifyRaceSkills("IDWA", "COMB","BUIL","HERB","FISH");
	    ModifyRaceSkills("HDWA", "COMB","MINI","WEAP","ARMO");
	    ModifyRaceSkills("UDWA", "XBOW","MINI","QUAR","STEA");
	    ModifyRaceSkills("DDWA", "XBOW","QUAR","ARMO","RANC");

	    ModifyRaceSkills("MERM", "STEA","FISH","ARMO","DOLP");
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
	    ModifyRaceSkillLevels("MERM",2,1);
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
	    ModifyItemProductionSkill(I_PPLATE, "ARMO", 4);
	    ModifyItemProductionSkill(I_WHORSE, "HORS", 6);
     	ModifyItemProductionSkill(I_MUSHROOM, "HERB", 4);
     	ModifyItemProductionSkill(I_HEALPOTION, "HEAL", 4);
     	ModifyMountBonuses("WING", 4, 8, 3);
     	ModifyItemType(I_MCARPET, IT_MAGIC|IT_MOUNT);
	    
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
    ModifyItemType(I_PEARL, IT_ADVANCED);
    ModifyItemProductionSkill(I_PEARL, "FISH", 4);
    ModifyItemProductionOutput(I_PEARL, 1, 1);
    
	EnableItem(I_MUSHROOM);
	EnableItem(I_HEALPOTION);
    //no rough gems, gems, or gemcutting
    
    DisableItem(I_TAROTCARDS);
    DisableItem(I_VELVET);
    DisableItem(I_VODKA);
    DisableItem(I_IVORY);
    DisableItem(I_MINK);
    DisableItem(I_SPICES);
    DisableItem(I_COTTON);  // total of 18 trade items
    ModifyItemWeight(I_CASHMERE,2);
    //changed roses to orchids
    //changed velvet to goatcheese
    //changed order of items for island placement
   	ModifyItemBasePrice(I_MCARPET,100);


//	ModifyItemBasePrice(I_WAGON,60);
	ModifyItemBasePrice(I_PLATEARMOR,200);
	ModifyItemBasePrice(I_DOUBLEBOW,320);
	ModifyItemBasePrice(I_MPLATE,320);
	ModifyItemBasePrice(I_PPLATE,320);
	ModifyItemBasePrice(I_MSWORD,320);
	ModifyItemBasePrice(I_YEW,200);
	ModifyItemBasePrice(I_MITHRIL,200);
    ModifyItemBasePrice(I_PEARL, 200);
	ModifyItemBasePrice(I_WHORSE,200);
	ModifyItemBasePrice(I_HEALPOTION,200);
//	ModifyItemBasePrice(I_ROUGHGEM,100);
//  ModifyItemType(I_ROUGHGEM, IT_ADVANCED);
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
	ModifyObjectManpower(O_TRIREME,0,2200,100,1);
	ModifyObjectManpower(O_ATRIREME,220,2200,120,1);
	ModifyObjectManpower(O_MERCHANT,0,1800,15,0);
 	ModifyObjectManpower(O_CORACLE,0,20,1,0);
 	ModifyObjectManpower(O_BARGE,0,6000,40,0);
 	ModifyObjectManpower(O_JUNK,0,1200,20,0);
	ModifyObjectManpower(O_LONGBOAT,0,300,10,0);
	ModifyObjectManpower(O_BALLOON,0,800,10,0);
	ModifyObjectManpower(O_TREASUREARK,0,10000,10,0);
	
	ModifyObjectConstruction(O_CORACLE, I_FUR, 2, "CONS", 1);
	ModifyObjectConstruction(O_LONGBOAT, I_WOOD, 20, "CONS", 1);
	ModifyObjectConstruction(O_MERCHANT, I_WOOD, 100, "CONS", 4);
	ModifyObjectConstruction(O_TRIREME, I_WOOD, 200, "CONS", 3);
	ModifyObjectConstruction(O_ATRIREME, I_IRONWOOD, 200, "CONS", 4);
	ModifyObjectConstruction(O_BARGE, I_WOOD, 100, "CONS", 2);
	ModifyObjectConstruction(O_JUNK, I_WOOD, 40, "CONS", 2);
	ModifyObjectConstruction(O_BALLOON, I_FLOATER, 50, "CONS", 5);
    ModifyObjectConstruction(O_TREASUREARK, I_WOOD, 150, "CONS", 3);
//	ModifyItemProductionSkill(I_WAGON, "CONS", 1);

    ModifyObjectDefence(O_TOWER, 2, 2, 2, 2, 2, 2);
    ModifyObjectDefence(O_FORT, 2, 3, 3, 3, 2, 3);
    ModifyObjectDefence(O_CASTLE, 3, 3, 3, 3, 3, 3);
    ModifyObjectDefence(O_CITADEL, 3, 4, 4, 4, 3, 4);
    ModifyObjectDefence(O_MFORTRESS, 4, 4, 4, 4, 3, 4);
    ModifyObjectDefence(O_ATRIREME, 2, 2, 2, 2, 2, 2);

	EnableObject(O_MESSAGESTONE);
	
//  Weapon and Armour tables

    ModifyWeaponAttack("DBOW", PIERCING, ATTACK_RANGED, WeaponType::NUM_ATTACKS_SKILL);
    ModifyWeaponAttack("MSWO", ARMORPIERCING, ATTACK_COMBAT, 1);
    ModifyWeaponAttack("RUNE", ARMORPIERCING, ATTACK_COMBAT, 1);
    ModifyWeaponBonuses("XBOW", 2, 0, 0);
    ModifyWeaponBonuses("DBOW", 2, 0, 0);
    ModifyWeaponBonuses("RUNE", 5, 5, 0);

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
	ModifyTerrainEconomy(R_JUNGLE, 300, 11, 20, 2);
	
	ClearTerrainItems(R_SWAMP);	
	ModifyTerrainItems(R_SWAMP, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_SWAMP, 1, I_HERBS, 100, 10);
	ModifyTerrainItems(R_SWAMP, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_SWAMP, 3, I_IRONWOOD, 25, 5);
	ModifyTerrainItems(R_SWAMP, 4, I_FLOATER, 33, 10);
	ModifyTerrainItems(R_SWAMP, 5, I_MUSHROOM, 25, 5);
	ModifyTerrainEconomy(R_SWAMP, 400, 12, 20, 2);
	
	ClearTerrainItems(R_DESERT);	
	ModifyTerrainItems(R_DESERT, 0, I_STONE, 100, 30);
	ModifyTerrainItems(R_DESERT, 1, I_IRON, 100, 10);
	ModifyTerrainItems(R_DESERT, 2, I_HORSE, 100, 10);
	ModifyTerrainItems(R_DESERT, 3, I_ROOTSTONE, 25, 10);
	ModifyTerrainItems(R_DESERT, 4, I_MITHRIL, 25, 5);	
	ModifyTerrainEconomy(R_DESERT, 200, 13, 10, 1);
	
	ClearTerrainItems(R_TUNDRA);	
	ModifyTerrainItems(R_TUNDRA, 0, I_HERBS, 100, 20);
	ModifyTerrainItems(R_TUNDRA, 1, I_FUR, 100, 20);
	ModifyTerrainItems(R_TUNDRA, 2, I_WOOD, 80, 5);
	ModifyTerrainItems(R_TUNDRA, 3, I_IRON, 80, 5);
	ModifyTerrainItems(R_TUNDRA, 4, I_WHORSE, 25, 5);
	ModifyTerrainEconomy(R_TUNDRA, 400, 11, 10, 2);

	ClearTerrainItems(R_MOUNTAIN);	
	ModifyTerrainItems(R_MOUNTAIN, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_MOUNTAIN, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_MOUNTAIN, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_MOUNTAIN, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainEconomy(R_MOUNTAIN, 600, 13, 10, 2);

	ClearTerrainItems(R_PLAIN);	
	ModifyTerrainItems(R_PLAIN, 0, I_HORSE, 100, 20);
	ModifyTerrainItems(R_PLAIN, 1, I_WHORSE, 25, 5);
	ModifyTerrainEconomy(R_PLAIN, 800, 14, 40, 1);
	ModifyTerrainWMons(R_PLAIN, 2, I_LION, I_EAGLE, I_CENTAUR);

	ClearTerrainItems(R_PARADISE);	
	ModifyTerrainItems(R_PARADISE, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_PARADISE, 1, I_IRON, 100, 20);
	ModifyTerrainItems(R_PARADISE, 2, I_HERBS, 100, 15);
	ModifyTerrainItems(R_PARADISE, 3, I_FUR, 100, 15);
	ModifyTerrainItems(R_PARADISE, 4, I_STONE, 100, 20);
	ModifyTerrainItems(R_PARADISE, 5, I_IRONWOOD, 100, 10);
	ModifyTerrainItems(R_PARADISE, 6, I_FLOATER, 100, 20);
	ModifyTerrainEconomy(R_PARADISE, 1000, 18, 40, 1);
/*
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
	*/
	ClearTerrainItems(R_OCEAN);	
	ModifyTerrainItems(R_OCEAN, 0, I_FISH, 100, 40);
	ModifyTerrainItems(R_OCEAN, 1, I_DOLPHIN, 50, 10);
	ModifyTerrainItems(R_OCEAN, 2, I_PEARL, 15, 8);
	ModifyTerrainItems(R_OCEAN, 3, I_FDOLPHIN, 10, 5);
	
	ClearTerrainItems(R_LAKE);	
	ModifyTerrainItems(R_LAKE, 0, I_FISH, 100, 60);
	ModifyTerrainItems(R_LAKE, 1, I_PEARL, 35, 10);

    
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
