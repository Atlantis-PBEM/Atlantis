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
	Awrite("Deleting Game");
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
					if(Globals->DEFAULT_WORK_ORDER) ProcessWorkOrder(u, 0, 1);
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
	Awrite("Creating Factions");
	CreateNPCFactions();
	Awrite("Creating City Guards");
	if(Globals->CITY_MONSTERS_EXIST)
		CreateCityMons();
	Awrite("Creating Monsters");
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
    elfguardfaction = f.GetInt();
    dwarfguardfaction = f.GetInt();
    independentguardfaction = f.GetInt();
	ghostfaction = f.GetInt();
	peasantfaction = f.GetInt();

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
    Awrite("Writing the Gamefile");
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
	f.PutInt(elfguardfaction);
	f.PutInt(dwarfguardfaction);
	f.PutInt(independentguardfaction);
	f.PutInt(ghostfaction);
	f.PutInt(peasantfaction);

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
	//Awrite("Writing Regions");
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

void Game::FakeGame(Faction *realfac)
{
    //in all regions not visible to the realfac, make terrain "fake" and destroy all units
    //in regions visible but not present, destroy all units
    //in regions present, destroy all units cannot see
    //this leaves us with just what the faction knows about from their last report.
    
    Awrite("Making a fake game!");
    
   	forlist(&regions) {
		ARegion *reg = (ARegion *)elem;
		reg->marker = 0;   //regions marked "0" default.
    }
    
	forlist_reuse(&regions) {
		ARegion *reg = (ARegion *)elem;
	    if(reg->Present(realfac)) {
	        reg->marker = 1;    //if we're present, regions get marked "1".
	        
	        for (int j=0; j<NDIRS; j++) {
				if (reg->neighbors[j]) {
				    if(!reg->neighbors[j]->marker) reg->neighbors[j]->marker = 2; //this region is visible to us. If we're not present there, mark it "2".
                }
            }
	    }
    }
	forlist_reuse(&regions) {
		ARegion *reg = (ARegion *)elem;
		if(reg->marker == 0) {
		    //we could not see this region. Make it a fake region type
		    reg->Fake();
		}

        if(reg->marker != 1) {
            //delete all units here since we're not present
       		forlist(&reg->objects) {
    			Object *obj = (Object *)elem;
    			forlist(&obj->units) {
    				Unit *u = (Unit *)elem;
        			u->MoveUnit(0); //routine for destroying a unit, from ARegion::Kill()
    	            reg->hell.Add(u);
                }
            }
            //clear hexsides that aren't visible
            if(Globals->HEXSIDE_TERRAIN) {
                for (int i=0; i<NDIRS; i++) {
                    if(reg->neighbors[i] && reg->neighbors[i]->marker != 1) {
                		reg->hexside[i]->type = H_DUMMY;
                		reg->hexside[i]->road = 0;
                		reg->hexside[i]->bridge = 0;
                		reg->hexside[i]->harbour = 0;
            		}
          		}
            }
        } else {
            //we are present here ...
       		forlist(&reg->objects) {
    			Object *obj = (Object *)elem;
    			forlist(&obj->units) {
    				Unit *u = (Unit *)elem;
        			if(u->faction != realfac) {
        			    if(!realfac->CanSee(reg, u, 0)) {
                            u->MoveUnit(0);  //we can't see it, so it doesn't exist!
                            reg->hell.Add(u);
        			    } //else if its foggy, delete the unit
        			}
                }
            }
        }
    }
    
    
    //spread free love around the world ...
    forlist_reuse(&factions) {
        Faction *f = (Faction *) elem;
        if(f == realfac) continue;
        //all other factions love everybody ...
        f->defaultattitude = A_ALLY;
        f->attitudes.DeleteAll();
    }
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
						z = 1;
						SAFE_DELETE(pToken);
						pToken = pLine->gettoken();
						if (pToken) {
							y = pToken->value();
							SAFE_DELETE(pToken);
							pToken = pLine->gettoken();
							if (pToken) {
								z = pToken->value();
							}
							pReg = regions.GetRegion(x, y, z);
						}
						if (pReg == NULL)
							Awrite(AString("Bad faction line: ")+save);
					} else {
					    int numfound = 0;
					    forlist(&regions) {
					        ARegion *reg = (ARegion *) elem;
					        if(reg->flagpole == FL_UNUSED_START_LOC) {
					            numfound++;
					            if(!getrandom(numfound)) pReg = reg;
					        }
					    }
					    if(pReg) pReg->flagpole = FL_USED_START_LOC;
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
//			        if(pFac) pFac->TimesReward(); //Arcadia mod, always on!
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
	} else if(*pToken == "Village:") {
		pTemp = pLine->StripWhite();
		if(pTemp) {
    		if(pFac->pStartLoc) {
    		    if(pFac->pStartLoc->town) {
    		        AString *newname = pTemp->getlegal();
    		        delete pFac->pStartLoc->town->name;
			        pFac->pStartLoc->town->name = newname;
    		    }
    		}
		}
	} else if(*pToken == "Hero:") {
		pTemp = pLine->StripWhite();
		if(pTemp && pFac->pStartLoc) {
		    forlist(&pFac->pStartLoc->objects) {
		        Object *o = (Object *) elem;
		        forlist(&o->units) {
		            Unit *u = (Unit *) elem;
		            if(u->faction == pFac && u->type == U_MAGE && pTemp) {
                        u->SetName(pTemp);
                        pTemp = 0;
                    }
		        }
		    }
		}
	} else if(*pToken == "Race:") {
		pTemp = pLine->gettoken();
		if(pTemp && pFac->pStartLoc) {
		    forlist(&pFac->pStartLoc->objects) {
		        Object *o = (Object *) elem;
		        forlist(&o->units) {
		            Unit *u = (Unit *) elem;
		            int starter = 1;
		            if(u->faction == pFac && u->type == U_MAGE) {
		                while(pTemp && starter) {
                            int item = ParseEnabledItem(pTemp);
                            ManType *mt = 0;
                            if(item > -1) mt = FindRace(ItemDefs[item].abr);
                		    if(mt && mt->ethnicity == u->GetEthnicity()) {
                            	pFac->pStartLoc->race = item;
                		        int firstman = 1;
                            	forlist(&u->items) {
                            		Item *i = (Item *) elem;
                            		if (ItemDefs[i->type].type & IT_MAN) {
                            			if(firstman) i->type = item;
                            			else i->num = 0;
                            			firstman = 0;
                            		}
                            	}
                		    }
                		    pTemp = pLine->gettoken();
		                }
		                starter = 0;
		            }
		        }
		    }
		}
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
		    AString * pDefault = new AString("none");
			pFac->password = pDefault;
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
						int getquietsign = pLine->getexclamation();
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
								ProcessOrder(o, u, pLine, NULL, getquietsign);
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

int Game::DoOrdersCheckAll(const AString &strOrders, const AString &strCheck)
{
	Awrite("Setting Up Turn...");
	PreProcessTurn();
	
	Awrite("Reading the Gamemaster File...");
	if(!ReadPlayers()) return(0);

	Awrite("Done.");

	if(gameStatus == GAME_STATUS_FINISHED) {
		Awrite("This game is finished!");
		return(0);
	}
	gameStatus = GAME_STATUS_RUNNING;
	
	if(Globals->ARCADIA_MAGIC) {
	    SetupGuardsmenAttitudes();
	}
	
 //setup the orders checking
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

    Faction *checkfaction = ReadUnknownOrders(&ordersFile);

    if(!checkfaction) {
        Awrite("We're stuffed now!");
        checkFile.PutStr("Something went wrong with identifying your faction. Please contact the GM.");
    } else {
        FakeGame(checkfaction);
        //we now have a game consisting only of the faction in question and what they can see.
        //normally we would now runorders. However, there are some which we might like not to use.
    
        RunCheckAllOrders();
        //the game's now done as far as the faction is concerned.
    
    	if (checkfaction->errors.Num()) {
    	    checkFile.PutStr("Testing your orders appears to have generated the following errors:");
    	    checkFile.PutStr("");
    	    
    		forlist((&checkfaction->errors)) {
    			checkFile.PutStr(*((AString *) elem));
    		}
    		checkfaction->errors.DeleteAll();
    		checkFile.PutStr("");
    		checkFile.PutStr("Please note that those errors were generated without consideration for the actions of other factions, the conduct of battles, or terrain or units not visible in your last report. As such they may not be an accurate representation of your eventual turn.");
    	} else {
    	    checkFile.PutStr("Testing your orders appears to have generated no errors.");
    	}
    }
	checkFile.PutStr("");
	checkFile.PutStr("Below is the traditional order check, which currently may show additional or different errors.");
	checkFile.PutStr("");
	
	//need to delete the game situation.
	//currently not done ... seems to work *fingers crossed*
	
    ordersFile.Close();
//    ordersFile.OpenByName(strOrders);  //recycling the old one this way doesn't seem to work, don't know why. So making a new one:
	Aorders ordersFile2;
	if(ordersFile2.OpenByName(strOrders) == -1) {
		Awrite("No such orders file!");
		return(0);
	}

    DummyGame();
	ParseOrders(0, &ordersFile2, &check);

	ordersFile2.Close();
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
	
	if(Globals->ARCADIA_MAGIC) {
	    SetupGuardsmenAttitudes();
	}

	Awrite("Running the Turn...");
	RunOrders();
	
	Awrite("Writing times articles...");
	CreateTimesReports();

	Awrite("Writing the Report File...");
	WriteReports();
	Awrite("");
	// LLS - write order templates
	if(Globals->SEPERATE_TEMPLATES) {
    	Awrite("Writing order templates...");
    	WriteTemplates();
    	Awrite("");
	}
	battles.DeleteAll();
	
	if(month==11 || month==5) WritePowers();

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
			pReg->DefaultOrders(peasantfaction);
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
			DefaultWorkOrder();       //does this need to be repeated after each load?
		}
	}
}

