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

Location *GetUnit(AList *list, int n)
{
	forlist(list) {
		Location *l = (Location *) elem;
		if (l->unit->num == n) return l;
	}
	return 0;
}

ARegionPtr *GetRegion(AList *l, int n)
{
	forlist(l) {
		ARegionPtr *p = (ARegionPtr *) elem;
		if (p->ptr->num == n) return p;
	}
	return 0;
}

Farsight::Farsight()
{
	faction = 0;
	unit = 0;
	level = 0;
	for(int i = 0; i < NDIRS; i++)
		exits_used[i] = 0;
}

Farsight *GetFarsight(AList *l, Faction *fac)
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
	growth = 0;
	mortality = 0;
}

TownInfo::~TownInfo()
{
	if (name) delete name;
}

void TownInfo::Readin(Ainfile *f, ATL_VER &v)
{
	name = f->GetStr();
	pop = f->GetInt();
	basepop = f->GetInt();
	growth = f->GetInt();
	mortality = f->GetInt();
}

void TownInfo::Writeout(Aoutfile *f)
{
	f->PutStr(*name);
	f->PutInt(pop);
	f->PutInt(basepop);
	f->PutInt(growth);
	f->PutInt(mortality);
}

int TownInfo::TownType()
{
	if (pop < 1000) return TOWN_VILLAGE;
	if (pop < 2000) return TOWN_TOWN;
	return TOWN_CITY;
}

ARegion::ARegion()
{
	name = new AString("Region");
	xloc = 0;
	yloc = 0;
	buildingseq = 1;
	gate = 0;
	gatemonth = 0;
	gateopen = 1;
	town = 0;
	development = 0;
	habitat = 0;
	mortality = 0;
	growth = 0;
	migration = 0;
	clearskies = 0;
	earthlore = 0;
	for (int i=0; i<NDIRS; i++)
		neighbors[i] = 0;
}

ARegion::~ARegion()
{
	if (name) delete name;
	if (town) delete town;
	delete roadsto;
}

void ARegion::ZeroNeighbors()
{
	for (int i=0; i<NDIRS; i++) {
		neighbors[i] = 0;
	}
}

void ARegion::SetName(char *c)
{
	if(name) delete name;
	name = new AString(c);
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
	if (p)
		return AString("$") + p->productivity + " (Max: $" + p->amount + ")";
	else
		return AString("$") + 0;
}

void ARegion::SetupPop()
{
	TerrainType *typer = &(TerrainDefs[type]);
	habitat = typer->pop+1;
	if (habitat < 100) habitat = 100;

	int pop = typer->pop;
	int mw = typer->wages;

	if (pop == 0) {
		population = 0;
		basepopulation = 0;
		wages = 0;
		maxwages = 0;
		money = 0;

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

		return;
	}

	// Only select race here if it hasn't been set during Race Growth
	// in the World Creation process.
	if (((Globals->GROW_RACES) && (race == -1)) || (!Globals->GROW_RACES)) {
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

	if(Globals->PLAYER_ECONOMY) {
		WagesFromDevelopment();
		maxwages = wages;
	}

	Production *p = new Production;
	p->itemtype = I_SILVER;
	money = Population() * (Wages() - Globals->MAINTENANCE_COST);
	if (money < 0) money = 0;
	p->amount = money / Globals->WORK_FRACTION;
	p->skill = -1;
	p->productivity = Wages();
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
							Population()/5, 0, 10000, 0, 2000);
	markets.Add(m);

	if(Globals->LEADERS_EXIST) {
		ratio = ItemDefs[I_LEADERS].baseprice / (float)Globals->BASE_MAN_COST;
		m = new Market(M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
						Population()/25, 0, 10000, 0, 400);
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

int ARegion::IsNativeRace(int item)
{
	TerrainType *typer = &(TerrainDefs[type]);
	int coastal = sizeof(typer->coastal_races)/sizeof(int);
	int noncoastal = sizeof(typer->races)/sizeof(int);
	if(IsCoastal()) {
		for(int i=0; i<coastal; i++) {
			if (item == typer->coastal_races[i]) return 1;
		}
	}
	for(int i=0; i<noncoastal; i++) {
		if (item == typer->races[i]) return 1;
	}
	return 0;
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

int ARegion::GetNearestProd(int item)
{
	AList regs, regs2;
	AList *rptr = &regs;
	AList *r2ptr = &regs2;
	AList *temp;
	ARegionPtr *p = new ARegionPtr;
	p->ptr = this;
	regs.Add(p);

	for (int i=0; i<5; i++) {
		forlist(rptr) {
			ARegion *r = ((ARegionPtr *) elem)->ptr;
			AString skname = ItemDefs[item].pSkill;
			int sk = LookupSkill(&skname);
			if (r->products.GetProd(item, sk)) {
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
	for (i=0; i<NITEMS; i++) {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(ItemDefs[i].flags & ItemType::NOMARKET) continue;

		int j;
		if(ItemDefs[i].type & IT_NORMAL) {
			if (i==I_SILVER) continue;
			if (i==I_GRAIN || i==I_LIVESTOCK || i==I_FISH) {
				if (i==I_FISH && !IsCoastal()) continue;

				int amt = Globals->CITY_MARKET_NORMAL_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
						100;
				} else {
					price = ItemDefs[i].baseprice;
				}

				Market *m = new Market (M_SELL, i, price, amt, population,
						population+2000, amt, amt*2);
				markets.Add(m);
			} else if (i == I_FOOD) {
				int amt = Globals->CITY_MARKET_NORMAL_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					price = (ItemDefs[i].baseprice * (120 + getrandom(80))) /
						100;
				} else {
					price = ItemDefs[i].baseprice;
				}

				Market *m = new Market (M_BUY, i, price, amt, population+500,
						population+3000, amt, amt*5);
				markets.Add(m);
			} else {
				if (ItemDefs[i].pInput[0].item == -1) {
					// Check if the product can be produced in the region
					int canProduce = 0;
					for(unsigned int c = 0;
							c<(sizeof(TerrainDefs[type].prods)/sizeof(Product));
							c++) {
						if(i == TerrainDefs[type].prods[c].product) {
							canProduce = 1;
							break;
						}
					}
					if(canProduce) {
						//
						// This item can be produced in this region, so it
						// can possibly be bought here.
						//
						if (getrandom(2)) {
							int amt = Globals->CITY_MARKET_NORMAL_AMT;
							int price;

							if(Globals->RANDOM_ECONOMY) {
								amt += getrandom(amt);
								price = (ItemDefs[i].baseprice *
										(150 + getrandom(50))) / 100;
							} else {
								price = ItemDefs[i].baseprice;
							}

							Market *m = new Market (M_BUY, i, price, 0,
									1000+population, 4000+population,
									0, amt);
							markets.Add(m);
						}
					} else {
						//
						// This item cannot be produced in this region;
						// perhaps it is in demand here?
						//
						if(!getrandom(6)) {
							int amt = Globals->CITY_MARKET_NORMAL_AMT;
							int price;

							if(Globals->RANDOM_ECONOMY) {
								amt += getrandom(amt);
								price = (ItemDefs[i].baseprice *
										(100 + getrandom(50))) / 100;
							} else {
								price = ItemDefs[i].baseprice;
							}

							Market *m = new Market (M_SELL, i, price, 0,
									1000+population, 4000+population, 0, amt);
							markets.Add(m);
						}
					}
				} else {
					if (!getrandom(3)) {
						int amt = Globals->CITY_MARKET_NORMAL_AMT;
						int price;
						if(Globals->RANDOM_ECONOMY) {
							amt += getrandom(amt);
							price = (ItemDefs[i].baseprice *
									(100 + getrandom(50))) / 100;
						} else {
							price = ItemDefs[i].baseprice;
						}

						Market *m = new Market (M_SELL, i, price, 0,
								1000+population, 4000+population, 0, amt);
						markets.Add(m);
					} else {
						if (!getrandom(6)) {
							int amt = Globals->CITY_MARKET_NORMAL_AMT;
							int price;

							if(Globals->RANDOM_ECONOMY) {
								amt += getrandom(amt);
								price = (ItemDefs[i].baseprice *
										(150 + getrandom(50))) / 100;
							} else {
								price = ItemDefs[i].baseprice;
							}

							Market *m = new Market (M_BUY, i, price, 0,
									1000+population, 4000+population, 0, amt);
							markets.Add(m);
						}
					}
				}
			}
		} else if(ItemDefs[i].type & IT_ADVANCED) {
			j = getrandom(4);
			if (j==2) {
				int amt = Globals->CITY_MARKET_ADVANCED_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
						100;
				} else {
					price = ItemDefs[i].baseprice;
				}

				Market *m = new Market (M_SELL, i, price, 0, 2000+population,
						4000+population, 0, amt);
				markets.Add(m);
			}
		} else if(ItemDefs[i].type & IT_MAGIC) {
			j = getrandom(8);
			if (j==2) {
				int amt = Globals->CITY_MARKET_MAGIC_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					price = (ItemDefs[i].baseprice * (100 + getrandom(50))) /
						100;
				} else {
					price = ItemDefs[i].baseprice;
				}

				Market *m = new Market (M_SELL, i, price, 0, 2000+population,
						4000+population, 0, amt);
				markets.Add(m);
			}
		} else if(ItemDefs[i].type & IT_TRADE) {
			numtrade++;
		}
	}

	// Set up the trade items
	int buy1 = getrandom(numtrade);
	int buy2 = getrandom(numtrade);
	int sell1 = getrandom(numtrade);
	int sell2 = getrandom(numtrade);

	buy1 = getrandom(numtrade);
	while (buy1 == buy2) buy2 = getrandom(numtrade);
	while (sell1 == buy1 || sell1 == buy2) sell1 = getrandom(numtrade);
	while (sell2 == sell1 || sell2 == buy2 || sell2 == buy1)
		sell2 = getrandom(numtrade);

	for (i=0; i<NITEMS; i++) {
		if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		if(ItemDefs[i].flags & ItemType::NOMARKET) continue;

		if(ItemDefs[i].type & IT_TRADE) {
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
						price=(ItemDefs[i].baseprice*(250+getrandom(100)))/100;
					} else {
						price=(ItemDefs[i].baseprice*(150+getrandom(50)))/100;
					}
				} else {
					price = ItemDefs[i].baseprice;
				}

				Market *m = new Market (M_SELL, i, price, 0, 2000+population,
						4000+population, 0, amt);
				markets.Add(m);
			}

			if(addsell) {
				int amt = Globals->CITY_MARKET_TRADE_AMT;
				int price;

				if(Globals->RANDOM_ECONOMY) {
					amt += getrandom(amt);
					if(Globals->MORE_PROFITABLE_TRADE_GOODS) {
						price=(ItemDefs[i].baseprice*(100+getrandom(90)))/100;
					} else {
						price=(ItemDefs[i].baseprice*(100+getrandom(50)))/100;
					}
				} else {
					price = ItemDefs[i].baseprice;
				}

				Market *m = new Market (M_BUY, i, price, 0, 2000+population,
						4000+population, 0, amt);
				markets.Add(m);
			}
		}
	}
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
		int popch = 2500;
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
		town->pop = 500+getrandom(popch);
	} else {
		town->pop = 500;
	}

	town->basepop = town->pop;
	town->activity = 0;

	SetupCityMarket();
}

