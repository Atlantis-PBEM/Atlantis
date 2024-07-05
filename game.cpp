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

#ifdef WIN32
#include <memory.h>	// Needed for memcpy on windows
#include "io.h"		// Needed for access() on windows
#define F_OK	0
#endif

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string.h>

#include "astring.h"
#include "game.h"
#include "gamedata.h"
#include "indenter.hpp"
#include "text_report_generator.hpp"
#include "quests.h"
#include "unit.h"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

using namespace std;

Game::Game()
{
	gameStatus = GAME_STATUS_UNINIT;
	ppUnits = 0;
	maxppunits = 0;
	events = new Events();

	if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT)
	{
		FactionTypes->push_back(F_WAR);
		FactionTypes->push_back(F_TRADE);
		FactionTypes->push_back(F_MAGIC);
	}
	else
	{
		FactionTypes->push_back(F_MARTIAL);
		FactionTypes->push_back(F_MAGIC);
	}
}

Game::~Game()
{
	delete[] ppUnits;
	delete events;
	ppUnits = 0;
	maxppunits = 0;
  	// Return the global array to it's original state. (needed for unit tests)
  	FactionTypes->clear();
}

int Game::TurnNumber()
{
	return (year-1)*12 + month + 1;
}

// ALT, 25-Jul-2000
// Default work order procedure
void Game::DefaultWorkOrder()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		if (r->type == R_NEXUS) continue;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->monthorders || u->faction->is_npc ||
						(Globals->TAX_PILLAGE_MONTH_LONG &&
						 u->taxing != TAX_NONE))
					continue;
				if (u->GetFlag(FLAG_AUTOTAX) &&
						(Globals->TAX_PILLAGE_MONTH_LONG && u->Taxers(1))) {
					u->taxing = TAX_AUTO;
				} else if (Globals->DEFAULT_WORK_ORDER) {
					ProcessWorkOrder(u, 1, 0);
				}
			}
		}
	}
}

AString Game::GetXtraMap(ARegion *reg,int type)
{
	int i;

	if (!reg)
		return(" ");

	switch (type) {
		case 0:
			return (reg->IsStartingCity() ? "!" :
					(reg->HasShaft() ? "*" : " "));
		case 1:
			i = reg->CountWMons();
			return (i ? ((AString) i) : (AString(" ")));
		case 2:
			forlist(&reg->objects) {
				Object *o = (Object *) elem;
				if (!(ObjectDefs[o->type].flags & ObjectType::CANENTER)) {
					if (o->units.First()) {
						return "*";
					} else {
						return ".";
					}
				}
			}
			return " ";
		case 3:
			if (reg->gate) return "*";
			return " ";
	}
	return(" ");
}

void Game::WriteSurfaceMap(ostream& f, ARegionArray *pArr, int type)
{
	ARegion *reg;
	int yy = 0;
	int xx = 0;

	f << "Map (" << xx*32 << "," << yy*16 << ")\n";
	for (int y=0; y < pArr->y; y+=2) {
		AString temp;
		int x;
		for (x=0; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16);
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);
			temp += "  ";
		}
		f << temp << "\n";
		temp = "  ";
		for (x=1; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16+1);
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);
			temp += "  ";
		}
		f << temp << "\n";
	}
	f << "\n";
}

void Game::WriteUnderworldMap(ostream& f, ARegionArray *pArr, int type)
{
	ARegion *reg, *reg2;
	int xx = 0;
	int yy = 0;
	f << "Map (" << xx*32 << "," << yy*16 << ")\n";
	for (int y=0; y< pArr->y; y+=2) {
		AString temp = " ";
		AString temp2;
		int x;
		for (x=0; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16);
			reg2 = pArr->GetRegion(x+xx*32+1,y+yy*16+1);
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);
			if (reg2 && reg2->neighbors[D_NORTH]) temp += "|";
			else temp += " ";

			temp += " ";
			if (reg && reg->neighbors[D_SOUTHWEST]) temp2 += "/";
			else temp2 += " ";

			temp2 += " ";
			if (reg && reg->neighbors[D_SOUTHEAST]) temp2 += "\\";
			else temp2 += " ";

			temp2 += " ";
		}
		f << temp << "\n" << temp2 << "\n";

		temp = " ";
		temp2 = "  ";
		for (x=1; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16+1);
			reg2 = pArr->GetRegion(x+xx*32-1,y+yy*16);

			if (reg2 && reg2->neighbors[D_SOUTH]) temp += "|";
			else temp += " ";

			temp += AString(" ");
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);

			if (reg && reg->neighbors[D_SOUTHWEST]) temp2 += "/";
			else temp2 += " ";

			temp2 += " ";
			if (reg && reg->neighbors[D_SOUTHEAST]) temp2 += "\\";
			else temp2 += " ";

			temp2 += " ";
		}
		f << temp << "\n" << temp2 << "\n";
	}
	f << "\n";
}

int Game::ViewMap(const AString & typestr,const AString & mapfile)
{
	int type = 0;
	if (typestr == "wmon") type = 1;
	if (typestr == "lair") type = 2;
	if (typestr == "gate") type = 3;
	if (typestr == "cities") type = 4;
	if (typestr == "hex") type = 5;

	ofstream f(mapfile.const_str(), ios::out|ios::ate);
	if (!f.is_open()) return(0);

	switch (type) {
		case 0:
			f << "Geographical Map\n";
			break;
		case 1:
			f << "Wandering Monster Map\n";
			break;
		case 2:
			f << "Lair Map\n";
			break;
		case 3:
			f << "Gate Map\n";
			break;
		case 4:
			f << "Cities Map\n";
			break;
	}

	// Cities map is a bit special since it is really just a list of all the cities in that region
	if (type == 4) {
		forlist(&regions) {
			ARegion *reg = (ARegion *)elem;
			// Ignore anything that isn't the surface
			if (reg->level->levelType != ARegionArray::LEVEL_SURFACE) continue;
			// Ignore anything with no city
			if (!reg->town || (reg->town->TownType() != TOWN_CITY)) continue;

			f << "(" << reg->xloc << "," << reg->yloc << "): " << reg->town->name << "\n";
		}
		return(1);
	}

	if (type == 5) {
		json worldmap;

		std::vector<ARegion *> start_regions;
		for (auto i = 0; i < regions.numLevels; i++) {
			ARegionArray *pArr = regions.pRegionArrays[i];
			if (pArr->levelType == ARegionArray::LEVEL_NEXUS) {
				if (!Globals->START_CITIES_EXIST) {
					ARegion *nexus = pArr->GetRegion(0, 0);
					forlist(&nexus->objects) {
						Object *o = (Object *) elem;
						if (o->inner != -1) {
							start_regions.push_back(regions.GetRegion(o->inner));
						}
					}
				}
				continue;
			};
			string label = (pArr->strName ? (pArr->strName->const_str() + to_string(i-1)) : "surface");
			worldmap[label] = json::array();

			for (int y = 0; y < pArr->y; y++) {
				for (int x = 0; x < pArr->x; x++) {
					ARegion *reg = pArr->GetRegion(x, y);
					if (!reg) continue;
					json data = reg->basic_region_data();
					json hexout = {
						{ "x", x },	{ "y", y },	{ "z", i },
						{ "terrain", data["terrain"] },
						{ "yew", reg->produces_item(I_YEW) },
						{ "mithril", reg->produces_item(I_MITHRIL) },
						{ "admantium", reg->produces_item(I_ADMANTIUM) },
						{ "floater", reg->produces_item(I_FLOATER) },
						{ "wing", reg->produces_item(I_WHORSE) },
						{ "gate", reg->gate != 0 },
						{ "exits", json::array() },
						{ "shaft", reg->HasShaft() },
						{ "starting",
						  (start_regions.end() != std::find(start_regions.begin(), start_regions.end(), reg)) ||
						  reg->IsStartingCity()
						}
					};
					// Fill in the exits
					for (auto d = 0; d < NDIRS; d++) {
						hexout["exits"].push_back(reg->neighbors[d] != nullptr);
					}
					if (reg->town) {
						hexout["town"] = data["settlement"]["size"];
					}
					worldmap[label].push_back(hexout);
				}
			}
		}
		f << worldmap.dump(2);
		return(1);
	}

	int i;
	for (i = 0; i < regions.numLevels; i++) {
		f << '\n';
		ARegionArray *pArr = regions.pRegionArrays[i];
		switch(pArr->levelType) {
			case ARegionArray::LEVEL_NEXUS:
				f << "Level " << i << ": Nexus\n";
				break;
			case ARegionArray::LEVEL_SURFACE:
				f << "Level " << i << ": Surface\n";
				WriteSurfaceMap(f, pArr, type);
				break;
			case ARegionArray::LEVEL_UNDERWORLD:
				f << "Level " << i << ": Underworld\n";
				WriteUnderworldMap(f, pArr, type);
				break;
			case ARegionArray::LEVEL_UNDERDEEP:
				f << "Level " << i << ": Underdeep\n";
				WriteUnderworldMap(f, pArr, type);
				break;
		}
	}
	return(1);
}

