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
// MODIFICATIONS
// Date        Person            Comments
// ----        ------            --------
// 2000/MAR/14 Davis Kulis       Added a new reporting Template.
// 2000/MAR/21 Azthar Septragen  Added roads.
// 2000/SEP/06 Joseph Traub      Added base man cost to allow races to have
//                               different base costs
// 2001/Feb/16 Joseph Traub      Semi-fixed a bug which allowed multiple
//                               disconnected regions in the underworld.
//

#include "game.h"
#include "gamedata.h"

Location * GetUnit(AList * list,int n)
{
    forlist(list) {
        Location * l = (Location *) elem;
        if (l->unit->num == n) return l;
    }
    return 0;
}

ARegionPtr * GetRegion(AList * l,int n)
{
    forlist(l) {
        ARegionPtr * p = (ARegionPtr *) elem;
        if (p->ptr->num == n) return p;
    }
    return 0;
}

Farsight *GetFarsight(AList *l,Faction *fac)
{
    forlist(l) {
        Farsight *f = (Farsight *) elem;
        if (f->faction == fac) return f;
    }
    return 0;
}

AString TownString(int i)
{
    switch (i) {
    case TOWN_VILLAGE:
        return "village";
    case TOWN_TOWN:
        return "town";
    case TOWN_CITY:
        return "city";
    }
    return "huh?";
}

TownInfo::TownInfo()
{
    name = 0;
    pop = 0;
    basepop = 0;
    activity = 0;
}

TownInfo::~TownInfo()
{
    if (name) delete name;
}

void TownInfo::Readin( Ainfile *f, ATL_VER ver)
{
    name = f->GetStr();
    pop = f->GetInt();
    basepop = f->GetInt();
}

void TownInfo::Writeout( Aoutfile *f )
{
    f->PutStr(*name);
    f->PutInt(pop);
    f->PutInt(basepop);
}

int TownInfo::TownType() {
  if (pop < 1000) return TOWN_VILLAGE;
  if (pop < 2000) return TOWN_TOWN;
  return TOWN_CITY;
}

ARegion::ARegion() {
  name = new AString("Region");
  xloc = 0;
  yloc = 0;
  buildingseq = 1;
  gate = 0;
  town = 0;
  clearskies = 0;
  earthlore = 0;
  for (int i=0; i<NDIRS; i++)
    neighbors[i] = 0;
}

ARegion::~ARegion() {
  if (name) delete name;
  if (town) delete town;
}

void ARegion::ZeroNeighbors() {
  for (int i=0; i<NDIRS; i++) {
    neighbors[i] = 0;
  }
}

void ARegion::SetName(char * c) {
  if(name) delete name;
  name = new AString(c);
}

int ARegion::Population() {
  if (town) {
    return population + town->pop;
  } else {
    return population;
  }
}

int ARegion::Wages() {
  int retval;
  if (town) {
    /* hack, assumes that TownType + 1 = town wages */
    retval = wages + town->TownType() + 1;
  } else {
    retval = wages;
  }
  if (earthlore) retval++;
  if (clearskies) retval++;
  // AS
  if (CountConnectingRoads() > 1) retval++;
  return retval;
}

AString ARegion::WagesForReport() {
  Production * p = products.GetProd(I_SILVER,-1);
  if (p) {
    return AString("$") + p->productivity + " (Max: $" + p->amount + ")";
  } else {
    return AString("$") + 0;
  }
}

void ARegion::SetupPop()
{
    TerrainType * typer = &(TerrainDefs[type]);

    int pop = typer->pop;
    int mw = typer->wages;

    if (pop == 0) {
        population = 0;
        basepopulation = 0;
        wages = 0;
        maxwages = 0;
        money = 0;
        return;
    }

    race = -1;
    while (race == -1) {
        switch (getrandom(IsCoastal() ? 5 : 3)) {
        case 0:
            race = typer->race1;
            break;
        case 1:
            race = typer->race2;
            break;
        case 2:
            race = typer->race3;
            break;
        case 3:
            race = typer->coastalrace1;
            break;
        case 4:
            race = typer->coastalrace2;
            break;
        }
    }

    if( Globals->RANDOM_ECONOMY )
    {
        population = (pop + getrandom(pop)) / 2;
    }
    else
    {
        population = pop;
    }

    basepopulation = population;

	if( Globals->RANDOM_ECONOMY )
    {
        mw += getrandom(3);
    }

    /* Setup wages */
    wages = mw;
    maxwages = mw;

    if( Globals->TOWNS_EXIST )
    {
        if (getrandom(200) < TerrainDefs[type].economy)
        {
            AddTown();
        }
    }

    Production * p = new Production;
    p->itemtype = I_SILVER;
    money = Population() * (Wages() - Globals->MAINTENANCE_COST);
    p->amount = money / Globals->WORK_FRACTION;
    p->skill = -1;
    p->level = 1;
    p->productivity = Wages();
    products.Add(p);

    //	
    // Setup entertainment
    //
    p = new Production;
    p->itemtype = I_SILVER;
    p->amount = money / Globals->ENTERTAIN_FRACTION;
    p->skill = S_ENTERTAINMENT;
    p->level = 1;
    p->productivity = Globals->ENTERTAIN_INCOME;
    products.Add(p);

	float ratio = ItemDefs[race].baseprice / (float)Globals->BASE_MAN_COST;
    /* Setup Recruiting */
    Market *m = new Market( M_BUY,
                            race,
                            Wages()*4*ratio,
                            Population()/5,
                            0,
                            10000,
                            0,
                            2000 );
    markets.Add(m);

    if( Globals->LEADERS_EXIST )
    {
		ratio = ItemDefs[I_LEADERS].baseprice / (float)Globals->BASE_MAN_COST;
        Market *m = new Market( M_BUY,
                                I_LEADERS,
                                Wages()*4*ratio,
                                Population()/25,
                                0,
                                10000,
                                0,
                                400 );
        markets.Add(m);
    }
}

int ARegion::GetNearestProd(int item)
{
    AList regs,regs2;
    AList * rptr = &regs;
    AList * r2ptr = &regs2;
    AList * temp;
    ARegionPtr * p = new ARegionPtr;
    p->ptr = this;
    regs.Add(p);
  
    for (int i=0; i<5; i++) {
        forlist(rptr) {
            ARegion * r = ((ARegionPtr *) elem)->ptr;
            if (r->products.GetProd(item,ItemDefs[item].skill)) {
                regs.DeleteAll();
                regs2.DeleteAll();
                return i;
            }
            for (int j=0; j<NDIRS; j++) {
                if (neighbors[j]) {
                    p = new ARegionPtr;
                    p->ptr = neighbors[j];
                    r2ptr->Add(p);
                }
            }
            rptr->DeleteAll();
            temp = rptr;
            rptr = r2ptr;
            r2ptr = temp;
        }
    }
    regs.DeleteAll();
    regs2.DeleteAll();
    return 5;
}

