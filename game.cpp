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
#include <memory.h>  // Needed for memcpy on windows
#endif

#include <string.h>

#include "game.h"
#include "unit.h"
#include "fileio.h"
#include "astring.h"
#include "gamedata.h"

Game::Game()
{
	gameStatus = GAME_STATUS_UNINIT;
	ppUnits = 0;
	maxppunits = 0;
}

Game::~Game()
{
	delete ppUnits;
	ppUnits = 0;
	maxppunits = 0;
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
		if(r->type == R_NEXUS) continue;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->monthorders || u->faction->IsNPC() ||
						(Globals->TAX_PILLAGE_MONTH_LONG &&
						 u->taxing != TAX_NONE))
					continue;
				if(u->GetFlag(FLAG_AUTOTAX) &&
						(Globals->TAX_PILLAGE_MONTH_LONG && u->Taxers(1))) {
					u->taxing = TAX_AUTO;
				} else {
					if(Globals->DEFAULT_WORK_ORDER) ProcessWorkOrder(u, 0);
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

void Game::WriteSurfaceMap(Aoutfile *f, ARegionArray *pArr, int type)
{
	ARegion *reg;
	int yy = 0;
	int xx = 0;

	f->PutStr(AString("Map (") + xx*32 + "," + yy*16 + ")");
	for (int y=0; y < pArr->y; y+=2) {
		AString temp;
		int x;
		for (x=0; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16);
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);
			temp += "  ";
		}
		f->PutStr(temp);
		temp = "  ";
		for (x=1; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16+1);
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);
			temp += "  ";
		}
		f->PutStr(temp);
	}
	f->PutStr("");
}

void Game::WriteUnderworldMap(Aoutfile *f, ARegionArray *pArr, int type)
{
	ARegion *reg, *reg2;
	int xx = 0;
	int yy = 0;
	f->PutStr(AString("Map (") + xx*32 + "," + yy*16 + ")");
	for (int y=0; y< pArr->y; y+=2) {
		AString temp = " ";
		AString temp2;
		int x;
		for (x=0; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16);
			reg2 = pArr->GetRegion(x+xx*32+1,y+yy*16+1);
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);
			if(reg2->neighbors[D_NORTH]) temp += "|";
			else temp += " ";

			temp += " ";
			if (reg->neighbors[D_SOUTHWEST]) temp2 += "/";
			else temp2 += " ";

			temp2 += " ";
			if (reg->neighbors[D_SOUTHEAST]) temp2 += "\\";
			else temp2 += " ";

			temp2 += " ";
		}
		f->PutStr(temp);
		f->PutStr(temp2);

		temp = " ";
		temp2 = "  ";
		for (x=1; x< pArr->x; x+=2) {
			reg = pArr->GetRegion(x+xx*32,y+yy*16+1);
			reg2 = pArr->GetRegion(x+xx*32-1,y+yy*16);

			if (reg2->neighbors[D_SOUTH]) temp += "|";
			else temp += " ";

			temp += AString(" ");
			temp += AString(GetRChar(reg));
			temp += GetXtraMap(reg,type);

			if (reg->neighbors[D_SOUTHWEST]) temp2 += "/";
			else temp2 += " ";

			temp2 += " ";
			if (reg->neighbors[D_SOUTHEAST]) temp2 += "\\";
			else temp2 += " ";

			temp2 += " ";
		}
		f->PutStr(temp);
		f->PutStr(temp2);
	}
	f->PutStr("");
}

int Game::ViewMap(const AString & typestr,const AString & mapfile)
{
	int type = 0;
	if (AString(typestr) == "wmon") type = 1;
	if (AString(typestr) == "lair") type = 2;
	if (AString(typestr) == "gate") type = 3;

	Aoutfile f;
	if(f.OpenByName(mapfile) == -1) return(0);

	switch (type) {
		case 0:
			f.PutStr("Geographical Map");
			break;
		case 1:
			f.PutStr("Wandering Monster Map");
			break;
		case 2:
			f.PutStr("Lair Map");
			break;
		case 3:
			f.PutStr("Gate Map");
			break;
	}

	int i;
	for(i = 0; i < regions.numLevels; i++) {
		f.PutStr("");
		ARegionArray *pArr = regions.pRegionArrays[i];
		switch(pArr->levelType) {
			case ARegionArray::LEVEL_NEXUS:
				f.PutStr(AString("Level ") + i + ": Nexus");
				break;
			case ARegionArray::LEVEL_SURFACE:
				f.PutStr(AString("Level ") + i + ": Surface");
				WriteSurfaceMap(&f, pArr, type);
				break;
			case ARegionArray::LEVEL_UNDERWORLD:
				f.PutStr(AString("Level ") + i + ": Underworld");
				WriteUnderworldMap(&f, pArr, type);
				break;
			case ARegionArray::LEVEL_UNDERDEEP:
				f.PutStr(AString("Level ") + i + ": Underdeep");
				WriteUnderworldMap(&f, pArr, type);
				break;
		}
	}

	f.Close();

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
	seedrandomrandom();

	CreateWorld();
	CreateNPCFactions();

	if(Globals->CITY_MONSTERS_EXIST)
		CreateCityMons();
	if(Globals->WANDERING_MONSTERS_EXIST)
		CreateWMons();
	if(Globals->LAIR_MONSTERS_EXIST)
		CreateLMons();

	if(Globals->LAIR_MONSTERS_EXIST)
		CreateVMons();
		
	if(Globals->PLAYER_ECONOMY) {
		DevelopTowns();
		Equilibrate();
	}

	return(1);
}

