#ifdef WIN32
#include <memory.h>  // Needed for memcpy on windows
#endif

#include <string.h>

#include "game.h"
#include "unit.h"
#include "astring.h"
#include "gamedata.h"
#include <type_traits>

int Game::EditGame(int *pSaveGame)
{
	*pSaveGame = 0;

	Awrite("Editing an Atlantis Game: ");
	do {
		int exit = 0;

		Awrite("Main Menu");
		Awrite("  1) Find a region...");
		Awrite("  2) Find a unit...");
		Awrite("  3) Create a new unit...");
		Awrite("  qq) Quit without saving.");
		Awrite("  x) Exit and save.");
		Awrite("> ");

		AString *pStr = AGetString();
		Awrite("");

		if (*pStr == "qq") {
			exit = 1;
			Awrite("Quitting without saving.");
		} else if (*pStr == "x") {
			exit = 1;
			*pSaveGame = 1;
			Awrite("Exit and save.");
		} else if (*pStr == "1") {
			ARegion *pReg = EditGameFindRegion();
			if (pReg) EditGameRegion(pReg);
		} else if (*pStr == "2") {
			EditGameFindUnit();
		} else if (*pStr == "3") {
			EditGameCreateUnit();
		} else {
			Awrite("Select from the menu.");
		}

		delete pStr;
		if (exit) {
			break;
		}
	} while(1);

	return(1);
}

ARegion *Game::EditGameFindRegion()
{
	ARegion *ret = 0;
	int x, y, z;
	AString *pStr = 0, *pToken = 0;
	Awrite("Region coords (x y z):");
	do {
		pStr = AGetString();

		pToken = pStr->gettoken();
		if (!pToken) {
			Awrite("No such region.");
			break;
		}
		x = pToken->value();
		SAFE_DELETE(pToken);

		pToken = pStr->gettoken();
		if (!pToken) {
			Awrite("No such region.");
			break;
		}
		y = pToken->value();
		SAFE_DELETE(pToken);

		pToken = pStr->gettoken();
		if (pToken) {
			z = pToken->value();
			SAFE_DELETE(pToken);
		} else {
			z = 0;
		}

		ARegion *pReg = regions.GetRegion(x, y, z);
		if (!pReg) {
			Awrite("No such region.");
			break;
		}

		ret = pReg;
	} while(0);

	if (pStr) delete pStr;
	if (pToken) delete pToken;

	return(ret);
}

void Game::EditGameFindUnit()
{
	AString *pStr;
	Awrite("Which unit number?");
	pStr = AGetString();
	int num = pStr->value();
	delete pStr;
	Unit *pUnit = GetUnit(num);
	if (!pUnit) {
		Awrite("No such unit!");
		return;
	}
	EditGameUnit(pUnit);
}

void Game::EditGameRegion(ARegion *pReg)
//copied direct from AtlantisDev 030730 post
{
	do {
		Awrite( AString("Region ") + pReg->num + ": " +
			pReg->Print() );
		Awrite( " 1) Edit objects..." );
		Awrite( " 2) Edit terrain..." );
		Awrite( " q) Return to previous menu." );

		int exit = 0;
		AString *pStr = AGetString();
		if ( *pStr == "1" ) {
			EditGameRegionObjects( pReg );
		}
		else if ( *pStr == "2" ) {
			EditGameRegionTerrain( pReg );
		}
		else if ( *pStr == "q" ) {
			exit = 1;
		}
		else {
			Awrite( "Select from the menu." );
		}
		if (pStr) delete pStr;

		if ( exit ) {
		break;
		}
	}
	while( 1 );
}


