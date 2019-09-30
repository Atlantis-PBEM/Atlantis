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

#include <stdio.h>
#include <string.h>
#include "game.h"
#include "gamedata.h"

// replace with [[maybe_unused]] when c++17 is supported
#define MAYBE_UNUSED __attribute__ ((unused))

/* TownType */

int TownInfo::TownType()
{
	if (pop < 1000*Globals->POP_LEVEL) return TOWN_VILLAGE;
	if (pop < 2000*Globals->POP_LEVEL) return TOWN_TOWN;
	return TOWN_CITY;
}

int ARegion::Population()
{
	if (town) {
		return population + town->pop;
	} else {
		return population;
	}
}

void ARegion::WagesFromDevelopment()
{
	// Calculate new wages
	wages = 0;
	if (Population() == 0) return;
	int level = 1;
	int last = 0;
	int dv = Development();
	while (dv >= level) {
		wages++;
		last = level;
		level += wages+1;
	}
	wages *= 10;
	if (dv > last)
		wages += 10 * (dv - last) / (level - last);
}

int ARegion::Wages()
{
	int retval;
	if (Globals->PLAYER_ECONOMY) {
		WagesFromDevelopment();
		retval = (int) wages/10;
	} else {
		retval = wages;
		if (town) {
			// hack, assumes that TownType + 1 = town wages
			retval = retval + town->TownType() + 1;
		}
		// AS
		if (CountConnectingRoads() > 1) retval++;
		
		// Check Lake Wage Effect
		if(LakeEffect()) retval++;
	}
	
	if (earthlore) retval++;
	if (clearskies) retval++;
	return retval;
}

int ARegion::LakeEffect()
{
	int raise = 0;
	if (Globals->LAKE_WAGE_EFFECT != GameDefs::NO_EFFECT) {
		int adjlake = 0;
		for (int d = 0; d < NDIRS; d++) {
			ARegion *check = neighbors[d];
			if(!check) continue;
			if(check->type == R_LAKE) adjlake++;
		}
		if (adjlake > 0) {
			// If lakes affect everything around them
			if (Globals->LAKE_WAGE_EFFECT & GameDefs::ALL)
				raise = 1;
			if (TerrainDefs[type].similar_type != R_PLAIN) {
				// If lakes affect towns, but only in non-plains
				if ((Globals->LAKE_WAGE_EFFECT &
						GameDefs::NONPLAINS_TOWNS_ONLY) && town)
					raise = 1;
				// If lakes affect all towns
				if ((Globals->LAKE_WAGE_EFFECT & GameDefs::TOWNS) && town)
					raise = 1;
				// If lakes affect any non plains terrain
				if (Globals->LAKE_WAGE_EFFECT & GameDefs::NONPLAINS)
					raise = 1;
				// If lakes affect only desert
				if((Globals->LAKE_WAGE_EFFECT & GameDefs::DESERT_ONLY) &&
					(TerrainDefs[type].similar_type == R_DESERT))
					raise = 1;
			} else {
				// If lakes affect any town, even those in plains
				if ((Globals->LAKE_WAGE_EFFECT & GameDefs::TOWNS) && town)
					raise = 1;
			}
		}
	}	
	return raise;
}

AString ARegion::WagesForReport()
{
	Production *p = products.GetProd(I_SILVER, -1);
	if (p) {
		if (Globals->PLAYER_ECONOMY) {
			return AString("$") + (p->productivity / 10) +
				"." + (p->productivity % 10) + " (Max: $" + p->amount + ")";
		} else {
			return AString("$") + p->productivity + " (Max: $" + p->amount + ")";
		}
	} else
		return AString("$") + 0;
}

void ARegion::SetupPop()
{

	TerrainType *typer = &(TerrainDefs[type]);
	habitat = typer->pop+1;
	if (habitat < 100) habitat = 100;
	habitat *= Globals->POP_LEVEL;

	int pop = typer->pop*Globals->POP_LEVEL;
	int mw = typer->wages;

	// fix economy when MAINTENANCE_COST has been adjusted
	mw += Globals->MAINTENANCE_COST - 10;
	if (mw < 0) mw = 0;
	
	if (pop == 0) {
		population = 0;
		basepopulation = 0;
		wages = 0;
		maxwages = 0;
		money = 0;
		if(typer->similar_type == R_OCEAN) race = I_MERMEN;

		/*
		if(Globals->PLAYER_ECONOMY) {
			// All regions need silver production
			// even if no income can be gained
			Production *p = new Production;
			p->itemtype = I_SILVER;
			p->amount = 0;
			p->skill = -1;
			p->productivity = 0;
			products.Add(p);

			p = new Production;
			p->itemtype = I_SILVER;
			p->amount = 0;
			p->skill = S_ENTERTAINMENT;
			p->productivity = Globals->ENTERTAIN_INCOME;
			products.Add(p);
		}
		*/

		return;
	}
	// Only select race here if it hasn't been set during Race Growth
	// in the World Creation process.
	if ((race == -1) || (!Globals->GROW_RACES)) {
		int noncoastalraces = sizeof(typer->races)/sizeof(int);
		int allraces =
			noncoastalraces + sizeof(typer->coastal_races)/sizeof(int);

		race = -1;
		while (race == -1 || (ItemDefs[race].flags & ItemType::DISABLED)) {
			int n = getrandom(IsCoastal() ? allraces : noncoastalraces);
			if(n > noncoastalraces-1) {
				race = typer->coastal_races[n-noncoastalraces-1];
			} else
				race = typer->races[n];
		}
	}
	if(Globals->PLAYER_ECONOMY) {
		if(Globals->RANDOM_ECONOMY)	habitat = habitat * 2/3 + getrandom(habitat/3);
		ManType *mt = FindRace(ItemDefs[race].abr);
		if (mt->terrain == typer->similar_type) {
			habitat = (habitat * 9)/8;
		}
		if (!IsNativeRace(race)) {
			habitat = (habitat * 4)/5;
		}
		basepopulation = habitat;
		// hmm... somewhere not too far off equilibrium pop
		population = habitat * 66 / 100;
	} else {
		if(Globals->RANDOM_ECONOMY) {
			population = (pop + getrandom(pop)) / 2;
		} else {
			population = pop;
		}
		basepopulation = population;
	}
	// Setup wages
	if(Globals->PLAYER_ECONOMY) {
		int level = 1;
		development = 1;
		int prev = 0;
		while (level < mw) {
			development++;
			prev++;
			if(prev > level) {
				level++;
				prev = 0;
			}
		}
		if(Globals->RANDOM_ECONOMY) {
			development += getrandom(36);
		}
	} else {
		if(Globals->RANDOM_ECONOMY) {
			mw += getrandom(3);
		}
		wages = mw;
		maxwages = mw;
	}
	if(Globals->TOWNS_EXIST) {
		int adjacent = 0;
		int prob = Globals->TOWN_PROBABILITY;
		if (prob < 1) prob = 100;
		int townch = (int) 80000 / prob;
		if (Globals->TOWNS_NOT_ADJACENT) {
			for (int d = 0; d < NDIRS; d++) {
				ARegion *newregion = neighbors[d];
				if ((newregion) &&  (newregion->town)) adjacent++;
			}
		}
		if(Globals->LESS_ARCTIC_TOWNS) {
			int dnorth = GetPoleDistance(D_NORTH);
			int dsouth = GetPoleDistance(D_SOUTH);
			if (dnorth < 9)
				townch = townch + 25 * (9 - dnorth) *
					(9 - dnorth) * Globals->LESS_ARCTIC_TOWNS;
			if (dsouth < 9)
				townch = townch + 25 * (9 - dsouth) *
					(9 - dsouth) * Globals->LESS_ARCTIC_TOWNS;
		}
		int spread = Globals->TOWN_SPREAD;
		if(spread > 100) spread = 100;
		int townprob = (TerrainDefs[type].economy * 4 * (100 - spread) +        
			100 * spread) / 100;
		if (adjacent > 0) townprob = townprob * (100 - Globals->TOWNS_NOT_ADJACENT) / 100;
		if (getrandom(townch) < townprob) AddTown();
	}

	Production *p = new Production;
	p->itemtype = I_SILVER;
	money = Population() * (Wages() - (Globals->MAINTENANCE_COST/2)) / (2*Globals->POP_LEVEL);  //BS Edit

	if(Globals->PLAYER_ECONOMY) {
		WagesFromDevelopment();
		maxwages = wages;
		money = Population() * (wages - 100) / 10;
	}	
	
	if (money < 0) money = 0;
	p->amount = money / Globals->WORK_FRACTION;
	p->skill = -1;
	if (Globals->PLAYER_ECONOMY) p->productivity = wages;
		else p->productivity = Wages();
	products.Add(p);

	//
	// Setup entertainment
	//
	p = new Production;
	p->itemtype = I_SILVER;
	p->amount = money / Globals->ENTERTAIN_FRACTION;
	p->skill = S_ENTERTAINMENT;
	p->productivity = Globals->ENTERTAIN_INCOME;
	products.Add(p);

	float ratio = ItemDefs[race].baseprice / (float)Globals->BASE_MAN_COST;
	// Setup Recruiting
	Market *m = new Market(M_BUY, race, (int)(Wages()*4*ratio),
							Population()/(5*Globals->POP_LEVEL), 0, 10000, 0, 2000);
	markets.Add(m);

	if(Globals->LEADERS_EXIST) {
		ratio = ItemDefs[I_LEADERS].baseprice / (float)Globals->BASE_MAN_COST;
		m = new Market(M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
						Population()/(25*Globals->POP_LEVEL), 0, 10000, 0, 400);
		markets.Add(m);
	}
}