int Game::NewGame()
{
	factionseq = 1;
	guardfaction = 0;
	monfaction = 0;
	unitseq = 1;
	SetupUnitNums();
	shipseq = 100;
	year = 1;
	month = -1;
	gameStatus = GAME_STATUS_NEW;

	//
	// Seed the random number generator with a different value each time.
	//
	// init_random_seed() is a function reference that can be overwritten by the test suites to control the random seed
	// so that tests are repeatable.
	init_random_seed();

	CreateWorld();
	CreateNPCFactions();

	if (Globals->CITY_MONSTERS_EXIST)
		CreateCityMons();
	if (Globals->WANDERING_MONSTERS_EXIST)
		CreateWMons();
	if (Globals->LAIR_MONSTERS_EXIST)
		CreateLMons();

	if (Globals->LAIR_MONSTERS_EXIST)
		CreateVMons();
	
	/*	
	if (Globals->PLAYER_ECONOMY) {
		Equilibrate();
	}
	*/
	
	

	return(1);
}

int Game::OpenGame()
{
	//
	// The order here must match the order in SaveGame
	//
	ifstream f("game.in", ios::in);
	if (!f.is_open()) return(0);
	//
	// Read in Globals
	AString s1;
	f >> ws >> s1;
	if (f.eof()) return(0);

	AString *s2 = s1.gettoken();
	if (!s2) return(0);

	if (!(*s2 == "atlantis_game")) {
		delete s2;
		return(0);
	}
	delete s2;

	ATL_VER eVersion;
	f >> eVersion;
	Awrite(AString("Saved Game Engine Version: ") + ATL_VER_STRING(eVersion));
	if (ATL_VER_MAJOR(eVersion) != ATL_VER_MAJOR(CURRENT_ATL_VER) ||
			ATL_VER_MINOR(eVersion) != ATL_VER_MINOR(CURRENT_ATL_VER)) {
		Awrite("Incompatible Engine versions!");
		return(0);
	}
	if (ATL_VER_PATCH(eVersion) > ATL_VER_PATCH(CURRENT_ATL_VER)) {
		Awrite("This game was created with a more recent Atlantis Engine!");
		return(0);
	}

	AString gameName;
	f >> ws >> gameName;
	if (f.eof()) return(0);

	if (!(gameName == Globals->RULESET_NAME)) {
		Awrite("Incompatible rule-set!");
		return(0);
	}

	ATL_VER gVersion;
	f >> gVersion;
	Awrite(AString("Saved Rule-Set Version: ") + ATL_VER_STRING(gVersion));

	if (ATL_VER_MAJOR(gVersion) < ATL_VER_MAJOR(Globals->RULESET_VERSION)) {
		Awrite(AString("Upgrading to ") +
				ATL_VER_STRING(MAKE_ATL_VER(
						ATL_VER_MAJOR(Globals->RULESET_VERSION), 0, 0)));
		if (!UpgradeMajorVersion(gVersion)) {
			Awrite("Unable to upgrade!  Aborting!");
			return(0);
		}
		gVersion = MAKE_ATL_VER(ATL_VER_MAJOR(Globals->RULESET_VERSION), 0, 0);
	}
	if (ATL_VER_MINOR(gVersion) < ATL_VER_MINOR(Globals->RULESET_VERSION)) {
		Awrite(AString("Upgrading to ") +
				ATL_VER_STRING(MAKE_ATL_VER(
						ATL_VER_MAJOR(Globals->RULESET_VERSION),
						ATL_VER_MINOR(Globals->RULESET_VERSION), 0)));
		if (! UpgradeMinorVersion(gVersion)) {
			Awrite("Unable to upgrade!  Aborting!");
			return(0);
		}
		gVersion = MAKE_ATL_VER(ATL_VER_MAJOR(gVersion),
				ATL_VER_MINOR(Globals->RULESET_VERSION), 0);
	}
	if (ATL_VER_PATCH(gVersion) < ATL_VER_PATCH(Globals->RULESET_VERSION)) {
		Awrite(AString("Upgrading to ") +
				ATL_VER_STRING(Globals->RULESET_VERSION));
		if (! UpgradePatchLevel(gVersion)) {
			Awrite("Unable to upgrade!  Aborting!");
			return(0);
		}
		gVersion = MAKE_ATL_VER(ATL_VER_MAJOR(gVersion),
				ATL_VER_MINOR(gVersion),
				ATL_VER_PATCH(Globals->RULESET_VERSION));
	}

	f >> year;
	f >> month;

	int seed;
	f >> seed;
	seedrandom(seed);

	f >> factionseq;
	f >> unitseq;
	f >> shipseq;
	f >> guardfaction;
	f >> monfaction;

	//
	// Read in the Factions
	//
	int i;
	f >> i;

	for (int j = 0; j < i; j++) {
		Faction *temp = new Faction;
		temp->Readin(f);
		factions.Add(temp);
	}

	//
	// Read in the ARegions
	//
	i = regions.ReadRegions(f, &factions);
	if (!i) return 0;

	// read in quests
	if (!quests.ReadQuests(f))
		return 0;

	SetupUnitNums();

	return(1);
}

int Game::SaveGame()
{
	ofstream f("game.out", ios::out|ios::ate);
	if (!f.is_open()) return(0);

	//
	// Write out Globals
	//
	f << "atlantis_game\n";
	f << CURRENT_ATL_VER << "\n";
	f << Globals->RULESET_NAME << "\n";
	f << Globals->RULESET_VERSION << "\n";

	f << year << "\n";
	f << month << "\n";
	f << getrandom(10000) << "\n";
	f << factionseq << "\n";
	f << unitseq << "\n";
	f << shipseq << "\n";
	f << guardfaction << "\n";
	f << monfaction << "\n";
	//
	// Write out the Factions
	//
	f << factions.Num() << "\n";
	forlist(&factions) {
		((Faction *) elem)->Writeout(f);
	}

	//
	// Write out the ARegions
	//
	regions.WriteRegions(f);

	// Write out quests
	quests.WriteQuests(f);

	return(1);
}

void Game::DummyGame()
{
	//
	// No need to set anything up; we're just syntax checking some orders.
	//
}

#define PLAYERS_FIRST_LINE "AtlantisPlayerStatus"

int Game::WritePlayers()
{
	ofstream f("players.out", ios::out|ios::ate);
	if (!f.is_open()) return(0);

	f << PLAYERS_FIRST_LINE << "\n";
	f << "Version: " << CURRENT_ATL_VER << "\n";
	f << "TurnNumber: " << TurnNumber() << "\n";
	if (gameStatus == GAME_STATUS_UNINIT)
		return(0);
	
	if (gameStatus == GAME_STATUS_NEW)
		f << "GameStatus: New\n\n";
	else if (gameStatus == GAME_STATUS_RUNNING)
		f << "GameStatus: Running\n\n";
	else if (gameStatus == GAME_STATUS_FINISHED)
		f << "GameStatus: Finished\n\n";

	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		fac->WriteFacInfo(f);
	}

	return(1);
}

