#ifdef WIN32
#include <memory.h>  // Needed for memcpy on windows
#endif

#include <string.h>

#include "game.h"
#include "unit.h"
#include "fileio.h"
#include "astring.h"
#include "gamedata.h"

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
		Awrite("  4) Global Effects...");
		Awrite("  qq) Quit without saving.");
		Awrite("  x) Exit and save.");
		Awrite("> ");

		AString *pStr = AGetString();
		Awrite("");

		if(*pStr == "qq") {
			exit = 1;
			Awrite("Quiting without saving.");
		} else if(*pStr == "x") {
			exit = 1;
			*pSaveGame = 1;
			Awrite("Exit and save.");
		} else if(*pStr == "1") {
			ARegion *pReg = EditGameFindRegion();
			if(pReg) EditGameRegion(pReg);
		} else if(*pStr == "2") {
			EditGameFindUnit();
		} else if(*pStr == "3") {
			EditGameCreateUnit();			
		} else if(*pStr == "4") {
			EditGameGlobalEffects();			
		} else {
			Awrite("Select from the menu.");
		}

		delete pStr;
		if(exit) {
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
		if(!pToken) {
			Awrite("No such region.");
			break;
		}
		x = pToken->value();
		SAFE_DELETE(pToken);

		pToken = pStr->gettoken();
		if(!pToken) {
			Awrite("No such region.");
			break;
		}
		y = pToken->value();
		SAFE_DELETE(pToken);

		pToken = pStr->gettoken();
		if(pToken) {
			z = pToken->value();
			SAFE_DELETE(pToken);
		} else {
			z = 0;
		}

		ARegion *pReg = regions.GetRegion(x, y, z);
		if(!pReg) {
			Awrite("No such region.");
			break;
		}

		ret = pReg;
	} while(0);

	if(pStr) delete pStr;
	if(pToken) delete pToken;

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
	if(!pUnit) {
		Awrite("No such unit!");
		return;
	}
	EditGameUnit(pUnit);
}

void Game::EditGameRegion(ARegion *pReg)
//copied direct from AtlantisDev 030730 post
{ 
    do {
        Awrite( AString( "Region: " ) + pReg->Print( &regions ) );
        Awrite( AString ("Region number ") + pReg->num);
        Awrite( " 1) Edit objects..." ); 
        Awrite( " 2) Edit terrain..." );
        Awrite( " 3) Edit exits..." );
        Awrite( " 4) Navigate regions..." );
        Awrite( " 4 [dirs]... Navigate specified regions");
        Awrite( " q) Return to previous menu." );

        int exit = 0; 
        AString *pStr = AGetString(); 
        if( *pStr == "1" ) {
            EditGameRegionObjects( pReg ); 
        } 
        else if( *pStr == "2" ) {
            EditGameRegionTerrain( pReg ); 
        } 
        else if( *pStr == "3" ) {
            EditGameRegionExits( pReg ); 
        }
        else if( *pStr == "4" ) {
            pReg = EditGameRegionNavigate( pReg ); 
        } 
        else if( *pStr == "q" ) {
            exit = 1; 
        } 
        else {
            AString *pToken = 0;
            pToken = pStr->gettoken();
            if(pToken && *pToken == "4") {
                pReg = EditGameRegionNavigate( pReg, pStr ); 
            } else {
                Awrite( "Select from the menu." ); 
            }
            SAFE_DELETE(pToken);
        }
        delete pStr; 

        if( exit ) {
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
        Awrite( AString( "Region: " ) + pReg->Print( &regions ) ); 
        Awrite( "" ); 
        int i = 0; 
        AString temp = AString("");
        forlist (&(pReg->objects)) { 
            Object * obj = (Object *)elem; 
            temp = AString ((AString(i) + ". " + *obj->name + " : " + ObjectDefs[obj->type].name));
            if (Globals->HEXSIDE_TERRAIN && obj->hexside>-1) temp += AString( AString(" (side:") + DirectionAbrs[obj->hexside] + ")."); 

    		if (obj->incomplete > 0) {
    			temp += AString(", needs ") + obj->incomplete;
    		} else if(Globals->DECAY && !(ObjectDefs[obj->type].flags & ObjectType::NEVERDECAY) && obj->incomplete < 1) {
    			if(obj->incomplete > (0 - ObjectDefs[obj->type].maxMonthlyDecay)) {
    				temp += ", about to decay";
    			} else if(obj->incomplete > (0 - ObjectDefs[obj->type].maxMaintenance/2)) {
    				temp += ", needs maintenance";
    			}
    		}
    		if (obj->inner != -1) {
    			temp += AString(", contains an inner location to ") + (regions.GetRegion(obj->inner))->ShortPrint(&regions);
    		}

    		if(obj->describe) temp += AString(". ") + *(obj->describe);

            Awrite(temp);
            i++; 
        } 
        Awrite( "" ); 
        if(pReg->flagpole) {
            Awrite(AString("This region is flagpoled ") + pReg->flagpole);
            Awrite("");
        }

        Awrite( " [a] [object type] [dir] to add object" );
        Awrite( " [d] [index] to delete object" ); 
        if (Globals->HEXSIDE_TERRAIN) Awrite( " [h] [index] [dir] to change the hexside of an object" );
        Awrite( " [n] [index] [name] to rename object" );
        Awrite( " [s] [index] [description] to describe an object" );
        Awrite( " [i] [index] to remove an inner connection" );
        Awrite( " [i] [index] [xloc] [yloc] [zloc] to add an inner connection to an object" );
        Awrite( " [f] [num] to flagpole the region with value num." );
        Awrite( " q) Return to previous menu." );

        int exit = 0; 
        AString *pStr = AGetString();
        if( *pStr == "q" ) { 
            exit = 1; 
        } else { 
            AString *pToken = 0; 
            do { 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break;
                }

                // add object 
                if (*pToken == "a") {
                    SAFE_DELETE( pToken ); 
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
    
                    int objType = ParseObject(pToken); 
                    if( (objType == -1) || (ObjectDefs[objType].flags & ObjectType::DISABLED) ) { 
                        Awrite( "No such object." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken ); 
                    int dir=-1;
                    if(ObjectDefs[objType].hexside && Globals->HEXSIDE_TERRAIN ) {
                        if(!ObjectIsShip(objType) || !(TerrainDefs[pReg->type].similar_type == R_OCEAN) ) {
                            pToken = pStr->gettoken(); 
                            if(!pToken) {
                                Awrite( "Specify direction" );
                                break;
                            }
                            dir = ParseHexsideDir(pToken);
                            if(dir<0) {
        					    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
        					    break;
                            }
                        }
                    }
                    
                    Object *o = new Object(pReg); 
                    o->type = objType; 
                    o->incomplete = 0; 
                    o->inner = -1; 
                    o->hexside = dir;
                    if (o->IsBoat()) { 
                        o->num = shipseq++; 
                        o->name = new AString(AString("Ship") + " [" + o->num + "]"); 
                    } 
                    else { 
                        o->num = pReg->buildingseq++; 
                        o->name = new AString(AString("Building") + " [" + o->num + "]"); 
                    } 
                    pReg->objects.Add(o); 
                } 
                // delete object 
                else if (*pToken == "d") { 
                    SAFE_DELETE( pToken ); 
    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                    break; 
                    } 
    
                    int index = pToken->value(); 
                    if( (index < 1) || (index >= pReg->objects.Num()) ) {
                        Awrite( "Incorrect index." ); 
                    break; 
                    } 
                    SAFE_DELETE( pToken ); 
    
                    int i = 0; 
                    AListElem *tmp = pReg->objects.First(); 
                    for (i = 0; i < index; i++) tmp = pReg->objects.Next(tmp); 
                    pReg->objects.Remove(tmp); 
                }
                else if (*pToken == "i") { 
                    SAFE_DELETE( pToken ); 
    
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                    break; 
                    } 
    
                    int index = pToken->value(); 
                    if( (index < 0) || (index >= pReg->objects.Num()) ) {   //modified minimum to <0 to allow deleting object 0. 030824 BS
                        Awrite( "Incorrect index." ); 
                    break; 
                    } 
                    SAFE_DELETE( pToken ); 
    
                    int i = 0; 
                    Object *tmp = (Object *)pReg->objects.First(); 
                    for (i = 0; i < index; i++) tmp = (Object *)pReg->objects.Next(tmp);
                    
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        tmp->inner = -1;
                        break; 
                    }
                    int xloc = pToken->value();
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        AString ("Specify y coordinate");
                        break; 
                    }
                    int yloc = pToken->value();
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        AString ("Specify z coordinate");
                        break; 
                    }
                    int zloc = pToken->value();
                    SAFE_DELETE( pToken );
                    
                    ARegion *target = regions.GetRegion(xloc,yloc,zloc);
                    tmp->inner = target->num;                    
                } 
    //hexside change
                else if (*pToken == "h") { 
                    SAFE_DELETE( pToken ); 
    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
    
                    int index = pToken->value(); 
                    if( (index < 1) || (index >= pReg->objects.Num()) ) { 
                        Awrite( "Incorrect index." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken ); 
    
                    int i = 0; 
                    Object *tmp = (Object *)pReg->objects.First(); 
                    for (i = 0; i < index; i++) tmp = (Object *)pReg->objects.Next(tmp); 
                    
                    if(!(ObjectDefs[tmp->type].hexside)) {
                        Awrite("Not a hexside object.");
                        break;
                    }
                    
                    if(!Globals->HEXSIDE_TERRAIN) {
                        Awrite("Hexside terrain disabled under game rules.");
                        break;
                    }
                    
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        Awrite( "Specify Direction." ); 
                    break; 
                    } 
                    
                    int dir=-1;
                    dir = ParseHexsideDir(pToken);
                    if(dir==-1) {
    				    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
    				    break;
                    }
                    
                    SAFE_DELETE(pToken); 
                    tmp->hexside = dir;
                }
    // rename object 
                else if (*pToken == "n") { 
                    SAFE_DELETE( pToken ); 
    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                    break; 
                    } 
    
                    int index = pToken->value(); 
                    if( (index < 1) || (index >= pReg->objects.Num()) ) { 
                        Awrite( "Incorrect index." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken ); 
    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "No name given." ); 
                        break; 
                    } 
    
                    int i = 0; 
                    Object *tmp = (Object *)pReg->objects.First(); 
                    for (i = 0; i < index; i++) tmp = (Object *)pReg->objects.Next(tmp); 
    
                    AString * newname = pToken->getlegal(); 
                    SAFE_DELETE(pToken); 
                    if (newname) { 
                        delete tmp->name; 
                        *newname += AString(" [") + tmp->num + "]"; 
                        tmp->name = newname; 
                    } 
                }
                else if (*pToken == "s") { 
                    SAFE_DELETE( pToken ); 
    
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
    
                    int index = pToken->value(); 
                    if( (index < 1) || (index >= pReg->objects.Num()) ) { 
                        Awrite( "Incorrect index." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "No description given." ); 
                        break; 
                    } 

                    int i = 0; 
                    Object *tmp = (Object *)pReg->objects.First(); 
                    for (i = 0; i < index; i++) tmp = (Object *)pReg->objects.Next(tmp); 

                    if(tmp->describe) delete tmp->describe;
                    
                    AString * newdesc = pToken->getlegal();
                    SAFE_DELETE(pToken);
                    tmp->describe = newdesc; 
                }
                else if (*pToken == "f") { 
                    SAFE_DELETE( pToken ); 
    
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
    
                    int flag = pToken->value(); 
                    pReg->flagpole = flag;
                    SAFE_DELETE( pToken );
                }

            } while( 0 );
            if(pToken) delete pToken;
        }
        if(pStr) delete pStr;

        if( exit ) {
            break;
        }
    }
    while( 1 ); 
} 