/* RegionEdit Patch 030829 BS */
void Game::EditGameRegionObjects( ARegion *pReg )
//template copied from AtlantisDev 030730 post. Modified option a, added option h, m.
{
	do {
		Awrite( AString( "Region: " ) + pReg->ShortPrint() );
		Awrite( "" );
		int i = 0;
		AString temp = AString("");
		for(const auto obj : pReg->objects) {
			temp = AString ((AString(i) + ". " + *obj->name + " : " + ObjectDefs[obj->type].name));
//			if (Globals->HEXSIDE_TERRAIN && obj->hexside>-1) temp += AString( AString(" (side:") + DirectionAbrs[obj->hexside] + ").");
			Awrite(temp);
			i++;
		}
		Awrite( "" );

		Awrite( " [a] [object type] [dir] to add object" );
		Awrite( " [d] [index] to delete object" );
//		if (Globals->HEXSIDE_TERRAIN) Awrite( " [h] [index] [dir] to change the hexside of an object" );
		Awrite( " [n] [index] [name] to rename object" );
//		if (Globals->HEXSIDE_TERRAIN) Awrite( " [m] [index] to add/delete a mirrored object" );
		Awrite( " q) Return to previous menu." );

		int exit = 0;
		AString *pStr = AGetString();
		if ( *pStr == "q" ) {
			exit = 1;
		} else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if ( !pToken ) {
					Awrite( "Try again." );
					break;
				}

				// add object
				if (*pToken == "a") {
					SAFE_DELETE( pToken );
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					int objType = ParseObject(pToken, 0);
					if ( (objType == -1) || (ObjectDefs[objType].flags & ObjectType::DISABLED) ) {
						Awrite( "No such object." );
						break;
					}
					SAFE_DELETE( pToken );

					/*
					int dir=-1;
					if (ObjectDefs[objType].hexside && Globals->HEXSIDE_TERRAIN ) {
						if (!ObjectIsShip(objType) || !(TerrainDefs[pReg->type].similar_type == R_OCEAN) ) {
							pToken = pStr->gettoken();
							if (!pToken) {
								Awrite( "Specify direction" );
								break;
							}
							dir = ParseHexside(pToken);
							if (dir<0) {
								Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
								break;
							}
						}
					}
					*/

					Object *o = new Object(pReg);
					o->type = objType;
					o->incomplete = 0;
					o->inner = -1;
					// o->hexside = dir;
					if (o->IsFleet()) {
						o->num = shipseq++;
						o->name = new AString(AString("Fleet") + " [" + o->num + "]");
					}
					else {
						o->num = pReg->buildingseq++;
						o->name = new AString(AString("Building") + " [" + o->num + "]");
					}
					pReg->objects.push_back(o);
				}
				// delete object
				else if (*pToken == "d") {
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					size_t index = pToken->value();
					if ( (index < 0) || (index >= pReg->objects.size()) ) { //modified minimum to <0 to allow deleting object 0. 030824 BS
						Awrite( "Incorrect index." );
						break;
					}
					SAFE_DELETE( pToken );

					pReg->objects.erase(pReg->objects.begin() + index);
				}
	//hexside change
	/*			else if (*pToken == "h") {
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					int index = pToken->value();
					if ( (index < 1) || (index >= pReg->objects.Num()) ) {
						Awrite( "Incorrect index." );
						break;
					}
					SAFE_DELETE( pToken );

					int i = 0;
					Object *tmp = (Object *)pReg->objects.First();
					for (i = 0; i < index; i++) tmp = (Object *)pReg->objects.Next(tmp);

					if (!(ObjectDefs[tmp->type].hexside)) {
						Awrite("Not a hexside object.");
						break;
					}

					if (!Globals->HEXSIDE_TERRAIN) {
						Awrite("Hexside terrain disabled under game rules.");
						break;
					}

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Specify Direction." );
						break;
					}

					int dir=-1;
					dir = ParseHexside(pToken);
					if (dir==-1) {
						Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
						break;
					}

					SAFE_DELETE(pToken);
					if (dir) {
						tmp->hexside = dir;
						if (tmp->mirror) { // reset mirrors, else problems later
							tmp->mirror->mirror = NULL;
							tmp->mirror->mirrornum = -1;
							Awrite("Object mirroring removed");
						}
						tmp->mirrornum = -1;
						tmp->mirror = NULL;
					}
				}
	//mirror change
				else if (*pToken == "m") {
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					int index = pToken->value();
					if ( (index < 1) || (index >= pReg->objects.Num()) ) {
						Awrite( "Incorrect index." );
						break;
					}
					SAFE_DELETE( pToken );

					int i = 0;
					Object *tmp = (Object *)pReg->objects.First();
					for (i = 0; i < index; i++) tmp = (Object *)pReg->objects.Next(tmp);

					// if has a mirror, delete the mirror
					if (tmp->mirror) {
	//					Awrite(AString(AString("Mirror ") + tmp->mirror->name + " deleted."));
						Awrite("Mirror deleted");
						tmp->mirror->region->objects.Remove(tmp->mirror);
						tmp->mirror == NULL;
						tmp->mirrornum == -1;
					}

					else {
						if (!(ObjectDefs[tmp->type].hexside)) {
							Awrite("Not a hexside object.");
							break;
						}
						if (tmp->hexside < 0) {
							Awrite("Object not on a hexside.");
							break;
						}
						if (tmp->IsFleet()) {
							Awrite("Fleets cannot be mirrored.");
							break;
						}
						if (!Globals->HEXSIDE_TERRAIN) {
							Awrite("Hexside terrain disabled under game rules.");
							break;
						}

						if (!pReg->neighbors[tmp->hexside]) {
							Awrite("No neighbouring region.");
							break;
						}

						Object *o = new Object(pReg->neighbors[tmp->hexside]);
						o->num = pReg->neighbors[tmp->hexside]->buildingseq++;
						o->type = ObjectDefs[tmp->type].mirror;
						o->name = new AString(AString("Building [") + o->num + "]");
						o->incomplete = 0;
						o->hexside = pReg->GetRealDirComp(tmp->hexside);
						o->inner = -1;
						o->mirrornum = tmp->num;
						o->mirror = tmp;
						pReg->neighbors[tmp->hexside]->objects.Add(o);

						tmp->mirrornum = o->num;
						tmp->mirror = o;
						Awrite("Mirror added");
					}
				}
	*/
	// rename object
				else if (*pToken == "n") {
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					size_t index = pToken->value();
					if ( (index < 1) || (index >= pReg->objects.size()) ) {
						Awrite( "Incorrect index." );
						break;
					}
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "No name given." );
						break;
					}

					Object *tmp = pReg->objects[index];
					AString * newname = pToken->getlegal();
					SAFE_DELETE(pToken);
					if (newname) {
						delete tmp->name;
						*newname += AString(" [") + tmp->num + "]";
						tmp->name = newname;
					}
				}

			} while( 0 );
		if (pToken) delete pToken;
		}
		if (pStr) delete pStr;

		if (exit) {
			break;
		}
	}
	while( 1 );
}