int Game::ReadPlayers()
{
	ifstream f("players.in", ios::in);
	if (!f.is_open()) return(0);

	AString pLine;
	AString *pToken = 0;

	//
	// Default: failure.
	//
	int rc = 0;

	do {
		//
		// The first line of the file should match.
		//
		f >> ws >> pLine;
		if (!(pLine == PLAYERS_FIRST_LINE)) break;

		//
		// Get the file version number.
		//
		f >> ws >> pLine;
		pToken = pLine.gettoken();
		if (!pToken || !(*pToken == "Version:")) break;
		SAFE_DELETE(pToken);

		pToken = pLine.gettoken();
		if (!pToken) break;

		int nVer = pToken->value();
		if (ATL_VER_MAJOR(nVer) != ATL_VER_MAJOR(CURRENT_ATL_VER) ||
				ATL_VER_MINOR(nVer) != ATL_VER_MINOR(CURRENT_ATL_VER) ||
				ATL_VER_PATCH(nVer) > ATL_VER_PATCH(CURRENT_ATL_VER)) {
			Awrite("The players.in file is not compatible with this "
					"version of Atlantis.");
			break;
		}
		SAFE_DELETE(pToken);

		//
		// Ignore the turn number line.
		//
		f >> ws >> pLine;

		//
		// Next, the game status.
		//
		f >> ws >> pLine;
		pToken = pLine.gettoken();
		if (!pToken || !(*pToken == "GameStatus:")) break;
		SAFE_DELETE(pToken);

		pToken = pLine.gettoken();
		if (!pToken) break;

		if (*pToken == "New")
			gameStatus = GAME_STATUS_NEW;
		else if (*pToken == "Running")
			gameStatus = GAME_STATUS_RUNNING;
		else if (*pToken == "Finished")
			gameStatus = GAME_STATUS_FINISHED;
		else {
			//
			// The status doesn't seem to be valid.
			//
			break;
		}
		SAFE_DELETE(pToken);

		//
		// Now, we should have a list of factions.
		//
		f >> ws >> pLine;
		Faction *pFac = 0;

		int lastWasNew = 0;

		//
		// OK, set our return code to success; we'll set it to fail below
		// if necessary.
		//
		rc = 1;

		while(!f.eof()) {
			pToken = pLine.gettoken();
			if (!pToken) {
				f >> ws >> pLine;
				continue;
			}

			if (*pToken == "Faction:") {
				//
				// Get the new faction
				//
				SAFE_DELETE(pToken);
				pToken = pLine.gettoken();
				if (!pToken) {
					rc = 0;
					break;
				}

				if (*pToken == "new") {
					AString save = pLine;
					int noleader = 0;
					int x, y, z;
					ARegion *pReg = NULL;

					/* Check for the noleader flag */
					SAFE_DELETE(pToken);
					pToken = pLine.gettoken();
					if (pToken && *pToken == "noleader") {
						noleader = 1;
						SAFE_DELETE(pToken);
						pToken = pLine.gettoken();
						/* Initialize pReg to something useful */
						pReg = regions.GetRegion(0, 0, 0);
					}
					if (pToken) {
						x = pToken->value();
						y = -1;
						z = -1;
						SAFE_DELETE(pToken);
						pToken = pLine.gettoken();
						if (pToken) {
							y = pToken->value();
							SAFE_DELETE(pToken);
							pToken = pLine.gettoken();
							if (pToken) {
								z = pToken->value();
								pReg = regions.GetRegion(x, y, z);
							}
						}
						if (pReg == NULL)
							Awrite(AString("Bad faction line: ")+save);
					}

					pFac = AddFaction(noleader, pReg);
					if (!pFac) {
						Awrite("Failed to add a new faction!");
						rc = 0;
						break;
					}

					lastWasNew = 1;
				} else {
					if (pFac && lastWasNew) {
						WriteNewFac(pFac);
					}
					int nFacNum = pToken->value();
					pFac = GetFaction(&factions, nFacNum);
					if (pFac)
						pFac->startturn = TurnNumber();
					lastWasNew = 0;
				}
			} else if (pFac) {
				if (!ReadPlayersLine(pToken, &pLine, pFac, lastWasNew)) {
					rc = 0;
					break;
				}
			}

			SAFE_DELETE(pToken);
			f >> ws >> pLine;
		}
		if (pFac && lastWasNew) {
			WriteNewFac(pFac);
		}
	} while(0);

	SAFE_DELETE(pToken);

	return(rc);
}

Unit *Game::ParseGMUnit(AString *tag, Faction *pFac)
{
	char *str = tag->Str();
	if (*str == 'g' && *(str+1) == 'm') {
		AString p = AString(str+2);
		int gma = p.value();
		forlist(&regions) {
			ARegion *reg = (ARegion *)elem;
			forlist(&reg->objects) {
				Object *obj = (Object *)elem;
				forlist(&obj->units) {
					Unit *u = (Unit *)elem;
					if (u->faction->num == pFac->num && u->gm_alias == gma) {
						return u;
					}
				}
			}
		}
	} else {
		int v = tag->value();
		if ((unsigned int)v >= maxppunits) return NULL;
		return GetUnit(v);
	}
	return NULL;
}