void ARegion::DisbandInRegion(int item, int amt)
{
	if (!Globals->PLAYER_ECONOMY) return;
	if (amt > 0) {
		if (amt > Population()) {
			// exchange region race!
			race = item;
			population = 0;
			if(town) town->pop = 0;
			AdjustPop(amt);
		} else {
			if (race != item) amt = amt * 2 / 3;
			AdjustPop(amt);
		}
	}
}

void ARegion::Recruit(int amt)
{
	if (!Globals->PLAYER_ECONOMY) return;
	AdjustPop(-amt);
}

void ARegion::AdjustPop(int adjustment)
{
	if(!town) {
		population += adjustment;
		return;
	}
	
	// split between town and rural pop
	int tspace = town->basepop - town->pop;
	int rspace = habitat - population;
	town->pop += adjustment * tspace / (tspace + rspace);
	if(town->pop < 0) town->pop = 0;
	population += adjustment * rspace / (tspace + rspace);
	if (population < 0) population = 0;

}

void ARegion::SetupCityMarket()
{
	int numtrade = 0;
	int cap;
	int offset = 0;
	int citymax = Globals->CITY_POP*Globals->POP_LEVEL;
	ManType *locals = FindRace(ItemDefs[race].abr);
	if(!locals) locals = FindRace("SELF");
	/* compose array of possible supply & demand items */
	int supply[NITEMS]; 
	int demand[NITEMS];
	/* possible advanced and magic items */
	int rare[NITEMS];
	int antiques[NITEMS];
	int i;
	for (i=0; i<NITEMS; i++) {
		supply[i] = 0;
		demand[i] = 0;
		rare[i] = 0;
		antiques[i] = 0;
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(ItemDefs[i].flags & ItemType::NOMARKET) continue;
		if(ItemDefs[i].type & IT_TRADE) numtrade++;
		if (i==I_SILVER) continue;
		if((ItemDefs[i].type & IT_MAN)
			|| (ItemDefs[i].type & IT_LEADER)) continue;
		
		int canProduceHere = 0;
		// Check if the product can be produced in the region
		// Raw goods
		if (ItemDefs[i].pInput[0].item == -1) {
			for(unsigned int c = 0;
				c<(sizeof(TerrainDefs[type].prods)/sizeof(Product));
				c++) {
				int resource = TerrainDefs[type].prods[c].product;
				if(i == resource) {
					canProduceHere = 1;
					break;
				}
			}
		} 
		// Non-raw goods
		else {
			canProduceHere = 1;
			for(unsigned int c = 0;
				c<(sizeof(ItemDefs[i].pInput)/sizeof(Materials));
				c++) {
				int match = 0;
				int need = ItemDefs[i].pInput[c].item;
				for(unsigned int r=0;
					r<(sizeof(TerrainDefs[type].prods)/sizeof(Product));
					r++) {
					if(TerrainDefs[type].prods[r].product == need)
						match = 1;
				}
				if(!match) {
					canProduceHere = 0;
					break;
				}
			}
		}
		MAYBE_UNUSED int canProduce = 0;
		// Check if the locals can produce this item
		if(canProduceHere) canProduce = locals->CanProduce(i);
		int isUseful = 0;
		// Check if the item is useful to the locals
		isUseful = locals->CanUse(i);
		//Normal Items
		if(ItemDefs[ i ].type & IT_NORMAL) {
			
			if (i==I_GRAIN || i==I_LIVESTOCK || i==I_FISH) {
				// Add foodstuffs directly to market	
				int amt = Globals->CITY_MARKET_NORMAL_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
						100;
				} else {
					price = ItemDefs[ i ].baseprice;
				}

				//reduce grain/livestock demand in ocean
				if(i != I_FISH && TerrainDefs[type].similar_type == R_OCEAN) amt /= 10;

				cap = (citymax * 3/4) - 1000;
				if(cap < 0) cap = citymax/2;
				
				Market * m = new Market (M_SELL, i, price, amt, population,
						population+cap, amt, amt*2);
				markets.Add(m);
			} else if (i == I_FOOD) {
				// Add foodstuffs directly to market
				int amt = Globals->CITY_MARKET_NORMAL_AMT;
				int price;
				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					price = (ItemDefs[i].baseprice * (120 + getrandom(80))) /
						100;
				} else {
					price = ItemDefs[ i ].baseprice;
				}

				cap = (citymax / 8);
				if(cap < citymax) cap = (citymax * 3/4);
				Market * m = new Market (M_BUY, i, price, amt, population+cap,
						population+6*cap, amt, amt*5);
				markets.Add(m);
			} else if (ItemDefs[i].pInput[0].item == -1) {   //Arcadia mods
				// Basic resource
				// Add to supply?
				if(canProduceHere) supply[i] = 4;            //forget the canproduce stuff
				// Add to demand?
				else demand[i] = 4;
				
				if(i == I_ROUGHGEM) {                        //specific item mod for Arcadia
				    if(getrandom(2)) demand[i] = 2;
				    else demand[i] = 0;
				}
				
			} else {
				if(canProduceHere) supply[i] = 2;            //forget the canproduce stuff
				else demand[i] = 2;
			}
		} // end Normal Items
		// Advanced Items
		else if((Globals->CITY_MARKET_ADVANCED_AMT)
			&& (ItemDefs[i].type & IT_ADVANCED)) {
			if(isUseful) rare[i] = 4;
			else rare[i] = 4;                      //Arcadia mod
			if(ItemDefs[i].hitchItem > 0) rare[i] = 2;
		}
		// Magic Items
		else if((Globals->CITY_MARKET_MAGIC_AMT)
			&& (ItemDefs[i].type & IT_MAGIC)) {
			if(isUseful) antiques[i] = 4;
				else antiques[i] = 1;
			if(ItemDefs[i].hitchItem > 0) antiques[i] = 2;
		}
	}
	/* Check for advanced item */
	if((Globals->CITY_MARKET_ADVANCED_AMT) && (getrandom(4) == 1)) {
		int ad = 0;
		for(int i=0; i<NITEMS; i++) ad += rare[i];
		ad = getrandom(ad);
		int i;
		int sum = 0;
		for(i=0; i<NITEMS; i++) {
			sum += rare[i];
			if(ad < sum) break;
		}
		if(ad < sum) {
			int amt = Globals->CITY_MARKET_ADVANCED_AMT;
			int price;
			if(Globals->RANDOM_ECONOMY) {
				amt += getrandom(amt);
				price = (ItemDefs[i].baseprice * (100 + getrandom(50))) / 100;
			} else {
				price = ItemDefs[ i ].baseprice;
			}

			cap = (citymax *3/4) - 1000;
			if(cap < citymax/2) cap = citymax / 2;
			offset = citymax / 8;
			if (cap+offset < citymax) {
				Market * m = new Market (M_SELL, i, price, amt/6, population+cap+offset,
					population+citymax, 0, amt);
				markets.Add(m);
			}
		}
	}
	/* Check for magic item */
	if((Globals->CITY_MARKET_MAGIC_AMT) && (getrandom(8) == 1)) {
		int mg = 0;
		for(int i=0; i<NITEMS; i++) mg += antiques[i];
		mg = getrandom(mg);
		int i;
		int sum = 0;
		for(i=0; i<NITEMS; i++) {
			sum += antiques[i];
			if(mg < sum) break;
		}
		if(mg < sum) {
			int amt = Globals->CITY_MARKET_MAGIC_AMT;
			int price;

			if(Globals->RANDOM_ECONOMY) {
				amt += getrandom(amt);
				price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
					100;
			} else {
				price = ItemDefs[ i ].baseprice;
			}
	
			cap = (citymax *3/4) - 1000;
			if(cap < citymax/2) cap = citymax / 2;
			offset = (citymax/20) + ((citymax/5) * 2);
			Market * m = new Market (M_SELL, i, price, amt/6, population+cap,
					population+citymax, 0, amt);
			markets.Add(m);
		}
	}
	
	/* Add demand (normal) items */
	int num = 3;
	int sum = 1;
	while((num > 0) && (sum > 0)) {
		int dm = 0;
		for(int i=0; i<NITEMS; i++) {
            if(demand[i]) dm++;
        }
		dm = getrandom(dm);
		int i;
		sum = 0;
		for(i=0; i<NITEMS; i++) {
			if(demand[i]) sum++;
			if(dm < sum) break;
		}
		if(dm >= sum) continue; //should not occur
		
		int amt = Globals->CITY_MARKET_NORMAL_AMT;
		amt = demand[i] * amt / 4;
		int price;

		if(Globals->RANDOM_ECONOMY) {
			amt += getrandom(amt);
			price = (ItemDefs[i].baseprice *
				(100 + getrandom(50))) / 100;
		} else {
			price = ItemDefs[i].baseprice;
		}
							
		cap = (citymax/4);
		offset = - (citymax/20) + ((5-num) * citymax * 3/40);
		Market * m = new Market (M_SELL, i, price, amt/6,
			population+cap+offset, population+citymax, 0, amt);
		markets.Add(m);
		demand[i] = 0;
		num--;
	}
	
	/* Add supply (normal) items */
	num = 1;
	sum = 1;
	while((num > 0) && (sum > 0)) {
		int su = 0;
		for(int i=0; i<NITEMS; i++) su += supply[i];
		su = getrandom(su);
		int i;
		sum = 0;
		for(i=0; i<NITEMS; i++) {
			sum += supply[i];
			if(su < sum) break;
		}
		if(su >= sum) continue;

		int amt = Globals->CITY_MARKET_NORMAL_AMT;
		amt = supply[i] * amt / 4;
		if(i == I_WOOD || i == I_IRON) amt /= 2;  //Arcadia mod
		else if(i == I_STONE || i == I_HERBS) amt = (amt*3)/2;  //ie half iron/wood, normal fur/horse, increased stone/herbs.
		int price;

		if(Globals->RANDOM_ECONOMY) {
			amt += getrandom(amt);
			price = (ItemDefs[i].baseprice *
				(150 + getrandom(50))) / 100;
		} else {
			price = ItemDefs[ i ].baseprice;
		}
							
		cap = (citymax/4);
		offset = ((3-num) * citymax * 3 / 40);
		if (supply[i] < 4) offset += citymax / 20;
		Market * m = new Market (M_BUY, i, price, 0,
			population+cap+offset, population+citymax,
			0, amt);
		markets.Add(m);
		supply[i] = 0;
		if (ItemDefs[i].pInput[0].item == -1) num--;	//Arcadia mod
	}
	/* Set up the trade items */