void Game::EditGameRegionTerrain( ARegion *pReg ) 
{ 
    do {
        Awrite("");
        Awrite( AString( "Region: " ) + pReg->Print( &regions ) ); 
        Awrite( "Hexsides:" );
        
        for(int i=0; i<6; i++) {
            Hexside *h = pReg->hexside[i];
            if(h->type || h->road != 0 || h->bridge != 0) {
                AString temp = AString(DirectionStrs[i]) + " : ";
                if(h->type) temp += AString(HexsideDefs[h->type].name) + ". ";
                if(h->road != 0) {
                    temp += HexsideDefs[H_ROAD].name;
    		        if(h->road > 0) temp += AString(" (needs ") + h->road + ")";
    		        temp += ". ";
                }
    		    if(h->bridge != 0) {
                    temp += HexsideDefs[H_BRIDGE].name;
    		        if(h->bridge > 0) temp += AString(" (needs ") + h->bridge + ")";
    		        temp += ". ";
                }
                Awrite(temp);
            }
        }
// write pop stuff
		Awrite( AString("") + pReg->population + " " + ItemDefs[pReg->race].names + " basepop");
		if(pReg->town) Awrite( AString("") + pReg->town->pop + " " + ItemDefs[pReg->race].names + " townpop");
		Awrite( AString("") + pReg->Population() + " " + ItemDefs[pReg->race].names + " totalpop");
		
// write wages
		Awrite(AString("Wages: ") + pReg->WagesForReport() + ".");
		Awrite(AString("Maxwages: ") + pReg->maxwages + ".");

// write products
    	AString temp = "Products: ";
    	int has = 0;
    	forlist((&pReg->products)) {
    		Production * p = ((Production *) elem);
    		if (ItemDefs[p->itemtype].type & IT_ADVANCED) {
    			if (has) {
    				temp += AString(", ") + p->WriteReport();
    			} else {
    				has = 1;
    				temp += p->WriteReport();
    			}
    		} else {
    			if (p->itemtype == I_SILVER) {
    				if (p->skill == S_ENTERTAINMENT) {
    					Awrite (AString("Entertainment available: $") +
    								p->amount + ".");
    				}
    			} else {
    				if (has) {
    					temp += AString(", ") + p->WriteReport();
    				} else {
    					has = 1;
    					temp += p->WriteReport();
    				}
    			}
    		}
    	}
    	if (has==0) temp += "none";
    	temp += ".";
	    Awrite(temp);
        Awrite( "" ); 

		if(Globals->GATES_EXIST && pReg->gate && pReg->gate != -1) {
			Awrite(AString("There is a Gate here (Gate ") + pReg->gate +
				" of " + (regions.numberofgates) + ").");
			if(Globals->GATES_NOT_PERENNIAL) Awrite(AString("This gate opens "
                "in month ") + pReg->gatemonth);
			Awrite("");
		}
		if(Globals->ARCADIA_MAGIC && pReg->willsink) {
		    Awrite(AString("This region will sink in ") + pReg->willsink + " months.");
		}


        Awrite( " [t] [terrain type] to modify terrain type" ); 
        Awrite( " [r] [race] to modify local race" );         
        Awrite( " [w] [maxwages] to modify local wages" );
        Awrite( " [p] to regenerate products according to terrain type" );
        Awrite( " [g] to regenerate all according to terrain type" );
        if(pReg->gate > 0) Awrite( " [dg] to delete the gate in this region" );
        else Awrite( " [ag] to add a gate to this region" );
        Awrite( " [n] [name] to modify region name" );
        Awrite( " [ncopy] to copy this region's name to all similar nearby regions" );
        if(Globals->HEXSIDE_TERRAIN) {
            Awrite( " [h] [dir] [type] to add/modify a hexside type ");
            Awrite( " [dh] [dir] to clear a hexside ");
        }
        if(Globals->ARCADIA_MAGIC) Awrite( " [sink] [num] to set sink status ");
        if(pReg->town) {
        Awrite( " [town] to regenerate a town" ); 
        Awrite( " [deltown] to remove a town" );
        Awrite( " [tn] [name] to rename a town" );
        Awrite( " [v] to view/modify town markets" );
        } else Awrite( " [town] to add a town" ); 
        Awrite( " q) Return to previous menu." ); 

        int exit = 0; 
        AString *pStr = AGetString(); 
        if( *pStr == "q" ) { 
            exit = 1; 
        } else { 
            AString *pToken = 0; 
            do { 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                Awrite( "Try again." ); 
                break; 
            } 

            // modify terrain
            if (*pToken == "t") { 
                SAFE_DELETE( pToken ); 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break; 
                } 

                int terType = ParseTerrain(pToken);
                if(terType == -1) { 
                    Awrite( "No such terrain." ); 
                    break; 
                } 
                SAFE_DELETE( pToken ); 
                int wasocean = 0;
                int toocean = 0;
                if(TerrainDefs[pReg->type].similar_type == R_OCEAN) wasocean = 1;
                if(TerrainDefs[terType].similar_type == R_OCEAN) toocean = 1;
                
                if(wasocean && !toocean) pReg->OceanToLand();
                if(toocean && !wasocean) pReg->SinkRegion(&regions);
                else pReg->type = terType;
                
            } 
            else if (*pToken == "r") { 
                SAFE_DELETE( pToken ); 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break; 
                } 

                int prace = ParseAllItems(pToken);
                if(!(ItemDefs[prace].type & IT_MAN) || (ItemDefs[prace].flags & ItemType::DISABLED) ) {
                    Awrite( "No such race." ); 
                    break;
                }
                if(prace>0) pReg->race = prace;
                pReg->UpdateEditRegion();
                SAFE_DELETE( pToken );
            }
            else if (*pToken == "h") { 
                SAFE_DELETE( pToken ); 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Specify a direction." ); 
                    break; 
                } 
                int dir=-1;
                dir = ParseHexsideDir(pToken);
                SAFE_DELETE( pToken ); 
                if(dir < 0 || dir > 5) {
				    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
				    break;
                }
                Hexside *h = pReg->hexside[dir];
                if(!h) {
                    Awrite("Hexside not present, gamefile corrupted.");
                    break;
                }
                pToken = pStr->gettoken();
                if(!pToken) {
                    Awrite("Try Again!");
                    break;
                }
                int type = ParseHexside(pToken);
                SAFE_DELETE( pToken ); 
                if(type < 0) {
                    Awrite("Try Again!");
                    break;
                }
                pToken = pStr->gettoken();
                if(type == H_BRIDGE) {
                    if(pToken) {
                        h->bridge = pToken->value();
                    } else {
                        if(h->bridge) h->bridge = 0;
                        else h->bridge = -1;
                    }
                } else if (type == H_ROAD) {
                    if(pToken) {
                        h->road = pToken->value();
                    } else {
                        if(h->road) h->road = 0;
                        else h->road = -1;
                    }
                } else {
                    h->type = type;
                }
            }
            else if (*pToken == "dh") { 
                SAFE_DELETE( pToken ); 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Specify a direction." ); 
                    break; 
                } 
                int dir=-1;
                dir = ParseHexsideDir(pToken);
                SAFE_DELETE( pToken ); 
                if(dir < 0 || dir > 5) {
				    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
				    break;
                }
                Hexside *h = pReg->hexside[dir];
                if(!h) {
                    Awrite("Hexside not present, gamefile corrupted.");
                    break;
                }
                h->bridge = 0;
                h->road = 0;
                h->type = 0;
            }
            else if (*pToken == "sink") { 
                SAFE_DELETE( pToken ); 
                pToken = pStr->gettoken();
                int months = 0;
                if(pToken) months = pToken->value();
                pReg->willsink = months;            
            }
            else if (*pToken == "dg") { 
                SAFE_DELETE( pToken ); 
                if(pReg->gate > 0) {
                    int numgates = regions.numberofgates;
                    int found = 0;
                    forlist(&regions) {
                        ARegion *reg = (ARegion *) elem;
                        if (reg->gate == numgates) {
                            reg->gate = pReg->gate;
                            pReg->gate = 0;
                            regions.numberofgates--;
                            found = 1;
                            break;
                        }
                    }
                    if(!found) Awrite("Error: Could not find last gate");
                }
            }
            else if (*pToken == "ag") { 
                SAFE_DELETE( pToken );
                if(pReg->gate > 0) break;
                regions.numberofgates++;                
                int gatenum = getrandom(regions.numberofgates) + 1;
                if(gatenum != regions.numberofgates) {
                    forlist(&regions) {
                        ARegion *reg = (ARegion *) elem;
                        if(reg->gate == gatenum) reg->gate = regions.numberofgates;
                    }
                }
                pReg->gate = gatenum;
                pReg->gatemonth = getrandom(12);                
            }
            else if (*pToken == "w") {
                SAFE_DELETE( pToken );
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break; 
                } 
                int val = pToken->value();
                SAFE_DELETE( pToken );
                if(val) {
                    int change = val - pReg->maxwages;
                    pReg->maxwages = val;
                    pReg->wages += change;
                }  
                pReg->UpdateEditRegion();
            }                     
            else if (*pToken == "p") { 
                SAFE_DELETE(pToken);
                forlist((&pReg->products)) {
                    Production * p = ((Production *) elem);
                    if(p->itemtype!=I_SILVER) {
                        pReg->products.Remove(p);
                        delete p;
                    }
                }
                pReg->SetupProds();
            }
            else if (*pToken == "g") { 
                SAFE_DELETE(pToken);

                if(pReg->town) delete pReg->town;
                pReg->town = NULL;

                pReg->products.DeleteAll();
                pReg->SetupProds();
                
                pReg->markets.DeleteAll();

                pReg->SetupEditRegion();
	            pReg->UpdateEditRegion();                
            }
            else if (*pToken == "n") { 
                SAFE_DELETE(pToken);
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break; 
                } 
                
                *pReg->name = *pToken;
                SAFE_DELETE(pToken);                
            }
            else if (*pToken == "ncopy") {
                SAFE_DELETE(pToken); 
                AString text = *pReg->name;
                *pReg->name = AString("");
                pReg->EditAdjustAreaName(text);
            }
            else if (*pToken == "tn") { 
                SAFE_DELETE(pToken);
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break; 
                } 
                
                if(pReg->town) *pReg->town->name = *pToken;
                SAFE_DELETE(pToken);                
            }                
            else if (*pToken == "town") { 
                SAFE_DELETE(pToken);
                
                if(pReg->race<0) pReg->race = 9;
                
                AString *townname = new AString("name");
                if(pReg->town) {
                    *townname = *pReg->town->name;
                    delete pReg->town;
                    pReg->markets.DeleteAll();
                }
                pReg->AddEditTown(townname);
	            
	            pReg->UpdateEditRegion(); // financial stuff! Does markets
            }
            else if (*pToken == "deltown") { 
                SAFE_DELETE(pToken);
                if(pReg->town) {
                    delete pReg->town;
                    pReg->town = NULL;
                    pReg->markets.DeleteAll();
                    pReg->UpdateEditRegion();
                }
            }
            else if (*pToken == "v") {
			    if(pReg->town) EditGameRegionMarkets(pReg);
			}
        } while( 0 ); 
        if(pToken) delete pToken; 
    } 
    if(pStr) delete pStr; 

    if( exit ) { 
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
        Awrite( AString( "Region: " ) + pReg->Print( &regions ) );
        Awrite( "" ); 
// write pop stuff
		Awrite( AString("") + pReg->town->pop + " " + ItemDefs[pReg->race].names + " townpop");
		
//write markets
        Awrite(AString("Market Format: ... price(base). minpop/maxpop. minamt/maxamt."));

    	Awrite("Wanted: ");
    	forlist(&pReg->markets) {
    		Market *m = (Market *) elem;
    		if (m->type == M_SELL) {
    		    AString temp = ItemString(m->item, m->amount) + " at $" + m->price + "(" + m->baseprice + ").";
    		    temp += AString(" Pop: ") + m->minpop + "/" + m->maxpop + ".";
    		    temp += AString(" Amount: ") + m->minamt + "/" + m->maxamt + ".";
    		    Awrite(temp);
    		}
    	}
    	Awrite("For Sale: ");	
    	forlist_reuse(&pReg->markets) {
    		Market *m = (Market *) elem;
    		if (m->type == M_BUY) {
    		    AString temp = ItemString(m->item, m->amount) + " at $" + m->price + "(" + m->baseprice + ").";
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
        Awrite( " [s] [item] to swop an item between wanted and sold" );
        Awrite( " [d] [item] to delete an item from the market" );
        Awrite( " q) Return to previous menu." ); 

        int exit = 0; 
        AString *pStr = AGetString(); 
        if( *pStr == "q" ) { 
            exit = 1; 
        } else { 
            AString *pToken = 0; 
            do { 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break; 
                } 
    
                // regenerate markets
                if (*pToken == "g") { 
                    SAFE_DELETE(pToken);
    
                    pReg->markets.DeleteAll();
                    pReg->SetupCityMarket();
    	            pReg->UpdateEditRegion();
                }
                else if (*pToken == "p") { 
                    SAFE_DELETE(pToken);
                    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int mitem = ParseEnabledItem(pToken);
                    if(mitem<0) {
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
                    
                    if(minimum + 1 > maximum) {
                        Awrite("Maximum must be more than minimum");
                        break;
                    }
    
                    int population = pReg->Population();
                    
                    forlist(&pReg->markets) {
                        Market *m = (Market *) elem;
                        if(m->item == mitem) {
                            m->minpop = minimum;
                            m->maxpop = maximum;
    
                        	if (population <= m->minpop)
                        		m->amount = m->minamt;
                        	else {
                        		if (population >= m->maxpop)
                        			m->amount = m->maxamt;
                        		else {
                        			m->amount = m->minamt + ((m->maxamt - m->minamt) *
                        					(population - m->minpop)) /
                        				(m->maxpop - m->minpop);
                        		}
                        	}
                            done = 1;
                        }
                    }
    
                    if(!done) {
    					int price = (ItemDefs[mitem].baseprice * (100 + getrandom(50))) /
    						100;                
                        Market *m = new Market(M_SELL, mitem, price, 0, minimum, maximum, 0, 0);
    //                    m->PostTurn(pReg->Population(),pReg->Wages());  // updates amounts
                        pReg->markets.Add(m);
                    }
    
                }
                else if (*pToken == "a") { 
                    SAFE_DELETE(pToken);
                    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int mitem = ParseEnabledItem(pToken);
                    if(mitem<0) {
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
                    
                    if(minimum + 1 > maximum) {
                        Awrite("Maximum must be more than minimum");
                        break;
                    }
                    
                    int population = pReg->Population();
                    
                    forlist(&pReg->markets) {
                        Market *m = (Market *) elem;
                        if(m->item == mitem) {
                            m->minamt = minimum;
                            m->maxamt = maximum;
                            
                        	if (population <= m->minpop)
                        		m->amount = m->minamt;
                        	else {
                        		if (population >= m->maxpop)
                        			m->amount = m->maxamt;
                        		else {
                        			m->amount = m->minamt + ((m->maxamt - m->minamt) *
                        					(population - m->minpop)) /
                        				(m->maxpop - m->minpop);
                        		}
                        	}
                            done = 1;
                        }
                    }
    
                    if(!done) {
    					int price = (ItemDefs[mitem].baseprice * (100 + getrandom(50))) /
    						100;    
                        int mamount = minimum + ( maximum * population / 5000 );
                        Market *m = new Market(M_SELL, mitem, price, mamount, 0, 5000, minimum, maximum);
                        pReg->markets.Add(m);
                    }
    
                }   
                else if (*pToken == "c") { 
                    SAFE_DELETE(pToken);
                    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int mitem = ParseEnabledItem(pToken);
                    if(mitem<0) {
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
    
                    if(price<1 || baseprice<1) {
                        Awrite("Price must be more than zero");
                        break;
                    }
    
                    int done = 0;
                    
                    
                    forlist(&pReg->markets) {
                        Market *m = (Market *) elem;
                        if(m->item == mitem) {
                            m->price = price;
                            m->baseprice = baseprice;
                            done = 1;
                        }
                    }
    
                    if(!done) {
                        Market *m = new Market(M_SELL, mitem, price, 0, 0, 5000, 0, 0);
                        m->baseprice = baseprice;
                        pReg->markets.Add(m);
                    }
    
                }                     
                else if (*pToken == "s") { 
                    SAFE_DELETE(pToken);
                    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int mitem = ParseEnabledItem(pToken);
                    if(mitem<0) {
                        Awrite("No such item");
                        break;
                    }
                    SAFE_DELETE( pToken );
                    
                    int done = 0;
                    forlist(&pReg->markets) {
                        Market *m = (Market *) elem;
                        if(m->item == mitem) {
                            if(!done) {
                                if(m->type == M_SELL) m->type = M_BUY;
                                else m->type = M_SELL;
                                done = 1;  
                            } else {
                                pReg->markets.Remove(m);
                                delete m;
                            }
                        }
                    }                
                    if(!done) Awrite("No such market");                
                }
                else if (*pToken == "d") { 
                    SAFE_DELETE(pToken);
                    
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int mitem = ParseEnabledItem(pToken);
                    if(mitem<0) {
                        Awrite("No such item");
                        break;
                    }
                    SAFE_DELETE( pToken );
                    
                    int done = 0;
                    forlist(&pReg->markets) {
                        Market *m = (Market *) elem;
                        if(m->item == mitem) {
                            pReg->markets.Remove(m);
                            delete m;
                            done = 1;  
                        }
                    }                
                    if(!done) Awrite("No such market");                
                }            
            } while( 0 ); 
            if(pToken) delete pToken; 
        } 
        if(pStr) delete pStr; 
    
        if( exit ) { 
            break; 
        } 
    } while( 1 ); 
}


void Game::EditGameRegionExits(ARegion *pReg)
{
	do {
        Awrite("");
        Awrite( AString( "Region: " ) + pReg->Print( &regions ) ); 
        Awrite("");
        Awrite("Exits:");
        
        int y=0;    
    	for (int i=0; i<NDIRS; i++) {
    		ARegion *r = pReg->neighbors[i];
    		if (r) {
    		    AString temp = AString(DirectionStrs[i]) + ": " + r->Print(&regions);

    		    Hexside *h = pReg->hexside[i];
    		    if(h && h->type) temp += AString(", ") + HexsideDefs[h->type].name;
    		    if(h && h->road != 0) {
                    temp += AString(", ") + HexsideDefs[H_ROAD].name;
    		        if(h->road > 0) temp += AString(" (needs ") + h->road + ")";
                }
    		    if(h && h->bridge != 0) {
                    temp += AString(", ") + HexsideDefs[H_BRIDGE].name;
    		        if(h->bridge > 0) temp += AString(" (needs ") + h->bridge + ")";
                }
    		    temp += ".";
    		    
    		    int compdir = (i+3)%6;
    		    if(r->neighbors[compdir] && r->neighbors[compdir] == pReg) temp += " (two-way).";
    		    else temp += " (one-way).";

    			Awrite(temp);
    			y = 1;
    		}
    		else {
    		    if(pReg->hexside[i]->type) {
        		    Hexside *h = pReg->hexside[i];
        		    AString temp = AString(DirectionStrs[i]) + " : ";
        		    if(h && h->type) temp += HexsideDefs[h->type].name;
        		    
    		        temp += ".";
    		    
    			    Awrite(temp);
    			    y = 1;
    		    }
    		}
    	}
    	if (!y) Awrite("none");

        Awrite( "" ); 
        Awrite( " [d] [dir] to remove that connection." ); 
        Awrite( " [a] [dir] to add a default (two-way) connection." ); 
        Awrite( " [s] [dir] [x] [y] [z] to add a specific (one-way) connection to region x,y,z." );
        Awrite( " q) Return to previous menu." ); 

        int exit = 0;
        AString *pStr = AGetString();
        if( *pStr == "q" ) {
            exit = 1; 
        } else { 
            AString *pToken = 0; 
            do { 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                    Awrite( "Try again." ); 
                    break; 
                }

                // add object 
                if (*pToken == "d") { 
                    SAFE_DELETE( pToken ); 
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
                    int dir = ParseHexsideDir(pToken);
                    if(dir<0) {
					    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
					    break;
                    }
                    SAFE_DELETE( pToken );

                    ARegion *regiontwo = pReg->neighbors[dir];
                    if(!regiontwo) {
                        Awrite("There is no connection in that direction.");
                        break;
                    }
                    //Have to remove the link, and create a new hexside.
                    int compdir = pReg->GetRealDirComp(dir);
                    if(regiontwo->neighbors[compdir] == pReg) {
                        regiontwo->neighbors[compdir] = 0;
                        regiontwo->hexside[compdir]->type = H_DUMMY;
                        regiontwo->hexside[compdir]->bridge = 0;
                        regiontwo->hexside[compdir]->road = 0;
                        Hexside *temp = new Hexside;
			            pReg->hexside[dir] = temp;
                    }
                    pReg->neighbors[dir] = 0;


                }
                // add connection
                else if (*pToken == "a") { 
                    SAFE_DELETE( pToken ); 
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
                    int dir = ParseHexsideDir(pToken);
                    if(dir<0) {
					    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
					    break;
                    }
                    SAFE_DELETE( pToken );

                    if(pReg->neighbors[dir]) {
                        Awrite("There is already a connection in that direction.");
                        break;
                    }
                    
                    ARegion *regiontwo = 0;
                    ARegionArray *ar = regions.GetRegionArray(pReg->zloc);
                    //Have to add a link, and delete one of the hexsides.
                    switch(dir) {
                        case D_NORTH:
                            if(pReg->yloc >=2 ) regiontwo = ar->GetRegion(pReg->xloc, pReg->yloc - 2);
                            break;
                        case D_NORTHEAST:
                            if(pReg->yloc >=1 ) regiontwo = ar->GetRegion(pReg->xloc + 1, pReg->yloc - 1);
                            break;
                        case D_NORTHWEST:
                            if(pReg->yloc >=1 ) regiontwo = ar->GetRegion(pReg->xloc - 1, pReg->yloc - 1);
                            break;
                        case D_SOUTHEAST:
                            if(pReg->yloc < ar->y - 1 ) regiontwo = ar->GetRegion(pReg->xloc + 1, pReg->yloc + 1);
                            break;
                        case D_SOUTHWEST:
                            if(pReg->yloc < ar->y - 1 ) regiontwo = ar->GetRegion(pReg->xloc - 1, pReg->yloc + 1);
                            break;
                        case D_SOUTH:
                            if(pReg->yloc < ar->y - 2 ) regiontwo = ar->GetRegion(pReg->xloc, pReg->yloc + 2);
                            break;
                        default:
                            Awrite("Unrecognised direction.");
                            break;
                    }
                    if(!regiontwo) {
                        Awrite("No default connection can be found.");
                        break;
                    }
                    int compdir = (dir+3)%6;
                    if(regiontwo->neighbors[compdir]) {
                        Awrite("Default neighbour is already connected.");
                        break;                    
                    }
                    //setup neighbours
                    regiontwo->neighbors[compdir] = pReg;
                    pReg->neighbors[dir] = regiontwo;
                    //combine hexsides
                    delete regiontwo->hexside[compdir];  // since this was not connected, no other region should point to this
                    regiontwo->hexside[compdir] = pReg->hexside[dir];
                }
                else if (*pToken == "s") { 
                    SAFE_DELETE( pToken ); 
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
                    int dir = ParseHexsideDir(pToken);
                    if(dir<0) {
					    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
					    break;
                    }
                    SAFE_DELETE( pToken );
                    if(pReg->neighbors[dir]) {
                        Awrite("There is already a connection in that direction.");
                        break;
                    }
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
                    int x = pToken->value();
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
                    int y = pToken->value();
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
                    int z = pToken->value();
                    SAFE_DELETE( pToken );
                    
                    ARegion *regiontwo = regions.GetRegion(x,y,z);
                    if(!regiontwo) {
                        Awrite( "Cannot find that region." );
                        break;
                    }
                    
                    pReg->neighbors[dir] = regiontwo;
                    int compdir = pReg->GetRealDirComp(dir);
                    if(regiontwo->neighbors[compdir] == pReg) {
                        //this linkage is two-way. Combine hexsides if they are different.
                        if(pReg->hexside[dir] != regiontwo->hexside[compdir]) {
                            delete regiontwo->hexside[compdir];
                            regiontwo->hexside[compdir] = pReg->hexside[dir];
                        }
                    }
                }
            } while( 0 ); 
            if(pToken) delete pToken; 
        }
        
        if(pStr) delete pStr; 
        if( exit ) break; 
	} while(1);
}

ARegion * Game::EditGameRegionNavigate(ARegion *pReg)
{
	do {
        Awrite("");
        Awrite( AString( "Region: " ) + pReg->Print( &regions ) ); 
        Awrite("");
        Awrite("Exits:");
        
        int y=0;    
    	for (int i=0; i<NDIRS; i++) {
    		ARegion *r = pReg->neighbors[i];
    		if (r) {
    		    AString temp = AString(DirectionStrs[i]) + " : " + r->Print(&regions);
    
    		    Hexside *h = pReg->hexside[i];
    		    if(h && h->type) temp += AString(", ") + HexsideDefs[h->type].name;
    		    if(h && h->road != 0) {
                    temp += AString(", ") + HexsideDefs[H_ROAD].name;
    		        if(h->road > 0) temp += AString(" (needs ") + h->road + ")";
                }
    		    if(h && h->bridge != 0) {
                    temp += AString(", ") + HexsideDefs[H_BRIDGE].name;
    		        if(h->bridge > 0) temp += AString(" (needs ") + h->bridge + ")";
                }
    		    temp += ".";
    		    
    			Awrite(temp);
    			y = 1;
    		}
    		else {
    		    if(pReg->hexside[i]->type) {
        		    Hexside *h = pReg->hexside[i];
        		    AString temp = AString(DirectionStrs[i]) + " : ";
        		    if(h && h->type) temp += HexsideDefs[h->type].name;
        		    
    		        temp += ".";
    		    
    			    Awrite(temp);
    			    y = 1;
    		    }
    		}
    	}
    	if (!y) Awrite("none");

        Awrite( "" ); 
        Awrite( " [dirs] to move to the region in that direction(s)." ); 
        Awrite( " q) Return to previous menu." );

        int exit = 0;
        AString *pStr = AGetString();
        if( *pStr == "q" ) {
            exit = 1; 
        } else {
            pReg = EditGameRegionNavigate(pReg, pStr);
        }

        if(pStr) delete pStr; 
        if( exit ) break; 
	} while(1);
	return pReg;
}

ARegion * Game::EditGameRegionNavigate(ARegion *pReg, AString *pStr)
{
    AString *pToken = 0;
    pToken = pStr->gettoken();
    if( !pToken ) { 
        Awrite( "Try again." ); 
        return pReg; 
    }

    do { 
        int dir = ParseHexsideDir(pToken);
        if(dir<0) {
		    Awrite("Incorrect direction. Use N,NE,SE,S,SW,NW");
		    break;
        }
        SAFE_DELETE( pToken );

        ARegion *regiontwo = pReg->neighbors[dir];
        if(!regiontwo) {
            Awrite("There is no connection in that direction.");
            break;
        }
        pReg = regiontwo;
        pToken = pStr->gettoken();
    } while( pToken );
    
    if(pToken) delete pToken;
    //pStr deleted in calling function
    return pReg;
}



void Game::EditGameUnit(Unit *pUnit)
{
	do {
		Awrite(AString("Unit: ") + *(pUnit->name));
		Awrite(AString("Faction: ") + pUnit->faction->num);
		Awrite(AString("  in ") +
				pUnit->object->region->ShortPrint(&regions));
		Awrite("  1) Edit items...");
		Awrite("  2) Edit skills...");
		Awrite("  3) Move unit...");
		Awrite("  4) Edit details...");	
		
		Awrite("  q) Stop editing this unit.");

		int exit = 0;
		AString *pStr = AGetString();
		if(*pStr == "1") {
			EditGameUnitItems(pUnit);
		} else if(*pStr == "2") {
			EditGameUnitSkills(pUnit);
		} else if(*pStr == "3") {
			EditGameUnitMove(pUnit);
		} else if(*pStr == "4") {
			EditGameUnitDetails(pUnit);			
		} else if(*pStr == "q") {
			exit = 1;
		} else {
			Awrite("Select from the menu.");
		}
		delete pStr;

		if(exit) {
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
		if(*pStr == "q") {
			exit = 1;
		} else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if(!pToken) {
					Awrite("Try again.");
					break;
				}

				int itemNum = ParseAllItems(pToken);
				if(itemNum == -1) {
					Awrite("No such item.");
					break;
				}
				if(ItemDefs[itemNum].flags & ItemType::DISABLED) {
					Awrite("No such item.");
					break;
				}
				SAFE_DELETE(pToken);

				int num;
				pToken = pStr->gettoken();
				if(!pToken) {
					num = 0;
				} else {
					num = pToken->value();
				}

				pUnit->items.SetNum(itemNum, num);
				/* Mark it as known about for 'shows' */
				pUnit->faction->items.SetNum(itemNum, 1);
			} while(0);
			if(pToken) delete pToken;
		}
		if(pStr) delete pStr;

		if(exit) {
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
		if(*pStr == "q") {
			exit = 1;
		} else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if(!pToken) {
					Awrite("Try again.");
					break;
				}

				int skillNum = ParseSkill(pToken);
				if(skillNum == -1) {
					Awrite("No such skill.");
					break;
				}
				if(SkillDefs[skillNum].flags & SkillType::DISABLED) {
					Awrite("No such skill.");
					break;
				}
				SAFE_DELETE(pToken);

				int days;
				pToken = pStr->gettoken();
				if(!pToken) {
					days = 0;
				} else {
					days = pToken->value();
				}
				SAFE_DELETE(pToken);
				
				int exper;
				pToken = pStr->gettoken();
				if(!pToken || !Globals->REAL_EXPERIENCE) {
					exper = 0;
				} else {
					exper = pToken->value();
				}
				
				if((SkillDefs[skillNum].flags & SkillType::MAGIC) &&
						(pUnit->type != U_MAGE)) {
				}
				if((SkillDefs[skillNum].flags & SkillType::APPRENTICE) &&
						(pUnit->type == U_NORMAL)) {
					pUnit->type = U_APPRENTICE;
				}
				pUnit->skills.SetDays(skillNum, days * pUnit->GetMen(), exper * pUnit->GetMen());
				int lvl = pUnit->GetRealSkill(skillNum);
				if(lvl > pUnit->faction->skills.GetDays(skillNum)) {
					pUnit->faction->skills.SetDays(skillNum, lvl);
				}
			} while(0);
			delete pToken;
		}
		delete pStr;

		if(exit) {
			break;
		}
	} while(1);
}