void ARegion::SetupCityMarket()
{
    int numtrade = 0;
    int i;
    for (i=0; i<NITEMS; i++)
    {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;

        int j;
        if( ItemDefs[ i ].type & IT_NORMAL )
        {

            if (i==I_SILVER) continue;
            if (i==I_GRAIN || i==I_LIVESTOCK || i==I_FISH)
            {
                if (i==I_FISH && !IsCoastal()) continue;

                int amt = Globals->CITY_MARKET_NORMAL_AMT;
                int price;

                if( Globals->RANDOM_ECONOMY )
                {
                    amt += getrandom( amt );
                    price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
                        100;
                }
                else
                {
                    price = ItemDefs[ i ].baseprice;
                }

                Market * m = new Market
                    (M_SELL,
                     i,
                     price,
                     amt,
                     population,
                     population+2000,
                     amt,
                     amt*2);
                markets.Add(m);
            } 
            else
            {
                if (ItemDefs[i].input == -1)
                {
                    if (TerrainDefs[type].prod1 == i ||
                        TerrainDefs[type].prod2 == i ||
                        TerrainDefs[type].prod3 == i ||
                        TerrainDefs[type].prod4 == i ||
                        TerrainDefs[type].prod5 == i)
                    {
                        //
                        // This item can be produced in this region, so it
                        // can possibly be bought here.
                        //
                        if (getrandom(2))
                        {
                            int amt = Globals->CITY_MARKET_NORMAL_AMT;
                            int price;

                            if( Globals->RANDOM_ECONOMY )
                            {
                                amt += getrandom( amt );
                                price = (ItemDefs[i].baseprice *
                                   (150 + getrandom(50))) / 100;
                            }
                            else
                            {
                                price = ItemDefs[ i ].baseprice;
                            }

                            Market * m = new Market
                                ( M_BUY,
                                  i,
                                  price,
                                  0,
                                  1000+population,
                                  4000+population,
                                  0,
                                  50 + getrandom(50));
                            markets.Add(m);
                        }
                    }
                    else
                    {
                        //
                        // This item cannot be produced in this region;
                        // perhaps it is in demand here?
                        //
                        if( !getrandom( 6 ))
                        {
                            int amt = Globals->CITY_MARKET_NORMAL_AMT;
                            int price;

                            if( Globals->RANDOM_ECONOMY )
                            {
                                amt += getrandom( amt );
                                price = (ItemDefs[i].baseprice *
                                   (100 + getrandom(50))) / 100;
                            }
                            else
                            {
                                price = ItemDefs[ i ].baseprice;
                            }

                            Market * m = new Market
                                ( M_SELL,
                                  i,
                                  price,
                                  0,
                                  1000+population,
                                  4000+population,
                                  0,
                                  amt );
                            markets.Add(m);
                        }
                    }
                } 
                else
                {
                    if (!getrandom(3))
                    {
                        int amt = Globals->CITY_MARKET_NORMAL_AMT;
                        int price;
                        if( Globals->RANDOM_ECONOMY )
                        {
                            amt += getrandom( amt );
                            price = (ItemDefs[i].baseprice *
                              (100 + getrandom(50))) / 100;
                        }
                        else
                        {
                            price = ItemDefs[ i ].baseprice;
                        }

                        Market * m = new Market
                            (M_SELL,
                             i,
                             price,
                             0,
                             1000+population,
                             4000+population,
                             0,
                             amt );
                        markets.Add(m);
                    } 
                    else
                    {
                        if (!getrandom(6))
                        {
                            int amt = Globals->CITY_MARKET_NORMAL_AMT;
                            int price;

                            if( Globals->RANDOM_ECONOMY )
                            {
                                amt += getrandom( amt );
                                price = (ItemDefs[i].baseprice *
                                  (150 + getrandom(50))) / 100;
                            }
                            else
                            {
                                price = ItemDefs[ i ].baseprice;
                            }

                            Market * m = new Market
                                (M_BUY,
                                 i,
                                 price,
                                 0,
                                 1000+population,
                                 4000+population,
                                 0,
                                 amt );
                            markets.Add(m);
                        }
                    }
                }
            }
        }
        else if( ItemDefs[ i ].type & IT_ADVANCED )
        {
            j = getrandom(4);
            if (j==2)
            {
                int amt = Globals->CITY_MARKET_ADVANCED_AMT;
                int price;

                if( Globals->RANDOM_ECONOMY )
                {
                    amt += getrandom( amt );
                    price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
                        100;
                }
                else
                {
                    price = ItemDefs[ i ].baseprice;
                }

                Market * m = new Market
                    (M_SELL,
                     i,
                     price,
                     0,
                     2000+population,
                     4000+population,
                     0,
                     amt );
                markets.Add(m);
            }
        }
        else if( ItemDefs[ i ].type & IT_TRADE )
        {
            numtrade++;
        }
    }
    
    /* Set up the trade items */
    int buy1 = getrandom(numtrade);
    int buy2 = getrandom(numtrade);
    int sell1 = getrandom(numtrade);
    int sell2 = getrandom(numtrade);
    
    buy1 = getrandom(numtrade);
    while (buy1 == buy2) buy2 = getrandom(numtrade);
    while (sell1 == buy1 || sell1 == buy2) sell1 = getrandom(numtrade);
    while (sell2 == sell1 || sell2 == buy2 || sell2 == buy1)
        sell2 = getrandom(numtrade);
    
    for (i=0; i<NITEMS; i++)
    {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;

        if( ItemDefs[ i ].type & IT_TRADE )
        {
            int addbuy = 0;
            int addsell = 0;

            if (buy1 == 0 || buy2 == 0)
            {
                addbuy = 1;
            }
            buy1--;
            buy2--;

            if( sell1 == 0 || sell2 == 0 )
            {
                addsell = 1;
            }
            sell1--;
            sell2--;

            if( addbuy )
            {
                int amt = Globals->CITY_MARKET_TRADE_AMT;
                int price;

                if( Globals->RANDOM_ECONOMY )
                {
                    amt += getrandom( amt );
                    price = (ItemDefs[i].baseprice * (150 + getrandom(50))) /
                        100;
                }
                else
                {
                    price = ItemDefs[ i ].baseprice;
                }

                Market * m = new Market
                    (M_SELL,
                     i,
                     price,
                     0,
                     2000+population,
                     4000+population,
                     0,
                     amt );
                markets.Add(m);
            }
            
            if( addsell )
            {
                int amt = Globals->CITY_MARKET_TRADE_AMT;
                int price;

                if( Globals->RANDOM_ECONOMY )
                {
                    amt += getrandom( amt );
                    price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
                        100;
                }
                else
                {
                    price = ItemDefs[ i ].baseprice;
                }

                Market * m = new Market
                    (M_BUY,
                     i,
                     price,
                     0,
                     2000+population,
                     4000+population,
                     0,
                     amt );
                markets.Add(m);
            }
        }
    }
}

void ARegion::SetupProds()
{
    Production * p;
    TerrainType * typer = &(TerrainDefs[type]);

    if( Globals->FOOD_ITEMS_EXIST )
    {
        if (typer->economy) {
            if (getrandom(2)) {
                p = new Production(I_GRAIN,typer->economy);
                products.Add(p);
            } else {
                p = new Production(I_LIVESTOCK,typer->economy);
                products.Add(p);
            }
        }
    }

    if (typer->prod1 != -1)
    {
        if (getrandom(100) < typer->chance1)
        {
            p = new Production(typer->prod1,typer->amt1);
            products.Add(p);
        }
    }

    if (typer->prod2 != -1)
    {
        if (getrandom(100) < typer->chance2)
        {
            p = new Production(typer->prod2,typer->amt2);
            products.Add(p);
        }
    }

    if (typer->prod3 != -1)
    {
        if (getrandom(100) < typer->chance3)
        {
            p = new Production(typer->prod3,typer->amt3);
            products.Add(p);
        }
    }

    if (typer->prod4 != -1)
    {
        if (getrandom(100) < typer->chance4)
        {
            p = new Production(typer->prod4,typer->amt4);
            products.Add(p);
        }
    }

    if (typer->prod5 != -1)
    {
        if (getrandom(100) < typer->chance5)
        {
            p = new Production(typer->prod5,typer->amt5);
            products.Add(p);
        }
    }
}

void ARegion::AddTown()
{
    town = new TownInfo;
    
    town->name = new AString( AGetNameString( AGetName( this, 1 )));

    if( Globals->RANDOM_ECONOMY )
    {
        town->pop = 500+getrandom(2500);
    }
    else
    {
        town->pop = 500;
    }

    town->basepop = town->pop;
    town->activity = 0;

    SetupCityMarket();
}

void ARegion::LairCheck()
{
    /* No lair if town in region */
    if (town) return;
    
    int check = getrandom(100);

    TerrainType *tt = &TerrainDefs[ type ];

    if( tt->lair1 == -1 )
    {
        return;
    }

	if(!(ObjectDefs[tt->lair1].flags & ObjectType::DISABLED)) {
		if( check < tt->lairChance ) {
			MakeLair( tt->lair1 );
			return;
		}
		check -= tt->lairChance;
	}

    if( tt->lair2 == -1 )
    {
        return;
    }

	if(!(ObjectDefs[tt->lair2].flags & ObjectType::DISABLED)) {
		if( check < tt->lairChance ) {
			MakeLair( tt->lair2 );
			return;
		}
		check -= tt->lairChance;
	}

    if( tt->lair3 == -1 )
    {
        return;
    }

	if(!(ObjectDefs[tt->lair3].flags & ObjectType::DISABLED)) {
		if( check < tt->lairChance ) {
			MakeLair( tt->lair3 );
			return;
		}
		check -= tt->lairChance;
	}

    if( tt->lair4 == -1 )
    {
        return;
    }

	if(!(ObjectDefs[tt->lair4].flags & ObjectType::DISABLED)) {
		if( check < tt->lairChance ) {
			MakeLair( tt->lair4 );
			return;
		}
	}
}

void ARegion::MakeLair(int t)
{
    Object * o = new Object( this );
    o->num = buildingseq++;
    o->name = new AString(AString(ObjectDefs[t].name) + " [" + o->num + "]");
    o->type = t;
    o->incomplete = 0;
    o->inner = -1;
    objects.Add(o);
}

void ARegion::Setup()
{
    //
    // type and location have been setup, do everything else
    SetupProds();

    SetupPop();

    //
    // Make the dummy object
    //
    Object * obj = new Object( this );
    objects.Add(obj);

	if(Globals->LAIR_MONSTERS_EXIST)
		LairCheck();
}

void ARegion::UpdateTown()
{
    if( !( Globals->VARIABLE_ECONOMY ))
    {
        //
        // Nothing to do here.
        //
        return;
    }

    //
    // Don't do pop stuff for AC Exit.
    //
    if (town->pop != 5000)
    {
        /* First, get the target population */
        int amt = 0;
        int tot = 0;
        forlist(&markets) {
            Market * m = (Market *) elem;
            if (town->pop > m->minpop)
            {
                if (ItemDefs[m->item].type & IT_TRADE)
                {
                    if (m->type == M_BUY)
                    {
                        amt += 5 * m->activity;
                        tot += 5 * m->maxamt;
                    }
                }
                else
                {
                    if (m->type == M_SELL)
                    {
                        amt += m->activity;
                        tot += m->maxamt;
                    }
                }
            }
        }
    
        int tarpop;
        if (tot)
        {
            tarpop = (Globals->CITY_POP * amt) / tot;
        } 
        else
        {
            tarpop = 0;
        }

        /* Let's bump tarpop up */
        tarpop = (tarpop * 3) / 2;
        if (tarpop > Globals->CITY_POP) tarpop = Globals->CITY_POP;
        
        town->pop = town->pop + (tarpop - town->pop) / 5;
        
        /* Check base population */
        if (town->pop < town->basepop)
        {
            town->pop = town->basepop;
        }
        if ((town->pop * 2) / 3 > town->basepop)
        {
            town->basepop = (town->pop * 2) / 3;
        }
    }
}