/*	int buy1 = getrandom(numtrade);
	int buy2 = getrandom(numtrade);
	int sell1 = getrandom(numtrade);
	int sell2 = getrandom(numtrade);
*/	
	int buy1 = GetArcadianTrade(numtrade,0);
	int buy2 = GetArcadianTrade(numtrade,0);
	int sell1 = GetArcadianTrade(numtrade,1);
	int sell2 = GetArcadianTrade(numtrade,1);
	int tradebuy = 0;
	int tradesell = 0;
	offset = 0;
	cap = 0;

/*	while (buy1 == buy2) buy2 = getrandom(numtrade);
	while (sell1 == buy1 || sell1 == buy2) sell1 = getrandom(numtrade);
	while (sell2 == sell1 || sell2 == buy2 || sell2 == buy1)
		sell2 = getrandom(numtrade);
*/

	while (buy1 == buy2) buy2 = GetArcadianTrade(numtrade,0);
	while (sell1 == buy1 || sell1 == buy2) sell1 = GetArcadianTrade(numtrade,1);
	while (sell2 == sell1 || sell2 == buy2 || sell2 == buy1) sell2 = GetArcadianTrade(numtrade,1);

	for (int i=0; i<NITEMS; i++) {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(ItemDefs[i].flags & ItemType::NOMARKET) continue;

		if(ItemDefs[ i ].type & IT_TRADE) {
			int addbuy = 0;
			int addsell = 0;

			if (buy1 == 0 || buy2 == 0) {
				addbuy = 1;
			}
			buy1--;
			buy2--;

			if(sell1 == 0 || sell2 == 0) {
				addsell = 1;
			}
			sell1--;
			sell2--;

			if(addbuy) {
				int amt = Globals->CITY_MARKET_TRADE_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					if(Globals->MORE_PROFITABLE_TRADE_GOODS) {
						price=(ItemDefs[i].baseprice*(400+getrandom(100)))/100;   //BS Arcadia increase from 250 to 350.
					} else {
						price=(ItemDefs[i].baseprice*(150+getrandom(50)))/100;
					}
				} else {
					price = ItemDefs[ i ].baseprice;
				}
				
//				cap = (citymax/2);        //2000  
                cap = 1200*Globals->POP_LEVEL;
				offset = tradesell++ *(citymax/9);   // 0,445  -  1200-1645
				if(cap + offset < citymax) {
					Market * m = new Market (M_SELL, i, price, amt/5, cap+population+offset,
						citymax+population+offset, 0, amt);     //BS offset added into maxpop
					markets.Add(m);
				}
			}
//60-90 (90-145) :  240-300 (360-450)    buy prices : sell prices
			if(addsell) {
				int amt = Globals->CITY_MARKET_TRADE_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					if(Globals->MORE_PROFITABLE_TRADE_GOODS) {
						price=(ItemDefs[i].baseprice*(100+getrandom(50)))/100;     //BS Arcadia decrease from 90 to 50
					} else {
						price=(ItemDefs[i].baseprice*(100+getrandom(50)))/100;
					}
				} else {
					price = ItemDefs[ i ].baseprice;
				}

//				cap = (citymax/2);          //2000
                cap = 1250*Globals->POP_LEVEL;
				offset = tradebuy++ * (citymax/12);      //0 or 333  -  1250-1583
				if(cap+offset < citymax) {
					Market * m = new Market (M_BUY, i, price, amt/6, cap+population+offset,
						citymax+population+offset, 0, amt);          //   BS : +offset added into max
					markets.Add(m);
				}
			}
		}
	}
}
/*
int ARegion::GetArcadianTradeAveraged(int numtrade, int producing)
{
    int sales[NITEMS];
    int buys[NITEMS];

    for(int i=0; i<NITEMS; i++) {
        if(ItemDefs[i].type & IT_TRADE) {
            sales[i] = 0;
            buys[i] = 0;
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
		        if(!(ItemDefs[m->type].type & IT_TRADE)) continue;
	            if(m->type == M_SELL) sales[m->item]++;
	            else buys[m->item]++;
	            total++;
		    }
	    }
    }
    
    total /= 2;
    int tradebuys[numtrade];
    int tradesales[numtrade];
    int j = 0;
    for(int i=0; i<numtrade; i++) {
        while(!(ItemDefs[j].type & IT_TRADE) && j < NITEMS) {
            j++;
        }
        if(j<NITEMS) {
            tradebuys[i] = buys[j];
            tradesales[i] = sales[j];
        } else {
            tradebuys[i] = 0;
            tradesales[i] = 0;
        }
    }
    
    int selection = -1;
    int isok = 0;
    while(isok == 0) {
        selection = GetArcadianTrade(numtrade, producing);
        int chance = tradesales[selection] - (total/numtrade);
        if(producing) chance = tradebuys[selection] - (total/numtrade);
        if(chance < 1) chance = 1;
        if(!getrandom(chance)) isok = 1;
    }
    return selection;
}
*/
int ARegion::GetArcadianTrade(int numtrade, int producing)
{
    if(numtrade != 12) {
        Awrite(AString("Wrong number of trade items: ") + numtrade);
        return getrandom(numtrade);
    }
    if(zloc != 1) return getrandom(numtrade);
    
    ManType *mt = FindRace(ItemDefs[race].abr);
    if(!mt) return getrandom(numtrade);

    if(producing) {
        switch(mt->ethnicity) {
            case RA_HUMAN:
                return getrandom(4); //wine, wool, orchids, caviar
            case RA_ELF:
                return 4+getrandom(4); //silk, perfume, chocolate, cashmere
            case RA_DWARF:
                return 8+getrandom(4); //jewelry, figurines, dye, truffles
            default:
                return getrandom(numtrade);
        }
    } else {
        switch(mt->ethnicity) {
            case RA_HUMAN:
                return 4+getrandom(8);
            case RA_ELF:
                if(getrandom(2)) return getrandom(4);
                else return 8+getrandom(4);
            case RA_DWARF:
                return getrandom(8);
            default:
                return getrandom(numtrade);
        }
    }
    return getrandom(numtrade); //should never occur
}