Faction * Game::ReadUnknownOrders(Aorders *f)
{
    Faction *fac = ParseOrders(0, f, 0);
    return fac;
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


int Game::RunStatistics()
{

    Aoutfile f;
	Ainfile g;

    MakeFactionReportLists();
    CountAllMages();
	if(Globals->APPRENTICES_EXIST) CountAllApprentices();
	CountAllPower(); //counts men as well
	CountGuardedCities();

    AString str = "stats";
    str = str + year;
    str = str + "-";
    str = str + (month+1);
    str = str + ".txt";        
    
    AString oldstr = "stats";
    int tmpmonth = month;
    int tmpyear = year;
    if(tmpmonth == 0) {
        tmpmonth = 12;
        tmpyear--;
    }
    oldstr = oldstr + tmpyear;
    oldstr = oldstr + "-";
    oldstr = oldstr + tmpmonth;
    oldstr = oldstr + ".txt";

	int i = f.OpenByName( str );
	int j = g.OpenByName(oldstr);

	if(i != -1) {
		WriteStatistics( &f, &g, j );
		f.Close();
	}

    return (1);
}

int Game::WriteStatistics( Aoutfile *f, Ainfile *g, int gfile )
{
    //stats stuff:
    int population = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		population += r->Population();
	}
    

//Format is:

//<num> Faction
//Value, <num>, <num>, ...
//Size, <num>, <num>, ...

    if(gfile == -1) {
        f->PutStr(AString("World_Population, ") + population);
        f->PutStr("_");
    
    	forlist(&factions) {
    	    Faction *fac = (Faction *)elem;
    	    f->PutStr( AString(fac->num) + " Faction");
       		f->PutStr( AString("Men, ") + fac->nummen.Value());
    	    f->PutStr( AString("Item_Value, ") + fac->itemnetworth.Value());
    	    f->PutStr( AString("Skill_Value, ") + fac->skillnetworth.Value());
    	    f->PutStr( AString("Total_Value, ") + fac->totalnetworth.Value());
    	    f->PutStr( AString("Heros, ") + fac->magepower.Value());
       		f->PutStr( AString("Cities, ") + fac->guardedcities.Value());
       		f->PutStr( AString("Labryinth, ") + fac->labryinth);
    	    f->PutStr( AString("_") ); //seems to have problems parsing empty lines
    	}
        return 1;
    }
//is a gfile!

    //general game stuff
    AString *pop = g->GetStr();
    *pop += AString(", ") + population;
    f->PutStr( *pop );
    delete pop;
    AString *drag4 = g->GetStr();
    *drag4 += "_";
    f->PutStr( *drag4 );
    delete drag4;
    

    //faction stuff
	forlist_reuse(&factions) {
	    Faction *fac = (Faction *)elem;
 	    AString *s1 = g->GetStr();
    	if(!s1) return(0);

	    int num = s1->value();
	    while(num != fac->num) {
	        //write an empty faction (ie one now dead)
	        f->PutStr(*s1); //faction line
	        delete s1;
	        for(int i=0; i<8; i++) {                    //must change this number if add more stats lines
	            AString *temp = g->GetStr();
	            if(!temp) return(0);
	            if(i<7) *temp += ", 0"; //all lines except the blank end. change this number also!
	            else *temp += "_";
	            f->PutStr(*temp);
	            delete temp;
	        }
	        s1 = g->GetStr();
    	    if(!s1) return(0);
    	    num = s1->value();
	    }
     
        f->PutStr(*s1); //faction line
        delete s1;
        AString *s2 = g->GetStr();
        *s2 += AString(", ") + fac->nummen.Value();
        f->PutStr(*s2);
        delete s2;
        AString *s3 = g->GetStr();
        *s3 += AString(", ") + fac->itemnetworth.Value();
        f->PutStr(*s3);
        delete s3;
        AString *ss3 = g->GetStr();
        *ss3 += AString(", ") + fac->skillnetworth.Value();
        f->PutStr(*ss3);
        delete ss3;
        AString *sss3 = g->GetStr();
        *sss3 += AString(", ") + fac->totalnetworth.Value();
        f->PutStr(*sss3);
        delete sss3;
        AString *s4 = g->GetStr();
        *s4 += AString(", ") + fac->magepower.Value();
        f->PutStr(*s4);
        delete s4;
        AString *s5 = g->GetStr();
        *s5 += AString(", ") + fac->guardedcities.Value();
	    f->PutStr( *s5 );
	    delete s5;
        AString *s6 = g->GetStr(); //blank
        *s6 += AString(", ") + fac->labryinth;
	    f->PutStr( *s6 );
	    delete s6;
        AString *s7 = g->GetStr(); //blank
        *s7 += "_";
	    f->PutStr( *s7 );
	    delete s7;
	}
	return 1;
}