void Game::EditGameRegionTerrain( ARegion *pReg )
{
	do {
		Awrite("");
		Awrite( AString( "Region: " ) + pReg->Print() );
		Awrite( "" );
// write pop stuff
		Awrite( AString("") + pReg->population + " " + ItemDefs[pReg->race].names + " basepop");
		if (pReg->town) Awrite( AString("") + pReg->town->pop + " " + ItemDefs[pReg->race].names + " townpop");
		Awrite( AString("") + pReg->Population() + " " + ItemDefs[pReg->race].names + " totalpop");

// write wages
		Awrite(AString("Wages: ") + pReg->WagesForReport() + ".");
		Awrite(AString("Maxwages: ") + pReg->maxwages + ".");

// write products
		AString temp = "Products: ";
		int has = 0;
		for (const auto& p : pReg->products) {
			if (ItemDefs[p->itemtype].type & IT_ADVANCED) {
				if (has) {
					temp += AString(", ") + p->write_report();
				} else {
					has = 1;
					temp += p->write_report();
				}
			} else {
				if (p->itemtype == I_SILVER) {
					if (p->skill == S_ENTERTAINMENT) {
						Awrite (AString("Entertainment available: $") +
									p->amount + ".");
					}
				} else {
					if (has) {
						temp += AString(", ") + p->write_report();
					} else {
						has = 1;
						temp += p->write_report();
					}
				}
			}
		}
		if (has==0) temp += "none";
		temp += ".";
		Awrite(temp);
		Awrite( "" );

		if (Globals->GATES_EXIST && pReg->gate && pReg->gate != -1) {
			Awrite(AString("There is a Gate here (Gate ") + pReg->gate +
				" of " + (regions.numberofgates) + ").");
			if (Globals->GATES_NOT_PERENNIAL) Awrite(AString("This gate opens "
				"in month ") + pReg->gatemonth);
			Awrite("");
		}


		Awrite( " [t] [terrain type] to modify terrain type" );
		Awrite( " [r] [race] to modify local race" );
		Awrite( "	 (use none, None or 0 to unset)" );
		Awrite( " [w] [maxwages] to modify local wages" );
		Awrite( " [p] to regenerate products according to terrain type" );
		Awrite( " [g] to regenerate all according to terrain type" );
		if (pReg->gate > 0) Awrite( " [dg] to delete the gate in this region" );
		else Awrite( " [ag] to add a gate to this region" );
		Awrite( " [n] [name] to modify region name" );
		if (pReg->town) {
		Awrite( " [town] to regenerate a town" );
		Awrite( " [deltown] to remove a town" );
		Awrite( " [tn] [name] to rename a town" );
		Awrite( " [v] to view/modify town markets" );
		} else Awrite( " [town] to add a town" );
		Awrite( " q) Return to previous menu." );

		int exit = 0;
		AString *pStr = AGetString();
		if ( *pStr == "q" ) {
			exit = 1;
		} else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if ( !pToken ) {
					Awrite( "Try again." );
					break;
				}

				// modify terrain
				if (*pToken == "t") {
					SAFE_DELETE( pToken );
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					int terType = ParseTerrain(pToken);
					if (terType == -1) {
						Awrite( "No such terrain." );
						break;
					}
					SAFE_DELETE( pToken );

					pReg->type = terType;
				}
				else if (*pToken == "r") {
					SAFE_DELETE( pToken );
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					int prace = 0;
					prace = ParseAllItems(pToken);
					if (!(ItemDefs[prace].type & IT_MAN) || (ItemDefs[prace].flags & ItemType::DISABLED) ) {
						if (!(*pToken == "none" || *pToken == "None" || *pToken == "0")) {
							Awrite( "No such race." );
							break;
						} else {
							prace = -1;
						}
					}
					if (prace != 0) pReg->race = prace;
					pReg->UpdateEditRegion();
					SAFE_DELETE( pToken );
				}
				else if (*pToken == "dg") {
					SAFE_DELETE( pToken );
					if (Globals->DISPERSE_GATE_NUMBERS) {
						pReg->gate = 0;
						regions.numberofgates--;
					} else {
						if (pReg->gate > 0) {
							int numgates = regions.numberofgates;
							for(const auto reg : regions) {
								if (reg->gate == numgates) {
									reg->gate = pReg->gate;
									pReg->gate = 0;
									regions.numberofgates--;
									break;
								}
							}
							Awrite("Error: Could not find last gate");
						}
					}
				}
				else if (*pToken == "ag") {
					SAFE_DELETE( pToken );
					if (pReg->gate > 0) break;
					regions.numberofgates++;
					if (Globals->DISPERSE_GATE_NUMBERS) {
						int ngates, log10, *used, i;
						log10 = 0;
						ngates = regions.numberofgates;
						while (ngates > 0) {
							ngates /= 10;
							log10++;
						}
						ngates = 10;
						while (log10 > 0) {
							ngates *= 10;
							log10--;
						}
						used = new int[ngates];
						for (i = 0; i < ngates; i++)
							used[i] = 0;
						for(const auto reg : regions) {
							if (reg->gate)
								used[reg->gate - 1] = 1;
						}
						pReg->gate = getrandom(ngates);
						while (used[pReg->gate])
							pReg->gate = getrandom(ngates);
						delete[] used;
						pReg->gate++;
					} else {
						int gatenum = getrandom(regions.numberofgates) + 1;
						if (gatenum != regions.numberofgates) {
							for(const auto reg : regions) {
								if (reg->gate == gatenum) reg->gate = regions.numberofgates;
							}
						}
						pReg->gate = gatenum;
					}
					pReg->gatemonth = getrandom(12);
				}
				else if (*pToken == "w") {
					SAFE_DELETE( pToken );
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int val = pToken->value();
					SAFE_DELETE( pToken );
					if (val) {
						int change = val - pReg->maxwages;
						pReg->maxwages = val;
						pReg->wages += change;
					}
					pReg->UpdateEditRegion();
				}
				else if (*pToken == "p") {
					SAFE_DELETE(pToken);
					auto removes = remove_if(
						pReg->products.begin(),
						pReg->products.end(),
						[](Production *p) { return p->itemtype != I_SILVER; }
					);
					for_each (removes, pReg->products.end(), [](Production *p) mutable { delete p; });
					pReg->products.erase(removes, pReg->products.end());
					pReg->SetupProds(1);
				}
				else if (*pToken == "g") {
					SAFE_DELETE(pToken);

					if (pReg->town) delete pReg->town;
					pReg->town = NULL;

					for (auto& p : pReg->products) delete p; // Free the allocated object
					pReg->products.clear(); // empty the vector.
					pReg->SetupProds(1);

					for (auto& m : pReg->markets) delete m; // Free the allocated object
					pReg->markets.clear(); // empty the vector.

					pReg->SetupEditRegion();
					pReg->UpdateEditRegion();
				}
				else if (*pToken == "n") {
					SAFE_DELETE(pToken);
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					*pReg->name = *pToken;
					SAFE_DELETE(pToken);
				}
				else if (*pToken == "tn") {
					SAFE_DELETE(pToken);
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}

					if (pReg->town) *pReg->town->name = *pToken;
					SAFE_DELETE(pToken);
				}
				else if (*pToken == "town") {
					SAFE_DELETE(pToken);

					if (pReg->race<0) pReg->race = 9;

					AString *townname = new AString("name");
					if (pReg->town) {
						*townname = *pReg->town->name;
						delete pReg->town;
						for (auto& m : pReg->markets) delete m; // Free the allocated object
						pReg->markets.clear(); // empty the vector.
					}
					pReg->AddTown(townname);

					pReg->UpdateEditRegion(); // financial stuff! Does markets
				}
				else if (*pToken == "deltown") {
					SAFE_DELETE(pToken);
					if (pReg->town) {
						delete pReg->town;
						pReg->town = NULL;
						for (auto& m : pReg->markets) delete m; // Free the allocated object
						pReg->markets.clear(); // empty the vector.

						pReg->UpdateEditRegion();
					}
				}
				else if (*pToken == "v") {
					if (pReg->town) EditGameRegionMarkets(pReg);
				}
			}
			while( 0 );
			if (pToken) delete pToken;
		}
		if (pStr) delete pStr;

		if ( exit ) {
			break;
		}
	}
	while( 1 );
}