void ARegion::SetupProds()
{
	Production *p = NULL;
	TerrainType *typer = &(TerrainDefs[type]);

	if(Globals->FOOD_ITEMS_EXIST) {
		if (typer->economy) {
			// Foodchoice = 0 or 1 if inland, 0, 1, or 2 if coastal
			int foodchoice = getrandom(2 +
					(Globals->COASTAL_FISH && IsCoastal()));
			switch (foodchoice) {
				case 0:
					if (!(ItemDefs[I_GRAIN].flags & ItemType::DISABLED))
						p = new Production(I_GRAIN, typer->economy);
					break;
				case 1:
					if (!(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED))
						p = new Production(I_LIVESTOCK, typer->economy);
					break;
				case 2:
					if (!(ItemDefs[I_FISH].flags & ItemType::DISABLED))
						p = new Production(I_FISH, typer->economy);
					break;
			}
			products.Add(p);
		}
	}

	for(unsigned int c= 0; c < (sizeof(typer->prods)/sizeof(Product)); c++) {
		int item = typer->prods[c].product;
		int chance = typer->prods[c].chance;
		int amt = typer->prods[c].amount;
		if(item != -1) {
			if(!(ItemDefs[item].flags & ItemType::DISABLED) &&
					(getrandom(100) < chance)) {
				p = new Production(item, amt);
				products.Add(p);
			}
		}
	}
}

void ARegion::AddTown()
{
	town = new TownInfo;
	town->name = new AString(AGetNameString(AGetName(1)));
	// PLAYER_ECONOMY
	// basepop is the town's habitat
	// set pop to a base level at first
	if(Globals->PLAYER_ECONOMY) {
		town->basepop = TownHabitat();
		town->pop = town->basepop * 2 / 3;
		town->activity = 0;
		town->growth = 0;
		town->mortality = 0;
		SetupCityMarket();
		return;
	}

	if(Globals->RANDOM_ECONOMY) {
		int popch = 2000*Globals->POP_LEVEL;
		if(Globals->LESS_ARCTIC_TOWNS) {
			int dnorth = GetPoleDistance(D_NORTH);
			int dsouth = GetPoleDistance(D_SOUTH);
			int dist = dnorth;
			//
			// On small worlds or the underworld levels, a city could be
			// within 9 of both poles.. chose the one it's closest to
			if (dsouth < dist) dist = dsouth;
			if (dist < 9)
				popch = popch - (9 - dist) * ((9 - dist) + 10) * 15;
		}
		town->pop = 500*Globals->POP_LEVEL+getrandom(popch);
	} else {
		town->pop = 500*Globals->POP_LEVEL;
	}
	town->basepop = town->pop;
	town->activity = 0;

	SetupCityMarket();
}