void Game::WritePowers()
{
    Areport f;

    AString str = "times.5";

	int i = f.OpenByName( str );
	if(i != -1) {
        f.PutStr("From the Peasants of Xanaxor.");
        f.PutStr("Bringing you statistics in June and December.");
        f.PutStr("");
        
        f.PutStr("A list of Empire Values:");
        f.PutStr("");
        for(int i=1; i<17; i++) {
            forlist(&factions) {
                Faction * fac = (Faction *) elem;
                if(fac->IsNPC()) continue;
                if(fac->totalnetworth.rank == i) {
                    f.PutStr( AString("Rank ") + i + ": " + fac->totalnetworth.Value() + " silver");
                }        
            }
        }

        f.PutStr("");
        f.PutStr("A list of Empire Sizes:");
        f.PutStr("");
        for(int i=1; i<17; i++) {
            forlist(&factions) {
                Faction * fac = (Faction *) elem;
                if(fac->IsNPC()) continue;
                if(fac->nummen.rank == i) {
                    f.PutStr( AString("Rank ") + i + ": " + fac->nummen.Value() + " men");
                }        
            }
        }

        f.PutStr("");
        f.PutStr("A list of Hero Scores:");
        f.PutStr("");
        for(int i=1; i<17; i++) {
            forlist(&factions) {
                Faction * fac = (Faction *) elem;
                if(fac->IsNPC()) continue;
                if(fac->magepower.rank == i) {
                    f.PutStr( AString("Rank ") + i + ": " + fac->magepower.Value());
                }        
            }
        }
/*
        f.PutStr("");
        f.PutStr("A count of Guarded Cities:");
        f.PutStr("");
        for(int i=1; i<18; i++) { //including guards
            forlist(&factions) {
                Faction * fac = (Faction *) elem;
                if(fac->num == 1 && i == 1) f.PutStr( AString("Guardsmen: ") + fac->guardedcities.Value());
                if(fac->IsNPC()) continue;
                if(fac->guardedcities.rank == i) {
                    f.PutStr( AString("Rank ") + i + ": " + fac->guardedcities.Value());
                }
            }
        }
*/
        f.PutStr("");

        f.PutStr("");
//        f.PutStr(AString("World Population: ") + population);

		f.Close();
	}
}

