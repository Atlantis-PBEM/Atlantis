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
#include "rules.h"

void InitItemDefs()
{
    return;
}

int Game::SetupFaction( Faction *pFac )
{
	// Factions can only be added at the start of the game
	if(TurnNumber() != 1)
	{
		return (0);
	}

	// Lets see if there is an available regions for this unit
	ARegion *pReg = 0;
	forlist(&regions) {
		pReg = (ARegion *)elem;
		ARegionArray *pRA = regions.pRegionArrays[pReg->zloc];

		if(!pReg->CanBeStartingCity(pRA)) {
			pReg = 0;
			continue;
		}

		if(pReg->IsStartingCity()) {
			// This region has already been set up.
			pReg = 0;
			continue;
		}
		break;
	}

	if(!pReg) {
		// We couldn't find a region to make the faction in.
		return 0;
	}

    pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 50;

	// Make a citadel for this faction
	Object *obj = new Object(pReg);
	obj->num = pReg->buildingseq++;
	obj->name = new AString(AString("Citadel [")+obj->num+"]");
	obj->type = O_CITADEL;
	obj->incomplete = 0;
	obj->inner = -1;
	pReg->objects.Add(obj);

    //
    // Set up first unit.
    //
    Unit *temp2 = GetNewUnit( pFac );
    temp2->SetMen( I_MAN, 1 );
    temp2->reveal = REVEAL_FACTION;
	temp2->MoveUnit(obj);

    return( 1 );
}

void Game::PostProcessUnitExtra(ARegion *r,Unit *u)
{
    if (u->type != U_WMON) {
        int escape = 0;
        int totlosses = 0;
        forlist (&u->items) {
            Item *i = (Item *) elem;
            if (i->type == I_IMP) {
                int top = i->num * i->num;
                if (top) {
                    int level = u->GetSkill(S_SUMMON_IMPS);
                    if (!level) {
                        escape = 10000;
                    } else {
                        int bottom = level * level * 4;
                        bottom = bottom * bottom * 20;
                        int chance = (top * 10000) / bottom;
                        if (chance > escape) escape = chance;
                    }
                }
            }
            
            if (i->type == I_DEMON) {
                int top = i->num * i->num;
                if (top) {
                    int level = u->GetSkill(S_SUMMON_DEMON);
                    if (!level) {
                        escape = 10000;
                    } else {
                        int bottom = level * level;
                        bottom = bottom * bottom * 20;
                        int chance = (top * 10000) / bottom;
                        if (chance > escape) escape = chance;
                    }
                }
            }
            
            if (i->type == I_BALROG) {
                int top = i->num * i->num;
                if (top) {
                    int level = u->GetSkill(S_SUMMON_BALROG);
                    if (!level) {
                        escape = 10000;
                    } else {
                        int bottom = level * level;
                        bottom = bottom * bottom * 4;
                        int chance = (top * 10000) / bottom;
                        if (chance > escape) escape = chance;
                    }
                }
            }
            
            if (i->type == I_SKELETON || i->type == I_UNDEAD ||
                i->type == I_LICH) {
                int losses = (i->num + getrandom(10)) / 10;
                u->items.SetNum(i->type,i->num - losses);
                totlosses += losses;
            }
        }
        
        if (totlosses)
        {
            u->Event(AString(totlosses) + " undead decay into nothingness.");
        }

        if (escape > getrandom(10000)) {
            Faction * mfac = GetFaction(&factions,monfaction);
            
            if (u->items.GetNum(I_IMP)) {
                Unit *mon = GetNewUnit( mfac, 0 );
                mon->MakeWMon(MonDefs[MONSTER_IMP].name,I_IMP,
                              u->items.GetNum(I_IMP));
                mon->MoveUnit( r->GetDummy() );
                u->items.SetNum(I_IMP,0);
            }
            
            if (u->items.GetNum(I_DEMON)) {
                Unit *mon = GetNewUnit( mfac, 0 );
                mon->MakeWMon(MonDefs[MONSTER_DEMON].name,I_DEMON,
                              u->items.GetNum(I_DEMON));
                mon->MoveUnit( r->GetDummy() );
                u->items.SetNum(I_DEMON,0);
            }
            
            if (u->items.GetNum(I_BALROG)) {
                Unit *mon = GetNewUnit( mfac, 0 );
                mon->MakeWMon(MonDefs[MONSTER_BALROG].name,I_BALROG,
                              u->items.GetNum(I_BALROG));
                mon->MoveUnit( r->GetDummy() );
                u->items.SetNum(I_BALROG,0);
            }
            
            u->Event("Controlled demons break free!");
        }
    }
}