void Game::EditGameUnitMove(Unit *pUnit)
{
	ARegion *pReg = EditGameFindRegion();
	if(!pReg) return;

	pUnit->MoveUnit(pReg->GetDummy());
}

void Game::EditGameUnitDetails(Unit *pUnit)
{
	do {
		int exit = 0;
		Awrite(AString("Unit: ") + *(pUnit->name));
		if(pUnit->describe) Awrite(AString("Description: ") + *(pUnit->describe));
		Awrite(AString("Unit faction: ") +
				*(pUnit->faction->name));
		AString temp;
		switch(pUnit->type) {
		    case U_NORMAL:
		        temp = AString(" (normal)");
		        break;
		    case U_SPECIALIST:
		        temp = AString(" (specialist)");
		        break;
		    case U_LEADER:
		        temp = AString(" (leader)");
		        break;
            case U_MAGE:
		        temp = AString(" (mage)");
		        break;
		    case U_GUARD:
		        temp = AString(" (guard)");
		        break;
		    case U_WMON:
		        temp = AString(" (monster)");
		        break;		        
            case U_GUARDMAGE:
		        temp = AString(" (guardmage)");
		        break;
            case U_APPRENTICE:
		        temp = AString(" (apprentice)");
		        break;		        
		}
		Awrite(AString("Unit type: ") + pUnit->type + temp);
		if(pUnit->type == U_MAGE && Globals->ARCADIA_MAGIC) {
		    temp = AString("Unit mortality: ");
		    if(pUnit->dead) temp += "dead";
		    else temp += "alive";
		    Awrite(temp);
		    Awrite(AString("Unit energy: ") + pUnit->energy + "/" + pUnit->MaxEnergy());
		    if(pUnit->resurrects) Awrite(AString("This unit has been resurrected ") + pUnit->resurrects + " times.");
		    AString temp = "none";
		}
		Awrite(AString("Unit is in object ") + *pUnit->object->name + " : " + ObjectDefs[pUnit->object->type].name);
		
		Awrite("");
		Awrite("  [f] [num] to change the unit's faction.");
		Awrite("  [t] [num] to change the unit's type.");
		Awrite("  [n] [name] to change the unit's name.");
		Awrite("  [c] [description] to change the unit's description.");
		Awrite("  [enter] [num] to enter an object.");
		if(pUnit->type == U_MAGE) {
            Awrite("  [d] to change the unit's death status.");
		    Awrite("  [e] [num] to change the unit's energy.");
        }
		Awrite("  [q] Go back one screen.");
		
		AString *pStr = AGetString();
		if(*pStr == "q") {
			exit = 1;
		} 
        else {
			AString *pToken = 0;
			do {
				pToken = pStr->gettoken();
				if(!pToken) {
					Awrite("Try again.");
					break;
				}
                // change faction
                if (*pToken == "f") { 
                    SAFE_DELETE( pToken ); 
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int fnum = pToken->value();
                    SAFE_DELETE( pToken );                     
                    if(fnum<1) {
                        Awrite("Invalid Faction Number");
                        break;
                    }
                    Awrite("fetching faction");
                    Faction *fac = GetFaction(&factions, fnum);
                    Awrite("back");
                    if(fac) pUnit->faction = fac;
                    else Awrite("Cannot Find Faction");
                    Awrite("done");
                }
                else if (*pToken == "d") { 
                    SAFE_DELETE( pToken ); 
                    if(pUnit->type != U_MAGE) {
                        Awrite("Warning: This only has an effect on mage units.");
                    }
                    if(!pUnit->dead) {
                        pUnit->dead = 1; /* If the unit is not a mage, this will not get saved in the game.out file 
                                                         and will thus be reset to zero when the game is loaded */
                        Faction *fac = GetFaction(&factions, ghostfaction);
                        if(fac) pUnit->faction = fac;
                        else Awrite("Warning: Cannot Find Ghost Faction");
                    } else {
                        pUnit->dead = 0;
                         Faction *fac = GetFaction(&factions, guardfaction);
                        if(fac) pUnit->faction = fac;
                        else Awrite("Warning: Cannot Find Guard Faction");
                    }
                    
                }
                else if (*pToken == "e") { 
                    SAFE_DELETE( pToken ); 
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int newenergy = pToken->value();
                    if(newenergy > pUnit->MaxEnergy()) newenergy = pUnit->MaxEnergy();
                    
                    if(pUnit->type == U_MAGE) {
                        pUnit->energy = newenergy;
                    }
                }
                else if (*pToken == "t") { 
                    SAFE_DELETE( pToken ); 
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int newtype = pToken->value();
                    SAFE_DELETE( pToken );                     
                    if(newtype<0 || newtype>NUNITTYPES-1) {
                        Awrite("Invalid Type");
                        break;
                    }
                    pUnit->type = newtype;
                }
                else if (*pToken == "n") {
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if( !pToken ) {
                        Awrite( "Try again." );
                        break;
                    }
                    AString *name = new AString(*pToken);
                    SAFE_DELETE( pToken );
                    pUnit->SetName(name);
                }
                else if (*pToken == "c") {
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if( !pToken ) {
                        Awrite( "Try again." );
                        break;
                    }
                    AString *desc = new AString(*pToken);
                    SAFE_DELETE( pToken );
                    pUnit->SetDescribe(desc);
                }
                else if (*pToken == "enter") {
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if( !pToken ) {
                        Awrite( "Try again." );
                        break;
                    }
                    int objnum = pToken->value();
                    SAFE_DELETE( pToken );
                    Object *to = pUnit->object->region->GetObject(objnum);
                    if(!to) {
                        Awrite("No such object");
                        break;
                    }
                    pUnit->MoveUnit(to);                    
                }
			} while(0);
			delete pToken;
		}
		delete pStr;

		if(exit) {
			break;
		}
	} while(1);
}