void ARegion::LairCheck()
{
	// No lair if town in region
	if (town) return;


	TerrainType *tt = &TerrainDefs[type];

	if(!tt->lairChance) return;

	int check = getrandom(100);
	if(check >= tt->lairChance) return;

	int count = 0;
	unsigned int c;
	for(c = 0; c < sizeof(tt->lairs)/sizeof(int); c++) {
		if(tt->lairs[c] != -1) {
			if(!(ObjectDefs[tt->lairs[c]].flags & ObjectType::DISABLED)) {
				count++;
			}
		}
	}
	count = getrandom(count);

	int lair = -1;
	for(c = 0; c < sizeof(tt->lairs)/sizeof(int); c++) {
		if(tt->lairs[c] != -1) {
			if(!(ObjectDefs[tt->lairs[c]].flags & ObjectType::DISABLED)) {
				if(!count) {
					lair = tt->lairs[c];
					break;
				}
				count--;
			}
		}
	}

	if(lair != -1) {
		MakeLair(lair);
		return;
	}
}

void ARegion::MakeLair(int t)
{
	Object *o = new Object(this);
	o->num = buildingseq++;
	o->name = new AString(AString(ObjectDefs[t].name) + " [" + o->num + "]");
	o->type = t;
	o->incomplete = 0;
	o->inner = -1;
	objects.Add(o);
}

int ARegion::GetPoleDistance(int dir)
{
	int ct = 1;
	ARegion *nreg = neighbors[dir];
	while (nreg) {
		ct++;
		nreg = nreg->neighbors[dir];
	}
	return ct;
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
	Object *obj = new Object(this);
	objects.Add(obj);

	if(Globals->LAIR_MONSTERS_EXIST)
		LairCheck();
}

// Used at start to set initial town's
// development level
void ARegion::CheckTownIncrease()
{
	if(!town) return;
	
	//if(town) return;
	
	if(town->pop > 3000) {
		development = development + ((getrandom(Globals->TOWN_DEVELOPMENT) + Globals->TOWN_DEVELOPMENT) / 10);
		return;
	}
	
	if(getrandom(200) > Globals->TOWN_DEVELOPMENT) return;
	
	int a = 5;
	if(Globals->TOWN_DEVELOPMENT < 10) a = Globals->TOWN_DEVELOPMENT / 2;
	int b = 20;
	if(Globals->TOWN_DEVELOPMENT < 60) b = Globals->TOWN_DEVELOPMENT / 3;
	development = development + a + getrandom(b);
	Awrite(AString("> increased city in ") + TerrainDefs[type].name + " in " +
		*name + " (" + xloc + ", " + yloc + ") to dev = " + development);

	TownHabitat();
	town->pop = town->basepop * 2 / 3;
}

int ARegion::CheckSea(int dir, int range, int remainocean)
{
	if (type != R_OCEAN) return 0;
	if (range-- < 1) return 1;
	for (int d2 = -1; d2< 2; d2++) {
		int direc = dir + d2;
		if (direc < 0) direc = NDIRS - direc;
		if (direc >= NDIRS) direc = direc - NDIRS;
		ARegion *newregion = neighbors[direc];
		if (!newregion) continue;
		remainocean += newregion->CheckSea(dir, range, remainocean);
		if (remainocean) break;
	}
	return remainocean;
}

int ARegion::TraceConnectedRoad(int dir, int sum, AList *con, int range)
{
	ARegionPtr *rn = new ARegionPtr();
	rn->ptr = this;
	int isnew = 1;
	forlist(con) {
		ARegionPtr *reg = (ARegionPtr *) elem;
		if ((reg) && (reg->ptr)) if(reg->ptr == this) isnew = 0;
	}
	if(isnew == 0) return sum;
	con->Add(rn);
#if 0
	Awrite(AString(" -> ") + *name + "(" + xloc + ", " + yloc + ")");
	Awrite(AString("   +") + (town->TownType()+1));
#endif
	if(town) sum += town->TownType() + 1;
	if(range > 0) {
		for(int d=0; d<NDIRS; d++) {
			if(!HasExitRoad(d)) continue;
			if(d == GetRealDirComp(dir)) continue;
			ARegion *r = neighbors[d];
			if(!r) continue;
			if(r->HasConnectingRoad(d)) sum = r->TraceConnectedRoad(d, sum, con, range-1);
		}
	}
	return sum;
}