int Game::ReadPlayersLine(AString *pToken, AString *pLine, Faction *pFac,
		int newPlayer)
{
	AString *pTemp = 0;

	if (*pToken == "Name:") {
		pTemp = pLine->StripWhite();
		if (pTemp) {
			if (newPlayer) {
				*pTemp += AString(" (") + (pFac->num) + ")";
			}
			pFac->SetNameNoChange(pTemp);
		}
	} else if (*pToken == "RewardTimes") {
		pFac->TimesReward();
	} else if (*pToken == "Email:") {
		pTemp = pLine->gettoken();
		if (pTemp) {
			delete pFac->address;
			pFac->address = pTemp;
			pTemp = 0;
		}
	} else if (*pToken == "Password:") {
		pTemp = pLine->StripWhite();
		delete pFac->password;
		if (pTemp) {
			pFac->password = pTemp;
			pTemp = 0;
		} else {
			AString * pDefault = new AString("none");
			pFac->password = pDefault;
		}
	} else if (*pToken == "Battle:") {
		pFac->battleLogFormat = 0;
	} else if (*pToken == "Template:") {
		// LLS - looked like a good place to stick the Template test
		pTemp = pLine->gettoken();
		int nTemp = ParseTemplate(pTemp);
		pFac->temformat = TEMPLATE_LONG;
		if (nTemp != -1) pFac->temformat = nTemp;
	} else if (*pToken == "Reward:") {
		pTemp = pLine->gettoken();
		int nAmt = pTemp->value();
		pFac->event("Reward of " + to_string(nAmt) + " silver.", "reward");
		pFac->unclaimed += nAmt;
	} else if (*pToken == "SendTimes:") {
		// get the token, but otherwise ignore it
		pTemp = pLine->gettoken();
		pFac->times = pTemp->value();
	} else if (*pToken == "LastOrders:") {
		// Read this line and correctly set the lastorders for this
		// faction if the game itself isn't maintaining them.
		pTemp = pLine->gettoken();
		if (Globals->LASTORDERS_MAINTAINED_BY_SCRIPTS)
			pFac->lastorders = pTemp->value();
	} else if (*pToken == "FirstTurn:") {
		pTemp = pLine->gettoken();
		pFac->startturn = pTemp->value();
	} else if (*pToken == "Loc:") {
		int x, y, z;
		pTemp = pLine->gettoken();
		if (pTemp) {
			x = pTemp->value();
			delete pTemp;
			pTemp = pLine->gettoken();
			if (pTemp) {
				y = pTemp->value();
				delete pTemp;
				pTemp = pLine->gettoken();
				if (pTemp) {
					z = pTemp->value();
					ARegion *pReg = regions.GetRegion(x, y, z);
					if (pReg) {
						pFac->pReg = pReg;
					} else {
						Awrite(AString("Invalid Loc:")+x+","+y+","+z+
								" in faction " + pFac->num);
						pFac->pReg = NULL;
					}
				}
			}
		}
	} else if (*pToken == "NewUnit:") {
		// Creates a new unit in the location specified by a Loc: line
		// with a gm_alias of whatever is after the NewUnit: tag.
		if (!pFac->pReg) {
			Awrite(AString("NewUnit is not valid without a Loc: ") +
					"for faction "+ pFac->num);
		} else {
			pTemp = pLine->gettoken();
			if (!pTemp) {
				Awrite(AString("NewUnit: must be followed by an alias ") +
						"in faction "+pFac->num);
			} else {
				int val = pTemp->value();
				if (!val) {
					Awrite(AString("NewUnit: must be followed by an alias ") +
							"in faction "+pFac->num);
				} else {
					Unit *u = GetNewUnit(pFac);
					u->gm_alias = val;
					u->MoveUnit(pFac->pReg->GetDummy());
					u->event("Is given to your faction.", "gm_gift");
				}
			}
		}
	} else if (*pToken == "Item:") {
		pTemp = pLine->gettoken();
		if (!pTemp) {
			Awrite(AString("Item: needs to specify a unit in faction ") +
					pFac->num);
		} else {
			Unit *u = ParseGMUnit(pTemp, pFac);
			if (!u) {
				Awrite(AString("Item: needs to specify a unit in faction ") +
						pFac->num);
			} else {
				if (u->faction->num != pFac->num) {
					Awrite(AString("Item: unit ")+ u->num +
							" doesn't belong to " + "faction " + pFac->num);
				} else {
					delete pTemp;
					pTemp = pLine->gettoken();
					if (!pTemp) {
						Awrite(AString("Must specify a number of items to ") +
								"give for Item: in faction " + pFac->num);
					} else {
						int v = pTemp->value();
						if (!v) {
							Awrite(AString("Must specify a number of ") +
										"items to give for Item: in " +
										"faction " + pFac->num);
						} else {
							delete pTemp;
							pTemp = pLine->gettoken();
							if (!pTemp) {
								Awrite(AString("Must specify a valid item ") +
										"to give for Item: in faction " +
										pFac->num);
							} else {
								int it = ParseAllItems(pTemp);
								if (it == -1) {
									Awrite(AString("Must specify a valid ") +
											"item to give for Item: in " +
											"faction " + pFac->num);
								} else {
									int has = u->items.GetNum(it);
									u->items.SetNum(it, has + v);
									if (!u->gm_alias) {
										u->event("Is given " + ItemString(it, v) + " by the gods.", "gm_gift");
									}
									u->faction->DiscoverItem(it, 0, 1);
								}
							}
						}
					}
				}
			}
		}
	} else if (*pToken == "Skill:") {
		pTemp = pLine->gettoken();
		if (!pTemp) {
			Awrite(AString("Skill: needs to specify a unit in faction ") +
					pFac->num);
		} else {
			Unit *u = ParseGMUnit(pTemp, pFac);
			if (!u) {
				Awrite(AString("Skill: needs to specify a unit in faction ") +
						pFac->num);
			} else {
				if (u->faction->num != pFac->num) {
					Awrite(AString("Skill: unit ")+ u->num +
							" doesn't belong to " + "faction " + pFac->num);
				} else {
					delete pTemp;
					pTemp = pLine->gettoken();
					if (!pTemp) {
						Awrite(AString("Must specify a valid skill for ") +
								"Skill: in faction " + pFac->num);
					} else {
						int sk = ParseSkill(pTemp);
						if (sk == -1) {
							Awrite(AString("Must specify a valid skill for ")+
									"Skill: in faction " + pFac->num);
						} else {
							delete pTemp;
							pTemp = pLine->gettoken();
							if (!pTemp) {
								Awrite(AString("Must specify a days for ") +
										"Skill: in faction " + pFac->num);
							} else {
								int days = pTemp->value() * u->GetMen();
								if (!days) {
									Awrite(AString("Must specify a days for ")+
											"Skill: in faction " + pFac->num);
								} else {
									int odays = u->skills.GetDays(sk);
									u->skills.SetDays(sk, odays + days);
									u->AdjustSkills();
									int lvl = u->GetRealSkill(sk);
									if (lvl > pFac->skills.GetDays(sk)) {
										pFac->skills.SetDays(sk, lvl);
										pFac->shows.push_back({ .skill = sk, .level = lvl });
									}
									if (!u->gm_alias) {
										u->event("Is taught " + to_string(days) + " days of " +
											SkillStrs(sk).const_str() + " by the gods.", "gm_gift");
									}
									/*
									 * This is NOT quite the same, but the gods
									 * are more powerful than mere mortals
									 */
									int mage = (SkillDefs[sk].flags &
											SkillType::MAGIC);
									int app = (SkillDefs[sk].flags &
											SkillType::APPRENTICE);
									if (mage) {
										u->type = U_MAGE;
									}
									if (app && u->type == U_NORMAL) {
										u->type = U_APPRENTICE;
									}
								}
							}
						}
					}
				}
			}
		}
	} else if (*pToken == "Order:") {
		pTemp = pLine->StripWhite();
		if (*pTemp == "quit") {
			pFac->quit = QUIT_BY_GM;
		} else {
			// handle this as a unit order
			delete pTemp;
			pTemp = pLine->gettoken();
			if (!pTemp) {
				Awrite(AString("Order: needs to specify a unit in faction ") +
						pFac->num);
			} else {
				Unit *u = ParseGMUnit(pTemp, pFac);
				if (!u) {
					Awrite(AString("Order: needs to specify a unit in ")+
							"faction " + pFac->num);
				} else {
					if (u->faction->num != pFac->num) {
						Awrite(AString("Order: unit ")+ u->num +
								" doesn't belong to " + "faction " +
								pFac->num);
					} else {
						delete pTemp;
						AString saveorder = *pLine;
						int getatsign = pLine->getat();
						pTemp = pLine->gettoken();
						if (!pTemp) {
							Awrite(AString("Order: must provide unit order ")+
									"for faction "+pFac->num);
						} else {
							int o = Parse1Order(pTemp);
							if (o == -1 || o == O_ATLANTIS || o == O_END ||
									o == O_UNIT || o == O_FORM ||
									o == O_ENDFORM) {
								Awrite(AString("Order: invalid order given ")+
										"for faction "+pFac->num);
							} else {
								if (getatsign) {
									u->oldorders.Add(new AString(saveorder));
								}
								ProcessOrder(o, u, pLine, NULL);
							}
						}
					}
				}
			}
		}
	} else {
		string temp = string(pToken->const_str()) + pLine->const_str();
		pFac->extra_player_data.push_back(temp);
	}

	if (pTemp) delete pTemp;
	return(1);
}

void Game::WriteNewFac(Faction *pFac)
{
	AString *strFac = new AString(AString("Adding ") + *(pFac->address) + ".");
	newfactions.Add(strFac);
}

int Game::DoOrdersCheck(const AString &strOrders, const AString &strCheck)
{
	ifstream ordersFile(strOrders.const_str(), ios::in);
	if (!ordersFile.is_open()) {
		Awrite("No such orders file!");
		return(0);
	}

	ofstream checkFile(strCheck.const_str(), ios::out|ios::ate);
	if (!checkFile.is_open()) {
		Awrite("Couldn't open the orders check file!");
		return(0);
	}

	OrdersCheck check(checkFile);
	ParseOrders(0, ordersFile, &check);

	return(1);
}