void Game::EditGameCreateUnit()
{
    Faction *fac = GetFaction(&factions, 1);
    Unit *newunit = GetNewUnit(fac);
    newunit->SetMen(I_LEADERS, 1);
    newunit->reveal = REVEAL_FACTION;
    newunit->MoveUnit(((ARegion *) regions.First())->GetDummy());

	EditGameUnit(newunit);
}

void Game::EditGameGlobalEffects()
{
    do {
        Awrite("");
        

        if(Globals->ARCADIA_MAGIC) Awrite( " [sink] [num] to set all land regions to sink" );
        if(Globals->HEXSIDE_TERRAIN) Awrite( " [remove] [feature] to remove rivers, bridges or roads ");
        Awrite( " [ocean] [level] to convert the specified level to ocean. ");
        Awrite( " [clearbarriers] [level] to reset all neighbours in the specified level. ");
        Awrite( " [renumber] to renumber all buildings (do after extensive map design). ");
        Awrite( " [buildingseq] to set all buildingseqs to minimum allowed value without changing building numbers" );
        Awrite( " [markets] [cycles] to regenerate all markets in the world. Use cycles for advanced balancing." );
        Awrite( " [addpop] [num] to change all town populations by [num] ");
        Awrite( " [regenproducts] [level] to regenerate all products on the specified level. ");
        Awrite( " [trade] to provide a trade summary ");
        Awrite( " [products] to provide a production summary ");
        Awrite( " [buildings] to provide a buildings summary ");
        Awrite( " [rename] [terrain] [level] [name] to rename all of a terrain type on a level. ");
        Awrite( " [importmap] [level] [filename] to set the terrain/cities of a level according to a text file.");
        Awrite( " [importeth] [level] [filename] to set the ethnicities/flagpoles of a level according to a text file.");
        Awrite( " [importriv] [level] [filename] to set the rivers/bridges of a level according to a text file.");
        Awrite( " [resetgates] [level] [frequency] to reset gates on specified level.");
        Awrite( " [makexan] to make Xanaxor.");
        Awrite( " q) Return to previous menu." );

        int exit = 0; 
        AString *pStr = AGetString(); 
        if( *pStr == "q" ) { 
            exit = 1; 
        } else { 
            AString *pToken = 0; 
            do { 
                pToken = pStr->gettoken(); 
                if( !pToken ) { 
                Awrite( "Try again." ); 
                break; 
                } 
                if (*pToken == "sink") { 
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int months = pToken->value();
                    SAFE_DELETE( pToken );
                    if(months < 1) {
                        Awrite("Try again!");
                        break;
                    }
                    forlist(&regions) {
                        ARegion *pReg = (ARegion *) elem;
                        if(TerrainDefs[pReg->type].similar_type != R_OCEAN && pReg->zloc == 1) 
                            pReg->willsink = months;                
                    }
                }
                else if (*pToken == "remove") {
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(*pToken == "rivers") {
                        forlist(&regions) {
                            ARegion *pReg = (ARegion *) elem;
                            for(int i=0; i<6; i++) {
                                if(pReg->hexside[i]->type == H_RIVER) pReg->hexside[i]->type = H_DUMMY;
                            }
                        }
                    } else if(*pToken == "bridges") {
                        forlist(&regions) {
                            ARegion *pReg = (ARegion *) elem;
                            for(int i=0; i<6; i++) pReg->hexside[i]->bridge = 0;
                        }
                    } else if(*pToken == "roads") {
                        forlist(&regions) {
                            ARegion *pReg = (ARegion *) elem;
                            for(int i=0; i<6; i++) pReg->hexside[i]->road = 0;
                        }
                    }			    
    			} else if (*pToken == "ocean") {
    			    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(!pToken) {
                        Awrite("Try again!");
                        break;
                    }
                    int level = pToken->value();
     
    			    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        if(r->zloc == level) {
                            r->SinkRegion(&regions);
                            r->type = R_OCEAN; //prevents lakes appearing in UW levels where there are barriers
                        }
    			    }
                } else if (*pToken == "markets")  {
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    int reduce = 0;
                    if(pToken) {
                        reduce = pToken->value();
                        SAFE_DELETE( pToken );
                    }
                    forlist(&regions) {
                        ARegion *r = (ARegion *) elem;
                        r->markets.DeleteAll();
                        if(r->town) r->SetupCityMarket();
        	            r->UpdateEditRegion();
                    }
                    int score = GetMarketTradeVariance();
                    int tries = 0;
                    while(reduce && tries++ < 500) {
                        forlist(&regions) {
                            ARegion *r = (ARegion *) elem;
                            r->markets.DeleteAll();
                            if(r->town) r->SetupCityMarket();
            	            r->UpdateEditRegion();
                        }
                        int temp = GetMarketTradeVariance();
                        if(temp < score) {
                            score = temp;
                            reduce--;
                        }
                    }
    			} else if (*pToken == "regenproducts")  {
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(!pToken) {
                        Awrite("Try again!");
                        break;
                    }
                    int level = pToken->value();
                    
                    forlist(&regions) {
                        ARegion *r = (ARegion *) elem;
                        if(r->zloc == level) {
                            forlist((&r->products)) {
                                Production * p = ((Production *) elem);
                                if(p->itemtype != I_SILVER) {
                                    r->products.Remove(p);
                                    delete p;
                                }
                            }
                            r->SetupProds();
        	            }
                    }            
    			} else if (*pToken == "clearbarriers") {
    			    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(!pToken) {
                        Awrite("Try again!");
                        break;
                    }
                    int level = pToken->value();
    
                    ARegionArray *ar = regions.GetRegionArray(level);
    
    			    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        if(r->zloc == level) regions.EditNeighSetup(r,ar);
    			    }
    			} else if (*pToken == "addpop") {
    			    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(!pToken) {
                        Awrite("Try again!");
                        break;
                    }
                    int amount = pToken->value();
    
    			    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        if(r->town) r->town->pop += amount;
    			    }
       			} else if (*pToken == "renumber") {
    			    SAFE_DELETE( pToken );
 			    
    			    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        int buildingnum = 1;
    			        forlist(&r->objects) {
    			            Object *o = (Object *) elem;
    			            if(o->type == O_DUMMY) o->num = 0;
    			            else {
                                o->num = buildingnum++;
        			            
        			            AString *newname = o->name->getlegal();
        			            AString *temp = newname->Trunc(newname->Len()-3); //temp gets deleted with newname.
                        		if(!newname) {
                        			newname = new AString("Building");
                        		}
                        		delete o->name;

                        		AString *newname2 = new AString(newname->Str());
                                *newname2 += AString(" [") + o->num + "]";
                        		delete newname;
                        		o->name = newname2;
                            }			            
    			        }
    			        r->buildingseq = buildingnum;
    			    }
    			} else if (*pToken == "rename") {
    			    SAFE_DELETE( pToken );

                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int terType = ParseTerrain(pToken);
                    if(terType == -1) { 
                        Awrite( "No such terrain." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(!pToken) {
                        Awrite("Try again!");
                        break;
                    }
                    int level = pToken->value();
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        Awrite( "No name given." ); 
                        break; 
                    }
                    
                    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        if(r->zloc == level && r->type == terType) {
                            *r->name = *pToken;
                        }
                    }
                    SAFE_DELETE(pToken);
    			} else if (*pToken == "importmap") {
    			    SAFE_DELETE( pToken );

                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int level = pToken->value();
                    if(level == -1) { 
                        Awrite( "Invalid level." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(pToken) ImportMapFile(pToken, level);
                    SAFE_DELETE( pToken );
    			} else if (*pToken == "importeth") {
    			    SAFE_DELETE( pToken );

                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int level = pToken->value();
                    if(level == -1) { 
                        Awrite( "Invalid level." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(pToken) ImportEthFile(pToken, level);
                    SAFE_DELETE( pToken );
    			} else if (*pToken == "importriv") {
    			    SAFE_DELETE( pToken );

                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    } 
                    int level = pToken->value();
                    if(level == -1) { 
                        Awrite( "Invalid level." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if(pToken) ImportRivFile(pToken, level);
                    SAFE_DELETE( pToken );
    			} else if (*pToken == "makexan") {
    			    SAFE_DELETE( pToken );

                    ImportMapFile(new AString("xanaxor.txt"), 1);
                    ImportEthFile(new AString("xanaxoreth.txt"), 1);
                    ImportRivFile(new AString("xanaxorriv.txt"), 1);
                    ImportFortFile(new AString("xanaxorfort.txt"), 1);
                    
                    int frequency = 10;
                    
    			    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        //reset building sequence
    			        int maxnum = 0;
    			        forlist(&r->objects) {
    			            Object *o = (Object *) elem;
    			            if(o->num > maxnum) maxnum = o->num;
    			        }
    			        r->buildingseq = maxnum + 1;
    			        
    			        //delete or make gates as necessary
    			        int needsgate = 0;
    			        if(TerrainDefs[r->type].similar_type != R_OCEAN) needsgate = !getrandom(frequency);
                        if(r->gate > 0 && !needsgate) {
                            //remove gate
                            int numgates = regions.numberofgates;
                            int found = 0;
                            forlist(&regions) {
                                ARegion *reg = (ARegion *) elem;
                                if (reg->gate == numgates) {
                                    reg->gate = r->gate;
                                    r->gate = 0;
                                    regions.numberofgates--;
                                    found = 1;
                                    break;
                                }
                            }
                            if(!found) Awrite("Error: Could not find last gate");
                        } else if(r->gate == 0 && needsgate) {
                            //add gate                        
                            regions.numberofgates++;                
                            int gatenum = getrandom(regions.numberofgates) + 1;
                            if(gatenum != regions.numberofgates) {
                                forlist(&regions) {
                                    ARegion *reg = (ARegion *) elem;
                                    if(reg->gate == gatenum) reg->gate = regions.numberofgates;
                                }
                            }
                            r->gate = gatenum;
                            r->gatemonth = getrandom(12);
                        }
                    }
                } else if (*pToken == "resetgates") {
    			    SAFE_DELETE( pToken );

                    pToken = pStr->gettoken(); 
                    if( !pToken ) { 
                        Awrite( "Try again." ); 
                        break; 
                    }
                    int level = pToken->value();
                    if(level == -1) { 
                        Awrite( "Invalid level." ); 
                        break; 
                    } 
                    SAFE_DELETE( pToken );
                    pToken = pStr->gettoken();
                    if( !pToken ) { 
                        Awrite( "Invalid frequency." ); 
                        break; 
                    }
                    int frequency = pToken->value();
                    SAFE_DELETE( pToken );
                    if( frequency < 1 ) { 
                        Awrite( "Invalid frequency." ); 
                        break; 
                    }
                    
    			    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        int needsgate = 0;
    			        if(TerrainDefs[r->type].similar_type != R_OCEAN) needsgate = !getrandom(frequency);
                        if(r->gate > 0 && !needsgate) {
                            //remove gate
                            int numgates = regions.numberofgates;
                            int found = 0;
                            forlist(&regions) {
                                ARegion *reg = (ARegion *) elem;
                                if (reg->gate == numgates) {
                                    reg->gate = r->gate;
                                    r->gate = 0;
                                    regions.numberofgates--;
                                    found = 1;
                                    break;
                                }
                            }
                            if(!found) Awrite("Error: Could not find last gate");
                        } else if(r->gate == 0 && needsgate) {
                            //add gate                        
                            regions.numberofgates++;                
                            int gatenum = getrandom(regions.numberofgates) + 1;
                            if(gatenum != regions.numberofgates) {
                                forlist(&regions) {
                                    ARegion *reg = (ARegion *) elem;
                                    if(reg->gate == gatenum) reg->gate = regions.numberofgates;
                                }
                            }
                            r->gate = gatenum;
                            r->gatemonth = getrandom(12);
                        }
                    }
    			} else if (*pToken == "buildingseq") {
    			    SAFE_DELETE( pToken );
    			    
    			    forlist(&regions) {
    			        ARegion *r = (ARegion *) elem;
    			        int maxnum = 0;
    			        forlist(&r->objects) {
    			            Object *o = (Object *) elem;
    			            if(o->num > maxnum) maxnum = o->num;
    			        }
    			        r->buildingseq = maxnum + 1;
    			    }
    			} else if (*pToken == "trade") {
    			    SAFE_DELETE( pToken );
    			    EditGameTradeSummary();
    			} else if (*pToken == "products") {
    			    SAFE_DELETE( pToken );
    			    EditGameProductsSummary();
    			} else if (*pToken == "buildings") {
    			    SAFE_DELETE( pToken );
    			    EditGameBuildingsSummary();
    			}
            } while( 0 ); 
        if(pToken) delete pToken; 
        } 
        if(pStr) delete pStr; 
    
        if( exit ) { 
            break; 
        } 
    } 
    while( 1 ); 
} 