int ARegion::CountRoadConnectedTowns(int range)
{
	int townsum = 0;
	AList *con = new AList();
	ARegionPtr *rp = new ARegionPtr();
	rp->ptr = this;
	con->Add(rp);
	for(int d=0; d<NDIRS; d++) {
		if(!HasExitRoad(d)) continue;
		ARegion *r = neighbors[d];
		if(!r) continue;
		if(r->HasConnectingRoad(d)) townsum = r->TraceConnectedRoad(d, townsum, con, range-1);
	}
	return townsum;	
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
					Population()/5, 0, 10000, 0, 2000);
			markets.Add(m);
			if(Globals->LEADERS_EXIST) {
				ratio = ItemDefs[I_LEADERS].baseprice /
					(float)Globals->BASE_MAN_COST;
				m = new Market(M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
						Population()/25, 0, 10000, 0, 400);
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
/* if 0 */
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
/* endif */
		return;
	}
	
	
	//
	// Don't do pop stuff for AC Exit.
	//
	if (town->pop != 5000) {
		// First, get the target population
		int amt = 0;
		int tot = 0;
		forlist(&markets) {
			Market *m = (Market *) elem;
			if (town->pop > m->minpop) {
				if (ItemDefs[m->item].type & IT_TRADE) {
					if (m->type == M_BUY) {
						amt += 5 * m->activity;
						tot += 5 * m->maxamt;
					}
				} else {
					if (m->type == M_SELL) {
						amt += m->activity;
						tot += m->maxamt;
					}
				}
			}
		}

		int tarpop;
		if (tot) {
			tarpop = (Globals->CITY_POP * amt) / tot;
		} else {
			tarpop = 0;
		}

		// Let's bump tarpop up
		tarpop = (tarpop * 3) / 2;
		if (tarpop > Globals->CITY_POP) tarpop = Globals->CITY_POP;

		town->pop = town->pop + (tarpop - town->pop) / 5;

		// Check base population
		if (town->pop < town->basepop) {
			town->pop = town->basepop;
		}
		if ((town->pop * 2) / 3 > town->basepop) {
			town->basepop = (town->pop * 2) / 3;
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
			if(HasExitRoad(i) && nbor->HasConnectingRoad(i)) {
				cv += 25;
				for(int d=0; d<NDIRS; d++) {
					if((d == GetRealDirComp(i)) || (!nbor->HasExitRoad(d))) continue;
					ARegion *ntwo = nbor->neighbors[d];
					if((nbor) && (ntwo->HasConnectingRoad(d))) {
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
		money = (Wages() - Globals->MAINTENANCE_COST) * Population();
		if (money < 0) money = 0;

		if (type != R_NEXUS) {
			// Setup working
			// Production * p = products.GetProd(I_SILVER,-1);
			int hack = 0;
			if (IsStartingCity()) {
				// Higher wages in the entry cities.
				hack = Wages() * Population();
			} else {
				// (Globals->WORK_FRACTION);
				hack = (Wages() * Population()) / 5;
			}
			// TODO: Silver available for work and entertainment
			// still causes a segmentation fault!!!
			// p->amount = hack;
			// p->productivity = Wages();

			// Entertainment.
			// p = products.GetProd(I_SILVER,S_ENTERTAINMENT);
			// p->baseamount = money / Globals->ENTERTAIN_FRACTION;
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

			markets.PostTurn(Population(),Wages());
		}
		return;
	}

	// Standard economy
	//
	// First update population based on production
	//
	int activity = 0;
	int amount = 0;
	if (basepopulation) {
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
		money = (Wages() - Globals->MAINTENANCE_COST) * Population();
		if (money < 0) money = 0;

		//
		// Setup working
		//
		Production *p = products.GetProd(I_SILVER, -1);
		if (IsStartingCity()) {
			//
			// Higher wages in the entry cities.
			//
			p->amount = Wages() * Population();
		} else {
			p->amount = (Wages() * Population()) / Globals->WORK_FRACTION;
		}
		p->productivity = Wages();

		//
		// Entertainment.
		//
		p = products.GetProd(I_SILVER, S_ENTERTAINMENT);
		p->baseamount = money / Globals->ENTERTAIN_FRACTION;

		markets.PostTurn(Population(), Wages());
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
void ARegion::DoDecayCheck(ARegionList *pRegs)
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		if(!(ObjectDefs[o->type].flags & ObjectType::NEVERDECAY)) {
			DoDecayClicks(o, pRegs);
		}
	}
}

// AS
void ARegion::DoDecayClicks(Object *o, ARegionList *pRegs)
{
	if(ObjectDefs[o->type].flags & ObjectType::NEVERDECAY) return;

	int clicks = getrandom(GetMaxClicks());
	clicks += PillageCheck();

	if(clicks > ObjectDefs[o->type].maxMonthlyDecay)
		clicks = ObjectDefs[o->type].maxMonthlyDecay;

	o->incomplete += clicks;

	if(o->incomplete > 0) {
		// trigger decay event
		RunDecayEvent(o, pRegs);
	}
}

// AS
void ARegion::RunDecayEvent(Object *o, ARegionList *pRegs)
{
	AList *pFactions;
	pFactions = PresentFactions();
	forlist (pFactions) {
		Faction *f = ((FactionPtr *) elem)->ptr;
		f->Event(GetDecayFlavor() + *o->name + " " +
				ObjectDefs[o->type].name + " in " +
				ShortPrint(pRegs));
	}
}

// AS
AString ARegion::GetDecayFlavor()
{
	AString flavor;
	int badWeather = 0;
	if (weather != W_NORMAL && !clearskies) badWeather = 1;
	if (!Globals->WEATHER_EXISTS) badWeather = 0;
	switch (type) {
		case R_PLAIN:
		case R_ISLAND_PLAIN:
		case R_CERAN_PLAIN1:
		case R_CERAN_PLAIN2:
		case R_CERAN_PLAIN3:
		case R_CERAN_LAKE:
			flavor = AString("Floods have damaged ");
			break;
		case R_DESERT:
		case R_CERAN_DESERT1:
		case R_CERAN_DESERT2:
		case R_CERAN_DESERT3:
			flavor = AString("Flashfloods have damaged ");
			break;
		case R_CERAN_WASTELAND:
		case R_CERAN_WASTELAND1:
			flavor = AString("Magical radiation has damaged ");
			break;
		case R_TUNDRA:
		case R_CERAN_TUNDRA1:
		case R_CERAN_TUNDRA2:
		case R_CERAN_TUNDRA3:
			if (badWeather) {
				flavor = AString("Ground freezing has damaged ");
			} else {
				flavor = AString("Ground thaw has damaged ");
			}
			break;
		case R_MOUNTAIN:
		case R_ISLAND_MOUNTAIN:
		case R_CERAN_MOUNTAIN1:
		case R_CERAN_MOUNTAIN2:
		case R_CERAN_MOUNTAIN3:
			if (badWeather) {
				flavor = AString("Avalanches have damaged ");
			} else {
				flavor = AString("Rockslides have damaged ");
			}
			break;
		case R_CERAN_HILL:
		case R_CERAN_HILL1:
		case R_CERAN_HILL2:
			flavor = AString("Quakes have damaged ");
			break;
		case R_FOREST:
		case R_SWAMP:
		case R_ISLAND_SWAMP:
		case R_JUNGLE:
		case R_CERAN_FOREST1:
		case R_CERAN_FOREST2:
		case R_CERAN_FOREST3:
		case R_CERAN_MYSTFOREST:
		case R_CERAN_MYSTFOREST1:
		case R_CERAN_MYSTFOREST2:
		case R_CERAN_SWAMP1:
		case R_CERAN_SWAMP2:
		case R_CERAN_SWAMP3:
		case R_CERAN_JUNGLE1:
		case R_CERAN_JUNGLE2:
		case R_CERAN_JUNGLE3:
			flavor = AString("Encroaching vegetation has damaged ");
			break;
		case R_CAVERN:
		case R_UFOREST:
		case R_TUNNELS:
		case R_CERAN_CAVERN1:
		case R_CERAN_CAVERN2:
		case R_CERAN_CAVERN3:
		case R_CERAN_UFOREST1:
		case R_CERAN_UFOREST2:
		case R_CERAN_UFOREST3:
		case R_CERAN_TUNNELS1:
		case R_CERAN_TUNNELS2:
		case R_CHASM:
		case R_CERAN_CHASM1:
		case R_GROTTO:
		case R_CERAN_GROTTO1:
		case R_DFOREST:
		case R_CERAN_DFOREST1:
			if (badWeather) {
				flavor = AString("Lava flows have damaged ");
			} else {
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
	switch (type) {
		case R_PLAIN:
		case R_ISLAND_PLAIN:
		case R_TUNDRA:
		case R_CERAN_PLAIN1:
		case R_CERAN_PLAIN2:
		case R_CERAN_PLAIN3:
		case R_CERAN_LAKE:
		case R_CERAN_TUNDRA1:
		case R_CERAN_TUNDRA2:
		case R_CERAN_TUNDRA3:
			terrainAdd = -1;
			if (badWeather) weatherAdd = 4;
			break;
		case R_MOUNTAIN:
		case R_ISLAND_MOUNTAIN:
		case R_CERAN_MOUNTAIN1:
		case R_CERAN_MOUNTAIN2:
		case R_CERAN_MOUNTAIN3:
		case R_CERAN_HILL:
		case R_CERAN_HILL1:
		case R_CERAN_HILL2:
			terrainMult = 2;
			if (badWeather) weatherAdd = 4;
			break;
		case R_FOREST:
		case R_SWAMP:
		case R_ISLAND_SWAMP:
		case R_JUNGLE:
		case R_CERAN_FOREST1:
		case R_CERAN_FOREST2:
		case R_CERAN_FOREST3:
		case R_CERAN_MYSTFOREST:
		case R_CERAN_MYSTFOREST1:
		case R_CERAN_MYSTFOREST2:
		case R_CERAN_SWAMP1:
		case R_CERAN_SWAMP2:
		case R_CERAN_SWAMP3:
		case R_CERAN_JUNGLE1:
		case R_CERAN_JUNGLE2:
		case R_CERAN_JUNGLE3:
			terrainAdd = -1;
			terrainMult = 2;
			if (badWeather) weatherAdd = 1;
			break;
		case R_DESERT:
		case R_CERAN_DESERT1:
		case R_CERAN_DESERT2:
		case R_CERAN_DESERT3:
			terrainAdd = -1;
			if (badWeather) weatherAdd = 5;
		case R_CAVERN:
		case R_UFOREST:
		case R_TUNNELS:
		case R_CERAN_CAVERN1:
		case R_CERAN_CAVERN2:
		case R_CERAN_CAVERN3:
		case R_CERAN_UFOREST1:
		case R_CERAN_UFOREST2:
		case R_CERAN_UFOREST3:
		case R_CERAN_TUNNELS1:
		case R_CERAN_TUNNELS2:
		case R_CHASM:
		case R_CERAN_CHASM1:
		case R_GROTTO:
		case R_CERAN_GROTTO1:
		case R_DFOREST:
		case R_CERAN_DFOREST1:
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
	forlist (&objects) {
		Object *o = (Object *) elem;
		if(o->IsRoad() && o->incomplete < 1) return 1;
	}
	return 0;
}

// AS
int ARegion::HasExitRoad(int realDirection)
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		if (o->IsRoad() && o->incomplete < 1) {
			if (o->type == GetRoadDirection(realDirection)) return 1;
		}
	}
	return 0;
}

// AS
int ARegion::CountConnectingRoads()
{
	int connections = 0;
	for (int i = 0; i < NDIRS; i++) {
		if (HasExitRoad(i) && neighbors[i] &&
				neighbors[i]->HasConnectingRoad(i))
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
	switch (realDirection) {
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
	switch (realDirection) {
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
			if (o->incomplete < 1 &&
					ObjectDefs[o->type].productionAided == prod->itemtype) {
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

AString ARegion::ShortPrint(ARegionList *pRegs)
{
	AString temp = TerrainDefs[type].name;

	temp += AString(" (") + xloc + "," + yloc;

	ARegionArray *pArr = pRegs->pRegionArrays[zloc];
	if(pArr->strName) {
		temp += ",";
		if(Globals->EASIER_UNDERWORLD &&
		   (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS > 1)) {
			temp += AString("") + zloc + " <";
		} else {
			// add less explicit multilevel information about the underworld
			if(zloc > 2 && zloc < Globals->UNDERWORLD_LEVELS+2) {
				for(int i = zloc; i > 3; i--) {
					temp += "very ";
				}
				temp += "deep ";
			} else if((zloc > Globals->UNDERWORLD_LEVELS+2) &&
					  (zloc < Globals->UNDERWORLD_LEVELS +
					   Globals->UNDERDEEP_LEVELS + 2)) {
				for(int i = zloc; i > Globals->UNDERWORLD_LEVELS + 3; i--) {
					temp += "very ";
				}
				temp += "deep ";
			}
		}
		temp += *pArr->strName;
		if(Globals->EASIER_UNDERWORLD &&
		   (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS > 1)) {
			temp += ">";
		}
	}
	temp += ")";

	temp += AString(" in ") + *name;
	return temp;
}

AString ARegion::Print(ARegionList *pRegs)
{
	AString temp = ShortPrint(pRegs);
	if (town) {
		temp += AString(", contains ") + *(town->name) + " [" +
			TownString(town->TownType()) + "]";
	}
	return temp;
}

void ARegion::SetLoc(int x, int y, int z)
{
	xloc = x;
	yloc = y;
	zloc = z;
}

void ARegion::SetGateStatus(int month)
{
	if ((type == R_NEXUS) || (Globals->START_GATES_OPEN && IsStartingCity())) {
		gateopen = 1;
		return;
	}
	gateopen = 0;
	for (int i = 0; i < Globals->GATES_NOT_PERENNIAL; i++) {
		int dmon = gatemonth + i;
		if (dmon > 11) dmon = dmon - 12;
		if (dmon == month) gateopen = 1;
	}
}

void ARegion::Kill(Unit *u)
{
	Unit *first = 0;
	forlist((&objects)) {
		Object *obj = (Object *) elem;
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
		// give u's stuff to first
		forlist(&u->items) {
			Item *i = (Item *) elem;
			if (!IsSoldier(i->type)) {
				first->items.SetNum(i->type, first->items.GetNum(i->type) +
									i->num);
				// If we're in ocean and not in a structure, make sure that
				// the first unit can actually hold the stuff and not drown
				// If the item would cause them to drown then they won't
				// pick it up.
				if(TerrainDefs[type].similar_type == R_OCEAN) {
					if(first->object->type == O_DUMMY) {
						if(!first->CanReallySwim()) {
							first->items.SetNum(i->type,
									first->items.GetNum(i->type) - i->num);
						}
					}
				}
			}
			u->items.SetNum(i->type, 0);
		}
	}

	u->MoveUnit(0);
	hell.Add(u);
}

void ARegion::ClearHell()
{
	hell.DeleteAll();
}

Object *ARegion::GetObject(int num)
{
	forlist(&objects) {
		Object *o = (Object *) elem;
		if (o->num == num) return o;
	}
	return 0;
}

Object *ARegion::GetDummy()
{
	forlist(&objects) {
		Object *o = (Object *) elem;
		if (o->type == O_DUMMY) return o;
	}
	return 0;
}

Unit *ARegion::GetUnit(int num)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		Unit *u = obj->GetUnit(num);
		if(u) {
			return(u);
		}
	}
	return 0;
}

Location *ARegion::GetLocation(UnitId *id, int faction)
{
	Unit *retval = 0;
	forlist(&objects) {
		Object *o = (Object *) elem;
		retval = o->GetUnitId(id, faction);
		if (retval) {
			Location *l = new Location;
			l->region = this;
			l->obj = o;
			l->unit = retval;
			return l;
		}
	}
	return 0;
}

Unit *ARegion::GetUnitAlias(int alias, int faction)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		Unit *u = obj->GetUnitAlias(alias, faction);
		if(u) {
			return(u);
		}
	}
	return 0;
}

Unit *ARegion::GetUnitId(UnitId *id, int faction)
{
	Unit *retval = 0;
	forlist(&objects) {
		Object *o = (Object *) elem;
		retval = o->GetUnitId(id, faction);
		if (retval) return retval;
	}
	return retval;
}

Location *ARegionList::GetUnitId(UnitId *id, int faction, ARegion *cur)
{
	Location *retval = NULL;
	// Check current region first
	retval = cur->GetLocation(id, faction);
	if (retval) return retval;

	// No? We must be looking for an existing unit.
	if (!id->unitnum) return NULL;

	return this->FindUnit(id->unitnum);
}

int ARegion::Present(Faction *f)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units))
			if (((Unit *) elem)->faction == f) return 1;
	}
	return 0;
}

AList *ARegion::PresentFactions()
{
	AList *facs = new AList;
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (!GetFaction2(facs, u->faction->num)) {
				FactionPtr *p = new FactionPtr;
				p->ptr = u->faction;
				facs->Add(p);
			}
		}
	}
	return facs;
}