int Game::RunGame()
{
	Awrite("Setting Up Turn...");
	PreProcessTurn();

	Awrite("Reading the Gamemaster File...");
	if (!ReadPlayers()) return(0);

	if (gameStatus == GAME_STATUS_FINISHED) {
		Awrite("This game is finished!");
		return(0);
	}
	gameStatus = GAME_STATUS_RUNNING;

	Awrite("Reading the Orders File...");
	ReadOrders();

	if (Globals->MAX_INACTIVE_TURNS != -1) {
		Awrite("QUITting Inactive Factions...");
		RemoveInactiveFactions();
	}

	Awrite("Running the Turn...");
	RunOrders();

	if (Globals->WORLD_EVENTS) {
		Awrite("Writing world events...");
		WriteWorldEvents();
	}

	Awrite("Writing the Report File...");
	WriteReport();
	Awrite("");

	battles.clear();

	EmptyHell();

	Awrite("Writing Playerinfo File...");
	WritePlayers();

	Awrite("Removing Dead Factions...");
	DeleteDeadFactions();

	Awrite("done");

	return(1);
}

void Game::RecordFact(FactBase* fact) {
	this->events->AddFact(fact);
}

void Game::WriteWorldEvents() {
	std::string text = this->events->Write(Globals->RULESET_NAME, MonthNames[this->month], this->year);
	if (text.empty() || text.length() == 0) return;

	this->WriteTimesArticle(text.c_str());
}

void Game::PreProcessTurn()
{
	month++;
	if (month>11) {
		month = 0;
		year++;
	}
	SetupUnitNums();
	forlist(&factions) {
		((Faction *) elem)->DefaultOrders();
	}
	forlist_reuse(&regions) {
		ARegion *pReg = (ARegion *) elem;
		if (Globals->WEATHER_EXISTS)
			pReg->SetWeather(regions.GetWeather(pReg, month));
		if (Globals->GATES_NOT_PERENNIAL)
			pReg->SetGateStatus(month);
		pReg->DefaultOrders();
	}
}

void Game::ClearOrders(Faction *f)
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == f) {
					u->ClearOrders();
				}
			}
		}
	}
}

void Game::ReadOrders()
{
	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		if (!fac->is_npc) {
			AString str = "orders.";
			str += fac->num;

			ifstream file(str.const_str(), ios::in);
			if(file.is_open()) {
				ParseOrders(fac->num, file, 0);
				file.close();
			}
			DefaultWorkOrder();
		}
	}
}

void Game::MakeFactionReportLists()
{
	vector<Faction *> facs (factionseq, nullptr);

	forlist(&regions) {
		// clear the temporary
		fill(facs.begin(), facs.end(), nullptr);

		ARegion *reg = (ARegion *) elem;
		forlist(&reg->farsees) {
			Faction *fac = ((Farsight *) elem)->faction;
			facs[fac->num] = fac;
		}
		{
			forlist(&reg->passers) {
				Faction *fac = ((Farsight *)elem)->faction;
				facs[fac->num] = fac;
			}
		}
		{
			forlist(&reg->objects) {
				Object *obj = (Object *) elem;

				forlist(&obj->units) {
					Unit *unit = (Unit *) elem;
					facs[unit->faction->num] = unit->faction;
				}
			}
		}

		for(const auto& fac: facs) {
			if (fac) fac->present_regions.push_back(reg);
		}
	}
}

void Game::WriteReport()
{
	MakeFactionReportLists();
	CountAllSpecialists();

	size_t ** citems = nullptr;
	
	if (Globals->FACTION_STATISTICS) {
		citems = new size_t * [factionseq];
		for (int i = 0; i < factionseq; i++)
		{
			citems [i] = new size_t [NITEMS];
			for (int j = 0; j < NITEMS; j++)
			{
				citems [i][j] = 0;
			}
		}
		CountItems(citems);
	}

	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		string report_file = "report." + to_string(fac->num);
		string template_file = "template." + to_string(fac->num);

		if (!fac->is_npc || fac->gets_gm_report(this)) {
			// Generate the report in JSON format and then write it to whatever formats we want
			json json_report;
			bool show_region_depth = Globals->EASIER_UNDERWORLD
				&& (Globals->UNDERWORLD_LEVELS + Globals->UNDERDEEP_LEVELS > 1);
			fac->build_json_report(json_report, this, citems);

			if (Globals->REPORT_FORMAT & GameDefs::REPORT_FORMAT_TEXT) {
				TextReportGenerator text_report;
				ofstream f(report_file, ios::out | ios::ate);
				if (f.is_open()) {
					text_report.output(f, json_report, show_region_depth);
				}
				if (!fac->is_npc && fac->temformat != TEMPLATE_OFF) {
					// even factions which get a gm report do not get a template.
					ofstream f(template_file, ios::out | ios::ate);
					if (f.is_open()) {
						text_report.output_template(f, json_report, fac->temformat, show_region_depth);
					}
				}
			}

			if (Globals->REPORT_FORMAT & GameDefs::REPORT_FORMAT_JSON) {
				ofstream jsonf(report_file + ".json", ios::out | ios::ate);
				if (jsonf.is_open()) {
					jsonf << json_report.dump(2);
				}
			}
		}
		Adot();
	}

	// stop leaking memory
	if (Globals->FACTION_STATISTICS) {
		for (auto i = 0; i < factionseq; i++) delete [] citems [i];
		delete [] citems;
	}
}

void Game::DeleteDeadFactions()
{
	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		if (!fac->is_npc && !fac->exists) {
			factions.Remove(fac);
			forlist((&factions))
				((Faction *) elem)->remove_attitude(fac->num);
			delete fac;
		}
	}
}

Faction *Game::AddFaction(int noleader, ARegion *pStart)
{
	//
	// set up faction
	//
	Faction *temp = new Faction(factionseq);
	AString x("NoAddress");
	temp->SetAddress(x);
	temp->lastorders = TurnNumber();
	temp->startturn = TurnNumber();
	temp->pStartLoc = pStart;
	temp->pReg = pStart;
	temp->noStartLeader = noleader;

	if (!SetupFaction(temp)) {
		delete temp;
		return 0;
	}
	factions.Add(temp);
	factionseq++;
	return temp;
}

void Game::ViewFactions()
{
	forlist((&factions))
		((Faction *) elem)->View();
}

void Game::SetupUnitSeq()
{
	int max = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if (u && u->num > max) max = u->num;
			}
		}
	}
	unitseq = max+1;
}

void Game::SetupUnitNums()
{
	if (ppUnits) delete[] ppUnits;

	SetupUnitSeq();

	maxppunits = unitseq+10000;

	ppUnits = new Unit *[maxppunits];

	unsigned int i;
	for (i = 0; i < maxppunits ; i++) ppUnits[i] = 0;

	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				i = u->num;
				if ((i > 0) && (i < maxppunits)) {
					if (!ppUnits[i])
						ppUnits[u->num] = u;
					else {
						Awrite(AString("Error: Unit number ") + i +
								" multiply defined.");
						if ((unitseq > 0) && (unitseq < maxppunits)) {
							u->num = unitseq;
							ppUnits[unitseq++] = u;
						}
					}
				} else {
					Awrite(AString("Error: Unit number ")+i+
							" out of range.");
					if ((unitseq > 0) && (unitseq < maxppunits)) {
						u->num = unitseq;
						ppUnits[unitseq++] = u;
					}
				}
			}
		}
	}
}

Unit *Game::GetNewUnit(Faction *fac, int an)
{
	unsigned int i;
	for (i = 1; i < unitseq; i++) {
		if (!ppUnits[i]) {
			Unit *pUnit = new Unit(i, fac, an);
			ppUnits[i] = pUnit;
			return(pUnit);
		}
	}

	Unit *pUnit = new Unit(unitseq, fac, an);
	ppUnits[unitseq] = pUnit;
	unitseq++;
	if (unitseq >= maxppunits) {
		Unit **temp = new Unit*[maxppunits+10000];
		memcpy(temp, ppUnits, maxppunits*sizeof(Unit *));
		maxppunits += 10000;
		delete[] ppUnits;
		ppUnits = temp;
	}

	return(pUnit);
}