void Game::ImportFortFile(AString *filename, int level)
{
    Ainfile f;
	if(f.OpenByName(*filename) == -1) {
        Awrite("Could not open file.");
        return;
    }

    int yy = 0;
    AString *line = f.GetStr();

    int maxx = regions.GetRegionArray(level)->x;
    int maxy = regions.GetRegionArray(level)->y;

   while(line && yy <= maxy) {
        AString *type = line->gettoken();
        int xx = yy%2;
        
        while(type && xx <= maxx) {
            ARegion *pReg = regions.GetRegion(xx,yy,level);
            if(pReg) {
                forlist(&pReg->objects) {
                    Object *o = (Object *) elem;
                    if(o->type == O_DUMMY) continue;
                    
                    forlist(&o->units) {
            			Unit *u = (Unit *) elem;
            			pReg->Kill(u);
            		}
		            pReg->objects.Remove(o);
                }
            
                int forttype = -1;
                
                if (*type == "t") forttype = O_TOWER;
                else if (*type == "f") forttype = O_FORT;
                else if (*type == "c") forttype = O_CASTLE;
                else if (*type == "i") forttype = O_CITADEL;
                else if (*type == "m") forttype = O_MFORTRESS;
                else if (*type == "1") forttype = TerrainDefs[pReg->type].lairs[1];
                else if (*type == "2") forttype = TerrainDefs[pReg->type].lairs[2];
                else if (*type == "3") forttype = TerrainDefs[pReg->type].lairs[3];
                else if (*type == "4") forttype = TerrainDefs[pReg->type].lairs[4];
                else if (*type == "5") forttype = TerrainDefs[pReg->type].lairs[5];
                else if (*type == "6") forttype = TerrainDefs[pReg->type].lairs[6];
                
                if(forttype > -1) {
                    ARegion *pReg = regions.GetRegion(xx,yy,level);
                    pReg->MakeLair(forttype);
                    forlist(&pReg->objects) {
                        Object *o = (Object *) elem;
                        if (o->units.Num()) continue;
                        if (ObjectDefs[o->type].monster != -1) MakeLMon(o);
                        if (ObjectDefs[o->type].protect > 0) CreateFortMon(pReg,o);
                    }
                }
            }
            xx += 2;
            delete type;
            type = line->gettoken();
        }
        yy += 1;
        if(line) delete line;
        line = f.GetStr();
    }
}