void Game::WriteReports()
{
	Areport f;

	MakeFactionReportLists();
	CountAllMages();
	if(Globals->APPRENTICES_EXIST)
		CountAllApprentices();
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT)
		CountAllQuarterMasters();
	if (Globals->TACTICS_NEEDS_WAR)
		CountAllTacticians();
    CountAllPower(); //counts men as well
	

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
	if(pStart) temp->start = pStart->num;
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

void Game::CountGuardedCities()
{
	forlist(&factions) {
	    Faction *f = (Faction *) elem;
		f->guardedcities.value = 0;

    	forlist(&regions) {
    		ARegion *r = (ARegion *) elem;
    		if(!r->town) continue;
    		if(r->town->TownType() != TOWN_CITY) continue;
    		int guarded = 0;
    		forlist(&r->objects) {
    			Object *o = (Object *) elem;
    			forlist(&o->units) {
    				Unit *u = (Unit *) elem;
    				if (u->faction == f && u->guard == GUARD_GUARD) guarded = 1;
    			}
    		}
    		if(guarded) f->guardedcities.value++;
    	}
    }

    forlist_reuse(&factions) {
	    Faction *f1 = (Faction *) elem;
	    forlist(&factions) {
	        Faction *f2 = (Faction *) elem;
            if(f2->IsNPC()) continue;
	        if(f2->guardedcities.Value() > f1->guardedcities.Value()) f1->guardedcities.rank++;
	        if(f2->guardedcities.Value() > f1->guardedcities.maxvalue) f1->guardedcities.maxvalue = f2->guardedcities.Value();
        }
    }
}

void Game::CountAllPower()
{
	forlist(&factions) {
		Faction *f = (Faction *)elem;
		f->totalsilver.value = f->unclaimed;
 		f->itemnetworth.value = 0;
 		f->skillnetworth.value = 0;
 		f->totalnetworth.value = 0;
 		f->nummen.value = 0;
		f->magepower.value = 0;
	}
	forlist_reuse(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if(r->zloc == 0 && u->faction->IsNPC()) continue; //don't count nexus guards (XXXX)
				if(u==o->GetOwner() && o->incomplete<1) {
                     if (ObjectDefs[o->type].item == I_WOOD_OR_STONE) u->faction->totalnetworth.value += ObjectDefs[o->type].cost * ItemDefs[I_WOOD].baseprice * 3 / 2;
                     else u->faction->totalnetworth.value += ObjectDefs[o->type].cost * ItemDefs[ObjectDefs[o->type].item].baseprice * 3 / 2;                         
                }
				forlist(&u->items) {
				    Item *item = (Item *)elem;
				    if(item->type == I_SILVER) u->faction->totalsilver.value += item->num;
				    else if(!(ItemDefs[item->type].type & IT_MAN) && !(ItemDefs[item->type].type & IT_MAGIC)) u->faction->itemnetworth.value += 3 * ItemDefs[item->type].baseprice * item->num / 2;
                    else if (ItemDefs[item->type].type & IT_MAN) {
                        u->faction->totalnetworth.value += ItemDefs[item->type].baseprice * item->num;
                        u->faction->nummen.value += item->num;                        
                    } else u->faction->itemnetworth.value += ItemDefs[item->type].baseprice * item->num; //eg magic items
                }
			}
		}
	}
	CountSkillPower();

	forlist_reuse(&factions) {
		Faction *f = (Faction *)elem;
 		f->totalnetworth.value += f->totalsilver.Value() + f->itemnetworth.Value() + f->skillnetworth.Value();
 		f->magepower.value = f->magepower.Value()/30;
	}
	
    forlist_reuse(&factions) {
	    Faction *f1 = (Faction *) elem;
	    forlist(&factions) {
	        Faction *f2 = (Faction *) elem;
	        if(f2->IsNPC()) continue;
	        //total silver
	        if(f2->totalsilver.Value() > f1->totalsilver.Value()) f1->totalsilver.rank++;
	        if(f2->totalsilver.Value() > f1->totalsilver.maxvalue) f1->totalsilver.maxvalue = f2->totalsilver.Value();
	        //items
	        if(f2->itemnetworth.Value() > f1->itemnetworth.Value()) f1->itemnetworth.rank++;
	        if(f2->itemnetworth.Value() > f1->itemnetworth.maxvalue) f1->itemnetworth.maxvalue = f2->itemnetworth.Value();
	        //skill
	        if(f2->skillnetworth.Value() > f1->skillnetworth.Value()) f1->skillnetworth.rank++;
	        if(f2->skillnetworth.Value() > f1->skillnetworth.maxvalue) f1->skillnetworth.maxvalue = f2->skillnetworth.Value();
	        //total NW
	        if(f2->totalnetworth.Value() > f1->totalnetworth.Value()) f1->totalnetworth.rank++;
	        if(f2->totalnetworth.Value() > f1->totalnetworth.maxvalue) f1->totalnetworth.maxvalue = f2->totalnetworth.Value();
	        //nummen
	        if(f2->nummen.Value() > f1->nummen.Value()) f1->nummen.rank++;
	        if(f2->nummen.Value() > f1->nummen.maxvalue) f1->nummen.maxvalue = f2->nummen.Value();
	        //magepower
	        if(f2->magepower.Value() > f1->magepower.Value()) f1->magepower.rank++;
	        if(f2->magepower.Value() > f1->magepower.maxvalue) f1->magepower.maxvalue = f2->magepower.Value();
        }
    }
	
}