Unit *Game::GetUnit(int num)
{
	if (num < 0 || (unsigned int)num >= maxppunits) return NULL;
	return(ppUnits[num]);
}


void Game::CountAllSpecialists()
{
	forlist(&factions) {
		((Faction *) elem)->nummages = 0;
		((Faction *) elem)->numqms = 0;
		((Faction *) elem)->numtacts = 0;
		((Faction *) elem)->numapprentices = 0;
	}

	{
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			forlist(&r->objects) {
				Object *o = (Object *) elem;
				forlist(&o->units) {
					Unit *u = (Unit *) elem;
					if (u->type == U_MAGE) u->faction->nummages++;
					if (u->GetSkill(S_QUARTERMASTER))
						u->faction->numqms++;
					if (u->GetSkill(S_TACTICS) == 5)
						u->faction->numtacts++;
					if (u->type == U_APPRENTICE)
						u->faction->numapprentices++;
				}
			}
		}
	}
}

// LLS
void Game::UnitFactionMap()
{
	unsigned int i;
	Unit *u;

	Awrite("Opening units.txt");
	ofstream f("units.txt", ios::out|ios::ate);
	if (!f.is_open()) {
		Awrite("Couldn't open file!");
		return;
	}
	Awrite(AString("Writing ") + unitseq + " units");
	for (i = 1; i < unitseq; i++) {
		u = GetUnit(i);
		if (!u) {
			Awrite("doesn't exist");
		} else {
			Awrite(AString(i) + ":" + u->faction->num);
			f << i << ":" << u->faction->num << endl;
		}
	}
}

//The following function added by Creative PBM February 2000
void Game::RemoveInactiveFactions()
{
	if (Globals->MAX_INACTIVE_TURNS == -1) return;

	int cturn;
	cturn = TurnNumber();
	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		if ((cturn - fac->lastorders) >= Globals->MAX_INACTIVE_TURNS &&	!fac->is_npc) {
			fac->quit = QUIT_BY_GM;
		}
	}
}

/*
void Game::CountAllApprentices()
{
	if (!Globals->APPRENTICES_EXIST) return;

	forlist(&factions) {
		((Faction *)elem)->numapprentices = 0;
	}
	{
		forlist(&regions) {
			ARegion *r = (ARegion *)elem;
			forlist(&r->objects) {
				Object *o = (Object *)elem;
				forlist(&o->units) {
					Unit *u = (Unit *)elem;
					if (u->type == U_APPRENTICE)
						u->faction->numapprentices++;
				}
			}
		}
	}
}
*/

int Game::CountMages(Faction *pFac)
{
	int i = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == pFac && u->type == U_MAGE) i++;
			}
		}
	}
	return(i);
}

int Game::CountQuarterMasters(Faction *pFac)
{
	int i = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if (u->faction == pFac && u->GetSkill(S_QUARTERMASTER)) i++;
			}
		}
	}
	return i;
}

int Game::CountTacticians(Faction *pFac)
{
	int i = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if (u->faction == pFac && u->GetSkill(S_TACTICS) == 5) i++;
			}
		}
	}
	return i;
}

int Game::CountApprentices(Faction *pFac)
{
	int i = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if (u->faction == pFac && u->type == U_APPRENTICE) i++;
			}
		}
	}
	return i;
}

int Game::AllowedMages(Faction *pFac)
{
	int points = pFac->type[F_MAGIC];

	if (points < 0) points = 0;
	if (points > allowedMagesSize - 1) points = allowedMagesSize - 1;

	return allowedMages[points];
}

int Game::AllowedQuarterMasters(Faction *pFac)
{
	int points = std::max(pFac->type[F_TRADE], pFac->type[F_MARTIAL]);

	if (points < 0) points = 0;
	if (points > allowedQuartermastersSize - 1)
		points = allowedQuartermastersSize - 1;

	return allowedQuartermasters[points];
}

int Game::AllowedTacticians(Faction *pFac)
{
	int points = std::max(pFac->type[F_WAR], pFac->type[F_MARTIAL]);

	if (points < 0) points = 0;
	if (points > allowedTacticiansSize - 1)
		points = allowedTacticiansSize - 1;

	return allowedTacticians[points];
}

int Game::AllowedApprentices(Faction *pFac)
{
	int points = pFac->type[F_MAGIC];

	if (points < 0) points = 0;
	if (points > allowedApprenticesSize - 1)
		points = allowedApprenticesSize - 1;

	return allowedApprentices[points];
}

int Game::AllowedTaxes(Faction *pFac)
{
	int points = std::max(pFac->type[F_WAR], pFac->type[F_MARTIAL]);

	if (points < 0) points = 0;
	if (points > allowedTaxesSize - 1) points = allowedTaxesSize - 1;

	return allowedTaxes[points];
}

int Game::AllowedTrades(Faction *pFac)
{
	int points = std::max(pFac->type[F_TRADE], pFac->type[F_MARTIAL]);

	if (points < 0) points = 0;
	if (points > allowedTradesSize - 1) points = allowedTradesSize - 1;

	return allowedTrades[points];
}

int Game::AllowedMartial(Faction *pFac)
{
	int points = pFac->type[F_MARTIAL];
	
	if (points < 0) points = 0;
	if (points > allowedMartialSize - 1) points = allowedMartialSize - 1;

	return allowedMartial[points];
}

int Game::UpgradeMajorVersion(int savedVersion)
{
	return 0;
}

int Game::UpgradeMinorVersion(int savedVersion)
{
	return 1;
}

int Game::UpgradePatchLevel(int savedVersion)
{
	return 1;
}

void Game::MidProcessUnitExtra(ARegion *r, Unit *u)
{
	if (Globals->CHECK_MONSTER_CONTROL_MID_TURN) MonsterCheck(r, u);
}

void Game::PostProcessUnitExtra(ARegion *r, Unit *u)
{
	if (!Globals->CHECK_MONSTER_CONTROL_MID_TURN) MonsterCheck(r, u);
}