void ARegion::AddEditTown(AString *townname)
{
#define NUMTOWNNAMES 1000
/* This should be a direct copy of the above except for the naming*/
    if(!townname) townname = new AString(AGetNameString(getrandom(NUMTOWNNAMES)));
    if(town) delete town;
	town = new TownInfo;
	town->name = townname;
	// PLAYER_ECONOMY
	// basepop is the town's habitat
	// set pop to a base level at first
	if(Globals->PLAYER_ECONOMY) {
		town->basepop = TownHabitat();
		town->pop = town->basepop * 2 / 3;
		town->activity = 0;
		town->growth = 0;
		town->mortality = 0;
		SetupCityMarket();
		return;
	}

	if(Globals->RANDOM_ECONOMY) {
		int popch = 2000*Globals->POP_LEVEL;
		if(Globals->LESS_ARCTIC_TOWNS) {
			int dnorth = GetPoleDistance(D_NORTH);
			int dsouth = GetPoleDistance(D_SOUTH);
			int dist = dnorth;
			//
			// On small worlds or the underworld levels, a city could be
			// within 9 of both poles.. chose the one it's closest to
			if (dsouth < dist) dist = dsouth;
			if (dist < 9)
				popch = popch - (9 - dist) * ((9 - dist) + 10) * 15;
		}
		town->pop = 500*Globals->POP_LEVEL+getrandom(popch);
	} else {
		town->pop = 500*Globals->POP_LEVEL;
	}
	town->basepop = town->pop;
	town->activity = 0;
	SetupCityMarket();
}

void ARegion::UpdateEditRegion()
{
    // redo markets and entertainment/tax income for extra people.

	money = Population() * (Wages() - (Globals->MAINTENANCE_COST/2)) / (2*Globals->POP_LEVEL);  //BS Edit
	if (money < 0) money = 0;
	// Setup working
	Production *p = products.GetProd(I_SILVER, -1);
	if(p) {
    	if (IsStartingCity()) {
    		// Higher wages in the entry cities.
    		p->amount = Wages() * Population();
    	} else {
    		p->amount = (Wages() * Population()) / Globals->WORK_FRACTION;
    	}
    	p->productivity = Wages();
	}
	// Entertainment.
	p = products.GetProd(I_SILVER, S_ENTERTAINMENT);
	if(p) {
    	p->baseamount = money / Globals->ENTERTAIN_FRACTION;
    	p->amount = p->baseamount;
	}
	markets.PostTurn(Population(), Wages(), race);
	
	//Replace man selling
	forlist(&markets) {
	    Market *m = (Market *) elem;
	    if(ItemDefs[m->item].type & IT_MAN) {
	        markets.Remove(m);
	        delete m;
        }
    }

	float ratio = ItemDefs[race].baseprice / (float)Globals->BASE_MAN_COST;
	int wage = Wages();
	if(wage < 10) wage = 10;
    Market *m = new Market(M_BUY, race, (int)(wage*4*ratio),
							Population()/(5*Globals->POP_LEVEL), 0, 10000, 0, 2000);
	markets.Add(m);

	if(Globals->LEADERS_EXIST) {
		ratio = ItemDefs[I_LEADERS].baseprice / (float)Globals->BASE_MAN_COST;
		m = new Market(M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
						Population()/(25*Globals->POP_LEVEL), 0, 10000, 0, 400);
		markets.Add(m);
	}
}

void ARegion::SetupEditRegion(int canmakecity)
{
//Direct copy of SetupPop() except calls AddEditTown() instead of AddTown()
	TerrainType *typer = &(TerrainDefs[type]);
	habitat = typer->pop+1;
	if (habitat < 100) habitat = 100;
	habitat *= Globals->POP_LEVEL;

	int pop = typer->pop*Globals->POP_LEVEL;
	int mw = typer->wages;
	
	// fix economy when MAINTENANCE_COST has been adjusted
	mw += Globals->MAINTENANCE_COST - 10;
	if (mw < 0) mw = 0;

	if (pop == 0) {
		population = 0;
		basepopulation = 0;
		wages = 0;
		maxwages = 0;
		money = 0;		
		if(typer->similar_type == R_OCEAN) race = I_MERMEN;

		/*
		if(Globals->PLAYER_ECONOMY) {
			// All regions need silver production
			// even if no income can be gained
			Production *p = new Production;
			p->itemtype = I_SILVER;
			p->amount = 0;
			p->skill = -1;
			p->productivity = 0;
			products.Add(p);

			p = new Production;
			p->itemtype = I_SILVER;
			p->amount = 0;
			p->skill = S_ENTERTAINMENT;
			p->productivity = Globals->ENTERTAIN_INCOME;
			products.Add(p);
		}
		*/

		return;
	}

	// Only select race here if it hasn't been set during Race Growth
	// in the World Creation process.
	if ((race == -1) || (!Globals->GROW_RACES)) {
		int noncoastalraces = sizeof(typer->races)/sizeof(int);
		int allraces =
			noncoastalraces + sizeof(typer->coastal_races)/sizeof(int);

		race = -1;
		while (race == -1 || (ItemDefs[race].flags & ItemType::DISABLED)) {
			int n = getrandom(IsCoastal() ? allraces : noncoastalraces);
			if(n > noncoastalraces-1) {
				race = typer->coastal_races[n-noncoastalraces-1];
			} else
				race = typer->races[n];
		}
	}

	if(Globals->PLAYER_ECONOMY) {
		if(Globals->RANDOM_ECONOMY)	habitat = habitat * 2/3 + getrandom(habitat/3);
		ManType *mt = FindRace(ItemDefs[race].abr);
		if (mt->terrain == typer->similar_type) {
			habitat = (habitat * 9)/8;
		}
		if (!IsNativeRace(race)) {
			habitat = (habitat * 4)/5;
		}
		basepopulation = habitat;
		// hmm... somewhere not too far off equilibrium pop
		population = habitat * 66 / 100;
	} else {
		if(Globals->RANDOM_ECONOMY) {
			population = (pop + getrandom(pop)) / 2;
		} else {
			population = pop;
		}
		basepopulation = population;
	}

	// Setup wages
	if(Globals->PLAYER_ECONOMY) {
		int level = 1;
		development = 1;
		int prev = 0;
		while (level < mw) {
			development++;
			prev++;
			if(prev > level) {
				level++;
				prev = 0;
			}
		}
		if(Globals->RANDOM_ECONOMY) {
			development += getrandom(36);
		}
	} else {
		if(Globals->RANDOM_ECONOMY) {
			mw += getrandom(3);
		}
		wages = mw;
		maxwages = mw;
	}

	if(Globals->TOWNS_EXIST) {
		int adjacent = 0;
		int prob = Globals->TOWN_PROBABILITY;
		if (prob < 1) prob = 100;
		int townch = (int) 80000 / prob;
		if (Globals->TOWNS_NOT_ADJACENT) {
			for (int d = 0; d < NDIRS; d++) {
				ARegion *newregion = neighbors[d];
				if ((newregion) &&  (newregion->town)) adjacent++;
			}
		}
		if(Globals->LESS_ARCTIC_TOWNS) {
			int dnorth = GetPoleDistance(D_NORTH);
			int dsouth = GetPoleDistance(D_SOUTH);
			if (dnorth < 9)
				townch = townch + 25 * (9 - dnorth) *
					(9 - dnorth) * Globals->LESS_ARCTIC_TOWNS;
			if (dsouth < 9)
				townch = townch + 25 * (9 - dsouth) *
					(9 - dsouth) * Globals->LESS_ARCTIC_TOWNS;
		}
		int spread = Globals->TOWN_SPREAD;
		if(spread > 100) spread = 100;
		int townprob = (TerrainDefs[type].economy * 4 * (100 - spread) +
			100 * spread) / 100;
		if (adjacent > 0) townprob = townprob * (100 - Globals->TOWNS_NOT_ADJACENT) / 100;
		if (getrandom(townch) < townprob) AddEditTown();
	}

	Production *p = new Production;
	p->itemtype = I_SILVER;
	money = Population() * (Wages() - (Globals->MAINTENANCE_COST/2)) / (2*Globals->POP_LEVEL);  //BS Edit

	if(Globals->PLAYER_ECONOMY) {
		WagesFromDevelopment();
		maxwages = wages;
		money = Population() * (wages - 100) / 10;
	}	
	
	if (money < 0) money = 0;
	p->amount = money / Globals->WORK_FRACTION;
	p->skill = -1;
	if (Globals->PLAYER_ECONOMY) p->productivity = wages;
		else p->productivity = Wages();
	products.Add(p);

	//
	// Setup entertainment
	//
	p = new Production;
	p->itemtype = I_SILVER;
	p->amount = money / Globals->ENTERTAIN_FRACTION;
	p->skill = S_ENTERTAINMENT;
	p->productivity = Globals->ENTERTAIN_INCOME;
	products.Add(p);

	float ratio = ItemDefs[race].baseprice / (float)Globals->BASE_MAN_COST;
	// Setup Recruiting
	Market *m = new Market(M_BUY, race, (int)(Wages()*4*ratio),
							Population()/(5*Globals->POP_LEVEL), 0, 10000, 0, 2000);
	markets.Add(m);

	if(Globals->LEADERS_EXIST) {
		ratio = ItemDefs[I_LEADERS].baseprice / (float)Globals->BASE_MAN_COST;
		m = new Market(M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
						Population()/(25*Globals->POP_LEVEL), 0, 10000, 0, 400);
		markets.Add(m);
	}
}