void Game::CountSkillPower()
{
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				forlist(&u->skills) {
				    Skill *sk = (Skill *)elem;
                    u->faction->skillnetworth.value += SkillDefs[sk->type].cost * (sk->days + sk->experience) / 30;
                    if(u->type == U_MAGE /*&& (SkillDefs[sk->type].flags & SkillType::MAGIC)*/) u->faction->magepower.value += sk->days + sk->experience;
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
					if (u->GetRealSkill(S_QUARTERMASTER))
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
					if (u->GetRealSkill(S_TACTICS) == 5)
						u->faction->numtacts++;
				}
			}
		}
	}
}

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

int Game::CountQuarterMasters(Faction *pFac)
{
	int i = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if(u->faction == pFac && u->GetRealSkill(S_QUARTERMASTER)) i++;
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
				if(u->faction == pFac && u->GetRealSkill(S_TACTICS) == 5) i++;
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
	//passive skills
	if(Globals->REAL_EXPERIENCE) {
    	int level = u->GetRealSkill(S_STEALTH);
    	if(level) u->Experience(S_STEALTH,level);  //can otherwise only be gained by assassinating or study
    	
    	level = u->GetSkill(S_OBSERVATION);
    	if(level) u->Experience(S_OBSERVATION,level); //can otherwise only be gained by killing would-be-assassins or study

    	level = u->GetSkill(S_TRUE_SEEING);
    	if(level) u->Experience(S_TRUE_SEEING,level); //can otherwise only be gained by study
    	
    	level = u->GetSkill(S_SECSIGHT);
    	if(level) u->Experience(S_SECSIGHT,level); //can otherwise only be gained by study
    	
    	level = u->GetSkill(S_TRADING);
    	if(level && u->numtraded) {
//            u->Experience(S_TRADING,level); //can otherwise only be gained by study
            int exper = 1;  //1 experience for trading anything at all
            int value = 200;
            while(u->numtraded > value) {
                exper++;
                value *= 2;
            }
            //ie 1 for trading $100, 2 for $200, 4 for $800, 6 for $3200, 8 for $12800 etc.
            u->Experience(S_TRADING,exper);
        }
        
    	level = u->GetSkill(S_MERCHANTRY);
    	if(level && u->nummerchanted) {
//            u->Experience(S_MERCHANTRY,level); //can otherwise only be gained by study
            int exper = 1;  //1 experience for trading anything at all
            int value = 100;
            while(u->nummerchanted > value) {
                exper++;
                value *= 2;
            }
            //ie 1 for trading $100, 2 for $200, 4 for $800, 6 for $3200, 8 for $12800 etc.
            u->Experience(S_MERCHANTRY,exper);
        }


    	level = u->GetSkill(S_ARCADIA_QUARTERMASTERY);
    	if(level && u->numquartermastered) {
//            u->Experience(S_ARCADIA_QUARTERMASTERY,level); //can otherwise only be gained by study
            int exper = 4;  //4 experience for trading anything at all
            int value = 400;
            while(u->numquartermastered > value && exper < 10) {   //this one can potentially be abused, so cap at 10.
                exper++;
                value *= 2;
            }
            //ie 5 for sending $400, 7 for $1600, 9 for $6400, 10 for $12800.
            u->Experience(S_ARCADIA_QUARTERMASTERY,exper);
        }
	}
}