void Game::MonsterCheck(ARegion *r, Unit *u)
{
	AString tmp;
	int skill;
	int linked = 0;
	map< int, int > chances;

	if (u->type != U_WMON) {

		forlist (&u->items) {
			Item *i = (Item *) elem;
			if (!i->num) continue;
			if (!ItemDefs[i->type].escape) continue;

			// Okay, check flat loss.
			if (ItemDefs[i->type].escape & ItemType::LOSS_CHANCE) {
				int losses = (i->num +
						getrandom(ItemDefs[i->type].esc_val)) /
					ItemDefs[i->type].esc_val;
				// LOSS_CHANCE and HAS_SKILL together mean the
				// decay rate only applies if you don't have
				// the required skill (this might get used if
				// you made illusions GIVEable, for example).
				if (ItemDefs[i->type].escape & ItemType::HAS_SKILL) {
					tmp = ItemDefs[i->type].esc_skill;
					skill = LookupSkill(&tmp);
					if (u->GetSkill(skill) >= ItemDefs[i->type].esc_val)
						losses = 0;
				}
				if (losses) {
					string temp = ItemString(i->type, losses) + plural(losses, " decay", " decays") +
						" into nothingness.";
					u->event(temp, "decay");
					u->items.SetNum(i->type,i->num - losses);
				}
			} else if (ItemDefs[i->type].escape & ItemType::HAS_SKILL) {
				tmp = ItemDefs[i->type].esc_skill;
				skill = LookupSkill(&tmp);
				if (u->GetSkill(skill) < ItemDefs[i->type].esc_val) {
					if (Globals->WANDERING_MONSTERS_EXIST) {
						Faction *mfac = GetFaction(&factions, monfaction);
						Unit *mon = GetNewUnit(mfac, 0);
						MonType *mp = FindMonster(ItemDefs[i->type].abr,
								(ItemDefs[i->type].type & IT_ILLUSION));
						mon->MakeWMon(mp->name, i->type, i->num);
						mon->MoveUnit(r->GetDummy());
						// This will be zero unless these are set. (0 means
						// full spoils)
						mon->free = Globals->MONSTER_NO_SPOILS +
							Globals->MONSTER_SPOILS_RECOVERY;
					}
					u->event("Loses control of " + ItemString(i->type, i->num) + ".", "escape");
					u->items.SetNum(i->type, 0);
				}
			} else {
				// ESC_LEV_*
				tmp = ItemDefs[i->type].esc_skill;
				skill = LookupSkill(&tmp);
				int level = u->GetSkill(skill);
				int chance;

				if (!level)
					chance = 10000;
				else {
					int top;
					if (ItemDefs[i->type].escape & ItemType::ESC_NUM_SQUARE)
						top = i->num * i->num;
					else
						top = i->num;
					int bottom = 0;
					if (ItemDefs[i->type].escape & ItemType::ESC_LEV_LINEAR)
						bottom = level;
					else if (ItemDefs[i->type].escape & ItemType::ESC_LEV_SQUARE)
						bottom = level * level;
					else if (ItemDefs[i->type].escape & ItemType::ESC_LEV_CUBE)
						bottom = level * level * level;
					else if (ItemDefs[i->type].escape & ItemType::ESC_LEV_QUAD)
						bottom = level * level * level * level;
					else
						bottom = 1;
					bottom = bottom * ItemDefs[i->type].esc_val;
					chance = (top * 10000)/bottom;
				}

				if (ItemDefs[i->type].escape & ItemType::LOSE_LINKED) {
					if (chance > chances[ItemDefs[i->type].type])
						chances[ItemDefs[i->type].type] = chance;
					linked = 1;
				} else if (chance > getrandom(10000)) {
					if (Globals->WANDERING_MONSTERS_EXIST) {
						Faction *mfac = GetFaction(&factions, monfaction);
						Unit *mon = GetNewUnit(mfac, 0);
						MonType *mp = FindMonster(ItemDefs[i->type].abr,
								(ItemDefs[i->type].type & IT_ILLUSION));
						mon->MakeWMon(mp->name, i->type, i->num);
						mon->MoveUnit(r->GetDummy());
						// This will be zero unless these are set. (0 means
						// full spoils)
						mon->free = Globals->MONSTER_NO_SPOILS +
							Globals->MONSTER_SPOILS_RECOVERY;
					}
					u->event("Loses control of " + ItemString(i->type, i->num) + ".", "escape");
					u->items.SetNum(i->type, 0);
				}
			}
		}

		if (linked) {
			map < int, int >::iterator i;
			for (i = chances.begin(); i != chances.end(); i++) {
				// walk the chances list and for each chance, see if
				// escape happens and if escape happens then walk all items
				// and everything that is that type, get rid of it.
				if ((*i).second < getrandom(10000)) continue;
				forlist (&u->items) {
					Item *it = (Item *)elem;
					if (ItemDefs[it->type].type == (*i).first) {
						if (Globals->WANDERING_MONSTERS_EXIST) {
							Faction *mfac = GetFaction(&factions, monfaction);
							Unit *mon = GetNewUnit(mfac, 0);
							MonType *mp = FindMonster(ItemDefs[it->type].abr,
									(ItemDefs[it->type].type & IT_ILLUSION));
							mon->MakeWMon(mp->name, it->type, it->num);
							mon->MoveUnit(r->GetDummy());
							// This will be zero unless these are set. (0 means
							// full spoils)
							mon->free = Globals->MONSTER_NO_SPOILS +
								Globals->MONSTER_SPOILS_RECOVERY;
						}
						u->event("Loses control of " + ItemString(it->type, it->num) + ".", "escape");
						u->items.SetNum(it->type, 0);
					}
				}
			}
		}

	}
}

void Game::CheckUnitMaintenance(int consume)
{
	CheckUnitMaintenanceItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE, consume);
	CheckUnitMaintenanceItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE, consume);
	CheckUnitMaintenanceItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE, consume);
	CheckUnitMaintenanceItem(I_FISH, Globals->UPKEEP_FOOD_VALUE, consume);
}

void Game::CheckFactionMaintenance(int con)
{
	CheckFactionMaintenanceItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE, con);
	CheckFactionMaintenanceItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE, con);
	CheckFactionMaintenanceItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE, con);
	CheckFactionMaintenanceItem(I_FISH, Globals->UPKEEP_FOOD_VALUE, con);
}