// Used at start to set initial town's
// development level
void ARegion::CheckTownIncrease()
{
	if(!town) return;
	
	//if(town) return;
	
	if(town->pop > 3000*Globals->POP_LEVEL) {
		development = development + ((getrandom(Globals->TOWN_DEVELOPMENT) + Globals->TOWN_DEVELOPMENT) / 10);
		return;
	}
	
	if(getrandom(200) > Globals->TOWN_DEVELOPMENT) return;
	
	int a = 5;
	if(Globals->TOWN_DEVELOPMENT < 10) a = Globals->TOWN_DEVELOPMENT / 2;
	int b = 20;
	if(Globals->TOWN_DEVELOPMENT < 60) b = Globals->TOWN_DEVELOPMENT / 3;
	development = development + a + getrandom(b);
	/*
	Awrite(AString("> increased city in ") + TerrainDefs[type].name + " in " +
		*name + " (" + xloc + ", " + yloc + ") to dev = " + development);
	*/
	TownHabitat();
	town->pop = town->basepop * 2 / 3;
}

int ARegion::TownHabitat()
{
	// Effect of existing buildings
	int farm = 0;
	int inn = 0;
	int bank = 0;
	int temple = 0;
	int caravan = 0;
	int fort = 0;
	forlist(&objects) {
		Object *obj = (Object *) elem;
		if(ObjectDefs[obj->type].protect > fort) fort = ObjectDefs[obj->type].protect;
		if(ItemDefs[ObjectDefs[obj->type].productionAided].flags & IT_FOOD) farm++;
		if(ObjectDefs[obj->type].productionAided == I_SILVER) inn++;
		if(ObjectDefs[obj->type].productionAided == I_HERBS) temple++;
		if(ObjectDefs[obj->type].name == "Bank") bank++;
		if((ObjectDefs[obj->type].flags & ObjectType::TRANSPORT)
			&& (ItemDefs[ObjectDefs[obj->type].productionAided].flags & IT_MOUNT)) caravan++;
	}
	int hab = 2;
	int step = 0;
	for(int i=0; i<5; i++) {
		if(fort > step) hab++;
		step *= 5;
		if(step == 0) step = 10;
	}
	int build = 0;
	if(farm) build++;
	if(inn) build++;
	if(temple) build++;
	if(bank) build++;
	if(caravan) build++;
	if(build > 2) build = 2;
	
	hab = (build++ * build + 1) * hab * hab + basepopulation / 4 + 50;
	
	// Lake Effect
	if(LakeEffect()) hab += 100;
	
	// Effect of Development:
	int totalhab = hab + TownDevelopment() / 100 * (basepopulation + 2800 + hab);
	
	return totalhab;
}

// Use this to determine development including
// bonuses for roads
int ARegion::Development()
{
	int dv = development;
	
	// Road bonus
	int roads = 0;
	for(int i=0; i<NDIRS; i++) if(HasExitRoad(i)) roads++;
	int dbonus = 0;
	if(roads > 0) {
#if 0
		Awrite("_____________________________________________________");
		Awrite("Checking Roads... for region:");
		Awrite(AString(" ") + *name + "(" + xloc + ", " + yloc + ")");
#endif
		dbonus = CountRoadConnectedTowns(8);
	}
	// Maximum bonus of 40 to development for roads
	int bonus = 5;
	int leveloff = 1;
	int totalb = 0;
	while((dbonus > 0) && (totalb < 41)) {
		dbonus--;
		if(leveloff > 4) if (bonus > 1) bonus--;
		leveloff++;
		totalb += bonus;
	}

	return dv;
}

// Measure of the economic development of a town
// on a scale of 0-100. Used for town growth/hab/limits
// and also for raising development through markets.
// Do NOT take roads into account as they are a bonus and
// considered outside of these limits.
int ARegion::TownDevelopment()
{
	int level = 1;
	int basedev = 1;
	int prev = 0;
	while (level <= TerrainDefs[type].wages) {
		prev++;
		basedev++;
		if(prev > level) {
			level++;
			prev = 0;
		}
	}
	
	int df = 100 * (development - basedev) / 90;
	if(df < 0) df = 0;
	if(df > 100) df = 100;
	
	return df;
} 