void Game::EditGameRegionMarkets( ARegion *pReg )
{
/* This only gets called if pReg->town exists! */
	do {
		Awrite("");
		Awrite( AString( "Region: " ) + pReg->Print() );
		Awrite( "" );
// write pop stuff
		Awrite( AString("") + pReg->town->pop + " " + ItemDefs[pReg->race].names + " townpop");

//write markets
		Awrite(AString("Market Format: ... price(base). minpop/maxpop. minamt/maxamt."));

		Awrite("Wanted: ");
		for (const auto &m : pReg->markets) {
			if (m->type == Market::MarketType::M_SELL) {
				AString temp = AString(ItemString(m->item, m->amount)) + " at $" + m->price + "(" + m->baseprice + ").";
				temp += AString(" Pop: ") + m->minpop + "/" + m->maxpop + ".";
				temp += AString(" Amount: ") + m->minamt + "/" + m->maxamt + ".";
				Awrite(temp);
			}
		}
		Awrite("For Sale: ");
		for (const auto &m : pReg->markets) {
			if (m->type == Market::MarketType::M_BUY) {
				AString temp = AString(ItemString(m->item, m->amount)) + " at $" + m->price + "(" + m->baseprice + ").";
				temp += AString(" Pop: ") + m->minpop + "/" + m->maxpop + ".";
				temp += AString(" Amount: ") + m->minamt + "/" + m->maxamt + ".";
				Awrite(temp);
			}
		}

		Awrite( "" );

		Awrite( " [g] to regenerate all markets" );
		Awrite( " [p] [item] [minpop] [maxpop] to add/modify market population" );
		Awrite( " [a] [item] [minamt] [maxamt] to add/modify market amounts" );
		Awrite( " [c] [item] [price] [baseprice] to add/modify item prices" );
		Awrite( " [s] [item] to swap an item between wanted and sold" );
		Awrite( " [d] [item] to delete an item from the market" );
		Awrite( " q) Return to previous menu." );

		int exit = 0;
		AString *pStr = AGetString();
		if ( *pStr == "q" ) {
			exit = 1;
		} else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if ( !pToken ) {
					Awrite( "Try again." );
					break;
				}

				// regenerate markets
				if (*pToken == "g") {
					SAFE_DELETE(pToken);

					for (auto& m : pReg->markets) delete m; // Free the allocated object
					pReg->markets.clear(); // empty the vector.

					pReg->SetupCityMarket();
					pReg->UpdateEditRegion();
				}
				else if (*pToken == "p") {
					SAFE_DELETE(pToken);

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int mitem = ParseEnabledItem(pToken);
					if (mitem<0) {
						Awrite("No such item");
						break;
					}
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					int minimum = pToken->value();
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					int maximum = pToken->value();
					SAFE_DELETE( pToken );

					int done = 0;

					if (minimum + 1 > maximum) {
						Awrite("Maximum must be more than minimum");
						break;
					}

					int population = pReg->Population();

					for (auto& m : pReg->markets) {
						if (m->item == mitem) {
							m->minpop = minimum;
							m->maxpop = maximum;

							if (population <= m->minpop)
								m->amount = m->minamt;
							else {
								if (population >= m->maxpop)
									m->amount = m->maxamt;
								else {
									m->amount = m->minamt
										+ ((m->maxamt - m->minamt) * (population - m->minpop))
										/ (m->maxpop - m->minpop);
								}
							}
							done = 1;
						}
					}

					if (!done) {
						int price = (ItemDefs[mitem].baseprice * (100 + getrandom(50))) / 100;
						Market *m = new Market(Market::MarketType::M_SELL, mitem, price, 0, minimum, maximum, 0, 0);
						pReg->markets.push_back(m);
					}

				}
				else if (*pToken == "a") {
					SAFE_DELETE(pToken);

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int mitem = ParseEnabledItem(pToken);
					if (mitem<0) {
						Awrite("No such item");
						break;
					}
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					int minimum = pToken->value();
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					int maximum = pToken->value();
					SAFE_DELETE( pToken );

					int done = 0;

					if (minimum + 1 > maximum) {
						Awrite("Maximum must be more than minimum");
						break;
					}

					int population = pReg->Population();

					for (auto& m: pReg->markets) {
						if (m->item == mitem) {
							m->minamt = minimum;
							m->maxamt = maximum;

							if (population <= m->minpop)
								m->amount = m->minamt;
							else {
								if (population >= m->maxpop)
									m->amount = m->maxamt;
								else {
									m->amount = m->minamt
										+ ((m->maxamt - m->minamt) * (population - m->minpop))
										/ (m->maxpop - m->minpop);
								}
							}
							done = 1;
						}
					}

					if (!done) {
						int price = (ItemDefs[mitem].baseprice * (100 + getrandom(50))) / 100;
						int mamount = minimum + (maximum * population / 5000);
						Market *m = new Market(
							Market::MarketType::M_SELL, mitem, price, mamount, 0, 5000, minimum, maximum
						);
						pReg->markets.push_back(m);
					}

				}
				else if (*pToken == "c") {
					SAFE_DELETE(pToken);

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int mitem = ParseEnabledItem(pToken);
					if (mitem<0) {
						Awrite("No such item");
						break;
					}
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					int price = pToken->value();
					SAFE_DELETE( pToken );

					pToken = pStr->gettoken();
					int baseprice = pToken->value();
					SAFE_DELETE( pToken );

					if (price<1 || baseprice<1) {
						Awrite("Price must be more than zero");
						break;
					}

					int done = 0;
					for (auto& m: pReg->markets) {
						if (m->item == mitem) {
							m->price = price;
							m->baseprice = baseprice;
							done = 1;
						}
					}

					if (!done) {
						Market *m = new Market(Market::MarketType::M_SELL, mitem, price, 0, 0, 5000, 0, 0);
						m->baseprice = baseprice;
						pReg->markets.push_back(m);
					}

				}
				else if (*pToken == "s") {
					SAFE_DELETE(pToken);

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int mitem = ParseEnabledItem(pToken);
					if (mitem<0) {
						Awrite("No such item");
						break;
					}
					SAFE_DELETE( pToken );

					// remove all duplicate market items of the same type.
					std::unordered_set<int> s;
					auto dupes = remove_if(
						pReg->markets.begin(),
						pReg->markets.end(),
						[&s, mitem](const Market *m) { return m->item == mitem && !s.insert(m->item).second; }
					);
					for_each (dupes, pReg->markets.end(), [](Market *m) mutable { delete m; });
					pReg->markets.erase(dupes, pReg->markets.end());
					auto m = find_if(
						pReg->markets.begin(),
						pReg->markets.end(),
						[mitem](const Market *m) { return m->item == mitem; }
					);
					if (m != pReg->markets.end()) {
						(*m)->type = (
							(*m)->type == Market::MarketType::M_SELL
							? Market::MarketType::M_BUY
							: Market::MarketType::M_SELL
						);
					} else {
						Awrite("No such market");
					}
				}
				else if (*pToken == "d") {
					SAFE_DELETE(pToken);

					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int mitem = ParseEnabledItem(pToken);
					if (mitem<0) {
						Awrite("No such item");
						break;
					}
					SAFE_DELETE( pToken );

					auto m = find_if(
						pReg->markets.begin(),
						pReg->markets.end(),
						[mitem](const Market *m) { return m->item == mitem; }
					);
					if (m != pReg->markets.end()) {
						auto dupes = remove_if(
							pReg->markets.begin(),
							pReg->markets.end(),
							[mitem](const Market *m) { return m->item == mitem; }
						);
						for_each (dupes, pReg->markets.end(), [](Market *m) mutable { delete m; });
						pReg->markets.erase(dupes,	pReg->markets.end());
					} else {
						Awrite("No such market");
					}
				}
			}
			while( 0 );
			if (pToken) delete pToken;
		}
		if (pStr) delete pStr;

		if ( exit ) {
			break;
		}
	}
	while( 1 );
}


