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

#include <time.h>

#include "game.h"
#include "gamedata.h"
#include "fileio.h"

int Game::GenRules(const AString &rules, const AString &css,
		const AString &intro)
{
	Ainfile introf;
	Arules f;

	if(f.OpenByName(rules) == -1) {
		return 0;
	}

	if(introf.OpenByName(intro) == -1) {
		return 0;
	}

	f.PutStr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 "
			"Transitional//EN\">");
	f.Enclose(1, "HTML");
	f.Enclose(1, "HEAD");
	f.PutStr("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; "
			"charset=utf-8\">");
	f.PutStr(AString("<LINK TYPE=\"text/css\" REL=stylesheet HREF=\"")+
			css + "\">");
	f.TagText("TITLE", AString(Globals->RULESET_NAME) + " " +
			ATL_VER_STR(Globals->RULESET_VERSION) + " Rules");
	f.Enclose(0, "HEAD");
	f.Enclose(1, "BODY");
	f.Enclose(1, "CENTER");
	f.TagText("H1", AString("Rules for ") + Globals->RULESET_NAME + " " +
			ATL_VER_STR(Globals->RULESET_VERSION));
	f.TagText("H1", AString("Based on Atlantis v") +
			ATL_VER_STR(CURRENT_ATL_VER));
	f.TagText("H2", AString("Copyright 1996 by Geoff Dunbar"));
	f.TagText("H2", AString("Based on Russell Wallace's Draft Rules"));
	f.TagText("H2", AString("Copyright 1993 by Russell Wallace"));
	char buf[500];
	time_t tval = time(NULL);
	struct tm *ltval = localtime(&tval);
	strftime(buf, 500, "%B %d, %Y", ltval);
	f.TagText("H3", AString("Last Change: ")+buf);
	f.Enclose(0, "CENTER");
	f.ClassTagText("DIV", "rule", "");
	f.PutStr("Note: This document is subject to change, as errors are found");
	f.PutStr("and corrected, and rules sometimes change. Be sure you have");
	f.PutStr("the latest available copy.");
	f.PutStr("<BR>");
	f.PutStr("<BR>");
	f.PutStr(f.LinkRef("table_of_contents"));
	f.ClassTagText("DIV", "rule", "");
 	f.TagText("H2", "Table of Contents");
	f.PutStr(AString("Thanks to ") +
			f.Link("mailto:ken@satori.gso.uri.edu","Kenneth Casey"));
	f.PutStr("for putting together this table of contents.");
	f.PutStr("<BR>");
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#intro", "Introduction"));
	f.TagText("LI", f.Link("#playing", "Playing Atlantis"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#playing_factions", "Factions"));
	f.TagText("LI", f.Link("#playing_units", "Units"));
	f.TagText("LI", f.Link("#playing_turns", "Turns"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#world", "The World"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#world_regions", "Regions"));
	f.TagText("LI", f.Link("#world_structures", "Structures"));
	if(Globals->NEXUS_EXISTS)
		f.TagText("LI", f.Link("#world_nexus", "The Nexus"));
	if(Globals->CONQUEST_GAME)
		f.TagText("LI", f.Link("#world_conquest", "Conquest"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#movement", "Movement"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#movement_normal", "Normal Movement"));
	if(!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#movement_sailing", "Sailing"));
	f.TagText("LI", f.Link("#movement_order", "Order of Movement"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#skills", "Skills"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#skills_limitations", "Limitations"));
	f.TagText("LI", f.Link("#skills_studying", "Studying"));
	f.TagText("LI", f.Link("#skills_teaching", "Teaching"));
	f.TagText("LI", f.Link("#skills_skillreports", "Skill Reports"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#economy", "The Economy"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#economy_maintenance", "Maintenance Costs"));
	f.TagText("LI", f.Link("#economy_recruiting", "Recruiting"));
	f.TagText("LI", f.Link("#economy_items", "Items"));
	if(Globals->TOWNS_EXIST)
		f.TagText("LI", f.Link("#economy_towns", "Villages, Towns, Cities"));
	f.TagText("LI", f.Link("#economy_buildings",
				"Buildings and Trade Structures"));
	if(!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED))
		f.TagText("LI", f.Link("#economy_roads", "Roads"));
	if(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#economy_ships", "Ships"));
	f.TagText("LI", f.Link("#economy_advanceditems", "Advanced Items"));
	f.TagText("LI", f.Link("#economy_income", "Income"));
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#economy_entertainment", "Entertainment"));
	f.TagText("LI", f.Link("#economy_taxingpillaging", "Taxing/Pillaging"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#com", "Combat"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#com_attitudes", "Attitudes"));
	f.TagText("LI", f.Link("#com_attacking", "Attacking"));
	f.TagText("LI", f.Link("#com_muster", "The Muster"));
	f.TagText("LI", f.Link("#com_thebattle", "The Battle"));
	f.TagText("LI", f.Link("#com_victory", "Victory!"));
	f.Enclose(0, "UL");

	int has_stea = !(SkillDefs[S_STEALTH].flags & SkillType::DISABLED);
	int has_obse = !(SkillDefs[S_OBSERVATION].flags & SkillType::DISABLED);
	if(has_stea || has_obse) {
		f.TagText("LI", f.Link("#sealthobs",
					AString(has_stea ? "Stealth" : "") +
					((has_stea && has_obse) ? " and " : "") +
					(has_obse ? "Observation" : "")));
		if(has_stea) {
			f.Enclose(1, "UL");
			f.TagText("LI", f.Link("#stealthobs_stealing", "Stealing"));
			f.TagText("LI", f.Link("#stealthobs_assassination",
						"Assassination"));
			f.Enclose(0, "UL");
		}
	}
	f.TagText("LI", f.Link("#magic", "Magic"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#magic_skills", "Magic Skills"));
	f.TagText("LI", f.Link("#magic_foundations", "Foundations"));
	f.TagText("LI", f.Link("#magic_furtherstudy", "Further Magic Study"));
	f.TagText("LI", f.Link("#magic_usingmagic", "Using Magic"));
	f.TagText("LI", f.Link("#magic_incombat", "Mages In Combat"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#nonplayers", "Non-Player Units"));
	f.Enclose(1, "UL");
	if(Globals->TOWNS_EXIST && Globals->CITY_MONSTERS_EXIST) {
		f.TagText("LI", f.Link("#nonplayers_guards",
					"City and Town Guardsmen"));
	}
	if(Globals->WANDERING_MONSTERS_EXIST) {
		f.TagText("LI", f.Link("#nonplayers_monsters", "Wandering Monsters"));
	}
	f.TagText("LI", f.Link("#noplayers_controlled", "Controlled Monsters"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#orders", "Orders"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#orders_appreviations", "Abbreviations"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#ordersummary", "Order Summary"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#address", "address"));
	f.TagText("LI", f.Link("#advance", "advance"));
	if(has_stea)
		f.TagText("LI", f.Link("#assassinate", "assassinate"));
	f.TagText("LI", f.Link("#attack", "attack"));
	f.TagText("LI", f.Link("#autotax", "autotax"));
	f.TagText("LI", f.Link("#avoid", "avoid"));
	f.TagText("LI", f.Link("#behind", "behind"));
	f.TagText("LI", f.Link("#build", "build"));
	f.TagText("LI", f.Link("#buy", "buy"));
	f.TagText("LI", f.Link("#cast", "cast"));
	f.TagText("LI", f.Link("#claim", "claim"));
	f.TagText("LI", f.Link("#combat", "combat"));
	if (Globals->FOOD_ITEMS_EXIST)
		f.TagText("LI", f.Link("#consume", "consume"));
	f.TagText("LI", f.Link("#declare", "declare"));
	f.TagText("LI", f.Link("#describe", "describe"));
	f.TagText("LI", f.Link("#destroy", "destroy"));
	f.TagText("LI", f.Link("#enter", "enter"));
	f.TagText("LI", f.Link("#entertain", "entertain"));
	f.TagText("LI", f.Link("#faction", "faction"));
	f.TagText("LI", f.Link("#find", "find"));
	f.TagText("LI", f.Link("#forget", "forget"));
	f.TagText("LI", f.Link("#form", "form"));
	f.TagText("LI", f.Link("#give", "give"));
	f.TagText("LI", f.Link("#guard", "guard"));
	f.TagText("LI", f.Link("#hold", "hold"));
	f.TagText("LI", f.Link("#leave", "leave"));
	f.TagText("LI", f.Link("#move", "move"));
	f.TagText("LI", f.Link("#name", "name"));
	f.TagText("LI", f.Link("#noaid", "noaid"));
	if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE)
		f.TagText("LI", f.Link("#nocross", "nocross"));
	f.TagText("LI", f.Link("#nospoils", "nospoils"));
	f.TagText("LI", f.Link("#option", "option"));
	f.TagText("LI", f.Link("#password", "password"));
	f.TagText("LI", f.Link("#pillage", "pillage"));
	f.TagText("LI", f.Link("#produce", "produce"));
	f.TagText("LI", f.Link("#promote", "promote"));
	f.TagText("LI", f.Link("#quit", "quit"));
	f.TagText("LI", f.Link("#restart", "restart"));
	f.TagText("LI", f.Link("#reveal", "reveal"));
	if (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#sail", "sail"));
	f.TagText("LI", f.Link("#sell", "sell"));
	f.TagText("LI", f.Link("#show", "show"));
	if(has_stea)
		f.TagText("LI", f.Link("#steal", "steal"));
	f.TagText("LI", f.Link("#study", "study"));
	f.TagText("LI", f.Link("#tax", "tax"));
	f.TagText("LI", f.Link("#teach", "teach"));
	if (Globals->ALLOW_WITHDRAW)
		f.TagText("LI", f.Link("#withdraw", "withdraw"));
	f.TagText("LI", f.Link("#work", "work"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#sequenceofevents", "Sequence of Events"));
	f.TagText("LI", f.Link("#reportformat", "Report Format"));
	f.TagText("LI", f.Link("#hintsfornew", "Hints for New Players"));
	f.TagText("LI", f.Link("#specialcommands", "Special Commands"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#create", "#Create"));
	f.TagText("LI", f.Link("#resend", "#Resend"));
	f.TagText("LI", f.Link("#times", "#Times"));
	f.TagText("LI", f.Link("#rumors", "#Rumors"));
	f.TagText("LI", f.Link("#remind", "#Remind"));
	f.TagText("LI", f.Link("#email", "#Email"));
	f.Enclose(0, "UL");
	f.TagText("LI", f.Link("#credits", "Credits"));
	f.Enclose(0, "UL");
	f.PutStr("<BR>");
	f.PutStr("Index of Tables<BR>");
	f.Enclose(1, "UL");
	if (Globals->FACTION_LIMIT_TYPE==GameDefs::FACLIM_FACTION_TYPES)
		f.TagText("LI", f.Link("#tablefactionpoints",
					"Table of Faction Points"));
	f.TagText("LI", f.Link("#tableitemweights", "Table of Item Weights"));
	if(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#tableshipcapacities",
					"Table of Ship Capacities"));
	if(Globals->RACES_EXIST)
		f.TagText("LI", f.Link("#tableraces", "Table of Races"));
	f.TagText("LI", f.Link("#tableiteminfo", "Table of Item Information"));
	f.TagText("LI", f.Link("#tablebuildings", "Table of Buildings"));
	f.TagText("LI", f.Link("#tabletradestructures",
				"Table of Trade Structures"));
	if(!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED))
		f.TagText("LI", f.Link("#tableroadstructures",
					"Table of Road Structures"));
	if(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#tableshipinfo", "Table of Ship Information"));
	f.TagText("LI", f.Link("#tablebuildcapacity",
				"Table of Building Capacity"));
	f.Enclose(0, "UL");
	f.PutStr("<BR>");
	f.PutStr(f.LinkRef("intro"));
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Introduction");
	AString *in;
	while((in = introf.GetStr()) != NULL) {
		f.PutStr(*in);
		delete in;
	}
	f.PutStr(f.LinkRef("playing"));
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Playing Atlantis");
	f.PutStr("<BR><BR>");
	f.PutStr("Atlantis (as you undoubtedly already know) is a play by email "
			"game.  When you sign up for Atlantis, you will be sent a turn "
			"report (via email).  Your report completely details your "
			"position in the game.  After going over this report, and "
			"possibly communicating with other players in the game, you "
			"determine your course of action, and create a file of "
			"\"orders\", which you then send back to the Atlantis server. "
			"Then, at a regular interval (often one week), Atlantis collects "
			"all the orders, runs another turn (covering one month in game "
			"time), and sends all the players another report.");
	f.PutStr("<BR>");
	f.PutStr(f.LinkRef("playing_factions"));
	f.TagText("H3", "Factions:");
	f.PutStr("<BR><BR>");
	f.PutStr("A player's position is called a \"faction\".  Each faction has "
			"a name and a number (the number is assigned by the computer, "
			"and used for entering orders). Each player is allowed to play "
			"one and ONLY one faction at any given time. Each faction is "
			"composed of a number of \"units\", each unit being a group of "
			"one or more people loyal to the faction.  You start the game "
			"with a single unit consisting of one character, plus a sum of "
			"money.  More people can be hired during the course of the game, "
			"and formed into more units.  (In these rules, the word "
			"\"character\" generally refers either to a unit consisting of "
			"only one person, or to a person within a larger unit.)");
	f.PutStr("<BR><BR>");
	f.PutStr("A faction is considered destroyed, and the player knocked out "
			"of the game, if ever all its people are killed or disbanded "
			"(i.e. the faction has no units left).  The program does not "
			"consider your starting character to be special; if your "
			"starting character gets killed, you will probably have been "
			"thinking of that character as the leader of your faction, so "
			"some other character can be regarded as having taken the dead "
			"leader's place (assuming of course that you have at least one "
			"surviving unit!).  As far as the computer is concerned, as "
			"long as any unit of the faction survives, the faction is not "
			"wiped out.  (If your faction is wiped out, you can rejoin the "
			"game with a new starting character.)");
	f.PutStr("<BR><BR>");
	Faction fac;
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f.PutStr(AString("A faction has one pre-set limit; it may not ") +
				"contain more than " + AllowedMages(&fac) + " mages and " +
				AllowedApprentices(&fac) " apprentices.  Magic is a rare "
				"art, and only a few in the world can master it. Aside from "
				"that, there  is no limit to the number of units a faction "
				"may contain, nor to how many items can be produced or "
				"regions taxed.");
		f.PutStr("<BR><BR>");
	} else if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f.PutStr(AString("Each faction has a type; this is decided by the "
					"player, and determines what the faction may do.  The "
					"faction has ") + Globals->FACTION_POINTS + "Faction "
				"Points, which may be spent on any of the 3 Faction Areas, "
				"War, Trade, and Magic.  The faction type may be changed at "
				"the beginning of each turn, so a faction can change and "
				"adapt to the conditions around it.  Faction Points spent "
				"on War determine the number of regions in which factions "
				"can obtain income by taxing or pillaging.");
		f.PutStr("<BR><BR>");
		f.PutStr(AString("Faction Points spent on Trade determine the "
					"number of regions in which a faction may conduct "
					"trade activity. Trade activity includes producing "
					"goods, building ships and buildings, and buying and "
					"selling trade items. Faction Points spent on Magic "
					"determines the number of mages ") +
				(Globals->APPRENTICES_EXIST ? "and appentices " : "") +
				"the faction may have. (More information on all of "
				"the faction activities is in further sections of the "
				"rules).  Here is a chart detailing the limits on factions "
				"by Faction Points.");
		f.PutStr("<BR><BR>");
		f.PutStr(f.LinkRef("tablefactionpoints"));
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TH", "Faction Points");
		f.TagText("TH", "War (max tax regions)");
		f.TagText("TH", "Trade (max trade regions)");
		if(Globals->APPRENTICES_EXIST)
			f.TagText("TH", "Magic (max mages/apprentices)");
		else
			f.TagText("TH", "Magic (max mages)");
		f.Enclose(0, "TR");
		int i;
		for(i = 1; i <= Globals->FACTION_POINTS; i++) {
			fac.type[F_WAR]=i;
			fac.type[F_TRADE]=i;
			fac.type[F_MAGIC]=i;
			f.Enclose(1, "TR ALIGN=CENTER");
			f.TagText("TD", AString(i));
			f.TagText("TD", AString(AllowedTaxes(&fac)));
			f.TagText("TD", AString(AllowedTrades(&fac)));
			if(Globals->APPRENTICES_EXIST)
				f.TagText("TD", AString(AllowedMages(&fac)) + "/" +
						AllowedApprentices(&fac));
			else
				f.TagText("TD", AString(AllowedMages(&fac)));
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
		int m,w,t;
		fac.type[F_WAR] = w = (Globals->FACTION_POINTS+1)/3;
		fac.type[F_MAGIC] = m = Globals->FACTION_POINTS/3;
		fac.type[F_TRADE] = t = (Globals->FACTION_POINTS+2)/3;
		int nm, na, nw, nt;
		nm = AllowedMages(&fac);
		na = AllowedApprentices(&fac);
		nt = AllowedTrades(&fac);
		nw = AllowedTaxes(&fac);
		f.PutStr(AString("For example, a well rounded faction might ")
				"spend " + w + " point" + (w==1?"":"s") + " on War, " +
				t + " point" + (t==1?"":"s") + " on Trade, and " + m +
				" point" + (m==1?"":"s") + " on Magic.  This faction's "
				"type would appear as \"War " + w + " Trade " + t +
				" Magic " + m + "\", and would be able to tax " + nw +
				" region" + (nw==1?"":"s") + ", perform trade in " + nt +
				" region" + (nt==1?"":"s") + ", and have " + nm +
				" mage" + (nm==1?"":"s") +
				(Globals->APPRENTICES_EXIST ? AString(" as well as ") +
				 na + " apprentice" + (na==1?"":"s") : "") + ".");
		fac.type[F_WAR] = w = Globals->FACTION_POINTS;
		fac.type[F_MAGIC] = m = 0;
		fac.type[F_TRADE] = t = 0;
		nw = AllowedTaxes(&fac);
		nt = AllowedTrades(&fac);
		nm = AllowedMages(&fac);
		f.PutStr(AString("As another example, a specialized faction ") +
				"might spend all " + w + " point" + (w==1?"":"s") +
				" on War. This faction's type would appear as \"War " +
				w + "\", and it would be able to tax " + nw + " region" +
				(nw==1?"":"s") +
				(nt==0?", but could not perform trade in any regions,":
				 AString(", but could only perform trade in " + nt +
					 " region"+(nt==1?",":"s,"))) +
				(nm==0?" and could not possess any mages":
				 AString(" and could possess only " + nm + " mage" +
					 (nm==1?".":"s."))));
		f.PutStr("<BR><BR>");
		if (Globals->FACTION_POINTS>3) {
			int rem=Globals->FACTION_POINTS-3;
			f.PutStr(AString("Note that it is possible to have a faction ") +
					"type with less than " + Globals->FACTION_POINTS +
					"points spent. In fact, a starting faction has one "
					"point spent on each of War, Trade, and Magic, leaving " +
					rem + " point" + (rem==1?"":"s") + " unspent.");
			f.PutStr("<BR><BR>");
		}
	}
	f.PutStr(AString("When a faction starts the game, it is given a ")+
			"one-man unit and " + (Globals->START_MONEY -
				(Globals->LEADERS_EXIST ? Globals->LEADER_COST :
				 Globals->MAINTENANCE_COST)) + ItemDefs[I_SILVER].name +
			" unclaimed money.  Unclaimed money is cash that your whole " +
			"faction has access to, but cannot be taken away in battle (" +
			ItemDefs[I_SILVER].name + " in a unit's possessions can be " +
			"taken in battle.  This allows a faction to get started "
			"without presenting an enticing target for other factions. " +
			"Units in your faction may use the " +
			f.Link("#claim", "CLAIM") + " order to take this " +
			ItemDefs[I_SILVER].name + ", and use it to buy goods or " +
			"recruit men" +
			(Globals->ALLOW_WITHDRAW ?
			 AString(", or") + f.Link("#withdraw", "WITHDRAW") +
			 " goods  directly." : "."));
	f.PutStr("<BR><BR>");
	f.PutStr("An exampel faction is shown below, consisting of a "
			"starting character, Merlin the Magician, who has formed "
			"two more units, Merlin's Guards and Merlin's Workers.  "
			"Each unit is assigned a unit number by the computer "
			"(completely independent of the faction number); this is used "
			"for entering orders.  Here, the player has chosen to give "
			"his faction the same name (\"Merlin the Magician\") as his "
			"starting character.  Alternatively, you can call your "
			"faction something like \"The Great Northern Mining Company\" "
			"or whatever.");
	f.PutStr("<BR><BR>");
	f.Enclose(1, "<PRE>");
	f.PutNoFormat("  * Merlin the Magician (17), Merlin (27), leader");
	f.PutNoFormat("    [LEAD].  Skills: none.");
	f.PutNoFormat("  * Merlin's Guards (33), Merlin (27), 20 vikings");
	f.PutNoFormat("    [VIKI], 20 swords [SWOR]. Skills: none.");
	f.PutNoFormat("  * Merlin's Workers (34), Merlin (27), 50 vikings");
	f.PutNoFormat("    [VIKI].  Skills: none.");
	f.Enclose(0, "<PRE>");
	f.PutStr("<BR><BR>");
	f.PutStr(f.LinkRef("playing_units"));
	f.TagText("H3", "Units:");
	/* FOO */
 printf("\n");
 printf("A unit is a grouping together of people, all loyal to the same faction.\n");
 printf("The people in a unit share skills and possessions, and execute the\n");
 printf("same orders each month.\n");
 printf("The reason for having units of many people, rather than keeping track of\n");
 printf("individuals, is to simplify the game play.  The computer does not keep track of\n");
 printf("individual names, possessions, or skills for people in the same unit, and all\n");
 printf("the people in a particular unit must be in the same place at all times.  If you\n");
 printf("want to send people in the same unit to different places, you must split up the\n");
 printf("unit.  Apart from this, there is no difference between having one unit of 50\n");
 printf("people, or 50 units of one person each, except that the former is very much\n");
 printf("easier to handle. <p>\n");
 printf("\n");
 if (Globals->RACES_EXIST)
  {
   printf("  There are different races that make up the population of Atlantis. (See the\n");
   printf("  section on skills for a list of these.)\n");
  }
 else
  {
   printf("  Units are mostly made of ordinary people.\n");
  }
 if (Globals->LEADERS_EXIST)
  {
   printf("  %shere are \"leaders\"\n",Globals->RACES_EXIST?"In addition, t":"T");
  if (Globals->RACES_EXIST)
   {
    printf("   , who are presumed to be of one of the other races, but are\n");
    printf("   all the same in game terms\n");
   }
   printf("  .\n");
  }
 if (Globals->LEADERS_EXIST&&Globals->SKILL_LIMIT_NONLEADERS)
  {
   printf("  Units made up of normal people may only know one skill,\n");
   printf("  and cannot teach other units.  Units made up of leaders may know\n");
   printf("  as many skills as desired, and may teach other units to speed the learning\n");
   printf("  process.\n");
  }
 if (Globals->LEADERS_EXIST)
  {
   printf("  Leaders and normal people may not be mixed in the same unit.  \n");
   printf("  However, leaders are more expensive to recruit and maintain.  (More\n");
   printf("  information is in the section on skills.)\n");
  }
 if (Globals->RACES_EXIST)
  {
   printf("  A unit is treated as the least\n");
   printf("  common denominator of the people within it, so a unit made up of two races\n");
   printf("  with different strengths and weaknesses will have all the weaknesses, and none\n");
   printf("  of the strengths of either race. <p>\n");
  }
 printf("\n");
#if 0
 printf("<a name=\"playing_turns\">\n");
 printf("<h3> Turns: </h3>\n");
 printf("\n");
 printf("Each turn, the Atlantis server takes the orders file that you mailed to it, and\n");
 printf("assigns the orders to the respective units.  All units in your faction are\n");
 printf("completely loyal to you, and will execute the orders to the best of their\n");
 printf("ability.  If the unit does something unintended, it is generally because of\n");
 printf("incorrect orders; a unit will not purposefully betray you. <p>\n");
 printf("\n");
 printf("A turn is equal to one game month.  A unit can do many actions at the start of\n");
 printf("the month, that only take a matter of hours, such as buying and selling\n");
 printf("commodities, or fighting an opposing faction.  Each unit can also do exactly\n");
 printf("one action that takes up the entire month, such as harvesting timber or moving\n");
 printf("from one region to another.  The orders which take an entire month are \n");
 printf("<a href=\"#advance\"> ADVANCE</a>,\n");
 printf("<a href=\"#build\"> BUILD</a>,\n");
 if (SKILL_ENABLED(S_ENTERTAINMENT))
  {
   printf("  <a href=\"#entertain\"> ENTERTAIN</a>,\n");
  }
 printf("<a href=\"#move\"> MOVE</a>,\n");
 if (Globals->TAX_PILLAGE_MONTH_LONG)
  {
   printf("  <a href=\"#pillage\"> PILLAGE</a>,\n");
  }
 printf("<a href=\"#produce\"> PRODUCE</a>,\n");
 if (SKILL_ENABLED(S_SAILING))
  {
   printf("  <a href=\"#sail\"> SAIL</a>,\n");
  }
 printf("<a href=\"#study\"> STUDY</a>,\n");
 if (Globals->TAX_PILLAGE_MONTH_LONG)
  {
   printf("  <a href=\"#tax\"> TAX</a>,\n");
  }
 printf("<a href=\"#teach\"> TEACH</a> and\n");
 printf("<a href=\"#work\"> WORK</a>.\n");
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"world\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> The World </h2>\n");
 printf("\n");
 printf("The Atlantis world is divided for game purposes into hexagonal regions.  Each\n");
 printf("region has a name, and one of the following terrain types:  Ocean, Plain,\n");
 printf("Forest, Mountain, Swamp, Jungle, Desert, or Tundra.  (There may be other types\n");
 printf("of terrain to be discovered as the game progresses.)  Regions can contain\n");
 printf("units belonging to players; they can also contain structures such as buildings\n");
 printf("and ships.  Two units in the same region can normally interact, unless one of\n");
 printf("them is concealed in some way.  Two units in different regions cannot normally\n");
 printf("interact.  NOTE: Combat is an exception to this. <p>\n");
 printf("\n");
 printf("<a name=\"world_regions\">\n");
 printf("<h3> Regions: </h3>\n");
 printf("\n");
 printf("Here is a sample region, as it might appear on your turn report: <p>\n");
 printf("\n");
 printf("<pre>\n");
 printf("plain (172,110) in Turia, 500 peasants (nomads), $2500.\n");
 printf("------------------------------------------------------\n");
 if (Globals->WEATHER_EXISTS)
 {
   printf("  The weather was clear last month; it will be clear next month.\n");
 }
   printf("  Wages: $15 (Max: $%d).\n",500*15/Globals->WORK_FRACTION);
   printf("  Wanted: none.\n");
   printf("  For Sale: 50 nomads [NOMA] at $60, 10 leaders [LEAD] at $120.\n");
   printf("  Entertainment available: $%d.\n",500*(15-Globals->MAINTENANCE_COST)/Globals->ENTERTAIN_FRACTION);
   printf("  Products: 37 horses [HORS].\n");
 printf("\n");
 printf("Exits:\n");
   printf("  North : ocean (172,108) in Atlantis Ocean.\n");
   printf("  Northeast : ocean (173,109) in Atlantis Ocean.\n");
   printf("  Southeast : ocean (173,111) in Atlantis Ocean.\n");
   printf("  South : plain (172,112) in Turia.\n");
   printf("  Southwest : plain (171,111) in Turia.\n");
   printf("  Northwest : plain (171,109) in Turia.\n");
 printf("\n");
 printf("* Hans Shadowspawn (15), Merry Pranksters (14),\n");
   printf("  leader [LEAD], 500 %s [SILV]. Skills:\n",silver);
   printf("  none.\n");
 printf("- Vox Populi (13), leader [LEAD].\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("This report gives all of the available information on this region.  The region\n");
 printf("type is plain, the name of the surrounding area is Turia, and the coordinates\n");
 printf("of this region are (172,110).  The population of this region are 500 nomads, and\n");
 printf("there is $2500 of taxable income currently in this region.  Then, under the\n");
 printf("dashed line, are various details about items for sale, wages, etc.  Finally,\n");
 printf("there is a list of all visible units.  Units that belong to your faction will\n");
 printf("be so denoted by a '*', whereas other faction's units are preceded by a '-'.\n");
 printf("<p>\n");
 printf("\n");
 printf("Since Atlantis is made up of hexagonal regions, the coordinate system is not\n");
 printf("always exactly intuitive.  Here is the layout of Atlantis regions:\n");
 printf("<p>\n");
 printf("\n");
 printf("<pre>\n");
    printf("   ____        ____    \n");
   printf("  /    \\      /    \\   \n");
  printf(" /(0,0) \\____/(2,0) \\____/\n");
  printf(" \\      /    \\      /    \\     N\n");
   printf("  \\____/(1,1) \\____/(3,1) \\_   |\n");
   printf("  /    \\      /    \\      /    |\n");
  printf(" /(0,2) \\____/(2,2) \\____/     |\n");
  printf(" \\      /    \\      /    \\   W-O-E\n");
   printf("  \\____/(1,3) \\____/(3,3) \\_   |\n");
   printf("  /    \\      /    \\      /    S\n");
  printf(" /(0,4) \\____/(2,4) \\____/\n");
  printf(" \\      /    \\      /    \\\n");
   printf("  \\____/      \\____/\n");
   printf("  /    \\      /    \\\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("Note that the are \"holes\" in the coordinate system; there is no region (1,2),\n");
 printf("for instance.  This is due to the hexagonal system of regions. <p>\n");
 printf("\n");
 printf("Most regions are similar to the region shown above, but the are certain\n");
 printf("exceptions.  Oceans, not surprisingly, have no population.\n");
 if (Globals->TOWNS_EXIST)
  {
   printf("  Some regions will\n");
   printf("  contain villages, towns, and cities.  More information on these is available\n");
   printf("  in the section on the ecomony.\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"world_structures\">\n");
 printf("<h3> Structures: </h3>\n");
 printf("\n");
 printf("Regions may also contain structures, such as buildings or ships.  These will\n");
 printf("appear directly above the list of units.  Here is a sample structure: <p>\n");
 printf("\n");
 printf("<pre>\n");
 printf("+ Temple of Agrik [3] : Tower.\n");
   printf("  - High Priest Chafin (9), leader\n");
     printf("    [LEAD], sword [SWOR].\n");
   printf("  - Rowing Doom (188), 10 ice dwarves\n");
     printf("    [IDWA], 10 swords [SWOR].\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("The structure lists the name, the number, and what type of structure it is.\n");
 printf("(More information of the types of structures can be found in the section on the\n");
 printf("economy.)  Following this is a list of units inside the structure.\n");
 if (st_ena)
  {
   printf("  Units within a structure are always visible, even if they would otherwise not be\n");
   printf("  seen.\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("Units inside structures are still considered to be in the region, and other\n");
 printf("units can interact with them; however, they may gain benefits, such as\n");
 printf("defensive bonuses in combat from being inside a building.  The first unit to\n");
 printf("enter an object is considered to be the owner; only this unit can do things\n");
 printf("such as renaming the object, or permitting other units to enter.  The owner of\n");
 printf("an object can be identified on the turn report, as it is the first unit listed\n");
 printf("under the object.  Only units with men in them can be structure owners, so\n");
 printf("newly created units cannot own a structure until they contain men.<p>\n");
 printf("\n");
 printf("<a name=\"world_nexus\">\n");
 printf("<h3> Atlantis Nexus: </h3>\n");
 printf("\n");
 printf("Note: the following section contains some details that you may wish\n");
 printf("to skip over until you have had a chance to read the rest of the\n");
 printf("rules, and understand the mechanics of Atlantis.  However, be sure\n");
 printf("to read this section before playing, as it will affect your early\n");
 printf("plans in Atlantis. <p>\n");
 printf("\n");
 printf("When a faction first starts in Atlantis, it begins with one unit,\n");
 printf("in a special region called the Atlantis Nexus.\n");
 if (!Globals->NEXUS_IS_CITY)
  {
   printf("  This region exists\n");
   printf("  outside of the normal world of Atlantis, and as such has no products\n");
   printf("  or marketplaces; it merely serves as the magical entry into\n");
   printf("  Atlantis.\n");
  }
 else
  {
  /* TODO -- is that true ? */
   printf("  This region contains a starting city with all its benefits.\n");
   printf("  It also serves as the magical entry into Atlantis.\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("From the Atlantis Nexus, there are six exits into the starting\n");
 printf("cities of Atlantis.  Units may move through these exits as normal,\n");
 printf("but once through an exit, there is no return path to the Nexus.\n");
 if (Globals->GATES_EXIST&&Globals->NEXUS_GATE_OUT)
  {
   printf("  It is also possible to use Gate Lore to get out of Nexus (but not to\n");
   printf("  get back).\n");
  }
 printf("The six starting cities offer much to a starting faction;\n");
 if (!Globals->SAFE_START_CITIES)
  {
   printf("  until someone conquers the guardsmen,\n");
  }
 printf("there are unlimited amounts of many materials\n");
 printf("and men (though the prices are often quite high).\n");
 printf("In addition,\n");
 if (Globals->SAFE_START_CITIES)
  {
   printf("  no battles are allowed in starting cities.\n");
  }
 else
  {
   printf("  the\n");
   printf("  starting cities are guarded by strong guardsmen, keeping any units within\n");
   printf("  the city much safer from attack (See the section on Non-Player Units\n");
   printf("  for more information on city guardsmen).\n");
  }
 printf("As a drawback, these cities tend\n");
 printf("to be extremely crowded, and most factions will wish to leave the starting\n");
 printf("cities when possible.<p>\n");
 if (!Globals->SAFE_START_CITIES)
  {
   printf("  It is always possible to enter any starting city from the nexus, even if\n");
   printf("  that starting city has been taken over and guarded.  This is due to the\n");
   printf("  transportation from the Nexus to the starting city being magical in\n");
   printf("  nature.<p>\n");
  }
 printf("\n");
 {
 int masa=SKILL_ENABLED(S_SAILING)&&SKILL_ENABLED(S_SHIPBUILDING);
 int numet=1+!!Globals->GATES_EXIST+!!masa;
 int met=1;
 char *mets[]={"You must go ","The first is ","The second is"};
 if (numet==1) met=0;
 printf("There %s %d\n",numet==1?"is":"are",numet);
 printf("method%s of departing the starting cities.\n",numet==1?"":"s");
 printf("%s\n",mets[met++]);
 printf("by land, but keep in mind that the lands immediately surrounding\n");
 printf("the starting cities will tend to be highly populated, and possibly\n");
 printf("quite dangerous to travel.\n");
 if (masa)
  {
   printf("  %s by sea; all of the starting\n",mets[met++]);
   printf("  cities lie against an ocean, and a faction may easily purchase wood\n");
   printf("  and construct a ship to <a href=\"#sail\"> SAIL </a> away.  Be wary\n");
   printf("  of pirates seeking to prey on new factions, however!\n");
  }
 }
 if (Globals->GATES_EXIST)
  {
   printf("  And last, rumors of a magical Gate Lore suggest yet another way to travel\n");
   printf("  from the starting cities.  The rumors are vague, but factions wishing\n");
   printf("  to travel far from the starting cities, taking only a few men with\n");
   printf("  them, might wish to pursue this method.\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"movement\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Movement </h2>\n");
 printf("\n");
 if (SKILL_ENABLED(S_SAILING))
  {
   printf("  There are two main methods of movements in Atlantis.  The first is done using\n");
   printf("  the <a href=\"#move\"> MOVE </a>\n");
   printf("  order (or the <a href=\"#advance\"> ADVANCE </a>\n");
   printf("  order), and moves units individually from one\n");
   printf("  region to another.  The other method is done using the\n");
   printf("  <a href=\"#sail\"> SAIL </a> order, which can\n");
   printf("  sail a ship, including all of it's occupants, from one regions to another.\n");
  }
 else
  {
   printf("  The main method of movement in Atlantis is done using\n");
   printf("  the <a href=\"#move\"> MOVE </a>\n");
   printf("  order (or the <a href=\"#advance\"> ADVANCE </a>\n");
   printf("  order), and moves units individually from one\n");
   printf("  region to another.\n");
  }
 printf("Certain powerful mages may also teleport themselves, or even other\n");
 printf("units, but knowledge of the workings of this magic is carefully\n");
 printf("guarded.\n");
 printf("<p>\n");
 printf("\n");
 {
 int ws=Globals->FOOT_SPEED;
 int hs=Globals->HORSE_SPEED;
 int ss=Globals->SHIP_SPEED;
 int fs=Globals->FLY_SPEED;
 printf("\n");
 printf("<a name=\"movement_normal\">\n");
 printf("<h3> Normal Movement: </h3>\n");
 printf("\n");
 printf("In one month, a unit can issue a single\n");
 printf("<a href=\"#move\"> MOVE </a> order, using one or more of its\n");
 printf("movement points.  There are three modes of travel, walking, riding and flying. \n");
 printf("Walking units have %d movement point%s,\n",ws,ws==1?"":"s");
 printf("riding units have %d, and flying\n",hs);
 printf("units have %d.  A unit will automatically use the fastest mode of travel it\n",fs);
 printf("has available.  (The <a href=\"#advance\"> ADVANCE </a>\n");
 printf("order is the same as <a href=\"#move\"> MOVE</a>,\n");
 printf("except that it implies\n");
 printf("attacks on units which try to forbid access; see the section on combat for\n");
 printf("details.) <p>\n");
 printf("\n");
 printf("Flying units are not initially available to starting players.  A unit can ride\n");
 printf("provided that the carrying capacity of its horses is at least as great as the\n");
 printf("weight of its people and all other items.  A unit can walk provided that the\n");
 printf("carrying capacity of its people, horses and wagons is at least as great as the\n");
 printf("weight of all its other items, and provided that it has at least as many horses\n");
 printf("as wagons (otherwise the excess wagons count as weight, not capacity).\n");
 printf("Otherwise the unit cannot issue a\n");
 printf("<a href=\"#move\"> MOVE </a> order.  People weigh 10 units and have a\n");
 printf("capacity of 5 units; data for items is as follows: <p>\n");
 printf("\n");
 printf("<a name=\"tableitemweights\">\n");
 printf("<center>\n");
 printf("<table border>\n");
  printf(" <tr>\n");
   printf("  <td> </td>\n");
   printf("  <th>Weight</th>\n");
   printf("  <th>Capacity</th>\n");
  printf(" </tr>\n");
 int zoits[]={I_SILVER,I_GRAIN,I_LIVESTOCK,I_IRON,I_WOOD,I_STONE,I_FUR,I_FISH,I_HERBS,
               I_HORSE ,I_SWORD,I_CROSSBOW,I_LONGBOW,
               I_CHAINARMOR,I_PLATEARMOR,I_WAGON,I_PICK,
               I_SPEAR,I_AXE,I_HAMMER,I_NET,I_LASSO,I_BAG,I_SPINNING,
               I_LEATHERARMOR,I_CLOTHARMOR,I_BAXE};
 int nz=sizeof(zoits)/sizeof(int);
 for (int i=0;i<nz;i++)
  {
  if (ITEM_ENABLED(zoits[i]))
   {
   int cap=ItemDefs[zoits[i]].walk;
   int wg=ItemDefs[zoits[i]].weight;
    printf("   <tr>\n");
     printf("    <td align=center>%s</td>\n",ItemDefs[zoits[i]].name);
     printf("    <td align=center>%d</td>\n",wg);
     printf("    <td>\n");
   if (zoits[i]==I_WAGON)
    {
     printf("    200\n");
    } else
   if (cap)
    {
     printf("    %d\n",cap-wg);
    }
   else
    {
     printf("    &nbsp;\n");
    }
     printf("    </td>\n");
    printf("   </tr>\n");
   }
  }
 printf("</table> </center> <p>\n");
 printf("\n");
 if (Globals->FLIGHT_OVER_WATER!=GameDefs::WFLIGHT_NONE)
  {
   printf("  A unit which can fly is capable of travelling over water.\n");
  if (Globals->FLIGHT_OVER_WATER==GameDefs::WFLIGHT_MUST_LAND)
   {
    printf("   However, if the\n");
    printf("   unit ends it's turn over a water hex, that unit will drown.\n");
   }
 printf("<p>\n");
  }
 printf("\n");
 printf("Since regions are hexagonal, each region has six neighbouring regions to the\n");
 printf("north, northeast, southeast, south, southwest and northwest.  Moving from one\n");
 printf("region to another normally takes one movement point, except that the following\n");
 printf("terrain types take two movement points for riding or walking units to enter:\n");
 printf("Forest, Mountain, Swamp, Jungle and Tundra.\n");
 if (Globals->WEATHER_EXISTS)
  {
   printf("  Also, during certain seasons\n");
   printf("  (depending on the latitude of the region), all units (including flying ones)\n");
   printf("  have a harder time and travel will take twice as many movement points\n");
   printf("  as normal, as freezing weather makes travel difficult (in the tropics,\n");
   printf("  seasonal hurricane winds and torrential rains have a similar effect).\n");
  }
 printf("Units may not move through ocean regions\n");
 if (SKILL_ENABLED(S_SAILING))
  {
 printf("without using the <a href=\"#sail\">SAIL</a> order \n");
  }
 if (Globals->FLIGHT_OVER_WATER!=GameDefs::WFLIGHT_NONE)
  {
   printf("  unless they are capable of flight\n");
  if (Globals->FLIGHT_OVER_WATER==GameDefs::WFLIGHT_MUST_LAND)
   {
    printf("   , and even then, flying units must end\n");
    printf("   their movement on land or else drown\n");
   }
  }
 printf(". <p>\n");
 printf("\n");
 printf("Units may also enter or exit structures while moving.  Moving into or out of a\n");
 printf("structure does not use any movement points at all.  Note that a unit can also\n");
 printf("use the <a href=\"#enter\"> ENTER </a> and <a href=\"#leave\"> LEAVE </a>\n");
 printf("orders to move in and out of structures, without\n");
 printf("issuing a <a href=\"#move\"> MOVE </a> order. <p>\n");
 printf("\n");
 printf("Finally, certain structures contain interior passages to other regions.  The\n");
 printf("<a href=\"#move\"> MOVE </a> IN order can be used to go\n");
 printf("through these passages; the movement point cost\n");
 printf("is equal to the normal cost to enter the destination region. <p>\n");
 printf("\n");
 /* TODO expects 4 mp/riding */
 printf("\n");
 printf("Example: One man with a horse, sword, and chain mail wants to move north, then\n");
 printf("northeast.  The capacity of the horse is 20 and the weight of the man and other\n");
 printf("items is 12, so he can ride.  The month is April so he has four movement\n");
 printf("points.  He issues the order MOVE NORTH NORTHEAST.  First he moves north, into\n");
 printf("a plain region.  This uses one movement point.  Then he moves northeast, into a\n");
 printf("forest region.  This uses two movement points, so the movement is completed\n");
 printf("with one to spare. <p>\n");
 printf("\n");
 if (SKILL_ENABLED(S_SAILING))
  {
 printf("<a name=\"movement_sailing\">\n");
 printf("<h3> Sailing: </h3>\n");
 printf("\n");
 printf("Movement by sea is in some ways similar.  It does not use the\n");
 printf("<a href=\"#move\">MOVE</a> order\n");
 printf("however.  Instead, the owner of a ship must issue the\n");
 printf("<a href=\"#sail\">SAIL</a> order, and other\n");
 printf("units wishing to help sail the ship must also issue the\n");
 printf("<a href=\"#sail\">SAIL</a> order.  The ship\n");
 printf("will then, if possible, make the indicated movement, carrying all units on the\n");
 printf("ship with it.  Units on board the ship, but not aiding in the sailing of the\n");
 printf("ship, may execute other orders while the ship is sailing.  A unit which does\n");
 printf("not wish to travel with the ship should leave the ship in a coastal region,\n");
 printf("before the <a href=\"#sail\">SAIL</a>\n");
 printf("order is processed.  (A coastal region is defined as a non-\n");
 printf("ocean region with at least one adjacent ocean region.) <p>\n");
  printf(" \n");
 printf("Note that a unit on board a sailing ship may not <a href=\"#move\">MOVE</a>\n");
 printf("later in the turn, even if he doesn't issue the <a href=\"#sail\">SAIL</a>\n");
 printf("order; sailing is considered to take the whole month. Also, units\n");
 printf("may not remain on guard while on board a sailing ship; they will have\n");
 printf("to reissue the <a href=\"#guard\">GUARD 1</a> order to guard a region\n");
 printf("after sailing. <p>\n");
 printf("\n");
 printf("Ships get %d movement point%s per turn.  A ship can move from an ocean region\n",ss,ss==1?"":"s");
 printf("to another ocean region, or from a coastal region to an ocean region, or from\n");
 printf("an ocean region to a coastal region.  Ships can only be constructed in coastal\n");
 printf("regions.  For a ship to enter any region only costs one movement point; the\n");
 printf("cost of two movement points for entering, say, a forest coastal region, does\n");
 printf("not apply.\n");
 if (Globals->WEATHER_EXISTS)
  {
   printf("  Ships do, however, only get half movement points during the winter\n");
   printf("  months (or monsoon months in the tropical latitudes).\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("\n");
 printf("A ship can only move if the total weight of everything aboard does not exceed\n");
 printf("the ship's capacity.  (The rules do not prevent an overloaded ship from staying\n");
 printf("afloat, only from moving.)  Also, there must be enough sailors aboard (using\n");
 printf("the <a href=\"#sail\"> SAIL </a>\n");
 printf("order), to sail the ship, or it will not go anywhere.  Note that\n");
 printf("the sailing skill increases the usefulness of a unit proportionally; thus, a\n");
 printf("1 man unit with level 5 sailing skill can sail a longboat alone.  (See the\n");
 printf("section on skills for further details on skills.)  The capacities (and costs in\n");
 printf("labor units) of the various ship types are as follows: <p>\n");
 printf("\n");
 printf("<a name=\"tableshipcapacities\">\n");
 printf("<center>\n");
 printf("<table border>\n");
  printf(" <tr>\n");
   printf("  <td> </td>\n");
   printf("  <th>Capacity</th>\n");
   printf("  <th>Cost</th>\n");
   printf("  <th>Sailors</th>\n");
  printf(" </tr>\n");
 {
 int otypes[]={O_LONGBOAT,O_CLIPPER,O_GALLEON};
 int not=sizeof(otypes)/sizeof(int);
 for (int i=0;i<not;i++)
  {
  if (OBJECT_ENABLED(otypes[i]))
   {
    printf("   <tr>\n");
    printf("   <td align=center>%s</td>\n",ObjectDefs[otypes[i]].name);
    printf("   <td align=center>%d</td>\n",ObjectDefs[otypes[i]].capacity);
    printf("   <td align=center>%d</td>\n",ObjectDefs[otypes[i]].cost);
    printf("   <td align=center>%d</td>\n",ObjectDefs[otypes[i]].sailors);
    printf("   </tr>\n");
   }
  }
 }
 printf("</table> </center> <p>\n");
 } /* sailing */
 printf("\n");
 } /* movement */
 printf("\n");
 printf("<a name=\"movement_order\">\n");
 printf("<h3> Order of Movement: </h3>\n");
 printf("\n");
 printf("This section is probably unimportant to beginning players, but it can\n");
 printf("be helpful for more experienced players. <p>\n");
 printf("\n");
 printf("Normal movement in Atlantis, meaning <a href=\"#advance\"> ADVANCE </a>\n");
 printf("and <a href=\"#move\"> MOVE</a> orders,\n");
 printf("is processed one hex of movement at a time, region by region. So,\n");
 printf("Atlantis cycles through all of the regions; for each region, it finds\n");
 printf("any units that wish to move, and moves them (if they can move) one\n");
 printf("hex (and only one hex). After processing one such region, it initiates\n");
 printf("any battles that take place due to these movements, and then moves on\n");
 printf("to the next region. After it has gone through all of the regions, you\n");
 printf("will note that units have only moved one hex, so it goes back and does\n");
 printf("the whole process again, except this time moving units their second\n");
 printf("hex (if they have enough movement points left). This continues until\n");
 printf("no units can move anymore. <p>\n");
 printf("\n");
 printf("Sailing is handled differently; Atlantis cycles through all of the\n");
 printf("ships in Atlantis, moving them one at a time. When Atlantis sails\n");
 printf("a ship, it sails it through it's entire course, either to the end, or until\n");
 printf("the ship enters a hex guarded against some unit on the ship, and then moves\n");
 printf("onto the next ship. <p>\n");
 printf("\n");
 printf("Note that in either case, the order in which the regions are processed\n");
 printf("is undefined by the rules. The computer generally does them in the same\n");
 printf("order every time, but it is up to the wiles of the player to determine\n");
 printf("(or not) these patterns. The order in which units or ships are moved\n");
 printf("within a region is the order that they appear on a turn report. <p>\n");
 printf("\n");
 printf("<a name=\"skills\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Skills </h2>\n");
 printf("\n");
 printf("The most important thing distinguishing one character from another in Atlantis\n");
 printf("is skills.  The following skills are available:\n");
 {
 int skl[]={
      S_FARMING, S_RANCHING, S_MINING, S_LUMBERJACK, S_QUARRYING, S_HUNTING,
      S_FISHING, S_HERBLORE, S_HORSETRAINING, S_WEAPONSMITH, S_ARMORER,
      S_CARPENTER, S_BUILDING, S_SHIPBUILDING, S_ENTERTAINMENT, S_TACTICS,
      S_COMBAT, S_RIDING, S_CROSSBOW, S_LONGBOW, S_STEALTH, S_OBSERVATION,
      S_HEALING, S_SAILING
      };
 int nskl=sizeof(skl)/sizeof(int);
 int fst=1;
 for (int i=0;i<nskl;i++)
 if (SKILL_ENABLED(skl[i]))
  {
  if (!fst)
   {
    printf("   , \n");
   }
  fst=0;
   printf("  %s\n",SkillDefs[skl[i]].name);
  }
 }
 printf("When a unit possesses a skill, he also has a skill level to go with\n");
 printf("it.  Generally, the effectiveness of a skill is directly proportional to the\n");
 printf("skill level involved, so a level 2 horse trainer is twice as good as a level 1\n");
 printf("horse trainer. <p>\n");
 printf("\n");
 printf("<a name=\"skills_limitations\"\n");
 printf("<h3> Limitations: </h3>\n");
 printf("\n");
 if (Globals->LEADERS_EXIST&&Globals->SKILL_LIMIT_NONLEADERS)
  {
   printf("  A unit made up of leaders may know one or more skills; for the rest of this\n");
   printf("  section, the word \"leader\" will refer to such a unit.  Other units, those which\n");
   printf("  contain non-leaders, will be refered to as normal units.\n");
  }
 if (Globals->SKILL_LIMIT_NONLEADERS)
  {
   printf("  A normal unit may only know one skill. <p>\n");
  }
 printf("\n");
 if (Globals->RACES_EXIST)
  {
   printf("  Skills may be learned up to a maximum depending on the race of the studying\n");
   printf("  unit (remembering that for units containing more than one race,\n");
   printf("  the maximum is determined by the least common\n");
   printf("  denominator).  Every race has normal maximum skill level (usually 2),\n");
   printf("  but every race has a list of skills that they specialize in, and can learn up to\n");
   printf("  higher level (usually 3).\n");
   printf("  Leaders can learn every skill up to a maximum level of 5.  Here is a list of the races and\n");
   printf("  the skills they specialize in and appropriate levels: <p>\n");
 printf("\n");
   printf("  <a name=\"tableraces\">\n");
   printf("  <center>\n");
   printf("  <table border>\n");
    printf("   <tr>\n");
     printf("    <th>Race</th>\n");
     printf("    <th>Maximum level</th>\n");
     printf("    <th>Maximum level in specialized skills</th>\n");
     printf("    <th>Specialized Skills</th>\n");
    printf("   </tr>\n");
   {
   int rcs[]={I_VIKING,I_BARBARIAN,I_PLAINSMAN,I_ESKIMO,I_NOMAD,I_TRIBESMAN,
              I_DARKMAN,I_WOODELF,I_SEAELF,I_HIGHELF,I_TRIBALELF,I_ICEDWARF,
              I_HILLDWARF,I_UNDERDWARF,I_DESERTDWARF,I_ORC};
   int nrcs=sizeof(rcs)/sizeof(int);
   for (int i=0;i<nrcs;i++)
    {
    if (ITEM_ENABLED(rcs[i]))
     {
     ManType *w=ManDefs+ItemDefs[rcs[i]].index;
      printf("     <tr>\n");
      printf("     <td>%s</td>\n",ItemDefs[rcs[i]].names);
      printf("     <td>%d</td>\n",w->defaultlevel);
      printf("     <td>%d</td>\n",w->speciallevel);
      printf("     <td>\n");
     int fst=1;
     for (int j=0;j<4;j++)
      {
      if (w->skills[j]>=0&&SKILL_ENABLED(w->skills[j]))
       {
       if (!fst)
        {
         printf("        , \n");
        }
       fst=0;
        printf("       %s\n",SkillDefs[w->skills[j]].name);
       }
      }
      printf("     </td>\n");
      printf("     </tr>\n");
     }
    }
   }
   printf("  </table> </center> <p>\n");
 printf("\n");
  }
 printf("\n");
 printf("If units are merged together, their skills are averaged out.  No rounding off\n");
 printf("is done; rather, the computer keeps track for each unit of how many total\n");
 printf("months of training that unit has in each skill.  When units are split up, these\n");
 printf("months are divided as evenly as possible among the people in the unit; but no\n");
 printf("months are ever lost. <p>\n");
 printf("\n");
 printf("<a name=\"skills_studying\">\n");
 printf("<h3> Studying: </h3>\n");
 printf("\n");
 printf("For a unit to gain level 1 of a skill, they must gain one months worth of\n");
 printf("training in that skill.  To raise this skill level to 2, the unit must add an\n");
 printf("additional two months worth of training.  Then, to raise this to skill level 3\n");
 printf("requires another three months worth of training, and so forth.  A month of\n");
 printf("training is gained when a unit uses the <a href=\"#study\"> STUDY </a>\n");
 printf("order.  Note that study months\n");
 printf("do not need to be consecutive; for a unit to go from level 1 to level 2, he\n");
 printf("can study for a month, do something else for a month, and then go back and\n");
 printf("complete his second month of study. <p>\n");
 printf("\n");
 printf("Most skills cost $%d per person per month to study (in addition to normal\n",SkillDefs[S_COMBAT].cost);
 printf("maintenance costs).  The exceptions are\n");
 if (st_ena||ob_ena)
  {
  if (st_ena)
   {
    printf("   Stealth\n");
   }
  if (so_ena)
   {
    printf("   and\n");
   }
  if (ob_ena)
   {
    printf("   Observation\n");
   }
   printf("  (%swhich cost $%d),\n",so_ena?"both of ":"",SkillDefs[S_STEALTH].cost);
  }
 printf("Magic skills (which cost $%d)\n",SkillDefs[S_FORCE].cost);
 if (SKILL_ENABLED(S_TACTICS))
  {
   printf("  ,and Tactics (which costs $%d).\n",SkillDefs[S_TACTICS].cost);
  }
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"skills_teaching\">\n");
 printf("<h3> Teaching: </h3>\n");
 printf("\n");
 printf("A unit with a teacher can learn up to twice as fast as normal.  The\n");
 printf("<a href=\"#teach\"> TEACH </a>\n");
 printf("order is used to spend the month teaching one or more other units (your own or\n");
 printf("another faction's).  The unit doing the teaching must have a skill level\n");
 printf("greater than the units doing the studying.  (Note:  for all skill uses, it is\n");
 printf("skill level, not number of months of training, that counts.  Thus, a unit with\n");
 printf("1 month of training is effectively the same as a unit with 2 months of\n");
 printf("training, since both have a skill level of 1.)  The units being taught\n");
 printf("simply issue the <a href=\"#study\"> STUDY </a> order normally (also, his\n");
 printf("faction must be declared Friendly to the teaching faction).\n");
 printf("Each person can only teach up to %d student%s in a month; additional students\n",Globals->STUDENTS_PER_TEACHER,Globals->STUDENTS_PER_TEACHER==1?"":"s");
 printf("dilute the training.  Thus, if 1 teacher teaches %d men, each man\n",2*Globals->STUDENTS_PER_TEACHER);
 printf("being taught will gain 1 1/2 months of training, not 2 months. <p>\n");
 printf("\n");
 printf("Note that it is quite possible for a single unit to teach two or more other\n");
 printf("units different skills in the same month, provided that the teacher has a\n");
 printf("higher skill level than each student in the skill that that student is\n");
 printf("studying, and that there are no more than %d students per teacher. <p>\n",Globals->STUDENTS_PER_TEACHER);
 printf("\n");
 printf("Note: Only leaders may use the <a href=\"#teach\"> TEACH </a>\n");
 printf("order. <p>\n");
 printf("\n");
 printf("<a name=\"skills_skillreports\">\n");
 printf("<h3> Skill Reports: </h3>\n");
 printf("\n");
 printf("When a faction learns a new skill level for this first time, it may be given\n");
 printf("a report on special abilities that a unit with this skill level has.  This\n");
 printf("report can be shown again at any time (once a faction knows the skill), using\n");
 printf("the <a href=\"#show\"> SHOW </a>\n");
 printf("command.  For example, when a faction learned the skill Shoemaking\n");
 printf("level 3 for the first time, it might receive the following (obviously\n");
 printf("farsical) report: <p>\n");
 printf("\n");
 printf("<pre>\n");
   printf("  Shoemaking 3:  A unit possessing this skill may produce\n");
     printf("    Sooper Dooper Air Max Winged Sandals.  Use PRODUCE\n");
     printf("    Winged Sandals to produce this item.\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<a name=\"economy\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> The Economy </h2>\n");
 printf("\n");
 printf("The unit of currency in Atlantis is the %s piece.\n",silver);
 printf("Silver is a normal item, with zero weight, appearing in your\n");
 printf("unit's reports. Silver is used for such things as buying items,\n");
 printf("and unit's maintenance. <p>\n");
 printf("\n");
 printf("<a name=\"economy_maintenance\">\n");
 printf("<h3> Maintenance Costs: </h3>\n");
 printf("\n");
 printf("IMPORTANT:  Each and every character in Atlantis requires a maintenance fee\n");
 printf("each month. Anyone who ends the month without\n");
 printf("this maintenance cost has a %d%% chance of\n",Globals->STARVE_PERCENT);
 if (Globals->SKILL_STARVATION!=GameDefs::STARVE_NONE)
  {
   printf("  starving, leading to following effects:\n");
   printf("  <ul>\n");
    printf("   <li>\n");
   switch (Globals->SKILL_STARVATION)
    {
    case GameDefs::STARVE_ALL:
      printf("     A unit\n");
    break;
    case GameDefs::STARVE_MAGES:
      printf("     If unit is a mage, it\n");
    break;
    case GameDefs::STARVE_LEADERS:
      printf("     If unit is a leader, it\n");
     break;
    }
    printf("   will loose a skill level in some of its skills.\n");
   if (Globals->SKILL_STARVATION!=GameDefs::STARVE_ALL)
    {
     printf("    <li> Otherwise, it will starve to death.\n");
    }
    printf("   <li> If unit should forget a skill and it doesn't know any, it will\n");
         printf("        starve to death\n");
   if (Globals->SKILL_STARVATION!=GameDefs::STARVE_ALL)
    {
     printf("    too\n");
    }
    printf("   .\n");
   printf("  </ul>\n");
  }
 else
  {
   printf("  starving to death.\n");
  }
 printf("It is up to you to make sure that your people have enough available.\n");
 printf("Money will be shared automatically between your units in the same region, if\n");
 printf("one is starving and another has more than enough; but this will not happen\n");
 printf("between units in different regions (this sharing of money applies only for\n");
 printf("maintenance costs, and does not occur for other purposes). If you have\n");
 printf("%s in your unclaimed fund, then that %s will be automatically\n",silver,silver);
 printf("claimed by units that would otherwise starve. Lastly, if a faction is\n");
 printf("allied to yours, their units will provide surplus cash to your units for\n");
 printf("maintenance, as a last resort. <p>\n");
 printf("\n");
 printf("This fee is generally %d %s for a normal character\n",Globals->MAINTENANCE_COST,silver);
 if (Globals->LEADERS_EXIST)
  {
   printf("  , and %d %s for a leader.\n",Globals->LEADER_COST,silver);
  }
 if (Globals->FOOD_ITEMS_EXIST)
  {
   printf("  If this is not available, units may\n");
   printf("  substitute one unit of grain, livestock, or fish for this maintenance\n");
  if (Globals->LEADERS_EXIST)
   {
    printf("   (two units for a leader)\n");
   }
   printf("  . A unit may use the \n");
   printf("  <a href=\"#consume\">CONSUME</a> order to specify that it wishes to\n");
   printf("  use food items in preference to %s.\n",silver);
   printf("  Note that these items are worth more when sold in towns, so selling\n");
   printf("  them and using the money is more economical than using them for\n");
   printf("  maintenance.\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("\n");
 printf("<a name=\"economy_recruiting\">\n");
 printf("<h3> Recruiting: </h3>\n");
 printf("\n");
 printf("People may be recruited in a region.  The total amount of recruits available\n");
 printf("per month in a region, and the amount that must be paid per person recruited,\n");
 printf("are shown in the region description.  The\n");
 printf("<a href=\"#buy\"> BUY </a> order is used to recruit new people.  New\n");
 printf("recruits will not have any skills or items.  Note that the process of\n");
 printf("recruiting a new unit is somewhat counterintuitive; it is necessary to\n");
 printf("<a href=\"#form\"> FORM </a> an\n");
 printf("empty unit, <a href=\"#give\"> GIVE </a> the empty unit some money,\n");
 printf("and have it <a href=\"#buy\"> BUY </a> people; see the\n");
 printf("description of the <a href=\"#form\"> FORM </a> order for further details. <p>\n");
 printf("\n");
 printf("<a name=\"economy_items\">\n");
 printf("<h3> Items: </h3>\n");
 printf("\n");
 printf("A unit may have a number of possessions, referred to as \"items\".  Some details\n");
 printf("were given above in the section on Movement, but many things were left out.\n");
 printf("Here is a table giving some information about common items in Atlantis: <p>\n");
 printf("\n");
 {
 int rawrs=0;
 printf("\n");
 printf("<a name=\"tableiteminfo\">\n");
 printf("<center>\n");
 printf("<table border>\n");
  printf(" <tr>\n");
   printf("  <td> </td>\n");
   printf("  <th>Skill</th>\n");
   printf("  <th>Material</th>\n");
   printf("  <th>Weight (Capacity)</th>\n");
   printf("  <th>Extra information</th>\n");
  printf(" </tr>\n");
 {
 int its[]={I_SILVER,I_GRAIN,I_LIVESTOCK,I_IRON,I_WOOD,I_STONE,I_FUR,I_FISH,
            I_HERBS,I_HORSE,I_SWORD,I_CROSSBOW,I_LONGBOW,I_CHAINARMOR,
            I_PLATEARMOR,I_WAGON,I_PICK,I_SPEAR,I_AXE,I_HAMMER,I_NET,
            I_LASSO,I_BAG,I_SPINNING,I_LEATHERARMOR,I_CLOTHARMOR,I_BAXE};
 int nits=sizeof(its)/sizeof(int);
 char *remarks[NITEMS];
 for (int i=0;i<NITEMS;i++)
  remarks[i]=NULL;
  remarks[I_PICK]="+1 production for iron/stone.";
  remarks[I_SPEAR]="+1 production for furs.";
  remarks[I_AXE]="+1 production for wood.";
  remarks[I_HAMMER]="+1 production for iron weapons/armors.";
  remarks[I_NET]="+2 production for fish.";
  remarks[I_LASSO]="+1 production for livestock/horses.";
  remarks[I_BAG]="+2 production for grain/herbs.";
  remarks[I_SPINNING]="+2 production for nets/lassos.";
  remarks[I_BAXE]="Only attacks every other round.";
  remarks[I_CROSSBOW]="Only attacks every other round.";
  remarks[I_WAGON]="Gives 200 riding capacity when pulled by horse.";
 for (int i=0;i<nits;i++)
  {
  if (ITEM_ENABLED(its[i]))
   {
   ItemType *ai=ItemDefs+its[i];
    printf("   <tr>\n");
     printf("    <td align=center>%s</td>\n",ai->name);
    if (ai->pSkill>=0&&SKILL_ENABLED(ai->pSkill))
     {
      printf("     <td>%s\n",SkillDefs[ai->pSkill].name);
     if (ai->pLevel>1)
      {
       printf("      (%d)\n",ai->pLevel);
      }
     if (ai->pMonths>1)
      {
       printf("      , %d months\n",ai->pMonths);
      }
      printf("     </td>\n");
      printf("     <td>\n");
     int fob=1;
     for (int j=0;j<2;j++)
      {
      if (ai->pInput[j].item>=0&&ITEM_ENABLED(ai->pInput[j].item))
       {
       if (!fob)
        {
         printf("        , \n");
        }
       fob=0;
       if (ai->pInput[j].amt>1)
        {
         printf("        %d %s\n",ai->pInput[j].amt,ItemDefs[ai->pInput[j].item].names);
        }
       else
        {
         printf("        %s\n",ItemDefs[ai->pInput[j].item].name);
        }
       }
      }
     rawrs+=fob;
      printf("     &nbsp;</td>\n");
     }
    else
     {
      printf("     <td>&nbsp;</td>\n");
      printf("     <td>&nbsp;</td>\n");
     if (its[i]!=I_SILVER) rawrs++;
     }
     printf("    <td align=center>\n");
     printf("    %d\n",ai->weight);
    if (ai->walk)
     {
      printf("     (%d)\n",ai->walk-ai->weight);
     }
     printf("    </td>\n");
     printf("    <td>\n");
    if (ai->type&IT_WEAPON)
     {
     WeaponType *wp=WeaponDefs+ai->index;
      printf("     This is a %s weapon\n",wp->flags&WeaponType::RANGED?"ranged":"");
     if (wp->flags&WeaponType::NEEDSKILL)
      {
       printf("      (with %s skill)\n",SkillDefs[wp->baseSkill].name);
      }
      printf("     . It gives %+d to attack\n",wp->attackBonus);
     if (!(wp->flags&WeaponType::RANGED))
      {
       printf("      and %+d to defense\n",wp->defenseBonus);
      }
     if (wp->mountBonus)
      {
       printf("      (%+d if mounted)\n",wp->mountBonus);
      }
      printf("     .\n");
     }
    if (ai->type&IT_MOUNT)
     {
      printf("     It gives riding bonus with %s skill.\n",SkillDefs[MountDefs[ai->index].skill].name);
     }
    if (ai->type&IT_ARMOR)
     {
     ArmorType *at=ArmorDefs+ai->index;
      printf("     %d in %d chance to survive a hit.\n",at->saves[SLASHING],at->from);
     if ((at->flags&ArmorType::USEINASSASSINATE)&&st_ena) 
      {
       printf("      Even assassins may use it.\n");
      }
     }
    if (remarks[its[i]])
     {
      printf("     %s\n",remarks[its[i]]);
     }
     printf("    </td>\n");
     printf("    </tr>\n");
   }
  }
 }
 printf("</table> </center> <p>\n");
 printf("\n");
 printf("All items except %s are produced with the\n",silver);
 printf("<a href=\"#produce\"> PRODUCE </a> order.  Example: \n");
 printf("PRODUCE SWORDS will produce as many swords as possible during the month,\n");
 printf("provided that the unit has adequate supplies of iron and has the Weaponsmith\n");
 printf("skill.  Required skills and raw materials are in the table above. <p>\n");
 printf("\n");
 printf("If an item requires raw material, then the specified amount of material is consumed for\n");
 printf("each item produced.  Thus, to produce 5 longbows (a supply of arrows is assumed\n");
 printf("to be included with the bow), 5 units of wood are required.  The higher one's\n");
 printf("skill, the more productive each man-month of work; thus, 5 longbows could be\n");
 printf("produced by a 5-man unit of skill 1, or a 1-man unit of skill 5.\n");
 printf("Only Trade factions can issue <a href=\"#produce\"> PRODUCE </a> orders\n");
 printf("however, regardless of skill levels. <p>\n");
 printf("\n");
 printf("Items which increase production may increase production of advanced items\n");
 printf("in addition to the basic items listed.    Some of them also increase\n");
 printf("production of other tools.   Read the skill descriptions for details on\n");
 printf("which tools aid which production when not noted above.    As noted above,\n");
 printf("all combat capable tools add 1 to production while all non-combat capable\n");
 printf("tools add 2\n");
 if (ITEM_ENABLED(I_LASSO))
  {
   printf("  , with the exception of lasso's which only add 1\n");
  }
 printf(".<p>\n");
 printf("\n");
 printf("The first %d items on the list do not require raw material; they are produced\n",rawrs);
 printf("directly from the land.  Each region generally has at least one item that can\n");
 printf("be produced there.  Shown on the\n");
 printf("description of a region is a list of the items that can be produced, and the\n");
 printf("amount of each that can be produced per month.  This depends on the region\n");
 printf("type; thus, mountains are the best places to quarry stone, and herbs are most\n");
 printf("commonly found in forests and jungles.  It also varies from region to region of\n");
 printf("the same type.  If the units in a region attempt to\n");
 printf("produce more of a commodity than can be produced that month, then the\n");
 printf("amount available is distributed among the workers. <p>\n");
 printf("\n");
 }
 printf("\n");
 if (Globals->TOWNS_EXIST)
  {
   printf("  <a name=\"economy_towns\">\n");
   printf("  <h3> Village, Towns, and Cities: </h3>\n");
 printf("\n");
   printf("  Some regions in Atlantis contain villages, towns, and cities.  Villages add to\n");
   printf("  the wages, population, and tax income of the region they are in.  Also,\n");
   printf("  villages will have an additional market for grain, livestock, and fish.  As\n");
   printf("  the village's demand for these goods is met, the population will increase.\n");
   printf("  When the population reaches a certain theshold, the village will turn into a\n");
   printf("  town.  A town will have some additional products that it demands, in\n");
   printf("  addition to the\n");
   printf("  other common items.  Also, a town will sell some common items as well. A\n");
   printf("  town whose demands are being met will grow, and above another threshold it\n");
   printf("  will become a full-blown city.  A city will have additional markets for common\n");
   printf("  items, and will also have markets for less common, more expensive trade items.\n");
   printf("  <p>\n");
 printf("\n");
   printf("  Trade items are bought and sold only by cities, and have no other practical\n");
   printf("  uses.  However, the profit margins on these items are usually quite high.\n");
   printf("  Buying and selling of these items in a region counts against a Trade faction's\n");
   printf("  quota of regions in which it may undertake trade activity (note that buying\n");
   printf("  and selling normal items does not). <p>\n");
 printf("\n");
  }
  printf(" \n");
 printf("<a name=\"economy_buildings\">\n");
 printf("<h3> Buildings and Trade Structures: </h3>\n");
 printf("\n");
 printf("Construction of buildings and ships goes as follows: each unit of work on a\n");
 printf("building requires a unit of stone and a man-month of work by a character with\n");
 printf("Building skill at least 1; higher skill levels allow work to proceed faster\n");
 printf("(still using one unit of stone per unit of work done).\n");
 printf("Again, only Trade factions can issue\n");
 printf("<a href=\"#build\"> BUILD </a>\n");
 printf("orders.  Here is a table of the various building types: <p>\n");
 printf("\n");
 printf("<a name=\"tablebuildings\">\n");
 printf("<center>\n");
 printf("<table border>\n");
  printf(" <tr>\n");
   printf("  <td> </td>\n");
   printf("  <th>Size</th>\n");
   printf("  <th>Cost</th>\n");
   printf("  <th>Material</th>\n");
  printf(" </tr>\n");
 {
 int lbs[]={O_TOWER,O_FORT,O_CASTLE,O_CITADEL};
 int nlbs=sizeof(lbs)/sizeof(int);
 for (int i=0;i<nlbs;i++)
  {
  if (OBJECT_ENABLED(lbs[i]))
   {
   ObjectType *ot=ObjectDefs+lbs[i];
    printf("   <tr>\n");
     printf("    <td>%s</td>\n",ot->name);
     printf("    <td>%d</td>\n",ot->protect);
     printf("    <td>%d</td>\n",ot->cost);
     printf("    <td>%s</td>\n",ItemDefs[ot->item].name);
    printf("   </tr>\n");
   }
  }
 }
 printf("</table> </center> <p>\n");
 printf("\n");
 printf("Size is the number of people that the building can shelter. Cost is\n");
 printf("the number of person-months of labor and the number of units of material\n");
 printf("required to complete the building.\n");
 printf("<p>\n");
 printf("\n");
 printf("There are other structures that increase the maximum production of certain\n");
 printf("items in regions; for example, a Mine will increase the amount of iron that is\n");
 printf("available to be mined in a region.  To construct these structures\n");
 printf("requires a high skill level in the production skill related to the\n");
 printf("item that the structure will help produce.\n");
 if (OBJECT_ENABLED(O_INN))
  {
   printf("  (Inns are an exception\n");
   printf("  to this rule, requiring the Building skill, not the Entertainment\n");
   printf("  skill.)\n");
  }
 printf("This bonus in production is available to any unit in the\n");
 printf("region; there is no need to be inside the structure. <p>\n");
 printf("\n");
 printf("The first structure built in a region will increase\n");
 printf("the maximum production of the related product by 25%%; the amount added\n");
 printf("by each additional structure will be half of the the effect of the\n");
 printf("previous one.  (Note that if you build enough of the same type of\n");
 printf("structure in a region, the new structures may not add _any_ to the\n");
 printf("production level). <p>\n");
 printf("\n");
 printf("<a name=\"tabletradestructures\">\n");
 printf("<center>\n");
 printf("<table border> \n");
  printf(" <tr>\n");
   printf("  <td> </td>\n");
   printf("  <th>Cost</th>\n");
   printf("  <th>Material</th>\n");
   printf("  <th>Skill</th>\n");
   printf("  <th>Production Aided</th>\n");
  printf(" </tr>\n");
 {
 int lbs[]={O_MINE,O_FARM,O_RANCH,O_TIMBERYARD,O_INN,O_QUARRY,O_TEMPLE,
            O_MQUARRY,O_AMINE,O_PRESERVE,O_SACGROVE};
 int nlbs=sizeof(lbs)/sizeof(int);
 for (int i=0;i<nlbs;i++)
  {
  if (OBJECT_ENABLED(lbs[i]))
   {
   ObjectType *ot=ObjectDefs+lbs[i];
    printf("   <tr>\n");
     printf("    <td align=center>%s</td>\n",ot->name);
     printf("    <td align=center>%d</td>\n",ot->cost);
     printf("    <td align=center>%s</td>\n",ot->item==I_WOOD_OR_STONE?"wood or stone":ItemDefs[ot->item].name);
     printf("    <td align=center>%s %d</td>\n",SkillDefs[ot->skill].name,ot->level);
     printf("    <td align=center>%s</td>\n",ItemDefs[ot->productionAided].name);
    printf("   </tr>\n");
   }
  }
 }
 printf("</table> </center> <p>\n");
 printf("\n");
 printf("Note that structures will not increase the availability of an\n");
 printf("item in a region that does not already have the item available.\n");
 printf("Also, Trade structures do not offer defensive bonuses\n");
 printf("(which is why they do not have a\n");
 printf("size associated with them).  As for regular buildings, the Cost\n");
 printf("is the number of person-months of labor and also the number of\n");
 printf("units of raw material required to complete a trade structure.\n");
 printf("You can use two different materials (wood or stone) to construct most trade\n");
 printf("structures.<p>\n");
 printf("\n");
 if (OBJECT_ENABLED(O_ROADN))
  {
   printf("  <a name=\"economy_roads\"></a>\n");
   printf("  <h3> Roads: </h3>\n");
 printf("\n");
   printf("  There is a another type of structure called roads.  They require the building\n");
   printf("  skill and do not protect units, nor aid in the production of resources, but \n");
   printf("  do aid movement, and can improve the economy of a hex.<p>\n");
 printf("\n");
   printf("  Roads are directional and are only considered to reach from one hexside to\n");
   printf("  the center of the hex.  To gain a movement bonus, there must be two\n");
   printf("  connecting roads, one in each adjacent hex.  Only one road may be built in\n");
   printf("  each direction.  If a road in the given direction is connected, units\n");
   printf("  move along that road at half cost to a minimum of 1 movement point.<p>\n");
 printf("\n");
   printf("  For example: If a unit is moving northwest, then hex it is in must have a\n");
   printf("  northwest road, and the hex it is moving into must have a southeast road.<p>\n");
 printf("\n");
   printf("  To gain an economy bonus, a hex must have roads that connect to roads in\n");
   printf("  two adjoining hexes.  The economy bonus for the connected roads raises\n");
   printf("  the wages in the region by 1 point.<p>\n");
 printf("\n");
   printf("  There are six different road structures, one for each direction. They each\n");
   printf("  require %d %s to build.<p>\n",ObjectDefs[O_ROADN].cost,ItemDefs[ObjectDefs[O_ROADN].item].name);
 printf("\n");
  if (Globals->DECAY&&!(ObjectDefs[O_ROADN].flags&ObjectType::NEVERDECAY))
   {
    printf("   Unlike other structures, roads will decay over time if they are not\n");
    printf("   maintained. Difficult terrain and bad weather will speed this decay.\n");
    printf("   Maintnenance involves having units with the appropriate level of building\n");
    printf("   skill expend a small amount of stone and labor on a fairly regular basis in\n");
    printf("   the exactly same manner as they would finish building it if it was not\n");
    printf("   completed. In other words, enter the structure and issue the BUILD command\n");
    printf("   with no parameters.<p>\n");
 printf("\n");
    printf("   Once a road decays, it will give no bonuses until it is repaired. <p>\n");
   }
 printf("\n");
   printf("  <a name=\"tableroadstructures\">\n");
   printf("  <center>\n");
   printf("  <table border> \n");
    printf("   <tr>\n");
     printf("    <td> </td>\n");
     printf("    <th>Cost</th>\n");
     printf("    <th>Material</th>\n");
     printf("    <th>Skill</th>\n");
    printf("   </tr>\n");
   {
   int lbs[]={O_ROADN,O_ROADNW,O_ROADNE,O_ROADS,O_ROADSW,O_ROADSE};
   int nlbs=sizeof(lbs)/sizeof(int);
   for (int i=0;i<nlbs;i++)
    {
    if (OBJECT_ENABLED(lbs[i]))
     {
     ObjectType *ot=ObjectDefs+lbs[i];
      printf("     <tr>\n");
       printf("      <td align=center>%s</td>\n",ot->name);
       printf("      <td align=center>%d</td>\n",ot->cost);
       printf("      <td align=center>%s</td>\n",ot->item==I_WOOD_OR_STONE?"wood or stone":ItemDefs[ot->item].name);
       printf("      <td align=center>%s %d</td>\n",SkillDefs[ot->skill].name,ot->level);
      printf("     </tr>\n");
     }
    }
   }
   printf("  </table> </center> <p>\n");
 printf("\n");
  }
 printf("\n");
 if (SKILL_ENABLED(S_SHIPBUILDING))
  {
   printf("  <a name=\"economy_ships\">\n");
   printf("  <h3> Ships: </h3>\n");
 printf("\n");
   printf("  Ships are constructed similarly to buildings, except that they\n");
   printf("  are constructed of wood, not stone; and their construction requires\n");
   printf("  the Shipbuilding skill, not the Building skill. Only factions with\n");
   printf("  at least one faction point spent on trade can issue\n");
   printf("  <a href=\"#build\"> BUILD </a> orders. Here is a table on the various\n");
   printf("  ship types: <p>\n");
 printf("\n");
   printf("  <a name=\"tableshipinfo\">\n");
   printf("  <center>\n");
   printf("  <table border>\n");
    printf("   <tr>\n");
     printf("    <td> </td>\n");
     printf("    <th>Capacity</th>\n");
     printf("    <th>Cost</th>\n");
     printf("    <th>Material</th>\n");
     printf("    <th>Sailors</th>\n");
    printf("   </tr>\n");
   {
   int lbs[]={O_LONGBOAT,O_CLIPPER,O_GALLEON};
   int nlbs=sizeof(lbs)/sizeof(int);
   for (int i=0;i<nlbs;i++)
    {
    if (OBJECT_ENABLED(lbs[i]))
     {
     ObjectType *ot=ObjectDefs+lbs[i];
      printf("     <tr>\n");
       printf("      <td align=center>%s</td>\n",ot->name);
       printf("      <td align=center>%d</td>\n",ot->capacity);
       printf("      <td align=center>%d</td>\n",ot->cost);
       printf("      <td align=center>%s</td>\n",ot->item==I_WOOD_OR_STONE?"wood or stone":ItemDefs[ot->item].name);
       printf("      <td align=center>%d</td>\n",ot->sailors);
      printf("     </tr>\n");
     }
    }
   }
 printf("</table> </center> <p>\n");
 printf("\n");
   printf("  The capacity of a ship is the maximum weight that the ship may have\n");
   printf("  aboard and still move. The cost is both the person-months of labor\n");
   printf("  and the number of units of wood required to complete the ship. The\n");
   printf("  sailors are the number of skill levels of the Sailing skill that\n");
   printf("  must be aboard the ship (and issuing the\n");
   printf("  <a href=\"#sail\"> SAIL </a> order in order for the ship to sail). <p>\n");
  }
 printf("\n");
 printf("<a name=\"economy_advanceditems\">\n");
 printf("<h3> Advanced Items: </h3>\n");
 printf("\n");
 printf("There are also certain advanced items that highly skilled units can produce.\n");
 printf("These are not available to starting players, but can be discovered through\n");
 printf("study.  When a unit is skilled enough to produce one of these items, he will\n");
 printf("generally receive a skill report describing the production of this item.\n");
 printf("Production of advanced items is generally done in a manner similar to\n");
 printf("the normal items. <p>\n");
 printf("\n");
 printf("<a name=\"economy_income\">\n");
 printf("<h3> Income: </h3>\n");
 printf("\n");
 printf("Units can earn money with the <a href=\"#work\"> WORK </a>\n");
 printf("order.  This means that the unit spends the\n");
 printf("month performing manual work for wages.  The amount to be earned from this is\n");
 printf("usually not very high, so it is generally a last resort to be used if one is\n");
 printf("running out of money.  The current wages are shown in the region description\n");
 printf("for each region.  All units may <a href=\"#work\"> WORK</a>,\n");
 printf("regardless of skills or faction type.\n");
 printf("<p>\n");
 printf("\n");
 if (SKILL_ENABLED(S_ENTERTAINMENT))
  {
 printf("\n");
   printf("  <a name=\"economy_entertainment\">\n");
   printf("  <h3> Entertainment: </h3>\n");
 printf("\n");
   printf("  Units with the Entertainment skill can use it to earn money.  A unit with\n");
   printf("  Entertainment level 1 will earn %d %s per man by issuing the \n",Globals->ENTERTAIN_INCOME,silver);
   printf("  <a href=\"#entertain\"> ENTERTAIN </a>\n");
   printf("  order.  The total amount of money that can be earned this way is shown in the\n");
   printf("  region descriptions.  Higher levels of Entertainment skill can earn more, so\n");
   printf("  a character with Entertainment skill 2 can earn twice as much money as one with\n");
   printf("  skill 1 (and uses twice as much of the demand for entertainment in the region).\n");
   printf("  Note that entertainment income is much less, per region, than the income\n");
   printf("  available through working or taxing.  All factions may have entertainers,\n");
   printf("  regardless of faction type. <p>\n");
  }
 printf("\n");
 printf("<a name=\"economy_taxingpillaging\">\n");
 printf("<h3> Taxing/Pillaging: </h3>\n");
 printf("\n");
 printf("War factions may collect taxes in a region.  This is done using the\n");
 printf("<a href=\"#tax\"> TAX </a> order\n");
 printf("(which is %s a full month order).  The amount of tax money that can be\n",Globals->TAX_PILLAGE_MONTH_LONG?"":"not");
 printf("collected each month in a region is shown in the region description.  Only\n");
 printf("combat ready units may <a href=\"#tax\"> TAX</a>;\n");
 printf("a unit is combat ready if it either: has Combat skill of at least 1,\n");
 printf("has Longbow or Crossbow skill of at least 1 and also has the appropriate bow\n");
 printf("in his possession, or has a weapon (such as a sword) which requires no skill\n");
 printf("in his possession. Each taxing character can collect $%d, though if the number\n",Globals->TAX_INCOME);
 printf("of taxers would tax more than the available tax income, the tax income is\n");
 printf("split evenly.<p>\n");
 printf("\n");
 printf("War factions may also pillage a region.  To do this requires the faction to\n");
 printf("have enough combat trained men in the region to tax half of the available\n");
 printf("money in the region.  The total amount of money that can be pillaged will then\n");
 printf("be shared out between every combat trained unit that issues the\n");
 printf("<a href=\"#pillage\"> PILLAGE </a> order.\n");
 printf("The amount of money collected is equal to twice the available tax money.\n");
 printf("However, the economy of the region will be seriously damaged by pillaging, and\n");
 printf("will only slowly recover over time.  Note that\n");
 printf("<a href=\"#pillage\"> PILLAGE </a> comes before\n");
 printf("<a href=\"#tax\"> TAX</a>, so\n");
 printf("<a href=\"#tax\"> TAX </a> will collect no money in that region that month . <p>\n");
 printf("\n");
 printf("It is possible to safeguard one's tax income in regions one controls.  Units\n");
 printf("which have the Guard flag set (using the\n");
 printf("<a href=\"#guard\"> GUARD </a> order) will block\n");
 printf("<a href=\"#tax\"> TAX </a> orders\n");
 printf("issued by other factions in the same region, unless you have declared the\n");
 printf("faction in question Friendly.  Units on guard will also block\n");
 printf("<a href=\"#pillage\"> PILLAGE </a> orders\n");
 printf("issued by other factions in the same region, regardless of your attitude\n");
 printf("towards the faction in question, and they will attempt to prevent Unfriendly\n");
 printf("units from entering the region.  Only units which are able to tax may be\n");
 printf("on guard.  Units on guard \n");
 if (st_ena)
  {
   printf("  are always visible regardless of Stealth skill, and\n");
  }
 printf("will be marked as being \"on guard\" in the region description.\n");
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"com\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Combat </h2>\n");
 printf("\n");
 printf("Combat occurs when one unit attacks another.  The computer then gathers\n");
 printf("together all the units on the attacking side, and all the units on the\n");
 printf("defending side, and the two sides fight until an outcome is reached. <p>\n");
 printf("\n");
 printf("<a name=\"com_attitudes\">\n");
 printf("<h3> Attitudes: </h3>\n");
 printf("\n");
 printf("Which side a faction's units will fight on depends on declared attitudes.  A\n");
 printf("faction can have one of the following attitudes towards another faction:  Ally,\n");
 printf("Friendly, Neutral, Unfriendly or Hostile.  Each faction has a general attitude,\n");
 printf("called the \"Default Attitude\", that it normally takes towards other factions;\n");
 printf("this is initially Neutral, but can be changed.  It is also possible to declare\n");
 printf("attitudes to specific factions, e.g.\n");
 printf("<a href=\"#declare\"> DECLARE </a> 27 ALLY will declare the Ally\n");
 printf("attitude to faction 27.  (Note that this does not necessarily mean that\n");
 printf("faction 27 is allied to you.) <p>\n");
 printf("\n");
 printf("Ally means that you will fight to defend units of that faction whenever they\n");
 printf("come under attack, if you have non-avoiding units in the region where the\n");
 printf("attack occurs. \n");
 if (st_ena)
  {
   printf("  You will also prevent stealing and assassination attempts\n");
   printf("  against units of the faction\n");
  if (ob_ena)
   {
    printf("   , if you are capable of seeing the unit attempting the crime\n");
   }
   printf("  .\n");
  }
 printf("It also has the implications of the Friendly\n");
 printf("attitude.\n");
 printf("<p>\n");
 printf("\n");
 printf("Friendly means that you will accept gifts from units of that faction.  This\n");
 printf("includes the giving of items, units of people, \n");
 printf("and the teaching of skills.  You will also admit units of that\n");
 printf("faction into\n");
 printf("buildings or ships owned by one of your units, and you will permit units of\n");
 printf("that faction to collect taxes (but not pillage) in regions where you have units\n");
 printf("on guard. <p>\n");
 printf("\n");
 printf("Unfriendly means that you will not admit units of that faction into any region\n");
 printf("where you have units on guard.  You will not, however, automatically attack\n");
 printf("unfriendly units which are already present. <p>\n");
 printf("\n");
 printf("Hostile means that any of your units which do not have the Avoid Combat flag\n");
 printf("set (using the <a href=\"#avoid\"> AVOID </a> order)\n");
 printf("will attack any units of that faction wherever they\n");
 printf("find them. <p>\n");
 printf("\n");
 printf("If a unit can see another unit, but \n");
 if (ob_ena)
  {
   printf("  does not have high enough Observation skill to determine its faction,\n");
  }
 else
  {
   printf("  it is not revealing its faction\n");
  }
 printf("it will treat the unit using the faction's default\n");
 printf("attitude, even if the unit belongs to an Unfriendly or Hostile faction,\n");
 printf("because it does not know the unit's identity.  However, if your faction has\n");
 printf("declared an attitude of Friendly or Ally towards that unit's faction,\n");
 printf("the unit will be treated with the better\n");
 printf("attitude; it is assumed that the unit will produce proof of identity when\n");
 printf("relevant.\n");
 if (st_ena)
  {
   printf("  (See the section on stealth for more information on when units can\n");
   printf("  see each other.)\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("If a faction declares Unfriendly or Hostile as default attitude (the latter is\n");
 printf("a good way to die fast), it will block or attack unidentified units, unless\n");
 printf("they belong to factions for which a more friendly attitude has been\n");
 printf("specifically declared.\n");
 if (st_ena)
  {
   printf("  Units which cannot be seen at all cannot be directly\n");
   printf("  blocked or attacked, of course.\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"com_attacking\">\n");
 printf("<h3> Attacking: </h3>\n");
 printf("\n");
 printf("A unit can attack another by issuing an\n");
 printf("<a href=\"#attack\">ATTACK</a> order.  A unit that does not\n");
 printf("have Avoid Combat set will automatically attack any Hostile units it identifies\n");
 printf("as such.  \n");
 {
 int ncond=!!st_ena+!!SKILL_ENABLED(S_RIDING);
 if (ncond)
  {
   printf("  When a unit issues the <a href=\"#attack\"> ATTACK </a> order, or\n");
   printf("  otherwise decides to attack another unit, it must first be able\n");
   printf("  to attack the unit.\n");
 printf("\n");
  if (st_ena)
  {
   if (ncond==2)
    {
     printf("    There are two conditions for this; the first is that the\n");
    }
   else
    {
     printf("    The\n");
    }
    printf("   attacking unit must be able to see the unit that it\n");
    printf("   wishes to attack.  More information is available on this in the\n");
    printf("   stealth section of the rules. <p>\n");
   }
 printf("\n");
  if (SKILL_ENABLED(S_RIDING))
   {
   if (ncond==2)
    {
     printf("    Secondly, the \n");
    }
   else
    {
     printf("    The\n");
    }
    printf("   attacking unit must be able to catch the unit it\n");
    printf("   wishes to attack.  A unit may only catch a unit if its effective\n");
    printf("   Riding skill is greater than or equal to the target unit's effective\n");
    printf("   Riding skill; otherwise, the target unit just rides away from the\n");
    printf("   attacking unit.  Effective Riding is the unit's Riding skill, but with\n");
    printf("   a potential maximum; if the unit can not ride, the effective Riding\n");
    printf("   skill is 0; if the unit can ride, the maximum effective Riding is\n");
    printf("   3; if the unit can fly, the maximum effective Riding is 5. Note that\n");
    printf("   the effective Riding also depends on whether the unit is attempting\n");
    printf("   to attack or defend;\n");
    printf("   for attack purposes, only one man in the unit needs to be able to\n");
    printf("   ride or fly (generally, this means one of the men must possess a\n");
    printf("   horse, or other form of transportation),\n");
    printf("   whereas for defense purposes the entire unit needs to\n");
    printf("   be able to ride or fly (usually meaning that every man in the unit must \n");
    printf("   possess a horse or other form of speedier transportation).\n");
    printf("   Also, note that for a unit to be able to\n");
    printf("   use its defensive Riding ability to avoid attack, the unit cannot\n");
    printf("   be in a building, ship, or structure of any type. <p>\n");
   }
  }
 printf("\n");
   printf("  A unit which is on guard, and is Unfriendly towards a unit, will\n");
   printf("  deny access to units using the <a href=\"#move\">MOVE</a> order to\n");
   printf("  enter its region.\n");
  if (ncond)
   {
    printf("   Note that to deny access to a unit, the guarding unit\n");
    printf("   must satisfy the above requirements.\n");
   }
   printf("  A unit using <a href=\"#advance\">ADVANCE</a>\n");
   printf("  instead of <a href=\"#move\">MOVE</a> to enter a region, will attack\n");
   printf("  any units that identify it as Unfriendly and attempt to deny it access.  If\n");
   printf("  the advancing unit loses the battle, it will be forced to retreat to the\n");
   printf("  previous region it moved through.  If the unit wins the battle\n");
   printf("  and its army doesn't lose any men, it is allowed to continue to\n");
   printf("  move, provided that it has enough movement points. <p>\n");
 printf("\n");
 if (ncond)
  {
   printf("  Note that these restrictions do not apply for sea combat, as\n");
  if (st_ena)
   {
    printf("   units within a ship are always visible\n");
   }
   printf("  %s\n",ncond==2?", and ":"");
  if (SKILL_ENABLED(S_RIDING))
   {
    printf("   Riding does not play a part in combat on board ships\n");
   }
   printf("  .<p>\n");
  }
 printf("\n");
 }
 printf("\n");
 printf("<a name=\"com_muster\">\n");
 printf("<h3> The Muster: </h3>\n");
 printf("\n");
 printf("Once the attack has been made, the sides are gathered.  Although the\n");
 printf("<a href=\"#attack\"> ATTACK </a>\n");
 printf("order takes a unit rather than a faction as its parameter (mainly so that\n");
 printf("unidentified units can be attacked), an attack is basically considered to be by\n");
 printf("an entire faction, against an entire faction and its allies. <p>\n");
 printf("\n");
 printf("On the attacking side are all units of the attacking faction in the region\n");
 printf("where the fight is taking place, except those with Avoid Combat set.  A unit\n");
 printf("which has explicitly issued an\n");
 printf("<a href=\"#attack\"> ATTACK </a> order will join the fight anyway,\n");
 printf("regardless of Avoid Combat. <p>\n");
 printf("\n");
 printf("Also on the attacking side are all units of other factions that attacked the\n");
 printf("target faction in the region where the fight is taking place.  In other words,\n");
 printf("if several factions attack one, then all their armies join together to attack\n");
 printf("at the same time (even if they are enemies and will later fight each other).\n");
 printf("<p>\n");
 printf("\n");
 printf("On the defending side are all identifiable units belonging to the defending\n");
 printf("faction.  If a unit has Avoid Combat set, and its faction cannot be identified\n");
 printf("by the attacking faction, it will not be involved in the battle.  A unit which\n");
 printf("was explicitly attacked will be involved anyway, regardless of Avoid Combat.\n");
 printf("Also, all non-avoiding units in factions allied with the defending unit will join in\n");
 printf("on the defending side. <p>\n");
 printf("\n");
 printf("Units in adjacent regions can also become involved.  This is the exception to\n");
 printf("the general rule that you cannot interact with units in a different region.\n");
 printf("<p>\n");
 printf("\n");
 printf("If a faction has at least one unit involved in the initial region, then any\n");
 printf("units in adjacent regions will join the fight, if they could reach the region\n");
 printf("and do not have Avoid Combat set.  There are a few flags that units may\n");
 printf("set to affect this; a unit with the Hold flag\n");
 printf("(set using the <a href=\"#hold\"> HOLD </a> order)\n");
 printf("will not join battles in adjacent regions.  \n");
 printf("This flag applies to both attacking and defending factions.  A unit with\n");
 printf("the Noaid flag (set using the <a href=\"#noaid\"> NOAID </a> order) will\n");
 printf("receive no aid from adjacent hexes when attacked, or when it issues an\n");
 printf("attack. <p>\n");
 printf("\n");
 printf("Example:  A fight starts in region A, in the initial combat phase (before any\n");
 printf("movement has occurred).  The defender has a unit of soldiers in adjacent region\n");
 printf("B.  They have 2 movement points at this stage.  They will buy horses later in\n");
 printf("the turn, so that when they execute their\n");
 printf("<a href=\"#move\"> MOVE </a> order they will have 4 movement\n");
 printf("points, but right now they have 2.  Region A is forest but fortunately it is\n");
 printf("summer so the soldiers can join the fight.\n");
 printf("<p>\n");
 printf("\n");
 printf("It is important to note that the units in nearby regions do not actually move\n");
 printf("to the region where the fighting happens; the computer only checks that they\n");
 printf("could move there.  (In game world terms, presumably they did move\n");
 printf("there to join the fight, and then moved back where they started.)  The computer\n");
 printf("checks for weight allowances and terrain types when determining whether a unit\n");
 printf("could reach the scene of the battle.\n");
 printf("Note that the use of ships is not allowed in this virtual movement. <p>\n");
 printf("\n");
 printf("If you order an attack on an ally (either with the\n");
 printf("<a href=\"#attack\"> ATTACK </a> order, or if your\n");
 printf("ally has declared you Unfriendly, by attempting to\n");
 printf("<a href=\"#advance\"> ADVANCE </a> into a region which\n");
 printf("he is guarding), then your commander will decide that a mistake has occurred\n");
 printf("somewhere, and withdraw your troops from the fighting altogether.  Thus, your\n");
 printf("units will not attack that faction in that region. Note that you will\n");
 printf("always defend an ally against attack, even if it means that you fight\n");
 printf("against other factions that you are allied with. <p>\n");
 printf("\n");
 printf("<a name=\"com_thebattle\">\n");
 printf("<h3> The Battle: </h3>\n");
 printf("\n");
 printf("The troops having lined up, the fight begins.\n");
 if (SKILL_ENABLED(S_TACTICS))
  {
   printf("  The computer selects the best\n");
   printf("  tactician from each side; that unit is regarded as the leader of its side.  If\n");
   printf("  two or more units on one side have the same Tactics skill, then the one with\n");
   printf("  the lower unit number is regarded as the leader of that side.  If one side's\n");
   printf("  leader has a better Tactics skill than the other side's, then that side gets a\n");
   printf("  free round of attacks.\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("In each combat round, the combatants each get to attack once, in a random\n");
 printf("order. \n");
 if (SKILL_ENABLED(S_TACTICS))
  {
   printf("  (In a free round of attacks, only one side's forces get to attack.)\n");
  }
 printf("Each combatant will attempt to hit a randomly selected enemy.  If he hits, and\n");
 printf("the target has no armor, then the target is automatically killed.  Armor may\n");
 printf("provide extra defense against otherwise successful attacks.<p>\n");
 printf("\n");
 printf("The basic skill used in battle is the Combat skill; this is used for hand to\n");
 printf("hand fighting.  If one soldier tries to hit another using melee weapons, there\n");
 printf("is a 50%% chance that the attacker will get an opportunity for a lethal blow.\n");
 printf("If the attacker does get that opportunity, then there is a contest between\n");
 printf("his combat skill (modified by weapon attack bonus) and the defender's combat\n");
 printf("skill (modified by weapon defense bonus).   Some weapons may not allow combat\n");
 printf("skill to affect defense (e.g. bows), and others may allow different skills\n");
 printf("to be used on defense.<p>\n");
 printf("\n");
 printf("If the skills are equal, then there is a 1:1 (i.e. 50%%) chance that the attack\n");
 printf("will succeed.  If the attacker's skill is 1 higher then there is a 2:1 (i.e.\n");
 printf("66%%) chance, if the attacker's skill is 2 higher then there is a 4:1 (i.e.\n");
 printf("80%%) chance, 3 higher means an 8:1 (i.e. 88%%) chance, and so on.  Similarly\n");
 printf("if the defender's skill is 1 higher, then there is only a 1:2 (i.e. 33%%)\n");
 printf("chance, etc. <p>\n");
 printf("\n");
 printf("Possession of a sword confers a +2 bonus to Combat skill for both attack\n");
 printf("and defense.  (Troops fighting hand-to-hand without specific weapons are \n");
 printf("assumed to be irregularly equipped with knives, clubs etc.)\n");
 if (SKILL_ENABLED(S_RIDING))
  {
   printf("  Possession of\n");
   printf("  a horse, and Riding skill, also confers a bonus to effective Combat skill\n");
   printf("  equal to the Riding skill level (up to a maximum of 3) provided that the\n");
   printf("  terrain allows horses to be used.  Winged horse are better yet, but require\n");
   printf("  more basic Riding skill to gain any advantage.\n");
  }
 printf("Certain weapons may provide different attack and defense bonuses, or have additional attack bonuses\n");
 printf("against mounted opponents.<p>\n");
 printf("\n");
 printf("Some melee weapons may be defined as Long or Short (this is relative to a\n");
 printf("normal weapon, e.g. the sword).  A soldier wielding a longer weapon than\n");
 printf("his opponent gets a +1 bonus to his attack skill.<p>\n");
 printf("\n");
 printf("Missile weapons are slightly different from melee weapons.  A soldier who\n");
 printf("has a longbow and is skilled in its use will use it; otherwise, he will use\n");
 printf("a crossbow if he has one, and skill in its use; otherwise, he will fight hand\n");
 printf("to hand.  The skill check to hit with a longbow is made against an effective\n");
 printf("defense of 2; i.e., a longbowman with skill 1, having made the 50%% chance of\n");
 printf("getting an effective attack, has a 1:2 chance of hitting a target.  A\n");
 printf("crossbow is an easier weapon to use, so the chance to hit is calculated\n");
 printf("against a defense of 0; on the other hand, a crossbow can only fire every\n");
 printf("other round (the first, third, fifth, etc., rounds, including the free\n");
 printf("round of attacks if one's side has one).  Note that the target unit's actual\n");
 printf("skills are irrelevant for bow attacks. <p>\n");
 printf("\n");
 printf("Weapons may have one of several different attack types: Slashing, Piercing,\n");
 printf("Crushing, Cleaving and Armor Piercing.  Different types of armor may give\n");
 printf("different survival chances against a sucessful attack of different types.<p>\n");
 printf("\n");
 printf("A soldier with a melee weapon attacking a bowman makes his attack just as if\n");
 printf("the bowman had a Combat skill of 0, even if the bowman is a leader who also\n");
 printf("has Combat skill. <p>\n");
 printf("\n");
 printf("Being inside a building confers a +2 bonus to defense.  This bonus is effective\n");
 printf("against bows as well as melee weapons.  The number of men that a building can\n");
 printf("protect is equal to its size.  The sizes of the different types of buildings\n");
 printf("are as follows: <p>\n");
 printf("\n");
 printf("<a name=\"tablebuildcapacity\">\n");
 printf("<center>\n");
 printf("<table border>\n");
  printf(" <tr>\n");
   printf("  <td> </td>\n");
   printf("  <th>Size</th>\n");
  printf(" </tr>\n");
 {
 int lbs[]={O_TOWER,O_FORT,O_CASTLE,O_CITADEL};
 int nlbs=sizeof(lbs)/sizeof(int);
 for (int i=0;i<nlbs;i++)
  {
  if (OBJECT_ENABLED(lbs[i]))
   {
   ObjectType *ot=ObjectDefs+lbs[i];
    printf("   <tr>\n");
     printf("    <td align=\"center\">%s</td>\n",ot->name);
     printf("    <td align=\"center\">%d</td>\n",ot->protect);
    printf("   </tr>\n");
   }
  }
 }
 printf("</table> </center> <p>\n");
 printf("\n");
 printf("If there are too many units in a building to all gain protection from it, then\n");
 printf("those units who have been in the building longest will gain protection.  (Note\n");
 printf("that these units appear first on the turn report.)  If a unit of 200 men is\n");
 printf("inside a Fort (capacity 50), then the first 50 men in the unit will gain the\n");
 printf("full +2 bonus, and the other 150 will gain no protection. <p>\n");
 printf("\n");
 printf("Units which have the Behind flag set are at the rear and cannot be attacked by\n");
 printf("any means until all non-Behind units have been wiped out.  On the other hand,\n");
 printf("neither can they attack with melee weapons, but only with bows or magic.  Once\n");
 printf("all front-line units have been wiped out, then the Behind flag no longer has\n");
 printf("any effect. <p>\n");
 printf("\n");
 printf("<a name=\"com_victory\">\n");
 printf("<h3> Victory! </h3>\n");
 printf("\n");
 printf("Combat rounds continue until one side has accrued 50%% losses (or more). The\n");
 printf("victorious side is then awarded one free round of attacks, after which the\n");
 printf("battle is over.  If both sides have more than 50%% losses, the battle is a draw,\n");
 printf("and neither side gets a free round. <p>\n");
 printf("\n");
 if (SKILL_ENABLED(S_HEALING))
  {
   printf("  Units with the Healing skill have a chance of being able to heal casualties of\n");
   printf("  the winning side, so that they recover rather than dying.  Each character with\n");
   printf("  this skill can attempt to heal %d casualties per skill level.\n",Globals->HEALS_PER_MAN);
   printf("  Each attempt however requires\n");
   printf("  one unit of Herbs, which is thereby used up.  Each attempt has a 50%% chance of\n");
   printf("  healing one casualty; only one attempt at Healing may be made per casualty.  \n");
   printf("  Healing occurs automatically, after the battle is over, by any living healers\n");
   printf("  on the winning side. <p>\n");
  }
 printf("\n");
 printf("Any items owned by dead combatants on the losing side have a 50%% chance of\n");
 printf("being found and collected by the winning side.  Each item which is recovered is\n");
 printf("picked up by one of the survivors at random, so the winners generally collect\n");
 printf("loot in proportion to their number of surviving men. <p>\n");
 printf("\n");
 printf("If you are expecting to fight an enemy who is carrying so much equipment that\n");
 printf("you would not be able to move after picking it up, and you want to move to\n");
 printf("another region later that month, it may be worth issuing some orders to drop\n");
 printf("items (with the <a href=\"#give\"> GIVE </a>\n");
 printf("0 order) in case you win the battle! Also, note that if\n");
 printf("the winning side took any losses in the battle, any units on this side will not\n");
 printf("be allowed to move, or attack again for the rest of the turn. <p>\n");
 printf("\n");
 if (st_ena||ob_ena)
  {
   printf("  <a name=\"stealthobs\">\n");
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <h2> %s%s%s</h2>\n",st_ena?"Stealth":"",so_ena?" and ":"",ob_ena?"Observation":"");
 printf("\n");
  if (so_ena)
   {
    printf("   The Stealth skill is used to hide units, while the Observation skill\n");
    printf("   is used to see units that would otherwise be hidden. A unit can be\n");
    printf("   seen only if you have at least one unit in the same region, with an\n");
    printf("   Observation skill at least as high as that unit's Stealth skill.\n");
    printf("   If your Observation skill is equal to the unit's Stealth skill, you\n");
    printf("   will see the unit, but not the name of the owning faction. If your\n");
    printf("   Observation skill is higher than the unit's Stealth skill, you will\n");
    printf("   also see the name of the faction that owns the unit. <p>\n");
   } else
  if (st_ena)
   {
    printf("   The Stealth skill is used to hide units\n");
    printf("   A unit can be seen only \n");
    printf("   if it doesn't know Stealth skill and\n");
    printf("   if you have at least one unit in the same region, \n");
    printf("   <p>\n");
   }
  if (ob_ena)
   {
    printf("   Observation skill\n");
    printf("   is used to see information about units that would otherwise be hidden. \n");
    printf("   If your unit knows Observation skill, it will\n");
    printf("   see the name of the faction that owns any unit in the same region. <p>\n");
   }
 printf("\n");
  if (st_ena)
   {
    printf("   Regardless of Stealth skill, units are always visible when participating\n");
    printf("   in combat; when guarding a region with the Guard flag; or when in a\n");
    printf("   building or aboard a ship. \n");
   if (ob_ena)
    {
     printf("    However, in order to see the faction that\n");
     printf("    owns the unit, you will still need a higher Observation skill than\n");
     printf("    the unit's Stealth skill.\n");
    }
     printf("    <p>\n");
   }
 printf("\n");
  if (st_ena)
   {
    printf("   <a name=\"stealthobs_stealing\">\n");
    printf("   <h3> Stealing: </h3>\n");
 printf("\n");
    printf("   The <a href=\"#steal\"> STEAL </a>\n");
    printf("   order is a way to steal items from other factions without a battle. \n");
    printf("   The order can only be issued by a one-man unit.  The order specifies a target\n");
    printf("   unit; the thief will then attempt to steal the specified item from the target\n");
    printf("   unit. <p>\n");
   printf("  \n");
   if (ob_ena)
    {
     printf("    If the thief has higher Stealth than any of the target faction's units have\n");
     printf("    Observation (i.e. the thief cannot be seen by the target faction), the theft\n");
     printf("    will succeed.\n");
    }
   else
    {
     printf("    The thief must know Stealh to attempt theft.\n");
    }
    printf("   The target faction will be told what was stolen, but not by\n");
    printf("   whom.  If the specified item is %s, then $200 or half the total available,\n",silver);
    printf("   whichever is less, will be stolen.  If it is any other item, then only one will\n");
    printf("   be stolen (if available). <p>\n");
 printf("\n");
   if (ob_ena)
    {
     printf("    Any unit with high enough Observation to see the thief will see the attempt to\n");
     printf("    steal, whether the attempt is successful or not.  Allies of the target unit\n");
     printf("    will prevent the theft, if they have high enough Observation to see the unit\n");
     printf("    trying to steal. <p>\n");
    }
 printf("\n");
    printf("   <a name=\"stealthobs_assassination\">\n");
    printf("   <h3> Assassination: </h3>\n");
 printf("\n");
    printf("   The <a href=\"#assassinate\"> ASSASSINATE </a>\n");
    printf("   order is a way to kill another person without attacking and going\n");
    printf("   through an entire battle. This order can only be issued by a one-man unit,\n");
    printf("   and specifies a target unit.  If the target unit contains more than one person,\n");
    printf("   then one will be singled out. <p>\n");
 printf("\n");
   if (ob_ena)
    {
     printf("    Success for assassination is determined as for theft, i.e. the assassin will\n");
     printf("    fail if any of the target faction's units can see him.  In this case, the\n");
     printf("    assassin will flee, and the target faction will be informed which unit made the\n");
     printf("    attempt.  As with theft, allies of the target unit will prevent the\n");
     printf("    assassination from succeeding, if their Observation level is high enough. <p>\n");
    }
   else
    {
     printf("    The assasin must know Stealh to attempt assassination.\n");
    }
 printf("\n");
   if (ob_ena)
    {
     printf("    If the assassin has higher Stealth than any of the target faction's units have\n");
     printf("    Observation, then a one-on-one \n");
    }
   else
    {
     printf("    One-on-one\n");
    }
    printf("   fight will take place between the assassin and\n");
    printf("   the target character.  The assassin automatically gets a free round of attacks;\n");
    printf("   after that, the battle is handled like a normal fight, with the exception that neither\n");
    printf("   assassin nor victim can use \n");
   {
   int disar[]={I_CHAINARMOR,I_PLATEARMOR,I_LEATHERARMOR};
   int ndisar=sizeof(disar)/sizeof(int);
   int aen=0;
   for (int i=0;i<ndisar;i++)
    {
    aen+=!!ITEM_ENABLED(disar[i]);
    }
   if (aen)
    {
    int fst=1;
    for (int i=0;i<ndisar;i++)
     {
     if (ITEM_ENABLED(disar[i]))
      {
       printf("      %s%s\n",fst?"":" or ",ItemDefs[disar[i]].name);
      fst=0;
      }
     }
    }
   else
    {
     printf("    any metal armor\n");
    }
   }
    printf("   (the assassin because he\n");
    printf("   cannot sneak around wearing metal armor, the victim because he was caught by\n");
    printf("   surprise with his armor off).  If the assassin wins, the target faction is told\n");
    printf("   merely that the victim was assassinated, but not by whom.  If the victim wins,\n");
    printf("   then the target faction learns which unit made the attempt.  (Of course, this\n");
    printf("   does not necessarily mean that the assassin's faction is known.)  The winner of\n");
    printf("   the fight gets 50%% of the loser's property as usual. <p>\n");
 printf("\n");
    printf("   <a href=\"#steal\"> STEAL </a> and <a href=\"#assassinate\"> ASSASSINATE </a>\n");
    printf("   are not full month orders, and do not interfere with\n");
    printf("   other activities, but a unit can only issue one\n");
    printf("   <a href=\"#steal\"> STEAL </a> order or one\n");
    printf("   <a href=\"#assassinate\"> ASSASSINATE </a>\n");
    printf("   order in a month. <p>\n");
 printf("\n");
   } /* Steal&assasinate */
  } /* stealth&observation */
 printf("\n");
 printf("<a name=\"magic\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Magic </h2>\n");
 printf("\n");
 printf("A character enters the world of magic in Atlantis by beginning study on one\n");
 printf("of the Foundation magic skills.  Only one man units\n");
 if (!Globals->MAGE_NONLEADERS)
  {
   printf("  , with the man being a leader , \n");
  }
 printf("are permitted to study these skills.  \n");
 printf("\n");
 if (Globals->FACTION_LIMIT_TYPE!=GameDefs::FACLIM_UNLIMITED)
  {
   printf("  The number of these units (known\n");
   printf("  as \"magicians\" or \"mages\") that a faction may own is determined by the\n");
   printf("  faction's type.  Any attempt to gain more, either through study, or by\n");
   printf("  transfer from another faction, will fail.  In addition, mages \n");
  }
  else
  {
   printf("  Mages\n");
  }
 printf("may not <a href=\"#give\">GIVE</a> men\n");
 printf("at all; once a unit becomes a mage (by studying one of the Foundations), the\n");
 printf("unit number is fixed. (The mage may be given to another faction using\n");
 printf("the <a href=\"#give\">GIVE UNIT</a> order.) <p>\n");
 printf("\n");
 printf("<a name=\"magic_skills\">\n");
 printf("<h3> Magic Skills: </h3>\n");
 printf("\n");
 printf("Magic skills are the same as normal skills, with a few differences.  The basic\n");
 printf("magic skills, called Foundations, are Force, Pattern, and Spirit.  To become\n");
 printf("a mage, a unit undertakes study in one of these Foundations.  As a unit\n");
 printf("studies the Foundations, he will be able to study deeper into the magical arts;\n");
 printf("the additional skills that he may study will be indicated on your turn report.\n");
 printf("<p>\n");
 printf("\n");
 printf("There are two major differences between Magic skills and normal skills. The\n");
 printf("first is that the ability to study Magic skills sometimes depends on lower \n");
 printf("level Magic skills.  The Magic skills that a mage may study are listed on\n");
 printf("his turn report, so he knows which areas he may pursue.  Studying higher\n");
 printf("in the Foundation skills, and certain other Magic skills, will make other\n");
 printf("skills available to the mage.\n");
 printf("Also, study into a magic skill above level 2 requires that the mage\n");
 printf("be located in some sort of protected building (a tower, fortress, castle or\n");
 printf("citadel; mines, quarries, etc, do not count).  If the mage is not in such a\n");
 printf("structure, his study rate is cut in half, as he does not have the proper\n");
 printf("environment and equipment for research. <p>\n");
 printf("\n");
 printf("<a name=\"magic_foundations\">\n");
 printf("<h3> Foundations: </h3>\n");
 printf("\n");
 printf("The three Foundation skills are called Force, Pattern, and Spirit.  Force\n");
 printf("indicates the quantity of magical energy that a mage is able to channel (a\n");
 printf("Force rating of 0 does not mean that the mage can channel no magical energy at\n");
 printf("all, but only a minimal amount).  Pattern indicates ability to handle complex\n");
 printf("patterns, and is important for things like healing and nature spells.  Spirit\n");
 printf("deals with meta-effects that lie outside the scope of the physical world.\n");
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"magic_furtherstudy\">\n");
 printf("<h3> Further Magic Study: </h3>\n");
 printf("\n");
 printf("Once a mage has begun study of one or more Foundations, more skills that he may\n");
 printf("study will begin to show up on his report.  These skills are the skills that\n");
 printf("give a mage his power.  As with normal skills, when a mage achieves a new level\n");
 printf("of a magic skill, he will be given a skill report, describing the new powers\n");
 printf("(if any) that the new skill confers.  The \n");
 printf("<a href=\"#show\"> SHOW </a> order may be used to show this\n");
 printf("information on future reports. <p>\n");
 printf("\n");
 printf("<a name=\"magic_usingmagic\">\n");
 printf("<h3> Using Magic: </h3>\n");
 printf("\n");
 printf("A mage may use his magical power in three different ways, depending\n");
 printf("on the type of spell he wants to use.  Some spells,\n");
 printf("once learned, take effect automatically and are considered always to\n");
 printf("be in use; these spells do not require any order to take effect. <p>\n");
 printf("\n");
 printf("Secondly, some spells are for use in combat. A mage may specify that\n");
 printf("he wishes to use a spell in combat by issuing the\n");
 printf("<a href=\"#combat\"> COMBAT </a> order.  A combat spell specified in this\n");
 printf("way will only be used if the mage finds himself taking part in a battle. <p\\>\n");
 printf("\n");
 printf("The third type of spell use is for spells that take an entire month\n");
 printf("to cast.  These spells are cast by the mage issuing the\n");
 printf("<a href=\"#cast\"> CAST </a> order.  Because\n");
 printf("<a href=\"#cast\"> CAST </a> takes an entire\n");
 printf("month, a mage may use only one of this type of spell each turn. Note,\n");
 printf("however, that a <a href=\"#cast\"> CAST </a> order is not a full\n");
 printf("month order; a mage may still <a href=\"#move\"> MOVE </a>,\n");
 printf("<a href=\"#study\"> STUDY </a>, or any other month long order. The\n");
 printf("justification for this (as well as being for game balance) is that\n");
 printf("a spell drains a mage of his magic power for the\n");
 printf("month, but does not actually take the entire month to cast. <p>\n");
 printf("\n");
 printf("The description that a mage receives when he first learns a spell\n");
 printf("specifies the manner in which the spell is used (automatic, in combat,\n");
 printf("or by casting). <p>\n");
 printf("\n");
 printf("<a name=\"magic_incombat\">\n");
 printf("<h3> Magic in Combat: </h3>\n");
 printf("\n");
 printf("NOTE: This section is rather vague, and quite advanced.  You may want to wait\n");
 printf("until you have figured out other parts of Atlantis before trying to understand\n");
 printf("exactly all of the rules in this section. <p>\n");
 printf("\n");
 printf("Although the magic skills and spells are unspecified in these rules, left for\n");
 printf("the players to discover, the rules for combat spells' interaction are spelled\n");
 printf("out here.  There are five major types of attacks, and defenses:\n");
 printf("Combat, Bow, Energy, Weather, and Spirit.  Every attack and defense has\n");
 printf("a type, and only the appropriate defense is effective against an attack. <p>\n");
 printf("\n");
 printf("Defensive spells are cast at the beginning of each round\n");
 printf("of combat, and will have a type of attack they deflect, and skill level\n");
 printf("(Defensive spells are generally called Shields).  Every time an attack\n");
 printf("is launched against an army, it must first attack the highest level Shield\n");
 printf("of the same type as the attack, before it may attack a soldier\n");
 printf("directly. Note that an attack only has to attack the highest Shield,\n");
 printf("any other Shields of the same type are ignored for that attack. <p>\n");
 printf("\n");
 printf("An attack spell (and any other type of attack) also has an attack type, and\n");
 printf("attack level, and a number of blows it deals.  When the attack spell is cast,\n");
 printf("it is matched up against the most powerful defensive spell of the appropriate\n");
 printf("type that the other army has cast.  If the other army has not cast any\n");
 printf("applicable defensive spells, the spell goes through unmolested.  Unlike normal\n");
 printf("combat however, men are at a disadvantage to defending against spells.   Men\n");
 printf("which are in the open (not protected by a building) have an effective skill\n");
 printf("of -2 unless they have a shield or some other defensive magic.  Some monsters\n");
 printf("have bonuses to resisting some attacks but are more susceptible to others.\n");
 printf("The skill level of the attack spell and the effective skill for defense are\n");
 printf("matched against each other.  The formula for determining the victor between a\n");
 printf("defensive and offensive spell is the same as for a contest of soldiers; if the\n");
 printf("levels are equal, there is a 1:1 chance of success, and so on.  If the\n");
 printf("offensive spell is victorious, the offensive spell deals its blows to the\n");
 printf("defending army, and the Shield in question is destroyed (thus, it can be\n");
 printf("useful to have more than one of the same type of Shield in effect, as the\n");
 printf("other Shield will take the place of the destroyed one).  Otherwise, the\n");
 printf("attack spell disperses, and the defending spell remains in place. <p>\n");
 printf("\n");
 printf("Some spells do not actually kill enemies, but rather have some negative\n");
 printf("effect on them. These spells are treated the same as normal spells; if\n");
 printf("there is a Shield of the same type as them, they must attack the Shield\n");
 printf("before attacking the army.\n");
 printf("Physical attacks that go through a defensive spell also must match their skill\n");
 printf("level against that of the defensive spell in question.  However, they do not\n");
 printf("destroy a layer of the spell when they are successful. <p>\n");
 printf("\n");
 printf("<a name=\"nonplayers\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Non-Player Units </h2>\n");
 printf("\n");
 printf("There are a number of units that are not controlled by players that may be\n");
 printf("encountered in Atlantis.  Most information about these units must be\n");
 printf("discovered in the course of the game, but a few basics are below. <p>\n");
 printf("\n");
 if (Globals->TOWNS_EXIST&&Globals->CITY_MONSTERS_EXIST)
  {
 printf("\n");
   printf("  <a name=\"nonplayers_guards\">\n");
   printf("  <h3> City and Town Guardsmen: </h3>\n");
 printf("\n");
   printf("  All cities and towns begin with guardsmen in them.  These units will defend any\n");
   printf("  units that are attacked in the city or town, and will also prevent theft and\n");
   printf("  assassination attempts, if their Observation level is high enough.  They are on\n");
   printf("  guard, and will prevent other units from taxing or pillaging.  The guards may\n");
   printf("  be killed by players, although they will form again if the city is left\n");
   printf("  unguarded. <p>\n");
 printf("\n");
  if (Globals->START_CITY_GUARDS_PLATE)
   {
    printf("   Note that the city guardsmen in the starting cities of Atlantis possess\n");
    printf("   plate armor in addition to being more numerous and are harder therefore\n");
    printf("   to kill. \n");
    printf("   if (Globals->START_CITY_MAGES)\n");
    {
     printf("    Additionally, in \n");
    }
   } else
  if (Globals->START_CITY_MAGES)
   {
    printf("   In\n");
   }
  if (Globals->START_CITY_MAGES)
   {
    printf("   the starting cities, Mage Guards will be found.\n");
    printf("   These mages are adept at the fire spell making any attempt to control\n");
    printf("   a starting city a much harder proposition.\n");
   }
   printf("  <p>\n");
 printf("\n");
  }
 printf("\n");
 if (Globals->WANDERING_MONSTERS_EXIST)
  {
   printf("  <a name=\"nonplayers_monsters\">\n");
   printf("  <h3> Wandering Monsters: </h3> \n");
 printf("\n");
   printf("  There are a number of monsters who wander free through Atlantis.  They will\n");
   printf("  occasionally attack player units, so be careful when wandering through the\n");
   printf("  wilderness. <p>\n");
  }
 printf("\n");
 printf("<a name=\"nonplayers_controlled\">\n");
 printf("<h3> Controlled Monsters: </h3>\n");
 printf("\n");
 printf("Through various magical methods, you may gain control of certain\n");
 printf("types of monsters. These monsters are just another item in a unit's\n");
 printf("inventory, with a few special rules. Monsters will be able to carry\n");
 printf("things at their speed of movement; use the <a href=\"#show\">SHOW ITEM</a>\n");
 printf("order to determine the carrying capacity and movement speed of a\n");
 printf("monster. Monsters will also fight for the controlling unit in combat;\n");
 printf("their strength can only be determined in battle. Also, note that a\n");
 printf("monster will always fight from the front rank, even if the controlling\n");
 printf("unit has the behind flag set. Whether or not you are allowed to give\n");
 printf("a monster to other units depends on the type of monster; some may be\n");
 printf("given freely, while others must remain with the controlling unit. <p>\n");
 printf("\n");
 printf("<a name=\"orders\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Orders </h2>\n");
 printf("\n");
 printf("To enter orders for Atlantis, you should send a mail message to the Atlantis\n");
 printf("server, containing the following: <p>\n");
 printf("\n");
 printf("<pre>\n");
 printf("#ATLANTIS faction-no <password>\n");
 printf("\n");
 printf("UNIT unit-no\n");
 printf("...orders...\n");
 printf("\n");
 printf("UNIT unit-no\n");
 printf("...orders...\n");
 printf("\n");
 printf("#END\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("For example, if your faction number (shown at the top of your report) is 27,\n");
 printf("and you have two units numbered 5 and 17: <p>\n");
 printf("\n");
 printf("<pre>\n");
 printf("#ATLANTIS 27\n");
 printf("\n");
 printf("UNIT 5\n");
 printf("...orders...\n");
 printf("\n");
 printf("UNIT 17\n");
 printf("...orders...\n");
 printf("\n");
 printf("#END\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("Thus, orders for each unit are given separately, and indicated with the UNIT\n");
 printf("keyword.  (In the case of an order, such as the command to rename your faction,\n");
 printf("that is not really for any particular unit, it does not matter which unit\n");
 printf("issues the command; but some particular unit must still issue it.) <p>\n");
 printf("\n");
 printf("IMPORTANT: You MUST use the correct #ATLANTIS line or else your orders will be\n");
 printf("silently ignored. <p>\n");
 printf("\n");
 printf("If you have a password set, you must specify it on you #atlantis line, or\n");
 printf("the game will reject your orders.  See the\n");
 printf("<a href=\"#password\"> PASSWORD </a> order for more details.\n");
 printf("<p>\n");
 printf("\n");
 printf("Each type of order is designated by giving a keyword as the first non-blank\n");
 printf("item on a line.  Parameters are given after this, separated by spaces or tabs.\n");
 printf("Blank lines are permitted, as are comments; anything after a semicolon is\n");
 printf("treated as a comment (provided the semicolon is not in the middle of a word).\n");
 printf("<p>\n");
 printf("\n");
 printf("The parser is not case sensitive, so all commands may be given in upper case,\n");
 printf("low case or a mixture of the two.  However, when supplying names containing\n");
 printf("spaces, the name must be surrounded by double quotes, or else underscore\n");
 printf("characters must be used in place of spaces in the name.  (These things apply to\n");
 printf("the #ATLANTIS and #END lines as well as to order lines.) <p>\n");
 printf("\n");
 printf("You may precede orders with the at sign (@), in which case they will\n");
 printf("appear in the Template at the bottom of your report.  This is useful\n");
 printf("for orders which your units repeat for several months in a row. <p>\n");
 printf("\n");
 printf("<a name=\"orders_abbreviations\">\n");
 printf("<h3> Abbreviations: </h3>\n");
 printf("\n");
 printf("All common items and skills have abbreviations that can be used when giving\n");
 printf("orders, for brevity.  Any time you see the item on your report, it will\n");
 printf("be followed by the abbreviation.  Please be careful using these, as they\n");
 printf("can easily be confused. <p>\n");
 printf("\n");
 printf("<a name=\"ordersummary\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Order Summary </h2>\n");
 printf("\n");
 printf("To specify a [unit], use the unit number.  If specifying a unit that will be\n");
 printf("created this turn, use the form \"NEW #\" if the unit belongs to your faction, or\n");
 printf("\"FACTION # NEW #\" if the unit belongs to a different faction.  See the\n");
 printf("<a href=\"#form\"> FORM </a>\n");
 printf("order for a more complete description.  [faction] means that a faction number\n");
 printf("is required; [object] means that an object number (generally the number of a\n");
 printf("building or ship) is required.  [item] means an item (like wood or\n");
 printf("longbow) that a unit can have in its possession. [flag] is an argument\n");
 printf("taken by several orders, that sets or unsets a flag for a unit. A [flag]\n");
 printf("value must be either 1 (set the flag) or 0 (unset the flag).  Other\n");
 printf("parameters are generally numbers or names. <p>\n");
 printf("\n");
 printf("IMPORTANT: Remember that names containing spaces (e.g., \"Plate Armor\"), must be\n");
 printf("surrounded by double quotes, or the space must be replaced with an underscore\n");
 printf("\"_\" (e.g., Plate_Armor). <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"address\"> </a>\n");
 printf("<h4> ADDRESS [new address] </h4>\n");
 printf("\n");
 printf("Change the email address to which your reports are sent. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Change your faction's email address to atlantis@rahul.net. <p>\n");
 printf("<pre>\n");
   printf("  ADDRESS atlantis@rahul.net\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"advance\"> </a>\n");
 printf("<h4> ADVANCE [dir] ... </h4>\n");
 printf("\n");
 printf("This is the same as the <a href=\"#move\"> MOVE </a>\n");
 printf("order, except that it implies attacks on units\n");
 printf("which attempt to forbid access.  See the\n");
 printf("<a href=\"#move\"> MOVE </a> order for details. <p>\n");
 printf("\n");
 printf("Examples: <p>\n");
 printf("Move north, then northwest, attacking any units that forbid access to the\n");
 printf("regions. <p>\n");
 printf("<pre>\n");
   printf("  ADVANCE N NW\n");
 printf("</pre> <p>\n");
 printf("In order, move north, then enter structure number 1, move through an inner\n");
 printf("route, and finally move southeast. Will attack any units that forbid access to\n");
 printf("any of these locations. <p>\n");
 printf("<pre>\n");
   printf("  ADVANCE N 1 IN SE\n");
 printf("</pre> <p>\n");
 printf("\n");
 if (st_ena)
  {
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <a name=\"assassinate\"> </a>\n");
   printf("  <h4> ASSASSINATE [unit] </h4>\n");
 printf("\n");
   printf("  Attempt to assassinate the specified unit, or one of the unit's people if the\n");
   printf("  unit contains more than one person.  The order may only be issued by a one-man\n");
   printf("  unit. <p>\n");
 printf("\n");
   printf("  A unit may only attempt to assassinate a unit which is able to be seen.<p>\n");
 printf("\n");
   printf("  Example: <p>\n");
   printf("  Assassinate unit number 177. <p>\n");
   printf("  <pre>\n");
   printf("  ASSASSINATE 177\n");
   printf("  </pre> <p>\n");
  }
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"attack\"> </a>\n");
 printf("<h4> ATTACK [unit] .. </h4>\n");
 printf("\n");
 printf("Attack a target unit.  If multiple ATTACK orders are given, all of the targets\n");
 printf("will be attacked. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("To attacks units 17, 431, and 985: <p>\n");
 printf("<pre>\n");
   printf("  ATTACK 17\n");
   printf("  ATTACK 431 985\n");
 printf("</pre> <p>\n");
 printf("or: <p>\n");
 printf("<pre>\n");
   printf("  ATTACK 17 431 985\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"autotax\"> </a>\n");
 printf("<h4> AUTOTAX [flag] </h4>\n");
 printf("\n");
 printf("AUTOTAX 1 causes the unit to attempt to tax every turn (without requiring\n");
 printf("the TAX order) until the flag is unset. AUTOTAX 0 unsets the flag. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("To cause the unit to attempt to tax every turn. <p>\n");
 printf("<pre>\n");
   printf("  AUTOTAX 1\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"avoid\"> </a>\n");
 printf("<h4> AVOID [flag] </h4>\n");
 printf("\n");
 printf("AVOID 1 instructs the unit to avoid combat wherever possible.  The unit will\n");
 printf("not enter combat unless it issues an ATTACK order, or the unit's faction\n");
 printf("is attacked in the unit's hex. AVOID 0 cancels this. <p>\n");
 printf("\n");
 printf("The Guard and Avoid Combat flags are mutually exclusive; setting one\n");
 printf("automatically cancels the other. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Set the unit to avoid combat when possible. <p>\n");
 printf("<pre>\n");
   printf("  AVOID 1\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"behind\"> </a>\n");
 printf("<h4> BEHIND [flag] </h4>\n");
 printf("\n");
 printf("BEHIND 1 sets the unit to be behind other units in combat.  BEHIND 0 cancels\n");
 printf("this. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Set the unit to be in front in combat. <p>\n");
 printf("<pre>\n");
   printf("  BEHIND 0\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"build\"> </a>\n");
 printf("<h4> BUILD </h4>\n");
 printf("<h4> BUILD [object type] </h4>\n");
 printf("\n");
 printf("BUILD given with no parameters causes the unit to perform work on the object\n");
 printf("that it is currently inside.  BUILD given with an [object type]\n");
 printf("(such as \"Tower\"\n");
 printf("or \"Galleon\") instructs the unit to begin work on a new object of the type\n");
 printf("given. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("To build a new tower. <p>\n");
 printf("<pre>\n");
   printf("  BUILD Tower\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"buy\"> </a>\n");
 printf("<h4> BUY [quantity] [item] </h4>\n");
 printf("\n");
 printf("Attempt to buy a number of the given item from a city or town\n");
 printf("marketplace, or to buy new people in any region where people are\n");
 printf("available for recruiting.  If the unit can't afford as many as [quantity],\n");
 printf("it will attempt to buy as many as it can.  If the demand for the item\n");
 printf("(from all units in the region) is greater than the number available, the\n");
 printf("available items will be split among the buyers in proportion to the amount\n");
 printf("each buyer attempted to buy.\n");
 if (Globals->RACES_EXIST)
  {
   printf("  When buying people, specify the race of the people as the [item]. \n");
  }
 else
  {
  /* TODO else what? */
  }
 printf("\n");
 printf("<p>\n");
 printf("\n");
 printf("Examples: <p>\n");
 printf("Buy one plate armor from the city market. <p>\n");
 printf("<pre>\n");
   printf("  BUY 1 \"Plate Armor\"\n");
 printf("</pre> <p>\n");
 if (Globals->RACES_EXIST)
  {
   printf("  Recruit 5 barbarians into the current unit. (This will dilute the skills that\n");
   printf("  the unit has.) <p>\n");
   printf("  <pre>\n");
   printf("  BUY 5 barbarians\n");
   printf("  </pre> <p>\n");
  }
 else
  {
  /* TODO else what? */
  }
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"cast\"> </a>\n");
 printf("<h4> CAST [skill] [arguments] </h4>\n");
 printf("\n");
 printf("Cast the given spell.  Note that most spell names contain\n");
 printf("spaces; be sure to enclose the name in quotes!  [arguments] depend\n");
 printf("on which spell you are casting; when you are able to cast a spell,\n");
 printf("the skill description will tell you the syntax. <p>\n");
 printf("\n");
 printf("Examples: <p>\n");
 printf("Cast the spell called \"Super Spell\". <p>\n");
 printf("<pre>\n");
   printf("  CAST \"Super Spell\"\n");
 printf("</pre> <p>\n");
 printf("Cast the fourth-level spell in the \"Super Magic\" skill. <p>\n");
 printf("<pre>\n");
   printf("  CAST \"Super Magic\" 4\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"claim\"> </a>\n");
 printf("<h4> CLAIM [amount] </h4>\n");
 printf("\n");
 printf("Claim an amount of the faction's unclaimed %s, and give it to the unit\n",silver);
 printf("issuing the order.  The claiming unit may then spend the %s or give\n",silver);
 printf("it to another unit. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Claim 100 %s. <p>\n",silver);
 printf("<pre>\n");
   printf("  CLAIM 100\n");
 printf("</pre> <p>\n");
   printf("  \n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"combat\"> </a>\n");
 printf("<h4> COMBAT [spell] </h4>\n");
 printf("\n");
 printf("Set the given spell as the spell that the unit will cast in combat.  This \n");
 printf("order may only be given if the unit can cast the spell in question. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Instruct the unit to use the spell \"Super Spell\", when the unit is involved in\n");
 printf("a battle. <p>\n");
 printf("<pre>\n");
   printf("  COMBAT \"Super Spell\"\n");
 printf("</pre> <p>\n");
 printf("\n");
 if (Globals->FOOD_ITEMS_EXIST)
  {
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <a name=\"consume\"></a>\n");
   printf("  <h4>CONSUME UNIT</h4>\n");
   printf("  <h4>CONSUME FACTION</h4>\n");
   printf("  <h4>CONSUME</h4>\n");
 printf("\n");
   printf("  The CONSUME order instructs the unit to use food items in preference\n");
   printf("  to %s for maintenance costs. CONSUME UNIT tells the unit to use\n",silver);
   printf("  food items that are in that unit's possession before using %s.\n",silver);
   printf("  CONSUME FACTION tells the unit to use any food items that the faction\n");
   printf("  owns (in the same region as the unit) before using %s. CONSUME\n",silver);
   printf("  tells the unit to use %s before food items (this is the default). <p>\n",silver);
 printf("\n");
   printf("  Example: <p>\n");
   printf("  Tell a unit to use food items in the unit's possession for maintenance\n");
   printf("  costs. <p>\n");
   printf("  <pre>\n");
   printf("  CONSUME UNIT\n");
   printf("  </pre>\n");
  }
 printf("<p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"declare\"> </a>\n");
 printf("<h4> DECLARE [faction] [attitude] </h4>\n");
 printf("<h4> DECLARE [faction] </h4>\n");
 printf("<h4> DECLARE DEFAULT [attitude] </h4>\n");
 printf("\n");
 printf("The first form of the DECLARE order sets the attitude of your faction towards\n");
 printf("the given faction.  The second form cancels any attitude towards the given\n");
 printf("faction (so your faction's attitude towards that faction will be its default\n");
 printf("attitude).  The third form sets your faction's default attitude. <p>\n");
 printf("\n");
 printf("Examples: <p>\n");
 printf("Declare your faction to be hostile to faction 15. <p>\n");
 printf("<pre>\n");
   printf("  DECLARE 15 hostile\n");
 printf("</pre> <p>\n");
 printf("Set your faction's attitude to faction 15 to its default attitude. <p>\n");
 printf("<pre>\n");
   printf("  DECLARE 15\n");
 printf("</pre> <p>\n");
 printf("Set your faction's default attitude to friendly. <p>\n");
 printf("<pre>\n");
   printf("  DECLARE DEFAULT friendly\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"describe\"> </a>\n");
 printf("<h4> DESCRIBE UNIT [new description] </h4>\n");
 printf("<h4> DESCRIBE SHIP [new description] </h4>\n");
 printf("<h4> DESCRIBE BUILDING [new description] </h4>\n");
 printf("\n");
 printf("Change the description of the unit, or of the object the unit is in (of which\n");
 printf("the unit must be the owner).  Descriptions can be of any length, up to the line\n");
 printf("length your mailer can handle.  If no description is given, the description\n");
 printf("will be cleared out. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Set the unit,s description to read \"Merlin's helper\". <p>\n");
 printf("<pre>\n");
   printf("  DESCRIBE UNIT \"Merlin's helper\"\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"destroy\"> </a>\n");
 printf("<h4> DESTROY </h4>\n");
 printf("\n");
 printf("Destroy the object you are in (of which you must be the owner).  The order\n");
 printf("cannot be used at sea. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Destroy the ship or building the unit is in. <p>\n");
 printf("<pre>\n");
   printf("  DESTROY\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"enter\"> </a>\n");
 printf("<h4> ENTER [object] </h4>\n");
 printf("\n");
 printf("Attempt to enter the specified object.  If issued from inside another object,\n");
 printf("the unit will first leave the object it is currently in.  The order will only\n");
 printf("work if the target object is unoccupied, or is owned by a unit in your faction,\n");
 printf("or is owned by a unit which has declared you Friendly. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Enter ship number 114. <p>\n");
 printf("<pre>\n");
   printf("  ENTER 114\n");
 printf("</pre> <p>\n");
 printf("\n");
 if (SKILL_ENABLED(S_ENTERTAINMENT))
  {
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <a name=\"entertain\"> </a>\n");
   printf("  <h4> ENTERTAIN </h4>\n");
 printf("\n");
   printf("  Spend the month entertaining the populace to earn money. <p>\n");
 printf("\n");
   printf("  Example: <p>\n");
   printf("  <pre>\n");
   printf("  ENTERTAIN\n");
   printf("  </pre> <p>\n");
  }
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"faction\"> </a>\n");
 printf("<h4> FACTION [type] [points]... </h4>\n");
 printf("\n");
 printf("Attempt to change your faction's type.  In the order, you can specify\n");
 printf("up to three faction types (WAR, TRADE, and MAGIC) and the number of\n");
 printf("faction points to assign to each type; if you are assigning points\n");
 printf("to only one or two types, you may omit the types that will not have\n");
 printf("any points. <p>\n");
 printf("\n");
 printf("Changing the number of faction points assigned to MAGIC may be tricky.\n");
 printf("Increasing the MAGIC points will always succeed, but if you decrease\n");
 printf("the number of points assigned to MAGIC, you must make sure that you\n");
 printf("have only the number of magic-skilled leaders allowed by the new\n");
 printf("number of MAGIC points BEFORE you change your point distribution.\n");
 printf("For example, if you have 3 mages (3 points assigned to MAGIC), but\n");
 printf("want to use one of those points for WAR or TRADE (change to MAGIC 2),\n");
 printf("you must first get rid of one of your mages by either giving it to\n");
 printf("another faction or ordering it to\n");
 printf("<a href=\"#forget\"> FORGET </a> all its magic skills.\n");
 printf("If you have too many mages for the number of points you try to assign\n");
 printf("to MAGIC, the FACTION order will fail. <p>\n");
 printf("\n");
 printf("Examples: <p>\n");
 printf("Assign 2 faction points to WAR, 2 to TRADE, and 1 to MAGIC. <p>\n");
 printf("<pre>\n");
   printf("  FACTION WAR 2 TRADE 2 MAGIC 1\n");
 printf("</pre> <p>\n");
 printf("Become a pure magic faction (assign all points to magic). <p>\n");
 printf("<pre>\n");
   printf("  FACTION MAGIC 5\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"find\"> </a>\n");
 printf("<h4> FIND [faction] </h4>\n");
 printf("<h4> FIND ALL </h4>\n");
 printf("\n");
 printf("Find the email address of the specified faction or of all factions. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Find the email address of faction 4. <p>\n");
 printf("<pre>\n");
   printf("  FIND 4\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"forget\"> </a>\n");
 printf("<h4> FORGET [skill] </h4>\n");
 printf("\n");
 printf("Forget the given skill. This order is useful for normal\n");
 printf("units who wish to learn a new skill, but already know a different skill.\n");
 printf("<p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Forget knowledge of Mining. <p>\n");
 printf("<pre>\n");
   printf("  FORGET Mining\n");
 printf("</pre> <p>\n");
   printf("  \n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>  \n");
 printf("<a name=\"form\"> </a>\n");
 printf("<h4> FORM [alias] </h4>\n");
 printf("\n");
 printf("Form a new unit.  The newly created unit will be in your faction, in the same\n");
 printf("region as the unit which formed it, and in the same structure if any.  It will\n");
 printf("start off, however, with no people or items; you should, in the same month,\n");
 printf("issue orders to transfer people into the new unit, or have it recruit members.\n");
 printf("The new unit will inherit its flags from the unit that forms it,\n");
 printf("such as avoiding, behind, and autotax.\n");
 printf("<p>\n");
 printf("\n");
 printf("The FORM order is followed by a list of orders for the newly created unit. \n");
 printf("This list is terminated by the END keyword, after which orders for the original\n");
 printf("unit resume. <p>\n");
 printf("\n");
 printf("The purpose of the \"alias\" parameter is so that you can refer to the new unit. \n");
 printf("You will not know the new unit's number until you receive the next turn\n");
 printf("report.  To refer to the new unit in this set of orders, pick an alias\n");
 printf("number (the only restriction on this is that it\n");
 printf("must be at least 1, and you should not create two units in the same region in\n");
 printf("the same month, with the same alias numbers).  The new unit can then be\n");
 printf("referred to as NEW <alias> in place of the regular unit number, e.g. <p>\n");
 printf("<pre>\n");
   printf("  UNIT 17\n");
   printf("  FORM 1\n");
     printf("    NAME UNIT \"Merlin's Guards\"\n");
     printf("    BUY 5 Plainsmen\n");
     printf("    STUDY COMBAT\n");
   printf("  END\n");
   printf("  FORM 2\n");
     printf("    NAME UNIT \"Merlin's Workers\"\n");
     printf("    DESCRIBE UNIT \"wearing dirty overalls\"\n");
     printf("    BUY 15 Plainsmen\n");
   printf("  END\n");
   printf("  CLAIM 2500\n");
   printf("  GIVE NEW 1 1000 %s\n",silver);
   printf("  GIVE NEW 2 2000 %s\n",silver);
 printf("</pre> <p>\n");
 printf("This set of orders for unit 17 would create two new units with alias numbers 1\n");
 printf("and 2, name them Merlin's Guards and Merlin's Workers, set the description for\n");
 printf("Merlin's Workers, have both units recruit men, and have Merlin's Guards study\n");
 printf("combat.  Merlin's Workers will have the default order\n");
 printf("<a href=\"#work\"> WORK</a>, as all newly\n");
 printf("created units do.  The unit that created these two then pays them enough money\n");
 printf("(using the NEW keyword to refer to them by alias numbers) to cover the costs of\n");
 printf("recruitment and the month's maintenance. <p>\n");
 printf("\n");
 printf("You can refer to newly created units belonging to other factions, if you know\n");
 printf("what alias number they are, e.g. FACTION 15 NEW 2 will refer to faction 15's\n");
 printf("newly created unit with alias 2. <p>\n");
 printf("\n");
 printf("Note: If a unit moves out of the region in which it was formed\n");
 printf("(by the <a href=\"#move\">MOVE</a> order, or otherwise), the alias\n");
 printf("will no longer work. This is to prevent conflicts with other units\n");
 printf("that may have the same alias in other regions. <p>\n");
 printf("\n");
 printf("If the demand for recruits in that region that month is much higher than the\n");
 printf("supply, it may happen that the new unit does not gain all the recruits\n");
 printf("you ordered it to buy, or it may not gain any recruits at all.  If the\n");
 printf("new units gains at least one recruit, the unit will form possessing\n");
 printf("any unused %s and all the other items it was given.  If no recruits\n",silver);
 printf("are gained at all, the empty unit will be dissolved, and the %s and\n",silver);
 printf("any other items it was given will revert to the lowest numbered unit you\n");
 printf("have in that region. <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"give\"> </a>\n");
 printf("<h4> GIVE [unit] [quantity] [item] </h4>\n");
 printf("<h4> GIVE [unit] ALL [item] </h4>\n");
 printf("<h4> GIVE [unit] ALL [item] EXCEPT [quantity] </h4>\n");
 printf("<h4> GIVE [unit] UNIT </h4>\n");
 printf("\n");
 printf("The first form of the GIVE order gives a quantity of an item to another unit.\n");
 printf("The second form of the GIVE order will give all of a given item to another\n");
 printf("unit.  The third form will give all of an item except for a specific quantity\n");
 printf("to another unit.  The fourth and final form the the GIVE order gives the\n");
 printf("entire unit to the specified unit's faction. <p>\n");
 printf("\n");
 printf("A unit may only give items, including %s, to a unit which it is able to\n",silver);
 printf("see, unless the faction of the target unit has declared you Friendly or better.\n");
  printf(" \n");
 printf("If the target unit is not a member of your faction, then its faction must have\n");
 printf("declared you Friendly, with a couple of exceptions. First, %s may be given\n",silver);
 printf("to any unit, regardless of factional affiliation. Secondly, men may not \n");
 printf("be given to units in other factions (you must give the entire unit);\n");
 printf("the reason for this is to prevent highly skilled units from being\n");
 printf("sabotaged with a <a href=\"#give\">GIVE</a> order. <p>\n");
 printf("\n");
 printf("There are also a few restrictions on orders given by units who been\n");
 printf("given to another faction. If the receiving faction is not allied to\n");
 printf("the giving faction, the unit may not issue the \n");
 printf("<a href=\"#advance\">ADVANCE</a> order, or issue any more \n");
 printf("<a href=\"#give\">GIVE</a> orders. Both of these rules are to prevent\n");
 printf("unfair sabotage tactics. <p>\n");
 printf("\n");
 printf("If 0 is specified as the unit number, then the items\n");
 printf("are discarded. <p>\n");
 printf("\n");
 printf("Examples: <p>\n");
 printf("Give 10 swords to unit 4573. <p>\n");
 printf("<pre>\n");
   printf("  GIVE 4573 10 swords\n");
 printf("</pre> <p>\n");
 printf("Give 5 chain armor to the new unit, alias 2, belonging to faction 14. <p>\n");
 printf("<pre>\n");
   printf("  GIVE FACTION 14 NEW 2 5 \"Chain armor\"\n");
 printf("</pre> <p>\n");
 printf("Give control of this unit to the faction owning unit 75. <p>\n");
 printf("<pre>\n");
   printf("  GIVE 75 UNIT\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"guard\"> </a>\n");
 printf("<h4> GUARD [flag] </h4>\n");
 printf("\n");
 printf("GUARD 1 sets the unit issuing the order to prevent non-Friendly units from\n");
 printf("collecting taxes in the region, and to prevent any units not your own from\n");
 printf("pillaging the region.  Guarding units will also attempt to prevent Unfriendly\n");
 printf("units from entering the region.  GUARD 0 cancels Guard status. <p>\n");
 printf("\n");
 printf("The Guard and Avoid Combat flags are mutually exclusive; setting one\n");
 printf("automatically cancels the other. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Instruct the current unit to be on guard. <p>\n");
 printf("<pre>\n");
   printf("  GUARD 1\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"hold\"> </a>\n");
 printf("<h4> HOLD [flag] </h4>\n");
 printf("\n");
 printf("HOLD 1 instructs the issuing unit to never join a battle in regions the unit is\n");
 printf("not it.  This can be useful if the unit is in a building, and doesn't want to\n");
 printf("leave the building to join combat.  HOLD 0 cancels holding status. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Instruct the unit to avoid combat in other regions. <p>\n");
 printf("<pre>\n");
   printf("  HOLD 1\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"leave\"> </a>\n");
 printf("<h4> LEAVE </h4>\n");
 printf("\n");
 printf("Leave the object you are currently in.  The order cannot be used at sea.\n");
 printf("<p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("<pre>\n");
 printf("LEAVE\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"move\"> </a>\n");
 printf("<h4> MOVE [dir] ... </h4>\n");
 printf("\n");
 printf("Attempt to move in the direction(s) specified.  If more than one direction is\n");
 printf("given, the unit will move multiple times, in the order specified by the MOVE\n");
 printf("order, until no more directions are given, or until one of the moves fails.  A\n");
 printf("move can fail because the units runs out of movement points, because the unit\n");
 printf("attempts to move into the ocean, or because the units attempts to enter a\n");
 printf("structure, and is rejected. <p>\n");
 printf("\n");
 printf("Valid directions are: <p>\n");
 printf("1) The compass directions North, Northwest, Southwest, South, Southeast, and\n");
 printf("Northeast.  These can be abbreviated N, NW, SW, S, SE, NE. <p>\n");
 printf("2) A structure number. <p>\n");
 printf("3) OUT, which will leave the structure that the unit is in. <p>\n");
 printf("4) IN, which will move through an inner passage in the structure that the unit\n");
 printf("is currently in. <p>\n");
 printf("\n");
 printf("Multiple MOVE orders given by one unit will chain together, so: <p>\n");
 printf("<pre>\n");
   printf("  MOVE N\n");
   printf("  MOVE NE IN \n");
 printf("</pre> <p>\n");
 printf("is equivalent to: <p>\n");
 printf("<pre>\n");
   printf("  MOVE N NE IN\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("Note that MOVE orders can lead to combat, due to hostile units meeting, or due\n");
 printf("to an advancing unit being forbidden access to a region.  In this case, combat\n");
 printf("occurs each time all movement out of a single region occurs. <p>\n");
 printf("\n");
 printf("Example 1: Units 1 and 2 are in Region A, and unit 3 is in Region B.  Units 1\n");
 printf("and 2 are hostile to unit 3.  Both unit 1 and 2 move into region B, and attack\n");
 printf("unit 3.  Since both units moved out of the same region, they attack unit 3 at\n");
 printf("the same time, and the battle is between units 1 and 2, and unit 3. <p>\n");
 printf("\n");
 printf("Example 2: Same as example 1, except unit 2 is in Region C, instead of region\n");
 printf("A.  Both units move into Region B, and attack unit 3.  Since unit 1 and unit 2\n");
 printf("moved out of different regions, their battles occur at different times.  Thus,\n");
 printf("unit 1 attacks unit 3 first, and then unit 2 attacks unit 3 (assuming unit 3\n");
 printf("survives the first attack).  Note that the order of battles could have happened\n");
 printf("either way. <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"name\"> </a>\n");
 printf("<h4> NAME UNIT [new name] </h4>\n");
 printf("<h4> NAME FACTION [new name] </h4>\n");
 printf("<h4> NAME OBJECT [new name] </h4>\n");
 if (Globals->TOWNS_EXIST)
  {
 printf("<h4> NAME CITY [new name] </h4>\n");
  }
 printf("\n");
 printf("Change the name of the unit, or of your faction, or of the object the unit is\n");
 printf("in (of which the unit must be the owner).  Names can be of any length, up to\n");
 printf("the line length your mailer can handle.  Names may not contain parentheses\n");
 printf("(square brackets can be used instead if necessary), or any control characters.\n");
 printf("<p>\n");
 if (Globals->TOWNS_EXIST)
  {
   printf("  In order to rename a settlement (city, town or village), the unit attempting\n");
   printf("  to rename it must be the owner of a large enough structure located in the\n");
   printf("  city.   It requires a tower or better to rename a village, a fort or better\n");
   printf("  to rename a town and a castle or mystic fortress to rename a city.\n");
  if (Globals->CITY_RENAME_COST)
   {
   int c=Globals->CITY_RENAME_COST;
    printf("   It also costs $%d to rename village,\n",c);
    printf("   $%d to rename town and\n",2*c);
    printf("   $%d to rename city.\n",3*c);
   }
 printf("<p>\n");
  }
 printf("\n");
 printf("Example: <p>\n");
 printf("Name your faction \"The Merry Pranksters\". <p>\n");
 printf("<pre>\n");
   printf("  NAME FACTION \"The Merry Pranksters\"\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"noaid\"> </a>\n");
 printf("<h4> NOAID [flag] </h4>\n");
 printf("\n");
 printf("NOAID 1 indicates that if the unit attacks, or is attacked, \n");
 printf("it is not to be aided\n");
 printf("by units in other hexes. NOAID status is very useful for scouts or\n");
 printf("probing units, who do not wish to drag their nearby armies into battle\n");
 printf("if they are caught. NOAID 0 cancels this. <p>\n");
 printf("\n");
 printf("If multiple units are on one side in a battle, they must all have the\n");
 printf("NOAID flag on, or they will receive aid from other hexes. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Set a unit to receive no aid in battle. <p>\n");
 printf("<pre>\n");
   printf("  NOAID 1\n");
 printf("</pre> <p>\n");
 printf("\n");
 if (Globals->FLIGHT_OVER_WATER!=GameDefs::WFLIGHT_NONE)
  {
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <a name=\"nocross\"> </a>\n");
   printf("  <h4> NOCROSS [flag] </h4>\n");
 printf("\n");
   printf("  NOCROSS 1 indicates that if a unit attempts to cross a body of water\n");
   printf("  then that unit should instead not cross it, regardless of whether the\n");
   printf("  unit otherwise could do so. \n");
  if (SKILL_ENABLED(S_SAILING))
   {
    printf("   Units inside of a ship are not affected\n");
    printf("   by this flag (IE, they are able to sail within the ship).   \n");
   }
   printf("  NOCROSS 0 cancels this.<p>\n");
 printf("\n");
   printf("  Example:<p>\n");
   printf("  Set a unit to not permit itself to cross water. <p>\n");
   printf("  <pre>\n");
   printf("  NOCROSS 1\n");
   printf("  </pre> <p>\n");
  }
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"nospoils\"></a>\n");
 printf("<h4> NOSPOILS [flag] </h4>\n");
 printf("NOSPOILS 1 indeicates that a unit should not stop to collect spoils after a \n");
 printf("battle.  The unit will not be given any spoils which have weight. NOSPOILS 0\n");
 printf("cancels this.<p>\n");
 printf("\n");
 printf("Example:<p>\n");
 printf("Set a unit to not recieve battle spoils.<p>\n");
 printf("<pre>\n");
    printf("   NOSPOILS 1\n");
 printf("</pre>\n");
 printf("<p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"option\"> </a>\n");
 printf("<h4> OPTION TIMES </h4>\n");
 printf("<h4> OPTION NOTIMES </h4>\n");
 printf("<h4> OPTION TEMPLATE OFF </h4>\n");
 printf("<h4> OPTION TEMPLATE SHORT </h4>\n");
 printf("<h4> OPTION TEMPLATE LONG </h4>\n");
 printf("<h4> OPTION TEMPLATE MAP </h4>\n");
 printf("\n");
 printf("The OPTION order is used to toggle various settings that affect your\n");
 printf("reports, and other email details.\n");
 printf("OPTION TIMES sets it so that your faction receives the times\n");
 printf("each week (this is the default); OPTION NOTIMES sets it so that your faction\n");
 printf("is not sent the times. <p>\n");
 printf("\n");
 printf("The OPTION TEMPLATE order toggles the length of the Orders Template\n");
 printf("that appears at the bottom of a turn report.  The OFF setting eliminates\n");
 printf("the Template altogether, and the SHORT, LONG and MAP settings control how\n");
 printf("much detail the Template contains.  The MAP setting will produce an ascii\n");
 printf("map of the region and surrounding regions in addition other details.<p>\n");
 printf("\n");
 printf("For the MAP template, the region identifiers are: (use a fixed width font)<p>\n");
 printf("<table>\n");
   printf("  <tr><td align=left>####</td><td>BLOCKED HEX (Underworld)</td></tr>\n");
   printf("  <tr><td align=left>~~~~</td><td>OCEAN HEX</td></tr>\n");
   printf("  <tr><td align=left>    </td><td>PLAINS/TUNNELS HEX</td></tr>\n");
   printf("  <tr><td align=left>^^^^</td><td>FOREST/UNDERFOREST HEX</td></tr>\n");
   printf("  <tr><td align=left>/\\/\\</td><td>MOUNTAIN HEX</td></tr>\n");
   printf("  <tr><td align=left>vvvv</td><td>SWAMP HEX</td></tr>\n");
   printf("  <tr><td align=left>@@@@</td><td>JUNGLE HEX</td></tr>\n");
   printf("  <tr><td align=left>....</td><td>DESERT/CAVERN HEX</td></tr>\n");
   printf("  <tr><td align=left>,,,,</td><td>TUNDRA HEX</td></tr>\n");
   printf("  <tr><td align=left>!!!!</td><td>THE NEXUS</td></tr>\n");
 printf("</table>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"password\"> </a>\n");
 printf("<h4> PASSWORD [password] </h4>\n");
 printf("<h4> PASSWORD </h4>\n");
 printf("\n");
 printf("The PASSWORD order is used to set your faction's password.  If you have a\n");
 printf("password set, you must specify it on your #ATLANTIS line for the game to\n");
 printf("accept your orders.  This protects you orders from being overwritten, either\n");
 printf("by accident or intentionally by other players.  PASSWORD with no password\n");
 printf("given clears out your faction's password. <p>\n");
 printf("\n");
 printf("IMPORTANT: The PASSWORD order does not take effect until the turn is actually\n");
 printf("run.  So if you set your password, and then want to re-submit orders, you\n");
 printf("should use the old password until the turn has been run. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Set the password to \"xyzzy\". <p>\n");
 printf("<pre>\n");
   printf("  PASSWORD xyzzy\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"pillage\"> </a>\n");
 printf("<h4> PILLAGE </h4>\n");
 printf("\n");
 printf("Use force to extort as much money as possible from the region. Note that\n");
 printf("the <a href=\"#tax\">TAX</a> order and PILLAGE order are mutually\n");
 printf("exclusive; a unit may only attempt to do one in a turn.<p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"produce\"> </a>\n");
 printf("<h4> PRODUCE [item] </h4>\n");
 printf("\n");
 printf("Spend the month producing as much as possible of the specified item. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Produce as many crossbows as possible. <p>\n");
 printf("<pre>\n");
   printf("  PRODUCE crossbows\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"promote\"> </a>\n");
 printf("<h4> PROMOTE [unit] </h4>\n");
 printf("\n");
 printf("Promote the specified unit to owner of the object of which you are currently\n");
 printf("the owner.  The target unit must have declared you Friendly. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Promote unit 415 to be the owner of the object that this unit owns. <p>\n");
 printf("<pre>\n");
   printf("  PROMOTE 415\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"quit\"> </a>\n");
 printf("<h4> QUIT [password] </h4>\n");
 printf("\n");
 printf("Quit the game.  On issuing this order, your faction will be completely and\n");
 printf("permanently destroyed. Note that you must give your password for the quit\n");
 printf("order to work; this is to provide some safety against accidentally issuing\n");
 printf("this order. <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"restart\"></a>\n");
 printf("<h4> RESTART [password] </h4>\n");
 printf("\n");
 printf("Similar to the <a href=\"#quit\">QUIT</a> order, this order will completely\n");
 printf("and permanently destroy your faction. However, it will begin a brand new\n");
 printf("faction for you (you will get a separate turn report for the new\n");
 printf("faction). Note that you must give your password for this order to work,\n");
 printf("to provide some protection against accidentally issuing this order. <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"reveal\"> </a>\n");
 printf("<h4> REVEAL </h4>\n");
 printf("<h4> REVEAL UNIT </h4>\n");
 printf("<h4> REVEAL FACTION </h4>\n");
 printf("\n");
 printf("Cause the unit to either show itself (REVEAL UNIT), or show itself and its\n");
 printf("faction affiliation (REVEAL FACTION), in the turn report, to all other factions\n");
 printf("in the region.  \n");
 if (st_ena)
  {
   printf("  Used to reveal high stealth scouts, should there be some reason to.  \n");
  }
 printf("REVEAL is used to cancel this. <p>\n");
 printf("\n");
 printf("Examples: <p>\n");
 printf("Show the unit to all factions. <p>\n");
 printf("<pre>\n");
   printf("  REVEAL UNIT \n");
 printf("</pre> <p>\n");
 printf("Show the unit and it's affiliation to all factions. <p>\n");
 printf("<pre>\n");
   printf("  REVEAL FACTION\n");
 printf("</pre> <p>\n");
 printf("Cancels revealling. <p>\n");
 printf("<pre>\n");
   printf("  REVEAL\n");
 printf("</pre> <p>\n");
 printf("\n");
 if (SKILL_ENABLED(S_SAILING))
  {
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <a name=\"sail\"> </a>\n");
   printf("  <h4> SAIL [dir] ... </h4>\n");
   printf("  <h4> SAIL </h4>\n");
   printf("  \n");
   printf("  The first form will sail the ship, which the unit must be the owner of, in the\n");
   printf("  directions given.  The second form will cause the unit to aid in the sailing\n");
   printf("  of the ship, using the Sailing skill.  See the section on movement for more\n");
   printf("  information on the mechanics of sailing. <p>\n");
   printf("  \n");
   printf("  Example: <p>\n");
   printf("  Sail north, then northwest. <p>\n");
   printf("  <pre>\n");
    printf("   SAIL N NW\n");
   printf("  </pre> <p>\n");
   printf("  or: <p>\n");
   printf("  <pre>\n");
   printf("  SAIL N\n");
   printf("  SAIL NW\n");
   printf("  </pre> <p>\n");
 printf("\n");
  }
   printf("  \n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"sell\"> </a>\n");
 printf("<h4> SELL [quantity] [item] </h4>\n");
 printf("\n");
 printf("Attempt to sell the amount given of the item given.  If the unit does not have\n");
 printf("as many of the item as it is trying to sell, it will attempt to sell all that\n");
 printf("it has.  If more of the item are on sale (by all the units in the region) than\n");
 printf("are wanted by the region, the number sold per unit will be split up in\n");
 printf("proportion to the number each unit tried to sell. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Sell 10 furs to the market. <p>\n");
 printf("<pre>\n");
   printf("  SELL 10 furs\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"show\"> </a>\n");
 printf("<h4> SHOW SKILL [skill] [level] </h4>\n");
 printf("<h4> SHOW ITEM [item] </h4>\n");
 printf("\n");
 printf("The first form of the order shows the skill description for a skill\n");
 printf("that your faction already possesses. The second form returns some\n");
 printf("information about an item that is not otherwise apparent on a\n");
 printf("report, such as the weight. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Show the skill report for Mining 3 again. <p>\n");
 printf("<pre>\n");
   printf("  SHOW SKILL Mining 3\n");
 printf("</pre> <p>\n");
 printf("Get some information about Iron. <p>\n");
 printf("<pre>\n");
   printf("  SHOW ITEM Iron\n");
 printf("</pre> <p>\n");
 printf("\n");
 if (st_ena)
  {
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <a name=\"steal\"> </a>\n");
   printf("  <h4> STEAL [unit] [item] </h4>\n");
 printf("\n");
   printf("  Attempt to steal as much as possible of the specified item from the specified\n");
   printf("  unit.  The order may only be issued by a one-man unit. <p>\n");
 printf("\n");
   printf("  A unit may only attempt to steal from a unit which is able to be seen.<p>\n");
 printf("\n");
   printf("  Example: <p>\n");
   printf("  Steal %s from unit 123. <p>\n",silver);
   printf("  <pre>\n");
   printf("  STEAL 123 SILVER\n");
   printf("  </pre> <p>\n");
   printf("  Steal wood from unit 321. <p>\n");
   printf("  <pre>\n");
   printf("  STEAL 321 wood\n");
   printf("  </pre> <p>\n");
  }
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"study\"> </a>\n");
 printf("<h4> STUDY [skill] </h4>\n");
 printf("\n");
 printf("Spend the month studying the specified skill. <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Study horse training. <p>\n");
 printf("<pre>\n");
   printf("  STUDY \"Horse Training\"\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"tax\"> </a>\n");
 printf("<h4> TAX </h4>\n");
 printf("\n");
 printf("Attempt to collect taxes from the region.  Only War factions may collect taxes,\n");
 printf("and then only if there are no non-Friendly units on guard. Only\n");
 printf("combat-ready units may issue this order. Note that the TAX order and the\n");
 printf("<a href=\"#pillage\">PILLAGE</a> order are mutually exclusive; a unit\n");
 printf("may only attempt to do one in a turn.<p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Attempt to collect taxes. <p>\n");
 printf("<pre>\n");
   printf("  TAX\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"teach\"> </a>\n");
 printf("<h4> TEACH [unit] ... </h4>\n");
 printf("\n");
 printf("Attempt to teach the specified units whatever skill they are studying that\n");
 printf("month.  A list of several units may be specified.  All units to be taught must\n");
 printf("have declared you Friendly.  Subsequent TEACH orders can be used to add units\n");
 printf("to be taught.  Thus: <p>\n");
 printf("<pre>\n");
   printf("  TEACH 1\n");
   printf("  TEACH 2\n");
 printf("</pre> <p>\n");
 printf("is equivalent to <p>\n");
 printf("<pre>\n");
   printf("  TEACH 1 2\n");
 printf("</pre> <p>\n");
 printf("\n");
 printf("Example: <p>\n");
 printf("Teach new unit 2 and unit 5 whatever they are studying. <p>\n");
 printf("<pre>\n");
   printf("  TEACH NEW 2 5\n");
 printf("</pre> <p>\n");
 printf("\n");
 if (Globals->ALLOW_WITHDRAW)
  {
 printf("\n");
   printf("  <center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
   printf("  <a name=\"withdraw\"> </a>\n");
   printf("  <h4> WITHDRAW [item]\n");
   printf("  <h4> WITHDRAW [quantity] [item]</h4>\n");
 printf("\n");
   printf("  Use unclaimed funds to aquire basic items that you need.  If you do not\n");
   printf("  have sufficient unclaimed, or if you try withdraw any other than a\n");
   printf("  non-basic item, an error will be given.   Withdraw can NOT be used in the\n");
   printf("  Nexus (to prevent building towers and such there).  The first form is\n");
   printf("  the same as WITHDRAW 1 [item] in the second form.<p>\n");
 printf("\n");
   printf("  Examples: <p>\n");
   printf("  Withdraw 5 stone.<p>\n");
   printf("  <pre>\n");
    printf("   WITHDRAW 5 STON\n");
   printf("  </pre>\n");
   printf("  Withdraw 1 iron.<p>\n");
   printf("  <pre>\n");
    printf("   WITHDRAW IRON\n");
   printf("  </pre>\n");
 printf("\n");
  }
 printf("\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<a name=\"work\"> </a>\n");
 printf("<h4> WORK </h4>\n");
 printf("\n");
 printf("Spend the month performing manual work for wages. <p>\n");
 printf("\n");
 printf("<a name=\"sequenceofevents\"> </a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Sequence of Events </h2>\n");
 printf("\n");
 printf("Each turn, the following sequence of events occurs: <p>\n");
 printf("\n");
 printf("<ol>\n");
 printf("<li> Instant orders.\n");
 printf("\n");
 printf("<ul>\n");
 printf("<li> <a href=\"#form\">FORM</a> orders are processed.\n");
 printf("<li> <a href=\"#address\">ADDRESS</a>,\n");
 printf("<a href=\"#autotax\">AUTOTAX</a>,\n");
 printf("<a href=\"#avoid\">AVOID</a>,\n");
 printf("<a href=\"#behind\">BEHIND</a>,\n");
 printf("<a href=\"#claim\">CLAIM</a>,\n");
 printf("<a href=\"#combat\">COMBAT</a>,\n");
 if (Globals->FOOD_ITEMS_EXIST)
  {
   printf("  <a href=\"#consume\">CONSUME</a>,\n");
  }
 printf("<a href=\"#declare\">DECLARE</a>,\n");
 printf("<a href=\"#describe\">DESCRIBE</a>,\n");
 printf("<a href=\"#faction\">FACTION</a>,\n");
 printf("<a href=\"#guard\">GUARD</a> 0,\n");
 printf("<a href=\"#hold\">HOLD</a>,\n");
 printf("<a href=\"#name\">NAME</a>,\n");
 printf("<a href=\"#noaid\">NOAID</a>,\n");
 if (Globals->FLIGHT_OVER_WATER!=GameDefs::WFLIGHT_NONE)
  {
   printf("  <a href=\"#nocross\">NOCROSS</a>,\n");
  }
 printf("<a href=\"#nospoils\">NOSPOILS</a>,\n");
 printf("<a href=\"#option\">OPTION</a>,\n");
 printf("<a href=\"#password\">PASSWORD</a>,\n");
 printf("<a href=\"#reveal\">REVEAL</a>, and\n");
 printf("<a href=\"#show\">SHOW</a>\n");
 printf("orders are processed.\n");
 printf("<li> <a href=\"#find\">FIND</a> orders are processed.\n");
 printf("<li> <a href=\"#leave\">LEAVE</a> orders are processed.\n");
 printf("<li> <a href=\"#enter\">ENTER</a> orders are processed.\n");
 printf("<li> <a href=\"#promote\">PROMOTE</a> orders are processed.\n");
 printf("</ul>\n");
 printf("\n");
 printf("<li> Combat is processed.\n");
 printf("\n");
 if (st_ena)
  {
   printf("  <li> Steal orders.\n");
 printf("\n");
   printf("  <ul>\n");
   printf("  <li> <a href=\"#steal\">STEAL</a> and\n");
   printf("  <a href=\"#assassinate\">ASSASSINATE</a> orders are processed.\n");
   printf("  </ul>\n");
  }
 printf("\n");
 printf("<li> Give orders.\n");
 printf("\n");
 printf("<ul>\n");
 printf("<li> <a href=\"#destroy\">DESTROY</a> and\n");
 printf("<a href=\"#give\">GIVE</a> orders are processed.\n");
 printf("</ul>\n");
 printf("\n");
 printf("<li> Tax orders.\n");
 printf("\n");
 printf("<ul>\n");
 printf("<li> <a href=\"#pillage\">PILLAGE</a> orders are processed.\n");
 printf("<li> <a href=\"#tax\">TAX</a> orders are processed.\n");
 printf("</ul>\n");
 printf("\n");
 printf("<li> Instant Magic\n");
 printf("\n");
 printf("<ul>\n");
 printf("<li> Old spells are cancelled.\n");
 printf("<li> Spells are <a href=\"#cast\">CAST</a> (except for Teleportation spells).\n");
 printf("</ul>\n");
 printf("\n");
 printf("<li> Market orders.\n");
 printf("\n");
 printf("<ul>\n");
 printf("<li> <a href=\"#guard\">GUARD</a> 1 orders are processed.\n");
 printf("<li> <a href=\"#sell\">SELL</a> orders are processed.\n");
 printf("<li> <a href=\"#buy\">BUY</a> orders are processed.\n");
 printf("<li> <a href=\"#quit\">QUIT</a> and <a href=\"#restart\">RESTART</a>\n");
 printf("orders are processed.\n");
 printf("<li> <a href=\"#forget\">FORGET</a> orders are processed.\n");
 printf("</ul>\n");
 printf("\n");
 if (Globals->ALLOW_WITHDRAW)
  {
   printf("  <li> Withdraw orders.\n");
   printf("  <ul>\n");
   printf("  <li> <a href=\"#withdraw\">WITHDRAW</a> orders are processed.\n");
   printf("  </ul>\n");
  }
 printf("\n");
 printf("<li> Movement orders.\n");
 printf("\n");
 printf("<ul>\n");
 if (SKILL_ENABLED(S_SAILING))
  {
   printf("  <li> <a href=\"#sail\">SAIL</a> orders are processed.  \n");
  }
 printf("<li> <a href=\"#advance\">ADVANCE</a>\n");
 printf("and <a href=\"#move\">MOVE</a>\n");
 printf("orders are processed (including any combat resulting from\n");
 printf("these orders).\n");
 printf("</ul>\n");
 printf("\n");
 printf("<li> Month long orders.\n");
 printf("\n");
 printf("<ul>\n");
 printf("<li> <a href=\"#build\">BUILD</a>,\n");
 if (SKILL_ENABLED(S_ENTERTAINMENT))
  {
   printf("  <a href=\"#entertain\">ENTERTAIN</a>,\n");
  }
 printf("<a href=\"#produce\">PRODUCE</a>,\n");
 printf("<a href=\"#study\">STUDY</a>,\n");
 printf("<a href=\"#teach\">TEACH</a> and\n");
 printf("<a href=\"#work\">WORK</a> orders are processed.\n");
 printf("<li>\n");
 printf("Costs associated with these orders (such as study fees) are collected.\n");
 printf("</ul>\n");
 printf("<li> Teleportation spells are <a href=\"#cast\">CAST</a>.\n");
 printf("\n");
 printf("<li> Maintenance costs are assessed.\n");
 printf("\n");
 printf("</ol>\n");
 printf("\n");
 printf("Where there is no other basis for deciding in which order units will be\n");
 printf("processed within a phase, units that appear higher on the report get\n");
 printf("precedence. <p>\n");
 printf("\n");
 printf("<a name=\"reportformat\"></a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Report Format </h2>\n");
 printf("\n");
 printf("The most important sections of the turn report are the \"Events During Turn\"\n");
 printf("section which lists what happened last month, and the \"Current Status\" section\n");
 printf("which gives the description of each region in which you have units.\n");
 printf("<p>\n");
 printf("\n");
 printf("Your units in the Current Status section are flagged with a \"*\" character. \n");
 printf("Units belonging to other factions are flagged with a \"-\" character.  You may be\n");
 printf("informed which faction they belong to, if you have high enough Observation\n");
 printf("skill. <p>\n");
 printf("\n");
 printf("Objects are flagged with a \"+\" character.  The units listed under an object (if\n");
 printf("any) are inside the object.  The first unit listed under an object is its\n");
 printf("owner. <p>\n");
 printf("\n");
 printf("If you can see a unit, you can see any large items it is carrying.  This means\n");
 printf("all items other than %s, herbs, and other small items (which are of zero\n",silver);
 printf("size units, and are small enough to be easily concealed).  Items carried by\n");
 printf("your own units of course will always be listed. <p>\n");
 printf("\n");
 printf("At the bottom of your turn report is an Orders Template.  This template\n");
 printf("gives you a formatted orders form, with all of your units listed.  You\n");
 printf("may use this to fill in your orders, or write them on your own. The\n");
 printf("<a href=\"#option\">OPTION</a> order gives you the option of giving\n");
 printf("more or less information in this template, or turning it of altogether.\n");
 printf("You can precede orders with an '@' sign in your orders, in which case\n");
 printf("they will appear in your template on the next turn's report. <p>\n");
 printf("\n");
 printf("<a name=\"hintsfornew\"></a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Hints for New Players </h2>\n");
 printf("\n");
 printf("Make sure to use the correct #ATLANTIS and UNIT lines in your orders.\n");
 printf("<p>\n");
 printf("\n");
 printf("Always have a month's supply of spare cash in every region in which you have\n");
 printf("units, so that even if they are deprived of income for a month (due to a\n");
 printf("mistake in your orders, for example), they will not starve to death.  It is\n");
 printf("very frustrating to have half your faction wiped out because you neglected to\n");
 printf("provide enough money for them to live on. <p>\n");
 printf("\n");
 printf("Be conservative with your money.  Leaders especially are very hard to maintain,\n");
 printf("as they cannot usually earn enough by <a href=\"#work\"> WORK</a>ing\n");
 printf("to pay their maintenance fee.  Even once\n");
 printf("you have recruited men, notice that it is expensive for them to\n");
 printf("<a href=\"#study\"> STUDY </a> (and\n");
 printf("become productive units), so be sure to save money to that end. <p>\n");
 printf("\n");
 printf("Don't leave it until the last minute to send orders.  If there is a delay in\n");
 printf("the mailer, your orders will not arrive on time, and turns will NOT be rerun,\n");
 printf("nor will it be possible to change the data file for the benefit of players\n");
 printf("whose orders weren't there by the deadline.  If you are going to send your\n");
 printf("orders at the last minute, send a preliminary set earlier in the week so that\n");
 printf("at worst your faction will not be left with no orders at all. <p>\n");
 printf("\n");
 printf("<a name=\"specialcommands\"></a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2> Special Commands </h2>\n");
 printf("These special commands have been added via the scripts processing the email\n");
 printf("to help you interact with the game and submit times and rumors.   Please\n");
 printf("read over these new commands and their uses.   Also note that all commands\n");
 printf("sent to the server are logged, including orders submissions, so if you have\n");
 printf("a problem, or if you attempt to abuse the system, it will get noticed and\n");
 printf("it will be tracked down.\n");
 printf("<p>\n");
 printf("\n");
 printf("<a name=\"create\"></a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h4>#Create \"faction name\" \"password\"</h4>\n");
 printf("This will create a new faction (add it to players.in), with the desired name\n");
 printf("and password, and it will use the player's \"from\" address as the email\n");
 printf("address of record (this, of course, can be changed in the game).<br>\n");
 printf("<br>\n");
 printf("The \"\" characters are required.  If they miss some, it'll do interesting\n");
 printf("things to your registration.<br>\n");
 printf("<br>\n");
 printf("So, if you wanted to join the game, you would send an email which only\n");
 printf("contained the message:<br>\n");
 printf("<br>\n");
 printf("#create \"Mighty Ducks\" \"quack!\"\n");
 printf("<br>\n");
 printf("\n");
 printf("<a name=\"resend\"> </a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h4>#Resend [faction] \"password\"</h4>\n");
 printf("The faction number and your current password (if you have one) are required.\n");
 printf("The most recent turn report (that can be found) will be sent.<br>\n");
 printf("<br>\n");
 printf("Please note:  Due to space limitations, only a certain number of turns of\n");
 printf("reports may be kept available.  Thus, the command is limited to the most\n");
 printf("recently run turn.  If the report is available, it'll be sent to your email\n");
 printf("address of record (whatever was set for the most recent turn).<br>\n");
 printf("<br>\n");
 printf("So, if you want a copy of last turn's report (because the cat danced on the\n");
 printf("keyboard and toasted your \"original\"):<br>\n");
 printf("<br>\n");
 printf("#resend 999 \"quack!\"<br>\n");
 printf("<br>\n");
 printf("\n");
 printf("<a name=\"times\"> </a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h4>#Times [faction] \"password\"<br>\n");
 printf("body of times<br>\n");
 printf("#end</h4>\n");
 printf("Everything between the #times and #end lines is included in your article.\n");
 printf("Your article is prefaced with the name and number of your faction, so you\n");
 printf("needn't worry about adding that. \n");
 if (Globals->TIMES_REWARD)
  {
   printf("  And, you'll get $%d for making your submission.<br>\n",Globals->TIMES_REWARD);
  }
 printf("<br>\n");
 printf("So, if you wanted to make a proclaimation, you might submit:<br>\n");
 printf("<br>\n");
 printf("#times 999 \"quack!\"<br>\n");
 printf("The Mighty Ducks declare war on the world!<br>\n");
 printf("Quack!<br>\n");
 printf("#end<br>\n");
 printf("<br>\n");
 printf("And it would appear something like:<br>\n");
 printf("<br>\n");
 printf("---------------------------------<br>\n");
 printf("Mighty Ducks (999) writes:<br>\n");
 printf("<br>\n");
 printf("The Might Ducks declare war on the world!<br>\n");
 printf("Quack!<br>\n");
 printf("<br>\n");
 printf("---------------------------------<br>\n");
 printf("<br>\n");
 printf("\n");
 printf("<a name=\"rumors\"> </a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h4>#Rumor [faction] \"password\"</h4>\n");
 printf("Now, you may be wondering -- won't people know I submitted the item because\n");
 printf("it appears in faction-number order?  Nope -- because it's not done that way.\n");
 printf("The list of rumors is randomized a bit -- so there's no telling who\n");
 printf("submitted which item, in what order.  Just don't depend on specific\n");
 printf("placement of your rumors. =) <br>\n");
 printf("<br>\n");
 printf("So, if you were malicious, you might do:<br>\n");
 printf("<br>\n");
 printf("#rumor 999 \"quack!\" <br>\n");
 printf("Oleg is a fink!<br>\n");
 printf("#end<br>\n");
 printf("<br>\n");
 printf("And it would appear somewhere in the Rumors section of the Times, without\n");
 printf("your faction name or number on it.<br>\n");
 printf("<br>\n");
 printf("\n");
 printf("<a name=\"remind\"> </a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h4>#Remind [faction] \"password\"</h4>\n");
 printf("This special order will have the software find your last set of orders and mail\n");
 printf("them to you.  Note:  don't try to get someone else's orders -- you'll be\n");
 printf("surprised when the person sends you a nasty-gram about the activity.  All\n");
 printf("bad password attempts are forwarded to the faction owner, so beware.<br>\n");
 printf("<br>\n");
 printf("Example:<br>\n");
 printf("<br>\n");
 printf("#Remind 999 \"quack!\" <br>\n");
 printf("<br>\n");
 printf("And it would remind you of your last orders you sent in.<br>\n");
 printf("<br>\n");
 printf("\n");
 printf("<a name=\"email\"> </a>\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h4>#Email [unit]<br>\n");
 printf("[ text of email ]</h4>\n");
 printf("\n");
 printf("A diplomatic email command has been added to the software to help with\n");
 printf("contacting units which have you have no way of contacting otherwise.\n");
 printf("The software will not tell you who the unit belongs to, but will forward\n");
 printf("your email to the owner of the unit specified.\n");
 printf("\n");
 printf("There is no need for a \"#end\" line -- the entire email message you send will\n");
 printf("be forwarded to the unit's master.  Yes, this does mean that your email\n");
 printf("address will be forwarded along with the note.   If you are contacting them\n");
 printf("then you lose your ability to be private to that person.\n");
 printf("\n");
 printf("So, if you sent:\n");
 printf("\n");
 printf("#email 9999\n");
 printf("Greetings.  You've entered the Kingdom of Foo.  Please contact us.\n");
 printf("\n");
 printf("Lord Foo\n");
 printf("foo@bar.com\n");
 printf("\n");
 printf("Faction X, owner of 9999, would receive:\n");
 printf("<pre>\n");
 printf("From: Foo &lt;foo@bar.com&gt;\n");
 printf("Subject:  Greetings!\n");
 printf("\n");
 printf("#email 9999\n");
 printf("Greetings.  You've entered the Kingdom of Foo.  Please\n");
 printf("contact us.\n");
 printf("\n");
 printf("Lord Foo\n");
 printf("foo@bar.com\n");
 printf("</pre>\n");
 printf("\n");
 printf("<a name=\"credits\">\n");
 printf("<center><img src=\"images/bar.jpg\" width=347 height=23></center>\n");
 printf("<h2>Credits</h2>\n");
 printf("\n");
 printf("Atlantis was originally created and programmed by Russell Wallace. Russell\n");
 printf("Wallace created Atlantis 1.0, and partially designed Atlantis 2.0 and\n");
 printf("Atlantis 3.0.<p>\n");
 printf("\n");
 printf("Geoff Dunbar designed and programmed Atlantis 2.0 and 3.0, and created the\n");
 printf("Atlantis Project to freely release and maintain the Atlantis source code.\n");
 printf("See the Atlantis Project web page at\n");
 printf("<a href=\"http://www.prankster.com/project\">http://www.prankster.com/project</a>\n");
 printf("for more information, and more information on Credits.<p>\n");
 printf("\n");
#endif
	f.Enclose(0, "BODY");
	f.Enclose(0, "HTML");
	return 1;
}