void ARegion::PostTurn()
{
    //
    // First update population based on production
    //
    int activity = 0;
    int amount = 0;
    if (basepopulation)
    {
        forlist(&products) {
            Production * p = (Production *) elem;
            if (ItemDefs[p->itemtype].type & IT_NORMAL &&
                p->itemtype != I_SILVER)
            {
                activity += p->activity;
                amount += p->amount;
            }
        }
        int tarpop = basepopulation + (basepopulation * activity) /
            (2 * amount);
        int diff = tarpop - population;

        if( Globals->VARIABLE_ECONOMY )
        {
            population = population + diff / 5;
        }
        
        //
        // Now, if there is a town, update its population.
        //
        if (town)
        {
            UpdateTown();
        }

        // AS
        if (HasRoad())
        {
            DoDecayCheck();
        }
        
        //
        // Now, reset population based stuff.
        // Recover Wages.
        //
        if (wages < maxwages) wages++;

        //
        // Set money
        //
        money = (Wages() - Globals->MAINTENANCE_COST) * Population();
        if (money < 0) money = 0;
        
        //
        // Setup working
        //
        Production * p = products.GetProd(I_SILVER,-1);
        if (IsStartingCity())
        {
            //
            // Higher wages in the entry cities.
            //
            p->amount = Wages() * Population();
        } 
        else
        {
            p->amount = (Wages() * Population()) / Globals->WORK_FRACTION;
        }
        p->productivity = Wages();
        
        //
        // Entertainment.
        //
        p = products.GetProd(I_SILVER,S_ENTERTAINMENT);
        p->baseamount = money / Globals->ENTERTAIN_FRACTION;
        
        markets.PostTurn(Population(),Wages());
    }
    
    UpdateProducts();
    
    //
    // Set these guys to 0.
    //
    earthlore = 0;
    clearskies = 0;
    
    forlist(&objects) {
        Object *o = (Object *) elem;
        forlist(&o->units) {
            Unit *u = (Unit *) elem;
            u->PostTurn(this);
        }
    }
}

// AS
void ARegion::DoDecayCheck()
{
    forlist (&objects)
    {
        Object *o = (Object *) elem;
        if (o->IsRoadUsable())
        {
            DoDecayClicks(o);
        }
    }
}

// AS
// value to add when road has reached decay level
#define DECAY_ADD 15

// AS
void ARegion::DoDecayClicks(Object *o)
{
    int clicks = getrandom(GetMaxClicks());
    clicks += PillageCheck();
    o->incomplete += clicks;
    if (!o->IsRoadUsable())
    {
        o->incomplete += DECAY_ADD;
        // trigger decay event
        RunDecayEvent(o);
    }
}

// AS
void ARegion::RunDecayEvent(Object *o)
{
    Areport * fReport;
    AList * pFactions;
    pFactions = PresentFactions();
    forlist (pFactions)
    {
        Faction * f = ((FactionPtr *) elem)->ptr;
        f->Event(GetDecayFlavor() + *o->name + " " +
            ObjectDefs[o->type].name + " in " +
            (AString) TerrainDefs[type].name + " (" + xloc + "," +
            yloc + ") in " + *name + ".");
    }
}

// AS
AString ARegion::GetDecayFlavor()
{
    AString flavor;
    int badWeather = 0;
    if (weather != W_NORMAL && !clearskies) badWeather = 1;
    if (!Globals->WEATHER_EXISTS) badWeather = 0;
    switch (type)
    {
        case R_PLAIN:
		case R_ISLAND_PLAIN:
            flavor = AString("Floods have damaged ");
            break;
        case R_DESERT:
            flavor = AString("Flashfloods have damaged ");
            break;
        case R_TUNDRA:
            if (badWeather)
            {
                flavor = AString("Ground freezing has damaged ");
            }
            else
            {
                flavor = AString("Ground thaw has damaged ");
            }
            break;
        case R_MOUNTAIN:
		case R_ISLAND_MOUNTAIN:
            if (badWeather)
            {
                flavor = AString("Avalanches have damaged ");
            }
            else
            {
                flavor = AString("Rockslides have damaged ");
            }
            break;
        case R_FOREST:
        case R_SWAMP:
		case R_ISLAND_SWAMP:
        case R_JUNGLE:
            flavor = AString("Encroaching vegetation has damaged ");
            break;
        case R_CAVERN:
        case R_UFOREST:
        case R_TUNNELS:
            if (badWeather)
            {
                flavor = AString("Lava flows have damaged ");
            }
            else
            {
                flavor = AString("Quakes have damaged ");
            }
            break;
        default:
            flavor = AString("Unexplained phenomena have damaged ");
            break;
    }
    return flavor;
}

// AS
int ARegion::GetMaxClicks()
{
    int terrainAdd = 0;
    int terrainMult = 1;
    int weatherAdd = 0;
    int badWeather = 0;
    int maxClicks;
    if (weather != W_NORMAL && !clearskies) badWeather = 1;
    if (!Globals->WEATHER_EXISTS) badWeather = 0;
    switch (type)
    {
        case R_PLAIN:
		case R_ISLAND_PLAIN:
        case R_TUNDRA:
            terrainAdd = -1;
            if (badWeather) weatherAdd = 4;
            break;
        case R_MOUNTAIN:
		case R_ISLAND_MOUNTAIN:
            terrainMult = 2;
            if (badWeather) weatherAdd = 4;
            break;
        case R_FOREST:
        case R_SWAMP:
		case R_ISLAND_SWAMP:
        case R_JUNGLE:
            terrainAdd = -1;
            terrainMult = 2;
            if (badWeather) weatherAdd = 1;
            break;
        case R_DESERT:
            terrainAdd = -1;
            if (badWeather) weatherAdd = 5;
        case R_CAVERN:
        case R_UFOREST:
        case R_TUNNELS:
            terrainAdd = 1;
            terrainMult = 2;
            if (badWeather) weatherAdd = 6;
            break;
        default:
            if (badWeather) weatherAdd = 4;
            break;
    }
    maxClicks = terrainMult * (terrainAdd + 2) + (weatherAdd + 1);
    return maxClicks;
}

// AS
int ARegion::PillageCheck()
{
    int pillageAdd = maxwages - wages;
    if (pillageAdd > 0) return pillageAdd;
    return 0;
}

// AS
int ARegion::HasRoad()
{
    forlist (&objects)
    {
        Object * o = (Object *) elem;
        if (o->IsRoadUsable()) return 1;
    }
    return 0;
}

// AS
int ARegion::HasExitRoad(int realDirection)
{
    forlist (&objects)
    {
        Object * o = (Object *) elem;
        if (o->IsRoadUsable())
        {
            if (o->type == GetRoadDirection(realDirection)) return 1;
        }
    }
    return 0;
}

// AS
int ARegion::CountConnectingRoads()
{
    int connections = 0;
    for (int i = 0; i < NDIRS; i++)
    {
        if (HasExitRoad(i) && neighbors[i]->HasConnectingRoad(i))
            connections ++;
    }
    return connections;
}

// AS
int ARegion::HasConnectingRoad(int realDirection)
{
    if (HasExitRoad(GetRealDirComp(realDirection))) return 1;
    return 0;
}

// AS
int ARegion::GetRoadDirection(int realDirection)
{
    int roadDirection = 0;
    switch (realDirection)
    {
        case D_NORTH:
            roadDirection = O_ROADN;
            break;
        case D_NORTHEAST:
            roadDirection = O_ROADNE;
            break;
        case D_NORTHWEST:
            roadDirection = O_ROADNW;
            break;
        case D_SOUTH:
            roadDirection = O_ROADS;
            break;
        case D_SOUTHEAST:
            roadDirection = O_ROADSE;
            break;
        case D_SOUTHWEST:
            roadDirection = O_ROADSW;
            break;
    }
    return roadDirection;
}

// AS
int ARegion::GetRealDirComp(int realDirection)
{
    int complementDirection = 0;
    switch (realDirection)
    {
        case D_NORTH:
            complementDirection = D_SOUTH;
            break;
        case D_NORTHEAST:
            complementDirection = D_SOUTHWEST;
            break;
        case D_NORTHWEST:
            complementDirection = D_SOUTHEAST;
            break;
        case D_SOUTH:
            complementDirection = D_NORTH;
            break;
        case D_SOUTHEAST:
            complementDirection = D_NORTHWEST;
            break;
        case D_SOUTHWEST:
            complementDirection = D_NORTHEAST;
            break;
    }
    return complementDirection;
}