void Game::MonsterCheck(ARegion *r, Unit *u)
{
	AString tmp;
	int skill;
//	int linked = 0;
//	map< int, int > chances; //what the hell is this?

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
				if(losses > 0) {  //to prevent getting '0 liches decay' messages.
    				u->items.SetNum(i->type,i->num - losses);
    				AString temp = ItemString(i->type, losses) + " decay";
    				if(losses == 1) temp += "s";
    				temp += " into nothingness";
    				u->Event(temp);
				}
			} else if (ItemDefs[i->type].escape & ItemType::HAS_SKILL) {
				tmp = ItemDefs[i->type].esc_skill;
				skill = LookupSkill(&tmp);
				if (u->GetRealSkill(skill) < ItemDefs[i->type].esc_val) {    //real skill so that units can't disable their skill
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
				int level = u->GetRealSkill(skill);
				int chance;

				if (!level) level = 1; //BS mod to make controlled releases for farming or missiles harder.
				
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
				

/*				if (ItemDefs[i->type].escape & ItemType::LOSE_LINKED) {    //disabled in Arcadia
					if (chance > chances[ItemDefs[i->type].type])
						chances[ItemDefs[i->type].type] = chance;
					linked = 1;
				} else*/ if (chance > getrandom(10000)) {
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

/*		if (linked) {
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
		}*/
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
		if(Globals->ARCADIA_MAGIC) {
            *temp = "Human Guardsmen";
            f->ethnicity = RA_HUMAN;
        }
		f->SetName(temp);
		f->SetNPC();
		f->lastorders = 0;
		factions.Add(f);
		if(Globals->ARCADIA_MAGIC) {
    		f = new Faction(factionseq++);
    		elfguardfaction = f->num;
    		temp = new AString("Elvish Guardsmen");
    		f->SetName(temp);
            f->ethnicity = RA_ELF;
    		f->SetNPC();
    		f->lastorders = 0;
    		factions.Add(f);
    		f = new Faction(factionseq++);
    		dwarfguardfaction = f->num;
    		temp = new AString("Dwarven Guardsmen");
            f->ethnicity = RA_DWARF;
    		f->SetName(temp);
    		f->SetNPC();
    		f->lastorders = 0;
    		factions.Add(f);
    		f = new Faction(factionseq++);
    		independentguardfaction = f->num;
    		temp = new AString("Independent Guardsmen");
            f->ethnicity = RA_OTHER;
    		f->SetName(temp);
    		f->SetNPC();
    		f->lastorders = 0;
    		factions.Add(f);
		} else {
		    elfguardfaction = 0;
		    dwarfguardfaction = 0;
		    independentguardfaction = 0;		
		}
	} else {
		guardfaction = 0;
	    elfguardfaction = 0;
	    dwarfguardfaction = 0;
	    independentguardfaction = 0;
    }
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
	// Create the ghost faction if ARCADIA_MAGIC enabled
	if(Globals->ARCADIA_MAGIC) {
		f = new Faction(factionseq++);
		ghostfaction = f->num;
		temp = new AString("Shades of the Dead");
		f->SetName(temp);
		f->SetNPC();
		f->lastorders = 0;
		factions.Add(f);
		f = new Faction(factionseq++);
		peasantfaction = f->num;
		temp = new AString("Peasantfolk");
		f->SetName(temp);
		f->SetNPC();
		f->lastorders = 0;
		factions.Add(f);
	} else{
        ghostfaction = 0;
        peasantfaction = 0;
    }
}

void Game::CreateFortMon(ARegion *pReg, Object *o)
{
    //no regional guards in towns, else their behaviour will get reset to town guards later :(
    if(pReg->town) return;

//The world is set up by now
//However, some regions (eg nexus) do not have a race assigned. If leaders are disabled that can cause this to crash. So:
    int i=0;
    while(pReg->race < 0) { //we're desperate, take the first enabled race ;)
        if((ItemDefs[i].type & IT_MAN) && !(ItemDefs[i].flags & ItemType::DISABLED)) {
            pReg->race = i;
        }
        i++;
    }
	int size = ObjectDefs[o->type].protect;
	if(size < 10) return;
	int skilllevel = 1;
	if(size > 40) skilllevel++;
	if(size > 160) skilllevel++;
	int num = size / (5*skilllevel);
	num *= 5; //ie only have guard units in multiples of 5
	
	int fac = guardfaction;
	if(Globals->ARCADIA_MAGIC) {
    	ManType *mt = FindRace(ItemDefs[pReg->race].abr);
    	switch(mt->ethnicity) {
    	    case RA_ELF: 
    	        fac = elfguardfaction;
    	        break;
    	    case RA_DWARF: 
    	        fac = dwarfguardfaction;
    	        break;
    	    case RA_OTHER: 
    	        fac = independentguardfaction;
    	        break;
    	    default: 
    	        fac = guardfaction;
    	        break;
    	}
	}

	Faction *pFac = GetFaction(&factions, fac);
	

	if(skilllevel > 1) {
    //one front unit
        AString *s = new AString("Regional Guards");
        
    	Unit *u = MakeManUnit(pFac, pReg, num, skilllevel, 1,
    		0, 0);
    	u->SetMoney(num * Globals->GUARD_MONEY / 2);
    	u->SetName(s);
    	u->type = U_GUARD;
    	u->guard = GUARD_GUARD;
    	u->SetSkill(S_OBSERVATION, skilllevel);
    	u->SetFlag(FLAG_HOLDING,1);
    	u->reveal = REVEAL_FACTION;
    	u->MoveUnit(o);
    //one behind unit
        AString *s2 = new AString("Regional Guards");
    
    	Unit *u2 = MakeManUnit(pFac, pReg, num, skilllevel, 1,
    		0, 1);
    	u2->SetMoney(num * Globals->GUARD_MONEY / 2);
    	u2->SetName(s2);
    	u2->type = U_GUARD;
    	u2->guard = GUARD_GUARD;
    	u2->SetSkill(S_OBSERVATION, skilllevel);
    	u2->SetFlag(FLAG_HOLDING,1);
    	u2->reveal = REVEAL_FACTION;
    	u2->MoveUnit(o);
   	}
   	if(skilllevel != 2) {
    //one unit of best type        
        int behind = SkillMax("XBOW", pReg->race);
        if (SkillMax("LBOW", pReg->race) > behind) behind = SkillMax("LBOW", pReg->race);
        int front = SkillMax("COMB", pReg->race);
        if (SkillMax("RIDI", pReg->race) > front && (TerrainDefs[pReg->type].flags & TerrainType::RIDINGMOUNTS) && !(TerrainDefs[pReg->type].flags & TerrainType::RIDINGLIMITED)) 
            front = SkillMax("RIDI", pReg->race);
        if(behind > front) behind = 1;
        else behind = 0;
        
        AString *s = new AString("Regional Guards");
        
    	Unit *u = MakeManUnit(pFac, pReg, num, skilllevel, 1,
    		0, behind);
    	u->SetMoney(num * Globals->GUARD_MONEY / 2);
    	u->SetName(s);
    	u->type = U_GUARD;
    	u->guard = GUARD_GUARD;
    	u->SetSkill(S_OBSERVATION, skilllevel);
    	u->SetFlag(FLAG_HOLDING,1);
    	u->reveal = REVEAL_FACTION;
    	u->MoveUnit(o);
   	}
}

void Game::CreateCityMon(ARegion *pReg, int percent, int needguard)
#define GUARDFRONT 1
#define GUARDBEHIND 2
#define GUARDMAGE 4
{
//The world is set up by now
//However, some regions (eg nexus) do not have a race assigned. If leaders are disabled that can cause this to crash. So:
    int i=0;
    while(pReg->race < 0) { //we're desperate, take the first enabled race ;)
        if((ItemDefs[i].type & IT_MAN) && !(ItemDefs[i].flags & ItemType::DISABLED)) {
            pReg->race = i;
        }
        i++;
    }
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
		if(!Globals->GUARD_DEPENDS_ON_TAX || !pReg->money) num = Globals->CITY_GUARD * skilllevel;
		else num = pReg->money * Globals->CITY_GUARD / 5000;
	}
	if(Globals->GUARD_DEPENDS_ON_TAX && percent < 20) {    //why less than 20? Need to excuse 100% for the initial start ...
	    num = pReg->untaxed / (20 * Globals->GUARD_MONEY);
	} else {
	    num = num * percent / 100;
	}
	int fac = guardfaction;
	if(Globals->ARCADIA_MAGIC) {
    	ManType *mt = FindRace(ItemDefs[pReg->race].abr);
    	switch(mt->ethnicity) {
    	    case RA_ELF: 
    	        fac = elfguardfaction;
    	        break;
    	    case RA_DWARF: 
    	        fac = dwarfguardfaction;
    	        break;
    	    case RA_OTHER: 
    	        fac = independentguardfaction;
    	        break;
    	    default: 
    	        fac = guardfaction;
    	        break;
    	}
	}
	Faction *pFac = GetFaction(&factions, fac);
	Unit *u = 0;
	Unit *u2 = 0;
	AString *s = new AString("City Guard");

	/*	
	AString temp = TerrainDefs[pReg->type].name;
	temp += AString(" (") + pReg->xloc + "," + pReg->yloc;
	temp += ")";
	temp += AString(" in ") + *pReg->name;
	Awrite(temp);
	*/
	if((Globals->LEADERS_EXIST)) {
		/* standard Leader-type guards */
		u = GetNewUnit(pFac);
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
		if(needguard%(2*GUARDFRONT)/GUARDFRONT) {
    		u = MakeManUnit(pFac, pReg, n, skilllevel, 1,
    			plate, 0);
    		if (IV) u->items.SetNum(I_AMULETOFI,num);
    		u->SetMoney(num * Globals->GUARD_MONEY / 2);
    		u->SetName(s);
    		u->type = U_GUARD;
    		u->guard = GUARD_GUARD;
		}
		if(needguard%(2*GUARDBEHIND)/GUARDBEHIND) {
    		u2 = MakeManUnit(pFac, pReg, n, skilllevel, 1,
    			plate, 1);
    		if (IV) u2->items.SetNum(I_AMULETOFI,num);
    		u2->SetMoney(num * Globals->GUARD_MONEY / 2);
    		AString *un = new AString("City Guard");
    		u2->SetName(un);
    		u2->type = U_GUARD;
    		u2->guard = GUARD_GUARD;
		}
	}
    if(u) {
    	u->SetSkill(S_OBSERVATION, skilllevel);
    	u->SetFlag(FLAG_HOLDING,1);
    	u->reveal = REVEAL_FACTION;
    	u->MoveUnit(pReg->GetDummy());
	}
	if(u2) {
		u2->SetFlag(FLAG_HOLDING,1);
		u2->reveal = REVEAL_FACTION;
		u2->MoveUnit(pReg->GetDummy());
	}
	if((pReg->type == R_NEXUS || (Globals->START_CITY_MAGES + skilllevel - 3) > 0) && (needguard%(2*GUARDMAGE)/GUARDMAGE)) {
		u = GetNewUnit(pFac);
		s = new AString("City Mage");
		u->SetName(s);
		u->type = U_GUARDMAGE;
    	if((Globals->LEADERS_EXIST)) {
    		u->SetMen(I_LEADERS,1);
    		if(IV) u->items.SetNum(I_AMULETOFI,1);
    		u->SetMoney(Globals->GUARD_MONEY);
    		if(!Globals->ARCADIA_MAGIC) u->SetSkill(S_FORCE,Globals->START_CITY_MAGES);
    		else u->SetSkill(S_BASE_WINDKEY,Globals->START_CITY_MAGES+skilllevel-3);
    		u->SetSkill(S_FIRE,Globals->START_CITY_MAGES+skilllevel-3);
    		if(Globals->START_CITY_TACTICS && !(SkillDefs[S_TACTICS].flags & SkillType::DISABLED))
    			u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
    		u->combat = S_FIRE;
    		u->SetFlag(FLAG_BEHIND, 1);
    		u->SetFlag(FLAG_HOLDING, 1);
    		u->reveal = REVEAL_FACTION;
    		u->MoveUnit(pReg->GetDummy());
    	} else {
    	    u->SetMen(pReg->race,1);
    		u->SetMoney(Globals->GUARD_MONEY);
    		if(!Globals->ARCADIA_MAGIC) u->SetSkill(S_FORCE,Globals->START_CITY_MAGES);
    		else u->SetSkill(S_BASE_WINDKEY,Globals->START_CITY_MAGES+skilllevel-3);
    		u->SetSkill(S_FIRE,Globals->START_CITY_MAGES+skilllevel-3);
    		u->combat = S_FIRE;
    		u->SetFlag(FLAG_BEHIND, 1);
    		u->SetFlag(FLAG_HOLDING, 1);
    		u->reveal = REVEAL_FACTION;
    		u->MoveUnit(pReg->GetDummy());
    	}
	}
}