void Game::EditGameUnit(Unit *pUnit)
{
	do {
		Awrite(AString("Unit: ") + *(pUnit->name));
		Awrite(AString("Faction: ") + pUnit->faction->num);
		Awrite(AString("  in ") +
				pUnit->object->region->ShortPrint());
		Awrite("  1) Edit items...");
		Awrite("  2) Edit skills...");
		Awrite("  3) Move unit...");
		Awrite("  4) Edit details...");

		Awrite("  q) Stop editing this unit.");

		int exit = 0;
		AString *pStr = AGetString();
		if (*pStr == "1") {
			EditGameUnitItems(pUnit);
		} else if (*pStr == "2") {
			EditGameUnitSkills(pUnit);
		} else if (*pStr == "3") {
			EditGameUnitMove(pUnit);
		} else if (*pStr == "4") {
			EditGameUnitDetails(pUnit);
		} else if (*pStr == "q") {
			exit = 1;
		} else {
			Awrite("Select from the menu.");
		}
		delete pStr;

		if (exit) {
			break;
		}
	} while(1);
}

void Game::EditGameUnitItems(Unit *pUnit)
{
	do {
		int exit = 0;
		Awrite(AString("Unit items: ") + pUnit->items.Report(2, 1, 1));
		Awrite("  [item] [number] to update an item.");
		Awrite("  q) Stop editing the items.");
		AString *pStr = AGetString();
		if (*pStr == "q") {
			exit = 1;
		} else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if (!pToken) {
					Awrite("Try again.");
					break;
				}

				int itemNum = ParseAllItems(pToken);
				if (itemNum == -1) {
					Awrite("No such item.");
					break;
				}
				if (ItemDefs[itemNum].flags & ItemType::DISABLED) {
					Awrite("No such item.");
					break;
				}
				SAFE_DELETE(pToken);

				int num;
				pToken = pStr->gettoken();
				if (!pToken) {
					num = 0;
				} else {
					num = pToken->value();
				}

				pUnit->items.SetNum(itemNum, num);
				/* Mark it as known about for 'shows' */
				pUnit->faction->items.SetNum(itemNum, 1);
			} while(0);
			if (pToken) delete pToken;
		}
		if (pStr) delete pStr;

		if (exit) {
			break;
		}
	} while(1);
}