void ARegion::UpdateProducts()
{
    forlist (&products) {
        Production *prod = (Production *) elem;
        int lastbonus = prod->baseamount / 2;
        int bonus = 0;

        if (prod->itemtype == I_SILVER && prod->skill == -1) continue;

        forlist (&objects) {
            Object *o = (Object *) elem;
            if (o->incomplete == 0 &&
                ObjectDefs[o->type].production == prod->itemtype) {
                lastbonus /= 2;
                bonus += lastbonus;
            }
        }
        prod->amount = prod->baseamount + bonus;

        if (prod->itemtype == I_GRAIN || prod->itemtype == I_LIVESTOCK) {
            prod->amount += ((earthlore + clearskies) * 40) / prod->baseamount;
        }
    }
}

AString ARegion::ShortPrint( ARegionList *pRegs )
{
    AString temp = (AString) TerrainDefs[type].name;

    temp += AString(" (") + xloc + "," + yloc;
    
    ARegionArray *pArr = pRegs->pRegionArrays[ zloc ];
    if( pArr->strName )
    {
        temp += ",";
        temp += *pArr->strName;
    }
    temp += ")";

    temp += AString(" in ") + *name;
    return temp;
}

AString ARegion::Print( ARegionList *pRegs )
{
    AString temp = ShortPrint( pRegs );
    if (town)
    {
        temp += AString(", contains ") + *(town->name) + " [" +
            TownString(town->TownType()) + "]";
    }
    return temp;
}

void ARegion::SetLoc(int x,int y,int z)
{
    xloc = x;
    yloc = y;
    zloc = z;
}

void ARegion::Kill(Unit * u)
{
    Unit * first = 0;
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        if (obj) {
            forlist((&obj->units)) {
                if (((Unit *) elem)->faction->num == u->faction->num &&
                    ((Unit *) elem) != u) {
                    first = (Unit *) elem;
                    break;
                }
            }
        }
        if (first) break;
    }

    if (first) {
        /* give u's stuff to first */
        forlist(&u->items) {
            Item * i = (Item *) elem;
            if (!IsSoldier(i->type)) {
                first->items.SetNum(i->type,first->items.GetNum(i->type) +
                                    i->num);
            }
            u->items.SetNum(i->type,0);
        }
    }
  
    u->MoveUnit( 0 );
    hell.Add(u);
}

void ARegion::ClearHell()
{
    hell.DeleteAll();
}

Object * ARegion::GetObject(int num)
{
    forlist(&objects) {
        Object * o = (Object *) elem;
        if (o->num == num) return o;
    }
    return 0;
}

Object *ARegion::GetDummy()
{
    forlist(&objects) {
        Object * o = (Object *) elem;
        if (o->type == O_DUMMY) return o;
    }
    return 0;
}

Unit * ARegion::GetUnit(int num)
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        Unit *u = obj->GetUnit( num );
        if( u )
        {
            return( u );
        }
    }
    return 0;
}

Location * ARegion::GetLocation(UnitId * id,int faction)
{
    Unit * retval = 0;
    forlist(&objects) {
        Object * o = (Object *) elem;
        retval = o->GetUnitId(id,faction);
        if (retval)
        {
            Location * l = new Location;
            l->region = this;
            l->obj = o;
            l->unit = retval;
            return l;
        }
    }
    return 0;
}

Unit * ARegion::GetUnitAlias(int alias,int faction)
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        Unit *u = obj->GetUnitAlias( alias, faction );
        if( u )
        {
            return( u );
        }
    }
    return 0;
}

Unit * ARegion::GetUnitId(UnitId * id,int faction)
{
    Unit * retval = 0;
    forlist(&objects) {
        Object * o = (Object *) elem;
        retval = o->GetUnitId(id,faction);
        if (retval) return retval;
    }
    return retval;
}

int ARegion::Present(Faction * f)
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        forlist((&obj->units))
            if (((Unit *) elem)->faction == f) return 1;
    }
    return 0;
}

AList *ARegion::PresentFactions()
{
    AList * facs = new AList;
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        forlist((&obj->units)) {
            Unit * u = (Unit *) elem;
            if (!GetFaction2(facs,u->faction->num)) {
                FactionPtr * p = new FactionPtr;
                p->ptr = u->faction;
                facs->Add(p);
            }
        }
    }
    return facs;
}

void ARegion::Writeout( Aoutfile *f )
{
    f->PutStr(*name);
    f->PutInt(num);
    f->PutInt(type);
    f->PutInt(buildingseq);
    f->PutInt(gate);
    f->PutInt(race);
    f->PutInt(population);
    f->PutInt(basepopulation);
    f->PutInt(wages);
    f->PutInt(maxwages);
    f->PutInt(money);
    
    if (town) {
        f->PutInt(1);
        town->Writeout( f );
    } else {
        f->PutInt(0);
    }
    
    f->PutInt(xloc);
    f->PutInt(yloc);
    f->PutInt(zloc);
    
    products.Writeout(f);
    markets.Writeout(f);
    
    f->PutInt(objects.Num());
    forlist ((&objects))
        ((Object *) elem)->Writeout( f );
}

void ARegion::Readin( Ainfile * f,AList * facs, ATL_VER v )
{
    name = f->GetStr();
  
    num = f->GetInt();
    type = f->GetInt();
    buildingseq = f->GetInt();
    gate = f->GetInt();

    race = f->GetInt();
    population = f->GetInt();
    basepopulation = f->GetInt();
    wages = f->GetInt();
    maxwages = f->GetInt();
    money = f->GetInt();
  
    if (f->GetInt()) {
        town = new TownInfo;
        town->Readin(f,v);
    } else {
        town = 0;
    }

    xloc = f->GetInt();
    yloc = f->GetInt();	
    zloc = f->GetInt();
  
    products.Readin(f);
    markets.Readin(f);

    int i = f->GetInt();
    for (int j=0; j<i; j++) {
        Object * temp = new Object( this );
        temp->Readin(f,facs,v);
        objects.Add(temp);
    }
}