void ARegion::UpdateTown()
{
	if(!(Globals->VARIABLE_ECONOMY)) {
		//
		// Nothing to do here.
		//
		return;
	}
	
	//
	// Check if we were a starting city and got taken over
	//
	if(IsStartingCity() && !HasCityGuard() && !Globals->SAFE_START_CITIES) {
		// Make sure we haven't already been modified.
		int done = 1;
		forlist(&markets) {
			Market *m = (Market *)elem;
			if(m->minamt == -1) {
				done = 0;
				break;
			}
		}
		if(!done) {
			markets.DeleteAll();
			SetupCityMarket();
			float ratio = ItemDefs[race].baseprice /
				(float)Globals->BASE_MAN_COST;
			// Setup Recruiting
			Market *m = new Market(M_BUY, race, (int)(Wages()*4*ratio),
					Population()/(5*Globals->POP_LEVEL), 0, 10000, 0, 2000);
			markets.Add(m);
			if(Globals->LEADERS_EXIST) {
				ratio = ItemDefs[I_LEADERS].baseprice /
					(float)Globals->BASE_MAN_COST;
				m = new Market(M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
						Population()/(25*Globals->POP_LEVEL), 0, 10000, 0, 400);
				markets.Add(m);
			}
		}
	}

	if(Globals->PLAYER_ECONOMY) {
	
		town->basepop = TownHabitat();
		
		// *Town Pop Growth*
		int delay = Globals->DELAY_GROWTH;
		if (delay < 2) delay = 2;
		int lastgrowth = town->growth;
		if (delay > 2)
			for (int i=1; i<delay-1; i++) town->growth += lastgrowth;
		// growth based on available space
		int space = town->basepop - town->pop;
		if(space < 0) space = 0;
		int sgrow = 0;
		if((3 * space/(town->basepop+1)) < 2) sgrow = space / 10; 
			else sgrow = (space / (town->basepop + 1) * 15) * space;
		// growth based on population and wages
		int curg = ((Globals->POP_GROWTH / 100) * (Wages()-5) *
			 (((town->basepop * (1+Wages()/8)) - 2 * town->pop) / (40 * delay)));
		if (sgrow > 0) sgrow = 0;
		if (curg > 0) curg = 0;
		town->growth += sgrow + curg;
		town->growth = town->growth / delay;
		if (town->pop < 1) town->growth = 0;
		
		// *Town Pop Mortality*
		delay = Globals->DELAY_MORTALITY;
		if (delay < 2) delay = 2;
		int lastmort = town->mortality;
		int curm = 0;
		int starve = 0;
		if (delay > 2)
			for (int i=1; i<delay-1; i++) curm += lastmort;
		
		// mortality based on starvation:
		
		// mortality based on crowding:
		int crowd = 3 * town->pop - 2 * town->basepop;
		if ((3 * crowd / town->basepop) < 2) crowd = 0;
		if (crowd > 0) {
			float cfactor = 2 / (crowd + 1);
			cfactor = cfactor * cfactor / 4;
			crowd = (int) ((town->pop / 10) * (1 - cfactor));
		}

		if(crowd > 0) town->mortality += crowd;
		if(starve > 0) town->mortality += starve;
		town->mortality = town->mortality / delay;
		if ((town->mortality < 0) || (town->pop < 1)) town->mortality = 0;
		
		town->pop += town->growth - town->mortality;
		if(town->pop < 0) town->pop = 0;
		/*
		if(crowd > 0) migration += crowd / 36;
		if(starve > 0) migration += starve / 36;
		*/
#if 0
		if((development > 190) && (!IsStartingCity())) {
		Awrite(AString("===== Town(") + town->TownType() + ") in " + *name 
			+ " in (" + xloc + ", " + yloc + ") =====");
		Awrite(AString(" growth:      ") + town->growth);
		Awrite(AString(" mortality:   ") + town->mortality);
		Awrite(AString(" town pop:    ") + town->pop);
		Awrite(AString(" town hab:    ") + town->basepop);
		Awrite(AString(" development: ") + development);
		int td = TownDevelopment();
		Awrite(AString(" towndevel.:  ") + td);
		Awrite(AString(" migration:   ") + migration);
		Awrite("");
		}
#endif
		return;
	}
	
	
	//
	// Don't do pop stuff for AC Exit.
	//
	//BS : This section modified from standard (for Arcadia)
	if (!IsStartingCity()) {
		// First, get the target population
		int amt = 0;
		int tot = 0;
		forlist(&markets) {
			Market *m = (Market *) elem;
			if (Population() > m->minpop) {
				if (ItemDefs[m->item].type & IT_TRADE) {
					amt += 3 * m->activity;
					tot += 3 * m->amount;     //BS mod (i) all trade items, (ii) use amount not maxamt here & below
				} else {
					if (m->type == M_SELL) {
						// Only food items except fish are mandatory
						// for town growth - other items can
						// be used in replacement
						if (ItemDefs[m->item].type & IT_FOOD) {
							amt += 2 * m->activity;
						} else amt += m->activity;
						if (ItemDefs[m->item].type & IT_FOOD)	tot += 2 * m->amount;  //so town growth is set by size of fish,livestock&grain markets (plus trade items) only ...  fish added by BS.
					} else {
					    //BS mod - buying town products should increase growth also
					    if (!(ItemDefs[m->item].type & IT_MAN)) {
							amt +=  m->activity;
						}
					}
				}
			}
		}

		if (amt > tot) amt = tot;

		int score;
		if (tot) {
			score = (1000 * amt) / tot;
		} else {
			score = 0;
		}

		int maxgrowth = 150*Globals->POP_LEVEL + (town->pop * ((3*Globals->CITY_POP*Globals->POP_LEVEL)/2 - town->pop)) / (60000*Globals->POP_LEVEL*Globals->POP_LEVEL);
		//equals 150 at pop=0, max 300 at pop = 3000, 233 at pop = 1000 or 5000.
		//this growth cannot be exceeded, but it should be fairly easy to get half this growth for small towns.

		int basescore = town->pop/(6*Globals->POP_LEVEL) - 80; // 3 at pop = 500, 320=32% at pop = 2400. Equals trade required for steady pop
		if(basescore < 1) basescore = 1;

    	int change = 0;
    	if(score < basescore) {
    	    change = ((score-basescore)*town->pop)/(basescore*20); //linear from -5% to 0 for basescore = 0 to score.
    	} else if (score > basescore) {  //approximately exponential, requiring score = 2*basescore to get growth = 63% of maxgrowth.
    	    float exponent = 1;
    	    while( (score-- > basescore) && (exponent*maxgrowth > 1) ) {
    	        exponent *= 1 - 1/((float) basescore);
    	    }
    	    change = maxgrowth - (int (maxgrowth*exponent));
    	}
		
		town->pop += change;

		// Check base population
		if (town->pop < town->basepop) {
			town->pop = town->basepop;
		}
		if ((town->pop * 4) / 5 > town->basepop) {
			town->basepop = (town->pop * 4) / 5;
		}
	}
}

void ARegion::Migrate()
{
	for(int i=0; i<NDIRS; i++) {
		ARegion *nbor = neighbors[i];
		if((nbor) && (nbor->migration > 0)) {
			int cv = 100;
			if(nbor->race != race) cv = 50;
			// Roads?
			if(HasExitRoad(i) && HasConnectingRoad(i)) {
				cv += 25;
				for(int d=0; d<NDIRS; d++) {
					if((i == GetRealDirComp(d)) || (!nbor->HasExitRoad(d))) continue;
					ARegion *ntwo = nbor->neighbors[d];
					if((nbor) && (nbor->HasConnectingRoad(d))) {
						int c2 = 100;
						if(ntwo->race != race) c2 = 50;
						population += ntwo->migration * c2 / 100;
					}
				}
			}
			population += nbor->migration * cv / 100;
		}
	}
}