void Game::EditGameUnitSkills(Unit *pUnit)
{
	do {
		int exit = 0;
		Awrite(AString("Unit skills: ") +
				pUnit->skills.Report(pUnit->GetMen()));
		Awrite("  [skill] [days] to update a skill.");
		Awrite("  q) Stop editing the skills.");
		AString *pStr = AGetString();
		if (*pStr == "q") {
			exit = 1;
		} else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if (!pToken) {
					Awrite("Try again.");
					break;
				}

				int skillNum = ParseSkill(pToken);
				if (skillNum == -1) {
					Awrite("No such skill.");
					break;
				}
				if (SkillDefs[skillNum].flags & SkillType::DISABLED) {
					Awrite("No such skill.");
					break;
				}
				SAFE_DELETE(pToken);

				int days;
				pToken = pStr->gettoken();
				if (!pToken) {
					days = 0;
				} else {
					days = pToken->value();
				}

				if ((SkillDefs[skillNum].flags & SkillType::MAGIC) &&
						(pUnit->type != U_MAGE)) {
					pUnit->type = U_MAGE;
				}
				if ((SkillDefs[skillNum].flags & SkillType::APPRENTICE) &&
						(pUnit->type == U_NORMAL)) {
					pUnit->type = U_APPRENTICE;
				}
				pUnit->skills.SetDays(skillNum, days * pUnit->GetMen());
				int lvl = pUnit->GetRealSkill(skillNum);
				if (lvl > pUnit->faction->skills.GetDays(skillNum)) {
					pUnit->faction->skills.SetDays(skillNum, lvl);
				}
			} while(0);
			delete pToken;
		}
		delete pStr;

		if (exit) {
			break;
		}
	} while(1);
}