int ARegion::CanMakeAdv(Faction * fac,int item)
{
    forlist(&objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->faction == fac)
            {
                if (u->GetSkill(ItemDefs[item].skill) >=
                    ItemDefs[item].level)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void ARegion::WriteProducts(Areport * f,Faction * fac)
{
    AString temp = "Products: ";
    int has = 0;
    forlist((&products)) {
        Production * p = ((Production *) elem);
        if (ItemDefs[p->itemtype].type & IT_ADVANCED) {
            if (CanMakeAdv(fac,p->itemtype)||(fac->IsNPC())) {
                if (has) {
                    temp += AString(", ") + p->WriteReport();
                } else {
                    has = 1;
                    temp += p->WriteReport();
                }
            }
        } else {
            if (p->itemtype == I_SILVER) {
                if (p->skill == S_ENTERTAINMENT) {
                    f->PutStr(AString("Entertainment available: $") +
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
    f->PutStr(temp);
}

int ARegion::HasItem(Faction * fac,int item)
{
    forlist(&objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->faction == fac) {
                if (u->items.GetNum(item)) return 1;
            }
        }
    }
    return 0;
}

void ARegion::WriteMarkets(Areport * f,Faction * fac)
{
    AString temp = "Wanted: ";
    int has = 0;
    forlist(&markets) {
        Market * m = (Market *) elem;
        if (!m->amount) continue;
        if (m->type == M_SELL) {
            if (ItemDefs[m->item].type & IT_ADVANCED) {
                if (!HasItem(fac,m->item)) {
                    continue;
                }
            }
            if (has) {
                temp += ", ";
            } else {
                has = 1;
            }
            temp += m->Report();
        }
    }
    if (!has) temp += "none";
    temp += ".";
    f->PutStr(temp);
    
    temp = "For Sale: ";
    has = 0;
    {
        forlist(&markets) {
            Market * m = (Market *) elem;
            if (!m->amount) continue;
            if (m->type == M_BUY) {
                if (has) {
                    temp += ", ";
                } else {
                    has = 1;
                }
                temp += m->Report();
            }
        }
    }
    if (!has) temp += "none";
    temp += ".";
    f->PutStr(temp);
}

void ARegion::WriteEconomy(Areport * f,Faction * fac)
{
    f->AddTab();
    f->PutStr(AString("Wages: ") + WagesForReport() + ".");
  
    WriteMarkets(f,fac);
  
    WriteProducts(f,fac);
    
    f->EndLine();
    f->DropTab();
}

void ARegion::WriteExits( Areport *f, ARegionList *pRegs )
{
    f->PutStr("Exits:");
    f->AddTab();
    int y = 0;
    for (int i=0; i<NDIRS; i++)
    {
        ARegion * r = neighbors[i];
        if (r)
        {
            f->PutStr(AString(DirectionStrs[i]) + " : " + 
                      r->Print( pRegs ) + ".");
            y = 1;
        }
    }
    if (!y) f->PutStr("none");
    f->DropTab();
    f->EndLine();
}

#define AC_STRING "Atlantis Nexus is a magical place; the entryway " \
"to the world of Atlantis. Enjoy your stay, the city guards should " \
"keep you safe as long as you should choose to stay. However, rumor " \
"has it that once you have left the Nexus, you can never return."

void ARegion::WriteReport(Areport * f,Faction * fac,int month,
                          ARegionList *pRegions )
{
    Farsight *farsight = GetFarsight(&farsees,fac);

    if (farsight || Present(fac) || fac->IsNPC())  {
        AString temp = Print( pRegions );
        if (Population()) {
            temp += AString(", ") + Population() + " peasants";
            if( Globals->RACES_EXIST )
            {
                temp += AString(" (") + ItemDefs[race].names + ")";
            }
            temp += AString( ", $" ) + money;
        }
        temp += ".";
        f->PutStr(temp);
        f->PutStr("-------------------------------------------------"
                  "-----------");
        
        f->AddTab();
        if( Globals->WEATHER_EXISTS )
        {
            if (weather == W_BLIZZARD)
                temp = "There was an unnatural blizzard last month; ";
            if (weather == W_WINTER) 
                temp = "It was winter last month; ";
            if (weather == W_MONSOON)
                temp = "It was monsoon season last month; ";
            if (weather == W_NORMAL)
                temp = "The weather was clear last month; ";
            int nxtweather = pRegions->GetWeather( this, (month + 1) % 12 );
            if (nxtweather == W_WINTER)
                temp += "it will be winter next month.";
            if (nxtweather == W_MONSOON)
                temp += "it will be monsoon season next month.";
            if (nxtweather == W_NORMAL)
                temp += "it will be clear next month.";
            f->PutStr(temp);
        }

        if (num == 0) {
            f->PutStr("");
            f->PutStr(AC_STRING);
            f->PutStr("");
        }
        
        f->DropTab();
        
        WriteEconomy(f,fac);
        
        WriteExits( f, pRegions );
        
        if( Globals->GATES_EXIST && gate )
        {
            int sawgate = 0;
            forlist(&objects) {
                Object *o = (Object *) elem;
				if(!o->units.Num()) {
					if(!sawgate && fac->IsNPC())
					{
                        f->PutStr(AString("There is a Gate here (Gate ") +
                                  gate + " of " +
                                  (pRegions->numberofgates - 1) + ").");
                        f->PutStr("");
                        sawgate = 1;
					}
				}
                forlist(&o->units) {
                    Unit *u = (Unit *) elem;
                    if (!sawgate &&
						((u->faction == fac && u->GetSkill(S_GATE_LORE)) ||
						fac->IsNPC()))
                    {
                        f->PutStr(AString("There is a Gate here (Gate ") +
                                  gate + " of " +
                                  (pRegions->numberofgates - 1) + ").");
                        f->PutStr("");
                        sawgate = 1;
                    }
                }
            }
        }
        
        int obs = GetObservation(fac);
        int truesight = GetTrueSight(fac);
        int detfac = 0;

		if(fac->IsNPC()) obs=10;

        if( S_MIND_READING != -1 )
        {
            forlist (&objects) {
                Object * o = (Object *) elem;
                forlist(&o->units) {
                    Unit * u = (Unit *) elem;
                    if (u->faction == fac && u->GetSkill(S_MIND_READING) > 2)
                    {
                        detfac = 1;
                    }
                }
            }
        }
        
        {
            forlist (&objects) {
                ((Object *) elem)->Report(f,fac,obs,truesight,detfac);
            }
            f->EndLine();
        }
    }
}

// DK
void ARegion::WriteTemplate( Areport *f, 
                             Faction *fac,
                             ARegionList *pRegs,
                             int month )
{
    int header = 0;
    
    forlist (&objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->faction == fac)
            {
                if (!header)
                {
                    // DK
                    if (fac->temformat == TEMPLATE_MAP)
                    {
                        WriteTemplateHeader(f, fac, pRegs, month);
                    }
                    else
                    {
                        f->PutStr("");
                        f->PutStr(AString("*** ") + Print( pRegs ) + " ***",1);
                    }
                    header = 1;
                }
                
                f->PutStr("");
                f->PutStr(AString("unit ") + u->num);
                // DK
                if (fac->temformat == TEMPLATE_LONG ||
                    fac->temformat == TEMPLATE_MAP)
                {
                    f->PutStr(u->TemplateReport(),1);
                }
                forlist(&(u->oldorders)) {
                    f->PutStr(*((AString *) elem));
                }
                u->oldorders.DeleteAll();
            }
        }
    }
}

int ARegion::GetTrueSight(Faction *f)
{
    int truesight = 0;
    if( S_TRUE_SEEING != -1 )
    {
        forlist ((&objects)) {
            Object * obj = (Object *) elem;
            forlist ((&obj->units)) {
                Unit * u = (Unit *) elem;
                if (u->faction == f) {
                    int temp = u->GetSkill(S_TRUE_SEEING);
                    if (temp>truesight) truesight = temp;
                }
            }
        }
    }
    return truesight;
}

int ARegion::GetObservation(Faction * f) 
{
    int obs = 0;
    forlist ((&objects)) {
        Object * obj = (Object *) elem;
        forlist ((&obj->units)) {
            Unit * u = (Unit *) elem;
            if (u->faction == f) {
                int temp = u->GetSkill(S_OBSERVATION);
                if (temp>obs) obs = temp;
            }
        }
    }
    return obs;
}

void ARegion::SetWeather( int newWeather)
{
    weather = newWeather;
}

int ARegion::IsCoastal()
{
    if (type == R_OCEAN) return 1;
    for (int i=0; i<NDIRS; i++) {
        if (neighbors[i] && neighbors[i]->type == R_OCEAN) return 1;
    }
    return 0;
}

int ARegion::MoveCost(int movetype)
{
    int mult = 1;
    if( Globals->WEATHER_EXISTS )
    {
        mult = 2;
        if (weather == W_BLIZZARD) return 10;
        if (weather == W_NORMAL) mult = 1;
        if (clearskies) mult = 1;
    }
    if (movetype == M_WALK || movetype == M_RIDE)
    {
        return (TerrainDefs[type].movepoints * mult);
    }
    return mult;
}

Unit * ARegion::Forbidden(Unit * u)
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        forlist ((&obj->units)) {
            Unit * u2 = (Unit *) elem;
            if (u2->Forbids(this,u)) return u2;
        }
    }
    return 0;
}

Unit * ARegion::ForbiddenByAlly(Unit * u)
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        forlist ((&obj->units)) {
            Unit * u2 = (Unit *) elem;
            if (u->faction->GetAttitude(u2->faction->num) == A_ALLY &&
                u2->Forbids(this,u)) return u2;
        }
    }
    return 0;
}

int ARegion::HasCityGuard()
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        forlist ((&obj->units)) {
            Unit * u = (Unit *) elem;
            if (u->type == U_GUARD && u->GetSoldiers() &&
                u->guard == GUARD_GUARD) {
                return 1;
            }
        }
    }
    return 0;
}

void ARegion::NotifySpell( Unit *caster, int spell, ARegionList *pRegs )
{
    AList flist;

    forlist((&objects)) {
        Object *o = (Object *) elem;
        forlist ((&o->units)) {
            Unit *u = (Unit *) elem;
            if (u->faction == caster->faction) continue;
            if (u->GetSkill(spell))
            {
                if (!GetFaction2(&flist,u->faction->num))
                {
                    FactionPtr *fp = new FactionPtr;
                    fp->ptr = u->faction;
                    flist.Add(fp);
                }
            }
        }
    }

    {
        forlist(&flist) {
            FactionPtr *fp = (FactionPtr *) elem;
            fp->ptr->Event(AString(*(caster->name)) + " uses " + 
                           SkillStrs(spell) +
                           " in " + Print( pRegs ) + ".");
        }
    }
}

// ALT, 26-Jul-2000
// Procedure to notify all units in city about city name change
void ARegion::NotifyCity( Unit *caster, AString& oldname, AString& newname )
{
	AList flist;
	forlist((&objects)) {
		Object *o = (Object *) elem;
		forlist ((&o->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == caster->faction) continue;
			if (!GetFaction2(&flist,u->faction->num))
			{
				FactionPtr *fp = new FactionPtr;
				fp->ptr = u->faction;
				flist.Add(fp);
			}
		}
	}
	{
		forlist(&flist) {
			FactionPtr *fp = (FactionPtr *) elem;
			fp->ptr->Event(AString(*(caster->name)) + " renames " +
					oldname + " to " + newname + ".");
		}
	}
}

int ARegion::CanTax(Unit * u)
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        forlist ((&obj->units)) {
            Unit * u2 = (Unit *) elem;
            if (u2->guard == GUARD_GUARD && u2->IsAlive())
                if (u2->GetAttitude(this,u) <= A_NEUTRAL)
                    return 0;
        }
    }
	return 1;
}

int ARegion::ForbiddenShip(Object * ship)
{
    forlist(&ship->units) {
        Unit * u = (Unit *) elem;
        if (Forbidden(u)) return 1;
    }
    return 0;
}

void ARegion::DefaultOrders()
{
    forlist((&objects)) {
        Object * obj = (Object *) elem;
        forlist ((&obj->units))
            ((Unit *) elem)->DefaultOrders(obj);
    }
}

