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
	AString temp, temp2;
	int cap;
	int i, j;

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
	f.PutStr("Note: This document is subject to change, as errors are found");
	f.PutStr("and corrected, and rules sometimes change. Be sure you have");
	f.PutStr("the latest available copy.");
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("table_of_contents"));
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Table of Contents");
	f.PutStr(AString("Thanks to ") +
			f.Link("mailto:ken@satori.gso.uri.edu","Kenneth Casey"));
	f.PutStr("for putting together this table of contents.");
	f.PutStr("<P></P>");
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
	if(Globals->NEXUS_EXISTS)
		f.TagText("LI", f.Link("#world_nexus", "The Nexus"));
	if(Globals->CONQUEST_GAME)
		f.TagText("LI", f.Link("#world_conquest", "Conquest"));
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
	f.TagText("LI", f.Link("#noplayers_controlled", "Controlled Monsters"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#orders", "Orders"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#orders_appreviations", "Abbreviations"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#ordersummary", "Order Summary"));
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
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
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
	f.Enclose(0, "LI");
	f.TagText("LI", f.Link("#sequenceofevents", "Sequence of Events"));
	f.TagText("LI", f.Link("#reportformat", "Report Format"));
	f.TagText("LI", f.Link("#hintsfornew", "Hints for New Players"));
	f.Enclose(1, "LI");
	f.PutStr(f.Link("#specialcommands", "Special Commands"));
	f.Enclose(1, "UL");
	f.TagText("LI", f.Link("#create", "#Create"));
	f.TagText("LI", f.Link("#resend", "#Resend"));
	f.TagText("LI", f.Link("#times", "#Times"));
	f.TagText("LI", f.Link("#rumors", "#Rumors"));
	f.TagText("LI", f.Link("#remind", "#Remind"));
	f.TagText("LI", f.Link("#email", "#Email"));
	f.Enclose(0, "UL");
	f.Enclose(0, "LI");
	f.TagText("LI", f.Link("#credits", "Credits"));
	f.Enclose(0, "UL");
	f.PutStr("<BR>");
	f.PutStr("Index of Tables");
	f.PutStr("<P></P>");
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
	f.PutStr("<P></P>");
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
	f.PutStr("<P></P>");
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
	f.PutStr("<P></P>");
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
	f.PutStr("<P></P>");
	Faction fac;
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		temp = "A faction has one pre-set limit; it may not contain more than ";
		temp += AString(AllowedMages(&fac)) + "mages";
		if(Globals->APPRENTICES_EXIST) {
			temp += AString("and ") + AllowedApprentices(&fac) + " apprentices";
		}
		temp += ". Magic is a rare art, and only a few in the world can "
			"master it. Aside from that, there  is no limit to the number "
			"of units a faction may contain, nor to how many items can be "
			"produced or regions taxed.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
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
			"goods, building ships and buildings, and buying and selling "
			"trade items. Faction Points spent on Magic determines the "
			"number of mages ";
		if(Globals->APPRENTICES_EXIST)
			temp += "and apprentices ";
		temp += "the faction may have. (More information on all of the "
			"faction activities is in further sections of the rules).  Here "
			"is a chart detailing the limits on factions by Faction Points.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		f.PutStr(f.LinkRef("tablefactionpoints"));
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
		for(i = 1; i <= Globals->FACTION_POINTS; i++) {
			fac.type[F_WAR]=i;
			fac.type[F_TRADE]=i;
			fac.type[F_MAGIC]=i;
			f.Enclose(1, "TR ALIGN=CENTER");
			f.TagText("TD", AString(i));
			f.TagText("TD", AString(AllowedTaxes(&fac)));
			f.TagText("TD", AString(AllowedTrades(&fac)));
			temp = AllowedMages(&fac);
			if(Globals->APPRENTICES_EXIST)
				temp += AString("/") + AllowedApprentices(&fac);
			f.TagText("TD", temp);
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
		f.PutStr(temp);
		f.PutStr("<P></P>");

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
		f.PutStr(temp);
		f.PutStr("<P></P>");
		if (Globals->FACTION_POINTS>3) {
			int rem=Globals->FACTION_POINTS-3;
			f.PutStr(AString("Note that it is possible to have a faction ") +
					"type with less than " + Globals->FACTION_POINTS +
					" points spent. In fact, a starting faction has one "
					"point spent on each of War, Trade, and Magic, leaving " +
					rem + " point" + (rem==1?"":"s") + " unspent.");
			f.PutStr("<P></P>");
		}
	}
	temp = "When a faction starts the game, it is given a one-man unit and ";
	temp += (Globals->START_MONEY -
			(Globals->LEADERS_EXIST ?
			 Globals->LEADER_COST : Globals->MAINTENANCE_COST));
	temp += " silver unclaimed money.  Unclaimed money is cash that your "
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr("An example faction is shown below, consisting of a "
			"starting character, Merlin the Magician, who has formed "
			"two more units, Merlin's Guards and Merlin's Workers.  "
			"Each unit is assigned a unit number by the computer "
			"(completely independent of the faction number); this is used "
			"for entering orders.  Here, the player has chosen to give "
			"his faction the same name (\"Merlin the Magician\") as his "
			"starting character.  Alternatively, you can call your "
			"faction something like \"The Great Northern Mining Company\" "
			"or whatever.");
	f.PutStr("<P></P>");
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
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("playing_units"));
	f.TagText("H3", "Units:");
	f.PutStr("A unit is a grouping together of people, all loyal to the "
			"same faction. The people in a unit share skills and "
			"possessions, and execute the same orders each month. The "
			"reason for having units of many people, rather than keeping "
			"track of individuals, is to simplify the game play.  The "
			"computer does not keep track of individual names, possessions, "
			"or skills for people in the same unit, and all the people in a "
			"particular unit must be in the same place at all times.  If "
			"you want to send people in the same unit to different places, "
			"you must split up the unit.  Apart from this, there is no "
			"difference between having one unit of 50 people, or 50 units of "
			"one person each, except that the former is very much easier "
			"to handle.");
	f.PutStr("<P></P>");
	temp = "";
	if(Globals->RACES_EXIST) {
		temp = AString("There are different races that make up the "
				"population of ") + Globals->WORLD_NAME + ". (See the "
				"section on skills for a list of these.)";
		if(Globals->LEADERS_EXIST) {
			temp += " In addition, there are \"leaders\" who are presumed "
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("playing_turns"));
	f.TagText("H3", "Turns:");
	f.PutStr("<P></P>");
	f.PutStr("Each turn, the Atlantis server takes the orders file that "
			"you mailed to it, and assigns the orders to the respective "
			"units.  All units in your faction are completely loyal to you, "
			"and will execute the orders to the best of their ability.  If "
			"the unit does something unintended, it is generally because of "
			"incorrect orders; a unit will not purposefully betray you.");
	f.PutStr("<P></P>");
	f.PutStr("A turn is equal to one game month.  A unit can do many "
			"actions at the start of the month, that only take a matter of "
			"hours, such as buying and selling commodities, or fighting an "
			"opposing faction.  Each unit can also do exactly one action "
			"that takes up the entire month, such as harvesting resources or "
			"moving from one region to another.  The orders which take an "
			"entire month are");
	f.PutStr(f.Link("#advance", "ADVANCE") + ", ");
	f.PutStr(f.Link("#build", "BUILD") + ", ");
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f.PutStr(f.Link("#entertain", "ENTERTAIN") + ", ");
	f.PutStr(f.Link("#move", "MOVE") + ", ");
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		f.PutStr(f.Link("#pillage", "PILLAGE") + ", ");
	f.PutStr(f.Link("#produce", "PRODUCE") + ", ");
	if(!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f.PutStr(f.Link("#sail", "SAIL") + ", ");
	f.PutStr(f.Link("#study", "STUDY") + ", ");
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		f.PutStr(f.Link("#tax", "TAX") + ", ");
	f.PutStr(f.Link("#teach", "TEACH") + " and");
	f.PutStr(f.Link("#work", "WORK") + ".");
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("world"));
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("world_regions"));
	f.TagText("H3", "Regions:");
	f.PutStr("Here is a sample region, as it might appear on your turn "
			"report:");
	f.PutStr("<P></P>");
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
	f.PutStr("<P></P>");
	temp = "This report gives all of the available information on this "
		"region.  The region type is plain, the name of the surrounding area "
		"is Turia, and the coordinates of this region are (172,110).  The "
		"population of this region are 500 ";
	if(Globals->RACES_EXIST)
		temp += "nomads";
	else
		temp += "peasants";
	temp += AString(", and there is $") + money + " of taxable income ";
	temp += "current in this region.  Then under the dashed line, are "
		"various details about items for sale, wages, etc.  Finally, "
		"there is a list of all visible units.  Units that belong to your "
		"faction will be so denoted by a '*', whereas other faction's "
		"units are preceded by a '-'.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr("Since Atlantis is made up of hexagonal regions, the coordinate "
			"system is not always exactly intuitive.  Here is the layout of "
			"Atlantis regions:");
	f.PutStr("<P></P>");
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
	f.PutStr("Note that the are \"holes\" in the coordinate system; there "
			"is no region (1,2), for instance.  This is due to the "
			"hexagonal system of regions.");
	f.PutStr("<P></P>");
	temp = "Most regions are similar to the region shown above, but the "
		"are certain exceptions.  Oceans, not surprisingly, have no "
		"population.";
	if (Globals->TOWNS_EXIST)
		temp += " Some regions will contain villages, towns, and cities. "
			"More information on these is available in the section on the "
			"ecomony.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("world_structures"));
	f.TagText("H3", "Structures:");
	temp = "Regions may also contain structures, such as buildings";
	if(may_sail)
		temp += " or ships";
	temp += ". These will appear directly below the list of units.  Here is "
		"a sample structure:";
	f.PutStr(temp);
	f.PutStr("<P></P>");
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
	f.PutStr("<P></P>");
	temp = "The structure lists the name, the number, and what type of "
		"structure it is.  (More information of the types of structures "
		"can be found in the section on the economy.)  Following this "
		"is a list of units inside the structure.";
	if (has_stea)
		temp += " Units within a structure are always visible, even if "
			"they would otherwise not be seen.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr("Units inside structures are still considered to be in the "
			"region, and other units can interact with them; however, they "
			"may gain benefits, such as defensive bonuses in combat from "
			"being inside a building.  The first unit to enter an object is "
			"considered to be the owner; only this unit can do things such "
			"as renaming the object, or permitting other units to enter. "
			"The owner of an object can be identified on the turn report, as "
			"it is the first unit listed under the object.  Only units with "
			"men in them can be structure owners, so newly created units "
			"cannot own a structure until they contain men.");
	f.PutStr("<P></P>");
	if(Globals->NEXUS_EXISTS) {
		f.PutStr(f.LinkRef("world_nexus"));
		f.TagText("H3", "The Nexus:");
		f.PutStr("Note: the following section contains some details that "
				"you may wish to skip over until you have had a chance to "
				"read the rest of the rules, and understand the mechanics "
				"of Atlantis.  However, be sure to read this section before "
				"playing, as it will affect your early plans in Atlantis.");
		f.PutStr("<P></P>");
		temp = "When a faction first starts in Atlantis, it begins with "
			"one unit, in a special region called the Nexus.";
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
		f.PutStr(temp);
		f.PutStr("<P></P>");
		if(Globals->MULTI_HEX_NEXUS) {
			temp = "From the Nexus hexes, there are exits either to other "
				"Nexus hexes, or to starting cities in Atlantis.  Units may "
				"move through these exits as normal, but once in a starting "
				"city, there is no way to regain entry to the Nexus.";
		} else {
			temp = "From the Nexus, there are six exits into the starting "
				"cities of Atlantis.  Units may move through these exits "
				"as normal, but once through an exit, there is no return "
				"path to the Nexus.";
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
		if (!Globals->SAFE_START_CITIES)
			temp += "until someone conquers the guardsmen, ";
		temp += "there are unlimited amounts of many materials and men "
			"(though the prices are often quite high).";
		temp += " In addition, ";
		if(Globals->SAFE_START_CITIES)
			temp += "no battles are allowed in starting cities.";
		else
			temp += "the starting cities are guarded by strong guardsmen, "
				"keeping any units within the city much safer from attack "
				"(See the section on Non-Player Units for more information "
				"on city guardsmen).";
		temp += " As a drawback, these cities tend to be extremely crowded, "
			"and most factions will wish to leave the starting cities when "
			"possible.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		temp = "It is always possible to enter any starting city from the "
			"nexus";
		if(!Globals->SAFE_START_CITIES)
			temp += ", even if that starting city has been taken over and "
				"guarded by another faction";
		temp += ". This is due to the transportation from the Nexus to the "
			"starting city being magical in nature.";
		if(!Globals->SAFE_START_CITIES)
			temp += " Once in the start city however, no gaurentee of "
				"safety is given.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		int num_methods = 1 + (Globals->GATES_EXIST?1:0) + (may_sail?1:0);
		char *methods[] = {"You must go ", "The first is ", "The second is "};
		int method = 1;
		if (num_methods == 1) method = 0;
		temp = AString("There ") + (num_methods == 1?"is ":"are ") +
			num_methods + " method" + (num_methods == 1?" ":"s ") +
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
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	if(Globals->CONQUEST_GAME) {
		f.PutStr(f.LinkRef("world_conquest"));
		f.TagText("H3", "The World of Atlantis Conquest");
		f.PutStr("In a game of Atlantis Conquest, each player begins the "
				"game on a small island of 8 regions, seperated by ocean "
				"from the rest of the players.  The starting islands are "
				"located around the perimeter of a larger central island. "
				"Sailing from the starting islands towards the center of "
				"the map should lead to the central island within a few "
				"regions.");
		f.PutStr("<P></P>");
	}

	f.PutStr(f.LinkRef("movement"));
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
	f.PutStr(temp);
	f.PutStr("<P></P>");

	f.PutStr(f.LinkRef("movement_normal"));
	f.TagText("H3", "Normal Movement:");
	temp = "In one month, a unit can issue a single ";
	temp += f.Link("#move", "MOVE") + " order, using one or more of its "
		"movement points. There are three modes of travel: walking, riding "
		"and flying. Walking units have ";
	temp += AString(Globals->FOOT_SPEED) + " movement point" +
		(Globals->FOOT_SPEED==1?"":"s") + ", riding units have ";
	temp += AString(Globals->HORSE_SPEED) + " movement point" +
		(Globals->HORSE_SPEED==1?"":"s") + ", and flying units have ";
	temp += AString(Globals->FLY_SPEED) + " movement point" +
		(Globals->FLY_SPEED==1?"":"s") + ".";
	temp += " A unit will automatically use the fastest mode of travel "
		"it has available. The ";
	temp += f.Link("#advance", "ADVANCE") + " order is the same as " +
		f.Link("#move", "MOVE") + " except that it implies attacks on " +
		"units which try to forbid access; see the section on combat for " +
		"details.";
	f.PutStr(temp);
	f.PutStr("<P></P>");

	temp = "Flying units are not initially available to starting players. "
		"A unit can ride provided that the carrying capacity of its "
		"horses is at least as great as the weight of its people and "
		"all other items.  A unit can walk provided that the carrying "
		"capacity of its people, horses and wagons is at least as great "
		"as the weight of all its other items, and provided that it has "
		"at least as many horses as wagons (otherwise the excess wagons "
		"count as weight, not capacity). Otherwise the unit cannot issue "
		"a ";
	temp += f.Link("#move", "MOVE") + " order.";
	temp += " Most people weigh 10 units and have a capacity of 5 units; "
		"data for items is as follows:";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("tableitemweights"));
	f.Enclose(1, "CENTER");
	f.Enclose(1, "TABLE BORDER=1");
	f.Enclose(1, "TR ALIGN=CENTER");
	f.TagText("TH", "Item");
	f.TagText("TH", "Weight");
	f.TagText("TH", "Capacity");
	f.Enclose(0, "TR");
	for(i = 0; i < NITEMS; i++) {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(!(ItemDefs[i].type & IT_NORMAL)) continue;
		f.Enclose(1, "TR ALIGN=CENTER");
		f.TagText("TD", ItemDefs[i].name);
		f.TagText("TD", ItemDefs[i].weight);
		cap = ItemDefs[i].walk - ItemDefs[i].weight;
		if(ItemDefs[i].walk || (ItemDefs[i].hitchItem != -1)) {
			if(ItemDefs[i].hitchItem == -1)
				f.TagText("TD", cap);
			else {
				temp = (cap + ItemDefs[i].hitchwalk);
				temp += " (with ";
				temp += ItemDefs[ItemDefs[i].hitchItem].name;
				temp += ")";
				f.TagText("TD", temp);
			}
		} else {
			f.TagText("TD", "&nbsp;");
		}
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Enclose(0, "CENTER");
	f.PutStr("<P></P>");
	if(Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) {
		temp = "A unit which can fly, is capable of travelling over water.";
		if(Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_MUST_LAND)
			temp += " However, if the unit ends its turn over a water hex "
				"that unit will drown.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
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
		temp += ", Jungle, Desert, or Tundra";
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Units may also enter or exit structures while moving.  Moving "
		"into or out of a structure does not use any movement points at "
		"all.  Note that a unit can also use the ";
	temp += f.Link("#enter", "ENTER") + " and " + f.Link("#leave", "LEAVE");
	temp += " orders to move in and out of structures, without issuing a ";
	temp += f.Link("#move", "MOVE") + " order.";
	temp += " The unit can also use the ";
	temp += f.Link("#move", "MOVE") + " order to enter or leave a structure.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Finally, certain structures contain interior passages to other "
		"regions.  The ";
	temp += f.Link("#move", "MOVE") + " IN order can be used to go through ";
	temp += "these passages; the movement point cost is equal to the normal "
		"cost to enter the destination region.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Example: One man with a horse, sword, and chain mail wants to "
		"move north, then northeast.  The capacity of the horse is ";
	cap = ItemDefs[I_HORSE].ride - ItemDefs[I_HORSE].weight;
	temp += cap;
	temp += " and the weight of the man and other items is ";
	int weight = ItemDefs[I_MAN].weight + ItemDefs[I_SWORD].weight + 
			ItemDefs[I_CHAINARMOR].weight;
	temp += weight;
	if(cap > weight)
		temp += ", so he can ride.";
	else
		temp += ", so he must walk.";
	temp += " The month is April, so he has ";
	temp += Globals->HORSE_SPEED;
	int travel = Globals->HORSE_SPEED;
	temp += " movement points.  He issues the order MOVE NORTH NORTHEAST.";
	temp += " First he moves north, into a plain region.  This uses ";
	int cost = TerrainDefs[R_PLAIN].movepoints;
	temp += AString(cost) + " movement point" + (cost == 1?"":"s") + ".";
	travel -= cost;
	if(travel > TerrainDefs[R_FOREST].movepoints) {
		temp += " Then he moves northeast, into a forest region. This uses ";
		cost = TerrainDefs[R_FOREST].movepoints;
		temp += AString(cost) + " movement point" + (cost == 1?"":"s") + ",";
		travel -= cost;
		temp += " so the movement is completed with ";
		temp += AString(travel) + " movement point" + (cost == 1?"":"s");
		temp += " to spare.";
	} else {
		temp += " He does not have the ";
		cost = TerrainDefs[R_FOREST].movepoints;
		temp += AString(cost) + " movement point" + (cost == 1?"":"s");
		temp += " needed to move into the forest region to the northeast, "
			"so the movement is halted at this point.";
	}
	f.PutStr(temp);
	f.PutStr("<P></P>");

	if(may_sail) {
		f.PutStr(f.LinkRef("movement_sailing"));
		f.TagText("H3", "Sailing:");
		temp = "Movement by sea is in some ways similar. It does not use the ";
		temp += f.Link("#move", "MOVE") + " order however.  Instead the " +
			"owner of the ship must issue the " + f.Link("#sail", "SAIL") +
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
		f.PutStr(temp);
		f.PutStr("<P></P>");
		temp = AString("Note that a unit on board a sailing ship may not ") +
			f.Link("#move", "MOVE") + " later in the turn, even if he " +
			"doesn't issue the " + f.Link("#sail", "SAIL");
		temp += " order; sailing is considered to take the whole month. "
			"Also, units may not remain on guard while on board a sailing "
			"ship; they will have to reissue the ";
		temp += f.Link("#guard", "GUARD") +  " 1 order to guard a " +
			"region after sailing.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		temp = AString("Ships get ") + Globals->SHIP_SPEED;
		temp += AString(" movement point") + (Globals->SHIP_SPEED==1?"":"s");
		temp += " per turn.  A ship can move from an ocean region to "
			"another ocean region, or from a coastal region to an ocean "
			"region, or from an ocean region to a coastal region. Ships "
			"can only be constructed in coastal regions.  For a ship to "
			"enter any region only costs one movement point; the cost of "
			"two movement points for entering, say, a forest coastal "
			"region, does not apply.";
		if(Globals->WEATHER_EXISTS) {
			temp += " Ships do, however, only get half movement points "
				"during the winter months (or monsoon months in the "
				"tropical latitudes).";
		}
		f.PutStr(temp);
		f.PutStr("<P></P>");
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
		f.PutStr(temp);
		f.PutStr("<P></P>");
		f.PutStr(f.LinkRef("tableshipcapacities"));
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
		f.PutStr("<P></P>");
	}
	f.PutStr(f.LinkRef("movement_order"));
	f.TagText("H3", "Order of Movement:");
	f.PutStr("This section is probably unimportant to beginning players, "
			"but it can be helpful for more experienced players.");
	f.PutStr("<P></P>");
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	if(may_sail) {
		f.PutStr("Sailing is handled differently; Atlantis cycles through "
				"all of the ships in Atlantis, moving them one at a time. "
				"When Atlantis sails a ship, it sails it through it's "
				"entire course, either to the end, or until the ship enters "
				"a hex guarded against some unit on the ship, and then "
				"moves onto the next ship.");
		f.PutStr("<P></P>");
	}
	temp = "Note that";
	if(may_sail) {
		temp += ", in either case,";
	}
	temp += " the order in which the regions are processed is undefined "
		"by the rules. The computer generally does them in the same "
		"order every time, but it is up to the wiles of the player to "
		"determine (or not) these patterns. The order in which units or "
		"ships are moved within a region is the order that they appear "
		"on a turn report.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("skills"));
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Skills");

	temp = "The most important thing distinguishing one character from "
		"another in Atlantis is skills.  The following skills are "
		"available:";
	int comma = 0;
	for(i = 0; i < NSKILLS; i++) {
		if(SkillDefs[i].flags & SkillType::DISABLED) continue;
		if(SkillDefs[i].flags & SkillType::APPRENTICE) continue;
		if(SkillDefs[i].flags & SkillType::MAGIC) continue;
		if(comma) temp += ", ";
		temp += SkillDefs[i].name;
		comma = 1;
	}
	temp += ". When a unit possesses a skill, he also has a skill level "
		"to go with it.  Generally, the effectiveness of a skill is "
		"directly proportional to the skill level involved, so a level "
		"2 is twice as good as a level 1 in the same skill.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("skills_limitations"));
	f.TagText("H3", "Limitations:");
	if (Globals->LEADERS_EXIST&&Globals->SKILL_LIMIT_NONLEADERS) {
		temp = "A unit made up of leaders may know one or more skills; "
			"for the rest of this section, the word \"leader\" will refer "
			"to such a unit.  Other units, those which contain "
			"non-leaders, will be refered to as normal units. A normal "
			"unit may only know one skill.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	if (Globals->RACES_EXIST) {
		temp = "Skills may be learned up to a maximum level depending on "
			"the race of the studying unit (remembering that for units "
			"containing more than one race, the maximum is determined by "
			"the least common denominator).  Every race has a normal "
			"maximum skill level, and  a list of skills that they "
			"specialize in, and can learn up to higher level.";
		if(Globals->LEADERS_EXIST) {
			temp += " Leaders, being more powerful, can learn skills to "
				"even higher levels.";
		}
		temp += "Here is a list of the races (including leaders) and the "
			"information on normal skill levels and specialized skills.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		f.PutStr(f.LinkRef("tableraces"));
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TH", "Race/Type");
		f.TagText("TH", "Specilized Skills");
		f.TagText("TH", "Specialized Level");
		f.TagText("TH", "Max Level (non-specialized)");
		f.Enclose(0, "TR");
		for(i = 0; i < NITEMS; i++) {
			if(ItemDefs[i].flags & ItemType::DISABLED) continue;
			if(!(ItemDefs[i].type & IT_MAN)) continue;
			f.Enclose(1, "TR");
			int m = ItemDefs[i].index;
			f.Enclose(1, "TD ALIGN=CENTER");
			f.PutStr(ItemDefs[i].names);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD");
			int spec = 0;
			comma = 0;
			temp = "";
			for(j = 0; j < 4; j++) {
				if(ManDefs[m].skills[j] < 0) continue;
				if(SkillDefs[ManDefs[m].skills[j]].flags & SkillType::DISABLED)
					continue;
				spec = 1;
				if(comma) temp += ", ";
				temp += SkillDefs[ManDefs[m].skills[j]].name;
			}
			if(!spec) temp = "None.";
			f.PutStr(temp);
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER");
			if(spec)
				f.PutStr(ManDefs[m].speciallevel);
			else
				f.PutStr("&nbsp;");
			f.Enclose(0, "TD");
			f.Enclose(1, "TD ALIGN=CENTER");
			f.PutStr(ManDefs[m].defaultlevel);
			f.Enclose(0, "TD");
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
		f.PutStr("<P></P>");
	}
	f.PutStr("If units are merged together, their skills are averaged out. "
			"No rounding off is done; rather, the computer keeps track for "
			"each unit of how many total months of training that unit has "
			"in each skill.  When units are split up, these months are "
			"divided as evenly as possible among the people in the unit; "
			"but no months are ever lost.");
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("skills_studying"));
	f.TagText("H3", "Studying");
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
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
			temp += "both of ";
		temp += "which cost $";
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("skills_teaching"));
	f.TagText("H3", "Teaching:");
	temp = AString("A unit with a teacher can learn up to twice as fast ") +
		"as normal. The " + f.Link("#teach", "TEACH") + " order is used to ";
	temp += "spend the month teaching one or more other units (your own or "
		"another faction's).  The unit doing the teaching must have a skill "
		"level greater than the units doing the studying.  (Note: for all "
		"skill uses, it is skill level, not number of months of training, "
		"that counts. Thus, a unit with 1 month of training is effectively "
		"the same as a unit with 2 months of training, since both have a "
		"skill level of 1.)  The units being taught simply issue the ";
	temp += f.Link("#study", "STUDY") + " order normally (also, his faction "
		"must be declared Friendly by the teaching faction).  Each person "
		"can only teach up to " + Globals->STUDENTS_PER_TEACHER +
		"student" + (Globals->STUDENTS_PER_TEACHER == 1?"":"s") + " in a ";
	temp += "month; additional students dilute the training.  Thus, if 1 "
		"teacher teaches ";
	temp += 2*Globals->STUDENTS_PER_TEACHER + " men, each man will gain 1 "
		"1/2 months of training, not 2 months.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Note that it is quite possible for a single unit to teach two "
		"or more other units different skills in the same month, provided "
		"that the teacher has a higher skill level than each student in "
		"the skill that that student is studying, and that there are no "
		"more than ";
	temp += Globals->STUDENTS_PER_TEACHER + "student";
	temp += (Globals->STUDENTS_PER_TEACHER == 1 ? "" : "s");
	temp += " per teacher.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	if(Globals->LEADERS_EXIST) {
		temp = "Note: Only leader may use the ";
		temp += f.Link("#teach", "TEACH") + "order.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	f.PutStr(f.LinkRef("skills_skillreports"));
	f.TagText("H3", "Skill Reports:");
	temp = "When a faction learns a new skill level for this first time, it "
		"will be given a report on special abilities that a unit with this "
		"skill level has. This report can be shown again at any time (once "
		"a faction knows the skill), using the ";
	temp += f.Link("#show", "SHOW") + " order. For example, when a faction ";
	temp += "learned the skill Shoemaking level 3 for the first time, it "
		"might receive the following (obviously farsical) report:";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.Enclose(1, "PRE");
	f.ClearWrapTab();
	f.WrapStr("Shoemaking [SHOE] 3: A unit with this skill may PRODUCE "
			"Sooper Dooper Air Max Winged Sandals.");
	f.Enclose(0, "PRE");
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("economy"));
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "The Economy");
	f.PutStr("The unit of currency in Atlantis is the silver piece. Silver "
			"is a normal item, with zero weight, appearing in your unit's "
			"reports. Silver is used for such things as buying items, and "
			"unit's maintenance.");
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("economy_maintenance"));
	f.TagText("H3", "Maintenance Costs:");
	temp = "IMPORTANT:  Each and every character in Atlantis requires a "
		"maintenance fee each month. Anyone who ends the month without "
		"this maintenance cost has a ";
	temp += Globals->STARVE_PERCENT;
	temp += "percent chance of ";
	if(Globals->SKILL_STARVATION != GameDefs::STARVE_NONE) {
		temp += "starving, leading to the following effects:";
		f.PutStr(temp);
		f.Enclose(1, "UL");
		f.Enclose(1, "LI");
		if(Globals->SKILL_STARVATION == GameDefs::STARVE_MAGES)
			temp = "If the unit is a mage, it";
		else if(Globals->SKILL_STARVATION == GameDefs::STARVE_LEADERS)
			temp = "If the unit is a leader, it";
		else
			temp = "A unit";
		temp += " will loose a skill level in some of its skills.";
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
		temp += " starving to death.  ";
	}
	temp += "It is up to you to make sure that your people have enough money "
		"available . Money will be shared automatically between your units "
		"in the same region, if one is starving and another has more than "
		"enough; but this will not happen between units in different "
		"regions (this sharing of money applies only for maintenance costs, "
		"and does not occur for other purposes). If you have silver in your "
		"unclaimed fund, then that silver will be automatically claimed by "
		"units that would otherwise starve. Lastly, if a faction is allied "
		"to yours, their units will provide surplus cash to your units for"
		"maintenance, as a last resort.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = AString("This fee is generally ") + Globals->MAINTENANCE_COST +
		" silver for a normal character";
	if (Globals->LEADERS_EXIST) {
		temp += AString(", and ") + Globals->LEADER_COST +
			" silver for a leader";
	}
	temp += ".";
	if (Globals->FOOD_ITEMS_EXIST) {
		temp += " If this is not available, units may substitute one unit "
			"of grain, livestock, or fish for this maintenance";
		if(Globals->LEADERS_EXIST)
			temp += " (two units for a leader)";
		temp += ". A unit may use the ";
		temp += f.Link("#consume", "CONSUME") + " order to specify that it "
			" wishes to use food items in preference to silver.  Note that ";
		temp += "these items are worth more when sold in towns, so selling "
			"them and using the money is more economical than using them for "
			"maintenance.";
	};
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("economy_recruiting"));
	f.TagText("H3", "Recruiting");
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("economy_items"));
	f.TagText("H3", "Items:");
	f.PutStr("A unit may have a number of possessions, referred to as "
			"\"items\".  Some details were given above in the section on "
			"Movement, but many things were left out. Here is a table giving "
			"some information about common items in Atlantis:");
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("tableiteminfo"));
	f.Enclose(1, "CENTER");
	f.Enclose(1, "TABLE BORDER=1");
	f.Enclose(1, "TR");
	f.TagText("TD", "&nbsp;");
	f.TagText("TH", "Skill");
	f.TagText("TH", "Material");
	f.TagText("TH", "Production time");
	f.TagText("TH", "Weight (capacity)");
	f.TagText("TH", "Extra Information");
	f.Enclose(0, "TR");
	for(i = 0; i < NITEMS; i++) {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(SkillDefs[ItemDefs[i].pSkill].flags & SkillType::DISABLED) continue;
		if(!(ItemDefs[i].type & IT_NORMAL)) continue;
		f.Enclose(1, "TR");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ItemDefs[i].name);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		temp = SkillDefs[ItemDefs[i].pSkill].name;
		if(ItemDefs[i].pLevel > 1)
			temp += AString("(") + ItemDefs[i].pLevel + ")";
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		comma = 0;
		temp = "";
		for(j = 0; j < 2; j++) {
			if(ItemDefs[i].pInput[j].item < 0 ||
			   (ItemDefs[ItemDefs[i].pInput[j].item].flags&ItemType::DISABLED))
				continue;
			if(comma) temp += ", ";
			if(ItemDefs[i].pInput[j].amt > 1)
				temp += ItemDefs[ItemDefs[i].pInput[j].item].names;
			else
				temp += ItemDefs[ItemDefs[i].pInput[j].item].name;
		}
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		temp = ItemDefs[i].pMonths;
		temp += AString(" month") + (ItemDefs[i].pMonths == 1 ? "" : "s");
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		temp = ItemDefs[i].weight;
		if(ItemDefs[i].walk) {
			temp += AString("(") + (ItemDefs[i].walk - ItemDefs[i].weight) +
					")";
		}
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT");
		temp = "";
		if(ItemDefs[i].type & IT_WEAPON) {
			WeaponType *wp = &WeaponDefs[ItemDefs[i].index];
			temp += "This is a ";
			if(wp->flags & WeaponType::RANGED)
				temp += "ranged ";
			temp += "weapon";
			if(wp->flags & WeaponType::NEEDSKILL &&
					!(SkillDefs[wp->baseSkill].flags && SkillType::DISABLED)) {
				temp += "(needs ";
				temp += SkillDefs[wp->baseSkill].name;
				temp += " skill)";
			}
			temp += ".<BR>";
			if(wp->attackBonus || wp->defenseBonus) {
				temp += "It gives ";
				if(wp->attackBonus) {
					if(wp->attackBonus > 0) temp += "+";
					temp += wp->attackBonus;
					temp += " on attack";
				}
				if(wp->defenseBonus) {
					if(wp->attackBonus) temp += " and ";
					if(wp->defenseBonus > 0) temp += "+";
					temp += wp->defenseBonus;
					temp += " on defense";
				}
				temp += ".<BR>";
			}
			if(wp->numAttacks < 0) {
				temp += "It gives 1 attack every ";
				temp += -(wp->numAttacks);
				temp += " rounds.<BR>";
			}
		}
		if(ItemDefs[i].type & IT_MOUNT) {
			MountType *mp = &MountDefs[ItemDefs[i].index];
			if(mp->skill > 0 &&
					!(SkillDefs[mp->skill].flags & SkillType::DISABLED)) {
				temp += "It gives a riding bonus with the ";
				temp += SkillDefs[mp->skill].name;
				temp += "skill.<BR>";
			}
		}
		if(ItemDefs[i].type & IT_ARMOR) {
			ArmorType *at = &ArmorDefs[ItemDefs[i].index];
			temp += "It gives a ";
			temp += at->saves[SLASHING];
			temp += " in ";
			temp += at->from;
			temp += " chance to survive a normal hit.<BR>";
			if((at->flags & ArmorType::USEINASSASSINATE) && has_stea) {
				temp += "It may be used during assassinations.<BR>";
			}
		}
		if(ItemDefs[i].type & IT_TOOL) {
			for(j = 0; j < NITEMS; j++) {
				if(ItemDefs[j].flags & ItemType::DISABLED) continue;
				if(ItemDefs[j].mult_item != i) continue;
				if(!(ItemDefs[j].type & IT_NORMAL)) continue;
				temp += AString("+") + ItemDefs[j].mult_val +
					" bonus when producing " + ItemDefs[j].names + ".<BR>";
			}
		}
		if(ItemDefs[i].hitchItem &&
				!(ItemDefs[ItemDefs[i].hitchItem].flags & ItemType::DISABLED)) {
			temp += "Gives ";
			temp += ItemDefs[i].hitchwalk;
			temp += " additional walking capacity when hitched to a ";
			temp += ItemDefs[ItemDefs[i].hitchItem].name;
			temp += ".<BR>";
		}
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Enclose(0, "CENTER");
	f.PutStr("<P></P>");
	temp = "All items except silver and trade goods are produced with the ";
	temp += f.Link("#produce", "PRODUCE") + " order.  Example: PRODUCE "
		"SWORDS will produce as many swords as possible during the month, "
		"provide that the unit has an adequate supply of the raw materials "
		"and the required skill.  Required skills and materials are in the "
		"table above.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "If an item requires raw materials, then the specified amount "
		"of material is consumed for each item produced.  The higher one's "
		"skill, the more productive each man-month of work.";
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		temp += "Only Trade factions can issue ";
		temp += f.Link("#produce", "PRODUCE") + " orders however, regardless "
			"of skill levels.";
	}
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Items which increase production may increase production of "
		"advanced items in addition to the basic items listed.    Some of "
		"them also increase production of other tools.   Read the skill "
		"descriptions for details on which tools aid which production when "
		"not noted above.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	if(Globals->TOWNS_EXIST) {
		f.PutStr(f.LinkRef("economy_towns"));
		f.TagText("H3", "Villages, Towns, and Cities:");
		temp = "Some regions in Atlantis contain villages, towns, and "
			"cities.  Villages add to the wages, population, and tax income "
			"of the region they are in. ";
		if(Globals->FOOD_ITEMS_EXIST) {
			temp += "Also, villages will have an additional market for "
				"grain, livestock, and fish. ";
		}
		temp += "As the village's demand for goods is met, the population "
			"will increase. When the population reaches a certain theshold, "
			"the village will turn into a town.  A town will have some "
			"additional products that it demands, in addition to what it "
			"previously wanted.  Also a town will sell some new items as "
			"well. A town whose demands are being met will grow, and above "
			"another threshold it will become a full-blown city.  A city "
			"will have additional markets above the town including markets "
			"for more expensive trade items.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		temp = "Trade items are bought and sold only by cities, and have "
			"no other practical uses.  However, the profit margins on "
			"these items are usually quite high. ";
		if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			temp += "Buying and selling of these items in a region counts "
				"against a Trade faction's quota of regions in which it "
				"may undertake trade activity (note that buying and selling "
				"normal items does not).";
		}
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	f.PutStr(f.LinkRef("economy_buildings"));
	f.TagText("H3", "Buildings and Trade Structures:");
	temp = "Construction of buildings ";
	if(may_sail) temp += "and ships ";
	temp += "goes as follows: each unit of work requires a unit of the "
		"required resource and a man-month of work by a character with "
		"the appropriate skill and level; higher skill levels allow work "
		"to proceed faster still using one unit of the required resource "
		"per unit of work done). ";
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		temp += "Again, only Trade factions can issue ";
		temp += f.Link("#build", "BUILD") + " orders. ";
	}
	temp += "Here is a table of the various building types:";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("tablebuildings"));
	f.Enclose(1, "CENTER");
	f.Enclose(1, "TABLE BORDER=1");
	f.Enclose(1, "TR");
	f.TagText("TD", "");
	f.TagText("TH", "Size");
	f.TagText("TH", "Cost");
	f.TagText("TH", "Material");
	f.TagText("TH", "Skill (level)");
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
		if(ObjectDefs[i].level > 1)
			temp += AString("(") + ObjectDefs[i].level + ")";
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "There are other structures that increase the maximum production "
		"of certain resources in regions";
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "The first structure built in a region will increase the maximum "
		"production of the related product by 25%; the amount added by each "
		"additional structure will be half of the the effect of the previous "
		"one.  (Note that if you build enough of the same type of structure "
		"in a region, the new structures may not add _any_ to the production "
		"'level).";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("tabletradestructures"));
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
			temp += AString("(") + ObjectDefs[i].level + ")";
		f.PutStr(temp);
		f.Enclose(0, "TD");
		f.Enclose(1, "TD ALIGN=LEFT NOWRAP");
		f.PutStr(ItemDefs[ObjectDefs[i].productionAided].name);
		f.Enclose(0, "TD");
		f.Enclose(0, "TR");
	}
	f.Enclose(0, "TABLE");
	f.Enclose(0, "CENTER");
	f.PutStr("<P></P>");
	temp = "Note that these structures will not increase the availability "
		"of an item in a region which does not already have that item "
		"available. Also Trade structures do not offer defensive bonuses "
		"(which is why they do not have a size associated with them).  As "
		"with regular buildings, the Cost is the number of man-months of "
		"labor and also the number of units of raw material required to "
		"complete the structure.  It is possible that there are structures "
		"not listed above which require either advanced resources to build "
		"or which increase the production of advanced resources.  The skill "
		"description for a skill will always note if new structures may "
		"be built based on knowing that skill.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	if(!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED)) {
		f.PutStr(f.LinkRef("economy_roads"));
		f.TagText("H3", "Roads:");
		temp = "There is a another type of structure called roads.  They do "
			"not protect units, nor aid in the production of resources, but "
			"do aid movement, and can improve the economy of a hex.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		temp = "Roads are directional and are only considered to reach from "
			"one hexside to the center of the hex.  To gain a movement "
			"bonus, there must be two matching roads, one in each adjacent "
			"hex.  Only one road may be built in each direction.  If a road "
			"in the given direction is connected, units move along that "
			"road at half cost to a minimum of 1 movement point.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		temp = "For example: If a unit is moving northwest, then hex it is "
			"in must have a northwest road, and the hex it is moving into "
			"must have a southeast road.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		temp = "To gain an economy bonus, a hex must have roads that connect "
			"to roads in at least two adjoining hexes.  The economy bonus "
			"for the connected roads raises the wages in the region by 1 "
			"point.";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		f.PutStr(f.LinkRef("tableroadstructures"));
		f.Enclose(1, "CENTER");
		f.Enclose(1, "TABLE BORDER=1");
		f.Enclose(1, "TR");
		f.TagText("TD", "");
		f.TagText("TH", "Cost");
		f.TagText("TH", "Material");
		f.TagText("TH", "Skill (level)");
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
			if(ObjectDefs[i].level > 1)
				temp += AString("(") + ObjectDefs[i].level + ")";
			f.PutStr(temp);
			f.Enclose(0, "TD");
			f.Enclose(0, "TR");
		}
		f.Enclose(0, "TABLE");
		f.Enclose(0, "CENTER");
		f.PutStr("<P></P>");
	}
	if (Globals->DECAY) {
		f.PutStr(f.LinkRef("economy_builddecay"));
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
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	if(may_sail) {
		f.PutStr(f.LinkRef("economy_ships"));
		f.TagText("H3", "Ships:");
		temp = "Ships are constructed similarly to buildings, except they "
			"tend to be constructed out of wood, not stone, and their "
			"construction tends to depend on the Shipbuilding skill, not
			"the Building skill. ";
		if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			temp += "Only faction with at least one faction point spent on "
				"trade can issue";
			temp += f.Link("#build", "BUILD") + " orders. ";
		}
		temp += "Here is a table on the various ship types:";
		f.PutStr(temp);
		f.PutStr("<P></P>");
		f.PutStr(f.LinkRef("tableshipinfo"));
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
		f.PutStr("<P></P>");
		temp = "The capacity of a ship is the maximum weight that the ship "
			"may have aboard and still move. The cost is both the "
			"man-months of labor and the number of units of material "
			"required to complete the ship. The sailors are the number of "
			"skill levels of the Sailing skill that must be aboard the "
			"ship (and issuing the ";
		temp += f.Link("#sail", "SAIL") + " order in order for the ship "
			"to sail).";
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	f.PutStr(f.LinkRef("economy_advanceditems"));
	f.TagText("H3", "Advanced Items:");
	temp = "There are also certain advanced items that highly skilled units "
		"can produce. These are not available to starting players, but can "
		"be discovered through study.  When a unit is skilled enough to "
		"produce one of these items, he will receive a skill report "
		"describing the production of this item. Production of advanced "
		"items is generally done in a manner similar to the normal items.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("economy_income"));
	f.TagText("H3", "Income");
	temp = "Units can earn money with the ";
	temp += f.Link("#work", "WORK") + " order.  This means that the unit "
		"spends the month performing manual work for wages. The amount to "
		"be earned from this is usually not very high, so it is generally "
		"a last resort to be used if one is running out of money. The "
		"current wages are shown in the region description for each region. "
		"All units may ";
	temp += f.Link("#work", "WORK") + " regardless of skills or faction "
		"type.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	if(!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f.PutStr(f.LinkRef("economy_entertainment"));
		f.TagText("H3", "Entertainment:");
		temp = "Units with the Entertainment skill can use it to earn "
			"money.  A unit with Entertainment level 1 will earn ";
		temp += Globals->ENTERTAIN_INCOME + " silver per man by issuing the ";
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
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	f.PutStr(f.LinkRef("economy_taxingpillaging"));
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
		"description. Only combat ready units may ";
	temp += f.Link("#tax", "TAX") + "; a unit is combat ready if it either: "
		"has Combat skill of at least 1 or has a weapon (along with the "
		"appropriate skill for the weapon if required) in its possession. "
		"Each taxing character can collect $";
	temp += AString(Globals->TAX_INCOME) + ", though if the number of "
		"taxers would tax more than the available tax income, the tax "
		"income is split evenly among all taxers.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "It is possible to safeguard one's tax income in regions one "
		"controls.  Units which have the Guard flag set (using the ";
	temp += f.Link("#guard", "GUARD") + " order) will block " +
		f.Link("#tax", "TAX") + "orders issued by other factions in the same "
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("com"));
	f.ClassTagText("DIV", "rule", "");
	f.TagText("H2", "Combat");
	temp = "Combat occurs when one unit attacks another.  The computer then "
		"gathers together all the units on the attacking side, and all the "
		"units on the defending side, and the two sides fight until an "
		"outcome is reached.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("com_attitudes"));
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
		"mean that faction 27 has decided to treat you as an ally.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Ally means that you will fight to defend units of that faction "
		"whenever they come under attack, if you have non-avoiding units in "
		"the region where the attack occurs. ";
	if(has_stea) {
		temp += " You will also attempt to prevent any theft of "
			"assassination attempts against units of the faction";
		if(has_obse) {
			temp += ", if you are capable of seeing the unit which is "
				"attempting the crime";
		}
		temp += ". ";
	}
	temp += "It also has the implications of the Friendly attitude.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Friendly means that you will accept gifts from units of that "
		"faction.  This includes the giving of items, units of people, and "
		"the teaching of skills.  You will also admit units of that faction "
		"into buildings or ships owned by one of your units, and you will "
		"permit units of that faction to collect taxes (but not pillage) "
		"in regions where you have units on guard.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Unfriendly means that you will not admit units of that faction "
		"into any region where you have units on guard.  You will not, "
		"however, automatically attack unfriendly units which are already "
		"present.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Hostile means that any of your units which do not have the "
		"Avoid Combat flag set (using the ";
	temp += f.Link("#avoid", "AVOID") + " order ) will attack any units of "
		"that faction wherever they find them.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "If a unit can see another unit, but ";
	if(has_obse) {
		temp += "does not have high enough Observation skill to determine "
			"its faction,";
	} else {
		temp += "it is not revealing its faction,"
	}
	temp += "it will treat the unit using the faction's default attitude, "
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "If a faction declares Unfriendly or Hostile as default attitude "
		"(the latter is a good way to die fast), it will block or attack "
		"unidentified units, unless they belong to factions for which a "
		"Friendly or Ally attitude has been specifically declared.";
	if(has_stea) {
		temp += " Units which cannot be seen at all cannot be directly "
			"blocked or attacked, of course.";
	}
	f.PutStr(temp);
	f.PutStr("<P></P>");
	f.PutStr(f.LinkRef("com_attacking"));
	f.TagText("H3", "Attacking");
	temp = "A unit can attack another by issuing an ";
	temp += f.Link("#attack", "ATTACK") + " order. A unit that does not "
		"have Avoid Combat set will automatically attack any Hostile units "
		"it identifies as such.";
	if(has_stea || !(SkillDefs[S_RIDING].flags & Skill::DISABLED)) {
		temp += " When a unit issues the ";
		temp += f.Link("#attack", "ATTACK") + " order, or otherwise "
			"decides to attack another unit, it must first be able to "
			"attack the unit. ";
		if(has_stea && !(SkillDefs[S_RIDING].flags & Skill::DISABLED))
			temp += "There are two conditions for this; the first is that the"
		else
			temp += "The";
		if(has_stea) {
			temp += " attacking unit must be able to see the unit that it "
				"wishes to attack.  More information is available on this "
				"in the stealth section of the rules."
		}
		if(!SkillDefs[S_RIDING].flags & Skill::DISABLED) {
			if(has_stea) {
				f.PutStr(temp);
				f.PutStr("<P></P>");
				temp += "Secondly, the";
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
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
		f.PutStr(temp);
		f.PutStr("<P></P>");
	}
	f.PutStr(f.LinkRef("#com_muster"));
	f.TagText("H3", "The Muster:");
	temp = "Once the attack has been made, the sides are gathered.  Although "
		"the ";
	temp += f.Link("#attack", "ATTACK") + " order takes a unit rather than "
		"a faction as its parameter (mainly so that unidentified units can "
		"be attacked), an attack is basically considered to be by an entire "
		"faction, against an entire faction and its allies.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "On the attacking side are all units of the attacking faction in "
		"the region where the fight is taking place, except those with Avoid "
		"Combat set.  A unit which has explicitly (or implicitly via ";
	temp += f.Link("#advance", "ADVANCE") + ") issued an " +
		f.Link("#attack", "ATTACK") + " order will join the fight anyway, "
		"regardless of whether Avoid Combat is set.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Also on the attacking side are all units of other factions that "
		"attacked the target faction (implicitly or explicitly) in the "
		"region where the fight is taking place.  In other words, if several "
		"factions attack one, then all their armies join together to attack "
		"at the same time (even if they are enemies and will later fight "
		"each other).";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "On the defending side are all identifiable units belonging to "
		"the defending faction.  If a unit has Avoid Combat set and it "
		"belongs to the target faction, it will be uninvolved only if its "
		"faction cannot be identified by the attacking faction.  A unit "
		"which was explicitly attacked will be involved anyway, regardless "
		"of Avoid Combat. Also, all non-avoiding units located in the target "
		"region belonging to factions allied with the defending unit will "
		"join in on the defending side.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Units in adjacent regions can also become involved.  This is "
		"the exception to the general rule that you cannot interact with "
		"units in a different region.";
	f.PutStr(temp);
	f.PutStr("<P></P>");
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
	f.PutStr(temp);
	f.PutStr("<P></P>");
	temp = "Example:  A fight starts in region A, in the initial combat "
		"phase (before any movement has occurred).  The defender has a unit "
		"of soldiers in adjacent region B.  They have 2 movement points at "
		"this stage. ";
	if(Globals->WEATHER_EXISTS) {
		temp += "They will buy horses later in the turn, so that when "
			"they execute their ";
		temp += f.Link("#move", "MOVE") + " order they will have 4 movement "
			"points, but right now they have 2. ";
		temp += "Region A is forest, but fortunately it is summer, ";
	} else {
		temp += "Fortunately, region A is plains, ";
	}
	temp += "so the soldiers can join the fight.";
	f.PutStr(temp);
	f.PutStr("<P></P>");

	// RESUME
#if 0
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