void Game::CheckAllyMaintenance()
{
	CheckAllyMaintenanceItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
	CheckAllyMaintenanceItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
	CheckAllyMaintenanceItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
	CheckAllyMaintenanceItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

void Game::CheckUnitHunger()
{
	CheckUnitHungerItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
	CheckUnitHungerItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
	CheckUnitHungerItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
	CheckUnitHungerItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

void Game::CheckFactionHunger()
{
	CheckFactionHungerItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
	CheckFactionHungerItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
	CheckFactionHungerItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
	CheckFactionHungerItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

void Game::CheckAllyHunger()
{
	CheckAllyHungerItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
	CheckAllyHungerItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
	CheckAllyHungerItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
	CheckAllyHungerItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

char Game::GetRChar(ARegion *r)
{
	int t;

	if (!r)
		return ' ';
	t = r->type;
	if (t < 0 || t > R_NUM) return '?';
	char c = TerrainDefs[r->type].marker;
	if (r->town) {
		c = (c - 'a') + 'A';
	}
	return c;
}

void Game::CreateNPCFactions()
{
	Faction *f;
	AString *temp;
	if (Globals->CITY_MONSTERS_EXIST) {
		f = new Faction(factionseq++);
		guardfaction = f->num;
		temp = new AString("The Guardsmen");
		f->SetName(temp);
		f->is_npc = true;
		f->lastorders = 0;
		factions.Add(f);
	} else
		guardfaction = 0;
	// Only create the monster faction if wandering monsters or lair
	// monsters exist.
	if (Globals->LAIR_MONSTERS_EXIST || Globals->WANDERING_MONSTERS_EXIST) {
		f = new Faction(factionseq++);
		monfaction = f->num;
		temp = new AString("Creatures");
		f->SetName(temp);
		f->is_npc = true;
		f->lastorders = 0;
		factions.Add(f);
	} else
		monfaction = 0;
}

void Game::CreateCityMon(ARegion *pReg, int percent, int needmage)
{
	int skilllevel;
	int AC = 0;
	int IV = 0;
	int num;
	if (pReg->type == R_NEXUS || pReg->IsStartingCity()) {
		skilllevel = TOWN_CITY + 1;
		if (Globals->SAFE_START_CITIES || (pReg->type == R_NEXUS))
			IV = 1;
		AC = 1;
		num = Globals->AMT_START_CITY_GUARDS;
	} else {
		skilllevel = pReg->town->TownType() + 1;
		num = Globals->CITY_GUARD * skilllevel;
	}
	num = num * percent / 100;
	Faction *pFac = GetFaction(&factions, guardfaction);
	Unit *u = GetNewUnit(pFac);
	Unit *u2;
	AString *s = new AString("City Guard");
	
	/*
	Awrite(AString("Begin setting up city guard in..."));
		
	AString temp = TerrainDefs[pReg->type].name;
	temp += AString(" (") + pReg->xloc + "," + pReg->yloc;
	temp += ")";
	temp += AString(" in ") + *pReg->name;
	Awrite(temp);
	*/
	
	if ((Globals->LEADERS_EXIST) || (pReg->type == R_NEXUS)) {
		/* standard Leader-type guards */
		u->SetMen(I_LEADERS,num);
		u->items.SetNum(I_SWORD,num);
		if (IV) u->items.SetNum(I_AMULETOFI,num);
		u->SetMoney(num * Globals->GUARD_MONEY);
		u->SetSkill(S_COMBAT,skilllevel);
		u->SetName(s);
		u->type = U_GUARD;
		u->guard = GUARD_GUARD;
		u->reveal = REVEAL_FACTION;
	} else {
		/* non-leader guards */
		int n = 3 * num / 4;
		int plate = 0;
		if ((AC) && (Globals->START_CITY_GUARDS_PLATE)) plate = 1;
		u = MakeManUnit(pFac, pReg->race, n, skilllevel, 1,
			plate, 0);
		if (IV) u->items.SetNum(I_AMULETOFI,num);
		u->SetMoney(num * Globals->GUARD_MONEY / 2);
		u->SetName(s);
		u->type = U_GUARD;
		u->guard = GUARD_GUARD;
		u->reveal = REVEAL_FACTION;
		u2 = MakeManUnit(pFac, pReg->race, n, skilllevel, 1,
			plate, 1);
		if (IV) u2->items.SetNum(I_AMULETOFI,num);
		u2->SetMoney(num * Globals->GUARD_MONEY / 2);
		AString *un = new AString("City Guard");
		u2->SetName(un);
		u2->type = U_GUARD;
		u2->guard = GUARD_GUARD;
		u2->reveal = REVEAL_FACTION;
	}			
	
	if (AC) {
		if (Globals->START_CITY_GUARDS_PLATE) {
			if (Globals->LEADERS_EXIST) u->items.SetNum(I_PLATEARMOR, num);
		}
		u->SetSkill(S_OBSERVATION,10);
		if (Globals->START_CITY_TACTICS)
			u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
	} else {
		u->SetSkill(S_OBSERVATION, skilllevel);
	}
	u->SetFlag(FLAG_HOLDING,1);
	u->MoveUnit(pReg->GetDummy());
	if ((!Globals->LEADERS_EXIST) && (pReg->type != R_NEXUS)) {
		u2->SetFlag(FLAG_HOLDING,1);
		u2->MoveUnit(pReg->GetDummy());
	}

	if (AC && Globals->START_CITY_MAGES && needmage) {
		u = GetNewUnit(pFac);
		s = new AString("City Mage");
		u->SetName(s);
		u->type = U_GUARDMAGE;
		u->reveal = REVEAL_FACTION;
		u->SetMen(I_LEADERS,1);
		if (IV) u->items.SetNum(I_AMULETOFI,1);
		u->SetMoney(Globals->GUARD_MONEY);
		u->SetSkill(S_FORCE,Globals->START_CITY_MAGES);
		u->SetSkill(S_FIRE,Globals->START_CITY_MAGES);
		if (Globals->START_CITY_TACTICS)
			u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
		u->combat = S_FIRE;
		u->SetFlag(FLAG_BEHIND, 1);
		u->SetFlag(FLAG_HOLDING, 1);
		u->MoveUnit(pReg->GetDummy());
	}
}

void Game::AdjustCityMons(ARegion *r)
{
	int needguard = 1;
	int needmage = 1;
	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->type == U_GUARD || u->type == U_GUARDMAGE) {
				AdjustCityMon(r, u);
				/* Don't create new city guards if we have some */
				needguard = 0;
				if (u->type == U_GUARDMAGE)
					needmage = 0;
			}
			if (u->guard == GUARD_GUARD) needguard = 0;
		}
	}

	if (needguard && (getrandom(100) < Globals->GUARD_REGEN)) {
		CreateCityMon(r, 10, needmage);
	}
}

void Game::AdjustCityMon(ARegion *r, Unit *u)
{
	int towntype;
	int AC = 0;
	int men;
	int IV = 0;
	int mantype;
	int maxmen;
	int weapon = -1;
	int maxweapon = 0;
	int armor = -1;
	int maxarmor = 0;
	for (int i=0; i<NITEMS; i++) {
		int num = u->items.GetNum(i);
		if (num == 0) continue;
		if (ItemDefs[i].type & IT_MAN) mantype = i;
		if ((ItemDefs[i].type & IT_WEAPON)
			&& (num > maxweapon)) {
			weapon = i;
			maxweapon = num;
		}
		if ((ItemDefs[i].type & IT_ARMOR)
			&& (num > maxarmor)) {
			armor = i;	
			maxarmor = num;
		}
	}
	int skill = S_COMBAT;
	
	if (weapon != -1) {
		WeaponType *wp = FindWeapon(ItemDefs[weapon].abr);
		if (FindSkill(wp->baseSkill) == FindSkill("XBOW")) skill = S_CROSSBOW;
		if (FindSkill(wp->baseSkill) == FindSkill("LBOW")) skill = S_LONGBOW;
	}
	
	int sl = u->GetRealSkill(skill);
		
	if (r->type == R_NEXUS || r->IsStartingCity()) {
		towntype = TOWN_CITY;
		AC = 1;
		if (Globals->SAFE_START_CITIES || (r->type == R_NEXUS))
			IV = 1;
		if (u->type == U_GUARDMAGE) {
			men = 1;
		} else {
			maxmen = Globals->AMT_START_CITY_GUARDS;
			if ((!Globals->LEADERS_EXIST) && (r->type != R_NEXUS))
				maxmen = 3 * maxmen / 4;
			men = u->GetMen() + (Globals->AMT_START_CITY_GUARDS/10);
			if (men > maxmen)
				men = maxmen;
		}
	} else {
		towntype = r->town->TownType();
		maxmen = Globals->CITY_GUARD * (towntype+1);
		if (!Globals->LEADERS_EXIST) maxmen = 3 * maxmen / 4;
		men = u->GetMen() + (maxmen/10);
		if (men > maxmen)
			men = maxmen;
	}

	u->SetMen(mantype,men);
	if (IV) u->items.SetNum(I_AMULETOFI,men);

	if (u->type == U_GUARDMAGE) {
		if (Globals->START_CITY_TACTICS)
			u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
		u->SetSkill(S_FORCE, Globals->START_CITY_MAGES);
		u->SetSkill(S_FIRE, Globals->START_CITY_MAGES);
		u->combat = S_FIRE;
		u->SetFlag(FLAG_BEHIND, 1);
		u->SetMoney(Globals->GUARD_MONEY);
	} else {
		int money = men * (Globals->GUARD_MONEY * men / maxmen);
		u->SetMoney(money);
		u->SetSkill(skill, sl);
		if (AC) {
			u->SetSkill(S_OBSERVATION,10);
			if (Globals->START_CITY_TACTICS)
				u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
			if (Globals->START_CITY_GUARDS_PLATE)
				u->items.SetNum(armor,men);
		} else {
			u->SetSkill(S_OBSERVATION,towntype + 1);
		}
		if (weapon!= -1) {
			u->items.SetNum(weapon,men);
		}
	}
}

void Game::Equilibrate()
{
	Awrite("Initialising the economy");
	for (int a=0; a<25; a++) {
		Adot();
		ProcessMigration();
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			r->PostTurn(&regions);
		}
	}
	Awrite("");
}

void Game::WriteTimesArticle(AString article)
{
	string fname;

	do { fname = "times." + to_string(getrandom(10000)); } while (filesystem::exists(fname));
	ofstream f(fname, ios::out | ios::trunc);
	if (f.is_open()) {
		f << indent::wrap(78,70,0) << article << '\n';
	}
}


void Game::CountItems(size_t ** citems)
{
	int i = 0;
	forlist (&factions)
	{
		Faction * fac = (Faction *) elem;
		if (!fac->is_npc)
		{
			for (int j = 0; j < NITEMS; j++)
			{
				citems[i][j] = CountItem (fac, j);
			}
		}
		i++;
	}
}

int Game::CountItem (Faction * fac, int item)
{
	if (ItemDefs[item].type & IT_SHIP) return 0;
	
	size_t all = 0;
	for (const auto& r : fac->present_regions) {
		forlist(&r->objects) {
			Object * obj = (Object *) elem;
			forlist(&obj->units) {
				Unit * unit = (Unit *) elem;
				if (unit->faction == fac)
					all += unit->items.GetNum (item);
			}
		}
	}
	return all;
}