//
// This is just used for mapping; just check if there is an inner region.
//
int ARegion::HasShaft()
{
    forlist (&objects) {
        Object * o = (Object *) elem;
        if (o->inner != -1) return 1;
    }
    return 0;
}

int ARegion::IsGuarded()
{
    forlist (&objects) {
        Object * o = (Object *) elem;
        forlist (&o->units) {
            Unit * u = (Unit *) elem;
            if (u->guard == GUARD_GUARD) return 1;
        }
    }
    return 0;
}

int ARegion::CountWMons()
{
    int count = 0;
    forlist (&objects) {
        Object * o = (Object *) elem;
        forlist (&o->units) {
            Unit * u = (Unit *) elem;
            if (u->type == U_WMON)
            {
                count ++;
            }
        }
    }
    return count;
}

ARegionList::ARegionList()
{
    pRegionArrays = 0;
    numLevels = 0;
    numberofgates = 0;
}

ARegionList::~ARegionList()
{
    if( pRegionArrays )
    {
        int i;
        for( i = 0; i < numLevels; i++ )
        {
            delete pRegionArrays[ i ];
        }

        delete pRegionArrays;
    }
}

void ARegionList::WriteRegions( Aoutfile * f )
{
    f->PutInt(Num());

    f->PutInt( numLevels );
    int i;
    for( i = 0; i < numLevels; i++ )
    {
        ARegionArray *pRegs = pRegionArrays[ i ];
        f->PutInt( pRegs->x );
        f->PutInt( pRegs->y );
        if( pRegs->strName )
        {
            f->PutStr( *pRegs->strName );
        }
        else
        {
            f->PutStr( "none" );
        }
        f->PutInt( pRegs->levelType );
    }

    f->PutInt( numberofgates );
    {
        forlist(this) {
            ((ARegion *) elem)->Writeout( f );
        }
    }
    {
        f->PutStr("Neighbors");
        forlist(this) {
            ARegion * reg = (ARegion *) elem;
            for( i = 0; i < NDIRS; i++ )
            {
                if (reg->neighbors[i]) {
                    f->PutInt(reg->neighbors[i]->num);
                } else {
                    f->PutInt(-1);
                }
            }
        }
    }
}

void ARegionList::ReadRegions(Ainfile * f,AList * factions, ATL_VER v )
{
    int num = f->GetInt();

    numLevels = f->GetInt();
    CreateLevels( numLevels );
    int i;
    for( i = 0; i < numLevels; i++ )
    {
        int curX = f->GetInt();
        int curY = f->GetInt();
        AString *name = f->GetStr();
        ARegionArray *pRegs = new ARegionArray( curX, curY );
        if( *name == "none" )
        {
            pRegs->strName = 0;
            delete name;
        }
        else
        {
            pRegs->strName = name;
        }
        pRegs->levelType = f->GetInt();
        pRegionArrays[ i ] = pRegs;
    }

    numberofgates = f->GetInt();
  
    ARegionFlatArray fa(num);

    Awrite("Reading the regions...");
    for( i = 0; i < num; i++ )
    {
        ARegion *temp = new ARegion;
        temp->Readin(f,factions,v);
        fa.SetRegion(temp->num,temp);
        Add(temp);

        pRegionArrays[ temp->zloc ]->SetRegion( temp->xloc, temp->yloc,
                                                temp );
    }
  
    Awrite("Setting up the neighbors...");
    {
        delete f->GetStr();
        forlist(this) {
            ARegion * reg = (ARegion *) elem;
            for( i = 0; i < NDIRS; i++ )
            {
                int j = f->GetInt();
                if (j != 0 && j != -1)
                {
                    reg->neighbors[i] = fa.GetRegion(j);
                } 
                else
                {
                    reg->neighbors[i] = 0;
                }
            }
        }
    }
}

ARegion * ARegionList::GetRegion(int n)
{
    forlist(this) {
        if (((ARegion *) elem)->num == n) return ((ARegion *) elem);
    }
    return 0;
}

ARegion *ARegionList::GetRegion( int x, int y, int z )
{
    ARegionArray *arr = pRegionArrays[ z ];

    x = ( x + arr->x ) % arr->x;
    y = ( y + arr->y ) % arr->y;

    return( arr->GetRegion( x, y ));
}

Location * ARegionList::FindUnit(int i) {
  forlist(this) {
    ARegion * reg = (ARegion *) elem;
    forlist((&reg->objects)) {
      Object * obj = (Object *) elem;
      forlist((&obj->units)) {
	Unit * u = (Unit *) elem;
	if (u->num == i) {
	  Location * retval = new Location;
	  retval->unit = u;
	  retval->region = reg;
	  retval->obj = obj;
	  return retval;
	}
      }
    }
  }
  return 0;
}

void ARegionList::NeighSetup(ARegion * r,ARegionArray * ar)
{
    r->ZeroNeighbors();

    if (r->yloc != 0 && r->yloc != 1)
    {
        r->neighbors[D_NORTH] = ar->GetRegion(r->xloc,r->yloc - 2);
    }
    if (r->yloc != 0)
    {
        r->neighbors[D_NORTHEAST] = ar->GetRegion(r->xloc + 1,r->yloc - 1);
        r->neighbors[D_NORTHWEST] = ar->GetRegion(r->xloc - 1,r->yloc - 1);
    }
    if (r->yloc != ar->y - 1)
    {
        r->neighbors[D_SOUTHEAST] = ar->GetRegion(r->xloc + 1,r->yloc + 1);
        r->neighbors[D_SOUTHWEST] = ar->GetRegion(r->xloc - 1,r->yloc + 1);
    }
    if (r->yloc != ar->y - 1 && r->yloc != ar->y - 2)
    {
        r->neighbors[D_SOUTH] = ar->GetRegion(r->xloc,r->yloc + 2);
    }
}

void ARegionList::CreateAbyssLevel( int level, char *name )
{
	MakeRegions(level, 4, 4);
	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_NEXUS;

	ARegion *reg = NULL;
	for(int x = 0; x < 4; x++) {
		for(int y = 0; y < 4; y++) {
			reg = pRegionArrays[level]->GetRegion(x, y);
			if(!reg) continue;
			reg->SetName("Abyssal Plains");
			reg->type = R_DESERT;
			reg->wages = -2;
		}
	}
	int gateset = 0;
	int tempx, tempy;
	do {
        tempx = getrandom(4);
        tempy = getrandom(4);
		reg = pRegionArrays[level]->GetRegion(tempx, tempy);
		if(reg) {
			gateset = 1;
			numberofgates++;
			reg->gate = -1;
		}
	} while(!gateset);

    FinalSetup( pRegionArrays[ level ] );

	ARegion *lair = NULL;
	do {
        tempx = getrandom(4);
        tempy = getrandom(4);
		lair = pRegionArrays[level]->GetRegion(tempx, tempy);
	} while(!lair || lair == reg);
	Object *o = new Object(lair);
	o->num = lair->buildingseq++;
	o->name = new AString(AString(ObjectDefs[O_BKEEP].name)+" ["+o->num+"]");
	o->type = O_BKEEP;
	o->incomplete = 0;
	o->inner = -1;
	lair->objects.Add(o);
}


void ARegionList::CreateNexusLevel( int level, char *name )
{
    MakeRegions( level, 1, 1 );

    pRegionArrays[ level ]->SetName( name );
    pRegionArrays[ level ]->levelType = ARegionArray::LEVEL_NEXUS;
    ARegion *reg = pRegionArrays[ level ]->GetRegion( 0, 0 );

    reg->SetName("Atlantis Nexus");
    reg->type = R_NEXUS;

    FinalSetup( pRegionArrays[ level ] );
}

void ARegionList::CreateSurfaceLevel( int level,
                                      int xSize,
                                      int ySize,
                                      int percentOcean,
                                      int continentSize,
                                      char *name )
{
    MakeRegions( level, xSize, ySize );

    pRegionArrays[ level ]->SetName( name );
    pRegionArrays[ level ]->levelType = ARegionArray::LEVEL_SURFACE;
    MakeLand( pRegionArrays[ level ], percentOcean, continentSize );

    SetupAnchors( pRegionArrays[ level ] );

    GrowTerrain( pRegionArrays[ level ], 0 );

    AssignTypes( pRegionArrays[ level ] );

    FinalSetup( pRegionArrays[ level ] );
}

void ARegionList::CreateIslandLevel( int level,
                                     int nPlayers,
                                     char *name )
{
    int xSize, ySize;
    xSize = 20 + ( nPlayers + 3 ) / 4 * 6 - 2;
    ySize = xSize;

    MakeRegions( level, xSize, ySize );

    pRegionArrays[ level ]->SetName( name );
    pRegionArrays[ level ]->levelType = ARegionArray::LEVEL_SURFACE;

    MakeCentralLand( pRegionArrays[ level ] );
    MakeIslands( pRegionArrays[ level ], nPlayers );
    RandomTerrain( pRegionArrays[ level ] );

    FinalSetup( pRegionArrays[ level ] );
}