void Game::AdjustCityMons(ARegion *r)
{
/*#define GUARDFRONT 1
#define GUARDBEHIND 2
#define GUARDMAGE 4*/
	int needguard = GUARDFRONT + GUARDBEHIND + GUARDMAGE;
	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->type == U_GUARD || u->type == U_GUARDMAGE) {
				AdjustCityMon(r, u);
				/* Don't create new city guards if we have some */
				if(u->type == U_GUARDMAGE) needguard -= GUARDMAGE;
				else if(u->GetFlag(FLAG_BEHIND)) needguard -= GUARDBEHIND;
				else needguard -= GUARDFRONT;
			}
			if(u->guard == GUARD_GUARD) needguard = 0; //ie if someone else is guarding, no guards needed. This could subsequently go negative ...
		}
	}

	if ((needguard > 0) && (getrandom(100) < Globals->GUARD_REGEN) && r->untaxed > getrandom(r->money)) {  //less chance if being taxed.
		if(!Globals->GUARD_DEPENDS_ON_TAX || r->untaxed >= 20*Globals->GUARD_MONEY) CreateCityMon(r, 10, needguard);
	}
}

void Game::AdjustCityMon(ARegion *r, Unit *u)
{
	int towntype;
    if(r->town) towntype = r->town->TownType();
    else towntype = TOWN_CITY;                 //nexus
//	int AC = 0;
	int men;
//	int IV = 0;
	int mantype;
	int maxmen;
	int weapon = -1;
	int maxweapon = 0;
	int armor = -1;
	int maxarmor = 0;
	int mount = -1;
	int maxmount = 0;
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
		if((ItemDefs[i].type & IT_MOUNT)
			&& (num > maxmount)) {
			mount = i;
			maxmount = num;
		}
	}
	
	int skill = S_COMBAT;         //put in a mod for riding races in riding terrain
	
	if (weapon != -1) {
		WeaponType *wp = FindWeapon(ItemDefs[weapon].abr);
		if(FindSkill(wp->baseSkill) == FindSkill("XBOW")) skill = S_CROSSBOW;
		if(FindSkill(wp->baseSkill) == FindSkill("LBOW")) skill = S_LONGBOW;
	} else weapon = I_SWORD;

	if (mount != -1) {
        if(skill == S_COMBAT && (u->GetSkillKnowledgeMax(S_RIDING) > u->GetSkillKnowledgeMax(S_COMBAT)) && (TerrainDefs[r->type].flags & TerrainType::RIDINGMOUNTS) && !(TerrainDefs[r->type].flags & TerrainType::RIDINGLIMITED)) {
		    skill = S_RIDING;
        } else mount = -1;
	} 

	if(u->type == U_GUARDMAGE) {
		men = 1;
	} else {
		if(!Globals->GUARD_DEPENDS_ON_TAX || !r->money) maxmen = Globals->CITY_GUARD * (towntype+1);
		else maxmen = r->money * Globals->CITY_GUARD / 5000;
		if(!Globals->LEADERS_EXIST) maxmen = 3 * maxmen / 4;
		if(!Globals->GUARD_DEPENDS_ON_TAX) men = u->GetMen() + (maxmen/10);
		else men = u->GetMen() + r->untaxed / (40 * Globals->GUARD_MONEY);  //one-quarter the Nylandor rate per unit!
		if(men > maxmen) men = maxmen;
	}

	u->SetMen(mantype,men);