void Game::ImportRivFile(AString *filename, int level)
{
    Ainfile f;
	if(f.OpenByName(*filename) == -1) {
        Awrite("Could not open file.");
        return;
    }

    int yy = 0;
    AString *line = f.GetStr();

    int maxx = regions.GetRegionArray(level)->x;
    int maxy = regions.GetRegionArray(level)->y;

    while(line && yy <= maxy) {
        AString *type = line->gettoken();
        int xx = yy%2;
        
        while(type && xx <= maxx) {
            int rivers = type->value()%10;
            int bridges = (type->value()/10)%10; //not implemented below yet
            
            ARegion *pReg = regions.GetRegion(xx,yy,level);
            //prevent stupid input making rivers in oceans
            if(TerrainDefs[pReg->type].similar_type == R_OCEAN) rivers = 0;
            if(pReg) {
                Hexside *h1 = pReg->hexside[0];
                Hexside *h2 = pReg->hexside[1];
                Hexside *h4 = pReg->hexside[2];
                if(rivers%2 && (!pReg->neighbors[0] || TerrainDefs[pReg->neighbors[0]->type].similar_type != R_OCEAN)) h1->type = H_RIVER;
                else if(h1->type == H_RIVER) h1->type = H_DUMMY;
                if((rivers%4 > 1) && (!pReg->neighbors[1] || TerrainDefs[pReg->neighbors[1]->type].similar_type != R_OCEAN)) h2->type = H_RIVER;
                else if(h2->type == H_RIVER) h2->type = H_DUMMY;
                if((rivers%8 > 3) && (!pReg->neighbors[2] || TerrainDefs[pReg->neighbors[2]->type].similar_type != R_OCEAN)) h4->type = H_RIVER;
                else if(h4->type == H_RIVER) h4->type = H_DUMMY;
            }
            xx += 2;
            delete type;
            type = line->gettoken();
        }
        yy += 1;
        if(line) delete line;
        line = f.GetStr();
    }
    Awrite("done");
}

void Game::ImportEthFile(AString *filename, int level)
{
    Ainfile f;
	if(f.OpenByName(*filename) == -1) {
        Awrite("Could not open file.");
        return;
    }

    int yy = 0;
    AString *line = f.GetStr();

    int maxx = regions.GetRegionArray(level)->x;
    int maxy = regions.GetRegionArray(level)->y;

    while(line && yy <= maxy) {
        AString *type = line->gettoken();
        int xx = yy%2;
        
        while(type && xx <= maxx) {
            int ethnicity = -1;
            
            if (*type == "d") ethnicity = RA_DWARF;
            else if (*type == "e") ethnicity = RA_ELF;
            else if (*type == "h") ethnicity = RA_HUMAN;
            else if (*type == "o") ethnicity = RA_OTHER;
            else if (*type == "r") ethnicity = getrandom(RA_NA);
            
            ARegion *pReg = regions.GetRegion(xx,yy,level);
            if(pReg) {
                if(ethnicity != -1) pReg->SetEthnicity(ethnicity, &regions);
                //extra feature flagpoling:
                char *str = type->Str();
                if(*str >= 'A' && *str <= 'Z') {
                    pReg->flagpole = 1;
//temp startup cities get named "Starter"
if(pReg->town) {
    AString *newname = new AString(EthnicityString(ethnicity) + " Start");
    delete pReg->town->name;
    pReg->town->name = newname;
}
                } else pReg->flagpole = 0;
            }
            xx += 2;
            delete type;
            type = line->gettoken();
        }
        yy += 1;
        if(line) delete line;
        line = f.GetStr();
    }
    
    Awrite("Clearing City Guards");
	forlist(&regions) {
		ARegion *reg = (ARegion *)elem;
		forlist(&reg->objects) {
			Object *obj = (Object *)elem;
			forlist(&obj->units) {
				Unit *u = (Unit *)elem;
				if(u->type == U_GUARD || u->type == U_GUARDMAGE) reg->Kill(u);
			}
		}
	}
    Awrite("Making City Guards");
    CreateCityMons();
    Awrite("Done.");
}