void Game::EditGameUnitMove(Unit *pUnit)
{
	ARegion *pReg = EditGameFindRegion();
	if (!pReg) return;

	pUnit->MoveUnit(pReg->GetDummy());
}

void Game::EditGameUnitDetails(Unit *pUnit)
{
	do {
		int exit = 0;
		Awrite(AString("Unit: ") + *(pUnit->name));
		Awrite(AString("Unit faction: ") +
				*(pUnit->faction->name));
		AString temp = " (";
		switch(pUnit->type) {
			case U_NORMAL:
				temp += "normal";
				break;
			case U_MAGE:
				temp += "mage";
				break;
			case U_GUARD:
				temp += "guard";
				break;
			case U_WMON:
				temp += "monster";
				break;
			case U_GUARDMAGE:
				temp += "guardmage";
				break;
			case U_APPRENTICE:
				temp += Globals->APPRENTICE_NAME;
				break;
		}
		temp += ")";
		Awrite(AString("Unit type: ") + pUnit->type + temp);

		Awrite("");
		Awrite("  [f] [num] to change the unit's faction.");
		Awrite("  [t] [num] to change the unit's type.");
		Awrite("  [q] Go back one screen.");

		AString *pStr = AGetString();
		if (*pStr == "q") {
			exit = 1;
		}
		else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if (!pToken) {
					Awrite("Try again.");
					break;
				}
				// change faction
				if (*pToken == "f") {
					SAFE_DELETE( pToken );
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int fnum = pToken->value();
					SAFE_DELETE( pToken );
					if (fnum<1) {
						Awrite("Invalid Faction Number");
						break;
					}

					Faction *fac = get_faction(factions, fnum);
					if (fac) pUnit->faction = fac;
					else Awrite("Cannot Find Faction");
				}
				else if (*pToken == "t") {
					SAFE_DELETE( pToken );
					pToken = pStr->gettoken();
					if ( !pToken ) {
						Awrite( "Try again." );
						break;
					}
					int newtype = pToken->value();
					SAFE_DELETE( pToken );
					if (newtype<0 || newtype>NUNITTYPES-1) {
						Awrite("Invalid Type");
						break;
					}
					pUnit->type = newtype;
				}

			} while(0);
			delete pToken;
		}
		delete pStr;

		if (exit) {
			break;
		}
	} while(1);
}

void Game::EditGameCreateUnit()
{
	Faction *fac = get_faction(factions, 1);
	Unit *newunit = GetNewUnit(fac);
	newunit->SetMen(I_LEADERS, 1);
	newunit->reveal = REVEAL_FACTION;
	newunit->MoveUnit((*regions.begin())->GetDummy());

	EditGameUnit(newunit);
}