void Game::CheckUnitMaintenance( int consume )
{
    CheckUnitMaintenanceItem(I_GRAIN, 10, consume );
    CheckUnitMaintenanceItem(I_LIVESTOCK, 10, consume );
    CheckUnitMaintenanceItem(I_FISH, 10, consume );
}

void Game::CheckFactionMaintenance( int consume )
{
    CheckFactionMaintenanceItem(I_GRAIN, 10, consume );
    CheckFactionMaintenanceItem(I_LIVESTOCK, 10, consume );
    CheckFactionMaintenanceItem(I_FISH, 10, consume );
}

void Game::CheckAllyMaintenance()
{
    CheckAllyMaintenanceItem(I_GRAIN, 10);
    CheckAllyMaintenanceItem(I_LIVESTOCK, 10);
    CheckAllyMaintenanceItem(I_FISH, 10);
}

Faction *Game::CheckVictory()
{
	Faction *pVictor = 0;

	// First, if there is only one living faction, it is the winner.
	forlist(&factions) {
		Faction *pFac = (Faction *)elem;
		if(pFac->exists) {
			if(pVictor) {
				// This is the second faction we've found.  No winner.
				pVictor = 0;
				break;
			}
			pVictor = pFac;
		}
	}

	if(pVictor)
		return pVictor;

	// Next, if one faction holds all citadels, it is the winner
	{
		forlist(&regions) {
			ARegion *pReg = (ARegion *)elem;
			if(pReg->IsStartingCity()) {
				forlist(&(pReg->objects)) {
					Object *pObj = (Object *)elem;
					if(pObj->type != O_CITADEL) {
						continue;
					}
					Unit *u = pObj->GetOwner();
					if(!u) {
						// Noone controls
						return 0;
					}
					if(!pVictor) {
						pVictor = u->faction;
					} else {
						if(pVictor != u->faction) {
							return 0;
						}
					}
					// We've found one citadel.. no need to keep going on
					// this object
					break;
				}
			}
		}
	}
	return pVictor;
}

int Game::AllowedMages( Faction *pFac )
{
    return 5;
}

int Game::AllowedApprentices( Faction *pFac )
{
    return 10;
}

int Game::AllowedTaxes( Faction *pFac )
{
    return -1;
}

int Game::AllowedTrades( Faction *pFac )
{
    return -1;
}

int Unit::GetSkillBonus( int sk )
{
    int bonus = 0;
    switch( sk )
    {
    case S_OBSERVATION:
        if (GetMen())
        {
            bonus = (GetSkill(S_TRUE_SEEING) + 1) / 2;
        }
        if ((bonus != 3) && GetMen() && items.GetNum(I_AMULETOFTS))
        {
            bonus = 2;
        }
        break;
    case S_STEALTH:
        if (GetFlag(FLAG_INVIS) || GetMen() <= items.GetNum(I_RINGOFI))
        {
            bonus = 3;
        }
        break;
    default:
        bonus = 0;
        break;
    }
    return bonus;
}

int Unit::GetProductionBonus( int item )
{
    int bonus = 0;
    if (ItemDefs[item].mult_item != -1)
    {
      bonus = items.GetNum(ItemDefs[item].mult_item);
    } else {
      bonus = GetMen();
    }
    if (bonus > GetMen()) bonus = GetMen();
    return bonus * ItemDefs[item].mult_val;
}

void Game::CreateOceanLairs()
{
    // here's where we add the creation.
    forlist (&regions)
    {
        ARegion * r = (ARegion *) elem;
        if (r->type == R_OCEAN)
        {
            r->LairCheck();
        }
    }
}

int Game::UpgradeMajorVersion(int savedVersion)
{
    return 1;
}

int Game::UpgradeMinorVersion(int savedVersion)
{
    return 1;
}

int Game::UpgradePatchLevel(int savedVersion)
{
    if ( ATL_VER_PATCH(savedVersion) < 5 )
    {
        InitItemDefs();
    }
    return 1;
}

void Game::CreateVMons()
{
	if(Globals->OPEN_ENDED) return;

	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * obj = (Object *) elem;
			if(obj->type != O_BKEEP) continue;
			Faction *monfac = GetFaction( &factions, 2 );
			Unit *u = GetNewUnit( monfac, 0 );
			u->MakeWMon( "Elder Demons", I_BALROG, 200);
			u->MoveUnit(obj);
		}
	}
}

void Game::GrowVMons()
{
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *obj = (Object *)elem;
			if(obj->type != O_BKEEP) continue;
			forlist(&obj->units) {
				Unit *u = (Unit *)elem;
				int men = u->GetMen(I_BALROG) + 2;
				if(men > 200) men = 200;
				u->items.SetNum(I_BALROG, men);
			}
		}
	}
}