void Game::ImportMapFile(AString *filename, int level)
{
    Ainfile f;
	if(f.OpenByName(*filename) == -1) {
        Awrite("Could not open file.");
        return;
    }

    int yy = 0;
    AString *line = f.GetStr();

    int maxx = regions.GetRegionArray(level)->x;
    int maxy = regions.GetRegionArray(level)->y;

    while(line && yy <= maxy) {
        AString *type = line->gettoken();
        int xx = yy%2;
        
        while(type && xx <= maxx) {
            int regiontype = -1;
            int iscity = 0;                    //0 means no city, 1 means city, -1 means random
            
            if (*type == "d") regiontype = R_DESERT;
            else if (*type == "f") regiontype = R_FOREST;
            else if (*type == "j") regiontype = R_JUNGLE;
            else if (*type == "l") regiontype = R_LAKE;
            else if (*type == "m") regiontype = R_MOUNTAIN;
            else if (*type == "o") regiontype = R_OCEAN;
            else if (*type == "p") regiontype = R_PLAIN;
            else if (*type == "s") regiontype = R_SWAMP;
            else if (*type == "t") regiontype = R_TUNDRA;
            else if (*type == "a") regiontype = R_PARADISE;
            
            char *str = type->Str();
            if(*str >= 'A' && *str <= 'Z') iscity = 1;
            
            if(regiontype > -1) {
                ARegion *pReg = regions.GetRegion(xx,yy,level);
                if(pReg) pReg->RedoAs(regiontype, iscity, &regions);
            }
            xx += 2;
            delete type;
            type = line->gettoken();
        }
        yy += 1;
        if(line) delete line;
        line = f.GetStr();
    }

#define NUMREGIONNAMES 1000

    Awrite("Clearing City Guards / Setting Region Names");
	forlist(&regions) {
		ARegion *reg = (ARegion *)elem;
		AString *name = 0;
		if(TerrainDefs[reg->type].similar_type == R_OCEAN) {
		    name = new AString(Globals->WORLD_NAME);
	        *name += " Ocean";
		} else {
    		for(int i=0; i<NDIRS; i++) {
    		    if(i > 1 && i < 5) continue;
    		    if(reg->neighbors[i] && reg->neighbors[i]->type == reg->type) name = new AString(*(reg->neighbors[i]->name));
    		}
    		if(!name) name = new AString(AGetNameString(NUMREGIONNAMES + getrandom(NUMREGIONNAMES)));
		}
        reg->SetName((*name).Str());
    	delete name;
      		
		forlist(&reg->objects) {
			Object *obj = (Object *)elem;
			forlist(&obj->units) {
				Unit *u = (Unit *)elem;
				if(u->type == U_GUARD || u->type == U_GUARDMAGE) reg->Kill(u);
			}
		}
	}
    Awrite("Making City Guards");
    CreateCityMons();
    Awrite("Done.");
}