void ARegion::PostTurn(ARegionList *pRegs)
{
	// Rules for PLAYER-RUN ECONOMY
	if (Globals->PLAYER_ECONOMY) {
	
		// Decay desolate areas
		if (Population() < (habitat / 100))
			if (development > 0) development--;
		// Check for Development increase
		int fooddemand = 1;
		int foodavailable = 1;
		forlist(&markets) {
			Market *m = (Market *) elem;
			if (m->amount < 1) continue;
			int npcprod = 0;
			forlist(&products) {
				Production *re = (Production *) elem;
				if (re->itemtype == m->item)
					npcprod = re->amount - re->activity;
			}
			if (ItemDefs[m->item].type & IT_MAN) {
				// reduce population
				population -= m->activity;
			} else if (m->type == M_SELL) {
				if (ItemDefs[m->item].type & IT_FOOD) {
					fooddemand += m->amount;
					foodavailable += m->activity + npcprod;
				}
				int supply = npcprod + m->activity;
				if ((m->amount <= supply * 4) &&
						(getrandom(m->amount) < supply)) {
					development++;
				}
			} else if (m->type == M_BUY) {
				int supply = m->activity;
				if ((m->amount <= supply * 3) &&
						(getrandom(3 * m->amount) < (supply * 2))) {
					development++;
				}
			}
			// TODO: a function to update markets (incl. npcprod)
		}

		WagesFromDevelopment();

		// *Population Growth*
		int delay = Globals->DELAY_GROWTH;
		if (delay < 2) delay = 2;
		int lastgrowth = growth;
		if (delay > 2)
			for (int i=1; i<delay-1; i++) growth += lastgrowth;
		// growth based on available space
		int space = habitat - population;
		if(space < 0) space = 0;
		int sgrow = 0;
		if((3*space/(habitat+1)) < 2) sgrow = space / 10; 
			else sgrow = (space / (habitat + 1) * 15) * space;			
		// growth based on wages
		int curg = ((Globals->POP_GROWTH / 100) * (Wages()-5) *
			 (habitat - 2 * population) / 200);
		if(curg < 0) curg = 0;
		curg = curg * (2 * habitat - 3 * population) / (habitat + 1);
		// growth based on existing population
		int pgrow = population;
		if(pgrow > habitat) pgrow = habitat;
		pgrow *= Globals->POP_GROWTH / 20000;
		if(sgrow < 0) sgrow = 0;
		if(curg < 0) curg = 0;
		if(population > 0) growth += curg + sgrow + pgrow;
		growth = growth / delay;

		// *Mortality*
		delay = Globals->DELAY_MORTALITY;
		if (delay < 2) delay = 2;
		int lastmort = mortality;
		int curm = 0;
		int starve = 0;
		if (delay > 2)
			for (int i=1; i<delay-1; i++) curm += lastmort;
		// mortality based on starvation:
		int totalhab = habitat;
		if(town) totalhab += town->basepop;
		/*
		if ((habitat > 0) && (fooddemand > 0))
			starve = (population * population *
					(fooddemand - foodavailable)) /
				(fooddemand * habitat * 4);
		*/
				
		// mortality based on crowding:
		int crowd = 3 * population - 2 * habitat;
		if ((3 * crowd / habitat) < 2) crowd = 0;
		if (crowd > 0) {
			float cfactor = 2 / (crowd + 1);
			cfactor = cfactor * cfactor / 4;
			crowd = (int) ((population / 10) * (1 - cfactor));
		}
		if(population > 0) mortality += crowd + starve;
		mortality = mortality / delay;
		if(mortality < 0) mortality = 0;
		migration = (crowd + starve) / 18;

		// Update population
		AdjustPop(growth - mortality);
		
		// Update town
		if(town) UpdateTown();

		// Check decay
		if(Globals->DECAY) {
			DoDecayCheck(pRegs);
		}

		// Set tax base
		money = (wages - 100) * Population() / 10;
		if (money < 0) money = 0;

		if (type != R_NEXUS) {
			// Setup working
			Production * p = products.GetProd(I_SILVER,-1);
			int hack = 0;
			if (IsStartingCity()) {
				// Higher wages in the entry cities.
				hack = wages * Population() / 10;
			} else {
				// (Globals->WORK_FRACTION); //This is PLAYER ECONOMY section
				hack = (wages * Population()) / 50;
			}
			// TODO: Silver available for work and entertainment
			// still causes a segmentation fault!!!
			if (p) {
				p->amount = hack;
				p->productivity = wages;
			}

			// Entertainment.
			p = products.GetProd(I_SILVER,S_ENTERTAINMENT);
			if (p) p->baseamount = money / Globals->ENTERTAIN_FRACTION;
#if 0
			//uncomment for economy info output used for debugging
			Awrite(AString("===== ") +
				TerrainDefs[type].name + " in " + *name
				+ " (" + xloc + ", " + yloc + ") =====");
			Awrite(AString("population:  ") + population + " " +
					ItemDefs[race].names);
			Awrite(AString("wages:	   ") +
					(wages/10) + "  (tax base: "+ money + ")");
			Awrite(AString("development: ") + development);
			Awrite(AString("habitat:	 ") +
					(habitat - population) + " / " + habitat);
			Awrite(AString("growth:	  ") + growth +
					"  (this month: " + curg + ")");
			Awrite(AString("mortality:   ") + mortality + "  (" + starve +
					" starve, " + crowd + " die of crowding)");
			Awrite(AString("food:		") + (foodavailable-1) + " / " +
					(fooddemand-1));
			Awrite("");
#endif

			markets.PostTurn(Population(),Wages(), race);
		}
		return;
	}

	// Standard economy
	//
	// First update population based on production
	//
	int activity = 0;
	int amount = 0;
	if (basepopulation || Population()) {
		forlist(&products) {
			Production *p = (Production *) elem;
			if (ItemDefs[p->itemtype].type & IT_NORMAL &&
				p->itemtype != I_SILVER) {
				activity += p->activity;
				amount += p->amount;
			}
		}
		int tarpop = basepopulation;
		if (amount) tarpop += (basepopulation * activity) / (2 * amount);
		int diff = tarpop - population;

		if(Globals->VARIABLE_ECONOMY) {
			population = population + diff / 5;
		}

		//
		// Now, if there is a town, update its population.
		//
		if (town) {
			UpdateTown();
		}

		// AS
		if(Globals->DECAY) {
			DoDecayCheck(pRegs);
		}

		//
		// Now, reset population based stuff.
		// Recover Wages.
		//
		if (wages < maxwages) wages++;

		//
		// Set money
		//
		money = Population() * (Wages() - (Globals->MAINTENANCE_COST/2)) / (2*Globals->POP_LEVEL);  //BS Edit
		if (money < 0) money = 0;

		//
		// Setup working
		//
		
		Production *p = products.GetProd(I_SILVER, -1);
        if(p) {
    		if (IsStartingCity()) {
    			//
    			// Higher wages in the entry cities.
    			//			
    			p->amount = Wages() * Population() / Globals->POP_LEVEL;
    		} else {
    			p->amount = (Wages() * Population()) / (Globals->WORK_FRACTION * Globals->POP_LEVEL);
    		}
    		p->productivity = Wages();
		}

		//
		// Entertainment.
		//
		p = products.GetProd(I_SILVER, S_ENTERTAINMENT);
		if(p) p->baseamount = money / Globals->ENTERTAIN_FRACTION;
		
		markets.PostTurn(Population(), Wages(), race);
	}

	UpdateProducts();

	//
	// Set these guys to 0.
	//
	//BS: Why? There's no need anymore, and it makes report-writing harder.
	/*
	earthlore = 0;
	clearskies = 0;
*/

	forlist(&objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			u->PostTurn(this);
		}
	}
}