void ARegionList::CreateUnderworldLevel( int level, 
                                         int xSize,
                                         int ySize,
                                         char *name )
{
    MakeRegions( level, xSize, ySize );

    pRegionArrays[ level ]->SetName( name );
    pRegionArrays[ level ]->levelType = ARegionArray::LEVEL_UNDERWORLD;

    SetRegTypes( pRegionArrays[ level ], R_NUM );

    SetupAnchors( pRegionArrays[ level ] );

    GrowTerrain( pRegionArrays[ level ], 1 );

    AssignTypes( pRegionArrays[ level ] );

    MakeUWMaze( pRegionArrays[ level ] );

    FinalSetup( pRegionArrays[ level ] );
}

void ARegionList::MakeRegions( int level, int xSize, int ySize )
{
    Awrite( "Making a level..." );

    ARegionArray *arr = new ARegionArray( xSize, ySize );
    pRegionArrays[ level ] = arr;

    //
    // Make the regions themselves
    //
    int x, y;
    for( y = 0; y < ySize; y++ )
    {
        for( x = 0; x < xSize; x++ )
        {
            if( !(( x + y ) % 2 ))
            {
                ARegion *reg = new ARegion;
                reg->SetLoc( x, y, level );
                reg->num = Num();

                //
                // Some initial values; these will get reset
                //
                reg->type = -1;
                reg->race = -1;
                reg->wages = -1;
                reg->maxwages = -1;
                
                Add( reg );
                arr->SetRegion( x, y, reg );
            }
        }
    }

    SetupNeighbors( arr );

    Awrite("");
}

void ARegionList::SetupNeighbors( ARegionArray *pRegs )
{
    int x, y;
    for( x = 0; x < pRegs->x; x++ )
    {
        for( y = 0; y < pRegs->y; y++ )
        {
            ARegion *reg = pRegs->GetRegion( x, y );
            if( !reg )
            {
                continue;
            }

            NeighSetup( reg, pRegs );
        }
    }
}

void ARegionList::MakeLand( ARegionArray *pRegs, 
                            int percentOcean,
                            int continentSize )
{
    int total = pRegs->x * pRegs->y / 2; 
    int ocean = total;
  
    Awrite("Making land");
    while (ocean > ( total * percentOcean ) / 100)
    {
        int sz = getrandom( continentSize );
        sz = sz * sz;

        int tempx = getrandom( pRegs->x );
        int tempy = getrandom( pRegs->y / 2 ) * 2 + tempx % 2;

        ARegion *reg = pRegs->GetRegion(tempx,tempy);

        if (reg->type == -1)
        {
            reg->type = R_NUM;
            ocean--;
        }
    
        int i;
        for (i=0; i<sz; i++)
        {
            ARegion *newreg = reg->neighbors[getrandom(NDIRS)];
            while (!newreg) newreg = reg->neighbors[getrandom(NDIRS)];
            reg = newreg;
            if (reg->type == -1)
            {
                reg->type = R_NUM;
                ocean--;
            }
        }
    }

    //
    // At this point, go back through and set all the rest to ocean
    //
    SetRegTypes( pRegs, R_OCEAN );

    Awrite("");
}

void ARegionList::MakeCentralLand( ARegionArray *pRegs )
{
    int i, j;
    for( i = 0; i < pRegs->x; i++ )
    {
        for( j = 0; j < pRegs->y; j++ )
        {
            ARegion *reg = pRegs->GetRegion( i, j );
            if( !reg )
            {
                continue;
            }

            //
            // Initialize region to ocean.
            //
            reg->type = R_OCEAN;

            //
            // If the region is close to the edges, it stays ocean
            //
            if( i < 8 || i >= pRegs->x - 8 ||
                j < 8 || j >= pRegs->y - 8 )
            {
                continue;
            }

            //
            // If the region is within 10 of the edges, it has a 50%
            // chance of staying ocean.
            //
            if( i < 10 || i >= pRegs->x - 10 ||
                j < 10 || j >= pRegs->y - 10 )
            {
                if( getrandom( 100 ) > 50 )
                {
                    continue;
                }
            }

            //
            // Otherwise, set the region to land.
            //
            reg->type = R_NUM;
        }
    }
}

void ARegionList::MakeIslands( ARegionArray *pArr, int nPlayers )
{
    //
    // First, make the islands along the top.
    //
    int nRow = ( nPlayers + 3 ) / 4;
    int i;
    for( i = 0; i < nRow; i++ )
    {
        MakeOneIsland( pArr, 10 + i * 6, 2 );
    }

    //
    // Next, along the left.
    //
    nRow = ( nPlayers + 2 ) / 4;
    for( i = 0; i < nRow; i++ )
    {
        MakeOneIsland( pArr, 2, 10 + i * 6 );
    }

    //
    // The islands along the bottom.
    //
    nRow = ( nPlayers + 1 ) / 4;
    for( i = 0; i < nRow; i++ )
    {
        MakeOneIsland( pArr, 10 + i * 6, pArr->y - 6 );
    }

    //
    // And the islands on the right.
    //
    nRow = nPlayers / 4;
    for( i = 0; i < nRow; i++ )
    {
        MakeOneIsland( pArr, pArr->x - 6, 10 + i * 6 );
    }
}

void ARegionList::MakeOneIsland( ARegionArray *pRegs, int xx, int yy )
{
    int i, j;
    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 4; j++ )
        {
            ARegion *pReg = pRegs->GetRegion( i + xx, j + yy );
            if( !pReg )
            {
                continue;
            }

            pReg->type = R_NUM;
        }
    }
}

void ARegionList::SetRegTypes( ARegionArray *pRegs, int newType )
{
    int i, j;
    for( i = 0; i < pRegs->x; i++ )
    {
        for( j = 0; j < pRegs->y; j++ )
        {
            ARegion *reg = pRegs->GetRegion( i, j );
            if( !reg )
            {
                continue;
            }

            if( reg->type == -1 )
            {
                reg->type = newType;
            }
        }
    }
}

void ARegionList::SetupAnchors(ARegionArray * ta)
{
    /* Now, setup the anchors */
    Awrite("Setting up the anchors");
    for (int x=0; x<(ta->x)/4; x++)
    {
        for (int y=0; y<(ta->y)/8; y++)
        {
            ARegion * reg = 0;
            for (int i=0; i<4; i++)
            {
                int tempx = x * 4 + getrandom(4);
                int tempy = y * 8 + getrandom(4)*2 + tempx%2;
                reg = ta->GetRegion(tempx,tempy);
                if (reg->type == R_NUM)
                {
                    reg->type = GetRegType( reg );

                    if (reg->type != R_OCEAN)
                    {
                        reg->wages = AGetName( reg, 0 );
                    }

                    break;
                }
            }
            Adot();
        }
    }

    Awrite("");
}

void ARegionList::GrowTerrain( ARegionArray *pArr, int growOcean )
{
    for (int j=0; j<10; j++)
    {
        int x, y;
        for( x = 0; x < pArr->x; x++ )
        {
            for( y = 0; y < pArr->y; y++ )
            {
                ARegion *reg = pArr->GetRegion( x, y );
                if( !reg )
                {
                    continue;
                }

                if (reg->type == R_NUM)
                {
                    int init = getrandom(6);
                    for (int i=0; i<NDIRS; i++)
                    {
                        ARegion * t = reg->neighbors[(i+init) % NDIRS];
                        if (t)
                        {
                            if( t->type != R_NUM &&
                                ( t->type != R_OCEAN || growOcean ))
                            {
                                reg->race = t->type;
                                reg->wages = t->wages;
                                break;
                            }
                        }
                    }
                }
            }
        }

        for( x = 0; x < pArr->x; x++ )
        {
            for( y = 0; y < pArr->y; y++ )
            {
                ARegion *reg = pArr->GetRegion( x, y );
                if( !reg )
                {
                    continue;
                }

                if( reg->type == R_NUM && reg->race != -1 )
                {
                    reg->type = reg->race;
                }
            }
        }
    }
}

void ARegionList::RandomTerrain( ARegionArray *pArr )
{
    int x, y;
    for( x = 0; x < pArr->x; x++ )
    {
        for( y = 0; y < pArr->y; y++ )
        {
            ARegion *reg = pArr->GetRegion( x, y );
            if( !reg )
            {
                continue;
            }

            if (reg->type == R_NUM)
            {
                reg->type = GetRegType( reg );
                reg->wages = AGetName( reg, 0 );
            }
        }
    }
}

void ARegionList::MakeUWMaze( ARegionArray *pArr )
{
    int x, y;

    for( x = 0; x < pArr->x; x++ )
    {
        for( y = 0; y < pArr->y; y++ )
        {
            ARegion *reg = pArr->GetRegion( x, y );
            if( !reg )
            {
                continue;
            }

            for (int i=D_SOUTHEAST; i<= D_SOUTHWEST; i++)
            {
				int count = 0;
				for(int j=D_NORTH; j< NDIRS; j++)
					if(reg->neighbors[j]) count++;
				if(count <= 1) break;

                ARegion *n = reg->neighbors[i];
                if (n)
                {
                    if( !CheckRegionExit( i, reg, n ))
                    {
						count = 0;
						for(int k = D_NORTH; k<NDIRS; k++) {
							if(n->neighbors[k]) count++;
						}
						if(count <= 1) break;
                        reg->neighbors[i] = 0;
                        n->neighbors[(i+3) % NDIRS] = 0;
                    }
                }
			}
        }
    }
}