int Game::OpenGame()
{
	//
	// The order here must match the order in SaveGame
	//
	Ainfile f;
	if(f.OpenByName("game.in") == -1) return(0);

	//
	// Read in Globals
	//
	AString *s1 = f.GetStr();
	if(!s1) return(0);

	AString *s2 = s1->gettoken();
	delete s1;
	if(!s2) return(0);

	if (!(*s2 == "atlantis_game")) {
		delete s2;
		f.Close();
		return(0);
	}
	delete s2;

	ATL_VER eVersion = f.GetInt();
	Awrite(AString("Saved Game Engine Version: ") + ATL_VER_STRING(eVersion));
	if(ATL_VER_MAJOR(eVersion) != ATL_VER_MAJOR(CURRENT_ATL_VER) ||
			ATL_VER_MINOR(eVersion) != ATL_VER_MINOR(CURRENT_ATL_VER)) {
		Awrite("Incompatible Engine versions!");
		return(0);
	}
	if(ATL_VER_PATCH(eVersion) > ATL_VER_PATCH(CURRENT_ATL_VER)) {
		Awrite("This game was created with a more recent Atlantis Engine!");
		return(0);
	}

	AString *gameName = f.GetStr();
	if(!gameName) return(0);

	if(!(*gameName == Globals->RULESET_NAME)) {
		Awrite("Incompatible rule-set!");
		return(0);
	}

	ATL_VER gVersion = f.GetInt();
	Awrite(AString("Saved Rule-Set Version: ") + ATL_VER_STRING(gVersion));

	if(ATL_VER_MAJOR(gVersion) < ATL_VER_MAJOR(Globals->RULESET_VERSION)) {
		Awrite(AString("Upgrading to ") +
				ATL_VER_STRING(MAKE_ATL_VER(
						ATL_VER_MAJOR(Globals->RULESET_VERSION), 0, 0)));
		if (!UpgradeMajorVersion(gVersion)) {
			Awrite("Unable to upgrade!  Aborting!");
			return(0);
		}
		gVersion = MAKE_ATL_VER(ATL_VER_MAJOR(Globals->RULESET_VERSION), 0, 0);
	}
	if(ATL_VER_MINOR(gVersion) < ATL_VER_MINOR(Globals->RULESET_VERSION)) {
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
	if(ATL_VER_PATCH(gVersion) < ATL_VER_PATCH(Globals->RULESET_VERSION)) {
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

	year = f.GetInt();
	month = f.GetInt();
	seedrandom(f.GetInt());
	factionseq = f.GetInt();
	unitseq = f.GetInt();
	shipseq = f.GetInt();
	guardfaction = f.GetInt();
	monfaction = f.GetInt();

	//
	// Read in the Factions
	//
	int i = f.GetInt();

	for (int j=0; j<i; j++) {
		Faction *temp = new Faction;
		temp->Readin(&f, eVersion);
		factions.Add(temp);
	}

	//
	// Read in the ARegions
	//
	i = regions.ReadRegions(&f, &factions, eVersion);
	if(!i) return 0;

	SetupUnitNums();

	f.Close();
	return(1);
}

int Game::SaveGame()
{
	Aoutfile f;
	if(f.OpenByName("game.out") == -1) return(0);

	//
	// Write out Globals
	//
	f.PutStr("atlantis_game");
	f.PutInt(CURRENT_ATL_VER);
	f.PutStr(Globals->RULESET_NAME);
	f.PutInt(Globals->RULESET_VERSION);

	f.PutInt(year);
	f.PutInt(month);
	f.PutInt(getrandom(10000));
	f.PutInt(factionseq);
	f.PutInt(unitseq);
	f.PutInt(shipseq);
	f.PutInt(guardfaction);
	f.PutInt(monfaction);

	//
	// Write out the Factions
	//
	f.PutInt(factions.Num());

	forlist(&factions) {
		((Faction *) elem)->Writeout(&f);
	}

	//
	// Write out the ARegions
	//
	regions.WriteRegions(&f);

	f.Close();
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
	Aoutfile f;
	if(f.OpenByName("players.out") == -1) return(0);

	f.PutStr(PLAYERS_FIRST_LINE);
	f.PutStr(AString("Version: ") + CURRENT_ATL_VER);
	f.PutStr(AString("TurnNumber: ") + TurnNumber());

	if(gameStatus == GAME_STATUS_UNINIT)
		return(0);
	else if(gameStatus == GAME_STATUS_NEW)
		f.PutStr(AString("GameStatus: New"));
	else if(gameStatus == GAME_STATUS_RUNNING)
		f.PutStr(AString("GameStatus: Running"));
	else if(gameStatus == GAME_STATUS_FINISHED)
		f.PutStr(AString("GameStatus: Finished"));

	f.PutStr("");

	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		fac->WriteFacInfo(&f);
	}

	f.Close();
	return(1);
}

int Game::ReadPlayers()
{
	Aorders f;
	if(f.OpenByName("players.in") == -1) return(0);

	AString *pLine = 0;
	AString *pToken = 0;

	//
	// Default: failure.
	//
	int rc = 0;

	do {
		//
		// The first line of the file should match.
		//
		pLine = f.GetLine();
		if(!(*pLine == PLAYERS_FIRST_LINE)) break;
		SAFE_DELETE(pLine);

		//
		// Get the file version number.
		//
		pLine = f.GetLine();
		pToken = pLine->gettoken();
		if(!pToken || !(*pToken == "Version:")) break;
		SAFE_DELETE(pToken);

		pToken = pLine->gettoken();
		if(!pToken) break;

		int nVer = pToken->value();
		if(ATL_VER_MAJOR(nVer) != ATL_VER_MAJOR(CURRENT_ATL_VER) ||
				ATL_VER_MINOR(nVer) != ATL_VER_MINOR(CURRENT_ATL_VER) ||
				ATL_VER_PATCH(nVer) > ATL_VER_PATCH(CURRENT_ATL_VER)) {
			Awrite("The players.in file is not compatible with this "
					"version of Atlantis.");
			break;
		}
		SAFE_DELETE(pToken);
		SAFE_DELETE(pLine);

		//
		// Ignore the turn number line.
		//
		pLine = f.GetLine();
		SAFE_DELETE(pLine);

		//
		// Next, the game status.
		//
		pLine = f.GetLine();
		pToken = pLine->gettoken();
		if(!pToken || !(*pToken == "GameStatus:")) break;
		SAFE_DELETE(pToken);

		pToken = pLine->gettoken();
		if(!pToken) break;

		if(*pToken == "New")
			gameStatus = GAME_STATUS_NEW;
		else if(*pToken == "Running")
			gameStatus = GAME_STATUS_RUNNING;
		else if(*pToken == "Finished")
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
		pLine = f.GetLine();
		Faction *pFac = 0;

		int lastWasNew = 0;

		//
		// OK, set our return code to success; we'll set it to fail below
		// if necessary.
		//
		rc = 1;

		while(pLine) {
			pToken = pLine->gettoken();
			if(!pToken) {
				SAFE_DELETE(pLine);
				pLine = f.GetLine();
				continue;
			}

			if(*pToken == "Faction:") {
				//
				// Get the new faction
				//
				SAFE_DELETE(pToken);
				pToken = pLine->gettoken();
				if(!pToken) {
					rc = 0;
					break;
				}

				if(*pToken == "new") {
					AString save = *pLine;
					int noleader = 0;
					int x, y, z;
					ARegion *pReg = NULL;

					/* Check for the noleader flag */
					SAFE_DELETE(pToken);
					pToken = pLine->gettoken();
					if (pToken && *pToken == "noleader") {
						noleader = 1;
						SAFE_DELETE(pToken);
						pToken = pLine->gettoken();
						/* Initialize pReg to something useful */
						pReg = regions.GetRegion(0, 0, 0);
					}
					if (pToken) {
						x = pToken->value();
						y = -1;
						z = -1;
						SAFE_DELETE(pToken);
						pToken = pLine->gettoken();
						if (pToken) {
							y = pToken->value();
							SAFE_DELETE(pToken);
							pToken = pLine->gettoken();
							if (pToken) {
								z = pToken->value();
								pReg = regions.GetRegion(x, y, z);
							}
						}
						if (pReg == NULL)
							Awrite(AString("Bad faction line: ")+save);
					}

					pFac = AddFaction(noleader, pReg);
					if(!pFac) {
						Awrite("Failed to add a new faction!");
						rc = 0;
						break;
					}

					lastWasNew = 1;
				} else {
					if(pFac && lastWasNew) {
						WriteNewFac(pFac);
					}
					int nFacNum = pToken->value();
					pFac = GetFaction(&factions, nFacNum);
					lastWasNew = 0;
				}
			} else if(pFac) {
				if(!ReadPlayersLine(pToken, pLine, pFac, lastWasNew)) {
					rc = 0;
					break;
				}
			}

			SAFE_DELETE(pToken);
			SAFE_DELETE(pLine);
			pLine = f.GetLine();
		}
		if(pFac && lastWasNew) {
			WriteNewFac(pFac);
		}
	} while(0);

	SAFE_DELETE(pLine);
	SAFE_DELETE(pToken);
	f.Close();

	return(rc);
}

Unit *Game::ParseGMUnit(AString *tag, Faction *pFac)
{
	char *str = tag->Str();
	if(*str == 'g' && *(str+1) == 'm') {
		AString p = AString(str+2);
		int gma = p.value();
		forlist(&regions) {
			ARegion *reg = (ARegion *)elem;
			forlist(&reg->objects) {
				Object *obj = (Object *)elem;
				forlist(&obj->units) {
					Unit *u = (Unit *)elem;
					if(u->faction->num == pFac->num && u->gm_alias == gma) {
						return u;
					}
				}
			}
		}
	} else {
		int v = tag->value();
		if((unsigned int)v >= maxppunits) return NULL;
		return GetUnit(v);
	}
	return NULL;
}

int Game::ReadPlayersLine(AString *pToken, AString *pLine, Faction *pFac,
		int newPlayer)
{
	AString *pTemp = 0;

	if(*pToken == "Name:") {
		pTemp = pLine->StripWhite();
		if(pTemp) {
			if(newPlayer) {
				*pTemp += AString(" (") + (pFac->num) + ")";
			}
			pFac->SetNameNoChange(pTemp);
		}
	} else if(*pToken == "RewardTimes") {
		pFac->TimesReward();
	} else if(*pToken == "Email:") {
		pTemp = pLine->gettoken();
		if(pTemp) {
			delete pFac->address;
			pFac->address = pTemp;
			pTemp = 0;
		}
	} else if(*pToken == "Password:") {
		pTemp = pLine->StripWhite();
		delete pFac->password;
		if(pTemp) {
			pFac->password = pTemp;
			pTemp = 0;
		} else {
			pFac->password = 0;
		}
	} else if(*pToken == "Template:") {
		// LLS - looked like a good place to stick the Template test
		pTemp = pLine->gettoken();
		int nTemp = ParseTemplate(pTemp);
		pFac->temformat = TEMPLATE_LONG;
		if (nTemp != -1) pFac->temformat = nTemp;
	} else if(*pToken == "Reward:") {
		pTemp = pLine->gettoken();
		int nAmt = pTemp->value();
		pFac->Event(AString("Reward of ") + nAmt + " silver.");
		pFac->unclaimed += nAmt;
	} else if(*pToken == "SendTimes:") {
		// get the token, but otherwise ignore it
		pTemp = pLine->gettoken();
		pFac->times = pTemp->value();
	} else if (*pToken == "LastOrders:") {
		// Read this line and correctly set the lastorders for this
		// faction if the game itself isn't maintaining them.
		pTemp = pLine->gettoken();
		if(Globals->LASTORDERS_MAINTAINED_BY_SCRIPTS)
			pFac->lastorders = pTemp->value();
	} else if(*pToken == "Loc:") {
		int x, y, z;
		pTemp = pLine->gettoken();
		if(pTemp) {
			x = pTemp->value();
			delete pTemp;
			pTemp = pLine->gettoken();
			if(pTemp) {
				y = pTemp->value();
				delete pTemp;
				pTemp = pLine->gettoken();
				if(pTemp) {
					z = pTemp->value();
					ARegion *pReg = regions.GetRegion(x, y, z);
					if(pReg) {
						pFac->pReg = pReg;
					} else {
						Awrite(AString("Invalid Loc:")+x+","+y+","+z+
								" in faction " + pFac->num);
						pFac->pReg = NULL;
					}
				}
			}
		}
	} else if(*pToken == "NewUnit:") {
		// Creates a new unit in the location specified by a Loc: line
		// with a gm_alias of whatever is after the NewUnit: tag.
		if(!pFac->pReg) {
			Awrite(AString("NewUnit is not valid without a Loc: ") +
					"for faction "+ pFac->num);
		} else {
			pTemp = pLine->gettoken();
			if(!pTemp) {
				Awrite(AString("NewUnit: must be followed by an alias ") +
						"in faction "+pFac->num);
			} else {
				int val = pTemp->value();
				if(!val) {
					Awrite(AString("NewUnit: must be followed by an alias ") +
							"in faction "+pFac->num);
				} else {
					Unit *u = GetNewUnit(pFac);
					u->gm_alias = val;
					u->MoveUnit(pFac->pReg->GetDummy());
					u->Event("Is given to your faction.");
				}
			}
		}
	} else if(*pToken == "Item:") {
		pTemp = pLine->gettoken();
		if(!pTemp) {
			Awrite(AString("Item: needs to specify a unit in faction ") +
					pFac->num);
		} else {
			Unit *u = ParseGMUnit(pTemp, pFac);
			if(!u) {
				Awrite(AString("Item: needs to specify a unit in faction ") +
						pFac->num);
			} else {
				if(u->faction->num != pFac->num) {
					Awrite(AString("Item: unit ")+ u->num +
							" doesn't belong to " + "faction " + pFac->num);
				} else {
					delete pTemp;
					pTemp = pLine->gettoken();
					if(!pTemp) {
						Awrite(AString("Must specify a number of items to ") +
								"give for Item: in faction " + pFac->num);
					} else {
						int v = pTemp->value();
						if(!v) {
							Awrite(AString("Must specify a number of ") +
										"items to give for Item: in " +
										"faction " + pFac->num);
						} else {
							delete pTemp;
							pTemp = pLine->gettoken();
							if(!pTemp) {
								Awrite(AString("Must specify a valid item ") +
										"to give for Item: in faction " +
										pFac->num);
							} else {
								int it = ParseAllItems(pTemp);
								if(it == -1) {
									Awrite(AString("Must specify a valid ") +
											"item to give for Item: in " +
											"faction " + pFac->num);
								} else {
									int has = u->items.GetNum(it);
									u->items.SetNum(it, has + v);
									if(!u->gm_alias) {
										u->Event(AString("Is given ") +
												ItemString(it, v) +
												" by the gods.");
									}
									u->faction->DiscoverItem(it, 0, 1);
								}
							}
						}
					}
				}
			}
		}
	} else if(*pToken == "Skill:") {
		pTemp = pLine->gettoken();
		if(!pTemp) {
			Awrite(AString("Skill: needs to specify a unit in faction ") +
					pFac->num);
		} else {
			Unit *u = ParseGMUnit(pTemp, pFac);
			if(!u) {
				Awrite(AString("Skill: needs to specify a unit in faction ") +
						pFac->num);
			} else {
				if(u->faction->num != pFac->num) {
					Awrite(AString("Skill: unit ")+ u->num +
							" doesn't belong to " + "faction " + pFac->num);
				} else {
					delete pTemp;
					pTemp = pLine->gettoken();
					if(!pTemp) {
						Awrite(AString("Must specify a valid skill for ") +
								"Skill: in faction " + pFac->num);
					} else {
						int sk = ParseSkill(pTemp);
						if(sk == -1) {
							Awrite(AString("Must specify a valid skill for ")+
									"Skill: in faction " + pFac->num);
						} else {
							delete pTemp;
							pTemp = pLine->gettoken();
							if(!pTemp) {
								Awrite(AString("Must specify a days for ") +
										"Skill: in faction " + pFac->num);
							} else {
								int days = pTemp->value() * u->GetMen();
								if(!days) {
									Awrite(AString("Must specify a days for ")+
											"Skill: in faction " + pFac->num);
								} else {
									int odays = u->skills.GetDays(sk);
									u->skills.SetDays(sk, odays + days);
									u->AdjustSkills();
									int lvl = u->GetRealSkill(sk);
									if(lvl > pFac->skills.GetDays(sk)) {
										pFac->skills.SetDays(sk, lvl);
										pFac->shows.Add(new ShowSkill(sk,lvl));
									}
									if(!u->gm_alias) {
										u->Event(AString("Is taught ") +
												days + " days of " +
												SkillStrs(sk) +
												" by the gods.");
									}
									/*
									 * This is NOT quite the same, but the gods
									 * are more powerful than mere mortals
									 */
									int mage = (SkillDefs[sk].flags &
											SkillType::MAGIC);
									int app = (SkillDefs[sk].flags &
											SkillType::APPRENTICE);
									if(mage) {
										u->type = U_MAGE;
									}
									if(app && u->type == U_NORMAL) {
										u->type = U_APPRENTICE;
									}
								}
							}
						}
					}
				}
			}
		}
	} else if(*pToken == "Order:") {
		pTemp = pLine->StripWhite();
		if(*pTemp == "quit") {
			pFac->quit = QUIT_BY_GM;
		} else {
			// handle this as a unit order
			delete pTemp;
			pTemp = pLine->gettoken();
			if(!pTemp) {
				Awrite(AString("Order: needs to specify a unit in faction ") +
						pFac->num);
			} else {
				Unit *u = ParseGMUnit(pTemp, pFac);
				if(!u) {
					Awrite(AString("Order: needs to specify a unit in ")+
							"faction " + pFac->num);
				} else {
					if(u->faction->num != pFac->num) {
						Awrite(AString("Order: unit ")+ u->num +
								" doesn't belong to " + "faction " +
								pFac->num);
					} else {
						delete pTemp;
						AString saveorder = *pLine;
						int getatsign = pLine->getat();
						pTemp = pLine->gettoken();
						if(!pTemp) {
							Awrite(AString("Order: must provide unit order ")+
									"for faction "+pFac->num);
						} else {
							int o = Parse1Order(pTemp);
							if(o == -1 || o == O_ATLANTIS || o == O_END ||
									o == O_UNIT || o == O_FORM ||
									o == O_ENDFORM) {
								Awrite(AString("Order: invalid order given ")+
										"for faction "+pFac->num);
							} else {
								if(getatsign) {
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
		pTemp = new AString(*pToken + *pLine);
		pFac->extraPlayers.Add(pTemp);
		pTemp = 0;
	}

	if(pTemp) delete pTemp;
	return(1);
}

void Game::WriteNewFac(Faction *pFac)
{
	AString *strFac = new AString(AString("Adding ") + *(pFac->address) + ".");
	newfactions.Add(strFac);
}

int Game::DoOrdersCheck(const AString &strOrders, const AString &strCheck)
{
	Aorders ordersFile;
	if(ordersFile.OpenByName(strOrders) == -1) {
		Awrite("No such orders file!");
		return(0);
	}

	Aoutfile checkFile;
	if(checkFile.OpenByName(strCheck) == -1) {
		Awrite("Couldn't open the orders check file!");
		return(0);
	}

	OrdersCheck check;
	check.pCheckFile = &checkFile;

	ParseOrders(0, &ordersFile, &check);

	ordersFile.Close();
	checkFile.Close();

	return(1);
}

int Game::RunGame()
{
	Awrite("Setting Up Turn...");
	PreProcessTurn();

	Awrite("Reading the Gamemaster File...");
	if(!ReadPlayers()) return(0);

	if(gameStatus == GAME_STATUS_FINISHED) {
		Awrite("This game is finished!");
		return(0);
	}
	gameStatus = GAME_STATUS_RUNNING;

	Awrite("Reading the Orders File...");
	ReadOrders();

	if(Globals->MAX_INACTIVE_TURNS != -1) {
		Awrite("QUITting Inactive Factions...");
		RemoveInactiveFactions();
	}

	Awrite("Running the Turn...");
	RunOrders();

	Awrite("Writing the Report File...");
	WriteReport();
	Awrite("");
	// LLS - write order templates
	Awrite("Writing order templates...");
	WriteTemplates();
	Awrite("");
	battles.DeleteAll();

	Awrite("Writing Playerinfo File...");
	WritePlayers();

	Awrite("Removing Dead Factions...");
	DeleteDeadFactions();
	Awrite("done");

	return(1);
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
	{
		forlist(&regions) {
			ARegion *pReg = (ARegion *) elem;
			if(Globals->WEATHER_EXISTS)
				pReg->SetWeather(regions.GetWeather(pReg, month));
			if(Globals->GATES_NOT_PERENNIAL)
				pReg->SetGateStatus(month);
			pReg->DefaultOrders();
		}
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
		if(!fac->IsNPC()) {
			AString str = "orders.";
			str += fac->num;

			Aorders file;
			if(file.OpenByName(str) != -1) {
				ParseOrders(fac->num, &file, 0);
				file.Close();
			}
			DefaultWorkOrder();
		}
	}
}

void Game::MakeFactionReportLists()
{
	FactionVector vector(factionseq);

	forlist(&regions) {
		vector.ClearVector();

		ARegion *reg = (ARegion *) elem;
		forlist(&reg->farsees) {
			Faction *fac = ((Farsight *) elem)->faction;
			vector.SetFaction(fac->num, fac);
		}
		{
			forlist(&reg->passers) {
				Faction *fac = ((Farsight *)elem)->faction;
				vector.SetFaction(fac->num, fac);
			}
		}
		{
			forlist(&reg->objects) {
				Object *obj = (Object *) elem;

				forlist(&obj->units) {
					Unit *unit = (Unit *) elem;
					vector.SetFaction(unit->faction->num, unit->faction);
				}
			}
		}

		for (int i=0; i<vector.vectorsize; i++) {
			if (vector.GetFaction(i)) {
				ARegionPtr *ptr = new ARegionPtr;
				ptr->ptr = reg;
				vector.GetFaction(i)->present_regions.Add(ptr);
			}
		}
	}
}

void Game::WriteReport()
{
	Areport f;

	MakeFactionReportLists();
	CountAllSpecialists();
	/*
	CountAllMages();
	if(Globals->APPRENTICES_EXIST)
		CountAllApprentices();
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT)
		CountAllQuarterMasters();
	if (Globals->TACTICS_NEEDS_WAR)
		CountAllTacticians();
	*/
	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		AString str = "report.";
		str = str + fac->num;

		if(!fac->IsNPC() ||
		   ((((month == 0) && (year == 1)) || Globals->GM_REPORT) &&
			(fac->num == 1))) {
			int i = f.OpenByName(str);
			if(i != -1) {
				fac->WriteReport(&f, this);
				f.Close();
			}
		}
		Adot();
	}
}

// LLS - write order templates for factions
void Game::WriteTemplates()
{
	Areport f;

	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		AString str = "template.";
		str = str + fac->num;

		if(!fac->IsNPC()) {
			int i = f.OpenByName(str);
			if (i != -1) {
				fac->WriteTemplate(&f, this);
				f.Close();
			}
			fac->present_regions.DeleteAll();
		}
		Adot();
	}
}


void Game::DeleteDeadFactions()
{
	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		if (!fac->IsNPC() && !fac->exists) {
			factions.Remove(fac);
			forlist((&factions))
				((Faction *) elem)->RemoveAttitude(fac->num);
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
				if(u && u->num > max) max = u->num;
			}
		}
	}
	unitseq = max+1;
}

void Game::SetupUnitNums()
{
	if(ppUnits) delete ppUnits;

	SetupUnitSeq();

	maxppunits = unitseq+10000;

	ppUnits = new Unit *[maxppunits];

	unsigned int i;
	for(i = 0; i < maxppunits ; i++) ppUnits[i] = 0;

	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				i = u->num;
				if((i > 0) && (i < maxppunits)) {
					if(!ppUnits[i])
						ppUnits[u->num] = u;
					else {
						Awrite(AString("Error: Unit number ") + i +
								" multiply defined.");
						if((unitseq > 0) && (unitseq < maxppunits)) {
							u->num = unitseq;
							ppUnits[unitseq++] = u;
						}
					}
				} else {
					Awrite(AString("Error: Unit number ")+i+
							" out of range.");
					if((unitseq > 0) && (unitseq < maxppunits)) {
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
	for(i = 1; i < unitseq; i++) {
		if(!ppUnits[i]) {
			Unit *pUnit = new Unit(i, fac, an);
			ppUnits[i] = pUnit;
			return(pUnit);
		}
	}

	Unit *pUnit = new Unit(unitseq, fac, an);
	ppUnits[unitseq] = pUnit;
	unitseq++;
	if(unitseq >= maxppunits) {
		Unit **temp = new Unit*[maxppunits+10000];
		memcpy(temp, ppUnits, maxppunits*sizeof(Unit *));
		maxppunits += 10000;
		delete ppUnits;
		ppUnits = temp;
	}

	return(pUnit);
}

Unit *Game::GetUnit(int num)
{
	if(num < 0 || (unsigned int)num >= maxppunits) return NULL;
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


/*
void Game::CountAllMages()
{
	forlist(&factions) {
		((Faction *) elem)->nummages = 0;
	}

	{
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			forlist(&r->objects) {
				Object *o = (Object *) elem;
				forlist(&o->units) {
					Unit *u = (Unit *) elem;
					if (u->type == U_MAGE) u->faction->nummages++;
				}
			}
		}
	}
}

void Game::CountAllQuarterMasters()
{
	forlist(&factions) {
		((Faction *) elem)->numqms = 0;
	}

	{
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			forlist(&r->objects) {
				Object *o = (Object *) elem;
				forlist(&o->units) {
					Unit *u = (Unit *) elem;
					if (u->GetSkill(S_QUARTERMASTER))
						u->faction->numqms++;
				}
			}
		}
	}
}

// This, along with counting apprentices, mages and quartermasters, 
// should all be in the one function (CountSpecialists?)
void Game::CountAllTacticians()
{
	forlist(&factions) {
		((Faction *) elem)->numtacts = 0;
	}

	{
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			forlist(&r->objects) {
				Object *o = (Object *) elem;
				forlist(&o->units) {
					Unit *u = (Unit *) elem;
					if (u->GetSkill(S_TACTICS) == 5)
						u->faction->numtacts++;
				}
			}
		}
	}
}
*/

// LLS
void Game::UnitFactionMap()
{
	Aoutfile f;
	unsigned int i;
	Unit *u;

	Awrite("Opening units.txt");
	if (f.OpenByName("units.txt") == -1) {
		Awrite("Couldn't open file!");
	} else {
		Awrite(AString("Writing ") + unitseq + " units");
		for (i = 1; i < unitseq; i++) {
			u = GetUnit(i);
			if (!u) {
			  Awrite("doesn't exist");
			} else {
			  Awrite(AString(i) + ":" + u->faction->num);
			  f.PutStr(AString(i) + ":" + u->faction->num);
			}
		}
	}
	f.Close();
}


//The following function added by Creative PBM February 2000
void Game::RemoveInactiveFactions()
{
	if(Globals->MAX_INACTIVE_TURNS == -1) return;

	int cturn;
	cturn = TurnNumber();
	forlist(&factions) {
		Faction *fac = (Faction *) elem;
		if ((cturn - fac->lastorders) >= Globals->MAX_INACTIVE_TURNS &&
				!fac->IsNPC()) {
			fac->quit = QUIT_BY_GM;
		}
	}
}

/*
void Game::CountAllApprentices()
{
	if(!Globals->APPRENTICES_EXIST) return;

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
					if(u->type == U_APPRENTICE)
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
				if(u->faction == pFac && u->GetSkill(S_QUARTERMASTER)) i++;
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
				if(u->faction == pFac && u->GetSkill(S_TACTICS) == 5) i++;
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
				if(u->faction == pFac && u->type == U_APPRENTICE) i++;
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
	int points = pFac->type[F_TRADE];

	if (points < 0) points = 0;
	if (points > allowedQuartermastersSize - 1)
		points = allowedQuartermastersSize - 1;

	return allowedQuartermasters[points];
}

int Game::AllowedTacticians(Faction *pFac)
{
	int points = pFac->type[F_WAR];

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
	int points = pFac->type[F_WAR];

	if (points < 0) points = 0;
	if (points > allowedTaxesSize - 1) points = allowedTaxesSize - 1;

	return allowedTaxes[points];
}

int Game::AllowedTrades(Faction *pFac)
{
	int points = pFac->type[F_TRADE];

	if (points < 0) points = 0;
	if (points > allowedTradesSize - 1) points = allowedTradesSize - 1;

	return allowedTrades[points];
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
	if(Globals->CHECK_MONSTER_CONTROL_MID_TURN) MonsterCheck(r, u);
}

void Game::PostProcessUnitExtra(ARegion *r, Unit *u)
{
	if(!Globals->CHECK_MONSTER_CONTROL_MID_TURN) MonsterCheck(r, u);
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
			if(!i->num) continue;
			if (!ItemDefs[i->type].escape) continue;

			// Okay, check flat loss.
			if (ItemDefs[i->type].escape & ItemType::LOSS_CHANCE) {
				int losses = (i->num +
						getrandom(ItemDefs[i->type].esc_val)) /
					ItemDefs[i->type].esc_val;
				u->items.SetNum(i->type,i->num - losses);
				u->Event(ItemString(i->type, losses) +
						" decay into nothingness.");
			} else if (ItemDefs[i->type].escape & ItemType::HAS_SKILL) {
				tmp = ItemDefs[i->type].esc_skill;
				skill = LookupSkill(&tmp);
				if (u->GetSkill(skill) < ItemDefs[i->type].esc_val) {
					if(Globals->WANDERING_MONSTERS_EXIST) {
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
					u->Event(AString("Loses control of ") +
							ItemString(i->type, i->num) + ".");
					u->items.SetNum(i->type, 0);
				}
			} else {
				// ESC_LEV_SQUARED or ESC_LEV_QUAD
				tmp = ItemDefs[i->type].esc_skill;
				skill = LookupSkill(&tmp);
				int level = u->GetSkill(skill);
				int chance;

				if (!level) chance = 10000;
				else {
					int top = i->num * i->num;
					int bottom = 0;
					if (ItemDefs[i->type].escape & ItemType::ESC_LEV_SQUARE)
						bottom = level * level;
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
					if(Globals->WANDERING_MONSTERS_EXIST) {
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
					u->Event(AString("Loses control of ") +
							ItemString(i->type, i->num) + ".");
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
						if(Globals->WANDERING_MONSTERS_EXIST) {
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
						u->Event(AString("Loses control of ") +
								ItemString(it->type, it->num) + ".");
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
		return '#';
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
	if(Globals->CITY_MONSTERS_EXIST) {
		f = new Faction(factionseq++);
		guardfaction = f->num;
		temp = new AString("The Guardsmen");
		f->SetName(temp);
		f->SetNPC();
		f->lastorders = 0;
		factions.Add(f);
	} else
		guardfaction = 0;
	// Only create the monster faction if wandering monsters or lair
	// monsters exist.
	if(Globals->LAIR_MONSTERS_EXIST || Globals->WANDERING_MONSTERS_EXIST) {
		f = new Faction(factionseq++);
		monfaction = f->num;
		temp = new AString("Creatures");
		f->SetName(temp);
		f->SetNPC();
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
	if(pReg->type == R_NEXUS || pReg->IsStartingCity()) {
		skilllevel = TOWN_CITY + 1;
		if(Globals->SAFE_START_CITIES || (pReg->type == R_NEXUS))
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
	
	if((Globals->LEADERS_EXIST) || (pReg->type == R_NEXUS)) {
		/* standard Leader-type guards */
		u->SetMen(I_LEADERS,num);
		u->items.SetNum(I_SWORD,num);
		if (IV) u->items.SetNum(I_AMULETOFI,num);
		u->SetMoney(num * Globals->GUARD_MONEY);
		u->SetSkill(S_COMBAT,skilllevel);
		u->SetName(s);
		u->type = U_GUARD;
		u->guard = GUARD_GUARD;
	} else {
		/* non-leader guards */
		int n = 3 * num / 4;
		int plate = 0;
		if((AC) && (Globals->START_CITY_GUARDS_PLATE)) plate = 1;
		u = MakeManUnit(pFac, pReg->race, n, skilllevel, 1,
			plate, 0);
		if (IV) u->items.SetNum(I_AMULETOFI,num);
		u->SetMoney(num * Globals->GUARD_MONEY / 2);
		u->SetName(s);
		u->type = U_GUARD;
		u->guard = GUARD_GUARD;
		u2 = MakeManUnit(pFac, pReg->race, n, skilllevel, 1,
			plate, 1);
		if (IV) u2->items.SetNum(I_AMULETOFI,num);
		u2->SetMoney(num * Globals->GUARD_MONEY / 2);
		AString *un = new AString("City Guard");
		u2->SetName(un);
		u2->type = U_GUARD;
		u2->guard = GUARD_GUARD;
	}			
	
	if (AC) {
		if(Globals->START_CITY_GUARDS_PLATE) {
			if(Globals->LEADERS_EXIST) u->items.SetNum(I_PLATEARMOR, num);
		}
		u->SetSkill(S_OBSERVATION,10);
		if(Globals->START_CITY_TACTICS)
			u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
	} else {
		u->SetSkill(S_OBSERVATION, skilllevel);
	}
	u->SetFlag(FLAG_HOLDING,1);
	u->MoveUnit(pReg->GetDummy());
	if((!Globals->LEADERS_EXIST) && (pReg->type != R_NEXUS)) {
		u2->SetFlag(FLAG_HOLDING,1);
		u2->MoveUnit(pReg->GetDummy());
	}

	if(AC && Globals->START_CITY_MAGES && needmage) {
		u = GetNewUnit(pFac);
		s = new AString("City Mage");
		u->SetName(s);
		u->type = U_GUARDMAGE;
		u->SetMen(I_LEADERS,1);
		if(IV) u->items.SetNum(I_AMULETOFI,1);
		u->SetMoney(Globals->GUARD_MONEY);
		u->SetSkill(S_FORCE,Globals->START_CITY_MAGES);
		u->SetSkill(S_FIRE,Globals->START_CITY_MAGES);
		if(Globals->START_CITY_TACTICS)
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
				if(u->type == U_GUARDMAGE)
					needmage = 0;
			}
			if(u->guard == GUARD_GUARD) needguard = 0;
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
	for(int i=0; i<NITEMS; i++) {
		int num = u->items.GetNum(i);
		if(num == 0) continue;
		if(ItemDefs[i].type & IT_MAN) mantype = i;
		if((ItemDefs[i].type & IT_WEAPON)
			&& (num > maxweapon)) {
			weapon = i;
			maxweapon = num;
		}
		if((ItemDefs[i].type & IT_ARMOR)
			&& (num > maxarmor)) {
			armor = i;	
			maxarmor = num;
		}
	}
	int skill = S_COMBAT;
	
	if (weapon != -1) {
		WeaponType *wp = FindWeapon(ItemDefs[weapon].abr);
		if(FindSkill(wp->baseSkill) == FindSkill("XBOW")) skill = S_CROSSBOW;
		if(FindSkill(wp->baseSkill) == FindSkill("LBOW")) skill = S_LONGBOW;
	}
	
	int sl = u->GetRealSkill(skill);
		
	if(r->type == R_NEXUS || r->IsStartingCity()) {
		towntype = TOWN_CITY;
		AC = 1;
		if(Globals->SAFE_START_CITIES || (r->type == R_NEXUS))
			IV = 1;
		if(u->type == U_GUARDMAGE) {
			men = 1;
		} else {
			maxmen = Globals->AMT_START_CITY_GUARDS;
			if((!Globals->LEADERS_EXIST) && (r->type != R_NEXUS))
				maxmen = 3 * maxmen / 4;
			men = u->GetMen() + (Globals->AMT_START_CITY_GUARDS/10);
			if(men > maxmen)
				men = maxmen;
		}
	} else {
		towntype = r->town->TownType();
		maxmen = Globals->CITY_GUARD * (towntype+1);
		if(!Globals->LEADERS_EXIST) maxmen = 3 * maxmen / 4;
		men = u->GetMen() + (maxmen/10);
		if(men > maxmen)
			men = maxmen;
	}

	u->SetMen(mantype,men);
	if (IV) u->items.SetNum(I_AMULETOFI,men);

	if(u->type == U_GUARDMAGE) {
		if(Globals->START_CITY_TACTICS)
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
			if(Globals->START_CITY_TACTICS)
				u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
			if(Globals->START_CITY_GUARDS_PLATE)
				u->items.SetNum(armor,men);
		} else {
			u->SetSkill(S_OBSERVATION,towntype + 1);
		}
		u->items.SetNum(weapon,men);
	}
}

void Game::BankInterest()
{
	int interest;

	forlist(&factions) {
		Faction * fac = (Faction *) elem;
		interest = (fac->bankaccount/100)*fac->type[F_TRADE];
		fac->bankaccount += interest;
		fac->interest = interest;
	}
}

void Game::ProcessMigration()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		r->Migrate();
	}
}

void Game::DevelopTowns()
{
	for(int i=0; i<5; i++) {
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			if((!r) || (!r->town)) continue;
			r->CheckTownIncrease();
		}
	}
}

void Game::Equilibrate()
{
	Awrite("Initialising the economy");
	for(int a=0; a<25; a++) {
		Adot();
		ProcessMigration();
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			r->PostTurn(&regions);
		}
	}
	Awrite("");
}