//	if (IV) u->items.SetNum(I_AMULETOFI,men);

	if(u->type == U_GUARDMAGE) {
		if(Globals->START_CITY_TACTICS && !(SkillDefs[S_TACTICS].flags & SkillType::DISABLED))
			u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
		if(!Globals->ARCADIA_MAGIC) u->SetSkill(S_FORCE,Globals->START_CITY_MAGES);
		else u->SetSkill(S_BASE_WINDKEY,Globals->START_CITY_MAGES+towntype-2);
		if(Globals->ARCADIA_MAGIC && u->IsASpeciality(S_LIGHT) && !u->IsASpeciality(S_FIRE) ) {
		    u->SetSkill(S_LIGHT, Globals->START_CITY_MAGES+towntype-2);
			u->combat = S_LIGHT;
		} else {
		    u->SetSkill(S_FIRE, Globals->START_CITY_MAGES+towntype-2);
			u->combat = S_FIRE;
		}
		u->SetSkill(S_OBSERVATION,towntype + 1); //added AFTER adjust skills, as otherwise non-leaders are limited to ONE skill :(.
		u->SetSkill(S_COMBAT,towntype + 1); //added AFTER adjust skills, as otherwise non-leaders are limited to ONE skill :(.

		u->SetFlag(FLAG_BEHIND, 1);
		u->reveal = REVEAL_FACTION;
		u->SetMoney(Globals->GUARD_MONEY);
	} else {
		int money = men * Globals->GUARD_MONEY;
		u->SetMoney(money);
		u->SetSkill(skill, towntype + 1);
		u->AdjustSkills(1);
		u->SetSkill(S_OBSERVATION,towntype); //added AFTER adjust skills, as otherwise non-leaders are limited to ONE skill :(. Note the mage has +1 obse compared to normal guards. This set so that the obse should never become a unit's default skill (fingers crossed ... maybe still in cities).
		u->items.SetNum(weapon,men);
		if(armor != -1) u->items.SetNum(armor,men);
		if(mount != -1) u->items.SetNum(mount,men);
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