void ARegionList::AssignTypes( ARegionArray *pArr )
{
    //
    // RandomTerrain() will set all of the un-set region types and names.
    //
    RandomTerrain( pArr );
}

void ARegionList::FinalSetup( ARegionArray *pArr )
{
    int x, y;
    for( x = 0; x < pArr->x; x++ )
    {
        for( y = 0; y < pArr->y; y++ )
        {
            ARegion *reg = pArr->GetRegion( x, y );
            if( !reg )
            {
                continue;
            }

            if (reg->type == R_OCEAN)
            {
                //
                // xxxxx
                //
                if( pArr->levelType == ARegionArray::LEVEL_UNDERWORLD )
                {
                    reg->SetName("The Undersea");
                } 
                else
                {
                    reg->SetName("Atlantis Ocean");
                }
            } 
            else
            {
                if (reg->wages == -1)
                {
                    reg->SetName("Unnamed");
                } 
                else if(reg->wages != -2)
                {
                    reg->SetName( AGetNameString( reg->wages ));
                }
				else
				{
					reg->wages = -1;
				}
            }

            reg->Setup();
        }
    }
}

void ARegionList::MakeShaft( ARegion *reg, ARegionArray *pFrom, 
                             ARegionArray *pTo )
{
    if( reg->type == R_OCEAN )
    {
        return;
    }

    int tempx = reg->xloc * pTo->x / pFrom->x + 
        getrandom( pTo->x / pFrom->x );
    int tempy = reg->yloc * pTo->y / pFrom->y + 
        getrandom( pTo->y / pFrom->y );
    //
    // Make sure we get a valid region.
    //
    tempy += ( tempx + tempy ) % 2;

    ARegion *temp = pTo->GetRegion( tempx, tempy );
    if( temp->type == R_OCEAN )
    {
        return;
    }

	Object * o = new Object( reg );
	o->num = reg->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = temp->num;
	reg->objects.Add(o);
	
	o = new Object( reg );
	o->num = temp->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = reg->num;
	temp->objects.Add(o);
}

void ARegionList::MakeShaftLinks( int levelFrom, int levelTo, int odds )
{
    ARegionArray *pFrom = pRegionArrays[ levelFrom ];
    ARegionArray *pTo = pRegionArrays[ levelTo ];

    int x, y;
    for( x = 0; x < pFrom->x; x++ )
    {
        for( y = 0; y < pFrom->y; y++ )
        {
            ARegion *reg = pFrom->GetRegion( x, y );
            if( !reg )
            {
                continue;
            }

            if( getrandom( odds ) != 0 )
            {
                continue;
            }

            MakeShaft( reg, pFrom, pTo );
        }
    }
}

void ARegionList::CalcDensities()
{
    Awrite("Densities:");
    int arr[R_NUM];
    int i;
    for (i=0; i<R_NUM; i++) 
    {
        arr[i] = 0;
    }
    {
        forlist(this) {
            ARegion * reg = ((ARegion *) elem);
            arr[reg->type]++;
        }
    }
    for (i=0; i<R_NUM; i++)
    {
        Awrite(AString(TerrainDefs[i].name) + " " + arr[i]);
    }
    
    Awrite("");
}

void ARegionList::SetACNeighbors( int levelSrc, 
                                  int levelTo, 
                                  int maxX, 
                                  int maxY )
{
    ARegion *AC = GetRegion( 0, 0, levelSrc );
    for (int i=0; i<NDIRS; i++)
    {
        ARegion *pReg = GetStartingCity( AC, i, levelTo, maxX, maxY );
        AC->neighbors[i] = pReg;
        pReg->MakeStartingCity();
        numberofgates++;
    }
}

void ARegionList::InitSetupGates( int level )
{
    ARegionArray *pArr = pRegionArrays[ level ];

    int i, j, k;
    for (i=0; i<pArr->x / 8; i++)
    {
        for (j=0; j<pArr->y / 16; j++)
        {
            for (k=0; k<5; k++)
            {
                int tempx = i*8 + getrandom(8);
                int tempy = j*16 + getrandom(8)*2 + tempx%2;
                ARegion *temp = pArr->GetRegion(tempx,tempy);
                if (temp->type != R_OCEAN && temp->gate != -1)
                {
                    numberofgates++;
                    temp->gate = -1;
                    break;
                }
            }
        }
    }
}

void ARegionList::FinalSetupGates()
{
    int *used = new int[numberofgates];

    int i;
    for (i=0; i<numberofgates; i++) used[i] = 0;
    
    forlist(this) {
        ARegion *r = (ARegion *) elem;
        
        if (r->gate == -1)
        {
            int index = getrandom(numberofgates);
            while (used[index])
            {
                index++;
                index = index % numberofgates;
            }
            r->gate = index;
            used[index] = 1;
        }
    }
    delete used;
}

ARegion *ARegionList::FindGate(int x) {
  if (!x) return 0;
  forlist(this) {
    ARegion *r = (ARegion *) elem;
    if (r->gate == x) return r;
  }
  return 0;
}

int ARegionList::GetPlanarDistance(ARegion *one, ARegion*two)
{
	if(one->zloc == ARegionArray::LEVEL_NEXUS ||
			two->zloc == ARegionArray::LEVEL_NEXUS) {
		return 10000000;
	}

	int one_x, one_y, two_x, two_y;
	int maxy;
    ARegionArray *pArr=pRegionArrays[ARegionArray::LEVEL_SURFACE];

	if(one->zloc == ARegionArray::LEVEL_SURFACE) {
		one_x = one->xloc;
		one_y = one->yloc;
	} else {
		one_x = one->xloc * 2;
		one_y = one->yloc * 2;
	}

	if(two->zloc == ARegionArray::LEVEL_SURFACE) {
		two_x = two->xloc;
		two_y = two->xloc;
	} else {
		two_x = two->xloc * 2;
		two_y = two->xloc * 2;
	}
    maxy = one_y - two_y;
	if(maxy < 0) maxy=-maxy;

	int maxx = one_x - two_x;
	if(maxx < 0) maxx = -maxx;

	int max2 = one_x - pArr->x - two_x;
	if(max2 < 0) max2 = -max2;
	if(max2 < maxx) maxx = max2;

	max2 = one_x - (two_x + pArr->x);
	if(max2 < 0) max2 = -max2;
	if(max2 < maxx) maxx = max2;

	if(maxy > maxx)
		maxx = (maxx+maxy)/2;

	if(one->zloc != two->zloc)
		maxx += 4;

	return maxx;
}

int ARegionList::GetDistance(ARegion *one,ARegion *two)
{
    if( one->zloc != two->zloc )
    {
        return( 10000000 );
    }

    ARegionArray *pArr = pRegionArrays[ one->zloc ];

    int maxy;
    maxy = one->yloc - two->yloc;
    if (maxy < 0) maxy = -maxy;

    int maxx = one->xloc - two->xloc;
    if (maxx < 0) maxx = -maxx;

    int max2 = one->xloc + pArr->x - two->xloc;
    if (max2 < 0) max2 = -max2;
    if (max2 < maxx) maxx = max2;

    max2 = one->xloc - (two->xloc + pArr->x);
    if (max2 < 0) max2 = -max2;
    if (max2 < maxx) maxx = max2;

    if (maxy > maxx) return (maxx + maxy) / 2;
    return maxx;
}

ARegionArray *ARegionList::GetRegionArray( int level )
{
    return( pRegionArrays[ level ] );
}

void ARegionList::CreateLevels( int n )
{
    numLevels = n;
    pRegionArrays = new ARegionArray *[ n ];
}

ARegionArray::ARegionArray(int xx,int yy)
{
    x = xx;
    y = yy;
    regions = new ARegion *[ x * y / 2 + 1];
    strName = 0;

    int i;
    for( i = 0; i < x * y / 2; i++ )
    {
        regions[ i ] = 0;
    }
}

ARegionArray::~ARegionArray()
{
    if (strName) delete strName;
    delete [] regions;
}

void ARegionArray::SetRegion(int xx,int yy,ARegion * r) 
{
    regions[ xx / 2 + yy * x / 2 ] = r;
}

ARegion * ARegionArray::GetRegion(int xx,int yy)
{
    xx = (xx + x) % x;
    yy = (yy + y) % y;
    if(( xx + yy ) % 2 )
    {
        return( 0 );
    }
    return( regions[ xx / 2 + yy * x / 2 ] );
}

void ARegionArray::SetName( char *name )
{
    if( name )
    {
        strName = new AString( name );
    }
    else
    {
        delete strName;
        strName = 0;
    }
}

ARegionFlatArray::ARegionFlatArray(int s)
{
    size = s;
    regions = new ARegion *[s];
}

ARegionFlatArray::~ARegionFlatArray()
{
    if (regions) delete regions;
}

void ARegionFlatArray::SetRegion(int x,ARegion * r) {
    regions[x] = r;
}

ARegion * ARegionFlatArray::GetRegion(int x) {
    return regions[x];
}

