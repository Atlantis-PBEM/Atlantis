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

AString NumToWord(int n)
{
	if(n > 20) return AString(n);
	switch(n) {
		case  0: return AString("zero");
		case  1: return AString("one");
		case  2: return AString("two");
		case  3: return AString("three");
		case  4: return AString("four");
		case  5: return AString("five");
		case  6: return AString("six");
		case  7: return AString("seven");
		case  8: return AString("eight");
		case  9: return AString("nine");
		case 10: return AString("ten");
		case 11: return AString("eleven");
		case 12: return AString("twelve");
		case 13: return AString("thirteen");
		case 14: return AString("fourteen");
		case 15: return AString("fifteen");
		case 16: return AString("sixteen");
		case 17: return AString("seventeen");
		case 18: return AString("eighteen");
		case 19: return AString("nineteen");
		case 20: return AString("twenty");
	}
	return AString("error");
}

int Game::GenRules(const AString &rules, const AString &css,
		const AString &intro)
{
	Ainfile introf;
	Arules f;
	AString temp, temp2;
	int cap;
	int i, j, k, l;
	int last = -1;

	if(f.OpenByName(rules) == -1) {
		return 0;
	}

	if(introf.OpenByName(intro) == -1) {
		return 0;
	}

	f.PutStr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 "
			"Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");
	f.Enclose(1, "HTML");
	f.Enclose(1, "HEAD");
	f.PutStr("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; "
			"charset=utf-8\">");
	f.PutStr(AString("<LINK TYPE=\"text/css\" REL=stylesheet HREF=\"")+
			css + "\">");
	temp = AString(Globals->RULESET_NAME) + " " +
		ATL_VER_STR(Globals->RULESET_VERSION);
	temp2 = temp + " Rules";
	f.TagText("TITLE", temp2);
	f.Enclose(0, "HEAD");
	f.Enclose(1, "BODY");
	f.Enclose(1, "CENTER");
	f.TagText("H1", AString("Rules for ") + temp);
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
	temp = "Note: This document is subject to change, as errors are found "
		"and corrected, and rules sometimes change. Be sure you have the "
		"latest available copy.";
	f.Paragraph(temp);
	f.LinkRef("table_of_contents");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Table of Contents");
	temp = AString("Thanks to ") +
		f.Link("mailto:ken@satori.gso.uri.edu","Kenneth Casey")+
		" for putting together this table of contents.";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#intro", "Introduction"));
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#playing", "Playing Atlantis"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#playing_factions", "Factions"));
	f.TagText("LI", f.Link("#playing_units", "Units"));
	f.TagText("LI", f.Link("#playing_turns", "Turns"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#world", "The World"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#world_regions", "Regions"));
	f.TagText("LI", f.Link("#world_structures", "Structures"));
	if(Globals->NEXUS_EXISTS) {
		temp = "Atlantis Nexus";
		f.TagText("LI", f.Link("#world_nexus", temp));
	}
	if(Globals->CONQUEST_GAME) {
		temp = "The World of Atlantis Conquest";
		f.TagText("LI", f.Link("#world_conquest", temp));
	}
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#movement", "Movement"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#movement_normal", "Normal Movement"));
	if(!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#movement_sailing", "Sailing"));
	f.TagText("LI", f.Link("#movement_order", "Order of Movement"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#skills", "Skills"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#skills_limitations", "Limitations"));
	f.TagText("LI", f.Link("#skills_studying", "Studying"));
	f.TagText("LI", f.Link("#skills_teaching", "Teaching"));
	f.TagText("LI", f.Link("#skills_skillreports", "Skill Reports"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#economy", "The Economy"));
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
	if(Globals->DECAY)
		f.TagText("LI", f.Link("#economy_builddecay", "Building Decay"));
	int may_sail = (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED)) &&
		(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED));
	if(may_sail)
		f.TagText("LI", f.Link("#economy_ships", "Ships"));
	f.TagText("LI", f.Link("#economy_advanceditems", "Advanced Items"));
	f.TagText("LI", f.Link("#economy_income", "Income"));
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#economy_entertainment", "Entertainment"));
	f.TagText("LI", f.Link("#economy_taxingpillaging", "Taxing/Pillaging"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#com", "Combat"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#com_attitudes", "Attitudes"));
	f.TagText("LI", f.Link("#com_attacking", "Attacking"));
	f.TagText("LI", f.Link("#com_muster", "The Muster"));
	f.TagText("LI", f.Link("#com_thebattle", "The Battle"));
	f.TagText("LI", f.Link("#com_victory", "Victory!"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");

	int has_stea = !(SkillDefs[S_STEALTH].flags & SkillType::DISABLED);
	int has_obse = !(SkillDefs[S_OBSERVATION].flags & SkillType::DISABLED);
	if(has_stea || has_obse) {
		if(has_stea) temp = "Stealth";
		else temp = "";
		if(has_obse) {
			if(has_stea) temp += " and ";
			temp += "Observation";
		}
		f.Enclose(1, "LI");
		f.PutStr(f.Link("#stealthobs", temp));
		if(has_stea) {
			f.Enclose(1, "UL");
			f.TagText("LI", f.Link("#stealthobs_stealing", "Stealing"));
			f.TagText("LI", f.Link("#stealthobs_assassination",
						"Assassination"));
			f.Enclose(0, "UL");
		}
		f.Enclose(0, "LI");
	}
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#magic", "Magic"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#magic_skills", "Magic Skills"));
	f.TagText("LI", f.Link("#magic_foundations", "Foundations"));
	f.TagText("LI", f.Link("#magic_furtherstudy", "Further Magic Study"));
	f.TagText("LI", f.Link("#magic_usingmagic", "Using Magic"));
	f.TagText("LI", f.Link("#magic_incombat", "Mages In Combat"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#nonplayers", "Non-Player Units"));
	f.Enclose(1, "UL");
	if(Globals->TOWNS_EXIST && Globals->CITY_MONSTERS_EXIST) {
		f.TagText("LI", f.Link("#nonplayers_guards",
					"City and Town Guardsmen"));
	}
	if(Globals->WANDERING_MONSTERS_EXIST) {
		f.TagText("LI", f.Link("#nonplayers_monsters", "Wandering Monsters"));
	}
	f.TagText("LI", f.Link("#nonplayers_controlled", "Controlled Monsters"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#orders", "Orders"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#orders_abbreviations", "Abbreviations"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#ordersummary", "Order Summary"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#address", "address"));
	f.TagText("LI", f.Link("#advance", "advance"));
	if(Globals->USE_WEAPON_ARMOR_COMMAND)
		f.TagText("LI", f.Link("#armor", "armor"));
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
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#entertain", "entertain"));
	f.TagText("LI", f.Link("#evict", "evict"));
	f.TagText("LI", f.Link("#exchange", "exchange"));
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
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
	int move_over_water = 0;
	if(Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE)
		move_over_water = 1;
	if(!move_over_water) {
		for(i = 0; i < NITEMS; i++) {
			if(ItemDefs[i].flags & ItemType::DISABLED) continue;
			if(ItemDefs[i].swim > 0) move_over_water = 1;
		}
	}
	if(move_over_water)
		f.TagText("LI", f.Link("#nocross", "nocross"));
	f.TagText("LI", f.Link("#option", "option"));
	f.TagText("LI", f.Link("#password", "password"));
	f.TagText("LI", f.Link("#pillage", "pillage"));
	if(Globals->USE_PREPARE_COMMAND)
		f.TagText("LI", f.Link("#prepare", "prepare"));
	f.TagText("LI", f.Link("#produce", "produce"));
	f.TagText("LI", f.Link("#promote", "promote"));
	f.TagText("LI", f.Link("#quit", "quit"));
	f.TagText("LI", f.Link("#restart", "restart"));
	f.TagText("LI", f.Link("#reveal", "reveal"));
	if (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f.TagText("LI", f.Link("#sail", "sail"));
	if(Globals->TOWNS_EXIST)
		f.TagText("LI", f.Link("#sell", "sell"));
	f.TagText("LI", f.Link("#show", "show"));
	f.TagText("LI", f.Link("#spoils", "spoils"));
	if(has_stea)
		f.TagText("LI", f.Link("#steal", "steal"));
	f.TagText("LI", f.Link("#study", "study"));
	f.TagText("LI", f.Link("#tax", "tax"));
	f.TagText("LI", f.Link("#teach", "teach"));
	f.TagText("LI", f.Link("#turn", "turn"));
	if(Globals->USE_WEAPON_ARMOR_COMMAND)
		f.TagText("LI", f.Link("#weapon", "weapon"));
	if (Globals->ALLOW_WITHDRAW)
		f.TagText("LI", f.Link("#withdraw", "withdraw"));
	f.TagText("LI", f.Link("#work", "work"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.TagText("LI", f.Link("#sequenceofevents", "Sequence of Events"));
	f.TagText("LI", f.Link("#reportformat", "Report Format"));
	f.TagText("LI", f.Link("#hintsfornew", "Hints for New Players"));
	if(Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f.Enclose(1, "LI");
		f.PutStr(f.Link("#specialcommands", "Special Commands"));
		f.Enclose(1, "UL");
		f.TagText("LI", f.Link("#_create", "#Create"));
		f.TagText("LI", f.Link("#_resend", "#Resend"));
		f.TagText("LI", f.Link("#_times", "#Times"));
		f.TagText("LI", f.Link("#_rumor", "#Rumor"));
		f.TagText("LI", f.Link("#_remind", "#Remind"));
		f.TagText("LI", f.Link("#_email", "#Email"));
		f.Enclose(0, "UL");
		f.Enclose(0, "LI");
	}
	f.TagText("LI", f.Link("#credits", "Credits"));
	f.Enclose(0, "UL");
	f.Paragraph("Index of Tables");
	f.Paragraph("");
	f.Enclose(1, "UL");
	if (Globals->FACTION_LIMIT_TYPE==GameDefs::FACLIM_FACTION_TYPES)
		f.TagText("LI", f.Link("#tablefactionpoints",
					"Table of Faction Points"));
	f.TagText("LI", f.Link("#tableitemweights", "Table of Item Weights"));
	if(may_sail)
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
	if(may_sail)
		f.TagText("LI", f.Link("#tableshipinfo", "Table of Ship Information"));
	if(Globals->LIMITED_MAGES_PER_BUILDING) {
		f.TagText("LI",
				f.Link("#tablemagebuildings", "Table of Mages/Building"));
	}
	f.Enclose(0, "UL");
	f.LinkRef("intro");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Introduction");
	AString *in;
	while((in = introf.GetStr()) != NULL) {
		f.PutStr(*in);
		delete in;
	}
	f.LinkRef("playing");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Playing Atlantis");
	temp = "Atlantis (as you undoubtedly already know) is a play by email "
		"game.  When you sign up for Atlantis, you will be sent a turn "
		"report (via email).  Your report completely details your position "
		"in the game.  After going over this report, and possibly "
		"communicating with other players in the game, you determine your "
		"course of action, and create a file of \"orders\", which you then "
		"send back to the Atlantis server. Then, at a regular interval "
		"(often one week), Atlantis collects all the orders, runs another "
		"turn (covering one month in game time), and sends all the players "
		"another report.";
	f.Paragraph(temp);
	f.LinkRef("playing_factions");
	f.TagText("H3", "Factions:");
	temp = "A player's position is called a \"faction\".  Each faction has "
		"a name and a number (the number is assigned by the computer, and "
		"used for entering orders). Each player is allowed to play one and "
		"ONLY one faction at any given time. Each faction is composed of a "
		"number of \"units\", each unit being a group of one or more people "
		"loyal to the faction.  You start the game with a single unit "
		"consisting of one character, plus a sum of money.  More people can "
		"be hired during the course of the game, and formed into more "
		"units.  (In these rules, the word \"character\" generally refers "
		"either to a unit consisting of only one person, or to a person "
		"within a larger unit.)";
	f.Paragraph(temp);
	temp = "A faction is considered destroyed, and the player knocked out "
		"of the game, if ever all its people are killed or disbanded (i.e. "
		"the faction has no units left).  The program does not consider "
		"your starting character to be special; if your starting character "
		"gets killed, you will probably have been thinking of that character "
		"as the leader of your faction, so some other character can be "
		"regarded as having taken the dead leader's place (assuming of "
		"course that you have at least one surviving unit!).  As far as the "
		"computer is concerned, as long as any unit of the faction "
		"survives, the faction is not wiped out.  (If your faction is "
		"wiped out, you can rejoin the game with a new starting "
		"character.)";
	f.Paragraph(temp);
	Faction fac;
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		temp = "A faction has one pre-set limit; it may not contain more than ";
		temp += AString(AllowedMages(&fac)) + " mages";
		if(Globals->APPRENTICES_EXIST) {
			temp += AString("and ") + AllowedApprentices(&fac) + " apprentices";
		}
		temp += ". Magic is a rare art, and only a few in the world can "
			"master it. Aside from that, there  is no limit to the number "
			"of units a faction may contain, nor to how many items can be "
			"produced or regions taxed.";
		f.Paragraph(temp);
	} else if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		temp = "Each faction has a type; this is decided by the player, "
			"and determines what the faction may do.  The faction has ";
		temp +=  Globals->FACTION_POINTS;
		temp += " Faction Points, which may be spent on any of the 3 "
			"Faction Areas, War, Trade, and Magic.  The faction type may "
			"be changed at the beginning of each turn, so a faction can "
			"change and adapt to the conditions around it.  Faction Points "
			"spent on War determine the number of regions in which factions "
			"can obtain income by taxing or pillaging. Faction Points spent "
			"on Trade determine the number of regions in which a faction "
			"may conduct trade activity. Trade activity includes producing "
			"goods, building ships and buildings, and buying trade items. "
			"Faction Points spent on Magic determines the number of mages ";
		if(Globals->APPRENTICES_EXIST)
			temp += "and apprentices ";
		temp += "the faction may have. (More information on all of the "
			"faction activities is in further sections of the rules).  Here "
			"is a chart detailing the limits on factions by Faction Points:";
		f.Paragraph(temp);
		f.LinkRef("tablefactionpoints");
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TH", "Faction Points");
		f.TagText("TH", "War (max tax regions)");
		f.TagText("TH", "Trade (max trade regions)");
		temp = "Magic (max mages";
		if(Globals->APPRENTICES_EXIST)
			temp += "/apprentices";
		temp += ")";
		f.TagText("TH", temp);
		f.Enclose(0, "TR");
		int i;
		for(i = 0; i <= Globals->FACTION_POINTS; i++) {
			fac.type[F_WAR]=i;
			fac.type[F_TRADE]=i;
			fac.type[F_MAGIC]=i;
			f.Enclose(1, "TR");
			f.Enclose(1, "TD ALIGN=CENTER NOWRAP");
			f.PutStr(i);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER NOWRAP");
			f.PutStr(AllowedTaxes(&fac));
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER NOWRAP");
			f.PutStr(AllowedTrades(&fac));
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER NOWRAP");
			temp = AllowedMages(&fac);
			if(Globals->APPRENTICES_EXIST)
				temp += AString("/") + AllowedApprentices(&fac);
			f.PutStr(temp);
			f.Enclose(0, "TD");
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
		f.PutStr("<P></P>");
		int m,w,t;
		fac.type[F_WAR] = w = (Globals->FACTION_POINTS+1)/3;
		fac.type[F_TRADE] = t = Globals->FACTION_POINTS/3;
		fac.type[F_MAGIC] = m = (Globals->FACTION_POINTS+2)/3;
		int nm, na, nw, nt;
		nm = AllowedMages(&fac);
		na = AllowedApprentices(&fac);
		nt = AllowedTrades(&fac);
		nw = AllowedTaxes(&fac);
		temp = "For example, a well rounded faction might spend ";
		temp += AString(w) + " point" + (w==1?"":"s") + " on War, ";
		temp += AString(t) + " point" + (t==1?"":"s") + " on Trade, and ";
		temp += AString(m) + " point" + (m==1?"":"s") + " on Magic.  ";
		temp += "This faction's type would appear as \"War ";
		temp += AString(w) + " Trade " + t + " Magic " + m;
		temp += "\", and would be able to tax ";
		temp += AString(nw) + " region" + (nw==1?"":"s") + ", ";
		temp += "perform trade in ";
		temp += AString(nt) + " region" + (nt==1?"":"s") + ", and have ";
		temp += AString(nm) + " mage" + (nm==1?"":"s");
		if(Globals->APPRENTICES_EXIST) {
			temp += " as well as ";
			temp += AString(na) + " apprentice" + (na==1?"":"s");
		}
		temp += ".";
		f.Paragraph(temp);

		fac.type[F_WAR] = w = Globals->FACTION_POINTS;
		fac.type[F_MAGIC] = m = 0;
		fac.type[F_TRADE] = t = 0;
		nw = AllowedTaxes(&fac);
		nt = AllowedTrades(&fac);
		nm = AllowedMages(&fac);
		na = AllowedApprentices(&fac);
		temp = "As another example, a specialized faction might spend all ";
		temp += AString(w) + " point" + (w==1?"":"s") + " on War. ";
		temp += "This faction's type would appear as \"War ";
		temp += AString(w) + "\", and it would be able to tax " + nw;
		temp += AString(" region") + (nw==1?"":"s") + ", but could ";
		if(nt == 0)
			temp += "not perform trade in any regions";
		else
			temp += AString("only perform trade in ") + nt + " region" +
				(nt == 1?"":"s");
		temp += ", ";
		if(!Globals->APPRENTICES_EXIST)
			temp += "and ";
		if(nm == 0)
			temp += "could not possess any mages";
		else
			temp += AString("could only possess ") + nm + " mage" +
				(nm == 1?"":"s");
		if(Globals->APPRENTICES_EXIST) {
			temp += ", and ";
			if(na == 0)
				temp += "could not possess any apprentices";
			else
				temp += AString("could only possess ") + na + " apprentice" +
					(na == 1?"":"s");
		}
		temp += ".";
		f.Paragraph(temp);
		if (Globals->FACTION_POINTS>3) {
			int rem=Globals->FACTION_POINTS-3;
			temp = "Note that it is possible to have a faction type with "
				"less than ";
			temp += Globals->FACTION_POINTS;
			temp += " points spent. In fact, a starting faction has one "
				"point spent on each of War, Trade, and Magic, leaving ";
			temp += AString(rem) + " point" + (rem==1?"":"s") + " unspent.";
			f.Paragraph(temp);
		}
	}
	temp = "When a faction starts the game, it is given a one-man unit and ";
	temp += Globals->START_MONEY;
	temp += " silver in unclaimed money.  Unclaimed money is cash that your "
		"whole faction has access to, but cannot be taken away in battle ("
		"silver in a unit's possessions can be taken in battle).  This allows "
		"a faction to get started without presenting an enticing target for "
		"other factions. Units in your faction may use the ";
	temp += f.Link("#claim", "CLAIM") + " order to take this silver, and use "
		"it to buy goods or recruit men";
	if(Globals->ALLOW_WITHDRAW) {
		temp += ", or use the ";
		temp += f.Link("#withdraw", "WITHDRAW");
		temp += " order to withdraw goods directly";
	}
	temp += ".";
	f.Paragraph(temp);
	temp = "An example faction is shown below, consisting of a starting "
		"character, Merlin the Magician, who has formed two more units, "
		"Merlin's Guards and Merlin's Workers. Each unit is assigned a "
		"unit number by the computer (completely independent of the "
		"faction number); this is used for entering orders. Here, the "
		"player has chosen to give his faction the same name (\"Merlin "
		"the Magician\") as his starting character. Alternatively, you "
		"can call your faction something like \"The Great Northern "
		"Mining Company\" or whatever.";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "PRE");
	f.ClearWrapTab();
	if(Globals->LEADERS_EXIST) {
		f.WrapStr("* Merlin the Magician (17), Merlin (27), leader [LEAD].  "
				"Skills: none.");
	} else {
		f.WrapStr("* Merlin the Magician (17), Merlin (27), man [MAN].  "
				"Skills: none.");
	}
	if(Globals->RACES_EXIST) {
		f.WrapStr("* Merlin's Guards (33), Merlin (27), 20 vikings [VIKI], "
				"20 swords [SWOR]. Skills: none.");
		f.WrapStr("* Merlin's Workers (34), Merlin (27), 50 vikings "
				"[VIKI].  Skills: none.");
	} else {
		f.WrapStr("* Merlin's Guards (33), Merlin (27), 20 men [MAN], "
				"20 swords [SWOR]. Skills: none.");
		f.WrapStr("* Merlin's Workers (34), Merlin (27), 50 men [MAN].  "
				"Skills: none.");
	}
	f.Enclose(0, "PRE");
	f.LinkRef("playing_units");
	f.TagText("H3", "Units:");
	temp = "A unit is a grouping together of people, all loyal to the "
		"same faction. The people in a unit share skills and possessions, "
		"and execute the same orders each month. The reason for having "
		"units of many people, rather than keeping track of individuals, "
		"is to simplify the game play.  The computer does not keep track of "
		"individual names, possessions, or skills for people in the same "
		"unit, and all the people in a particular unit must be in the same "
		"place at all times.  If you want to send people in the same unit "
		"to different places, you must split up the unit.  Apart from "
		"this, there is no difference between having one unit of 50 people, "
		"or 50 units of one person each, except that the former is very "
		"much easier to handle.";
	f.Paragraph(temp);
	if(Globals->RACES_EXIST) {
		temp = "There are different races that make up the population of "
			"Atlantis. (See the section on skills for a list of these.)";
		if(Globals->LEADERS_EXIST) {
			temp += " In addition, there are \"leaders\", who are presumed "
				"to be of one of the other races, but are all the same "
				"in game terms.";
		}
	} else {
		temp = "Units are made of of ordinary people";
		if(Globals->LEADERS_EXIST) {
			temp += "as well as leaders";
		}
		temp += ".";
	}
	if (Globals->LEADERS_EXIST&&Globals->SKILL_LIMIT_NONLEADERS) {
		temp += " Units made up of normal people may only know one skill, "
			"and cannot teach other units.  Units made up of leaders "
			"may know as many skills as desired, and may teach other "
			"units to speed the learning process.";
	}
	if (Globals->LEADERS_EXIST) {
		temp += " Leaders and normal people may not be mixed in the same "
			"unit. However, leaders are more expensive to recruit and "
			"maintain. (More information is in the section on skills.)";
	}
	if (Globals->RACES_EXIST) {
		temp += " A unit is treated as the least common denominator of "
			"the people within it, so a unit made up of two races with "
			"different strengths and weaknesses will have all the "
			"weaknesses, and none of the strengths of either race.";
	}
	f.Paragraph(temp);
	f.LinkRef("playing_turns");
	f.TagText("H3", "Turns:");
	temp = "Each turn, the Atlantis server takes the orders file that "
		"you mailed to it, and assigns the orders to the respective units. "
		"All units in your faction are completely loyal to you, and will "
		"execute the orders to the best of their ability. If the unit does "
		"something unintended, it is generally because of incorrect orders; "
		"a unit will not purposefully betray you.";
	f.Paragraph(temp);
	temp = "A turn is equal to one game month.  A unit can do many actions "
		"at the start of the month, that only take a matter of hours, such "
		"as buying and selling commodities, or fighting an opposing "
		"faction.  Each unit can also do exactly one action that takes up "
		"the entire month, such as harvesting resources or moving from one "
		"region to another.  The orders which take an entire month are ";
	temp += f.Link("#advance", "ADVANCE") + ", ";
	temp += f.Link("#build", "BUILD") + ", ";
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		temp += f.Link("#entertain", "ENTERTAIN") + ", ";
	temp += f.Link("#move", "MOVE") + ", ";
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		temp += f.Link("#pillage", "PILLAGE") + ", ";
	temp += f.Link("#produce", "PRODUCE") + ", ";
	if(!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		temp += f.Link("#sail", "SAIL") + ", ";
	temp += f.Link("#study", "STUDY") + ", ";
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		temp += f.Link("#tax", "TAX") + ", ";
	temp += f.Link("#teach", "TEACH") + " and ";
	temp += f.Link("#work", "WORK") + ".";
	f.Paragraph(temp);
	f.LinkRef("world");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "The World");
	temp = "The Atlantis world is divided for game purposes into "
		"hexagonal regions.  Each region has a name, and one of the "
		"following terrain types:  Ocean, Plain, Forest, Mountain, ";
	if(Globals->CONQUEST_GAME)
		temp += "or ";
	temp += "Swamp";
	if(!Globals->CONQUEST_GAME)
		temp += ", Jungle, Desert, or Tundra";
	temp += ". (There may be other types of terrain to be discovered as the "
		"game progresses.)  Regions can contain units belonging to players; "
		"they can also contain structures such as buildings";
	if(may_sail)
		temp += " and ships";
	temp += ". Two units in the same region can normally interact, unless "
		"one of them is concealed in some way.  Two units in different "
		"regions cannot normally interact.  NOTE: Combat is an exception "
		"to this.";
	f.Paragraph(temp);
	f.LinkRef("world_regions");
	f.TagText("H3", "Regions:");
	temp = "Here is a sample region, as it might appear on your turn report:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "PRE");
	f.ClearWrapTab();
	temp = "plain (172,110) in Turia, 500 peasants";
	if(Globals->RACES_EXIST)
		temp += " (nomads)";
	int money = (500 * (15 - Globals->MAINTENANCE_COST));
	temp += AString(", $") + money + ".";
	f.WrapStr(temp);
	f.WrapStr("------------------------------------------------------");
	f.AddWrapTab();
	if (Globals->WEATHER_EXISTS)
		f.WrapStr("The weather was clear last month; it will be clear next "
				"month.");
	temp = AString("Wages: $15 (Max: $") + (money/Globals->WORK_FRACTION) +
			").";
	f.WrapStr(temp);
	f.WrapStr("Wanted: none.");
	temp = "For Sale: 50 ";
	if(Globals->RACES_EXIST)
		temp += "nomads [NOMA]";
	else
		temp += "men [MAN]";
	temp += " at $";
	float ratio = ItemDefs[(Globals->RACES_EXIST?I_NOMAD:I_MAN)].baseprice/
		(float)Globals->BASE_MAN_COST;
	temp += (int)(60*ratio);
	if(Globals->LEADERS_EXIST) {
		ratio = ItemDefs[I_LEADERS].baseprice/(float)Globals->BASE_MAN_COST;
		temp += ", 10 leaders [LEAD] at $";
		temp += (int)(60*ratio);
	}
	temp += ".";
	f.WrapStr(temp);
	temp = AString("Entertainment available: $") +
		(money/Globals->ENTERTAIN_FRACTION) + ".";
	f.WrapStr(temp);
	temp = "Products: ";
	if(Globals->FOOD_ITEMS_EXIST)
		temp += "23 grain [GRAI], ";
	temp += "37 horses [HORS].";
	f.WrapStr(temp);
	f.PutNoFormat("");
	f.PutNoFormat("Exits:");
	f.WrapStr("North : ocean (172,108) in Atlantis Ocean.");
	f.WrapStr("Northeast : ocean (173,109) in Atlantis Ocean.");
	f.WrapStr("Southeast : ocean (173,111) in Atlantis Ocean.");
	f.WrapStr("South : plain (172,112) in Turia.");
	f.WrapStr("Southwest : plain (171,111) in Turia.");
	f.WrapStr("Northwest : plain (171,109) in Turia.");
	f.PutNoFormat("");
	f.DropWrapTab();
	temp = "* Hans Shadowspawn (15), Merry Pranksters (14), ";
	if(Globals->LEADERS_EXIST)
		temp2 = "leader [LEAD]";
	else if(Globals->RACES_EXIST)
		temp2 = "nomad [NOMA]";
	else
		temp2 = "man [MAN]";
	temp += temp2 + ", 500 silver [SILV]. Skills: none.";
	f.WrapStr(temp);
	temp = AString("- Vox Populi (13), ") + temp2 + ".";
	f.WrapStr(temp);
	f.Enclose(0, "PRE");
	temp = "This report gives all of the available information on this "
		"region.  The region type is plain, the name of the surrounding area "
		"is Turia, and the coordinates of this region are (172,110).  The "
		"population of this region is 500 ";
	if(Globals->RACES_EXIST)
		temp += "nomads";
	else
		temp += "peasants";
	temp += AString(", and there is $") + money + " of taxable income ";
	temp += "currently in this region.  Then, under the dashed line, are "
		"various details about items for sale, wages, etc.  Finally, "
		"there is a list of all visible units.  Units that belong to your "
		"faction will be so denoted by a '*', whereas other faction's "
		"units are preceded by a '-'.";
	f.Paragraph(temp);
	temp = "Since Atlantis is made up of hexagonal regions, the coordinate "
		"system is not always exactly intuitive.  Here is the layout of "
		"Atlantis regions:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "PRE");
	f.PutNoFormat("   ____        ____");
	f.PutNoFormat("  /    \\      /    \\");
	f.PutNoFormat(" /(0,0) \\____/(2,0) \\____/");
	f.PutNoFormat(" \\      /    \\      /    \\     N");
	f.PutNoFormat("  \\____/(1,1) \\____/(3,1) \\_   |");
	f.PutNoFormat("  /    \\      /    \\      /    |");
	f.PutNoFormat(" /(0,2) \\____/(2,2) \\____/     |");
	f.PutNoFormat(" \\      /    \\      /    \\   W-O-E");
	f.PutNoFormat("  \\____/(1,3) \\____/(3,3) \\_   |");
	f.PutNoFormat("  /    \\      /    \\      /    S");
	f.PutNoFormat(" /(0,4) \\____/(2,4) \\____/");
	f.PutNoFormat(" \\      /    \\      /    \\");
	f.PutNoFormat("  \\____/      \\____/");
	f.PutNoFormat("  /    \\      /    \\");
	f.Enclose(0, "PRE");
	temp = "Note that the are \"holes\" in the coordinate system; there "
		"is no region (1,2), for instance.  This is due to the hexagonal "
		"system of regions.";
	f.Paragraph(temp);
	temp = "Most regions are similar to the region shown above, but the "
		"are certain exceptions.  Oceans, not surprisingly, have no "
		"population.";
	if (Globals->TOWNS_EXIST)
		temp += " Some regions will contain villages, towns, and cities. "
			"More information on these is available in the section on the "
			"ecomony.";
	f.Paragraph(temp);
	f.LinkRef("world_structures");
	f.TagText("H3", "Structures:");
	temp = "Regions may also contain structures, such as buildings";
	if(may_sail)
		temp += " or ships";
	temp += ". These will appear directly below the list of units.  Here is "
		"a sample structure:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "PRE");
	f.ClearWrapTab();
	f.WrapStr("+ Temple of Agrik [3] : Tower.");
	f.AddWrapTab();
	temp = "- High Priest Chafin (9), ";
	temp += temp2 + ", sword [SWOR]";
	f.WrapStr(temp);
	temp = "- Rowing Doom (188), ";
	if(Globals->RACES_EXIST)
		temp += "10 nomads [NOMA]";
	else if(Globals->LEADERS_EXIST)
		temp += "10 leaders [LEAD]";
	else
		temp += "10 men [MAN]";
	temp += ", 10 swords [SWOR].";
	f.WrapStr(temp);
	f.Enclose(0, "PRE");
	temp = "The structure lists the name, the number, and what type of "
		"structure it is.  (More information of the types of structures "
		"can be found in the section on the economy.)  Following this "
		"is a list of units inside the structure.";
	if (has_stea)
		temp += " Units within a structure are always visible, even if "
			"they would otherwise not be seen.";
	f.Paragraph(temp);
	temp = "Units inside structures are still considered to be in the "
		"region, and other units can interact with them; however, they "
		"may gain benefits, such as defensive bonuses in combat from being "
		"inside a building.  The first unit to enter an object is "
		"considered to be the owner; only this unit can do things such as "
		"renaming the object, or permitting other units to enter. The owner "
		"of an object can be identified on the turn report, as it is the "
		"first unit listed under the object.  Only units with men in them "
		"can be structure owners, so newly created units cannot own a "
		"structure until they contain men.";
	f.Paragraph(temp);
	if(Globals->NEXUS_EXISTS) {
		f.LinkRef("world_nexus");
		temp = "Atlantis Nexus:";
		f.TagText("H3", temp);
		temp = "Note: the following section contains some details that "
			"you may wish to skip over until you have had a chance to "
			"read the rest of the rules, and understand the mechanics "
			"of Atlantis.  However, be sure to read this section before "
			"playing, as it will affect your early plans in Atlantis.";
		f.Paragraph(temp);
		temp = "When a faction first starts in Atlantis, it begins with "
			"one unit, in a special region called the Atlantis Nexus.";
		if(Globals->MULTI_HEX_NEXUS)
			temp += " These regions exist ";
		else
			temp += " This region exists ";
		if (!Globals->NEXUS_IS_CITY) {
			temp += "outside of the normal world of Atlantis, and as such ";
			if(Globals->MULTI_HEX_NEXUS)
				temp += "have ";
			else
				temp += "has ";
			temp += "no products or marketplaces; ";
			if(Globals->MULTI_HEX_NEXUS)
				temp += "they merely serve ";
			else
				temp += "it merely serves ";
			temp += "as the magical entry into Atlantis.";
		} else {
			temp += "outside of the normal world of Atlantis, but ";
			if(Globals->MULTI_HEX_NEXUS)
				temp += "each contains ";
			else
				temp += "contains ";
			temp += "a starting city with all its benefits";
			if(Globals->GATES_EXIST)
				temp += ", including a gate";
			temp += ". ";
			if(Globals->MULTI_HEX_NEXUS)
				temp += "They also serve ";
			else
				temp += "It also serves ";
			temp += "as the magical entry into ";
			temp += Globals->WORLD_NAME;
			temp += ".";
		}
		f.Paragraph(temp);
		if(Globals->MULTI_HEX_NEXUS) {
			temp = "From the Nexus hexes, there are exits either to other "
				"Nexus hexes, or to starting cities in Atlantis.  Units may "
				"move through these exits as normal, but once in a starting "
				"city, there is no way to regain entry to the Nexus.";
		} else {
			temp = "From the Atlantis Nexus, there are six exits into the "
				"starting cities of Atlantis.  Units may move through "
				"these exits as normal, but once through an exit, there is "
				"no return path to the Nexus.";
		}
		if(Globals->GATES_EXIST &&
				(Globals->NEXUS_GATE_OUT || Globals->NEXUS_IS_CITY)) {
			temp += " It is also possible to use Gate Lore to get out of "
				"Nexus";
			if(Globals->NEXUS_GATE_OUT && !Globals->NEXUS_IS_CITY)
				temp += " (but not to return)";
			temp += ".";
		}
		temp += " The ";
		if(!Globals->MULTI_HEX_NEXUS)
			temp += "six ";
		temp += "starting cities offer much to a starting faction; ";
		if(Globals->START_CITIES_START_UNLIMITED) {
			if (!Globals->SAFE_START_CITIES && Globals->CITY_MONSTERS_EXIST)
				temp += "until someone conquers the guardsmen, ";
			temp += "there are unlimited amounts of many materials and men "
				"(though the prices are often quite high).";
		} else {
			temp += "there are materials as well as a very large supply of "
				"men (though the prices are often quite high).";
		}
		if(Globals->SAFE_START_CITIES || Globals->CITY_MONSTERS_EXIST)
			temp += " In addition, ";
		if(Globals->SAFE_START_CITIES)
			temp += "no battles are allowed in starting cities";
		if(Globals->CITY_MONSTERS_EXIST) {
			if(Globals->SAFE_START_CITIES) temp += " and ";
			temp += "the starting cities are guarded by strong guardsmen, "
				"keeping any units within the city ";
			if(!Globals->SAFE_START_CITIES)
				temp += "much safer ";
			else
				temp += "safe ";
			temp += "from attack. See the section on Non-Player Units for "
				"more information on city guardsmen";
		}
		temp += ". ";
		temp += "As a drawback, these cities tend to be extremely crowded, "
			"and most factions will wish to leave the starting cities when "
			"possible.";
		f.Paragraph(temp);
		temp = "It is always possible to enter any starting city from the "
			"nexus";
		if(!Globals->SAFE_START_CITIES)
			temp += ", even if that starting city has been taken over and "
				"guarded by another faction";
		temp += ". This is due to the transportation from the Nexus to the "
			"starting city being magical in nature.";
		if(!Globals->SAFE_START_CITIES)
			temp += " Once in the starting city however, no gaurentee of "
				"safety is given.";
		f.Paragraph(temp);
		int num_methods = 1 + (Globals->GATES_EXIST?1:0) + (may_sail?1:0);
		char *methods[] = {"You must go ", "The first is ", "The second is "};
		int method = 1;
		if (num_methods == 1) method = 0;
		temp = AString("There ") + (num_methods == 1?"is ":"are ") +
			NumToWord(num_methods) + " method" + (num_methods == 1?" ":"s ") +
			"of departing the starting cities. ";
		temp += methods[method++];
		temp += " by land, but keep in mind that the lands immediately "
			"surrounding the starting cities will tend to be highly "
			"populated, and possibly quite dangerous to travel.";
		if(may_sail) {
			temp += AString(" ") + methods[method];
			temp += " by sea; all of the starting cities lie against an "
				"ocean, and a faction may easily purchase wood and "
				"construct a ship to ";
			temp += f.Link("#sail", "SAIL");
			temp += " away.  Be wary of pirates seeking to prey on new "
				"factions, however!";
		}
		if (Globals->GATES_EXIST) {
			temp += " And last, rumors of a magical Gate Lore suggest yet "
				"another way to travel from the starting cities.  The rumors "
				"are vague, but factions wishing to travel far from the "
				"starting cities, taking only a few men with them, might "
				"wish to pursue this method.";
		}
		f.Paragraph(temp);
	}
	if(Globals->CONQUEST_GAME) {
		f.LinkRef("world_conquest");
		f.TagText("H3", "The World of Atlantis Conquest");
		temp = "In a game of Atlantis Conquest, each player begins the "
			"game on a small island of 8 regions, seperated by ocean from "
			"the rest of the players.  The starting islands are located "
			"around the perimeter of a larger central island. Sailing from "
			"the starting islands towards the center of the map should "
			"lead to the central island within a few regions.";
		f.Paragraph(temp);
	}

	f.LinkRef("movement");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Movement");
	if(may_sail)
		temp = "There are two main methods of movement in Atlantis.  The "
			"first ";
	else
		temp = "The main method of movement in Atlantis ";
	temp += "is done using the ";
	temp += f.Link("#move", "MOVE") + " order (or the " +
		f.Link("#advance", "ADVANCE");
	temp += " order), and moves units individually from one region to "
		"another. ";
	if(may_sail) {
		temp += "The other method is done using the ";
		temp += f.Link("#sail", "SAIL") + " order, which can sail a ";
		temp += "ship, including all of its occupants from one region to "
			"another. ";
	}
	temp += "Certain powerful mages may also teleport themselves, or even "
		"other units, but the knowledge of the workings of this magic is "
		"carefully guarded.";
	f.Paragraph(temp);

	f.LinkRef("movement_normal");
	f.TagText("H3", "Normal Movement:");
	temp = "In one month, a unit can issue a single ";
	temp += f.Link("#move", "MOVE") + " order, using one or more of its "
		"movement points. There are three modes of travel: walking, riding "
		"and flying. Walking units have ";
	temp += NumToWord(Globals->FOOT_SPEED) + " movement point" +
		(Globals->FOOT_SPEED==1?"":"s") + ", riding units have ";
	temp += NumToWord(Globals->HORSE_SPEED) + ", and flying units have ";
	temp += NumToWord(Globals->FLY_SPEED) + ". ";
	temp += "A unit will automatically use the fastest mode of travel "
		"it has available. The ";
	temp += f.Link("#advance", "ADVANCE") + " order is the same as " +
		f.Link("#move", "MOVE") + ", except that it implies attacks on " +
		"units which try to forbid access; see the section on combat for " +
		"details.";
	f.Paragraph(temp);

	temp = "Flying units are not initially available to starting players. "
		"A unit can ride provided that the carrying capacity of its "
		"horses is at least as great as the weight of its people and "
		"all other items.  A unit can walk provided that the carrying "
		"capacity of its people";
	if(!(ItemDefs[I_HORSE].flags & ItemType::DISABLED)) {
		if(!(ItemDefs[I_WAGON].flags & ItemType::DISABLED)) temp += ", ";
		else temp += " and ";
		temp += "horses";
	}
	if(!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) &&
			(!(ItemDefs[I_HORSE].flags & ItemType::DISABLED))) {
		temp += ", and wagons";
	}
	temp += " is at least as great as the weight of all its other items";
	if(!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) &&
			(!(ItemDefs[I_HORSE].flags & ItemType::DISABLED))) {
		temp += ", and provided that it has at least as many horses as "
			"wagons (otherwise the excess wagons count as weight, not "
			"capacity)";
	}
	temp += ". Otherwise the unit cannot issue a ";
	temp += f.Link("#move", "MOVE") + " order.";
	temp += " Most people weigh 10 units and have a capacity of 5 units; "
		"data for items is as follows:";
	f.Paragraph(temp);
	f.LinkRef("tableitemweights");
	f.Enclose(1, "CENTER");
	f.Enclose(1, "TABLE BORDER=1");
	f.Enclose(1, "TR");
	f.TagText("TD", "");
	f.TagText("TH", "Weight");
	f.TagText("TH", "Capacity");
	f.Enclose(0, "TR");
	for(i = 0; i < NITEMS; i++) {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(!(ItemDefs[i].type & IT_NORMAL)) continue;
		last = ItemDefs[i].pSkill;
		if(last != -1 && (SkillDefs[last].flags & SkillType::DISABLED))
			continue;
		last = 0;
		for(j = 0; j < (int) (sizeof(ItemDefs->pInput) /
				sizeof(ItemDefs->pInput[0])); j++) {
			k = ItemDefs[i].pInput[j].item;
			if(k != -1 && (ItemDefs[k].flags & ItemType::DISABLED))
				last = 1;
			if(k != -1 && !(ItemDefs[k].type & IT_NORMAL)) last = 1;
		}
		if(last == 1) continue;
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ItemDefs[i].name);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ItemDefs[i].weight);
		f.Enclose(0, "TD");
		cap = ItemDefs[i].walk - ItemDefs[i].weight;
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		if(ItemDefs[i].walk || (ItemDefs[i].hitchItem != -1)) {
			if(ItemDefs[i].hitchItem == -1)
				f.PutStr(cap);
			else {
				temp = (cap + ItemDefs[i].hitchwalk);
				temp += " (with ";
				temp += ItemDefs[ItemDefs[i].hitchItem].name;
				temp += ")";
				f.PutStr(temp);
			}
		} else {
			f.PutStr("&nbsp;");
		}
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Enclose(0, "CENTER");
	if(Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) {
		temp = "A unit which can fly, is capable of travelling over water.";
		if(Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_MUST_LAND)
			temp += " However, if the unit ends its turn over a water hex "
				"that unit will drown.";
		f.Paragraph(temp);
	}

	temp = "Since regions are hexagonal, each region has six neighbouring "
		"regions to the north, northeast, southeast, south, southwest and "
		"northwest.  Moving from one region to another normally takes one "
		"movement point, except that the following terrain types take two "
		"movement points for riding or walking units to enter:";
	temp += " Forest, Mountain, ";
	if(Globals->CONQUEST_GAME)
		temp += "and ";
	temp += "Swamp";
	if(!Globals->CONQUEST_GAME)
		temp += ", Jungle, and Tundra";
	temp += ".";
	if (Globals->WEATHER_EXISTS) {
		temp += " Also, during certain seasons (depending on the latitude "
			"of the region), all units (including flying ones) have a "
			"harder time and travel will take twice as many movement "
			"points as normal, as freezing weather makes travel difficult; "
			"in the tropics, seasonal hurricane winds and torrential "
			"rains have a similar effect.";
	}
	temp += " Units may not move through ocean regions ";
	if(may_sail) {
		temp += "without using the ";
		temp += f.Link("#sail", "SAIL") + " order";
	}
	if(Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) {
		temp += " unless they are capable of flight";
		if (Globals->FLIGHT_OVER_WATER==GameDefs::WFLIGHT_MUST_LAND) {
			temp += ", and even then, flying units must end their "
				"movement on land or else drown";
		}
	}
	temp += ".";
	f.Paragraph(temp);
	temp = "Units may also enter or exit structures while moving.  Moving "
		"into or out of a structure does not use any movement points at "
		"all.  Note that a unit can also use the ";
	temp += f.Link("#enter", "ENTER") + " and " + f.Link("#leave", "LEAVE");
	temp += " orders to move in and out of structures, without issuing a ";
	temp += f.Link("#move", "MOVE") + " order.";
	temp += " The unit can also use the ";
	temp += f.Link("#move", "MOVE") + " order to enter or leave a structure.";
	f.Paragraph(temp);
	if(Globals->UNDERWORLD_LEVELS || Globals->UNDERDEEP_LEVELS) {
		temp = "Finally, certain structures contain interior passages to "
			"other regions.  The ";
		temp += f.Link("#move", "MOVE") + " IN order can be used to go ";
		temp += "through these passages; the movement point cost is equal "
			"to the normal cost to enter the destination region.";
		f.Paragraph(temp);
	}
	temp = "Example: One man with a horse, sword, and chain mail wants to "
		"move north, then northeast.  The capacity of the horse is ";
	cap = ItemDefs[I_HORSE].ride - ItemDefs[I_HORSE].weight;
	temp += cap;
	temp += " and the weight of the man and other items is ";
	int weight = ItemDefs[I_MAN].weight + ItemDefs[I_SWORD].weight +
			ItemDefs[I_CHAINARMOR].weight;
	temp += weight;
	if(cap > weight)
		temp += ", so he can ride";
	else
		temp += ", so he must walk";
	if(Globals->WEATHER_EXISTS)
		temp += ". The month is April, so he has ";
	else
		temp += " and has ";
	temp += NumToWord(Globals->HORSE_SPEED);
	int travel = Globals->HORSE_SPEED;
	temp += " movement point";
	temp += AString((Globals->HORSE_SPEED == 1) ? "" : "s") + ". ";
	temp += "He issues the order MOVE NORTH NORTHEAST. First he moves north, "
		"into a plain region.  This uses ";
	int cost = TerrainDefs[R_PLAIN].movepoints;
	temp += NumToWord(cost) + " movement point" + (cost == 1?"":"s") + ".";
	travel -= cost;
	if(travel > TerrainDefs[R_FOREST].movepoints) {
		temp += " Then he moves northeast, into a forest region. This uses ";
		cost = TerrainDefs[R_FOREST].movepoints;
		temp += NumToWord(cost) + " movement point" + (cost == 1?"":"s") + ",";
		travel -= cost;
		temp += " so the movement is completed with ";
		temp += NumToWord(travel) + " to spare.";
	} else {
		temp += " He does not have the ";
		cost = TerrainDefs[R_FOREST].movepoints;
		temp += NumToWord(cost) + " movement point" + (cost == 1?"":"s");
		temp += " needed to move into the forest region to the northeast, "
			"so the movement is halted at this point.  The remaining move"
			"will be added to his orders for the next turn, before any";
		temp += f.Link("#turn", "TURN") + " orders are processed.";
	}
	f.Paragraph(temp);

	if(may_sail) {
		f.LinkRef("movement_sailing");
		f.TagText("H3", "Sailing:");
		temp = "Movement by sea is in some ways similar. It does not use the ";
		temp += f.Link("#move", "MOVE") + " order however.  Instead, the " +
			"owner of a ship must issue the " + f.Link("#sail", "SAIL") +
			" order, and other units wishing to help sail the ship must " +
			"also issue the " + f.Link("#sail", "SAIL") + " order. ";
		temp += "The ship will then, if possible, make the indicated "
			"movement, carrying all units on the ship with it.  Units on "
			"board the ship, but not aiding in the sailing of the ship, "
			"may execute other orders while the ship is sailing.  A unit "
			"which does not wish to travel with the ship should leave the "
			"ship in a coastal region, before the ";
		temp += f.Link("#sail", "SAIL") + " order is processed.  (A coastal " +
			"region is defined as a non-ocean region with at least one "
			"adjacent ocean region.)";
		f.Paragraph(temp);
		temp = AString("Note that a unit on board a ship while it is ") +
			"sailing may not " + f.Link("#move", "MOVE") +
			" later in the turn, even if he doesn't issue the " +
			f.Link("#sail", "SAIL");
		temp += " order; sailing is considered to take the whole month. "
			"Also, units may not remain on guard while on board a sailing "
			"ship; they will have to reissue the ";
		temp += f.Link("#guard", "GUARD") +  " 1 order to guard a " +
			"region after sailing.";
		f.Paragraph(temp);
		temp = AString("Ships get ") + NumToWord(Globals->SHIP_SPEED);
		temp += AString(" movement point") + (Globals->SHIP_SPEED==1?"":"s");
		temp += " per turn.  A ship can move from an ocean region to "
			"another ocean region, or from a coastal region to an ocean "
			"region, or from an ocean region to a coastal region.";
		if (Globals->PREVENT_SAIL_THROUGH) {
			temp += " Ships may not sail through single hex land masses "
				"and must leave via the same side they entered or a side "
				"adjacent to that one.";
			if (Globals->ALLOW_TRIVIAL_PORTAGE) {
				temp += " Ships ending their movement in a land hex may "
					"sail out along any side connecting to water.";
			}
		}
		temp += " Ships can only be constructed in coastal regions. For a "
			"ship to enter any region only costs one movement point; the "
			"cost of two movement points for entering, say, a forest "
			"coastal region, does not apply.";
		if(Globals->WEATHER_EXISTS) {
			temp += " Ships do, however, only get half movement points "
				"during the winter months (or monsoon months in the "
				"tropical latitudes).";
		}
		f.Paragraph(temp);
		temp = "A ship can only move if the total weight of everything "
			"aboard does not exceed the ship's capacity.  (The rules do "
			"not prevent an overloaded ship from staying afloat, only "
			"from moving.)  Also, there must be enough sailors aboard "
			"(using the ";
		temp += f.Link("#sail", "SAIL") + " order), to sail the ship, or ";
		temp += "it will not go anywhere.  Note that the sailing skill "
			"increases the usefulness of a unit proportionally; thus, a "
			"1 man unit with level 5 sailing skill can sail a longboat "
			"alone.  (See the section on skills for further details on "
			"skills.)  The capacities (and costs in labor units) of the "
			"various basic ship types are as follows:";
		f.Paragraph(temp);
		f.LinkRef("tableshipcapacities");
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TD", "&nbsp;");
		f.TagText("TH", "Capacity");
		f.TagText("TH", "Cost");
		f.TagText("TH", "Sailors");
		f.Enclose(0, "TR");
		for(i = 0; i < NOBJECTS; i++) {
			if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if(!ObjectIsShip(i)) continue;
			if(ItemDefs[ObjectDefs[i].item].flags & ItemType::DISABLED)
				continue;
			int normal = (ItemDefs[ObjectDefs[i].item].type & IT_NORMAL);
			normal |= (ItemDefs[ObjectDefs[i].item].type & IT_TRADE);
			if(!normal) continue;
			f.Enclose(1, "TR");
			f.Enclose(1, "TD ALIGN=CENTER");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER");
			f.PutStr(ObjectDefs[i].capacity);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER");
			f.PutStr(ObjectDefs[i].cost);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER");
			f.PutStr(ObjectDefs[i].sailors);
			f.Enclose(0, "TD");
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
	}
	f.LinkRef("movement_order");
	f.TagText("H3", "Order of Movement:");
	temp = "This section is probably unimportant to beginning players, but "
		"it can be helpful for more experienced players.";
	f.Paragraph(temp);
	temp = "Normal movement in Atlantis, meaning ";
	temp += f.Link("#advance", "ADVANCE") + " and " + f.Link("#move", "MOVE");
	temp += " orders, is processed one hex of movement at a time, region "
		"by region. So, Atlantis cycles through all of the regions; for "
		"each region, it finds any units that wish to move, and moves "
		"them (if they can move) one hex (and only one hex). After "
		"processing one such region, it initiates any battles that take "
		"place due to these movements, and then moves on to the next "
		"region. After it has gone through all of the regions, you will "
		"note that units have only moved one hex, so it goes back and "
		"does the whole process again, except this time moving units "
		"their second hex (if they have enough movement points left). This "
		"continues until no units can move anymore.";
	f.Paragraph(temp);
	if(may_sail) {
		temp = "Sailing is handled differently; Atlantis cycles through all "
			"of the ships in Atlantis, moving them one at a time. When "
			"Atlantis sails a ship, it sails it through its entire course, "
			"either to the end, or until the ship enters a hex guarded "
			"against some unit on the ship, and then moves onto the next "
			"ship.";
		f.Paragraph(temp);
	}
	temp = "Note that";
	if(may_sail) {
		temp += " in either case,";
	}
	temp += " the order in which the regions are processed is undefined "
		"by the rules. The computer generally does them in the same "
		"order every time, but it is up to the wiles of the player to "
		"determine (or not) these patterns. The order in which units or "
		"ships are moved within a region is the order that they appear "
		"on a turn report.";
	f.Paragraph(temp);
	f.LinkRef("skills");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Skills");

	temp = "The most important thing distinguishing one character from "
		"another in Atlantis is skills.  The following skills are "
		"available: ";
	int comma = 0;
	int found = 0;
	last = -1;
	for(i = 0; i < NSKILLS; i++) {
		if(SkillDefs[i].flags & SkillType::DISABLED) continue;
		if(SkillDefs[i].flags & SkillType::APPRENTICE) continue;
		if(SkillDefs[i].flags & SkillType::MAGIC) continue;
		found = 0;
		for(j = 0; j < 3; j++) {
			k = SkillDefs[i].depends[j].skill;
			if(k != -1 && !(SkillDefs[k].flags & SkillType::DISABLED)) {
				found = 1;
				break;
			}
		}
		if(found) continue;
		if(last == -1) {
			last = i;
			continue;
		}

		temp += SkillDefs[last].name;
		temp += ", ";
		last = i;
		comma++;
	}
	if(last != -1) {
		if(comma) temp += "and ";
		temp += SkillDefs[last].name;
	}

	temp += ". When a unit possesses a skill, he also has a skill level "
		"to go with it.  Generally, the effectiveness of a skill is "
		"directly proportional to the skill level involved, so a unit with "
		"level 2 in a skill is twice as good as a unit with level 1 in the "
		"same skill.";
	f.Paragraph(temp);
	f.LinkRef("skills_limitations");
	f.TagText("H3", "Limitations:");
	if (Globals->LEADERS_EXIST && Globals->SKILL_LIMIT_NONLEADERS) {
		temp = "A unit made up of leaders may know one or more skills; "
			"for the rest of this section, the word \"leader\" will refer "
			"to such a unit.  Other units, those which contain "
			"non-leaders, will be refered to as normal units. A normal "
			"unit may only know one skill.";
		f.Paragraph(temp);
	}
	if(!Globals->RACES_EXIST) {
		if(Globals->SKILL_LIMIT_NONLEADERS) {
			temp = "A unit may only learn one skill. ";
		} else {
			temp = "A unit may learn as many skills as it requires. ";
		}
		temp += "Skills can be learned up to a maximum level of ";
		temp += ManDefs[MAN_MAN].defaultlevel;
		temp += ".";
		f.Paragraph(temp);
	}

	if (Globals->RACES_EXIST) {
		temp = "Skills may be learned up to a maximum level depending on "
			"the race of the studying unit (remembering that for units "
			"containing more than one race, the maximum is determined by "
			"the least common denominator).  Every race has a normal "
			"maximum skill level, and  a list of skills that they "
			"specialize in, and can learn up to higher level. ";
		if(Globals->LEADERS_EXIST) {
			temp += "Leaders, being more powerful, can learn skills to "
				"even higher levels. ";
		}
		temp += "Here is a list of the races (including leaders) and the "
			"information on normal skill levels and specialized skills.";
		f.Paragraph(temp);
		f.LinkRef("tableraces");
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TH", "Race/Type");
		f.TagText("TH", "Specilized Skills");
		f.TagText("TH", "Max Level (specialized skills)");
		f.TagText("TH", "Max Level (non-specialized skills)");
		f.Enclose(0, "TR");
		for(i = 0; i < NITEMS; i++) {
			if(ItemDefs[i].flags & ItemType::DISABLED) continue;
			if(!(ItemDefs[i].type & IT_MAN)) continue;
			f.Enclose(1, "TR");
			int m = ItemDefs[i].index;
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ItemDefs[i].names);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			int spec = 0;
			comma = 0;
			temp = "";
			for(j = 0; j < (int)(sizeof(ManDefs->skills) /
						 sizeof(ManDefs->skills[0])); j++) {
				if(ManDefs[m].skills[j] < 0) continue;
				if(SkillDefs[ManDefs[m].skills[j]].flags & SkillType::DISABLED)
					continue;
				spec = 1;
				if(comma) temp += ", ";
				temp += SkillDefs[ManDefs[m].skills[j]].name;
				comma++;
			}
			if(!spec) temp = "None.";
			f.PutStr(temp);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			if(spec)
				f.PutStr(ManDefs[m].speciallevel);
			else
				f.PutStr("--");
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ManDefs[m].defaultlevel);
			f.Enclose(0, "TD");
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
	}
	temp = "If units are merged together, their skills are averaged out. "
		"No rounding off is done; rather, the computer keeps track for each "
		"unit of how many total months of training that unit has in each "
		"skill. When units are split up, these months are divided as evenly "
		"as possible among the people in the unit; but no months are ever "
		"lost.";
	f.Paragraph(temp);
	f.LinkRef("skills_studying");
	f.TagText("H3", "Studying:");
	temp = "For a unit to gain level 1 of a skill, they must gain one "
		"months worth of training in that skill.  To raise this skill level "
		"to 2, the unit must add an additional two months worth of "
		"training.  Then, to raise this to skill level 3 requires another "
		"three months worth of training, and so forth.  A month of "
		"training is gained when a unit uses the ";
	temp += f.Link("#study", "STUDY") + " order.  Note that study months "
		"do not need to be consecutive; for a unit to go from level 1 to "
		"level 2, he can study for a month, do something else for a month, "
		"and then go back and complete his second month of study.";
	if (Globals->SKILL_PRACTISE_AMOUNT > 0) {
		temp += "  A unit can also increase its level of training by "
			"using a skill.  This progress is ";
		if (Globals->SKILL_PRACTISE_AMOUNT < 11)
			temp += "much slower than";
		else if (Globals->SKILL_PRACTISE_AMOUNT < 30)
			temp += "slower than";
		else if (Globals->SKILL_PRACTISE_AMOUNT == 30)
			temp += "the same as";
		else if (Globals->SKILL_PRACTISE_AMOUNT < 61)
			temp += "faster than";
		else
			temp += "much faster than";
		temp += " studying.  Only one skill can be improved through "
			"practice in any month; if multiple skills are used, only the "
			"first will be improved.  A skill will only improve with "
			"practice if the unit has first studied the rudiments of the "
			"skill.";
	}
	f.Paragraph(temp);
	// XXX -- This is not as nice as it could be and could cause problems
	// if the skills are given disparate costs.   This should probably be
	// a table of all skills/costs.
	temp = "Most skills cost $";
	temp += SkillDefs[S_COMBAT].cost;
	temp += " per person per month to study (in addition to normal "
		"maintenance costs).  The exceptions are ";
	if(has_stea || has_obse) {
		if(has_stea) temp += "Stealth";
		if(has_obse) {
			if(has_stea) temp += " and ";
			temp += "Observation";
		}
		temp += " (";
		if(has_stea && has_obse)
			temp += "both of which cost $";
		else
			temp += "which costs $";
		temp += SkillDefs[S_STEALTH].cost;
		temp += "), ";
	}
	temp += "Magic skills (which cost $";
	temp += SkillDefs[S_FORCE].cost;
	temp += ")";
	if(!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED)) {
		temp += ", and Tactics (which costs $";
		temp += SkillDefs[S_TACTICS].cost;
		temp += ")";
	}
	temp += ".";
	f.Paragraph(temp);
	f.LinkRef("skills_teaching");
	f.TagText("H3", "Teaching:");
	temp = AString("A unit with a teacher can learn up to twice as fast ") +
		"as normal. The " + f.Link("#teach", "TEACH") + " order is used to ";
	temp += "spend the month teaching one or more other units (your own or "
		"another factions).  The unit doing the teaching must have a skill "
		"level greater than the unit doing the studying.  (Note: for all "
		"skill uses, it is skill level, not number of months of training, "
		"that counts. Thus, a unit with 1 month of training is effectively "
		"the same as a unit with 2 months of training, since both have a "
		"skill level of 1.)  The units being taught simply issue the ";
	temp += f.Link("#study", "STUDY") + " order normally (also, his faction "
		"must be declared Friendly by the teaching faction).  Each person "
		"can only teach up to " + Globals->STUDENTS_PER_TEACHER +
		" student" + (Globals->STUDENTS_PER_TEACHER == 1?"":"s") + " in a ";
	temp += "month; additional students dilute the training.  Thus, if 1 "
		"teacher teaches ";
	temp += AString(2*Globals->STUDENTS_PER_TEACHER) +
		" men, each man being taught will gain 1 1/2 months of training, "
		"not 2 months.";
	f.Paragraph(temp);
	temp = "Note that it is quite possible for a single unit to teach two "
		"or more other units different skills in the same month, provided "
		"that the teacher has a higher skill level than each student in "
		"the skill that that student is studying, and that there are no "
		"more than ";
	temp += AString(Globals->STUDENTS_PER_TEACHER) + " student";
	temp += (Globals->STUDENTS_PER_TEACHER == 1 ? "" : "s");
	temp += " per teacher.";
	f.Paragraph(temp);
	if(Globals->LEADERS_EXIST) {
		temp = "Note: Only leaders may use the ";
		temp += f.Link("#teach", "TEACH") + " order.";
		f.Paragraph(temp);
	}
	f.LinkRef("skills_skillreports");
	f.TagText("H3", "Skill Reports:");
	temp = "When a faction learns a new skill level for this first time, it "
		"will be given a report on special abilities that a unit with this "
		"skill level has. This report can be shown again at any time (once "
		"a faction knows the skill), using the ";
	temp += f.Link("#show", "SHOW") + " order. For example, when a faction ";
	temp += "learned the skill Shoemaking level 3 for the first time, it "
		"might receive the following (obviously farsical) report:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "PRE");
	f.ClearWrapTab();
	f.WrapStr("Shoemaking [SHOE] 3: A unit with this skill may PRODUCE "
			"Sooper Dooper Air Max Winged Sandals.");
	f.Enclose(0, "PRE");
	f.LinkRef("economy");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "The Economy");
	temp = "The unit of currency in Atlantis is the silver piece. Silver "
		"is a normal item, with zero weight, appearing in your unit's "
		"reports. Silver is used for such things as buying items, and "
		"unit's maintenance.";
	f.Paragraph(temp);
	f.LinkRef("economy_maintenance");
	f.TagText("H3", "Maintenance Costs:");
	temp = "IMPORTANT:  Each and every character in Atlantis requires a "
		"maintenance fee each month. Anyone who ends the month without "
		"this maintenance cost has a ";
	temp += Globals->STARVE_PERCENT;
	temp += " percent chance of ";
	if(Globals->SKILL_STARVATION != GameDefs::STARVE_NONE) {
		temp += "starving, leading to the following effects:";
		f.Paragraph(temp);
		f.Enclose(1, "UL");
		f.Enclose(1, "LI");
		if(Globals->SKILL_STARVATION == GameDefs::STARVE_MAGES)
			temp = "If the unit is a mage, it";
		else if(Globals->SKILL_STARVATION == GameDefs::STARVE_LEADERS)
			temp = "If the unit is a leader, it";
		else
			temp = "A unit";
		temp += " will lose a skill level in some of its skills.";
		f.PutStr(temp);
		f.Enclose(0, "LI");
		if(Globals->SKILL_STARVATION != GameDefs::STARVE_ALL) {
			f.Enclose(1, "LI");
			f.PutStr("Otherwise, it will starve to death.");
			f.Enclose(0, "LI");
		}
		f.Enclose(1, "LI");
		f.PutStr("If a unit should forget a skill level and it knows none, "
				"it will starve to death.");
		f.Enclose(0, "LI");
		f.Enclose(0, "UL");
		temp = "";
	} else {
		temp += "starving to death. ";
	}
	temp += "It is up to you to make sure that your people have enough money ";
	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
		temp += "and food ";
	temp +=	"available. Money ";
	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
		temp += "and food ";
	temp += "will be shared automatically between your units "
		"in the same region, if one is starving and another has more than "
		"enough; but this will not happen between units in different "
		"regions (this sharing of money applies only for maintenance costs, "
		"and does not occur for other purposes). If you have silver in your "
		"unclaimed fund, then that silver will be automatically claimed by "
		"units that would otherwise starve. ";
	if (Globals->UPKEEP_MINIMUM_FOOD && Globals->ALLOW_WITHDRAW) {
		temp += "Similarly, food will automatically be ";
		temp +=	f.Link("#withdraw", "withdraw");
		temp += "n if needed and unclaimed funds are available. ";
	}
	temp += "Lastly, if a faction is allied to yours, their units will "
		"provide surplus cash ";
	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
		temp += "or food ";
	temp += "to your units for maintenance, as a last resort.";
	f.Paragraph(temp);
	temp = "";
	if(Globals->MULTIPLIER_USE == GameDefs::MULT_NONE) {
		temp += AString("This fee is generally ") + Globals->MAINTENANCE_COST +
			" silver for a normal character";
		if (Globals->LEADERS_EXIST) {
			temp += AString(", and ") + Globals->LEADER_COST +
				" silver for a leader";
		}
	} else {
		if(Globals->MULTIPLIER_USE == GameDefs::MULT_MAGES) {
			temp += "Mages ";
		} else if(Globals->MULTIPLIER_USE==GameDefs::MULT_LEADERS &&
				Globals->LEADERS_EXIST) {
			temp += "Leaders ";
		} else {
			temp += "All units ";
		}
		temp += "pay a fee based on the number of skill levels the character "
			"has.  This fee is the maximum of $";
		temp += AString(Globals->MAINTENANCE_MULTIPLIER) + " per skill level";
		temp += " and a cost of $";
		temp += AString(Globals->MAINTENANCE_COST) + " for normal characters";
		temp += AString(" or $") + Globals->LEADER_COST + " for leaders";
		if(Globals->MULTIPLIER_USE != GameDefs::MULT_ALL) {
			temp += ". All other characters pay a fee of ";
			temp += Globals->MAINTENANCE_COST;
			temp += " silver for a normal character";
			if (Globals->LEADERS_EXIST) {
				temp += ", and ";
				temp += Globals->LEADER_COST;
				temp += " silver for a leader";
			}
		}
	}
	temp += ".";
	if (Globals->FOOD_ITEMS_EXIST) {
		temp += " Units may substitute one unit of grain, livestock, or "
			"fish for each ";
		temp += Globals->UPKEEP_FOOD_VALUE;
		temp += " silver of maintenance owed. ";
		if (Globals->UPKEEP_MINIMUM_FOOD > 0) {
			temp += "A unit must be given at least ";
			temp +=	Globals->UPKEEP_MINIMUM_FOOD;
			temp += " maintenance per man in the form of food. ";
		}
		if (Globals->UPKEEP_MAXIMUM_FOOD >= 0) {
			temp += "At most ";
			temp += Globals->UPKEEP_MAXIMUM_FOOD;
			temp += " silver worth of food can be counted against each "
					"man's maintenance. ";
		}
		temp += "A unit may use the ";
		temp += f.Link("#consume", "CONSUME") + " order to specify that it "
			"wishes to use food items in preference to silver.  Note that ";
		temp += "these items are worth more when sold in towns, so selling "
			"them and using the money is more economical than using them for "
			"maintenance.";
	};
	f.Paragraph(temp);
	f.LinkRef("economy_recruiting");
	f.TagText("H3", "Recruiting:");
	temp = "People may be recruited in a region.  The total amount of "
		"recruits available per month in a region, and the amount that must "
		"be paid per person recruited, are shown in the region description. "
		"The ";
	temp += f.Link("#buy", "BUY") + " order is used to recruit new people. ";
	temp += "New recruits will not have any skills or items.  Note that the "
		"process of recruiting a new unit is somewhat counterintuitive; it "
		"is necessary to ";
	temp += f.Link("#form", "FORM")+" an empty unit, ";
	temp += f.Link("#give", "GIVE")+" the empty unit some money, and have it ";
	temp += f.Link("#buy", "BUY") + " people; see the description of the ";
	temp += f.Link("#form", "FORM")+ " order for further details.";
	f.Paragraph(temp);
	f.LinkRef("economy_items");
	f.TagText("H3", "Items:");
	temp = "A unit may have a number of possessions, referred to as "
		"\"items\".  Some details were given above in the section on "
		"Movement, but many things were left out. Here is a table giving "
		"some information about common items in Atlantis:";
	f.Paragraph(temp);
	f.LinkRef("tableiteminfo");
	f.Enclose(1, "CENTER");
	f.Enclose(1, "TABLE BORDER=1");
	f.Enclose(1, "TR");
	f.TagText("TD", "&nbsp;");
	f.TagText("TH", "Skill (min level)");
	f.TagText("TH", "Material");
	f.TagText("TH", "Production time");
	f.TagText("TH", "Weight (capacity)");
	f.TagText("TH", "Extra Information");
	f.Enclose(0, "TR");
	for(i = 0; i < NITEMS; i++) {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(!(ItemDefs[i].type & IT_NORMAL)) continue;
		j = ItemDefs[i].pSkill;
		if(j != -1 && (SkillDefs[j].flags & SkillType::DISABLED)) continue;
		last = ItemDefs[i].pSkill;
		if(last != -1 && (SkillDefs[last].flags & SkillType::DISABLED))
			continue;
		last = 0;
		for(j = 0; j < (int) (sizeof(ItemDefs->pInput) /
						sizeof(ItemDefs->pInput[0])); j++) {
			k = ItemDefs[i].pInput[j].item;
			if(k != -1 &&
					!(ItemDefs[k].flags & ItemType::DISABLED) &&
					!(ItemDefs[k].type & IT_NORMAL))
				last = 1;
		}
		if(last == 1) continue;
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ItemDefs[i].name);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		if(ItemDefs[i].pSkill != -1) {
			temp = SkillDefs[ItemDefs[i].pSkill].name;
			temp += AString(" (") + ItemDefs[i].pLevel + ")";
			f.PutStr(temp);
		}
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		comma = 0;
		temp = "";
		if (ItemDefs[k].flags & ItemType::ORINPUTS)
			temp = "Any of : ";
		for(j = 0; j < (int) (sizeof(ItemDefs->pInput) /
						sizeof(ItemDefs->pInput[0])); j++) {
			k = ItemDefs[i].pInput[j].item;
			if(k < 0 || (ItemDefs[k].flags&ItemType::DISABLED))
				continue;
			if(comma) temp += ", ";
			temp += ItemDefs[i].pInput[j].amt;
			temp += " ";
			if(ItemDefs[i].pInput[j].amt > 1)
				temp += ItemDefs[k].names;
			else
				temp += ItemDefs[k].name;
			comma = 1;
		}
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		if(ItemDefs[i].pMonths) {
			temp = ItemDefs[i].pMonths;
			temp += AString(" month") + (ItemDefs[i].pMonths == 1 ? "" : "s");
		} else {
			temp = "&nbsp;";
		}
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		temp = ItemDefs[i].weight;
		cap = ItemDefs[i].walk - ItemDefs[i].weight;
		if(ItemDefs[i].walk || (ItemDefs[i].hitchItem != -1)) {
			temp += " (";
			if(ItemDefs[i].hitchItem == -1)
				temp += cap;
			else {
				temp += (cap + ItemDefs[i].hitchwalk);
				temp += " with ";
				temp += ItemDefs[ItemDefs[i].hitchItem].name;
			}
			temp += ")";
		}
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT");
		temp = "";
		if(ItemDefs[i].type & IT_WEAPON) {
			WeaponType *wp = &WeaponDefs[ItemDefs[i].index];
			if(wp->attackBonus || wp->defenseBonus ||
					(wp->flags & WeaponType::RANGED) ||
					(wp->flags & WeaponType::NEEDSKILL)) {
				if(wp->flags & WeaponType::RANGED)
					temp += "Ranged weapon";
				else
					temp += "Weapon";
				temp += " which gives ";
				if(wp->attackBonus > -1) temp += "+";
				temp += wp->attackBonus;
				temp += " on attack";
				temp += " and ";
				if(wp->defenseBonus > -1) temp += "+";
				temp += wp->defenseBonus;
				temp += " on defense";
			    if(wp->flags & WeaponType::NEEDSKILL &&
						!(SkillDefs[wp->baseSkill].flags&SkillType::DISABLED)){
					temp += " (needs ";
					temp += SkillDefs[wp->baseSkill].name;
					temp += " skill)";
				}
				temp += ".<BR>";
			}
			if(wp->numAttacks < 0) {
				temp += "Gives 1 attack every ";
				temp += -(wp->numAttacks);
				temp += " rounds.<BR>";
			}
		}
		if(ItemDefs[i].type & IT_MOUNT) {
			MountType *mp = &MountDefs[ItemDefs[i].index];
			if(mp->skill > 0 &&
					!(SkillDefs[mp->skill].flags & SkillType::DISABLED)) {
				temp += "Gives a riding bonus with the ";
				temp += SkillDefs[mp->skill].name;
				temp += " skill.<BR>";
			}
		}
		if(ItemDefs[i].type & IT_ARMOR) {
			ArmorType *at = &ArmorDefs[ItemDefs[i].index];
			temp += "Gives a ";
			temp += at->saves[SLASHING];
			temp += " in ";
			temp += at->from;
			temp += " chance to survive a normal hit.<BR>";
			if((at->flags & ArmorType::USEINASSASSINATE) && has_stea) {
				temp += "May be used during assassinations.<BR>";
			}
		}
		if(ItemDefs[i].type & IT_TOOL) {
			for(j = 0; j < NITEMS; j++) {
				if(ItemDefs[j].flags & ItemType::DISABLED) continue;
				if(ItemDefs[j].mult_item != i) continue;
				if(!(ItemDefs[j].type & IT_NORMAL)) continue;
				k = ItemDefs[j].pSkill;
				if(k != -1 && (SkillDefs[k].flags & SkillType::DISABLED))
					continue;
				last = 0;
				for(k = 0; k < (int) (sizeof(ItemDefs->pInput) /
						sizeof(ItemDefs->pInput[0])); k++) {
					l = ItemDefs[j].pInput[k].item;
					if(l != -1 &&
							!(ItemDefs[l].flags & ItemType::DISABLED) &&
							!(ItemDefs[l].type & IT_NORMAL))
						last = 1;
				}
				if(last == 1) continue;
				temp += AString("+") + ItemDefs[j].mult_val +
					" bonus when producing " + ItemDefs[j].names + ".<BR>";
			}
		}
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Enclose(0, "CENTER");
	temp = "All items except silver and trade goods are produced with the ";
	temp += f.Link("#produce", "PRODUCE") + " order.";
	if(!(ItemDefs[I_SWORD].flags & ItemType::DISABLED)) {
		last = -1;
		temp2 = "";
		for(i = 0; i < (int) (sizeof(ItemDefs->pInput) /
				sizeof(ItemDefs->pInput[0])); i++) {
			j = ItemDefs[I_SWORD].pInput[i].item;
			if(j == -1) continue;
			if(ItemDefs[j].flags & ItemType::DISABLED) continue;
			if(last == -1) {
				last = j;
				continue;
			}
			temp2 += ItemDefs[last].names;
			temp2 += " and ";
			last = j;
		}
		if(last != -1) temp2 += ItemDefs[last].names;
		j = ItemDefs[I_SWORD].pSkill;
		if(last != -1 && !(SkillDefs[j].flags & SkillType::DISABLED)) {
			temp += " Example: PRODUCE SWORDS will produce as many swords "
				"as possible during the month, provided that the unit has "
				"adequate supplies of ";
			temp += temp2;
			temp += " and has the ";
			temp += SkillDefs[j].name;
			temp += " skill. Required skills and raw materials are in the "
				"table above.";
		}
	}
	f.Paragraph(temp);
	temp = "If an item requires raw materials, then the specified "
		"amount of each material is consumed for each item produced. ";
	if(!(ItemDefs[I_LONGBOW].flags & ItemType::DISABLED)) {
		last = -1;
		temp2 = "";
		k = 0;
		for(i = 0; i < (int) (sizeof(ItemDefs->pInput) /
				sizeof(ItemDefs->pInput[0])); i++) {
			j = ItemDefs[I_LONGBOW].pInput[i].item;
			if(j == -1) continue;
			if(ItemDefs[j].flags & ItemType::DISABLED) continue;
			if(last == -1) {
				k = ItemDefs[I_LONGBOW].pInput[i].amt;
				last = j;
				continue;
			}
			temp2 += AString(5*k);
			temp2 += " units of ";
			temp2 += ItemDefs[last].names;
			temp2 += " and ";
			last = j;
			k = ItemDefs[I_LONGBOW].pInput[i].amt;
		}
		if(last != -1) {
			temp2 += AString(5*k);
			temp2 += " units of ";
			temp2 += ItemDefs[last].names;
		}
		j = ItemDefs[I_LONGBOW].pSkill;
		if(last != -1 && !(SkillDefs[j].flags & SkillType::DISABLED)) {
			temp += "Thus to produce 5 longbows (a supply of arrows is "
				"assumed to be included with the bow), ";
			temp += temp2;
			temp += " are required. ";
		}
	}
	temp += "The higher ones skill, the more productive each man-month "
		"of work";
	if(!(ItemDefs[I_LONGBOW].flags & ItemType::DISABLED) &&
			(ItemDefs[I_LONGBOW].pMonths == 1) &&
			(ItemDefs[I_LONGBOW].pOut == 1)) {
		temp += "; thus, 5 longbows could be produced by a 5-man unit of "
			"skill 1, or a 1-man unit of skill 5.";
		if(!(ItemDefs[I_PLATEARMOR].flags & ItemType::DISABLED) &&
			(ItemDefs[I_PLATEARMOR].pMonths==ItemDefs[I_PLATEARMOR].pLevel)) {
			temp += " (Plate armor is an exception; a unit must have skill ";
			temp += ItemDefs[I_PLATEARMOR].pLevel;
			temp += " to be able to produce it at all, and each man can only "
				"produce 1 plate armor per month.";
			last = -1;
			temp2 = "";
			k = 0;
			for(i = 0; i < (int) (sizeof(ItemDefs->pInput) /
					sizeof(ItemDefs->pInput[0])); i++) {
				j = ItemDefs[I_PLATEARMOR].pInput[i].item;
				if(j == -1) continue;
				if(ItemDefs[j].flags & ItemType::DISABLED) continue;
				if(last == -1) {
					last = j;
					k = ItemDefs[I_PLATEARMOR].pInput[i].amt;
					continue;
				}
				temp2 += AString(k);
				temp2 += " units of ";
				temp2 += ItemDefs[last].names;
				temp2 += " and ";
				last = j;
				k = ItemDefs[I_PLATEARMOR].pInput[i].amt;
			}
			if(last != -1) {
				temp2 += AString(k);
				temp2 += " units of ";
				temp2 += ItemDefs[last].names;
			}
			j = ItemDefs[I_PLATEARMOR].pSkill;
			if(last != -1 && !(SkillDefs[j].flags & SkillType::DISABLED)) {
				temp += " Plate armor also takes ";
				temp += temp2;
				temp += " to produce.";
			}
			temp += ")";
		}
	} else {
		temp += ".";
	}
	if (!(ItemDefs[I_FOOD].flags & ItemType::DISABLED)) {
		temp += " Production of food works slightly differently; each worker "
				"will only use one unit of material, but will produce "
				"a number of units of food equal to their skill level.";
	}

	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		temp += " Only Trade factions can issue ";
		temp += f.Link("#produce", "PRODUCE") + " orders however, regardless "
			"of skill levels.";
	}
	f.Paragraph(temp);
	temp = "Items which increase production may increase production of "
		"advanced items in addition to the basic items listed.  Some of "
		"them also increase production of other tools.  Read the skill "
		"descriptions for details on which tools aid which production when "
		"not noted above.";
	f.Paragraph(temp);
	temp = "If an item does not list a raw material it may be produced "
		"directly from the land. Each region generally has at least one item "
		"that can be produced there.  Shown on the description of a region "
		"is a list of the items that can be produced, and the amount of "
		"each that can be produced per month.  This amount depends on the "
		"region type. ";
	if(Globals->RANDOM_ECONOMY) {
		temp += "It also varies from region to region of the same type. ";
	}
	temp += "If the units in a region attempt to produce more of a commodity "
		"than can be produced that month, then the amount available is "
		"distributed among the producers";
	f.Paragraph(temp);
	if(Globals->TOWNS_EXIST) {
		f.LinkRef("economy_towns");
		f.TagText("H3", "Villages, Towns, and Cities:");
		temp = "Some regions in Atlantis contain villages, towns, and "
			"cities.  Villages add to the wages, population, and tax income "
			"of the region they are in. ";
		if(Globals->FOOD_ITEMS_EXIST) {
			temp += "Also, villages will have an additional market for "
				"grain, livestock, and fish. ";
		}
		temp += "As the village's demand for these goods is met, the "
			"population will increase. When the population reaches a "
			"certain theshold, the village will turn into a town.  A "
			"town will have some additional products that it demands, "
			"in addition to what it previously wanted.  Also a town "
			"will sell some new items as well. A town whose demands are "
			"being met will grow, and above another threshold it will "
			"become a full-blown city.  A city will have additional "
			"markets for common items, and will also have markets for "
			"less common, more expensive trade items.";
		f.Paragraph(temp);
		temp = "Trade items are bought and sold only by cities, and have "
			"no other practical uses.  However, the profit margins on "
			"these items are usually quite high. ";
		if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			temp += "Buying of trade items in a region counts against a "
				"Trade faction's quota of regions in which it may "
				"undertake trade activity (note that buying and selling "
				"normal items does not, nor does selling of Trade items).";
		}
		f.Paragraph(temp);
	}
	f.LinkRef("economy_buildings");
	f.TagText("H3", "Buildings and Trade Structures:");
	temp = "Construction of buildings ";
	if(may_sail) temp += "and ships ";
	temp += "goes as follows: each unit of work on a building requires a "
		"unit of the required resource and a man-month of work by a "
		"character with the appropriate skill and level; higher skill "
		"levels allow work proceed faster still using one unit of the "
		"required resource per unit of work done). ";
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		temp += "Again, only Trade factions can issue ";
		temp += f.Link("#build", "BUILD") + " orders. ";
	}
	temp += "Here is a table of the various building types:";
	f.Paragraph(temp);
	f.LinkRef("tablebuildings");
	f.Enclose(1, "CENTER");
	f.Enclose(1, "TABLE BORDER=1");
	f.Enclose(1, "TR");
	f.TagText("TD", "");
	f.TagText("TH", "Size");
	f.TagText("TH", "Cost");
	f.TagText("TH", "Material");
	f.TagText("TH", "Skill (min level)");
	f.Enclose(0, "TR");
	for(i = 0; i < NOBJECTS; i++) {
		if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
		if(!ObjectDefs[i].protect) continue;
		j = ObjectDefs[i].skill;
		if(j == -1) continue;
		if(SkillDefs[j].flags & SkillType::MAGIC) continue;
		if(ObjectIsShip(i)) continue;
		j = ObjectDefs[i].item;
		if(j == -1) continue;
		/* Need the >0 since item could be WOOD_OR_STONE (-2) */
		if(j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
		if(j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
		/* Okay, this is a valid object to build! */
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ObjectDefs[i].name);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ObjectDefs[i].protect);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ObjectDefs[i].cost);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		if(j == I_WOOD_OR_STONE)
			temp = "wood or stone";
		else
			temp = ItemDefs[j].name;
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		temp = SkillDefs[ObjectDefs[i].skill].name;
		temp += AString(" (") + ObjectDefs[i].level + ")";
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Enclose(0, "CENTER");
	temp = "Size is the number of people that the building can shelter. Cost "
		"is both the number of man-months of labor and the number of units "
		"of material required to complete the building.  There are possibly "
		"other buildings which can be built that require more advanced "
		"resources, or odd skills to construct.   The description of a skill "
		"will include any buildings which it allows to be built.";
	f.Paragraph(temp);
	temp = "There are other structures that increase the maximum production "
		"of certain items in regions";
	if(!(ObjectDefs[O_MINE].flags & ObjectType::DISABLED))
		temp += "; for example, a Mine will increase the amount of iron "
			"that is available to be mined in a region";
	temp += ".  To construct these structures requires a high skill level in "
		"the production skill related to the item that the structure will "
		"help produce. ";
	if(!(ObjectDefs[O_INN].flags & ObjectType::DISABLED)) {
		temp += "(Inns are an exception to this rule, requiring the Building "
			"skill, not the Entertainment skill.) ";
	}
	temp += "This bonus in production is available to any unit in the "
		"region; there is no need to be inside the structure.";
	f.Paragraph(temp);
	temp = "The first structure built in a region will increase the maximum "
		"production of the related product by 25%; the amount added by each "
		"additional structure will be half of the the effect of the previous "
		"one.  (Note that if you build enough of the same type of structure "
		"in a region, the new structures may not add _any_ to the production "
		"level).";
	f.Paragraph(temp);
	f.LinkRef("tabletradestructures");
	f.Enclose(1, "CENTER");
	f.Enclose(1, "TABLE BORDER=1");
	f.Enclose(1, "TR");
	f.TagText("TD", "");
	f.TagText("TH", "Cost");
	f.TagText("TH", "Material");
	f.TagText("TH", "Skill (level)");
	f.TagText("TH", "Production Aided");
	f.Enclose(0, "TR");
	for(i = 0; i < NOBJECTS; i++) {
		if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
		if(ObjectDefs[i].protect) continue;
		if(ObjectIsShip(i)) continue;
		j = ObjectDefs[i].productionAided;
		if(j == -1) continue;
		if(ItemDefs[j].flags & ItemType::DISABLED) continue;
		if(!(ItemDefs[j].type & IT_NORMAL)) continue;
		j = ObjectDefs[i].skill;
		if(j == -1) continue;
		if(SkillDefs[j].flags & SkillType::MAGIC) continue;
		j = ObjectDefs[i].item;
		if(j == -1) continue;
		/* Need the >0 since item could be WOOD_OR_STONE (-2) */
		if(j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
		if(j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
		/* Okay, this is a valid object to build! */
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ObjectDefs[i].name);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ObjectDefs[i].cost);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		if(j == I_WOOD_OR_STONE)
			temp = "wood or stone";
		else
			temp = ItemDefs[j].name;
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		temp = SkillDefs[ObjectDefs[i].skill].name;
		if(ObjectDefs[i].level > 1)
			temp += AString(" (") + ObjectDefs[i].level + ")";
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		if(ObjectDefs[i].productionAided == I_SILVER)
			f.PutStr("entertainment");
		else
			f.PutStr(ItemDefs[ObjectDefs[i].productionAided].names);
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Enclose(0, "CENTER");
	temp = "Note that these structures will not increase the availability "
		"of an item in a region which does not already have that item "
		"available. Also, Trade structures do not offer defensive bonuses "
		"(which is why they do not have a size associated with them).  As "
		"with regular buildings, the Cost is the number of man-months of "
		"labor and also the number of units of raw material required to "
		"complete the structure. ";
	if(!(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
			!(ItemDefs[I_STONE].flags & ItemType::DISABLED)) {
		temp += "You can use two different materials (wood or stone) to "
			"construct most trade structures. ";
	}
	temp += "It is possible that there are structures not listed above "
		"which require either advanced resources to build or which "
		"increase the production of advanced resources.  The skill "
		"description for a skill will always note if new structures may "
		"be built based on knowing that skill.";
	f.Paragraph(temp);
	if(!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED)) {
		f.LinkRef("economy_roads");
		f.TagText("H3", "Roads:");
		temp = "There is a another type of structure called roads.  They do "
			"not protect units, nor aid in the production of resources, but "
			"do aid movement, and can improve the economy of a hex.";
		f.Paragraph(temp);
		temp = "Roads are directional and are only considered to reach from "
			"one hexside to the center of the hex.  To gain a movement "
			"bonus, there must be two connecting roads, one in each "
			"adjacent hex.  Only one road may be built in each direction. "
			"If a road in the given direction is connected, units move "
			"along that road at half cost to a minimum of 1 movement point.";
		f.Paragraph(temp);
		temp = "For example: If a unit is moving northwest, then hex it is "
			"in must have a northwest road, and the hex it is moving into "
			"must have a southeast road.";
		f.Paragraph(temp);
		temp = "To gain an economy bonus, a hex must have roads that connect "
			"to roads in at least two adjoining hexes.  The economy bonus "
			"for the connected roads raises the wages in the region by 1 "
			"point.";
		f.Paragraph(temp);
		f.LinkRef("tableroadstructures");
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TD", "");
		f.TagText("TH", "Cost");
		f.TagText("TH", "Material");
		f.TagText("TH", "Skill (min level)");
		f.Enclose(0, "TR");
		for(i = 0; i < NOBJECTS; i++) {
			if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if(ObjectDefs[i].productionAided != -1) continue;
			if(ObjectDefs[i].protect) continue;
			if(ObjectIsShip(i)) continue;
			j = ObjectDefs[i].skill;
			if(j == -1) continue;
			if(SkillDefs[j].flags & SkillType::MAGIC) continue;
			j = ObjectDefs[i].item;
			if(j == -1) continue;
			/* Need the >0 since item could be WOOD_OR_STONE (-2) */
			if(j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
			if(j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
			/* Okay, this is a valid object to build! */
			f.Enclose(1, "TR");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].cost);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			if(j == I_WOOD_OR_STONE)
				temp = "wood or stone";
			else
				temp = ItemDefs[j].name;
			f.PutStr(temp);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			temp = SkillDefs[ObjectDefs[i].skill].name;
			temp += AString(" (") + ObjectDefs[i].level + ")";
			f.PutStr(temp);
			f.Enclose(0, "TD");
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
	}
	if (Globals->DECAY) {
		f.LinkRef("economy_builddecay");
		f.TagText("H3", "Building Decay:");
		temp = "Some structures will decay over time if they are not "
			"maintained. Difficult terrain and bad weather will speed up "
			"this decay. Maintnenance involves having units with the "
			"appropriate level of skill expend a small amount of the "
			"material used to build the structure and labor on a fairly "
			"regular basis in the exactly same manner as they would work on "
			"the building it if it was not completed. In other words, enter "
			"the structure and issue the BUILD command with no parameters. "
			"If a structure will need maintenance, that information will be "
			"related in the object information given about the structure. "
			"If a structure is allowed to decay, it will not give any of "
			"its bonuses until it is repaired.";
		f.Paragraph(temp);
	}
	if(may_sail) {
		f.LinkRef("economy_ships");
		f.TagText("H3", "Ships:");
		temp = "Ships are constructed similarly to buildings, except they "
			"tend to be constructed out of wood, not stone, and their "
			"construction tends to depend on the Shipbuilding skill, not "
			"the Building skill. ";
		if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			temp += "Only factions with at least one faction point spent on "
				"trade can issue ";
			temp += f.Link("#build", "BUILD") + " orders. ";
		}
		temp += "Here is a table on the various ship types:";
		f.Paragraph(temp);
		f.LinkRef("tableshipinfo");
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TD", "");
		f.TagText("TH", "Capacity");
		f.TagText("TH", "Cost");
		f.TagText("TH", "Material");
		f.TagText("TH", "Sailors");
		f.Enclose(0, "TR");
		for(i = 0; i < NOBJECTS; i++) {
			if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if(!ObjectIsShip(i)) continue;
			j = ObjectDefs[i].skill;
			if(j == -1) continue;
			if(SkillDefs[j].flags & SkillType::MAGIC) continue;
			j = ObjectDefs[i].item;
			if(j == -1) continue;
			/* Need the >0 since item could be WOOD_OR_STONE (-2) */
			if(j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
			if(j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
			/* Okay, this is a valid object to build! */
			f.Enclose(1, "TR");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].capacity);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].cost);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			if(j == I_WOOD_OR_STONE)
				temp = "wood or stone";
			else
				temp = ItemDefs[j].name;
			f.PutStr(temp);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].sailors);
			f.Enclose(0, "TD");
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
		temp = "The capacity of a ship is the maximum weight that the ship "
			"may have aboard and still move. The cost is both the "
			"man-months of labor and the number of units of material "
			"required to complete the ship. The sailors are the number of "
			"skill levels of the Sailing skill that must be aboard the "
			"ship (and issuing the ";
		temp += f.Link("#sail", "SAIL") + " order in order for the ship "
			"to sail).";
		f.Paragraph(temp);
	}
	f.LinkRef("economy_advanceditems");
	f.TagText("H3", "Advanced Items:");
	temp = "There are also certain advanced items that highly skilled units "
		"can produce. These are not available to starting players, but can "
		"be discovered through study.  When a unit is skilled enough to "
		"produce one of these items, he will receive a skill report "
		"describing the production of this item. Production of advanced "
		"items is generally done in a manner similar to the normal items.";
	f.Paragraph(temp);
	f.LinkRef("economy_income");
	f.TagText("H3", "Income:");
	temp = "Units can earn money with the ";
	temp += f.Link("#work", "WORK") + " order.  This means that the unit "
		"spends the month performing manual work for wages. The amount to "
		"be earned from this is usually not very high, so it is generally "
		"a last resort to be used if one is running out of money. The "
		"current wages are shown in the region description for each region. "
		"All units may ";
	temp += f.Link("#work", "WORK") + ", regardless of skills";
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp += " or faction type";
	temp += ".";
	f.Paragraph(temp);
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f.LinkRef("economy_entertainment");
		f.TagText("H3", "Entertainment:");
		temp = "Units with the Entertainment skill can use it to earn "
			"money.  A unit with Entertainment level 1 will earn ";
		temp += AString(Globals->ENTERTAIN_INCOME) +
			" silver per man by issuing the ";
		temp += f.Link("#entertain", "ENTERTAIN") + " order.  The total "
			"amount of money that can be earned this way is shown in the "
			"region descriptions.  Higher levels of Entertainment skill can "
			"earn more, so a character with Entertainment skill 2 can earn "
			"twice as much money as one with skill 1 (and uses twice as "
			"much of the demand for entertainment in the region). Note that "
			"entertainment income is much less, per region, than the income "
			"available through working or taxing.";
		if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			temp += " All factions may have entertainers, regardless of "
				"faction type.";
		}
		f.Paragraph(temp);
	}
	f.LinkRef("economy_taxingpillaging");
	f.TagText("H3", "Taxing/Pillaging:");
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp = "War factions ";
	else
		temp = "Factions ";
	temp += "may collect taxes in a region.  This is done using the ";
	temp += f.Link("#tax", "TAX") + " order (which is ";
	if(!Globals->TAX_PILLAGE_MONTH_LONG) temp += "not ";
	temp += "a full month order). The amount of tax money that can be "
		"collected each month in a region is shown in the region "
		"description. ";
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) {
		temp += "Any unit may ";
		temp += f.Link("#tax", "TAX");
	} else {
		AString temp3;
		int prev = 0, hold = 0;
		temp += "A unit may ";
		temp += f.Link("#tax", "TAX");
		temp += " if it ";
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_COMBAT_SKILL | GameDefs::TAX_BOW_SKILL |
				 GameDefs::TAX_RIDING_SKILL | GameDefs::TAX_STEALTH_SKILL)) {
			int	prev2 = 0, hold2 = 0;
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "has ";
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_COMBAT_SKILL) {
				temp3 = "Combat";
				hold2 = 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) {
				if (hold2) {
					temp2 += temp3;
					prev2 = 1;
				}
				if (prev2) temp2 += ", ";
				temp2 += "Longbow";
				prev2 = 1;
				temp3 = "Crossbow";
				hold2 = 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) {
				if (hold2) {
					if (prev2) temp2 += ", ";
					temp2 += temp3;
					prev2 = 1;
				}
				temp3 = "Riding";
				hold2 = 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_STEALTH_SKILL) {
				if (hold2) {
					if (prev2) temp2 += ", ";
					temp2 += temp3;
					prev2= 1;
				}
				temp3 = "Stealth";
				hold2 = 1;
			}
			if (prev2) temp2 += " or ";
			temp2 += temp3;
			temp2 += " skill of at least level 1";
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_ANY_WEAPON | GameDefs::TAX_USABLE_WEAPON |
				 GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL |
				 GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "has ";
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_WEAPON)
				temp2 += "a weapon (regardless of skill requirements)";
			else if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_WEAPON)
				temp2 += "a weapon and the appropriate skill to use it";
			else if (Globals->WHO_CAN_TAX &
					(GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL |
					 GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON)) {
				AString temp3;
				int prev2 = 0, hold2 = 0;
				if (Globals->WHO_CAN_TAX &
						GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL) {
					temp2 += "Combat skill of at least level 1 and a "
						"weapon which does not require any skill";
					temp3 = "Riding skill of at least level 1 and "
						"a weapon which requires riding skill";
					prev2 = 1;
					hold2 = 1;
				}
				if (Globals->WHO_CAN_TAX &
						GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON) {
					if (hold2) {
						temp2 += ", ";
						temp2 += temp3;
					}
					temp3 = "a Bow (Longbow or Crossbow) skill and a weapon "
						"which requires that skill";
					hold2 = 1;
				}
				if (prev2) temp2 += " or ";
				temp2 += temp3;
			}
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_HORSE | GameDefs::TAX_HORSE_AND_RIDING_SKILL)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "has a mount";
			if (!(Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE))
				temp2 += " and sufficient skill to ride it in combat";
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_ANY_MAGE | GameDefs::TAX_MAGE_DAMAGE |
				 GameDefs::TAX_MAGE_FEAR | GameDefs::TAX_MAGE_OTHER)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "is a mage ";
			if ((Globals->WHO_CAN_TAX &
						(GameDefs::TAX_MAGE_DAMAGE |
						 GameDefs::TAX_MAGE_FEAR |
						 GameDefs::TAX_MAGE_OTHER)) ==
					(GameDefs::TAX_MAGE_DAMAGE | GameDefs::TAX_MAGE_FEAR |
					 GameDefs::TAX_MAGE_OTHER)) {
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_COMBAT_SPELL)
					temp2 += "who has a combat spell set";
				else
					temp2 += "who knows a combat spell";
			} else {
				int hold2 = 0, prev2 = 0;
				AString temp3;
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_COMBAT_SPELL)
					temp2 += "whose combat spell ";
				else
					temp2 += "who knows a spell which ";
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) {
					temp3 = "damages enemies";
					hold2 = 1;
				}
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) {
					if (hold2) {
						temp2 += temp3;
						prev2 = 1;
					}
					temp3 = "weakens opponents";
					hold2 = 1;
				}
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) {
					if (hold2) {
						if (prev2) temp2 += ", ";
						temp2 += temp3;
						prev2 = 1;
					}
					temp3 = "protects in combat";
					hold2 = 1;
				}
				if (prev2) temp2 += " or ";
				temp2 += temp3;
			}
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_BATTLE_ITEM |
				 GameDefs::TAX_USABLE_BATTLE_ITEM)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_BATTLE_ITEM)
				temp2 = "has a ";
			else
				temp2 = "can use a ";
			temp2 += "magical item which gives a special attack in combat";
			hold = 1;
		}
		if (prev) temp += " or ";
		temp += temp2;
		temp += ". ";
	}
	if (Globals->WHO_CAN_TAX &
			(GameDefs::TAX_CREATURES | GameDefs::TAX_ILLUSIONS)) {
		if (Globals->WHO_CAN_TAX & GameDefs::TAX_CREATURES) {
			temp += "Summoned ";
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
				temp += "and illusory ";
		} else if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
			temp += "Illusory ";
		temp += "creatures will assist in taxation. ";
	}
	temp +=	"Each taxing character can collect $";
	temp += AString(Globals->TAX_INCOME) + ", though if the number of "
		"taxers would tax more than the available tax income, the tax "
		"income is split evenly among all taxers.";
	f.Paragraph(temp);
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp = "War factions ";
	else
		temp = "Factions ";
	temp += "may also pillage a region. To do this requires the faction to "
		"have enough combat ready men in the region to tax half of the "
		"available money in the region. The total amount of money that can "
		"be pillaged will then be shared out between every combat ready "
		"unit that issues the ";
	temp += f.Link("#pillage", "PILLAGE") + " order. The amount of money "
		"collected is equal to twice the available tax money. However, the "
		"economy of the region will be seriously damaged by pillaging, and "
		"will only slowly recover over time.  Note that ";
	temp += f.Link("#pillage", "PILLAGE") + " comes before " +
		f.Link("#tax", "TAX") + ", so a unit performing " +
		f.Link("#tax", "TAX") + " will collect no money in that region that "
		"month.";
	f.Paragraph(temp);
	temp = "It is possible to safeguard one's tax income in regions one "
		"controls.  Units which have the Guard flag set (using the ";
	temp += f.Link("#guard", "GUARD") + " order) will block " +
		f.Link("#tax", "TAX") + " orders issued by other factions in the same "
		"region, unless you have declared the faction in question Friendly. "
		"Units on guard will also block ";
	temp += f.Link("#pillage", "PILLAGE") + " orders issued by other "
		"factions in the same region, regardless of your attitude towards "
		"the faction in question, and they will attempt to prevent "
		"Unfriendly units from entering the region.  Only units which are "
		"able to tax may be on guard.  Units on guard ";
	if(has_stea)
		temp += " are always visible regardless of Stealth skill, and ";
	temp += "will be marked as being \"on guard\" in the region description.";
	f.Paragraph(temp);
	f.LinkRef("com");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Combat");
	temp = "Combat occurs when one unit attacks another.  The computer then "
		"gathers together all the units on the attacking side, and all the "
		"units on the defending side, and the two sides fight until an "
		"outcome is reached.";
	f.Paragraph(temp);
	f.LinkRef("com_attitudes");
	f.TagText("H3", "Attitudes:");
	temp = "Which side a faction's units will fight on depends on declared "
		"attitudes.  A faction can have one of the following attitudes "
		"towards another faction:  Ally, Friendly, Neutral, Unfriendly or "
		"Hostile.  Each faction has a general attitude, called the \"Default "
		"Attitude\", that it normally takes towards other factions; this is "
		"initially Neutral, but can be changed.  It is also possible to ";
	temp += f.Link("#declare", "DECLARE") + " attitudes to specific "
		"factions, e.g. ";
	temp += f.Link("#declare", "DECLARE") + " 27 ALLY will declare the "
		"Ally attitude to faction 27.  (Note that this does not necessarily "
		"mean that faction 27 has decided to treat you as an ally.)";
	f.Paragraph(temp);
	temp = "Ally means that you will fight to defend units of that faction "
		"whenever they come under attack, if you have non-avoiding units in "
		"the region where the attack occurs. ";
	if(has_stea) {
		temp += " You will also attempt to prevent any theft or "
			"assassination attempts against units of the faction";
		if(has_obse) {
			temp += ", if you are capable of seeing the unit which is "
				"attempting the crime";
		}
		temp += ". ";
	}
	temp += "It also has the implications of the Friendly attitude.";
	f.Paragraph(temp);
	temp = "Friendly means that you will accept gifts from units of that "
		"faction.  This includes the giving of items, units of people, and "
		"the teaching of skills.  You will also admit units of that faction "
		"into buildings or ships owned by one of your units, and you will "
		"permit units of that faction to collect taxes (but not pillage) "
		"in regions where you have units on guard.";
	f.Paragraph(temp);
	temp = "Unfriendly means that you will not admit units of that faction "
		"into any region where you have units on guard.  You will not, "
		"however, automatically attack unfriendly units which are already "
		"present.";
	f.Paragraph(temp);
	temp = "Hostile means that any of your units which do not have the "
		"Avoid Combat flag set (using the ";
	temp += f.Link("#avoid", "AVOID") + " order) will attack any units of "
		"that faction wherever they find them.";
	f.Paragraph(temp);
	temp = "If a unit can see another unit, but ";
	if(has_obse) {
		temp += "does not have high enough Observation skill to determine "
			"its faction,";
	} else {
		temp += "it is not revealing its faction,";
	}
	temp += " it will treat the unit using the faction's default attitude, "
		"even if the unit belongs to an Unfriendly or Hostile faction, "
		"because it does not know the unit's identity.  However, if your "
		"faction has declared an attitude of Friendly or Ally towards that "
		"unit's faction, the unit will be treated with the better attitude; "
		"it is assumed that the unit will produce proof of identity when "
		"relevant.";
	if(has_stea) {
		temp += " (See the section on stealth for more information on when "
			"units can see each other.)";
	}
	f.Paragraph(temp);
	temp = "If a faction declares Unfriendly or Hostile as default attitude "
		"(the latter is a good way to die fast), it will block or attack "
		"all unidentified units, unless they belong to factions for which a "
		"Friendly or Ally attitude has been specifically declared.";
	if(has_stea) {
		temp += " Units which cannot be seen at all cannot be directly "
			"blocked or attacked, of course.";
	}
	f.Paragraph(temp);
	f.LinkRef("com_attacking");
	f.TagText("H3", "Attacking:");
	temp = "A unit can attack another by issuing an ";
	temp += f.Link("#attack", "ATTACK") + " order. A unit that does not "
		"have Avoid Combat set will automatically attack any Hostile units "
		"it identifies as such.";
	if(has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		temp += " When a unit issues the ";
		temp += f.Link("#attack", "ATTACK") + " order, or otherwise "
			"decides to attack another unit, it must first be able to "
			"attack the unit. ";
		if(has_stea && !(SkillDefs[S_RIDING].flags & SkillType::DISABLED))
			temp += "There are two conditions for this; the first is that the";
		else
			temp += "The";
		if(has_stea) {
			temp += " attacking unit must be able to see the unit that it "
				"wishes to attack.  More information is available on this "
				"in the stealth section of the rules.";
		}
		if(!(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
			if(has_stea) {
				f.Paragraph(temp);
				temp = "Secondly, the";
			}
			temp += " attacking unit must be able to catch the unit it "
				"wishes to attack.  A unit may only catch a unit if its "
				"effective Riding skill is greater than or equal to the "
				"target unit's effective Riding skill; otherwise, the "
				"target unit just rides away from the attacking unit.  "
				"Effective Riding is the unit's Riding skill, but with "
				"a potential maximum; if the unit can not ride, the "
				"effective Riding skill is 0; if the unit can ride, the "
				"maximum effective Riding is 3; if the unit can fly, the "
				"maximum effective Riding is 5. Note that the effective "
				"Riding also depends on whether the unit is attempting to "
				"attack or defend; for attack purposes, only one man in "
				"the unit needs to be able to ride or fly (generally, this "
				"means one of the men must possess a horse, or other form "
				"of transportation), whereas for defense purposes the entire "
				"unit needs to be able to ride or fly (usually meaning "
				"that every man in the unit must possess a horse or other "
				"form of speedier transportation). Also, note that for a "
				"unit to be able to use its defensive Riding ability to "
				"avoid attack, the unit cannot be in a building, ship, or "
				"structure of any type.";
		}
	}
	f.Paragraph(temp);
	temp = "A unit which is on guard, and is Unfriendly towards a unit, "
		"will deny access to units using the ";
	temp += f.Link("#move", "MOVE") + " order to enter its region. ";
	if(has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		temp += "Note that to deny access to a unit, at least one unit "
			"from the same faction as the unit guarding the hex must satisfy "
			"the above requirements. ";
	}
	temp += "A unit using ";
	temp += f.Link("#advance", "ADVANCE") + " instead of " +
		f.Link("#move", "MOVE") + " to enter a region, will attack any "
		"units that attempt to deny it access.  If the advancing unit loses "
		"the battle, it will be forced to retreat to the previous region it "
		"moved through.  If the unit wins the battle and its army doesn't "
		"lose any men, it is allowed to continue to move, provided that it "
		"has enough movement points.";
	f.Paragraph(temp);
	if(has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		temp = "Note that ";
		if(has_stea && !(SkillDefs[S_RIDING].flags & SkillType::DISABLED))
			temp += "these restrictions do ";
		else
			temp += "this restriction does ";
		temp += "not apply for sea combat, as ";
		if(has_stea)
			temp += "units within a ship are always visible";
		if(!(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
			if(has_stea) temp += ", and";
			temp += " Riding does not play a part in combat on board ships";
		}
		temp += ".";
		f.Paragraph(temp);
	}
	f.LinkRef("com_muster");
	f.TagText("H3", "The Muster:");
	temp = "Once the attack has been made, the sides are gathered.  Although "
		"the ";
	temp += f.Link("#attack", "ATTACK") + " order takes a unit rather than "
		"a faction as its parameter (mainly so that unidentified units can "
		"be attacked), an attack is basically considered to be by an entire "
		"faction, against an entire faction and its allies.";
	f.Paragraph(temp);
	temp = "On the attacking side are all units of the attacking faction in "
		"the region where the fight is taking place, except those with Avoid "
		"Combat set.  A unit which has explicitly (or implicitly via ";
	temp += f.Link("#advance", "ADVANCE") + ") issued an " +
		f.Link("#attack", "ATTACK") + " order will join the fight anyway, "
		"regardless of whether Avoid Combat is set.";
	f.Paragraph(temp);
	temp = "Also on the attacking side are all units of other factions that "
		"attacked the target faction (implicitly or explicitly) in the "
		"region where the fight is taking place.  In other words, if several "
		"factions attack one, then all their armies join together to attack "
		"at the same time (even if they are enemies and will later fight "
		"each other).";
	f.Paragraph(temp);
	temp = "On the defending side are all identifiable units belonging to "
		"the defending faction.  If a unit has Avoid Combat set and it "
		"belongs to the target faction, it will be uninvolved only if its "
		"faction cannot be identified by the attacking faction.  A unit "
		"which was explicitly attacked will be involved anyway, regardless "
		"of Avoid Combat. ";
	if(has_stea) {
		temp += "(This means that Avoid Combat is mostly useful for high "
			"stealth scouts.) ";
	}
	temp += "Also, all non-avoiding units located in the target region "
		"belonging to factions allied with the defending unit will join "
		"in on the defending side.";
	f.Paragraph(temp);
	temp = "Units in adjacent regions can also become involved.  This is "
		"the exception to the general rule that you cannot interact with "
		"units in a different region.";
	f.Paragraph(temp);
	temp = "If a faction has at least one unit involved in the initial "
		"region, then any units in adjacent regions will join the fight, "
		"if they could reach the region and do not have Avoid Combat set. "
		"There are a few flags that units may set to affect this; a unit "
		"with the Hold flag (set using the ";
	temp += f.Link("#hold", "HOLD") + " order) will not join battles in "
		"adjacent regions.  This flag applies to both attacking and "
		"defending factions.  A unit with the Noaid flag (set using the ";
	temp += f.Link("#noaid", "NOAID") + " order) will receive no aid from "
		"adjacent hexes when attacked, or when it issues an attack.";
	f.Paragraph(temp);
	temp = "Example:  A fight starts in region A, in the initial combat "
		"phase (before any movement has occurred).  The defender has a unit "
		"of soldiers in adjacent region B.  They have 2 movement points at "
		"this stage. ";
	temp += "They will buy horses later in the turn, so that when "
		"they execute their ";
	temp += f.Link("#move", "MOVE") + " order they will have 4 movement "
		"points, but right now they have 2. ";
	if(Globals->WEATHER_EXISTS)
		temp += "Region A is forest, but fortunately it is summer, ";
	else
		temp += "Region A is forest, ";
	temp += "so the soldiers can join the fight.";
	f.Paragraph(temp);
	temp = "It is important to note that the units in nearby regions do not "
		"actually move to the region where the fighting happens; the "
		"computer only checks that they could move there.  (In game world "
		"terms, presumably they did move there to join the fight, and then "
		"moved back where they started.)  The computer checks for weight "
		"allowances and terrain types when determining whether a unit could "
		"reach the scene of the battle. Note that the use of ships is not "
		"allowed in this virtual movement.";
	f.Paragraph(temp);
	temp = "If you order an attack on an ally (either with the ";
	temp += f.Link("#attack", "ATTACK") + " order, or if your ally has "
		"declared you Unfriendly, by attempting to ";
	temp += f.Link("#advance", "ADVANCE") +" into a region which he is "
		"guarding), then your commander will decide that a mistake has "
		"occurred somewhere, and withdraw your troops from the fighting "
		"altogether.  Thus, your units will not attack that faction in "
		"that region. Note that you will always defend an ally against "
		"attack, even if it means that you fight against other factions "
		"that you are allied with.";
	f.Paragraph(temp);
	f.LinkRef("com_thebattle");
	f.TagText("H3", "The Battle:");
	temp = "The troops having lined up, the fight begins.";
	if(!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED)) {
		temp += " The computer selects the best tactician from each side; "
			"that unit is regarded as the leader of its side.  If two or "
			"more units on one side have the same Tactics skill, then the "
			"one with the lower unit number is regarded as the leader of "
			"that side.  If one side's leader has a better Tactics skill "
			"than the other side's, then that side gets a free round of "
			"attacks.";
	}
	f.Paragraph(temp);
	temp = "In each combat round, the combatants each get to attack once, in "
		"a random order. ";
	if(!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED)) {
		temp += "(In a free round of attacks, only one side's forces get to "
			"attack.) ";
	}
	temp += "Each combatant will attempt to hit a randomly selected enemy. "
		"If he hits, and the target has no armor, then the target is "
		"automatically killed.  Armor may provide extra defense against "
		"otherwise successful attacks.";
	f.Paragraph(temp);
	temp = "The basic skill used in battle is the Combat skill; this is "
		"used for hand to hand fighting.  If one soldier tries to hit "
		"another using most weapons, there is a 50% chance that the "
		"attacker will get an opportunity for a lethal blow.  If the "
		"attacker does get that opportunity, then there is a contest "
		"between his combat skill (modified by weapon attack bonus) and "
		"the defender's combat skill (modified by weapon defense bonus). "
		"Some weapons may not allow combat skill to affect defense (e.g. "
		"bows), and others may allow different skills to be used on "
		"defense (or offense).";
	f.Paragraph(temp);
	temp = "If the skills are equal, then there is a 1:1 (i.e. 50%) "
		"chance that the attack will succeed.  If the attacker's skill is 1 "
		"higher then there is a 2:1 (i.e. 66%) chance, if the attacker's "
		"skill is 2 higher then there is a 4:1 (i.e. 80%) chance, 3 higher "
		"means an 8:1 (i.e. 88%) chance, and so on. Similarly if the "
		"defender's skill is 1 higher, then there is only a 1:2 (i.e. 33%) "
		"chance, etc.";
	f.Paragraph(temp);
	temp = "";
	if(!(ItemDefs[I_SWORD].flags & ItemType::DISABLED)) {
		temp += "Possession of a sword confers a ";
		if(WeaponDefs[WEAPON_SWORD].attackBonus > -1) temp += "+";
		temp += WeaponDefs[WEAPON_SWORD].attackBonus;
		temp += " bonus to Combat skill for attack and a ";
		if(WeaponDefs[WEAPON_SWORD].defenseBonus > -1) temp += "+";
		temp += WeaponDefs[WEAPON_SWORD].defenseBonus;
		temp += " bonus for defense. ";
	}
	temp += "Troops which are fighting hand-to-hand without specific weapons "
		"are assumed to be irregularly armed with makeshift weapons such as "
		"clubs, pitchforks, torches, etc. ";
	if(!(SkillDefs[S_RIDING].flags & SkillType::DISABLED) &&
			!(ItemDefs[I_HORSE].flags & ItemType::DISABLED)) {
		temp += "Possession of a horse, and Riding skill, also confers a "
			"bonus to effective Combat skill equal to the Riding skill "
			"level (up to a maximum of ";
		temp += MountDefs[MOUNT_HORSE].maxBonus;
		temp += ") provided that the terrain allows horses to be used in "
			"combat. ";
		if(!(ItemDefs[I_WHORSE].flags & ItemType::DISABLED)) {
			temp += "Winged horse are better yet, but require more basic "
				"Riding skill to gain any advantage. ";
		}
	}
	temp += "Certain weapons may provide different attack and defense "
		"bonuses, or have additional attack bonuses against mounted "
		"opponents or other special characteristics. These bonuses will "
		"be listed in the item descriptions in the turn reports.";
	f.Paragraph(temp);
	temp = "Some melee weapons may be defined as Long or Short (this is "
		"relative to a normal weapon, e.g. the sword).  A soldier wielding "
		"a longer weapon than his opponent gets a +1 bonus to his attack "
		"skill.";
	f.Paragraph(temp);
	temp = "Ranged weapons are slightly different from melee weapons.  The "
		"target will generally not get any sort of combat bonus to defense "
		"against a ranged attack. ";
	if(!(ItemDefs[I_LONGBOW].flags & ItemType::DISABLED) &&
			!(SkillDefs[S_LONGBOW].flags & SkillType::DISABLED)) {
		temp += "The skill check to hit with a long bow is made against an "
			"effective defense of ";
		temp += -(WeaponDefs[WEAPON_LONGBOW].attackBonus);
		temp += "; i.e., a longbowman with a skill 1, having made the 50% "
			"chance of getting an effective attack, has a 1:2 chance of "
			"hitting a target. ";
	}
	if(!(ItemDefs[I_CROSSBOW].flags & ItemType::DISABLED) &&
			!(SkillDefs[S_CROSSBOW].flags & SkillType::DISABLED)) {
		temp += "A crossbow is an easier weapon to use, so the chance to hit "
			"is calculated against a defense of ";
		temp += -(WeaponDefs[WEAPON_CROSSBOW].attackBonus);
		if(WeaponDefs[WEAPON_CROSSBOW].numAttacks < 0) {
			temp += "; on the other hand, a crossbow can only fire once "
				"every ";
			temp += -(WeaponDefs[WEAPON_CROSSBOW].numAttacks);
			temp += " rounds, including the free round of attacks if ones "
				"side has one";
		}
		temp += ".";
	}
	f.Paragraph(temp);
	temp = "Weapons may have one of several different attack types: "
		"Slashing, Piercing, Crushing, Cleaving and Armor Piercing.  "
		"Different types of armor may give different survival chances "
		"against a sucessful attack of different types.";
	f.Paragraph(temp);
	temp = "A soldier attacking with a ranged weapon will generally be "
		"treated as if they have a Combat skill of 0, even if they have an "
		"actual Combat skill.  This is the trade off for being able to hit "
		"from the back line of fighting.";
	f.Paragraph(temp);
	temp = "Being inside a building confers a +2 bonus to defense.  This "
		"bonus is effective against ranged as well as melee weapons.  The "
		"number of men that a building can protect is equal to its size. "
		"The size of the various common buildings was listed in the ";
	temp += f.Link("#tablebuildings", "Table of Buildings") + " earlier. ";
	f.Paragraph(temp);
	temp = "If there are too many units in a building to all gain "
		"protection from it, then those units who have been in the building "
		"longest will gain protection.  (Note that these units appear first "
		"on the turn report.)";
	if(!(ObjectDefs[O_FORT].flags & ObjectType::DISABLED)) {
		temp += " If a unit of 200 men is inside a Fort (capacity ";
		temp += ObjectDefs[O_FORT].protect;
		temp += "), then the first ";
		temp += ObjectDefs[O_FORT].protect;
		temp += " men in the unit will gain the full +2 bonus, and the other ";
		temp += (200 - ObjectDefs[O_FORT].protect);
		temp += " will gain no protection.";
	}
	f.Paragraph(temp);
	temp = "Units which have the Behind flag set are at the rear and "
		"cannot be attacked by any means until all non-Behind units have "
		"been wiped out.  On the other hand, neither can they attack with "
		"melee weapons, but only with ranged weapons or magic.  Once all "
		"front-line units have been wiped out, then the Behind flag no "
		"longer has any effect.";
	f.Paragraph(temp);
	f.LinkRef("com_victory");
	f.TagText("H3", "Victory!");
	temp = "Combat rounds continue until one side has accrued 50% losses "
		"(or more). The victorious side is then awarded one free round of "
		"attacks, after which the battle is over.  If both sides have "
		"more than 50% losses, the battle is a draw, and neither side gets "
		"a free round.";
	f.Paragraph(temp);
	/* XXX -- Here is where to put the ROUT information */
	if(!(SkillDefs[S_HEALING].flags & SkillType::DISABLED) &&
			!(ItemDefs[I_HERBS].flags & SkillType::DISABLED)) {
		temp = "Units with the Healing skill have a chance of being able "
			"to heal casualties of the winning side, so that they recover "
			"rather than dying.  Each character with this skill can attempt "
			"to heal ";
		temp += Globals->HEALS_PER_MAN;
		temp += " casualties per skill level. Each attempt however "
			"requires one unit of Herbs, which is thereby used up. Each "
			"attempt has a 50% chance of healing one casualty; only one "
			"attempt at Healing may be made per casualty. Healing occurs "
			"automatically, after the battle is over, by any living "
			"healers on the winning side.";
		f.Paragraph(temp);
	}
	temp = "Any items owned by dead combatants on the losing side have a "
		"50% chance of being found and collected by the winning side. "
		"Each item which is recovered is picked up by one of the "
		"survivors able to carry it (see the ";
	temp += f.Link("#spoils", "SPOILS") + " command) at random, so the "
		"winners generally collect loot in proportion to their number of "
		"surviving men.";
	f.Paragraph(temp);
	temp = "If you are expecting to fight an enemy who is carrying so "
		"much equipment that you would not be able to move after picking "
		"it up, and you want to move to another region later that month, it "
		"may be worth issuing some orders to drop items (with the ";
	temp += f.Link("#give", "GIVE") + " 0 order) or to prevent yourself "
		"picking up certain types of spoils (with the ";
	temp += f.Link("#spoils", "SPOILS") + " order) in case you win the "
		"battle! Also, note that if the winning side took any losses in "
		"the battle, any units on this side will not be allowed to move, "
		"or attack again for the rest of the turn.";
	f.Paragraph(temp);
	if(has_stea || has_obse) {
		f.LinkRef("stealthobs");
		f.ClassTagText("DIV", "rule", "");
		temp = (has_stea ? "Stealth" : "");
		if(has_obse) {
			if(has_stea) temp += " and ";
			temp += "Observation";
		}
		f.TagText("H2", temp);
		if(has_stea && has_obse) {
			temp = "The Stealth skill is used to hide units, while the "
				"Observation skill is used to see units that would otherwise "
				"be hidden. A unit can be seen only if you have at least "
				"one unit in the same region, with an Observation skill at "
				"least as high as that unit's Stealth skill. If your "
				"Observation skill is equal to the unit's Stealth skill, "
				"you will see the unit, but not the name of the owning "
				"faction. If your Observation skill is higher than the "
				"unit's Stealth skill, you will also see the name of the "
				"faction that owns the unit.";
		} else if(has_stea) {
			temp = "The Stealth skill is used to hide units. A unit can be "
				"seen only if it doesn't know the Stealth skill and if you "
				"have at least one unit in the same region.";
		} else if(has_obse) {
			temp = "The Observation skill is used to see information about "
				"units that would otherwise be hidden.  If your unit knows "
				"the Observation skill, it will see the name of the faction "
				"that owns any unit in the same region.";
		}
		f.Paragraph(temp);
		if(has_stea) {
			temp = "Regardless of Stealth skill, units are always visible "
				"when participating in combat; when guarding a region with "
				"the Guard flag; or when in a building or aboard a ship.";
			if(has_obse) {
				temp += " However, in order to see the faction that owns "
					"the unit, you will still need a higher Observation "
					"skill than the unit's Stealth skill.";
			}
			f.Paragraph(temp);
			f.LinkRef("stealthobs_stealing");
			f.TagText("H3", "Stealing:");
			temp = AString("The ") + f.Link("#steal", "STEAL") +
				" order is a way to steal items from other factions without "
				"a battle.  The order can only be issued by a one-man unit. "
				"The order specifies a target unit; the thief will then "
				"attempt to steal the specified item from the target unit.";
			f.Paragraph(temp);
			if(has_obse) {
				temp = "If the thief has higher Stealth than any of the "
					"target faction's units have Observation (i.e. the "
					"thief cannot be seen by the target faction), the theft "
					"will succeed.";
			} else {
				temp = "The thief must know Stealth to attempt theft.";
			}
			temp += " The target faction will be told what was stolen, but "
				"not by whom.  If the specified item is silver, then $200 "
				"or half the total available, whichever is less, will be "
				"stolen.  If it is any other item, then only one will be "
				"stolen (if available).";
			f.Paragraph(temp);
			if(has_obse) {
				temp = "Any unit with high enough Observation to see the "
					"thief will see the attempt to steal, whether the "
					"attempt is successful or not.  Allies of the target "
					"unit will prevent the theft, if they have high enough "
					"Observation to see the unit trying to steal.";
				f.Paragraph(temp);
			}
			f.LinkRef("stealthobs_assassination");
			f.TagText("H3", "Assassination:");
			temp = AString("The ") + f.Link("#assassinate", "ASSASSINATE") +
				" order is a way to kill another person without attacking "
				"and going through an entire battle. This order can only be "
				"issued by a one-man unit, and specifies a target unit.  If "
				"the target unit contains more than one person, then one "
				"will be singled out at random.";
			f.Paragraph(temp);
			if(has_obse) {
				temp = "Success for assassination is determined as for "
					"theft, i.e. the assassin will fail if any of the "
					"target faction's units can see him.  In this case, "
					"the assassin will flee, and the target faction will "
					"be informed which unit made the attempt.  As with "
					"theft, allies of the target unit will prevent the "
					"assassination from succeeding, if their Observation "
					"level is high enough.";
				f.Paragraph(temp);
				temp = "";
			} else {
				temp = "The assasin must know Stealh to attempt "
					"assassination.";
			}
			if(has_obse) {
				temp += "If the assassin has higher stealth than any of the "
					"target faction's units have Observation, then a "
					"one-on-one ";
			} else {
				temp += " A one-on-one ";
			}
			temp += "fight will take place between the assassin and the "
				"target character.  The assassin automatically gets a "
				"free round of attacks";
			if(Globals->MAX_ASSASSIN_FREE_ATTACKS) {
				temp += ", except he is limited to ";
				temp += Globals->MAX_ASSASSIN_FREE_ATTACKS;
				temp += " total during the free round";
			}
			temp += "; after that, the battle is handled like a normal "
				"fight, with the exception that neither assassin nor "
				"victim can use any armor";
			temp2 = "";
			last = -1;
			comma = 0;
			for(i = 0; i < NITEMS; i++) {
				if(!(ItemDefs[i].type & IT_ARMOR)) continue;
				if(!(ItemDefs[i].type & IT_NORMAL)) continue;
				if(ItemDefs[i].flags & ItemType::DISABLED) continue;
				j = ItemDefs[i].index;
				if(!(ArmorDefs[j].flags & ArmorType::USEINASSASSINATE))
					continue;
				if(last == -1) {
					last = i;
					continue;
				}
				temp2 += ItemDefs[last].name;
				temp2 += ", ";
				last = i;
				comma++;
			}
			if(comma) temp2 += "or ";
			if(last != -1) {
				temp2 += ItemDefs[last].name;
				temp += " except ";
				temp += temp2;
			}
			temp += ".";
			if(last == -1)
				temp += " Armor ";
			else
				temp += " Most armor ";
			temp += "is forbidden for the assassin because it would "
				"make it too hard to sneak around, and for the victim "
				"because he was caught by surprise with his armor off. If "
				"the assassin wins, the target faction is told merely that "
				"the victim was assassinated, but not by whom.  If the "
				"victim wins, then the target faction learns which unit "
				"made the attempt.  (Of course, this does not necessarily "
				"mean that the assassin's faction is known.)  The winner of "
				"the fight gets 50% of the loser's property as usual.";
			f.Paragraph(temp);
			temp = f.Link("#steal", "STEAL") + " and " +
				f.Link("#assassinate", "ASSASSINATE") +
				" are not full month orders, and do not interfere with other "
				"activities, but a unit can only issue one " +
				f.Link("#steal", "STEAL") + " order or one " +
				f.Link("#assassinate", "ASSASSINATE") + " order in a month.";
			f.Paragraph(temp);
		}
	}
	f.LinkRef("magic");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Magic");
	temp = "A character enters the world of magic in Atlantis by beginning "
		"study on one of the Foundation magic skills.  Only one man units";
	if(!Globals->MAGE_NONLEADERS && Globals->LEADERS_EXIST)
		temp += ", with the man being a leader,";
	temp += " are permitted to study these skills. ";
	if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
		temp += "The number of these units (known as \"magicians\" or "
			"\"mages\") that a faction may own is ";
		if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT)
			temp += "limited.";
		else
			temp += "determined by the faction's type.";
		temp += " Any attempt to gain more, either through study, or by "
			"transfer from another faction, will fail.  In addition, mages ";
	} else {
		temp += "Mages ";
	}
	temp += "may not ";
	temp += f.Link("#give", "GIVE") + " men at all; once a unit becomes a "
		"mage (by studying one of the Foundations), the unit number is "
		"fixed. (The mage may be given to another faction using the ";
	temp += f.Link("#give", "GIVE") + " UNIT order.)";
	f.Paragraph(temp);
	f.LinkRef("magic_skills");
	f.TagText("H3", "Magic Skills:");
	temp = "Magic skills are the same as normal skills, with a few "
		"differences.  The basic magic skills, called Foundations, are ";
	last = -1;
	comma = 0;
	j = 0;
	for(i = 0; i < NSKILLS; i++) {
		if(SkillDefs[i].flags & SkillType::DISABLED) continue;
		if(!(SkillDefs[i].flags & SkillType::FOUNDATION)) continue;
		j++;
		if(last == -1) {
			last = i;
			continue;
		}
		temp += SkillDefs[last].name;
		temp += ", ";
		comma++;
		last = i;
	}
	if(comma) temp += "and ";
	temp += SkillDefs[last].name;
	temp += ". To become a mage, a unit undertakes study in one of these "
		"Foundations.  As a unit studies the Foundations, he will be able "
		"to study deeper into the magical arts; the additional skills that "
		"he may study will be indicated on your turn report.";
	f.Paragraph(temp);
	temp = "There are two major differences between Magic skills and most "
		"normal skills. The first is that the ability to study Magic skills "
		"sometimes depends on lower level Magic skills.  The Magic skills "
		"that a mage may study are listed on his turn report, so he knows "
		"which areas he may pursue.  Studying higher in the Foundation "
		"skills, and certain other Magic skills, will make other skills "
		"available to the mage. Also, study into a magic skill above "
		"level 2 requires that the mage be located in some sort of "
		"building which can ";
	if(!Globals->LIMITED_MAGES_PER_BUILDING) {
		temp += "offer protection.  Trade structures do not count. ";
	} else {
		temp += "offer specific protection to mages.  Certain types of "
			"buildings can offer shelter and support and a proper "
			"environment, some more so than others. ";
	}
	temp += "If the mage is not in such a structure, his study rate is cut "
			"in half, as he does not have the proper environment and "
			"equipment for research.";
	f.Paragraph(temp);

	if(Globals->LIMITED_MAGES_PER_BUILDING) {
		temp = "It is possible that there are advanced buildings not listed "
			"here which also can support mages.  The description of a "
			"building will tell you for certain.  The common buildings and "
			"the mages a building of that type can support follows:";
		f.LinkRef("tablemagebuildings");
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TD", "");
		f.TagText("TH", "Mages");
		f.Enclose(0, "TR");
		for(i = 0; i < NOBJECTS; i++) {
			if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if(!ObjectDefs[i].maxMages) continue;
			j = ObjectDefs[i].skill;
			if(j == -1) continue;
			if(SkillDefs[j].flags & SkillType::MAGIC) continue;
			j = ObjectDefs[i].item;
			if(j == -1) continue;
			/* Need the >0 since item could be WOOD_OR_STONE (-2) */
			if(j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
			if(j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
			/* Okay, this is a valid object to build! */
			f.Enclose(1, "TR");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
			f.PutStr(ObjectDefs[i].maxMages);
			f.Enclose(0, "TD");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
	}

	f.LinkRef("magic_foundations");
	f.TagText("H3", "Foundations:");
	temp = "The ";
	temp += NumToWord(j);
	temp += " Foundation skills are called ";
	last = -1;
	comma = 0;
	for(i = 0; i < NSKILLS; i++) {
		if(SkillDefs[i].flags & SkillType::DISABLED) continue;
		if(!(SkillDefs[i].flags & SkillType::FOUNDATION)) continue;
		if(last == -1) {
			last = i;
			continue;
		}
		temp += SkillDefs[last].name;
		temp += ", ";
		comma++;
		last = i;
	}
	if(comma) temp += "and ";
	temp += SkillDefs[last].name;
	temp += ".";
	/* XXX -- This needs better handling! */
	/* Add each foundation here if it exists */
	if(!(SkillDefs[S_FORCE].flags & SkillType::DISABLED)) {
		temp += " Force indicates the quantity of magical energy that a "
			"mage is able to channel (a Force rating of 0 does not mean "
			"that the mage can channel no magical energy at all, but only "
			"a minimal amount).";
	}
	if(!(SkillDefs[S_PATTERN].flags & SkillType::DISABLED)) {
		temp += " Pattern indicates ability to handle complex patterns, and "
			"is important for things like healing and nature spells. ";
	}
	if(!(SkillDefs[S_SPIRIT].flags & SkillType::DISABLED)) {
		temp += " Spirit deals with meta-effects that lie outside the scope "
			"of the physical world.";
	}
	f.Paragraph(temp);
	f.LinkRef("magic_furtherstudy");
	f.TagText("H3", "Further Magic Study:");
	temp = "Once a mage has begun study of one or more Foundations, more "
		"skills that he may study will begin to show up on his report. "
		"These skills are the skills that give a mage his power.  As with "
		"normal skills, when a mage achieves a new level of a magic skill, "
		"he will be given a skill report, describing the new powers (if "
		"any) that the new skill confers.  The ";
	temp += f.Link("#show", "SHOW") + " order may be used to show this "
		"information on future reports.";
	f.Paragraph(temp);
	f.LinkRef("magic_usingmagic");
	f.TagText("H3", "Using Magic:");
	temp = "A mage may use his magical power in three different ways, "
		"depending on the type of spell he wants to use.  Some spells, "
		"once learned, take effect automatically and are considered "
		"always to be in use; these spells do not require any order to "
		"take effect.";
	f.Paragraph(temp);
	temp = "Secondly, some spells are for use in combat. A mage may specify "
		"that he wishes to use a spell in combat by issuing the ";
	temp += f.Link("#combat", "COMBAT") + " order.  A combat spell "
		"specified in this way will only be used if the mage finds "
		"himself taking part in a battle.";
	f.Paragraph(temp);
	temp = "The third type of spell use is for spells that take an entire "
		"month to cast.  These spells are cast by the mage issuing the ";
	temp += f.Link("#cast", "CAST") + " order. Because " +
		f.Link("#cast", "CAST") + " takes an entire month, a mage may use "
		"only one of this type of spell each turn. Note, however, that a ";
	temp += f.Link("#cast", "CAST") + " order is not a full month order; "
		"a mage may still ";
	temp += f.Link("#move", "MOVE") + ", ";
	temp += f.Link("#study", "STUDY") + ", or use any other month long order. ";
	temp += "The justification for this (as well as being for game balance) "
		"is that a spell drains a mage of his magic power for the month, "
		"but does not actually take the entire month to cast.";
	f.Paragraph(temp);
	temp = "The description that a mage receives when he first learns a "
		"spell specifies the manner in which the spell is used (automatic, "
		"in combat, or by casting).";
	f.Paragraph(temp);
	f.LinkRef("magic_incombat");
	f.TagText("H3", "Magic in Combat:");
	temp = "NOTE: This section is rather vague, and quite advanced.  You "
		"may want to wait until you have figured out other parts of "
		"Atlantis before trying to understand exactly all of the rules in "
		"this section.";
	f.Paragraph(temp);
	temp = "Although the magic skills and spells are unspecified in these "
		"rules, left for the players to discover, the rules for combat "
		"spells' interaction are spelled out here.  There are five major "
		"types of attacks, and defenses: Combat, Ranged, Energy, Weather, "
		"and Spirit.  Every attack and defense has a type, and only the "
		"appropriate defense is effective against an attack.";
	f.Paragraph(temp);
	temp = "Defensive spells are cast at the beginning of each round of "
		"combat, and will have a type of attack they deflect, and skill "
		"level (Defensive spells are generally called Shields).  Every "
		"time an attack is launched against an army, it must first attack "
		"the highest level Shield of the same type as the attack, before "
		"it may attack a soldier directly. Note that an attack only has "
		"to attack the highest Shield, any other Shields of the same "
		"type are ignored for that attack.";
	f.Paragraph(temp);
	temp = "An attack spell (and any other type of attack) also has an "
		"attack type, and attack level, and a number of blows it deals. "
		"When the attack spell is cast, it is matched up against the most "
		"powerful defensive spell of the appropriate type that the other "
		"army has cast.  If the other army has not cast any applicable "
		"defensive spells, the attack goes through unmolested.  Unlike "
		"normal combat however, men are at a disadvantage to defending "
		"against spells.   Men which are in the open (not protected by "
		"a building) have an effective skill of -2 unless they have a "
		"shield or some other defensive magic.  Some monsters "
		"have bonuses to resisting some attacks but are more susceptible "
		"to others. The skill level of the attack spell and the effective "
		"skill for defense are matched against each other.  The formula "
		"for determining the victor between a defensive and offensive "
		"spell is the same as for a contest of soldiers; if the levels "
		"are equal, there is a 1:1 chance of success, and so on.  If the "
		"offensive spell is victorious, the offensive spell deals its blows "
		"to the defending army, and the Shield in question is destroyed "
		"(thus, it can be useful to have more than one of the same type "
		"of Shield in effect, as the other Shield will take the place of "
		"the destroyed one).  Otherwise, the attack spell disperses, and "
		"the defending spell remains in place.";
	f.Paragraph(temp);
	temp = "Some spells do not actually kill enemies, but rather have some "
		"negative effect on them. These spells are treated the same as "
		"normal spells; if there is a Shield of the same type as them, "
		"they must attack the Shield before attacking the army. "
		"Physical attacks that go through a defensive spell also must "
		"match their skill level against that of the defensive spell in "
		"question.  However, they do not destroy the defensive spell when "
		"they are successful.";
	f.Paragraph(temp);
	f.LinkRef("nonplayers");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Non-Player Units");
	temp = "There are a number of units that are not controlled by players "
		"that may be encountered in Atlantis.  Most information about "
		"these units must be discovered in the course of the game, but a "
		"few basics are below.";
	f.Paragraph(temp);
	if(Globals->TOWNS_EXIST && Globals->CITY_MONSTERS_EXIST) {
		f.LinkRef("nonplayers_guards");
		f.TagText("H3", "City and Town Guardsmen:");
		temp = "All cities and towns begin with guardsmen in them.  These "
			"units will defend any units that are attacked in the city or "
			"town, and will also prevent theft and assassination attempts, ";
		if(has_obse)
			temp += "if their Observation level is high enough. ";
		else
			temp += "if they can see the criminal. ";
		temp += "They are on guard, and will prevent other units from "
			"taxing or pillaging. ";
		if(Globals->SAFE_START_CITIES)
			temp += "Except in the starting cities, the ";
		else
			temp += "The ";
		temp += "guards may be killed by players, although they will form "
			"again if the city is left unguarded.";
		f.Paragraph(temp);
		if(Globals->SAFE_START_CITIES || Globals->START_CITY_GUARDS_PLATE ||
				Globals->START_CITY_MAGES) {
			if(Globals->AMT_START_CITY_GUARDS) {
				temp = "Note that the city guardsmen in the starting cities "
					"of Atlantis possess ";
				if(Globals->SAFE_START_CITIES)
					temp += "Amulets of Invincibility ";
				if(Globals->START_CITY_GUARDS_PLATE) {
					if(Globals->SAFE_START_CITIES) temp += "and ";
					temp += "plate armor ";
				}
				temp += "in addition to being more numerous and ";
				if(Globals->SAFE_START_CITIES)
					temp += "may not be defeated.";
				else
					temp += "are therefore harder to kill.";
			}

			if(Globals->START_CITY_MAGES) {
				if(Globals->AMT_START_CITY_GUARDS)
					temp += " Additionally, in ";
				else
					temp += "In ";
				temp += "the starting cities, Mage Guards will be found. "
					"These mages are adept at the fire spell";
				if(!Globals->SAFE_START_CITIES) {
					temp += " making any attempt to control a starting "
						"city a much harder proposition";
				}
				temp += ".";
			}
			f.Paragraph(temp);
		}
	}
	if (Globals->WANDERING_MONSTERS_EXIST) {
		f.LinkRef("nonplayers_monsters");
		f.TagText("H3", "Wandering Monsters:");
		temp = "There are a number of monsters who wander free throughout "
			"Atlantis.  They will occasionally attack player units, so be "
			"careful when wandering through the wilderness.";
		f.Paragraph(temp);
	}
	f.LinkRef("nonplayers_controlled");
	f.TagText("H3", "Controlled Monsters:");
	temp = "Through various magical methods, you may gain control of "
		"certain types of monsters. These monsters are just another item "
		"in a unit's inventory, with a few special rules. Monsters will "
		"be able to carry things at their speed of movement; use the ";
	temp += f.Link("#show", "SHOW") + " ITEM order to determine the "
		"carrying capacity and movement speed of a monster. Monsters will "
		"also fight for the controlling unit in combat; their strength "
		"can only be determined in battle. Also, note that a monster will "
		"always fight from the front rank, even if the controlling unit "
		"has the behind flag set. Whether or not you are allowed to give a "
		"monster to other units depends on the type of monster; some may be "
		"given freely, while others must remain with the controlling unit.";
	if(Globals->RELEASE_MONSTERS) {
		temp += " All monsters may be released completely by using the ";
		temp += f.Link("#give", "GIVE") + " order targetting unit 0.  When "
			"this is done, the monster will become a wandering monster.";
	}
	f.Paragraph(temp);
	f.LinkRef("orders");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Orders");
	temp = "To enter orders for Atlantis, you should send a mail message "
		"to the Atlantis server, containing the following:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "PRE");
	f.ClearWrapTab();
	f.WrapStr("#ATLANTIS faction-no <password>");
	f.PutNoFormat("");
	f.WrapStr("UNIT unit-no");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("UNIT unit-no");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("#END");
	f.Enclose(0, "PRE");
	temp = "For example, if your faction number (shown at the top of your "
		"report) is 27, your password if \"foobar\", and you have two "
		"units numbered 5 and 17:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "PRE");
	f.WrapStr("#ATLANTIS 27 \"foobar\"");
	f.PutNoFormat("");
	f.WrapStr("UNIT 5");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("UNIT 17");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("#END");
	f.Enclose(0, "PRE");
	temp = "Thus, orders for each unit are given separately, and indicated "
		"with the UNIT keyword.  (In the case of an order, such as the "
		"command to rename your faction, that is not really for any "
		"particular unit, it does not matter which unit issues the command; "
		"but some particular unit must still issue it.)";
	f.Paragraph(temp);
	temp = "IMPORTANT: You MUST use the correct #ATLANTIS line or else your "
		"orders will be ignored.";
	f.Paragraph(temp);
	temp = "If you have a password set, you must specify it on you "
		"#atlantis line, or the game will reject your orders.  See the ";
	temp += f.Link("#password", "PASSWORD") + " order for more details.";
	f.Paragraph(temp);
	temp = "Each type of order is designated by giving a keyword as the "
		"first non-blank item on a line.  Parameters are given after this, "
		"separated by spaces or tabs. Blank lines are permitted, as are "
		"comments; anything after a semicolon is treated as a comment "
		"(provided the semicolon is not in the middle of a word).";
	f.Paragraph(temp);
	temp = "The parser is not case sensitive, so all commands may be given "
		"in upper case, lower case or a mixture of the two.  However, when "
		"supplying names containing spaces, the name must be surrounded "
		"by double quotes, or else underscore characters must be used in "
		"place of spaces in the name.  (These things apply to the #ATLANTIS "
		"and #END lines as well as to order lines.)";
	f.Paragraph(temp);
	temp = "You may precede orders with the at sign (@), in which case they "
		"will appear in the Template at the bottom of your report.  This is "
		"useful for orders which your units repeat for several months in a "
		"row.";
	f.Paragraph(temp);
	f.LinkRef("orders_abbreviations");
	f.TagText("H3", "Abbreviations:");
	temp = "All common items and skills have abbreviations that can be used "
		"when giving orders, for brevity.  Any time you see the item on your "
		"report, it will be followed by the abbreviation.  Please be careful "
		"using these, as they can easily be confused.";
	f.Paragraph(temp);
	f.LinkRef("ordersummary");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Order Summary");
	temp = "To specify a [unit], use the unit number.  If specifying a "
		"unit that will be created this turn, use the form \"NEW #\" if "
		"the unit belongs to your faction, or \"FACTION # NEW #\" if the "
		"unit belongs to a different faction.  See the ";
	temp += f.Link("#form", "FORM");
	temp += " order for a more complete description.  [faction] means that "
		"a faction number is required; [object] means that an object "
		"number (generally the number of a building or ship) is required. "
		"[item] means an item (like wood or longbow) that a unit can have "
		"in its possession. [flag] is an argument taken by several orders, "
		"that sets or unsets a flag for a unit. A [flag] value must be "
		"either 1 (set the flag) or 0 (unset the flag).  Other parameters "
		"are generally numbers or names.";
	f.Paragraph(temp);
	temp = "IMPORTANT: Remember that names containing spaces (e.g., "
		"\"Plate Armor\"), must be surrounded by double quotes, or the "
		"spaces must be replaced with underscores \"_\" (e.g., Plate_Armor).";
	f.Paragraph(temp);
	temp = "Also remember that anything used in an example is just that, "
		"an example and makes no gaurentee that such an item, structure, "
		"or skill actually exists within the game.";
	f.Paragraph(temp);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("address");
	f.TagText("H4", "ADDRESS [new address]");
	f.Paragraph("Change the email address to which your reports are sent.");
	f.Paragraph("Example:");
	temp = "Change your faction's email address to atlantis@rahul.net.";
	temp2 = "ADDRESS atlantis@rahul.net";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("advance");
	f.TagText("H4", "ADVANCE [dir] ...");
	temp = "This is the same as the ";
	temp += f.Link("#move", "MOVE");
	temp += " order, except that it implies attacks on units which attempt "
		"to forbid access.  See the ";
	temp += f.Link("#move", "MOVE") + " order for details.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Move north, then northwest, attacking any units that forbid "
		"access to the regions.";
	temp2 = "ADVANCE N NW";
	f.CommandExample(temp, temp2);
	temp = "In order, move north, then enter structure number 1, move "
		"through an inner route, and finally move southeast. Will attack "
		"any units that forbid access to any of these locations.";
	temp2 = "ADVANCE N 1 IN SE";
	f.CommandExample(temp, temp2);

	if(Globals->USE_WEAPON_ARMOR_COMMAND) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("armor");
		f.TagText("H4", "ARMOR [item1] [item2] [item3] [item4]");
		f.TagText("H4", "ARMOR");
		temp = "This command allows you to set a list of preferred armor "
			"for a unit.  After searching for armor on the preferred "
			"list, the standard armor precedence takes effect if an armor "
			"hasn't been set.  The second form clears the preferred armor "
			"list.";
		f.Paragraph(temp);
		f.Paragraph("Examples");
		temp = "Set the unit to select chain armor before plate armor.";
		temp2 = "ARMOR CARM PARM";
		f.CommandExample(temp, temp2);
		temp = "Clear the preferred armor list.";
		temp2 = "ARMOR";
		f.CommandExample(temp, temp2);
	}

	if(has_stea) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("assassinate");
		f.TagText("H4", "ASSASSINATE [unit]");
		temp = "Attempt to assassinate the specified unit, or one of the "
			"unit's people if the unit contains more than one person.  The "
			"order may only be issued by a one-man unit.";
		f.Paragraph(temp);
		temp = "A unit may only attempt to assassinate a unit which is able "
			"to be seen.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Assassinate unit number 177.";
		temp2 = "ASSASSINATE 177";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("attack");
	f.TagText("H4", "ATTACK [unit] ... ");
	temp = "Attack a target unit.  If multiple ATTACK orders are given, "
		"all of the targets will be attacked.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "To attack units 17, 431, and 985:";
	temp2 = "ATTACK 17\nATTACK 431 985";
	f.CommandExample(temp, temp2);
	temp = "or:";
	temp2 = "ATTACK 17 431 985";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("autotax");
	f.TagText("H4", "AUTOTAX [flag]");
	temp = "AUTOTAX 1 causes the unit to attempt to tax every turn "
		"(without requiring the TAX order) until the flag is unset. "
		"AUTOTAX 0 unsets the flag.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "To cause the unit to attempt to tax every turn.";
	temp2 = "AUTOTAX 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("avoid");
	f.TagText("H4", "AVOID [flag]");
	temp = "AVOID 1 instructs the unit to avoid combat wherever possible. "
		"The unit will not enter combat unless it issues an ATTACK order, "
		"or the unit's faction is attacked in the unit's hex. AVOID 0 "
		"cancels this.";
	f.Paragraph(temp);
	temp = "The Guard and Avoid Combat flags are mutually exclusive; "
		"setting one automatically cancels the other.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the unit to avoid combat when possible.";
	temp2 = "AVOID 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("behind");
	f.TagText("H4", "BEHIND [flag]");
	temp = "BEHIND 1 sets the unit to be behind other units in combat.  "
		"BEHIND 0 cancels this.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the unit to be in front in combat.";
	temp2 = "BEHIND 0";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("build");
	f.TagText("H4", "BUILD");
	f.TagText("H4", "BUILD [object type]");
	f.TagText("H4", "BUILD HELP [unit]");
	temp = "BUILD given with no parameters causes the unit to perform work "
		"on the object that it is currently inside.  BUILD given with an "
		"[object type] (such as \"Tower\" or \"Galleon\") instructs the unit "
		"to begin work on a new object of the type given. The final form "
		"instructs the unit to enter the same building as [unit] and to "
		"assist in building that structure, even if it is a structure which "
		"was begun that same turn.  This help will be rejected if the unit "
		"you are helping does not consider you to be friendly.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "To build a new tower.";
	temp2 = "BUILD Tower";
	f.CommandExample(temp, temp2);
	temp = "To help unit 5789 build a structure.";
	temp2 = "BUILD HELP 5789";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("buy");
	f.TagText("H4", "BUY [quantity] [item]");
	f.TagText("H4", "BUY ALL [item]");
	temp = "Attempt to buy a number of the given item from a city or town "
		"marketplace, or to buy new people in any region where people are "
		"available for recruiting.  If the unit can't afford as many as "
		"[quantity], it will attempt to buy as many as it can. If the "
		"demand for the item (from all units in the region) is greater "
		"than the number available, the available items will be split "
		"among the buyers in proportion to the amount each buyer attempted "
		"to buy. ";
	if (Globals->RACES_EXIST) {
		temp += "When buying people, specify the race of the people as the "
			"[item]. ";
	}
	temp += "If the second form is specified, the unit will attempt to buy "
		"as many as it can afford.";
	f.Paragraph(temp);
	f.Paragraph(AString("Example") + (Globals->RACES_EXIST?"s":"") + ":");
	temp = "Buy one plate armor from the city market.";
	temp2 = "BUY 1 \"Plate Armor\"";
	f.CommandExample(temp, temp2);
	if (Globals->RACES_EXIST) {
		temp = "Recruit 5 barbarians into the current unit. (This will "
			"dilute the skills that the unit has.)";
		temp2 = "BUY 5 barbarians";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("cast");
	f.TagText("H4", "CAST [skill] [arguments]");
	temp = "Cast the given spell.  Note that most spell names contain "
		"spaces; be sure to enclose the name in quotes!  [arguments] "
		"depends on which spell you are casting; when you are able to cast "
		"a spell, the skill description will tell you the syntax.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Cast the spell called \"Super Spell\".";
	temp2 = "CAST \"Super Spell\"";
	f.CommandExample(temp, temp2);
	temp = "Cast the fourth-level spell in the \"Super Magic\" skill.";
	temp2 = "CAST Super_Magic 4";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("claim");
	f.TagText("H4", "CLAIM [amount]");
	temp = "Claim an amount of the faction's unclaimed silver, and give it "
		"to the unit issuing the order.  The claiming unit may then spend "
		"the silver or give it to another unit.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Claim 100 silver.";
	temp2 = "CLAIM 100";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("combat");
	f.TagText("H4", "COMBAT [spell]");
	temp = "Set the given spell as the spell that the unit will cast in "
		"combat.  This order may only be given if the unit can cast the "
		"spell in question.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the unit to use the spell \"Super Spell\", when the "
		"unit is involved in a battle.";
	temp2 = "COMBAT \"Super Spell\"";
	f.CommandExample(temp, temp2);

	if (Globals->FOOD_ITEMS_EXIST) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("consume");
		f.TagText("H4", "CONSUME UNIT");
		f.TagText("H4", "CONSUME FACTION");
		f.TagText("H4", "CONSUME");
		temp = "The CONSUME order instructs the unit to use food items in "
			"preference to silver for maintenance costs. CONSUME UNIT tells "
			"the unit to use food items that are in that unit's possession "
			"before using silver. CONSUME FACTION tells the unit to use any "
			"food items that the faction owns (in the same region as the "
			"unit) before using silver. CONSUME tells the unit to use "
			"silver before food items (this is the default).";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Tell a unit to use food items in the unit's possession for "
			"maintenance costs.";
		temp2 = "CONSUME UNIT";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("declare");
	f.TagText("H4", "DECLARE [faction] [attitude]");
	f.TagText("H4", "DECLARE [faction]");
	f.TagText("H4", "DECLARE DEFAULT [attitude]");
	temp = "The first form of the DECLARE order sets the attitude of your "
		"faction towards the given faction.  The second form cancels any "
		"attitude towards the given faction (so your faction's attitude "
		"towards that faction will be its default attitude).  The third "
		"form sets your faction's default attitude.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Declare your faction to be hostile to faction 15.";
	temp2 = "DECLARE 15 hostile";
	f.CommandExample(temp, temp2);
	temp = "Set your faction's attitude to faction 15 to its default "
		"attitude.";
	temp2 = "DECLARE 15";
	f.CommandExample(temp, temp2);
	temp = "Set your faction's default attitude to friendly.";
	temp2 = "DECLARE DEFAULT friendly";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("describe");
	f.TagText("H4", "DESCRIBE UNIT [new description]");
	f.TagText("H4", "DESCRIBE SHIP [new description]");
	f.TagText("H4", "DESCRIBE BUILDING [new description]");
	f.TagText("H4", "DESCRIBE OBJECT [new description]");
	f.TagText("H4", "DESCRIBE STRUCTURE [new description]");
	temp = "Change the description of the unit, or of the object the unit "
		"is in (of which the unit must be the owner). Descriptions can be "
		"of any length, up to the line length your mailer can handle. If "
		"no description is given, the description will be cleared out. The "
		"last four are completely identical and serve to modify the "
		"description of the object you are currently in.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the unit,s description to read \"Merlin's helper\".";
	temp2 = "DESCRIBE UNIT \"Merlin's helper\"";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("destroy");
	f.TagText("H4", "DESTROY");
	temp = "Destroy the object you are in (of which you must be the owner). "
		"The order cannot be used at sea.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Destroy the current object";
	temp2 = "DESTROY";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("enter");
	f.TagText("H4", "ENTER [object]");
	temp = "Attempt to enter the specified object.  If issued from inside "
		"another object, the unit will first leave the object it is "
		"currently in.  The order will only work if the target object is "
		"unoccupied, or is owned by a unit in your faction, or is owned by "
		"a unit which has declared you Friendly.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Enter ship number 114.";
	temp2 = "ENTER 114";
	f.CommandExample(temp, temp2);

	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("entertain");
		f.TagText("H4", "ENTERTAIN");
		temp = "Spend the month entertaining the populace to earn money.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Entertain for money.";
		temp2 = "ENTERTAIN";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("evict");
	f.TagText("H4", "EVICT [unit] ...");
	temp = "Evict the specified unit from the object of which you are "
		"currently the owner.  If multiple EVICT orders are given, all "
		"of the units will be evicted.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Evict units 415 and 698 from an object that this unit owns.";
	temp2 = "EVICT 415 698";
	f.CommandExample(temp, temp2);
	temp = "or";
	temp2 = "EVICT 415\nEVICT 698";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("exchange");
	f.TagText("H4", "EXCHANGE [unit] [quantity given] [item given] "
			"[quantity expected] [item expected]");
	temp = "This order allows any two units that can see each other, to "
		"trade items regardless of faction stances.  The orders given by "
		"the two units must be complementary.  If either unit involved does "
		"not have the items it is offering, or if the exchange orders given "
		"are not complementary, the exchange is aborted.  Men may not be "
		"exchanged.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Exchange 10 LBOW for 10 SWOR with unit 1310";
	temp2 = "EXCHANGE 1310 10 LBOW 10 SWOR";
	f.CommandExample(temp, temp2);
	temp = "Unit 1310 would issue (assuming the other unit is 3453)";
	temp2 = "EXCHANGE 3453 10 SWOR 10 LBOW";
	f.CommandExample(temp, temp2);

	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("faction");
		f.TagText("H4", "FACTION [type] [points] ...");
		temp = "Attempt to change your faction's type.  In the order, you "
			"can specify up to three faction types (WAR, TRADE, and MAGIC) "
			"and the number of faction points to assign to each type; if "
			"you are assigning points to only one or two types, you may "
			"omit the types that will not have any points.";
		f.Paragraph(temp);
		temp = "Changing the number of faction points assigned to MAGIC may "
			"be tricky. Increasing the MAGIC points will always succeed, but "
			"if you decrease the number of points assigned to MAGIC, you "
			"must make sure that you have only the number of magic-skilled "
			"leaders allowed by the new number of MAGIC points BEFORE you "
			"change your point distribution. For example, if you have 3 "
			"mages (3 points assigned to MAGIC), but want to use one of "
			"those points for WAR or TRADE (change to MAGIC 2), you must "
			"first get rid of one of your mages by either giving it to "
			"another faction or ordering it to ";
		temp += f.Link("#forget", "FORGET") + " all its magic skills. ";
		temp += "If you have too many mages for the number of points you "
			"try to assign to MAGIC, the FACTION order will fail.";
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		temp = "Assign 2 faction points to WAR, 2 to TRADE, and 1 to MAGIC.";
		temp2 = "FACTION WAR 2 TRADE 2 MAGIC 1";
		f.CommandExample(temp, temp2);
		temp = "Become a pure magic faction (assign all points to magic).";
		temp2 = "FACTION MAGIC ";
		temp2 += Globals->FACTION_POINTS;
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("find");
	f.TagText("H4", "FIND [faction]");
	f.TagText("H4", "FIND ALL");
	temp = "Find the email address of the specified faction or of all "
		"factions.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Find the email address of faction 4.";
	temp2 = "FIND 4";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("forget");
	f.TagText("H4", "FORGET [skill]");
	temp = "Forget the given skill. This order is useful for normal units "
		"who wish to learn a new skill, but already know a different skill.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Forget knowledge of Mining.";
	temp2 = "FORGET Mining";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("form");
	f.TagText("H4", "FORM [alias]");
	temp = "Form a new unit.  The newly created unit will be in your "
		"faction, in the same region as the unit which formed it, and in "
		"the same structure if any.  It will start off, however, with no "
		"people or items; you should, in the same month, issue orders to "
		"transfer people into the new unit, or have it recruit members. The "
		"new unit will inherit its flags from the unit that forms it, such "
		"as avoiding, behind, and autotax.";
	f.Paragraph(temp);
	temp = "The FORM order is followed by a list of orders for the newly "
		"created unit.  This list is terminated by the END keyword, after "
		"which orders for the original unit resume.";
	f.Paragraph(temp);
	temp = "The purpose of the \"alias\" parameter is so that you can refer "
		"to the new unit. You will not know the new unit's number until "
		"you receive the next turn report.  To refer to the new unit in "
		"this set of orders, pick an alias number (the only restriction on "
		"this is that it must be at least 1, and you should not create two "
		"units in the same region in the same month, with the same alias "
		"numbers).  The new unit can then be referred to as NEW <alias> in "
		"place of the regular unit number.";
	f.Paragraph(temp);
	temp = "You can refer to newly created units belonging to other "
		"factions, if you know what alias number they are, e.g. FACTION 15 "
		"NEW 2 will refer to faction 15's newly created unit with alias 2.";
	f.Paragraph(temp);
	temp = "Note: If a unit moves out of the region in which it was formed "
		"(by the ";
	temp += f.Link("#move", "MOVE") + " order, or otherwise), the alias "
		"will no longer work. This is to prevent conflicts with other units "
		"that may have the same alias in other regions.";
	f.Paragraph(temp);
	temp = "If the demand for recruits in that region that month is much "
		"higher than the supply, it may happen that the new unit does not "
		"gain all the recruits you ordered it to buy, or it may not gain "
		"any recruits at all.  If the new units gains at least one recruit, "
		"the unit will form possessing any unused silver and all the other "
		"items it was given.  If no recruits are gained at all, the empty "
		"unit will be dissolved, and the silver and any other items it was "
		"given will revert to the first unit you have in that region.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "This set of orders for unit 17 would create two new units with "
		"alias numbers 1 and 2, name them Merlin's Guards and Merlin's "
		"Workers, set the description for Merlin's Workers, have both units "
		"recruit men, and have Merlin's Guards study combat.  Merlin's "
		"Workers will have the default order ";
	temp += f.Link("#work", "WORK") + ", as all newly created units do. The "
		"unit that created these two then pays them enough money (using the "
		"NEW keyword to refer to them by alias numbers) to cover the costs "
		"of recruitment and the month's maintenance.";
	temp2 = "UNIT 17\n";
	temp2 += "FORM 1\n";
	temp2 += "    NAME UNIT \"Merlin's Guards\"\n";
	if(Globals->RACES_EXIST)
		temp2 += "    BUY 5 Plainsmen\n";
	else
		temp2 += "    BUY 5 men\n";
	temp2 += "    STUDY COMBAT\n";
	temp2 += "END\n";
	temp2 += "FORM 2\n";
	temp2 += "    NAME UNIT \"Merlin's Workers\"\n";
	temp2 += "    DESCRIBE UNIT \"wearing dirty overalls\"\n";
	if(Globals->RACES_EXIST)
		temp2 += "    BUY 15 Plainsmen\n";
	else
		temp2 += "    BUY 15 men\n";
	temp2 += "END\n";
	temp2 += "CLAIM 2500\n";
	temp2 += "GIVE NEW 1 1000 silver\n";
	temp2 += "GIVE NEW 2 2000 silver\n";
	f.CommandExample(temp,temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("give");
	f.TagText("H4", "GIVE [unit] [quantity] [item]");
	f.TagText("H4", "GIVE [unit] ALL [item]");
	f.TagText("H4", "GIVE [unit] ALL [item] EXCEPT [quantity]");
	f.TagText("H4", "GIVE [unit] ALL [item class]");
	f.TagText("H4", "GIVE [unit] UNIT");
	temp = "The first form of the GIVE order gives a quantity of an item to "
		"another unit. The second form of the GIVE order will give all of "
		"a given item to another unit.  The third form will give all of an "
		"item except for a specific quantity to another unit.  The fourth "
		"form will give all items of a specific type to another unit.  The "
		"final form of the GIVE order gives the entire unit to the "
		"specified unit's faction.";
	f.Paragraph(temp);
	temp = "The classes of items which are acceptable for the fourth form of "
		"this order are, NORMAL, ADVANCED, TRADE, MAN or MEN, MONSTER or "
		"MONSTERS, MAGIC, WEAPON OR WEAPONS, ARMOR, MOUNT or MOUNTS, BATTLE, "
		"SPECIAL, TOOL or TOOLS, FOOD, and ITEM or ITEMS (which is the "
		"combination of all of the previous categories).";
	f.Paragraph(temp);
	temp = "A unit may only give items, including silver, to a unit which "
		"it is able to see, unless the faction of the target unit has "
		"declared you Friendly or better.  If the target unit is not a "
		"member of your faction, then its faction must have declared you "
		"Friendly, with a couple of exceptions. First, silver may be given "
		"to any unit, regardless of factional affiliation. Secondly, men "
		"may not be given to units in other factions (you must give the "
		"entire unit); the reason for this is to prevent highly skilled "
		"units from being sabotaged with a ";
	temp += f.Link("#give", "GIVE") + " order.";
	f.Paragraph(temp);
	temp = "There are also a few restrictions on orders given by units who "
		"been given to another faction. If the receiving faction is not "
		"allied to the giving faction, the unit may not issue the ";
	temp += f.Link("#advance", "ADVANCE") + " order, or issue any more ";
	temp += f.Link("#give", "GIVE") + " orders.  Both of these rules are to "
		"prevent unfair sabotage tactics.";
	f.Paragraph(temp);
	temp = "If 0 is specified as the unit number, then the items are "
		"discarded.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Give 10 swords to unit 4573.";
	temp2 = "GIVE 4573 10 swords";
	f.CommandExample(temp, temp2);
	temp = "Give 5 chain armor to the new unit, alias 2, belonging to "
		"faction 14.";
	temp2 = "GIVE FACTION 14 NEW 2 5 \"Chain armor\"";
	f.CommandExample(temp, temp2);
	temp = "Give control of this unit to the faction owning unit 75.";
	temp2 = "GIVE 75 UNIT";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("guard");
	f.TagText("H4", "GUARD [flag]");
	temp = "GUARD 1 sets the unit issuing the order to prevent non-Friendly "
		"units from collecting taxes in the region, and to prevent any "
		"units not your own from pillaging the region.  Guarding units "
		"will also attempt to prevent Unfriendly units from entering the "
		"region.  GUARD 0 cancels Guard status.";
	f.Paragraph(temp);
	temp = "The Guard and Avoid Combat flags are mutually exclusive; "
		"setting one automatically cancels the other.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the current unit to be on guard.";
	temp2 = "GUARD 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("hold");
	f.TagText("H4", "HOLD [flag]");
	temp = "HOLD 1 instructs the issuing unit to never join a battle in "
		"regions the unit is not in.  This can be useful if the unit is in "
		"a building, and doesn't want to leave the building to join combat. "
		"HOLD 0 cancels holding status.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the unit to avoid combat in other regions.";
	temp2 = "HOLD 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("leave");
	f.TagText("H4", "LEAVE");
	temp = "Leave the object you are currently in.";
	if(move_over_water) {
		temp += " If a unit is capable of swimming ";
		if(Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE)
			temp += "or flying ";
		temp += "then this order is usable to leave a boat while at sea.";
	} else
		temp += " The order cannot be used at sea.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Leave the current object";
	temp2 = "LEAVE";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("move");
	f.TagText("H4", "MOVE [dir] ...");
	temp = "Attempt to move in the direction(s) specified.  If more than "
		"one direction is given, the unit will move multiple times, in "
		"the order specified by the MOVE order, until no more directions "
		"are given, or until one of the moves fails.  A move can fail "
		"because the units runs out of movement points, because the unit "
		"attempts to move into the ocean, or because the units attempts "
		"to enter a structure, and is rejected.";
	f.Paragraph(temp);
	temp = "Valid directions are:";
	f.Paragraph(temp);
	temp = "1) The compass directions North, Northwest, Southwest, South, "
		"Southeast, and Northeast.  These can be abbreviated N, NW, SW, S, "
		"SE, NE.";
	f.Paragraph(temp);
	temp = "2) A structure number.";
	f.Paragraph(temp);
	temp = "3) OUT, which will leave the structure that the unit is in.";
	f.Paragraph(temp);
	temp = "4) IN, which will move through an inner passage in the "
		"structure that the unit is currently in.";
	f.Paragraph(temp);
	temp = "Multiple MOVE orders given by one unit will chain together.";
	f.Paragraph(temp);
	temp = "Note that MOVE orders can lead to combat, due to hostile units "
		"meeting, or due to an advancing unit being forbidden access to a "
		"region.  In this case, combat occurs each time all movement out "
		"of a single region occurs.";
	f.Paragraph(temp);
	temp = "Example 1: Units 1 and 2 are in Region A, and unit 3 is in "
		"Region B.  Units 1 and 2 are hostile to unit 3.  Both unit 1 and "
		"2 move into region B, and attack unit 3.  Since both units moved "
		"out of the same region, they attack unit 3 at the same time, and "
		"the battle is between units 1 and 2, and unit 3.";
	f.Paragraph(temp);
	temp = "Example 2: Same as example 1, except unit 2 is in Region C, "
		"instead of region A.  Both units move into Region B, and attack "
		"unit 3.  Since unit 1 and unit 2 moved out of different regions, "
		"their battles occur at different times.  Thus, unit 1 attacks unit "
		"3 first, and then unit 2 attacks unit 3 (assuming unit 3 survives "
		"the first attack).  Note that the order of battles could have "
		"happened either way.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Move N, NE and In";
	temp2 = "MOVE N\nMOVE NE IN";
	f.CommandExample(temp, temp2);
	temp = "or:";
	temp2 = "MOVE N NE IN";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("name");
	f.TagText("H4", "NAME UNIT [new name]");
	f.TagText("H4", "NAME FACTION [new name]");
	f.TagText("H4", "NAME OBJECT [new name]");
	if (Globals->TOWNS_EXIST)
		f.TagText("H4", "NAME CITY [new name]");
	temp = "Change the name of the unit, or of your faction, or of "
		"the object the unit is in (of which the unit must be the owner). "
		"Names can be of any length, up to the line length your mailer "
		"can handle.  Names may not contain parentheses (square brackets "
		"can be used instead if necessary), or any control characters.";
	f.Paragraph(temp);
	if (Globals->TOWNS_EXIST) {
		temp = "In order to rename a settlement (city, town or village), "
			"the unit attempting to rename it must be the owner of a large "
			"enough structure located in the city. It requires a tower or "
			"better to rename a village, a fort or better to rename a town "
			"and a castle or mystic fortress to rename a city. ";
		if (Globals->CITY_RENAME_COST) {
			int c=Globals->CITY_RENAME_COST;
			temp += AString("It also costs $") + c + " to rename a village, $";
			temp += AString(2*c) + " to rename a town, and $";
			temp += AString(3*c) + " to rename a city.";
		}
		f.Paragraph(temp);
	}
	f.Paragraph("Example:");
	temp = "Name your faction \"The Merry Pranksters\".";
	temp2 = "NAME FACTION \"The Merry Pranksters\"";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("noaid");
	f.TagText("H4", "NOAID [flag]");
	temp = "NOAID 1 indicates that if the unit attacks, or is attacked, it "
		"is not to be aided by units in other hexes. NOAID status is very "
		"useful for scouts or probing units, who do not wish to drag "
		"their nearby armies into battle if they are caught. NOAID 0 "
		"cancels this.";
	f.Paragraph(temp);
	temp = "If multiple units are on one side in a battle, they must all "
		"have the NOAID flag on, or they will receive aid from other hexes.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set a unit to receive no aid in battle.";
	temp2 = "NOAID 1";
	f.CommandExample(temp, temp2);

	if(move_over_water) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("nocross");
		f.TagText("H4", "NOCROSS [flag]");
		temp = "NOCROSS 1 indicates that if a unit attempts to cross a "
			"body of water then that unit should instead not cross it, "
			"regardless of whether the unit otherwise could do so. ";
		if(may_sail) {
			temp += "Units inside of a ship are not affected by this flag "
				"(IE, they are able to sail within the ship). ";
		}
		temp += "This flag is useful to prevent scouts from accidentally "
			"drowning when exploring in games where movement over water "
			"is allowed. NOCROSS 0 cancels this.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Set a unit to not permit itself to cross water.";
		temp2 = "NOCROSS 1";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("option");
	f.TagText("H4", "OPTION TIMES");
	f.TagText("H4", "OPTION NOTIMES");
	f.TagText("H4", "OPTION TEMPLATE OFF");
	f.TagText("H4", "OPTION TEMPLATE SHORT");
	f.TagText("H4", "OPTION TEMPLATE LONG");
	f.TagText("H4", "OPTION TEMPLATE MAP");
	temp = "The OPTION order is used to toggle various settings that "
		"affect your reports, and other email details. OPTION TIMES sets it "
		"so that your faction receives the times each week (this is the "
		"default); OPTION NOTIMES sets it so that your faction is not sent "
		"the times.";
	f.Paragraph(temp);
	temp = "The OPTION TEMPLATE order toggles the length of the Orders "
		"Template that appears at the bottom of a turn report.  The OFF "
		"setting eliminates the Template altogether, and the SHORT, LONG "
		"and MAP settings control how much detail the Template contains. "
		"The MAP setting will produce an ascii map of the region and "
		"surrounding regions in addition other details.";
	f.Paragraph(temp);
	temp = "For the MAP template, the region identifiers are (there might "
		"be additional symbols for unusual/special terrain):";
	f.Paragraph(temp);
	f.Enclose(1, "TABLE");
	if(Globals->UNDERWORLD_LEVELS) {
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
		f.PutStr("####");
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr("BLOCKED HEX (Underworld)");
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(1, "TR");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
	f.PutStr("~~~~");
	f.Enclose(0, "TD");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
	f.PutStr("OCEAN HEX");
	f.Enclose(0, "TD");
	f.Enclose(0, "TR");
	f.Enclose(1, "TR");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
	f.PutStr("    ");
	f.Enclose(0, "TD");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
	temp = "PLAINS";
	if(Globals->UNDERWORLD_LEVELS)
		temp += "/TUNNELS";
	temp += " HEX";
	f.PutStr(temp);
	f.Enclose(0, "TD");
	f.Enclose(0, "TR");
	f.Enclose(1, "TR");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
	f.PutStr("^^^^");
	f.Enclose(0, "TD");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
	temp = "FOREST";
	if(Globals->UNDERWORLD_LEVELS)
		temp += "/UNDERFOREST";
	temp += " HEX";
	f.PutStr(temp);
	f.Enclose(0, "TD");
	f.Enclose(0, "TR");
	f.Enclose(1, "TR");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
	f.PutStr("/\\/\\");
	f.Enclose(0, "TD");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
	f.PutStr("MOUNTAIN HEX");
	f.Enclose(0, "TD");
	f.Enclose(0, "TR");
	f.Enclose(1, "TR");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
	f.PutStr("vvvv");
	f.Enclose(0, "TD");
	f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
	f.PutStr("SWAMP HEX");
	f.Enclose(0, "TD");
	f.Enclose(0, "TR");
	if(!Globals->CONQUEST_GAME) {
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
		f.PutStr("@@@@");
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr("JUNGLE HEX");
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
		f.PutStr("....");
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		temp = "DESERT";
		if(Globals->UNDERWORLD_LEVELS)
			temp += "/CAVERN";
		temp += " HEX";
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
		f.PutStr(",,,,");
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr("TUNDRA HEX");
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	if(Globals->NEXUS_EXISTS) {
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP CLASS=fixed");
		f.PutStr("!!!!");
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr("THE NEXUS");
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Paragraph("Example:");
	temp = "Set your faction to recieve the map format order template";
	temp2 = "OPTION TEMPLATE MAP";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("password");
	f.TagText("H4", "PASSWORD [password]");
	f.TagText("H4", "PASSWORD");
	temp = "The PASSWORD order is used to set your faction's password. If "
		"you have a password set, you must specify it on your #ATLANTIS "
		"line for the game to accept your orders.  This protects you orders "
		"from being overwritten, either by accident or intentionally by "
		"other players.  PASSWORD with no password given clears out your "
		"faction's password.";
	f.Paragraph(temp);
	temp = "IMPORTANT: The PASSWORD order does not take effect until the "
		"turn is actually run.  So if you set your password, and then want "
		"to re-submit orders, you should use the old password until the "
		"turn has been run.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the password to \"xyzzy\".";
	temp2 = "PASSWORD xyzzy";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("pillage");
	f.TagText("H4", "PILLAGE");
	temp = "Use force to extort as much money as possible from the region. "
		"Note that the ";
	temp += f.Link("#tax", "TAX") + " order and the PILLAGE order are ";
	temp += "mutually exclusive; a unit may only attempt to do one in a "
		"turn.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Pillage the current hex.";
	temp2 = "PILLAGE";
	f.CommandExample(temp, temp2);

	if(Globals->USE_PREPARE_COMMAND) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("prepare");
		f.TagText("H4", "PREPARE [item]");
        temp = "This command allows a mage or apprentice to prepare a "
			"battle item (e.g. a Staff of Fire) for use in battle. ";
		if (Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_STRICT) {
			temp += " This selects the battle item which will be used, ";
		} else {
			temp += "This allows the mage to override the usual selection "
				"of battle items, ";
		}
		temp += "and also cancels any spells set via the ";
		temp += f.Link("#combat", "COMBAT") + " order.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Select a staff of fire as the ";
		if (!(Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_STRICT))
			temp += "preferred ";
		temp += "battle item.";
		temp2 = "PREPARE STAF";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("produce");
	f.TagText("H4", "PRODUCE [item]");
	temp = "Spend the month producing as much as possible of the specified "
		"item.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Produce as many crossbows as possible.";
	temp2 = "PRODUCE crossbows";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("promote");
	f.TagText("H4", "PROMOTE [unit]");
	temp = "Promote the specified unit to owner of the object of which you "
		"are currently the owner.  The target unit must have declared you "
		"Friendly.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Promote unit 415 to be the owner of the object that this unit "
		"owns.";
	temp2 = "PROMOTE 415";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("quit");
	f.TagText("H4", "QUIT [password]");
	temp = "Quit the game.  On issuing this order, your faction will be "
		"completely and permanently destroyed. Note that you must give "
		"your password for the quit order to work; this is to provide "
		"some safety against accidentally issuing this order.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Quit the game if your password is foobar.";
	temp2 = "QUIT \"foobar\"";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("restart");
	f.TagText("H4", "RESTART [password]");
	temp = "Similar to the ";
	temp += f.Link("#quit", "QUIT") + " order, this order will completely "
		"and permanently destroy your faction. However, it will begin a "
		"brand new faction for you (you will get a separate turn report for "
		"the new faction). Note that you must give your password for this "
		"order to work, to provide some protection against accidentally "
		"issuing this order.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Restart as a new faction if your password is foobar.";
	temp2 = "RESTART \"foobar\"";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("reveal");
	f.TagText("H4", "REVEAL");
	f.TagText("H4", "REVEAL UNIT");
	f.TagText("H4", "REVEAL FACTION");
	temp = "Cause the unit to either show itself (REVEAL UNIT), or show "
		"itself and its faction affiliation (REVEAL FACTION), in the turn "
		"report, to all other factions in the region. ";
	if(has_stea) {
		temp += "Used to reveal high stealth scouts, should there be some "
			"reason to. ";
	}
	temp += "REVEAL is used to cancel this.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Show the unit to all factions.";
	temp2 = "REVEAL UNIT";
	f.CommandExample(temp, temp2);
	temp = "Show the unit and it's affiliation to all factions.";
	temp2 = "REVEAL FACTION";
	f.CommandExample(temp, temp2);
	temp = "Cancels revealling.";
	temp2 = "REVEAL";
	f.CommandExample(temp, temp2);

	if(may_sail) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("sail");
		f.TagText("H4", "SAIL [dir] ...");
		f.TagText("H4", "SAIL");
		temp = "The first form will sail the ship, which the unit must be "
			"the owner of, in the directions given.  The second form "
			"will cause the unit to aid in the sailing of the ship, using "
			"the Sailing skill.  See the section on movement for more "
			"information on the mechanics of sailing.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Sail north, then northwest.";
		temp2 = "SAIL N NW";
		f.CommandExample(temp, temp2);
		temp = "or:";
		temp2 = "SAIL N\nSAIL NW";
		f.CommandExample(temp, temp2);
	}

	if(Globals->TOWNS_EXIST) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("sell");
		f.TagText("H4", "SELL [quantity] [item]");
		f.TagText("H4", "SELL ALL [item]");
		temp = "Attempt to sell the amount given of the item given.  If the "
			"unit does not have as many of the item as it is trying to sell, "
			"it will attempt to sell all that it has. The second form will "
			"attempt to sell all of that item, regardless of how many it has. "
			"If more of the item are on sale (by all the units in the region) "
			"than are wanted by the region, the number sold per unit will be "
			"split up in proportion to the number each unit tried to sell.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Sell 10 furs to the market.";
		temp2 = "SELL 10 furs";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("show");
	f.TagText("H4", "SHOW SKILL [skill] [level]");
	f.TagText("H4", "SHOW ITEM [item]");
	f.TagText("H4", "SHOW OBJECT [object]");
	temp = "The first form of the order shows the skill description for a "
		"skill that your faction already possesses. The second form "
		"returns some information about an item that is not otherwise "
		"apparent on a report, such as the weight. The last form "
		"returns some information about an object (such as a ship or a "
		"building).";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Show the skill report for Mining 3 again.";
	temp2 = "SHOW SKILL Mining 3";
	f.CommandExample(temp, temp2);
	temp = "Show the item information for swords again.";
	temp2 = "SHOW ITEM sword";
	f.CommandExample(temp, temp2);
	temp = "Show the information for towers again.";
	temp2 = "SHOW OBJECT tower";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("spoils");
	f.TagText("H4", "SPOILS [type]");
	f.TagText("H4", "SPOILS");
	temp = "The SPOILS order determines which types of spoils the unit "
		"should take after a battle.  The valid values for type are "
		"'NONE', 'WALK', 'RIDE', 'FLY', or 'ALL'. The second form is "
		"equivalent to 'SPOILS ALL'.";
	f.Paragraph(temp);
	temp = "When this command is issued, only spoils with 0 weight (at "
		"level NONE) or spoils which weigh less than or equal to their "
		"capacity in the specified movement mode (at any level other than "
		"ALL) will be picked up.  SPOILS ALL will allow a unit to collect "
		"any spoils which are dropped regardless of weight or capacity.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set a unit to only pick up items which have flying capacity";
	temp2 = "SPOILS FLY";
	f.CommandExample(temp, temp2);

	if(has_stea) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("steal");
		f.TagText("H4", "STEAL [unit] [item]");
		temp = "Attempt to steal as much as possible of the specified "
			"item from the specified unit. The order may only be issued "
			"by a one-man unit.";
		f.Paragraph(temp);
		temp = "A unit may only attempt to steal from a unit which is "
			"able to be seen.";
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		temp = "Steal silver from unit 123.";
		temp2 = "STEAL 123 SILVER";
		f.CommandExample(temp, temp2);
		temp = "Steal wood from unit 321.";
		temp2 = "STEAL 321 wood";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("study");
	f.TagText("H4", "STUDY [skill]");
	temp = "Spend the month studying the specified skill.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Study horse training.";
	temp2 = "STUDY \"Horse Training\"";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("tax");
	f.TagText("H4", "TAX");
	temp = "Attempt to collect taxes from the region. ";
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp += "Only War factions may collect taxes, and then ";
	else
		temp += "Taxes may be collected ";
	temp += "only if there are no non-Friendly units on guard. Only "
		"combat-ready units may issue this order. Note that the TAX order "
		"and the ";
	temp += f.Link("#pillage", "PILLAGE") + " order are mutually exclusive; "
		"a unit may only attempt to do one in a turn.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Attempt to collect taxes.";
	temp2 = "TAX";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("teach");
	f.TagText("H4", "TEACH [unit] ...");
	temp = "Attempt to teach the specified units whatever skill they are "
		"studying that month.  A list of several units may be specified. "
		"All units to be taught must have declared you Friendly. "
		"Subsequent TEACH orders can be used to add units to be taught.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Teach new unit 2 and unit 510 whatever they are studying.";
	temp2 = "TEACH NEW 2 510";
	f.CommandExample(temp, temp2);
	temp = "or:";
	temp2 = "TEACH NEW 2\nTEACH 510";
	f.CommandExample(temp, temp2);

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("turn");
	f.TagText("H4", "TURN");
	temp = "The TURN order may be used to delay orders by one (or more) "
		"turns. By making the TURN order repeating (via '@'), orders inside "
		"the TURN/ENDTURN construct will repeat.  Multiple TURN orders in a "
		"row will execute on successive turns, and if they all repeat, they "
		"will form a loop of orders.  Each TURN section must be ended by an "
		"ENDTURN line.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Study combat this month, move north next month, and then in two "
		"months, pillaging and advance north.";
	temp2 = "STUDY COMB\n";
	temp2 += "TURN\n";
	temp2 += "    MOVE N\n";
	temp2 += "ENDTURN\n";
	temp2 += "TURN\n";
	temp2 += "    PILLAGE\n";
	temp2 += "    ADVANCE N\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "After the turn, the orders for that unit would look as "
		"follows in the orders template:";
	temp2 = "MOVE N\n";
	temp2 += "TURN\n";
	temp2 += "    PILLAGE\n";
	temp2 += "    ADVANCE N\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "Set up a simple cash caravan (It's assumed here that someone is "
		"funnelling cash into this unit.";
	temp2 = "MOVE N\n";
	temp2 += "@TURN\n";
	temp2 += "    GIVE 13523 1000 SILV\n";
	temp2 += "    MOVE S S S\n";
	temp2 += "ENDTURN\n";
	temp2 += "@TURN\n";
	temp2 += "    MOVE N N N\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "After the turn, the orders for that unit would look as "
		"follows in the orders template:";
	temp2 = "GIVE 13523 1000 SILV\n";
	temp2 += "MOVE S S S\n";
	temp2 += "@TURN\n";
	temp2 += "    MOVE N N N\n";
	temp2 += "ENDTURN\n";
	temp2 += "@TURN\n";
	temp2 += "    GIVE 13523 1000 SILV\n";
	temp2 += "    MOVE S S S\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "If the unit does not have enough movement points to cover "
			"the full distance, the MOVE commands will automatically "
			"be completed over multiple turns before executing the next "
			"TURN block.";
	f.Paragraph(temp);

	if(Globals->USE_WEAPON_ARMOR_COMMAND) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("weapon");
		f.TagText("H4", "WEAPON [item] ...");
		f.TagText("H4", "WEAPON");
		temp = "This command allows you to set a list of preferred weapons "
			"for a unit.  After searching for weapons on the preferred "
			"list, the standard weapon precedence takes effect if a weapon "
			"hasn't been set.  The second form clears the preferred weapon "
			"list.";
		f.Paragraph(temp);
		f.Paragraph("Examples");
		temp = "Set the unit to select double bows, then longbows then "
			"crossbows";
		temp2 = "WEAPON DBOW LBOW XBOW";
		f.CommandExample(temp, temp2);
		temp = "Clear the preferred weapon list.";
		temp2 = "WEAPON";
		f.CommandExample(temp, temp2);
	}

	if(Globals->ALLOW_WITHDRAW) {
		f.ClassTagText("DIV", "rule", "");
		f.LinkRef("withdraw");
		f.TagText("H4", "WITHDRAW [item]");
		f.TagText("H4", "WITHDRAW [quantity] [item]");
		temp = "Use unclaimed funds to aquire basic items that you need. "
			"If you do not have sufficient unclaimed, or if you try "
			"withdraw any other than a basic item, an error will be given. "
			"Withdraw CANNOT be used in the Nexus (to prevent building "
			"towers and such there).  The first form is the same as "
			"WITHDRAW 1 [item] in the second form.";
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		temp = "Withdraw 5 stone.";
		temp2 = "WITHDRAW 5 stone";
		f.CommandExample(temp, temp2);
		temp = "Withdraw 1 iron.";
		temp2 = "WITHDRAW iron";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("DIV", "rule", "");
	f.LinkRef("work");
	f.TagText("H4", "WORK");
	temp = "Spend the month performing manual work for wages.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Work all month.";
	temp2 = "WORK";
	f.CommandExample(temp, temp2);

	f.LinkRef("sequenceofevents");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Sequence of Events");
	temp = "Each turn, the following sequence of events occurs:";
	f.Paragraph(temp);
	f.Enclose(1, "OL");
	f.Enclose(1, "LI");
	f.PutStr("Instant orders.");
	f.Enclose(1, "UL");
	temp = f.Link("#turn", "TURN") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#form", "FORM") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#address", "ADDRESS") + ", ";
	if(Globals->USE_WEAPON_ARMOR_COMMAND)
		temp += f.Link("#armor", "ARMOR") + ", ";
	temp += f.Link("#autotax", "AUTOTAX") + ", ";
	temp += f.Link("#avoid", "AVOID") + ", ";
	temp += f.Link("#behind", "BEHIND") + ", ";
	temp += f.Link("#claim", "CLAIM") + ", ";
	temp += f.Link("#combat", "COMBAT") + ", ";
	if(Globals->FOOD_ITEMS_EXIST)
		temp += f.Link("#consume", "CONSUME") + ", ";
	temp += f.Link("#declare", "DECLARE") + ", ";
	temp += f.Link("#describe", "DESCRIBE") + ", ";
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp += f.Link("#faction", "FACTION") + ", ";
	temp += f.Link("#guard", "GUARD") + " 0, ";
	temp += f.Link("#hold", "HOLD") + ", ";
	temp += f.Link("#name", "NAME") + ", ";
	temp += f.Link("#noaid", "NOAID") + ", ";
	if(move_over_water)
		temp += f.Link("#nocross", "NOCROSS") + ", ";
	temp += f.Link("#option", "OPTION") + ", ";
	temp += f.Link("#password", "PASSWORD") + ", ";
	if(Globals->USE_PREPARE_COMMAND)
		temp += f.Link("#prepare", "PREPARE") + ", ";
	temp += f.Link("#reveal", "REVEAL") + ", ";
	temp += f.Link("#show", "SHOW") + ", ";
	if(!Globals->USE_WEAPON_ARMOR_COMMAND)
		temp += "and ";
	temp += f.Link("#spoils", "SPOILS");
	if(Globals->USE_WEAPON_ARMOR_COMMAND) {
		temp += ", and ";
		temp += f.Link("#weapon", "WEAPON");
	}
	temp += " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#find", "FIND") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#leave", "LEAVE") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#enter", "ENTER") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#promote", "PROMOTE") + " orders are processed.";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr("Combat is processed.");
	f.Enclose(1, "UL");
	temp = f.Link("#attack", "ATTACK") + " orders are processed.";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	if (has_stea) {
		f.Enclose(1, "LI");
		f.PutStr("Steal orders.");
		f.Enclose(1, "UL");
		temp = f.Link("#steal", "STEAL") + " and ";
		temp += f.Link("#assassinate", "ASSASSINATE") +
			" orders are processed.";
		f.TagText("LI", temp);
		f.Enclose(0, "UL");
		f.Enclose(0, "LI");
	}
	f.Enclose(1, "LI");
	f.PutStr("Give orders.");
	f.Enclose(1, "UL");
	temp = f.Link("#destroy", "DESTROY") + " and ";
	temp += f.Link("#give", "GIVE") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#exchange", "EXCHANGE") + " orders are processed.";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr("Tax orders.");
	f.Enclose(1, "UL");
	temp = f.Link("#pillage","PILLAGE") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#tax","TAX") + " orders are processed.";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr("Instant Magic");
	f.Enclose(1, "UL");
	f.TagText("LI", "Old spells are cancelled.");
	temp = "Spells are ";
	temp += f.Link("#cast", "CAST");
	temp += " (except for Teleportation spells).";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr("Market orders.");
	f.Enclose(1, "UL");
	temp = f.Link("#guard","GUARD") + " 1 orders are processed.";
	f.TagText("LI", temp);
	if(Globals->TOWNS_EXIST) {
		temp = f.Link("#sell","SELL") + " orders are processed.";
		f.TagText("LI", temp);
	}
	temp = f.Link("#buy","BUY") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#quit","QUIT") + " and ";
	temp += f.Link("#restart", "RESTART") + " orders are processed.";
	f.TagText("LI", temp);
	temp = f.Link("#forget","FORGET") + " orders are processed.";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	if (Globals->ALLOW_WITHDRAW) {
		f.Enclose(1, "LI");
		f.PutStr("Withdraw orders.");
		f.Enclose(1, "UL");
		temp = f.Link("#withdraw","WITHDRAW") + " orders are processed.";
		f.TagText("LI", temp);
		f.Enclose(0, "UL");
		f.Enclose(0, "LI");
	}
	f.Enclose(1, "LI");
	f.PutStr("Movement orders.");
	f.Enclose(1, "UL");
	if(may_sail) {
		temp = f.Link("#sail","SAIL") + " orders are processed.";
		f.TagText("LI", temp);
	}
	temp = f.Link("#advance","ADVANCE") + " and ";
	temp += f.Link("#move", "MOVE") + " orders are processed (including any "
		"combat resulting from these orders).";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr("Month long orders.");
	f.Enclose(1, "UL");
	temp = f.Link("#build", "BUILD") + ", ";
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		temp += f.Link("#entertain", "ENTERTAIN") + ", ";
	temp += f.Link("#produce", "PRODUCE") + ", ";
	temp += f.Link("#study", "STUDY") + ", ";
	temp += f.Link("#teach", "TEACH") + ", and ";
	temp += f.Link("#work", "WORK") + " orders are processed.";
	f.TagText("LI", temp);
	temp = "Costs associated with these orders (such as study fees) are "
		"collected.";
	f.TagText("LI", temp);
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	temp = "Teleportation spells are ";
	temp += f.Link("#cast", "CAST") + ".";
	f.TagText("LI", temp);
	f.TagText("LI", "Maintenance costs are assessed.");
	f.Enclose(0, "OL");
	temp = "Where there is no other basis for deciding in which order units "
		"will be processed within a phase, units that appear higher on the "
		"report get precedence.";
	f.Paragraph(temp);
	f.LinkRef("reportformat");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Report Format");
	temp = "The most important sections of the turn report are the \"Events "
		"During Turn\" section which lists what happened last month, and "
		"the \"Current Status\" section which gives the description of each "
		"region in which you have units.";
	f.Paragraph(temp);
	temp = "Your units in the Current Status section are flagged with a "
		"\"*\" character. Units belonging to other factions are flagged "
		"with a \"-\" character. You may be informed which faction they "
		"belong to, if ";
	if(has_obse)
		temp += "you have high enough Observation skill or ";
	temp += "they are revealing that information.";
	f.Paragraph(temp);
	temp = "Objects are flagged with a \"+\" character.  The units listed "
		"under an object (if any) are inside the object.  The first unit "
		"listed under an object is its owner.";
	f.Paragraph(temp);
	temp = "If you can see a unit, you can see any large items it is "
		"carrying.  This means all items other than silver";
	if(!(ItemDefs[I_HERBS].flags & ItemType::DISABLED))
		temp += ", herbs,";
	temp += " and other small items (which are of zero size units, and are "
		"small enough to be easily concealed). Items carried by your own "
		"units of course will always be listed.";
	f.Paragraph(temp);
	temp = "At the bottom of your turn report is an Orders Template.  This "
		"template gives you a formatted orders form, with all of your "
		"units listed. You may use this to fill in your orders, or write "
		"them on your own. The ";
	temp += f.Link("#option", "OPTION") + " order gives you the option of "
		"giving more or less information in this template, or turning it "
		"of altogether. You can precede orders with an '@' sign in your "
		"orders, in which case they will appear in your template on the "
		"next turn's report.";
	f.Paragraph(temp);
	f.LinkRef("hintsfornew");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Hints for New Players");
	temp = "Make sure to use the correct #ATLANTIS and UNIT lines in your "
		"orders.";
	f.Paragraph(temp);
	temp = "Always have a month's supply of spare cash in every region in "
		"which you have units, so that even if they are deprived of "
		"income for a month (due to a mistake in your orders, for "
		"example), they will not starve to death.  It is very frustrating "
		"to have half your faction wiped out because you neglected to "
		"provide enough money for them to live on.";
	f.Paragraph(temp);
	temp = "Be conservative with your money. ";
	if(Globals->LEADERS_EXIST) {
		temp += "Leaders especially are very hard to maintain, as they "
			"cannot usually earn enough by ";
		temp += f.Link("#work", "WORK") + "ing to pay their maintenance "
			"fee. ";
	}
	temp += "Even once you have recruited men, notice that it is "
		"expensive for them to ";
	temp += f.Link("#study", "STUDY") + " (and become productive units), "
		"so be sure to save money to that end.";
	f.Paragraph(temp);
	temp = "Don't leave it until the last minute to send orders.  If "
		"there is a delay in the mailer, your orders will not arrive "
		"on time, and turns will NOT be rerun, nor will it be possible "
		"to change the data file for the benefit of players whose orders "
		"weren't there by the deadline.  If you are going to send your "
		"orders at the last minute, send a preliminary set earlier in the "
		"week so that at worst your faction will not be left with no "
		"orders at all.";
	f.Paragraph(temp);

	if(Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f.LinkRef("specialcommands");
		f.ClassTagText("DIV", "rule", "");
		f.TagText("H2", "Special Commands");
		temp = "These special commands have been added via the scripts "
			"processing the email to help you interact with the game "
			"and submit times and rumors. Please read over these new "
			"commands and their uses. Also note that all commands sent "
			"to the server are logged, including orders submissions, so "
			"if you have a problem, or if you attempt to abuse the system, "
			"it will get noticed and it will be tracked down.";
		f.Paragraph(temp);
		f.LinkRef("_create");
		f.ClassTagText("DIV", "rule", "");
		f.TagText("H4", "#create \"faction name\" \"password\"");
		temp = "This will create a new faction with the desired name and "
			"password, and it will use the player's \"from\" address as "
			"the email address of record (this, of course, can be changed "
			"from within the game).";
		f.Paragraph(temp);
		temp = "The \"\" characters are required. If they are missing, the "
			"server will not create the faction.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Join the game as the faction named \"Mighty Ducks\" with the "
			"password of \"quack\"";
		temp2="#create \"Mighty Ducks\" \"quack\"";
		f.CommandExample(temp, temp2);

		f.LinkRef("_resend");
		f.ClassTagText("DIV", "rule", "");
		f.TagText("H4", "#resend [faction] \"password\"");
		temp = "The faction number and your current password (if you have "
			"one) are required. The most recent turn report will be sent to "
			"the address of record.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "You are faction 999 with password \"quack\" and need another "
			"copy of the last turn (because your harddrive crashed)";
		temp2 = "#resend 999 \"quack\"";
		f.CommandExample(temp, temp2);

		f.LinkRef("_times");
		f.ClassTagText("DIV", "rule", "");
		f.TagText("H4", "#times [faction] \"password\"");
		f.PutStr("[body of article]");
		f.TagText("H4", "#end");
		temp = "Everything between the #times and #end lines is included "
			"in your article. Your article will be marked as being "
			"sent by your fation, so you need not include that "
			"attribution in the article.";
		if (Globals->TIMES_REWARD) {
			temp += " You will receive $";
			temp += Globals->TIMES_REWARD;
			temp += " for submitting the article.";
		}
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Faction 999 wants to declare war on everyone";
		temp2 = "#times 999 \"quack\"\n";
		temp2 += "The Mighty Ducks declare war on the world!!\n";
		temp2 += "Quack!\n";
		temp2 += "#end";
		f.CommandExample(temp, temp2);
		temp = "And it would appear something like:";
		temp2 = "---------------------------------\n";
		temp2 += "The Mighty Ducks declare war on the world!!\n";
		temp2 += "Quack!\n\n";
		temp2 += "[Article submitted by The Mighty Ducks (999)]\n";
		temp2 += "---------------------------------";
		f.CommandExample(temp, temp2);

		f.LinkRef("_rumor");
		f.ClassTagText("DIV", "rule", "");
		f.TagText("H4", "#rumor [faction] \"password\"");
		f.PutStr("[body of rumor]");
		f.TagText("H4", "#end");
		temp = "Submit a rumor for publication in the next news.  These "
			"articles are not attributed (unlike times articles) and will "
			"appear in the rumor section of the next news in a random order.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Start a malicious rumor";
		temp2 = "#rumor 999 \"quack\"\n";
		temp2 += "Oleg is a running-dog lackey of Azthar Spleenmonger.\n";
		temp2 += "#end";
		f.CommandExample(temp, temp2);

		f.LinkRef("_remind");
		f.ClassTagText("DIV", "rule", "");
		f.TagText("H4", "#remind [faction] \"password\"");
		temp = "This order will have the server find the most recent set of "
			"orders you have submitted for the current turn and mail them "
			"back to your address of record.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Remind faction 999 of its last order set.";
		temp2 = "#remind 999 \"quack\"";
		f.CommandExample(temp, temp2);

		f.LinkRef("_email");
		f.ClassTagText("DIV", "rule", "");
		f.TagText("H4", "#email [unit]");
		f.PutStr("[text of email]");
		temp = "This command allows you to send email to the owner of a unit "
			"even when you cannot see that unit's faction affiliation.  You "
			"will not be told who the unit belongs to, but will simply "
			"forward your email to them. When you use this command, they "
			"will recieve YOUR email and can contact you if they choose. It "
			"is provided simply as a courtesy to players to help with "
			"diplomacy in first contact situations.";
		f.Paragraph(temp);
		temp = "There is no need for a \"#end\" line (such as is used in "
			"times and rumor submissions -- the entire email message you "
			"send will be forwarded to the unit's master.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Send an email to the owner of unit 9999";
		temp2 = "#email 9999\n";
		temp2 += "Greetings.  You've entered the Kingdom of Foo.\n";
		temp2 += "Please contact us.\n\n";
		temp2 += "Lord Foo\n";
		temp2 += "foo@some.email";
		f.CommandExample(temp, temp2);
		temp = "Faction X, the owner of 9999 would receive:";
		temp2 = "From: Foo &lt;foo@some.email&gt;\n";
		temp2 += "Subject:  Greetings!\n\n";
		temp2 += "#email 9999\n";
		temp2 += "Greetings.  You've entered the Kingdom of Foo.\n";
		temp2 += "Please contact us.\n\n";
		temp2 += "Lord Foo\n";
		temp2 += "foo@some.email";
		f.CommandExample(temp, temp2);
	}
	f.LinkRef("credits");
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Credits");
	temp = "Atlantis was originally created and programmed by Russell "
		"Wallace. Russell Wallace created Atlantis 1.0, and partially "
		"designed Atlantis 2.0 and Atlantis 3.0.";
	f.Paragraph(temp);
	temp = "Geoff Dunbar designed and programmed Atlantis 2.0, 3.0, and 4.0 "
		"up through version 4.0.4 and created the Atlantis Project to "
		"freely release and maintain the Atlantis source code.";
	f.Paragraph(temp);
	temp = "Larry Stanbery created the Atlantis 4.0.4+ derivative.";
	f.Paragraph(temp);
	temp = f.Link("mailto:jtraub@dragoncat.net", "JT Traub");
	temp += " took over the source code and merged the then forking versions "
		"of 4.0.4c and 4.0.4+ back into 4.0.5 along with modifications of his "
		"own and has been maintaining the code.";
	f.Paragraph(temp);
	temp = "Development of the code is open and there is a egroup devoted to "
		"it located at ";
	temp += f.Link("http://groups.yahoo.com/group/atlantisdev",
			"The YahooGroups AtlantisDev egroup");
	temp += ". Please join this egroup if you work on the code and share your "
		"changes back into the codebase as a whole";
	f.Paragraph(temp);
	temp = "Please see the CREDITS file in the source distribution for a "
		"complete (hopefully) list of all contributers.";
	f.Paragraph(temp);
	f.Enclose(0, "BODY");
	f.Enclose(0, "HTML");
	return 1;
}