void ARegion::Writeout(Aoutfile *f)
{
	f->PutStr(*name);
	f->PutInt(num);
	if (type != -1) f->PutStr(TerrainDefs[type].type);
	else f->PutStr("NO_TERRAIN");
	f->PutInt(buildingseq);
	f->PutInt(gate);
	if(gate > 0) f->PutInt(gatemonth);
	if (race != -1) f->PutStr(ItemDefs[race].abr);
	else f->PutStr("NO_RACE");
	f->PutInt(population);
	f->PutInt(basepopulation);
	f->PutInt(migration);
	f->PutInt(wages);
	f->PutInt(maxwages);
	f->PutInt(money);

	f->PutInt(habitat);
	f->PutInt(development);
	f->PutInt(growth);
	f->PutInt(mortality);

	if (town) {
		f->PutInt(1);
		town->Writeout(f);
	} else {
		f->PutInt(0);
	}

	f->PutInt(xloc);
	f->PutInt(yloc);
	f->PutInt(zloc);

	products.Writeout(f);
	markets.Writeout(f);

	f->PutInt(objects.Num());
	forlist ((&objects)) ((Object *) elem)->Writeout(f);
}

int LookupRegionType(AString *token)
{
	for (int i = 0; i < R_NUM; i++) {
		if (*token == TerrainDefs[i].type) return i;
	}
	return -1;
}

void ARegion::Readin(Ainfile *f, AList *facs, ATL_VER v)
{
	AString *temp;

	name = f->GetStr();

	num = f->GetInt();
	temp = f->GetStr();
	type = LookupRegionType(temp);
	delete temp;
	buildingseq = f->GetInt();
	gate = f->GetInt();
	if(gate > 0) gatemonth = f->GetInt();

	temp = f->GetStr();
	race = LookupItem(temp);
	delete temp;

	population = f->GetInt();
	basepopulation = f->GetInt();
	migration = f->GetInt();
	wages = f->GetInt();
	maxwages = f->GetInt();
	money = f->GetInt();

	habitat = f->GetInt();
	development = f->GetInt();
	growth = f->GetInt();
	mortality = f->GetInt();

	if (f->GetInt()) {
		town = new TownInfo;
		town->Readin(f, v);
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
		Object *temp = new Object(this);
		temp->Readin(f, facs, v);
		objects.Add(temp);
	}
}

int ARegion::CanMakeAdv(Faction *fac, int item)
{
	AString skname;
	int sk;
	Farsight *f;

	if(Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			f = (Farsight *)elem;
			if(f && f->faction == fac && f->unit) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if(f->unit->GetSkill(sk) >= ItemDefs[item].pLevel)
					return 1;
			}
		}
	}

	if((Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_RESOURCES)) {
		forlist(&passers) {
			f = (Farsight *)elem;
			if(f && f->faction == fac && f->unit) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if(f->unit->GetSkill(sk) >= ItemDefs[item].pLevel)
					return 1;
			}
		}
	}

	forlist(&objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->faction == fac) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if (u->GetSkill(sk) >= ItemDefs[item].pLevel)
					return 1;
			}
		}
	}
	return 0;
}

void ARegion::WriteProducts(Areport *f, Faction *fac, int present)
{
	AString temp = "Products: ";
	int has = 0;
	forlist((&products)) {
		Production *p = ((Production *) elem);
		if (ItemDefs[p->itemtype].type & IT_ADVANCED) {
			if (CanMakeAdv(fac, p->itemtype) || (fac->IsNPC())) {
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
					if((Globals->TRANSIT_REPORT &
							GameDefs::REPORT_SHOW_ENTERTAINMENT) || present) {
						f->PutStr(AString("Entertainment available: $") +
								p->amount + ".");
					} else {
						f->PutStr(AString("Entertainment available: $0."));
					}
				}
			} else {
				if(!present &&
				   !(Globals->TRANSIT_REPORT &
					   GameDefs::REPORT_SHOW_RESOURCES))
					continue;
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

int ARegion::HasItem(Faction *fac, int item)
{
	forlist(&objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->faction == fac) {
				if (u->items.GetNum(item)) return 1;
			}
		}
	}
	return 0;
}

void ARegion::WriteMarkets(Areport *f, Faction *fac, int present)
{
	AString temp = "Wanted: ";
	int has = 0;
	forlist(&markets) {
		Market *m = (Market *) elem;
		if (!m->amount) continue;
		if(!present &&
		   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_MARKETS))
			continue;
		if (m->type == M_SELL) {
			if (ItemDefs[m->item].type & IT_ADVANCED) {
				if(!Globals->MARKETS_SHOW_ADVANCED_ITEMS) {
					if (!HasItem(fac, m->item)) {
						continue;
					}
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
			Market *m = (Market *) elem;
			if (!m->amount) continue;
			if(!present &&
			   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_MARKETS))
				continue;
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

void ARegion::WriteEconomy(Areport *f, Faction *fac, int present)
{
	f->AddTab();

	if((Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_WAGES) || present) {
		f->PutStr(AString("Wages: ") + WagesForReport() + ".");
	} else {
		f->PutStr(AString("Wages: $0."));
	}

	WriteMarkets(f, fac, present);

	WriteProducts(f, fac, present);

	f->EndLine();
	f->DropTab();
}

void ARegion::WriteExits(Areport *f, ARegionList *pRegs, int *exits_seen)
{
	f->PutStr("Exits:");
	f->AddTab();
	int y = 0;
	for (int i=0; i<NDIRS; i++) {
		ARegion *r = neighbors[i];
		if (r && exits_seen[i]) {
			f->PutStr(AString(DirectionStrs[i]) + " : " +
					r->Print(pRegs) + ".");
			y = 1;
		}
	}
	if (!y) f->PutStr("none");
	f->DropTab();
	f->EndLine();
}

#define AC_STRING "%s Nexus is a magical place; the entryway " \
"to the world of %s. Enjoy your stay, the city guards should " \
"keep you safe as long as you should choose to stay. However, rumor " \
"has it that once you have left the Nexus, you can never return."

void ARegion::WriteReport(Areport *f, Faction *fac, int month,
		ARegionList *pRegions)
{
	Farsight *farsight = GetFarsight(&farsees, fac);
	Farsight *passer = GetFarsight(&passers, fac);
	int present = Present(fac) || fac->IsNPC();

	if (farsight || passer || present)  {
		AString temp = Print(pRegions);
		if (Population() &&
			(present || farsight ||
			 (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_PEASANTS))) {
			temp += AString(", ") + Population() + " peasants";
			if(Globals->RACES_EXIST) {
				temp += AString(" (") + ItemDefs[race].names + ")";
			}
			if(present || farsight ||
			   Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_REGION_MONEY) {
				temp += AString(", $") + money;
			} else {
				temp += AString(", $0");
			}
		}
		temp += ".";
		f->PutStr(temp);
		f->PutStr("-------------------------------------------------"
				"-----------");

		f->AddTab();
		if(Globals->WEATHER_EXISTS) {
			temp = "It was ";
			if (clearskies) temp = "unnaturally clear ";
			else {
				if(weather == W_BLIZZARD) temp = "There was an unnatural ";
				else if(weather == W_NORMAL) temp = "The weather was ";
				temp += SeasonNames[weather];
			}
			temp += " last month; ";
			int nxtweather = pRegions->GetWeather(this, (month + 1) % 12);
			temp += "it will be ";
			temp += SeasonNames[nxtweather];
			temp += " next month.";
			f->PutStr(temp);
		}

		if (type == R_NEXUS) {
			int len = strlen(AC_STRING)+2*strlen(Globals->WORLD_NAME);
			char *nexus_desc = new char[len];
			sprintf(nexus_desc, AC_STRING, Globals->WORLD_NAME,
					Globals->WORLD_NAME);
			f->PutStr("");
			f->PutStr(nexus_desc);
			f->PutStr("");
			delete [] nexus_desc;
		}

		f->DropTab();

		WriteEconomy(f, fac, present || farsight);

		int exits_seen[NDIRS];
		if(present || farsight ||
		   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_ALL_EXITS)) {
			for(int i = 0; i < NDIRS; i++)
				exits_seen[i] = 1;
		} else {
			// This is just a transit report and we're not showing all
			// exits.   See if we are showing used exits.

			// Show none by default.
			int i;
			for(i = 0; i < NDIRS; i++)
				exits_seen[i] = 0;
			// Now, if we should, show the ones actually used.
			if(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_USED_EXITS) {
				forlist(&passers) {
					Farsight *p = (Farsight *)elem;
					if(p->faction == fac) {
						for(i = 0; i < NDIRS; i++) {
							exits_seen[i] |= p->exits_used[i];
						}
					}
				}
			}
		}

		WriteExits(f, pRegions, exits_seen);

		if(Globals->GATES_EXIST && gate && gate != -1) {
			int sawgate = 0;
			if(fac->IsNPC())
				sawgate = 1;
			if(Globals->IMPROVED_FARSIGHT && farsight) {
				forlist(&farsees) {
					Farsight *watcher = (Farsight *)elem;
					if(watcher && watcher->faction == fac && watcher->unit) {
						if(watcher->unit->GetSkill(S_GATE_LORE)) {
							sawgate = 1;
						}
					}
				}
			}
			if(Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) {
				forlist(&passers) {
					Farsight *watcher = (Farsight *)elem;
					if(watcher && watcher->faction == fac && watcher->unit) {
						if(watcher->unit->GetSkill(S_GATE_LORE)) {
							sawgate = 1;
						}
					}
				}
			}
			forlist(&objects) {
				Object *o = (Object *) elem;
				forlist(&o->units) {
					Unit *u = (Unit *) elem;
					if (!sawgate &&
							((u->faction == fac) &&
							 u->GetSkill(S_GATE_LORE))) {
						sawgate = 1;
					}
				}
			}
			if(sawgate) {
				if(gateopen) {
					f->PutStr(AString("There is a Gate here (Gate ") + gate +
							" of " + (pRegions->numberofgates) + ").");
					f->PutStr("");
				} else if (Globals->SHOW_CLOSED_GATES) {
					f->PutStr(AString("There is a closed Gate here."));
					f->PutStr("");
				}
			}
		}

		int obs = GetObservation(fac, 0);
		int truesight = GetTrueSight(fac, 0);
		int detfac = 0;

		int passobs = GetObservation(fac, 1);
		int passtrue = GetTrueSight(fac, 1);
		int passdetfac = detfac;

		if(fac->IsNPC()) {
			obs = 10;
			passobs = 10;
		}

		forlist (&objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == fac && u->GetSkill(S_MIND_READING) > 2) {
					detfac = 1;
				}
			}
		}
		if(Globals->IMPROVED_FARSIGHT && farsight) {
			forlist(&farsees) {
				Farsight *watcher = (Farsight *)elem;
				if(watcher && watcher->faction == fac && watcher->unit) {
					if(watcher->unit->GetSkill(S_MIND_READING) > 2) {
						detfac = 1;
					}
				}
			}
		}

		if((Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
		   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
			forlist(&passers) {
				Farsight *watcher = (Farsight *)elem;
				if(watcher && watcher->faction == fac && watcher->unit) {
					if(watcher->unit->GetSkill(S_MIND_READING) > 2) {
						passdetfac = 1;
					}
				}
			}
		}

		{
			forlist (&objects) {
				((Object *) elem)->Report(f, fac, obs, truesight, detfac,
										  passobs, passtrue, passdetfac,
										  present || farsight);
			}
			f->EndLine();
		}
	}
}

// DK
void ARegion::WriteTemplate(Areport *f, Faction *fac,
		ARegionList *pRegs, int month)
{
	int header = 0;

	forlist (&objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->faction == fac) {
				if (!header) {
					// DK
					if (fac->temformat == TEMPLATE_MAP) {
						WriteTemplateHeader(f, fac, pRegs, month);
					} else {
						f->PutStr("");
						f->PutStr(AString("*** ") + Print(pRegs) + " ***", 1);
					}
					header = 1;
				}

				f->PutStr("");
				f->PutStr(AString("unit ") + u->num);
				// DK
				if (fac->temformat == TEMPLATE_LONG ||
						fac->temformat == TEMPLATE_MAP) {
					f->PutStr(u->TemplateReport(), 1);
				}
				forlist(&(u->oldorders)) {
					f->PutStr(*((AString *) elem));
				}
				u->oldorders.DeleteAll();

				if (u->turnorders.First()) {
					int first = 1;
					TurnOrder *tOrder;
					forlist(&u->turnorders) {
						tOrder = (TurnOrder *)elem;
						if (first) {
							forlist(&tOrder->turnOrders) {
								f->PutStr(*((AString *) elem));
							}
							first = 0;
						} else {
							if (tOrder->repeating)
								f->PutStr(AString("@TURN"));
							else
								f->PutStr(AString("TURN"));

							forlist(&tOrder->turnOrders) {
								f->PutStr(*((AString *) elem));
							}
							f->PutStr(AString("ENDTURN"));
						}
					}
					tOrder = (TurnOrder *) u->turnorders.First();
					if (tOrder->repeating) {
						f->PutStr(AString("@TURN"));
						forlist(&tOrder->turnOrders) {
							f->PutStr(*((AString *) elem));
						}
						f->PutStr(AString("ENDTURN"));
					}
				}
				u->turnorders.DeleteAll();
			}
		}
	}
}