void ARegion::SetEthnicity(int ethnicity, ARegionList *pRegs)
{
Awrite(EthnicityString(ethnicity));
    int chance = getrandom(100);
    
    switch(type) {
        case R_OCEAN:
            race = I_MERFOLK;
            break;
        case R_LAKE:
            race = I_MERFOLK;
            break;
            //raiders = I_VIKING
            //ancient elves = I_DARKMAN
            //plain dwarves = I_PLAINSMAN
        case R_MOUNTAIN:
            if(ethnicity == RA_HUMAN) {
                if(chance < 80) race = I_BARBARIAN;
                else race = I_VIKING;
                if(IsCoastal() && !getrandom(10)) race = I_VIKING;
                
            } else if(ethnicity == RA_DWARF) {
                if(chance < 85) race = I_HILLDWARF;
                else race = I_UNDERDWARF;
                if(IsCoastal() && !getrandom(10)) race = I_ICEDWARF;
                
            } else if(ethnicity == RA_ELF) {
                if(chance < 70) race = I_HIGHELF;
                else if(chance < 90) race = I_DARKMAN;
                else race = I_TRIBALELF;
                if(IsCoastal() && !getrandom(2)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        case R_FOREST:
            if(ethnicity == RA_HUMAN) {
                if(chance < 60) race = I_TRIBESMAN;
                else race = I_ESKIMO;
                if(IsCoastal() && !getrandom(4)) race = I_VIKING;
            
            } else if(ethnicity == RA_DWARF) {
                if(chance < 60) race = I_UNDERDWARF;
                else if(chance < 90) race = I_ICEDWARF;
                else race = I_PLAINSMAN;
                            
            } else if(ethnicity == RA_ELF) {
                if(chance < 50) race = I_WOODELF;
                else if(chance < 90) race = I_DARKMAN;
                else race = I_TRIBALELF;
                if(IsCoastal() && !getrandom(6)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        case R_PLAIN:
            if(ethnicity == RA_HUMAN) {
                if(chance < 75) race = I_NOMAD;
                else race = I_VIKING;
                if(IsCoastal() && !getrandom(3)) race = I_VIKING;
            
            } else if(ethnicity == RA_DWARF) {
                if(chance < 75) race = I_PLAINSMAN;
                else race = I_DESERTDWARF;
                            
            } else if(ethnicity == RA_ELF) {
                if(chance < 80) race = I_HIGHELF;
                else race = I_DARKMAN;
                if(IsCoastal() && !getrandom(4)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        case R_DESERT:
            if(ethnicity == RA_HUMAN) {
                if(chance < 50) race = I_NOMAD;
                else race = I_BARBARIAN;
                if(IsCoastal() && !getrandom(6)) race = I_VIKING;
            
            } else if(ethnicity == RA_DWARF) {
                if(chance < 80) race = I_DESERTDWARF;
                else race = I_PLAINSMAN;
                            
            } else if(ethnicity == RA_ELF) {
                if(chance < 70) race = I_DARKMAN;
                else if(chance < 85) race = I_TRIBALELF;
                else race = I_HIGHELF;
                if(IsCoastal() && !getrandom(2)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        case R_JUNGLE:
            if(ethnicity == RA_HUMAN) {
                if(chance < 90) race = I_TRIBESMAN;
                else race = I_NOMAD;
                if(IsCoastal() && !getrandom(5)) race = I_VIKING;
            
            } else if(ethnicity == RA_DWARF) {
                race = I_UNDERDWARF;

            } else if(ethnicity == RA_ELF) {
                if(chance < 70) race = I_TRIBALELF;
                else race = I_WOODELF;
                if(IsCoastal() && !getrandom(10)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        case R_SWAMP:
            if(ethnicity == RA_HUMAN) {
                if(chance < 50) race = I_TRIBESMAN;
                else if(chance < 75) race = I_BARBARIAN;
                else race = I_VIKING;
            
            } else if(ethnicity == RA_DWARF) {
                race = I_UNDERDWARF;

            } else if(ethnicity == RA_ELF) {
                if(chance < 90) race = I_TRIBALELF;
                else race = I_SEAELF;
                if(IsCoastal() && !getrandom(5)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        case R_TUNDRA:
            if(ethnicity == RA_HUMAN) {
                if(chance < 90) race = I_ESKIMO;
                else race = I_TRIBESMAN;
            } else if(ethnicity == RA_DWARF) {
                if(chance < 90) race = I_ICEDWARF;
                else race = I_UNDERDWARF;
            } else if(ethnicity == RA_ELF) {
                if(chance < 60) race = I_TRIBALELF;
                else if(chance < 80) race = I_HIGHELF;
                else race = I_SEAELF;
                if(IsCoastal() && !getrandom(2)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        case R_PARADISE:
            if(ethnicity == RA_HUMAN) {
                if(chance < 60) race = I_TRIBESMAN;
                else race = I_ESKIMO;
                if(IsCoastal() && !getrandom(4)) race = I_VIKING;
            
            } else if(ethnicity == RA_DWARF) {
                if(chance < 60) race = I_UNDERDWARF;
                else if(chance < 90) race = I_ICEDWARF;
                else race = I_PLAINSMAN;
                            
            } else if(ethnicity == RA_ELF) {
                if(chance < 50) race = I_WOODELF;
                else if(chance < 90) race = I_DARKMAN;
                else race = I_TRIBALELF;
                if(IsCoastal() && !getrandom(6)) race = I_SEAELF;
                
            } else {
                race = I_ORC;
            }
            break;
        default:
            break;
    }
    
    UpdateEditRegion();
}

void ARegion::RedoAs(int tertype, int hastown, ARegionList *pRegs)
{
Awrite(TerrainDefs[tertype].name);
    //reset terrain
    int wasocean = 0;
    int toocean = 0;
    if(TerrainDefs[type].similar_type == R_OCEAN) wasocean = 1;
    if(TerrainDefs[tertype].similar_type == R_OCEAN) toocean = 1;
    
    if(wasocean && !toocean) OceanToLand();
    
    if(toocean && !wasocean) SinkRegion(pRegs);
    else type = tertype;
    
    if(town) delete town;
    town = NULL;

    products.DeleteAll();
    SetupProds();
    
    markets.DeleteAll();

    SetupEditRegion();
    UpdateEditRegion();
    
    if(hastown < 0) return;
    else if(hastown == 0 && town) {
        //remove town - for some reason this code doesn't seem to be 100% working?!
        delete town;
        town = NULL;
        markets.DeleteAll();
        UpdateEditRegion();
    } else if(hastown == 1 && !town) {
        AddEditTown();
        UpdateEditRegion();
    }
}

int Game::GetMarketTradeVariance()
{
    int sales[NITEMS];
    int buys[NITEMS];
    int numtrade = 0;

    for(int i=0; i<NITEMS; i++) {
        if(ItemDefs[i].type & IT_TRADE) {
            sales[i] = 0;
            buys[i] = 0;
            numtrade++;
        } else {
            sales[i] = -1;
            buys[i] = -1;
        }
    }

    int total = 0;

    forlist(&regions) {
        ARegion *r = (ARegion *) elem;
        if(!r->town) continue;
	    forlist(&r->markets) {
		    Market *m = (Market *) elem;
	        if(!(ItemDefs[m->item].type & IT_TRADE)) continue;
            if(m->type == M_SELL) sales[m->item]++;
            else buys[m->item]++;
            total++;
	    }
    }
    
    float average = (float) total;
    average /= (float) 2*numtrade;
    
    float variance = 0;
    
    for(int i=0; i<NITEMS; i++) {
        if(ItemDefs[i].type & IT_TRADE) {
            float diff = (float) sales[i] - average;
            variance += diff*diff;
            diff = (float) buys[i] - average;
            variance += diff*diff;
        }
    }
    
    variance -= average*average; //effect from gems not being sold.
    
    int value = (int) variance;
    Awrite(AString("Variance: ") + value);
    return value;
    
    //for Arcadia with 80 settlements, 
}

void Game::EditGameBuildingsSummary()
{
    int buildings[NOBJECTS];
    int unconnectedshafts = 0;
    
    for(int i=0; i<NOBJECTS; i++) {
        buildings[i] = 0;
    }
    
    forlist(&regions) {
        ARegion *r = (ARegion *) elem;
	    forlist(&r->objects) {
            Object *o = (Object *) elem;
            buildings[o->type]++;
            if(o->type == O_SHAFT) {
                if(o->inner == -1) unconnectedshafts++;            
            }
	    }
    }
    
    for(int i=0; i<NOBJECTS; i++) {
        if(buildings[i] == 0) continue;
        cout << ObjectDefs[i].name << "\t" << buildings[i] << endl;
        if(i == O_SHAFT) cout << "Unconnected shafts: " << unconnectedshafts << endl;
    }
}

void Game::EditGameProductsSummary()
{
    int northproducts[NITEMS];
    int westproducts[NITEMS];
    int eastproducts[NITEMS];
    int southproducts[NITEMS];
    int otherproducts[NITEMS];
    
    for(int i=0; i<NITEMS; i++) {
        northproducts[i] = 0;
        westproducts[i] = 0;
        eastproducts[i] = 0;
        southproducts[i] = 0;
        otherproducts[i] = 0;
    }
    
    forlist(&regions) {
        ARegion *r = (ARegion *) elem;
//        if(!r->town) continue;
        int island = 0;
        if(r->zloc != 1) island = 5;
        else if(r->xloc < 10 || r->xloc > 45) island = 5;
        else if(r->xloc > r->yloc) {
            if(r->xloc + r->yloc < (regions.GetRegionArray(1)->x + regions.GetRegionArray(1)->y)/2 ) island = 2;
            else island = 3;
        } else {
            if(r->xloc + r->yloc < (regions.GetRegionArray(1)->x + regions.GetRegionArray(1)->y)/2 ) island = 1;
            else island = 4;        
        }

	    forlist(&r->products) {
            Production *p = (Production *) elem;
		    switch(island) {
		         case 1:
		             westproducts[p->itemtype] += p->baseamount;
		             break;
		         case 2:
		             northproducts[p->itemtype] += p->baseamount;
		             break;
		         case 3:
		             eastproducts[p->itemtype] += p->baseamount;
		             break;
		         case 4:
		             southproducts[p->itemtype] += p->baseamount;
		             break;
		         case 5:
		             otherproducts[p->itemtype] += p->baseamount;
		             break;
                 default:
                     break;
		    }
	    }
    }
    
    cout << "    \t" << "   W   " << "  N   " << "  E   " << "  S   " << "  O   " << endl;
    for(int i=0; i<NITEMS; i++) {
        if(westproducts[i] == 0 && northproducts[i] == 0 && eastproducts[i] == 0 && southproducts[i] == 0 && otherproducts[i] == 0) continue;
        cout << ItemDefs[i].abr << "\t";
        EditGameWriteoutLine(westproducts[i],northproducts[i],eastproducts[i],southproducts[i],otherproducts[i]);
    }
}

void Game::EditGameTradeSummary()
{
    int northbuyers[NITEMS];
    int northsellers[NITEMS];
    int westbuyers[NITEMS];
    int westsellers[NITEMS];
    int eastbuyers[NITEMS];
    int eastsellers[NITEMS];
    int southbuyers[NITEMS];
    int southsellers[NITEMS];
    int otherbuyers[NITEMS];
    int othersellers[NITEMS];

    for(int i=0; i<NITEMS; i++) {
        westbuyers[i] = 0;
        northbuyers[i] = 0;
        eastbuyers[i] = 0;
        southbuyers[i] = 0;
        otherbuyers[i] = 0;
        westsellers[i] = 0;
        northsellers[i] = 0;
        eastsellers[i] = 0;
        southsellers[i] = 0;
        othersellers[i] = 0;
    }

    forlist(&regions) {
        ARegion *r = (ARegion *) elem;
//        if(!r->town) continue;
        int island = 0;
        if(r->zloc != 1) island = 5;
        else if(r->xloc < 10 || r->xloc > 45) island = 5;
        else if(r->xloc > r->yloc) {
            if(r->xloc + r->yloc < (regions.GetRegionArray(1)->x + regions.GetRegionArray(1)->y)/2 ) island = 2;
            else island = 3;
        } else {
            if(r->xloc + r->yloc < (regions.GetRegionArray(1)->x + regions.GetRegionArray(1)->y)/2 ) island = 1;
            else island = 4;        
        }

	    forlist(&r->markets) {
		    Market *m = (Market *) elem;
		    switch(island) {
		         case 1:
		             if(m->type == M_BUY) westbuyers[m->item]++;
		             else westsellers[m->item]++;
		             break;
		         case 2:
		             if(m->type == M_BUY) northbuyers[m->item]++;
		             else northsellers[m->item]++;
		             break;
		         case 3:
		             if(m->type == M_BUY) eastbuyers[m->item]++;
		             else eastsellers[m->item]++;
		             break;
		         case 4:
		             if(m->type == M_BUY) southbuyers[m->item]++;
		             else southsellers[m->item]++;
		             break;
		         case 5:
		             if(m->type == M_BUY) otherbuyers[m->item]++;
		             else othersellers[m->item]++;
		             break;
                 default:
                     break;
		    }
	    }
    }
    //we have a big trade table. What to do with it?
    int exit = 0;
	do {
		Awrite("");
		Awrite("  [trade] to display trade items.");
		Awrite("  [normal] to display normal items.");
		Awrite("  [men] to display men.");
		Awrite("  [q] Go back one screen.");
		
		AString *pStr = AGetString();
		if(*pStr == "q") exit = 1;
        else {
			AString *pToken = 0;
			
			pToken = pStr->gettoken();
			if(!pToken) {
				Awrite("Try again.");
				break;
			}
            if (*pToken == "trade") { 
                cout << "    \t" << "     Suppliers     \t" << "     Demands" << endl;
                cout << "    \t" << "  W  " << " N  " << " E  " << " S  " << " O  " << "\t" << "  W  " << " N  " << " E  " << " S  " << " O  " << endl;
                for(int i=0; i<NITEMS; i++) {
                    if(!(ItemDefs[i].type & IT_TRADE)) continue;
                    if(ItemDefs[i].flags & ItemType::DISABLED) continue;
                    cout << ItemDefs[i].abr << "\t";
                    EditGameWriteoutLine(westbuyers[i],northbuyers[i],eastbuyers[i],southbuyers[i],otherbuyers[i],westsellers[i],northsellers[i],eastsellers[i],southsellers[i],othersellers[i]);
                }
            }
            else if (*pToken == "normal") { 
                cout << "    \t" << "     Suppliers     \t" << "     Demands" << endl;
                cout << "    \t" << " W  " << " N  " << " E  " << " S  " << " O  " << "\t" << " W  " << " N  " << " E  " << " S  " << " O  " << endl;
                for(int i=0; i<NITEMS; i++) {
                    if(!(ItemDefs[i].type & IT_NORMAL)) continue;
                    if(ItemDefs[i].flags & ItemType::DISABLED) continue;
                    cout << ItemDefs[i].abr << "\t";
                    EditGameWriteoutLine(westbuyers[i],northbuyers[i],eastbuyers[i],southbuyers[i],otherbuyers[i],westsellers[i],northsellers[i],eastsellers[i],southsellers[i],othersellers[i]);
                }
            }
            else if (*pToken == "men") { 
                cout << "    \t" << "     Suppliers     \t" << "     Demands" << endl;
                cout << "    \t" << " W  " << " N  " << " E  " << " S  " << " O  " << "\t" << " W  " << " N  " << " E  " << " S  " << " O  " << endl;
                for(int i=0; i<NITEMS; i++) {
                    if(!(ItemDefs[i].type & IT_MAN)) continue;
                    if(ItemDefs[i].flags & ItemType::DISABLED) continue;
                    cout << ItemDefs[i].abr << "\t";
                    EditGameWriteoutLine(westbuyers[i],northbuyers[i],eastbuyers[i],southbuyers[i],otherbuyers[i],westsellers[i],northsellers[i],eastsellers[i],southsellers[i],othersellers[i]);
                }
            }
            else if (*pToken == "advanced") {
                cout << "    \t" << "     Suppliers     \t" << "     Demands" << endl;
                cout << "    \t" << " W  " << " N  " << " E  " << " S  " << " O  " << "\t" << " W  " << " N  " << " E  " << " S  " << " O  " << endl;
                for(int i=0; i<NITEMS; i++) {
                    if(!(ItemDefs[i].type & IT_ADVANCED)) continue;
                    if(ItemDefs[i].flags & ItemType::DISABLED) continue;
                    cout << ItemDefs[i].abr << "\t";
                    EditGameWriteoutLine(westbuyers[i],northbuyers[i],eastbuyers[i],southbuyers[i],otherbuyers[i],westsellers[i],northsellers[i],eastsellers[i],southsellers[i],othersellers[i]);
                }
            }

			delete pToken;
		}
		if(pStr) delete pStr;

	} while(exit==0);

}

void Game::EditGameWriteoutLine(int one, int two, int three, int four, int five)
{
    if(one<10) cout << "    ";
    else if(one<100) cout << "   ";
    else if(one<1000) cout << "  ";
    else if(one<10000) cout << " ";
    cout << one << " ";
    if(two<10) cout << "    ";
    else if(two<100) cout << "   ";
    else if(two<1000) cout << "  ";
    else if(two<10000) cout << " ";
    cout << two << " ";    
    if(three<10) cout << "    ";
    else if(three<100) cout << "   ";
    else if(three<1000) cout << "  ";
    else if(three<10000) cout << " ";
    cout << three << " ";    
    if(four<10) cout << "    ";
    else if(four<100) cout << "   ";
    else if(four<1000) cout << "  ";
    else if(four<10000) cout << " ";
    cout << four << " ";    
    if(five<10) cout << "    ";
    else if(five<100) cout << "   ";
    else if(five<1000) cout << "  ";
    else if(five<10000) cout << " ";
    cout << five << endl;
}
void Game::EditGameWriteoutLine(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine, int ten)
{
    if(one<10) cout << "  ";
    else if(one<100) cout << " ";
    cout << one << " ";
    if(two<10) cout << "  ";
    else if(two<100) cout << " ";
    cout << two << " ";    
    if(three<10) cout << "  ";
    else if(three<100) cout << " ";
    cout << three << " ";    
    if(four<10) cout << "  ";
    else if(four<100) cout << " ";
    cout << four << " ";    
    if(five<10) cout << "  ";
    else if(five<100) cout << " ";
    cout << five << "\t";    
    if(six<10) cout << "  ";
    else if(six<100) cout << " ";
    cout << six << " ";    
    if(seven<10) cout << "  ";
    else if(seven<100) cout << " ";
    cout << seven << " ";    
    if(eight<10) cout << "  ";
    else if(eight<100) cout << " ";
    cout << eight << " ";    
    if(nine<10) cout << "  ";
    else if(nine<100) cout << " ";
    cout << nine << " ";    
    if(ten<10) cout << "  ";
    else if(ten<100) cout << " ";
    cout << ten << endl;
}


void ARegion::EditAdjustAreaName(AString text)
{
    if(text == *name) return; //prevents reprocessing a region.
 	SetName(text.Str());

   	for(int j=0; j<6; j++) {
   	    if(!neighbors[j]) continue;
   	    if(TerrainDefs[neighbors[j]->type].similar_type != TerrainDefs[type].similar_type) continue;
   	    neighbors[j]->EditAdjustAreaName(text);
   	}
}