int ARegion::GetTrueSight(Faction *f, int usepassers)
{
	int truesight = 0;

	if(Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int t = farsight->unit->GetSkill(S_TRUE_SEEING);
				if(t > truesight) truesight = t;
			}
		}
	}

	if(usepassers &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
		forlist(&passers) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int t = farsight->unit->GetSkill(S_TRUE_SEEING);
				if(t > truesight) truesight = t;
			}
		}
	}

	forlist ((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == f) {
				int temp = u->GetSkill(S_TRUE_SEEING);
				if (temp>truesight) truesight = temp;
			}
		}
	}
	return truesight;
}

int ARegion::GetObservation(Faction *f, int usepassers)
{
	int obs = 0;

	if(Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int o = farsight->unit->GetAttribute("observation");
				if(o > obs) obs = o;
			}
		}
	}

	if(usepassers &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
		forlist(&passers) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int o = farsight->unit->GetAttribute("observation");
				if(o > obs) obs = o;
			}
		}
	}

	forlist ((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == f) {
				int temp = u->GetAttribute("observation");
				if (temp>obs) obs = temp;
			}
		}
	}
	return obs;
}

void ARegion::SetWeather(int newWeather)
{
	weather = newWeather;
}

int ARegion::IsCoastal()
{
	if ((type != R_LAKE) && (TerrainDefs[type].similar_type == R_OCEAN)) return 1;
	int seacount = 0;
	for (int i=0; i<NDIRS; i++) {
		if (neighbors[i] && TerrainDefs[neighbors[i]->type].similar_type == R_OCEAN) {
		if (!Globals->LAKESIDE_IS_COASTAL && neighbors[i]->type == R_LAKE) continue;
		seacount++;
	}
	}
	return seacount;
}

int ARegion::IsCoastalOrLakeside()
{
	if ((type != R_LAKE) && (TerrainDefs[type].similar_type == R_OCEAN)) return 1;
	int seacount = 0;
	for (int i=0; i<NDIRS; i++) {
		if (neighbors[i] && TerrainDefs[neighbors[i]->type].similar_type == R_OCEAN) {
		seacount++;
	}
	}
	return seacount;
}

int ARegion::MoveCost(int movetype, ARegion *fromRegion, int dir)
{
	int cost = 1;
	if(Globals->WEATHER_EXISTS) {
		cost = 2;
		if (weather == W_BLIZZARD) return 10;
		if (weather == W_NORMAL || clearskies) cost = 1;
	}
	if (movetype == M_WALK || movetype == M_RIDE) {
		cost = (TerrainDefs[type].movepoints * cost);
		if((cost>1) && fromRegion->HasExitRoad(dir) && HasConnectingRoad(dir))
			cost -= cost/2;
	}
	if(cost < 1) cost = 1;
	return cost;
}

Unit *ARegion::Forbidden(Unit *u)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u2 = (Unit *) elem;
			if (u2->Forbids(this, u)) return u2;
		}
	}
	return 0;
}

Unit *ARegion::ForbiddenByAlly(Unit *u)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u2 = (Unit *) elem;
			if (u->faction->GetAttitude(u2->faction->num) == A_ALLY &&
				u2->Forbids(this, u)) return u2;
		}
	}
	return 0;
}

int ARegion::HasCityGuard()
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->type == U_GUARD && u->GetSoldiers() &&
				u->guard == GUARD_GUARD) {
				return 1;
			}
		}
	}
	return 0;
}

int ARegion::NotifySpell(Unit *caster, char *spell, ARegionList *pRegs)
{
	AList flist;
	unsigned int i;

	SkillType *pS = FindSkill(spell);

	if (!(pS->flags & SkillType::NOTIFY)) {
		// Okay, we aren't notifyable, check our prerequisites
		for(i = 0; i < sizeof(pS->depends)/sizeof(SkillDepend); i++) {
			if (pS->depends[i].skill == NULL) break;
			if(NotifySpell(caster, pS->depends[i].skill, pRegs)) return 1;
		}
		return 0;
	}

	AString skname = spell;
	int sp = LookupSkill(&skname);
	forlist((&objects)) {
		Object *o = (Object *) elem;
		forlist ((&o->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == caster->faction) continue;
			if (u->GetSkill(sp)) {
				if (!GetFaction2(&flist, u->faction->num)) {
					FactionPtr *fp = new FactionPtr;
					fp->ptr = u->faction;
					flist.Add(fp);
				}
			}
		}
	}

	forlist_reuse (&flist) {
		FactionPtr *fp = (FactionPtr *) elem;
		fp->ptr->Event(AString(*(caster->name)) + " uses " + SkillStrs(sp) +
				" in " + Print(pRegs) + ".");
	}
	return 1;
}

// ALT, 26-Jul-2000
// Procedure to notify all units in city about city name change
void ARegion::NotifyCity(Unit *caster, AString& oldname, AString& newname)
{
	AList flist;
	forlist((&objects)) {
		Object *o = (Object *) elem;
		forlist ((&o->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == caster->faction) continue;
			if (!GetFaction2(&flist, u->faction->num)) {
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

int ARegion::CanTax(Unit *u)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u2 = (Unit *) elem;
			if (u2->guard == GUARD_GUARD && u2->IsAlive())
				if (u2->GetAttitude(this, u) <= A_NEUTRAL)
					return 0;
		}
	}
	return 1;
}

int ARegion::CanPillage(Unit *u)
{
	forlist(&objects) {
		Object *obj = (Object *)elem;
		forlist (&obj->units) {
			Unit *u2 = (Unit *)elem;
			if(u2->guard == GUARD_GUARD && u2->IsAlive() &&
					u2->faction != u->faction)
				return 0;
		}
	}
	return 1;
}

int ARegion::ForbiddenShip(Object *ship)
{
	forlist(&ship->units) {
		Unit *u = (Unit *) elem;
		if (Forbidden(u)) return 1;
	}
	return 0;
}

void ARegion::DefaultOrders()
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
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
		Object *o = (Object *) elem;
		if (o->inner != -1) return 1;
	}
	return 0;
}

int ARegion::IsGuarded()
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		forlist (&o->units) {
			Unit *u = (Unit *) elem;
			if (u->guard == GUARD_GUARD) return 1;
		}
	}
	return 0;
}

int ARegion::CountWMons()
{
	int count = 0;
	forlist (&objects) {
		Object *o = (Object *) elem;
		forlist (&o->units) {
			Unit *u = (Unit *) elem;
			if (u->type == U_WMON) {
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
	if(pRegionArrays) {
		int i;
		for(i = 0; i < numLevels; i++) {
			delete pRegionArrays[i];
		}

		delete pRegionArrays;
	}
}

void ARegionList::WriteRegions(Aoutfile *f)
{
	f->PutInt(Num());

	f->PutInt(numLevels);
	int i;
	for(i = 0; i < numLevels; i++) {
		ARegionArray *pRegs = pRegionArrays[i];
		f->PutInt(pRegs->x);
		f->PutInt(pRegs->y);
		if(pRegs->strName) {
			f->PutStr(*pRegs->strName);
		} else {
			f->PutStr("none");
		}
		f->PutInt(pRegs->levelType);
	}

	f->PutInt(numberofgates);
	forlist(this) ((ARegion *) elem)->Writeout(f);
	{
		f->PutStr("Neighbors");
		forlist(this) {
			ARegion *reg = (ARegion *) elem;
			for(i = 0; i < NDIRS; i++) {
				if (reg->neighbors[i]) {
					f->PutInt(reg->neighbors[i]->num);
				} else {
					f->PutInt(-1);
				}
			}
		}
	}
}

int ARegionList::ReadRegions(Ainfile *f, AList *factions, ATL_VER v)
{
	int num = f->GetInt();

	numLevels = f->GetInt();
	CreateLevels(numLevels);
	int i;
	for(i = 0; i < numLevels; i++) {
		int curX = f->GetInt();
		int curY = f->GetInt();
		AString *name = f->GetStr();
		ARegionArray *pRegs = new ARegionArray(curX, curY);
		if(*name == "none") {
			pRegs->strName = 0;
			delete name;
		} else {
			pRegs->strName = name;
		}
		pRegs->levelType = f->GetInt();
		pRegionArrays[i] = pRegs;
	}

	numberofgates = f->GetInt();

	ARegionFlatArray fa(num);

	Awrite("Reading the regions...");
	for(i = 0; i < num; i++) {
		ARegion *temp = new ARegion;
		temp->Readin(f, factions, v);
		fa.SetRegion(temp->num, temp);
		Add(temp);

		pRegionArrays[temp->zloc]->SetRegion(temp->xloc, temp->yloc,
												temp);
	}

	Awrite("Setting up the neighbors...");
	{
		delete f->GetStr();
		forlist(this) {
			ARegion *reg = (ARegion *) elem;
			for(i = 0; i < NDIRS; i++) {
				int j = f->GetInt();
				if (j != -1) {
					reg->neighbors[i] = fa.GetRegion(j);
				} else {
					reg->neighbors[i] = 0;
				}
			}
		}
	}
	return 1;
}

ARegion *ARegionList::GetRegion(int n)
{
	forlist(this) {
		if (((ARegion *) elem)->num == n) return ((ARegion *) elem);
	}
	return 0;
}

ARegion *ARegionList::GetRegion(int x, int y, int z)
{

	if (z >= numLevels) return NULL;

	ARegionArray *arr = pRegionArrays[z];

	x = (x + arr->x) % arr->x;
	y = (y + arr->y) % arr->y;

	return(arr->GetRegion(x, y));
}

Location *ARegionList::FindUnit(int i)
{
	forlist(this) {
		ARegion *reg = (ARegion *) elem;
		forlist((&reg->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->num == i) {
					Location *retval = new Location;
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

void ARegionList::NeighSetup(ARegion *r, ARegionArray *ar)
{
	r->ZeroNeighbors();

	if (r->yloc != 0 && r->yloc != 1) {
		r->neighbors[D_NORTH] = ar->GetRegion(r->xloc, r->yloc - 2);
	}
	if (r->yloc != 0) {
		r->neighbors[D_NORTHEAST] = ar->GetRegion(r->xloc + 1, r->yloc - 1);
		r->neighbors[D_NORTHWEST] = ar->GetRegion(r->xloc - 1, r->yloc - 1);
	}
	if (r->yloc != ar->y - 1) {
		r->neighbors[D_SOUTHEAST] = ar->GetRegion(r->xloc + 1, r->yloc + 1);
		r->neighbors[D_SOUTHWEST] = ar->GetRegion(r->xloc - 1, r->yloc + 1);
	}
	if (r->yloc != ar->y - 1 && r->yloc != ar->y - 2) {
		r->neighbors[D_SOUTH] = ar->GetRegion(r->xloc, r->yloc + 2);
	}
}

void ARegionList::CreateAbyssLevel(int level, char *name)
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

	int tempx, tempy;
	if(Globals->GATES_EXIST) {
		int gateset = 0;
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
	}

	FinalSetup(pRegionArrays[level]);

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


void ARegionList::CreateNexusLevel(int level, int xSize, int ySize, char *name)
{
	MakeRegions(level, xSize, ySize);

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_NEXUS;

	AString nex_name = Globals->WORLD_NAME;
	nex_name += " Nexus";

	int x, y;
	for(y = 0; y < ySize; y++) {
		for(x = 0; x < xSize; x++) {
			ARegion *reg = pRegionArrays[level]->GetRegion(x, y);
			if(reg) {
				reg->SetName(nex_name.Str());
				reg->type = R_NEXUS;
			}
		}
	}

	FinalSetup(pRegionArrays[level]);

	for(y = 0; y < ySize; y++) {
		for(int x = 0; x < xSize; x++) {
			ARegion *reg = pRegionArrays[level]->GetRegion(x, y);
			if(reg && Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
				reg->MakeStartingCity();
				if(Globals->GATES_EXIST) {
					numberofgates++;
				}
			}
		}
	}
}

void ARegionList::CreateSurfaceLevel(int level, int xSize, int ySize, char *name)
{
	MakeRegions(level, xSize, ySize);

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;
	int sea = Globals->OCEAN;
	if(Globals->LAKES_EXIST || Globals->ARCHIPELAGO)
		sea = sea * (100 + 12 * (100 - Globals->ARCHIPELAGO) / 100) / 100;
	MakeLand(pRegionArrays[level], sea, Globals->CONTINENT_SIZE);

	CleanUpWater(pRegionArrays[level]);

	SetupAnchors(pRegionArrays[level]);

	GrowTerrain(pRegionArrays[level], 0);

	AssignTypes(pRegionArrays[level]);

	SeverLandBridges(pRegionArrays[level]);

	if (Globals->LAKES_EXIST) RemoveCoastalLakes(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateIslandLevel(int level, int nPlayers, char *name)
{
	int xSize, ySize;
	xSize = 20 + (nPlayers + 3) / 4 * 6 - 2;
	ySize = xSize;

	MakeRegions(level, xSize, ySize);

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;

	MakeCentralLand(pRegionArrays[level]);
	MakeIslands(pRegionArrays[level], nPlayers);
	RandomTerrain(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateUnderworldLevel(int level, int xSize, int ySize,
		char *name)
{
	MakeRegions(level, xSize, ySize);

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERWORLD;

	SetRegTypes(pRegionArrays[level], R_NUM);

	SetupAnchors(pRegionArrays[level]);

	GrowTerrain(pRegionArrays[level], 1);

	AssignTypes(pRegionArrays[level]);

	MakeUWMaze(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateUnderdeepLevel(int level, int xSize, int ySize,
		char *name)
{
	MakeRegions(level, xSize, ySize);

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERDEEP;

	SetRegTypes(pRegionArrays[level], R_NUM);

	SetupAnchors(pRegionArrays[level]);

	GrowTerrain(pRegionArrays[level], 1);

	AssignTypes(pRegionArrays[level]);

	MakeUWMaze(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::MakeRegions(int level, int xSize, int ySize)
{
	Awrite("Making a level...");

	ARegionArray *arr = new ARegionArray(xSize, ySize);
	pRegionArrays[level] = arr;

	//
	// Make the regions themselves
	//
	int x, y;
	for(y = 0; y < ySize; y++) {
		for(x = 0; x < xSize; x++) {
			if(!((x + y) % 2)) {
				ARegion *reg = new ARegion;
				reg->SetLoc(x, y, level);
				reg->num = Num();

				//
				// Some initial values; these will get reset
				//
				reg->type = -1;
				reg->race = -1;
				reg->wages = -1;
				reg->maxwages = -1;
				reg->population = -1;

				Add(reg);
				arr->SetRegion(x, y, reg);
			}
		}
	}

	SetupNeighbors(arr);

	Awrite("");
}

void ARegionList::SetupNeighbors(ARegionArray *pRegs)
{
	int x, y;
	for(x = 0; x < pRegs->x; x++) {
		for(y = 0; y < pRegs->y; y++) {
			ARegion *reg = pRegs->GetRegion(x, y);
			if(!reg) continue;
			NeighSetup(reg, pRegs);
		}
	}
}

void ARegionList::MakeLand(ARegionArray *pRegs, int percentOcean,
		int continentSize)
{
	int total = pRegs->x * pRegs->y / 2;
	int ocean = total;

	Awrite("Making land");
	while (ocean > (total * percentOcean) / 100) {
		int sz = getrandom(continentSize);
		sz = sz * sz;

		int tempx = getrandom(pRegs->x);
		int yoff = pRegs->y / 40;
		int yband = pRegs->y / 2 - 2 * yoff;
		int tempy = (getrandom(yband)+yoff) * 2 + tempx % 2;

		ARegion *reg = pRegs->GetRegion(tempx, tempy);
		ARegion *newreg = reg;
		ARegion *seareg = reg;

		// Archipelago or Continent?
		if (getrandom(100) < Globals->ARCHIPELAGO) {
			// Make an Archipelago:
			sz = sz / 5 + 1;
			int first = 1;
			int tries = 0;
			for (int i=0; i<sz; i++) {
				int direc = getrandom(NDIRS);
				newreg = reg->neighbors[direc];
				while (!newreg) {
					direc = getrandom(6);
					newreg = reg->neighbors[direc];
				}
				tries++;
				for (int m = 0; m < 2; m++) {
					seareg = newreg;
					newreg = seareg->neighbors[direc];
					if (!newreg) break;
				}
				if (!newreg) break;
				if (newreg) {
					seareg = newreg;
					newreg = seareg->neighbors[getrandom(NDIRS)];
					if (!newreg) break;
					// island start point (~3 regions away from last island)
					seareg = newreg;
					if (first) {
						seareg = reg;
						first = 0;
					}
					if (seareg->type == -1) {
						reg = seareg;
						tries = 0;
						reg->type = R_NUM;
						ocean--;
					} else {
						if (tries > 5) break;
						continue;
					}
					int growit = getrandom(20);
					int growth = 0;
					int growch = 2;
					// grow this island
					while (growit > growch) {
						growit = getrandom(20);
						tries = 0;
						int newdir = getrandom(NDIRS);
						while (newdir == reg->GetRealDirComp(direc))
							newdir = getrandom(NDIRS);
						newreg = reg->neighbors[newdir];
						while ((!newreg) && (tries < 36)) {
							while (newdir == reg->GetRealDirComp(direc))
								newdir = getrandom(NDIRS);
							newreg = reg->neighbors[newdir];
							tries++;
						}
						if (!newreg) continue;
						reg = newreg;
						tries = 0;
						if (reg->type == -1) {
							reg->type = R_NUM;
							growth++;
							if (growth > growch) growch = growth;
							ocean--;
						} else continue;
					}
				}
			}
		} else {
			// make a continent
			if (reg->type == -1) {
				reg->type = R_NUM;
				ocean--;
			}
			for (int i=0; i<sz; i++) {
				int dir = getrandom(NDIRS);
				if ((reg->yloc < yoff*2) && ((dir < 2) || (dir = NDIRS-1))
					&& (getrandom(4) < 3)) continue;
				if ((reg->yloc > (yband+yoff)*2) && ((dir < 5) && (dir > 1))
					&& (getrandom(4) < 3)) continue;				
				ARegion *newreg = reg->neighbors[dir];
				if (!newreg) break;
				int polecheck = 0;
				for (int v=0; v < NDIRS; v++) {
					ARegion *creg = newreg->neighbors[v];
					if (!creg) polecheck = 1;
				}
				if(polecheck) break;
				reg = newreg;
				if (reg->type == -1) {
					reg->type = R_NUM;
					ocean--;
				}
			}
		}
	}

	// At this point, go back through and set all the rest to ocean
	SetRegTypes(pRegs, R_OCEAN);
	Awrite("");
}

void ARegionList::MakeCentralLand(ARegionArray *pRegs)
{
	for(int i = 0; i < pRegs->x; i++) {
		for(int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if(!reg) continue;
			// Initialize region to ocean.
			reg->type = R_OCEAN;
			// If the region is close to the edges, it stays ocean
			if(i < 8 || i >= pRegs->x - 8 || j < 8 || j >= pRegs->y - 8)
				continue;
			// If the region is within 10 of the edges, it has a 50%
			// chance of staying ocean.
			if(i < 10 || i >= pRegs->x - 10 || j < 10 || j >= pRegs->y - 10) {
				if(getrandom(100) > 50) continue;
			}

			// Otherwise, set the region to land.
			reg->type = R_NUM;
		}
	}
}

void ARegionList::MakeIslands(ARegionArray *pArr, int nPlayers)
{
	// First, make the islands along the top.
	int i;
	int nRow = (nPlayers + 3) / 4;
	for(i = 0; i < nRow; i++)
		MakeOneIsland(pArr, 10 + i * 6, 2);
	// Next, along the left.
	nRow = (nPlayers + 2) / 4;
	for(i = 0; i < nRow; i++)
		MakeOneIsland(pArr, 2, 10 + i * 6);
	// The islands along the bottom.
	nRow = (nPlayers + 1) / 4;
	for(i = 0; i < nRow; i++)
		MakeOneIsland(pArr, 10 + i * 6, pArr->y - 6);
	// And the islands on the right.
	nRow = nPlayers / 4;
	for(i = 0; i < nRow; i++)
		MakeOneIsland(pArr, pArr->x - 6, 10 + i * 6);
}

void ARegionList::MakeOneIsland(ARegionArray *pRegs, int xx, int yy)
{
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			ARegion *pReg = pRegs->GetRegion(i + xx, j + yy);
			if(!pReg) continue;
			pReg->type = R_NUM;
		}
	}
}

void ARegionList::CleanUpWater(ARegionArray *pRegs)
{
	Awrite("Converting Scattered Water");
	for (int ctr = 0; ctr < Globals->SEA_LIMIT+1; ctr++) {
		for(int i = 0; i < pRegs->x; i++) {
			for(int j = 0; j < pRegs->y; j++) {
				ARegion *reg = pRegs->GetRegion(i, j);
				int remainocean = 0;
				if((!reg) || (reg->type != R_OCEAN)) continue;
				for (int d = 0; d < NDIRS; d++) {
					remainocean += reg->CheckSea(d, Globals->SEA_LIMIT, remainocean);
				}
				if (remainocean > 0) continue;
				reg->wages = 0;
				if (getrandom(100) < Globals->LAKES_EXIST) {
					reg->type = R_LAKE;
				} else reg->type = R_NUM;
				Adot();
			}
		}
	}
	Awrite("");
}

void ARegionList::RemoveCoastalLakes(ARegionArray *pRegs)
{
	Awrite("Removing coastal 'lakes'");
	for (int c = 0; c < 2; c++) {
		for(int i = 0; i < pRegs->x; i++) {
			for(int j = 0; j < pRegs->y; j++) {
				ARegion *reg = pRegs->GetRegion(i, j);
				if ((!reg) || (reg->type != R_LAKE)) continue;
				if (reg->IsCoastal() > 0) {
					reg->type = R_OCEAN;
					reg->wages = -1;
					Adot();
				} else if (reg->wages <= 0) { // name the Lake
					int wage1 = 0;
					int count1 = 0;
					int wage2 = 0;
					int count2 = 0;
					int temp = 0;
					for (int d = 0; d < NDIRS; d++) {
						ARegion *newregion = reg->neighbors[d];
						if (!newregion) continue;
						// name after neighboring lake regions preferrentially
						if ((newregion->wages > 0) &&
								(newregion->type == R_LAKE)) {
							count1 = 1;
							wage1 = newregion->wages;
							break;
						}
						if ((TerrainDefs[newregion->type].similar_type !=
									R_OCEAN) && (newregion->wages > -1)) {
							if (newregion->wages == wage1) count1++;
							else if (newregion->wages == wage2) count2++;
							else if (count2 == 0) {
								wage2 = newregion->wages;
								count2++;
							}
							if (count2 > count1) {
								temp = wage1;
								wage1 = wage2;
								wage2 = temp;
								int tmpin = count1;
								count1 = count2;
								count2 = tmpin;
							}
						}
					}
					if (count1 > 0) reg->wages = wage1;
				}
			}
		}
	}
	Awrite("");
}

void ARegionList::SeverLandBridges(ARegionArray *pRegs)
{
	Awrite("Severing land bridges");
	// mark land hexes to delete
	for(int i = 0; i < pRegs->x; i++) {
		for(int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if ((!reg) || (TerrainDefs[reg->type].similar_type == R_OCEAN))
				continue;
			if (reg->IsCoastal() != 4) continue;
			int tidych = Globals->SEVER_LAND_BRIDGES;
			for (int d = 0; d < NDIRS; d++) {
				ARegion *newregion = reg->neighbors[d];
				if ((!newregion) ||
						(TerrainDefs[newregion->type].similar_type == R_OCEAN))
					continue;
				if (newregion->IsCoastal() == 4) tidych = tidych * 2;
			}
			if (getrandom(100) < (tidych)) reg->wages = -2;
		}
	}
	// now change to ocean
	for(int i = 0; i < pRegs->x; i++) {
		for(int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if ((!reg) || (reg->wages > -2)) continue;
			reg->type = R_OCEAN;
			reg->wages = -1;
			Adot();
		}
	}
	Awrite("");
}

void ARegionList::SetRegTypes(ARegionArray *pRegs, int newType)
{
	for(int i = 0; i < pRegs->x; i++) {
		for(int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if(!reg) continue;
			if(reg->type == -1) reg->type = newType;
		}
	}
}

void ARegionList::SetupAnchors(ARegionArray *ta)
{
	// Now, setup the anchors
	Awrite("Setting up the anchors");
	int skip = 250;
	int f = 2;
	if(Globals->TERRAIN_GRANULARITY) {
		skip = Globals->TERRAIN_GRANULARITY;
		while (skip > 5) {
			f++;
			skip -= 5;  // yes, that's intended!
			if (skip < 1) skip = 1;
		}
		skip = 100 * ((skip+3) * f + 2) / (skip + f - 2);
	}
	for (int x=0; x<(ta->x)/f; x++) {
		for (int y=0; y<(ta->y)/(f*2); y++) {
			if(getrandom(1000) > skip) continue;
			ARegion *reg = 0;
			for (int i=0; i<4; i++) {
				int tempx = x * f + getrandom(f);
				int tempy = y * f * 2 + getrandom(f)*2 + tempx%2;
				reg = ta->GetRegion(tempx, tempy);
				if (reg->type == R_NUM) {
					reg->type = GetRegType(reg);
					reg->population = 1;
					if (TerrainDefs[reg->type].similar_type != R_OCEAN)
						reg->wages = AGetName(0);
					break;
				}
			}
			Adot();
		}
	}

	Awrite("");
}

void ARegionList::GrowTerrain(ARegionArray *pArr, int growOcean)
{
	for (int j=0; j<30; j++) {
		int x, y;
		for(x = 0; x < pArr->x; x++) {
			for(y = 0; y < pArr->y; y++) {
				ARegion *reg = pArr->GetRegion(x, y);
				if(!reg) continue;
				if ((j > 0) && (j < 21) && (getrandom(3) < 2)) continue;
				if (reg->type == R_NUM) {
					// Check for Lakes
					if (Globals->LAKES_EXIST &&
							(getrandom(100) < (Globals->LAKES_EXIST/10 + 1))) {
						reg->type = R_LAKE;
						break;
					}
					// Check for Odd Terrain
					if (getrandom(1000) < Globals->ODD_TERRAIN) {
						reg->type = GetRegType(reg);
						if (TerrainDefs[reg->type].similar_type != R_OCEAN)
							reg->wages = AGetName(0);
						break;
					}

					int init = getrandom(6);
					for (int i=0; i<NDIRS; i++) {
						ARegion *t = reg->neighbors[(i+init) % NDIRS];
						if (t) {
							if ((j==0) && (t->population < 1)) continue;
							if (j==0) t->population--;
							if(t->type != R_NUM &&
								(TerrainDefs[t->type].similar_type!=R_OCEAN ||
								 (growOcean && (t->type != R_LAKE)))) {
								reg->race = t->type;
								reg->wages = t->wages;
								break;
							}
						}
					}
				}
			}
		}

		for(x = 0; x < pArr->x; x++) {
			for(y = 0; y < pArr->y; y++) {
				ARegion *reg = pArr->GetRegion(x, y);
				if(!reg) continue;
				if(reg->type == R_NUM && reg->race != -1)
					reg->type = reg->race;
			}
		}
	}
}

void ARegionList::RandomTerrain(ARegionArray *pArr)
{
	int x, y;
	for(x = 0; x < pArr->x; x++) {
		for(y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if(!reg) continue;

			if (reg->type == R_NUM) {
				int adjtype = 0;
				int adjname = -1;
				for (int d = 0; d < NDIRS; d++) {
					ARegion *newregion = reg->neighbors[d];
					if (!newregion) continue;
					if ((TerrainDefs[newregion->type].similar_type !=
								R_OCEAN) && (newregion->type != R_NUM) &&
							(newregion->wages > 0)) {
						adjtype = newregion->type;
						adjname = newregion->wages;
					}
				}
				if (adjtype && !Globals->CONQUEST_GAME) {
					reg->type = adjtype;
					reg->wages = adjname;
				} else {
					reg->type = GetRegType(reg);
					reg->wages = AGetName(0);
				}
			}
		}
	}
}

void ARegionList::MakeUWMaze(ARegionArray *pArr)
{
	int x, y;

	for(x = 0; x < pArr->x; x++) {
		for(y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if(!reg) continue;

			for (int i=D_SOUTHEAST; i<= D_SOUTHWEST; i++) {
				int count = 0;
				for(int j=D_NORTH; j< NDIRS; j++)
					if(reg->neighbors[j]) count++;
				if(count <= 1) break;

				ARegion *n = reg->neighbors[i];
				if (n) {
					if(!CheckRegionExit(reg, n)) {
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

void ARegionList::AssignTypes(ARegionArray *pArr)
{
	// RandomTerrain() will set all of the un-set region types and names.
	RandomTerrain(pArr);
}

void ARegionList::UnsetRace(ARegionArray *pArr)
{
	int x, y;
	for(x = 0; x < pArr->x; x++) {
		for(y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if(!reg) continue;
			reg->race = - 1;
		}
	}
}


void ARegionList::RaceAnchors(ARegionArray *pArr)
{
	UnsetRace(pArr);
	int x, y;
	for(x = 0; x < pArr->x; x++) {
		for(y = 0; y < pArr->y; y++) {
			// Anchor distribution: depends on GROW_RACES value
			int jiggle = 4 + 2 * Globals->GROW_RACES;
			if((y + ((x % 2) * jiggle/2)) % jiggle > 1) continue;
			int xoff = x + 2 - getrandom(3) - getrandom(3);
			ARegion *reg = pArr->GetRegion(xoff, y);
			if(!reg) continue;

			if((reg->type == R_LAKE) && (!Globals->LAKESIDE_IS_COASTAL))
				continue;

			reg->race = -1;

			if(TerrainDefs[reg->type].similar_type == R_OCEAN) {
				// setup near coastal race here
				int d = getrandom(NDIRS);
				int ctr = 0;
				ARegion *nreg = reg->neighbors[d];
				if(!nreg) continue;
				while((ctr++ < 20) && (reg->race == -1)) {
					if(TerrainDefs[nreg->type].similar_type != R_OCEAN) {
						int rnum =
							sizeof(TerrainDefs[nreg->type].coastal_races) /
							sizeof(int);
						reg->race = TerrainDefs[nreg->type].coastal_races[getrandom(rnum)];
					} else {
						int dir = getrandom(NDIRS);
						if(dir == nreg->GetRealDirComp(d)) continue;
						if(!(nreg->neighbors[dir])) continue;
						nreg = nreg->neighbors[dir];
					}
				}
			} else {
				// setup noncoastal race here
				int rnum = sizeof(TerrainDefs[reg->type].races)/sizeof(int);
				reg->race = TerrainDefs[reg->type].races[getrandom(rnum)];
			}
		}
	}
}

void ARegionList::GrowRaces(ARegionArray *pArr)
{
	RaceAnchors(pArr);
	int a, x, y;
	for(a = 0; a < 25; a++) {
		for(x = 0; x < pArr->x; x++) {
			for(y = 0; y < pArr->y; y++) {
				ARegion *reg = pArr->GetRegion(x, y);
				if((!reg) || (reg->race == -1)) continue;

				for(int dir = 0; dir < NDIRS; dir++) {
					ARegion *nreg = reg->neighbors[dir];
					if ((!nreg) || (nreg->race != -1)) continue;
					int iscoastal = 0;
					int cnum = sizeof(TerrainDefs[reg->type].coastal_races) /
						sizeof(int);
					for(int i=0; i<cnum; i++) {
						if (reg->race ==
								TerrainDefs[reg->type].coastal_races[i])
							iscoastal = 1;
					}
					// Only coastal races may pass from sea to land
					if ((TerrainDefs[nreg->type].similar_type == R_OCEAN) &&
							(!iscoastal))
						continue;

					int ch = getrandom(5);
					if (iscoastal) {
						if (TerrainDefs[nreg->type].similar_type == R_OCEAN)
							ch += 2;
					} else {
						ManType *mt = FindRace(ItemDefs[reg->race].abr);
						if (mt->terrain==TerrainDefs[nreg->type].similar_type)
							ch += 2;
						int rnum = sizeof(TerrainDefs[nreg->type].races) /
							sizeof(int);
						for(int i=0; i<rnum; i++) {
							if (TerrainDefs[nreg->type].races[i] == reg->race)
								ch++;
						}
					}
					if (ch > 3) nreg->race = reg->race;
				}
			}
		}
	}
}

void ARegionList::FinalSetup(ARegionArray *pArr)
{
	int x, y;
	for(x = 0; x < pArr->x; x++) {
		for(y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if(!reg) continue;

			if ((TerrainDefs[reg->type].similar_type == R_OCEAN) &&
					(reg->type != R_LAKE)) {
				if(pArr->levelType == ARegionArray::LEVEL_UNDERWORLD)
					reg->SetName("The Undersea");
				else if(pArr->levelType == ARegionArray::LEVEL_UNDERDEEP)
					reg->SetName("The Deep Undersea");
				else {
					AString ocean_name = Globals->WORLD_NAME;
					ocean_name += " Ocean";
					reg->SetName(ocean_name.Str());
				}
			} else {
				if (reg->wages == -1) reg->SetName("Unnamed");
				else if(reg->wages != -2)
					reg->SetName(AGetNameString(reg->wages));
				else
					reg->wages = -1;
			}

			reg->Setup();
		}
	}
}

void ARegionList::MakeShaft(ARegion *reg, ARegionArray *pFrom,
		ARegionArray *pTo)
{
	if(TerrainDefs[reg->type].similar_type == R_OCEAN) return;

	int tempx = reg->xloc * pTo->x / pFrom->x +
		getrandom(pTo->x / pFrom->x);
	int tempy = reg->yloc * pTo->y / pFrom->y +
		getrandom(pTo->y / pFrom->y);
	//
	// Make sure we get a valid region.
	//
	tempy += (tempx + tempy) % 2;

	ARegion *temp = pTo->GetRegion(tempx, tempy);
	if(TerrainDefs[temp->type].similar_type == R_OCEAN) return;

	Object *o = new Object(reg);
	o->num = reg->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = temp->num;
	reg->objects.Add(o);

	o = new Object(reg);
	o->num = temp->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = reg->num;
	temp->objects.Add(o);
}

void ARegionList::MakeShaftLinks(int levelFrom, int levelTo, int odds)
{
	ARegionArray *pFrom = pRegionArrays[levelFrom];
	ARegionArray *pTo = pRegionArrays[levelTo];

	int x, y;
	for(x = 0; x < pFrom->x; x++) {
		for(y = 0; y < pFrom->y; y++) {
			ARegion *reg = pFrom->GetRegion(x, y);
			if(!reg) continue;

			if(getrandom(odds) != 0) continue;

			MakeShaft(reg, pFrom, pTo);
		}
	}
}

void ARegionList::CalcDensities()
{
	Awrite("Densities:");
	int arr[R_NUM];
	int i;
	for (i=0; i<R_NUM; i++)
		arr[i] = 0;
	forlist(this) {
		ARegion *reg = ((ARegion *) elem);
		arr[reg->type]++;
	}
	for (i=0; i<R_NUM; i++)
		if(arr[i]) Awrite(AString(TerrainDefs[i].name) + " " + arr[i]);

	Awrite("");
}

void ARegionList::SetACNeighbors(int levelSrc, int levelTo, int maxX, int maxY)
{
	ARegionArray *ar = GetRegionArray(levelSrc);

	for(int x = 0; x < ar->x; x++) {
		for(int y = 0; y < ar->y; y++) {
			ARegion *AC = ar->GetRegion(x, y);
			if(!AC) continue;
			for (int i=0; i<NDIRS; i++) {
				if(AC->neighbors[i]) continue;
				ARegion *pReg = GetStartingCity(AC, i, levelTo, maxX, maxY);
				if(!pReg) continue;
				AC->neighbors[i] = pReg;
				pReg->MakeStartingCity();
				if(Globals->GATES_EXIST) {
					numberofgates++;
				}
			}
		}
	}
}

void ARegionList::InitSetupGates(int level)
{

	if(!Globals->GATES_EXIST) return;

	ARegionArray *pArr = pRegionArrays[level];

	int i, j, k;
	for (i=0; i<pArr->x / 8; i++) {
		for (j=0; j<pArr->y / 16; j++) {
			for (k=0; k<5; k++) {
				int tempx = i*8 + getrandom(8);
				int tempy = j*16 + getrandom(8)*2 + tempx%2;
				ARegion *temp = pArr->GetRegion(tempx, tempy);
				if (TerrainDefs[temp->type].similar_type != R_OCEAN &&
						temp->gate != -1) {
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
	if(!Globals->GATES_EXIST) return;

	int *used = new int[numberofgates];

	int i;
	for (i=0; i<numberofgates; i++) used[i] = 0;

	forlist(this) {
		ARegion *r = (ARegion *) elem;

		if (r->gate == -1) {
			int index = getrandom(numberofgates);
			while (used[index]) {
				index++;
				index = index % numberofgates;
			}
			r->gate = index+1;
			used[index] = 1;
			// setting up gatemonth
			int nmon = (getrandom(3) - 1) + (getrandom(3) - 1) + ((index+1) % 12);
			if (nmon > 11) nmon = nmon - 12;
			if (nmon < 0) nmon = nmon + 12;
			r->gatemonth = nmon;
		}
	}
	delete used;
}

ARegion *ARegionList::FindGate(int x)
{
	if (!x) return 0;
	forlist(this) {
		ARegion *r = (ARegion *) elem;
		if (r->gate == x) return r;
	}
	return 0;
}

int ARegionList::GetPlanarDistance(ARegion *one, ARegion *two, int penalty)
{
	if(one->zloc == ARegionArray::LEVEL_NEXUS ||
			two->zloc == ARegionArray::LEVEL_NEXUS)
		return 10000000;

	if (Globals->ABYSS_LEVEL) {
		// make sure you cannot teleport into or from the abyss
		int ablevel = Globals->UNDERWORLD_LEVELS +
			Globals->UNDERDEEP_LEVELS + 2;
		if (one->zloc == ablevel || two->zloc == ablevel)
			return 10000000;
	}

	int one_x, one_y, two_x, two_y;
	int maxy;
	ARegionArray *pArr=pRegionArrays[ARegionArray::LEVEL_SURFACE];

	one_x = one->xloc * GetLevelXScale(one->zloc);
	one_y = one->yloc * GetLevelYScale(one->zloc);

	two_x = two->xloc * GetLevelXScale(two->zloc);
	two_y = two->yloc * GetLevelYScale(two->zloc);

	maxy = one_y - two_y;
	if(maxy < 0) maxy=-maxy;

	int maxx = one_x - two_x;
	if(maxx < 0) maxx = -maxx;

	int max2 = one_x + pArr->x - two_x;
	if(max2 < 0) max2 = -max2;
	if(max2 < maxx) maxx = max2;

	max2 = one_x - (two_x + pArr->x);
	if(max2 < 0) max2 = -max2;
	if(max2 < maxx) maxx = max2;

	if(maxy > maxx) maxx = (maxx+maxy)/2;

	if(one->zloc != two->zloc) {
		int zdist = (one->zloc - two->zloc);
		if ((two->zloc - one->zloc) > zdist)
			zdist = two->zloc - one->zloc;
		maxx += (penalty * zdist);
	}

	return maxx;
}

int ARegionList::GetDistance(ARegion *one, ARegion *two)
{
	if(one->zloc != two->zloc) return(10000000);

	ARegionArray *pArr = pRegionArrays[one->zloc];

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

ARegionArray *ARegionList::GetRegionArray(int level)
{
	return(pRegionArrays[level]);
}

void ARegionList::CreateLevels(int n)
{
	numLevels = n;
	pRegionArrays = new ARegionArray *[n];
}

ARegionArray::ARegionArray(int xx, int yy)
{
	x = xx;
	y = yy;
	regions = new ARegion *[x * y / 2 + 1];
	strName = 0;

	int i;
	for(i = 0; i < x * y / 2; i++) regions[i] = 0;
}

ARegionArray::~ARegionArray()
{
	if (strName) delete strName;
	delete [] regions;
}

void ARegionArray::SetRegion(int xx, int yy, ARegion *r)
{
	regions[xx / 2 + yy * x / 2] = r;
}

ARegion *ARegionArray::GetRegion(int xx, int yy)
{
	xx = (xx + x) % x;
	yy = (yy + y) % y;
	if((xx + yy) % 2) return(0);
	return(regions[xx / 2 + yy * x / 2]);
}

void ARegionArray::SetName(char *name)
{
	if(name) {
		strName = new AString(name);
	} else {
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

void ARegionFlatArray::SetRegion(int x, ARegion *r) {
	regions[x] = r;
}

ARegion *ARegionFlatArray::GetRegion(int x) {
	return regions[x];
}
