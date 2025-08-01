#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include "game.h"
#include "gamedata.h"

/* TownType */
// Depends on population and town development
int TownInfo::TownType()
{
    int prestige = pop * (dev + 220) / 270;
    if (prestige < Globals->CITY_POP / 4) return TOWN_VILLAGE;
    if (prestige < Globals->CITY_POP * 4 / 5) return TOWN_TOWN;
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

// IMPORTANT: wages now represent fractional wages, that
// means the value of the wages variable is 10 times the silver value
int ARegion::Wages()
{
    // Calculate new wages
    wages = 0;
    if (Population() == 0) return 0;
    int level = 1;
    int last = 0;
    int dv = development + RoadDevelopment() + (earthlore + clearskies) * 12;
    // Note: earthlore and clearskies represent LEVEL of the spell
    // Adjust for TownType
    if (town) {
        int tsize = town->TownType();
        dv += (tsize * tsize + 1);
    }
    while (dv >= level) {
        wages++;
        last = level;
        level += wages+1;
    }
    wages *= 10;
    if (dv > last)
        wages += 10 * (dv - last) / (level - last);
    return wages;
}

std::string ARegion::wages_for_report()
{
    Production *p = get_production_for_skill(I_SILVER, -1);
    if (p) {
        return "$" + std::to_string(p->productivity / 10) +
            "." + std::to_string(p->productivity % 10) + " (Max: $" + std::to_string(p->amount) + ")";
    } else
        return "$" + std::to_string(0);
}

void ARegion::SetupHabitat(TerrainType* terrain) {
    if (habitat > 1) habitat *= 5;
    if ((habitat < 100) && (terrain->similar_type != R_OCEAN)) habitat = 100;

    int pop = terrain->pop;
    int mw = terrain->wages;

    // fix economy when MAINTENANCE_COST has been adjusted
    mw += Globals->MAINTENANCE_COST - 10;
    if (mw < 0) mw = 0;

    if (pop == 0) {
        population = 0;
        basepopulation = 0;
        wages = 0;
        maxwages = 0;
        wealth = 0;
        return;
    }

    // Only select race here if it hasn't been set during Race Growth
    // in the World Creation process.
    if ((race == -1) || (!Globals->GROW_RACES)) {
        int noncoastalraces = sizeof(terrain->races) / sizeof(terrain->races[0]);
        int allraces =
            noncoastalraces + sizeof(terrain->coastal_races) / sizeof(terrain->coastal_races[0]);

        race = -1;
        while (race == -1 || (ItemDefs[race].flags & ItemType::DISABLED)) {
            int n = rng::get_random(IsCoastal() ? allraces : noncoastalraces);
            if (n > noncoastalraces-1) {
                race = terrain->coastal_races[n-noncoastalraces-1];
            } else
                race = terrain->races[n];
        }
    }

    habitat = habitat * 2 / 3 + rng::get_random(habitat / 3);
    auto mt = find_race(ItemDefs[race].abr)->get();
    if (mt.terrain == terrain->similar_type) {
        habitat = (habitat * 9)/8;
    }
    if (!IsNativeRace(race)) {
        habitat = (habitat * 4)/5;
    }
    basepopulation = habitat / 3;
    // hmm... somewhere not too far off equilibrium pop
    population = habitat * (60 + rng::get_random(6) + rng::get_random(6)) / 100;

    // Setup development
    int level = 1;
    development = 1;
    int prev = 0;
    while (level < mw) {
        development++;
        prev++;
        if (prev > level) {
            level++;
            prev = 0;
        }
    }
    development += rng::get_random(25);
    maxdevelopment = development;
}

void ARegion::SetupEconomy() {
    /* Setup basic economy */
    maxwages = Wages();

    /* taxable region wealth */
    wealth = (int) ((float) (Population()
        * (Wages() - 10 * Globals->MAINTENANCE_COST) / 50));
    if (wealth < 0) wealth = 0;

    // wage-relevant population (/10 wages /5 popfactor)
    int pp = Population();
    // adjustment for rural areas
    if (pp < 3000) {
        float wpfactor = (float) (120 / (61 - pp / 50));
        pp += (int) ((float) ((wpfactor * pp + 3000)/(wpfactor + 1)));
    }
    int wagelimit = (int) ((float) (pp * (Wages() - 10 * Globals->MAINTENANCE_COST) /50));
    if (wagelimit < 0) wagelimit = 0;
    Production * w = new Production;
    w->itemtype = I_SILVER;
    w->amount = wagelimit / Globals->WORK_FRACTION;
    w->baseamount = wagelimit / Globals->WORK_FRACTION;
    w->skill = -1;
    w->productivity = wages;

    /* Entertainment - setup or adjust */
    int ep = Population();
    // adjustment for rural areas
    if (ep < 3000) {
        int epf = (ep / 10 + 300) / 6;
        ep = ep * epf / 100;
    }
    int maxent = (int) ((float) (ep * ((Wages() - 10 * Globals->MAINTENANCE_COST) + 1) /50));
    if (maxent < 0) maxent = 0;

    Production * e = new Production;
    e->itemtype = I_SILVER;
    e->skill = S_ENTERTAINMENT;
    e->amount = maxent / Globals->ENTERTAIN_FRACTION;

    e->baseamount = maxent / Globals->ENTERTAIN_FRACTION;
    // raise entertainment income by productivity factor 10
    e->productivity = Globals->ENTERTAIN_INCOME * 10;

    // note: wage factor 10, population factor 5 - included as "/ 50"
    /* More wealth in safe Starting Cities */
    if ((Globals->SAFE_START_CITIES) && (IsStartingCity())) {
        int wbonus = (Population() / 5) * Globals->MAINTENANCE_COST;
        wealth += wbonus;
        w->amount += wbonus / Globals->WORK_FRACTION;
        w->baseamount += wbonus / Globals->WORK_FRACTION;
        e->amount += wbonus / Globals->ENTERTAIN_FRACTION;
        e->baseamount += wbonus / Globals->ENTERTAIN_FRACTION;
    }
    products.push_back(w);
    products.push_back(e);

    float ratio = ItemDefs[race].baseprice / ((float)Globals->BASE_MAN_COST * 10);
    // hack: include wage factor of 10 in float assignment above
    // Setup Recruiting
    Market *m = new Market(
        Market::MarketType::M_BUY, race, (int)(Wages() * 4 * ratio), Population() / 25, 0, 10000, 0, 2000
    );
    markets.push_back(m);

    if (Globals->LEADERS_EXIST) {
        ratio = ItemDefs[I_LEADERS].baseprice / ((float)Globals->BASE_MAN_COST * 10);
        // hack: include wage factor of 10 in float assignment above
        m = new Market(
            Market::MarketType::M_BUY, I_LEADERS, (int)(Wages() * 4 * ratio), Population() / 125, 0, 10000, 0, 400
        );
        markets.push_back(m);
    }
}

void ARegion::SetupPop()
{
    TerrainType *typer = &(TerrainDefs[type]);
    habitat = typer->pop+1;

    SetupHabitat(typer);
    if (population == 0) {
        return;
    }

    if (Globals->TOWNS_EXIST) {
        int adjacent = 0;
        int prob = Globals->TOWN_PROBABILITY;
        if (prob < 1) prob = 100;
        int townch = (int) 80000 / prob;
        if (Globals->TOWNS_NOT_ADJACENT) {
                for (int d = 0; d < NDIRS; d++) {
                    ARegion *newregion = neighbors[d];
                    if ((newregion) && (newregion->town)) adjacent++;
                }
            }
        if (Globals->LESS_ARCTIC_TOWNS) {
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
        if (spread > 100) spread = 100;
        int townprob = (TerrainDefs[type].economy * 4 * (100 - spread) +
            100 * spread) / 100;
        if (adjacent > 0) townprob = townprob * (100 - Globals->TOWNS_NOT_ADJACENT) / 100;
        if (rng::get_random(townch) < townprob) add_town();
    }

    SetupEconomy();
}

/* ONE function where the wage and entertainment income
 * is set (previously this has been all over the place!) */
void ARegion::SetIncome()
{
    /* do nothing in unpopulated regions */
    if (basepopulation == 0) return;

    maxwages = Wages();

    /* taxable region wealth */
    wealth = (int) ((float) (Population()
        * (Wages() - 10 * Globals->MAINTENANCE_COST) / 50));
    if (wealth < 0) wealth = 0;

    /* Wages */
    // wage-relevant population (/10 wages /5 popfactor)
    int pp = Population();
    // adjustment for rural areas
    if (pp < 3000) {
        float wpfactor = (float) (120 / (61 - pp / 50));
        pp += (int) ((float) ((wpfactor * pp + 3000)/(wpfactor + 1)));
    }
    int maxwages = (int) ((float) (pp * (Wages() - 10 * Globals->MAINTENANCE_COST) /50));
    if (maxwages < 0) maxwages = 0;
    Production * w = get_production_for_skill(I_SILVER,-1);
    // In some cases (ie. after products.DeleteAll() in EditGameRegionTerrain)
    // I_SILVER is not in ProductionList
    if( !w ) {
      w = new Production;
      products.push_back(w);
    }
    w->itemtype = I_SILVER;
    w->amount = maxwages / Globals->WORK_FRACTION;
    w->baseamount = maxwages / Globals->WORK_FRACTION;
    w->skill = -1;
    w->productivity = wages;

    /* Entertainment - setup or adjust */
    int ep = Population();
    // adjustment for rural areas
    if (ep < 3000) {
        int epf = (ep / 10 + 300) / 6;
        ep = ep * epf / 100;
    }
    int maxent = (int) ((float) (ep * ((Wages() - 10 * Globals->MAINTENANCE_COST) + 10) /50));
    if (maxent < 0) maxent = 0;
    Production * e = get_production_for_skill(I_SILVER,S_ENTERTAINMENT);
    // In some cases (ie. after products.DeleteAll() in EditGameRegionTerrain)
    // I_SILVER is not in ProductionList
    if( !e ) {
      e = new Production;
      products.push_back(e);
    }
    e->itemtype = I_SILVER;
    e->amount = maxent / Globals->ENTERTAIN_FRACTION;
    e->baseamount = maxent / Globals->ENTERTAIN_FRACTION;
    e->skill = S_ENTERTAINMENT;
    // raise entertainment income by productivity factor 10
    e->productivity = Globals->ENTERTAIN_INCOME * 10;

    // note: wage factor 10, population factor 5 - included as "/ 50"
    /* More wealth in safe Starting Cities */
    if ((Globals->SAFE_START_CITIES) && (IsStartingCity())) {
        int wbonus = (Population() / 5) * Globals->MAINTENANCE_COST;
        wealth += wbonus;
        w->amount += wbonus / Globals->WORK_FRACTION;
        w->baseamount += wbonus / Globals->WORK_FRACTION;
        e->amount += wbonus / Globals->ENTERTAIN_FRACTION;
        e->baseamount += wbonus / Globals->ENTERTAIN_FRACTION;
    }
}

void ARegion::DisbandInRegion(int item, int amt)
{
    if (!Globals->DYNAMIC_POPULATION) return;
    if (amt > 0) {
        if (amt > Population()) {
            // exchange region race!
            race = item;
            population = 0;
            if (town) town->pop = 0;
            AdjustPop(amt);
        } else {
            if (race != item) amt = amt * 2 / 3;
            AdjustPop(amt);
        }
    }
}

void ARegion::Recruit(int amt)
{
    if (!Globals->DYNAMIC_POPULATION) return;
    AdjustPop(-amt);
}

void ARegion::AdjustPop(int adjustment)
{
    if (!town) {
        population += adjustment;
        return;
    }
    // split between town and rural pop
    int tspace = town->hab - town->pop;
    int rspace = habitat - population;

    // Region with zero room to
    if (tspace == 0 && rspace == 0) {
        return;
    }

    town->pop += adjustment * tspace / (tspace + rspace);
    if (town->pop < 0) town->pop = 0;
    population += adjustment * rspace / (tspace + rspace);
    if (population < 0) population = 0;
}

void ARegion::SetupCityMarket()
{
    int numtrade = 0;
    int cap;
    int offset = 0;
    int citymax = Globals->CITY_POP;
    auto localrace = find_race(ItemDefs[race].abr);
    if (!localrace) localrace = find_race("SELF");
    auto locals = localrace->get();
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
        if (ItemDefs[i].flags & ItemType::DISABLED) continue;
        if (ItemDefs[i].flags & ItemType::NOMARKET) continue;
        if (ItemDefs[i].type & IT_SHIP) continue;
        if (ItemDefs[i].type & IT_TRADE) numtrade++;
        if (i==I_SILVER) continue;
        if ((ItemDefs[i].type & IT_MAN)
            || (ItemDefs[i].type & IT_LEADER)) continue;

        int canProduceHere = 0;
        // Check if the product can be produced in the region
        // Raw goods
        if (ItemDefs[i].pInput[0].item == -1) {
            for (unsigned int c = 0;
                c<(sizeof(TerrainDefs[type].prods)/sizeof(TerrainDefs[type].prods[0]));
                c++) {
                int resource = TerrainDefs[type].prods[c].product;
                if (i == resource) {
                    canProduceHere = 1;
                    break;
                }
            }
        }
        // Non-raw goods
        else {
            canProduceHere = 1;
            for (unsigned int c = 0;
                c<(sizeof(ItemDefs[i].pInput)/sizeof(ItemDefs[i].pInput[0]));
                c++) {
                int match = 0;
                int need = ItemDefs[i].pInput[c].item;
                for (unsigned int r=0;
                    r<(sizeof(TerrainDefs[type].prods)/sizeof(TerrainDefs[type].prods[0]));
                    r++) {
                    if (TerrainDefs[type].prods[r].product == need)
                        match = 1;
                }
                if (!match) {
                    canProduceHere = 0;
                    break;
                }
            }
        }
        bool canProduce = false;
        // Check if the locals can produce this item
        if (canProduceHere) canProduce = locals.CanProduce(i);
        bool isUseful = locals.CanUse(i);
        //Normal Items
        if (ItemDefs[ i ].type & IT_NORMAL) {

            if (i==I_GRAIN || i==I_LIVESTOCK || i==I_FISH) {
                // Add foodstuffs directly to market
                int amt = Globals->CITY_MARKET_NORMAL_AMT;
                int price;

                if (Globals->RANDOM_ECONOMY) {
                    amt += rng::get_random(amt);
                    price = (ItemDefs[i].baseprice * (100 + rng::get_random(50))) / 100;
                } else {
                    price = ItemDefs[ i ].baseprice;
                }

                cap = (citymax * 3/4) - 5000;
                if (cap < 0) cap = citymax/2;
                Market * m = new Market(
                    Market::MarketType::M_SELL, i, price, amt, population,  population + cap, amt, amt * 2
                );
                markets.push_back(m);
            } else if (i == I_FOOD) {
                // Add foodstuffs directly to market
                int amt = Globals->CITY_MARKET_NORMAL_AMT;
                int price;
                if (Globals->RANDOM_ECONOMY) {
                    amt += rng::get_random(amt);
                    price = (ItemDefs[i].baseprice * (120 + rng::get_random(80))) /
                        100;
                } else {
                    price = ItemDefs[ i ].baseprice;
                }

                cap = (citymax * 3/4) - 5000;
                if (cap < 0) cap = citymax/2;
                Market * m = new Market(
                    Market::MarketType::M_BUY, i, price, amt, population, population + 2 * cap, amt, amt * 5
                );
                markets.push_back(m);
            } else if (ItemDefs[i].pInput[0].item == -1) {
                // Basic resource
                // Add to supply?
                if (canProduce) supply[i] = 4;
                // Add to demand?
                if (!canProduceHere) {
                    // Is it a mount?
                    if (ItemDefs[i].type & IT_MOUNT) {
                        if (locals.CanProduce(i)) demand[i] = 4;
                    } else if (isUseful) demand[i] = 4;
                }
            } else {

                // Tool, weapon or armor
                if (isUseful) {
                    // Add to supply?
                    if (canProduce) supply[i] = 2;
                    // Add to demand?
                    if (!canProduceHere) demand[i] = 2;
                }
            }
        } // end Normal Items
        // Advanced Items
        else if ((Globals->CITY_MARKET_ADVANCED_AMT)
            && (ItemDefs[i].type & IT_ADVANCED)) {
            if (isUseful) rare[i] = 4;
            if (ItemDefs[i].hitchItem > 0) rare[i] = 2;
        }
        // Magic Items
        else if ((Globals->CITY_MARKET_MAGIC_AMT)
            && (ItemDefs[i].type & IT_MAGIC)) {
            if (isUseful) antiques[i] = 4;
                else antiques[i] = 1;
            if (ItemDefs[i].hitchItem > 0) antiques[i] = 2;
        }
    }
    /* Check for advanced item */
    if ((Globals->CITY_MARKET_ADVANCED_AMT) && (rng::get_random(4) == 1)) {
        int ad = 0;
        for (int i=0; i<NITEMS; i++) ad += rare[i];
        ad = rng::get_random(ad);
        int i;
        int sum = 0;
        for (i=0; i<NITEMS; i++) {
            sum += rare[i];
            if (ad < sum) break;
        }
        if (ad < sum) {
            int amt = Globals->CITY_MARKET_ADVANCED_AMT;
            int price;
            if (Globals->RANDOM_ECONOMY) {
                amt += rng::get_random(amt);
                price = (ItemDefs[i].baseprice * (100 + rng::get_random(50))) / 100;
            } else {
                price = ItemDefs[ i ].baseprice;
            }

            cap = (citymax *3/4) - 5000;
            if (cap < citymax/2) cap = citymax / 2;
            offset = citymax / 8;
            if (cap+offset < citymax) {
                Market * m = new Market(
                    Market::MarketType::M_SELL, i, price, amt / 6, population + cap + offset,
                    population + citymax, 0, amt
                );
                markets.push_back(m);
            }
        }
    }
    /* Check for magic item */
    if ((Globals->CITY_MARKET_MAGIC_AMT) && (rng::get_random(8) == 1)) {
        int mg = 0;
        for (int i=0; i<NITEMS; i++) mg += antiques[i];
        mg = rng::get_random(mg);
        int i;
        int sum = 0;
        for (i=0; i<NITEMS; i++) {
            sum += antiques[i];
            if (mg < sum) break;
        }
        if (mg < sum) {
            int amt = Globals->CITY_MARKET_MAGIC_AMT;
            int price;

            if (Globals->RANDOM_ECONOMY) {
                amt += rng::get_random(amt);
                price = (ItemDefs[i].baseprice * (100 + rng::get_random(50))) / 100;
            } else {
                price = ItemDefs[ i ].baseprice;
            }

            cap = (citymax *3/4) - 5000;
            if (cap < citymax/2) cap = citymax / 2;
            offset = (citymax/20) + ((citymax/5) * 2);
            Market * m = new Market(
                Market::MarketType::M_SELL, i, price, amt / 6, population + cap, population + citymax, 0, amt
            );
            markets.push_back(m);
        }
    }

    /* Add demand (normal) items */
    int num = 4;
    int sum = 1;
    while((num > 0) && (sum > 0)) {
        int dm = 0;
        for (int i=0; i<NITEMS; i++) dm += demand[i];
        dm = rng::get_random(dm);
        int i;
        sum = 0;
        for (i=0; i<NITEMS; i++) {
            sum += demand[i];
            if (dm < sum) break;
        }
        if (dm >= sum) continue;

        int amt = Globals->CITY_MARKET_NORMAL_AMT;
        amt = demand[i] * amt / 4;
        int price;

        if (Globals->RANDOM_ECONOMY) {
            amt += rng::get_random(amt);
            price = (ItemDefs[i].baseprice *
                (100 + rng::get_random(50))) / 100;
        } else {
            price = ItemDefs[i].baseprice;
        }

        cap = (citymax/4);
        offset = - (citymax/20) + ((5-num) * citymax * 3/40);
        Market * m = new Market(
            Market::MarketType::M_SELL, i, price, amt / 6, population + cap + offset, population + citymax, 0, amt
        );
        markets.push_back(m);
        demand[i] = 0;
        num--;
    }

    /* Add supply (normal) items */
    num = 2;
    sum = 1;
    while((num > 0) && (sum > 0)) {
        int su = 0;
        for (int i=0; i<NITEMS; i++) su += supply[i];
        su = rng::get_random(su);
        int i;
        sum = 0;
        for (i=0; i<NITEMS; i++) {
            sum += supply[i];
            if (su < sum) break;
        }
        if (su >= sum) continue;

        int amt = Globals->CITY_MARKET_NORMAL_AMT;
        amt = supply[i] * amt / 4;
        int price;

        if (Globals->RANDOM_ECONOMY) {
            amt += rng::get_random(amt);
            price = (ItemDefs[i].baseprice *
                (150 + rng::get_random(50))) / 100;
        } else {
            price = ItemDefs[ i ].baseprice;
        }

        cap = (citymax/4);
        offset = ((3-num) * citymax * 3 / 40);
        if (supply[i] < 4) offset += citymax / 20;
        Market * m = new Market(
            Market::MarketType::M_BUY, i, price, 0, population + cap + offset, population + citymax, 0, amt
        );
        markets.push_back(m);
        supply[i] = 0;
        num--;
    }

    // If we don't have at least 4 trade items don't set up trade markets
    if (numtrade < 4) return;

    /* Set up the trade items */
    int buy1 = rng::get_random(numtrade);
    int buy2 = rng::get_random(numtrade);
    int sell1 = rng::get_random(numtrade);
    int sell2 = rng::get_random(numtrade);
    int tradebuy = 0;
    int tradesell = 0;
    offset = 0;
    cap = 0;

    buy1 = rng::get_random(numtrade);
    while (buy1 == buy2) buy2 = rng::get_random(numtrade);
    while (sell1 == buy1 || sell1 == buy2) sell1 = rng::get_random(numtrade);
    while (sell2 == sell1 || sell2 == buy2 || sell2 == buy1) sell2 = rng::get_random(numtrade);

    for (int i=0; i<NITEMS; i++) {
        if (ItemDefs[i].flags & ItemType::DISABLED) continue;
        if (ItemDefs[i].flags & ItemType::NOMARKET) continue;

        if (ItemDefs[ i ].type & IT_TRADE) {
            int addbuy = 0;
            int addsell = 0;

            if (buy1 == 0 || buy2 == 0) {
                addbuy = 1;
            }
            buy1--;
            buy2--;

            if (sell1 == 0 || sell2 == 0) {
                addsell = 1;
            }
            sell1--;
            sell2--;

            if (addbuy) {
                int amt = Globals->CITY_MARKET_TRADE_AMT;
                int price;

                if (Globals->RANDOM_ECONOMY) {
                    amt += rng::get_random(amt);
                    if (Globals->MORE_PROFITABLE_TRADE_GOODS) {
                        price=(ItemDefs[i].baseprice*(250+rng::get_random(100)))/100;
                    } else {
                        price=(ItemDefs[i].baseprice*(150+rng::get_random(50)))/100;
                    }
                } else {
                    price = ItemDefs[ i ].baseprice;
                }

                cap = (citymax/2);
                tradesell++;
                offset = - (citymax/20) + tradesell * (tradesell * tradesell * citymax/40);
                if (cap + offset < citymax) {
                    Market * m = new Market(
                        Market::MarketType::M_SELL, i, price, amt / 5, cap + population + offset,
                        citymax + population, 0, amt
                    );
                    markets.push_back(m);
                }
            }

            if (addsell) {
                int amt = Globals->CITY_MARKET_TRADE_AMT;
                int price;

                if (Globals->RANDOM_ECONOMY) {
                    amt += rng::get_random(amt);
                    if (Globals->MORE_PROFITABLE_TRADE_GOODS) {
                        price=(ItemDefs[i].baseprice*(100+rng::get_random(90)))/100;
                    } else {
                        price=(ItemDefs[i].baseprice*(100+rng::get_random(50)))/100;
                    }
                } else {
                    price = ItemDefs[ i ].baseprice;
                }

                cap = (citymax/2);
                offset = tradebuy++ * (citymax/6);
                if (cap+offset < citymax) {
                    Market * m = new Market(
                        Market::MarketType::M_BUY, i, price, amt / 6, cap + population + offset,
                        citymax + population, 0, amt
                    );
                    markets.push_back(m);
                }
            }
        }
    }
}

void ARegion::SetupProds(double weight)
{
    Production *p = NULL;
    TerrainType *typer = &(TerrainDefs[type]);

    if (Globals->FOOD_ITEMS_EXIST) {
        if (typer->economy) {
            // Foodchoice = 0 or 1 if inland, 0, 1, or 2 if coastal
            int foodchoice = rng::get_random(2 +
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
            products.push_back(p);
        }
    }

    for (unsigned int c= 0; c < (sizeof(typer->prods)/sizeof(typer->prods[0])); c++) {
        int item = typer->prods[c].product;
        int chance = typer->prods[c].chance * weight;
        int amt = typer->prods[c].amount;
        if (item != -1) {
            if (!(ItemDefs[item].flags & ItemType::DISABLED) &&
                    (rng::get_random(100) < chance)) {
                p = new Production(item, amt);
                products.push_back(p);
            }
        }
    }
}

/* Create a town randomly */
void ARegion::add_town()
{
    std::string tname = AGetNameString(AGetName(1, this));
    int size = DetermineTownSize();
    add_town(size, tname);
}

/* Create a town of any type with given name */
void ARegion::add_town(const std::string& tname)
{
    int size = DetermineTownSize();
    add_town(size, tname);
}

/* Create a town of given Town Type */
void ARegion::add_town(int size)
{
    std::string tname = AGetNameString(AGetName(1, this));
    add_town(size, tname);
}

/* Create a town of specific type with name
 * All other town creation functions call this one
 * in the last instance. */
void ARegion::add_town(int size, const std::string& name)
{
    town = new TownInfo;
    town->name = name;
    SetTownType(size);
    SetupCityMarket();
    /* remove all lairs */
    for(const auto obj : objects) {
        if (obj->type == O_DUMMY) continue;
        if ((ObjectDefs[obj->type].monster != -1) && (!(ObjectDefs[obj->type].flags & ObjectType::CANENTER))) {
            std::for_each(obj->units.begin(), obj->units.end(), [](Unit *u) { delete u; });
            obj->units.clear();
            std::erase(objects, obj);
            delete obj;
        }
    }
}

// Used at start to set initial town's size
int ARegion::DetermineTownSize()
{
    // is it a city?
    if (rng::get_random(300) < Globals->TOWN_DEVELOPMENT) {
        return TOWN_CITY;
    }
    // is it a town?
    if (rng::get_random(220) < Globals->TOWN_DEVELOPMENT + 10) {
        return TOWN_TOWN;
    }
    // ... then it's a village!
    return TOWN_VILLAGE;
}

// Set an existing town to a specific town type
void ARegion::SetTownType(int level)
{
    if (!town) return;
    // set some basics
    town->hab = TownHabitat();
    town->pop = town->hab * 2 / 3;
    town->dev = TownDevelopment();

    // Sanity check
    if ((level < TOWN_VILLAGE) || (level > TOWN_CITY)) return;

    // increment values
    int poptown = rng::get_random((level -1) * (level -1) * Globals->CITY_POP/12) + level * level * Globals->CITY_POP/12;
    town->hab += poptown;
    town->pop = town->hab * 2 / 3;
    development += level * 6 + 2;
    town->dev = TownDevelopment();

    // now increment until we reach the right size
    while(town->TownType() != level) {
        // Increase?
        if (level > town->TownType()) {
            development += rng::get_random(Globals->TOWN_DEVELOPMENT / 10 + 5);
            int poplus = rng::get_random(Globals->CITY_POP/3) + rng::get_random(Globals->CITY_POP/3);
            // don't overgrow!
            while (town->pop + poplus > Globals->CITY_POP) {
                poplus = poplus / 2;
            }
            town->hab += poplus;
            town->pop = town->hab * 2 / 3;
            town->dev = TownDevelopment();
        }
            // or decrease...
        else {
            development -= rng::get_random(20 - Globals->TOWN_DEVELOPMENT / 10);
            int popdecr = rng::get_random(Globals->CITY_POP/3) + rng::get_random(Globals->CITY_POP/3);
            // don't depopulate
            while ((town->pop < popdecr) || (town->hab < popdecr)) {
                popdecr = popdecr / 2;
            }
            town->hab -= popdecr;
            town->pop = town->hab * 2 / 3;
            town->dev = TownDevelopment();
        }
    }

    maxdevelopment = development;
}

void ARegion::UpdateEditRegion()
{
    // redo markets and entertainment/tax income for extra people.
    SetIncome();
    for (auto& m : markets) m->post_turn(Population(), Wages());

    //Replace man selling
    markets.erase(
        remove_if(markets.begin(), markets.end(), [](const Market * m) { return ItemDefs[m->item].type & IT_MAN; }),
        markets.end()
    );

    float ratio = ItemDefs[race].baseprice / (float) (Globals->BASE_MAN_COST * 10);
    // hack: include wage factor of 10 in float calculation above
    Market *m = new Market(
        Market::MarketType::M_BUY, race, (int)(Wages() * 4 * ratio), Population()/ 25, 0, 10000, 0, 2000
    );
    markets.push_back(m);

    if (Globals->LEADERS_EXIST) {
        ratio = ItemDefs[I_LEADERS].baseprice / (float) (Globals->BASE_MAN_COST * 10);
        // hack: include wage factor of 10 in float calculation above
        m = new Market(
            Market::MarketType::M_BUY, I_LEADERS, (int)(Wages() * 4 * ratio), Population() / 125, 0, 10000, 0, 400
        );
        markets.push_back(m);
    }
}

void ARegion::SetupEditRegion()
{
    // Direct copy of SetupPop() except that it calls AddTown(AString*)
    TerrainType *typer = &(TerrainDefs[type]);
    habitat = typer->pop+1;
    // Population factor: 5 times
    if (habitat > 1) habitat *= 5;
    if ((habitat < 100) && (typer->similar_type != R_OCEAN)) habitat = 100;

    int pop = typer->pop;
    int mw = typer->wages;

    // fix economy when MAINTENANCE_COST has been adjusted
    mw += Globals->MAINTENANCE_COST - 10;
    if (mw < 0) mw = 0;

    if (pop == 0) {
        population = 0;
        basepopulation = 0;
        wages = 0;
        maxwages = 0;
        wealth = 0;
        return;
    }

    // Only select race here if it hasn't been set during Race Growth
    // in the World Creation process.
    if ((race == -1) || (!Globals->GROW_RACES)) {
        int noncoastalraces = sizeof(typer->races)/sizeof(typer->races[0]);
        int allraces =
            noncoastalraces + sizeof(typer->coastal_races)/sizeof(typer->coastal_races[0]);

        race = -1;
        while (race == -1 || (ItemDefs[race].flags & ItemType::DISABLED)) {
            int n = rng::get_random(IsCoastal() ? allraces : noncoastalraces);
            if (n > noncoastalraces-1) {
                race = typer->coastal_races[n-noncoastalraces-1];
            } else
                race = typer->races[n];
        }
    }

    habitat = habitat * 2/3 + rng::get_random(habitat/3);
    auto mt = find_race(ItemDefs[race].abr)->get();
    if (mt.terrain == typer->similar_type) {
        habitat = (habitat * 9)/8;
    }
    if (!IsNativeRace(race)) {
        habitat = (habitat * 4)/5;
    }
    basepopulation = habitat / 3;
    // hmm... somewhere not too far off equilibrium pop
    population = habitat * (60 + rng::get_random(6) + rng::get_random(6)) / 100;

    // Setup development
    int level = 1;
    development = 1;
    int prev = 0;
    while (level < mw) {
        development++;
        prev++;
        if (prev > level) {
            level++;
            prev = 0;
        }
    }
    development += rng::get_random(25);
    maxdevelopment = development;

    if (Globals->TOWNS_EXIST) {
        int adjacent = 0;
        int prob = Globals->TOWN_PROBABILITY;
        if (prob < 1) prob = 100;
        int townch = (int) 80000 / prob;
        if (Globals->TOWNS_NOT_ADJACENT) {
            for (int d = 0; d < NDIRS; d++) {
                ARegion *newregion = neighbors[d];
                if ((newregion) && (newregion->town)) adjacent++;
            }
        }
        if (Globals->LESS_ARCTIC_TOWNS) {
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
        if (spread > 100) spread = 100;
        int townprob = (TerrainDefs[type].economy * 4 * (100 - spread) +
            100 * spread) / 100;
        if (adjacent > 0) townprob = townprob * (100 - Globals->TOWNS_NOT_ADJACENT) / 100;
        std::string tname = AGetNameString(AGetName(1, this));
        if (rng::get_random(townch) < townprob) add_town(tname);
    }

    // set up work and entertainment income
    SetIncome();

    float ratio = ItemDefs[race].baseprice / ((float)Globals->BASE_MAN_COST * 10);
    // hack: include wage factor of 10 in float assignment above
    // Setup Recruiting
    Market *m = new Market(
        Market::MarketType::M_BUY, race, (int)(Wages() * 4 * ratio), Population() / 25, 0, 10000, 0, 2000
    );
    markets.push_back(m);

    if (Globals->LEADERS_EXIST) {
        ratio = ItemDefs[I_LEADERS].baseprice / ((float)Globals->BASE_MAN_COST * 10);
        // hack: include wage factor of 10 in float assignment above
        m = new Market(
            Market::MarketType::M_BUY, I_LEADERS, (int)(Wages() * 4 * ratio), Population() / 125, 0, 10000, 0, 400
        );
        markets.push_back(m);
    }
}

void ARegion::UpdateProducts()
{
    for (auto& prod : products) {
        int lastbonus = prod->baseamount / 2;
        int bonus = 0;

        if (prod->itemtype == I_SILVER && prod->skill == -1) continue;

        for(const auto o : objects) {
            if (o->incomplete < 1 && ObjectDefs[o->type].productionAided == prod->itemtype) {
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

/* BaseDev is the development floor at which poor
 * regions stabilise without player activity */
int ARegion::BaseDev()
{
    int level = 1;
    int basedev = 1;
    int prev = 0;
    while (level <= TerrainDefs[type].wages) {
        prev++;
        basedev++;
        if (prev > level) {
            level++;
            prev = 0;
        }
    }

    basedev = (Globals->MAINTENANCE_COST + basedev) / 2;
    return basedev;
}

/* ProdDev is the development floor for regions
 * factoring in player production in the region */
int ARegion::ProdDev()
{
    int basedev = BaseDev();
    for (const auto& p : products) {
        if (ItemDefs[p->itemtype].type & IT_NORMAL && p->itemtype != I_SILVER) {
            basedev += p->activity;
        }
    }
    return basedev;
}

int ARegion::TownHabitat()
{
    // Effect of existing buildings
    int farm = 0;
    int inn = 0;
    int temple = 0;
    int caravan = 0;
    int fort = 0;
    for(const auto obj : objects) {
        if (ObjectDefs[obj->type].protect > fort) fort = ObjectDefs[obj->type].protect;
        if (ObjectDefs[obj->type].productionAided >= 0) {
            if (ItemDefs[ObjectDefs[obj->type].productionAided].type & IT_FOOD) farm++;
            if (ObjectDefs[obj->type].productionAided == I_SILVER) inn++;
            if (ObjectDefs[obj->type].productionAided == I_HERBS) temple++;
            if (
                (ObjectDefs[obj->type].flags & ObjectType::TRANSPORT) &&
                (ItemDefs[ObjectDefs[obj->type].productionAided].type & IT_MOUNT)
                ) {
                caravan++;
            }
        }
    }
    int hab = 2;
    int step = 0;
    for (int i=0; i<5; i++) {
        if (fort > step) hab++;
        step *= 5;
        if (step == 0) step = 10;
    }
    int build = 0;
    if (farm) build++;
    if (inn) build++;
    if (temple) build++;
    if (caravan) build++;
    if (build > 2) build = 2;

    build++;
    hab = (build * build + 1) * hab * hab + habitat / 4 + 50;

    // Effect of town development on habitat:
    int totalhab = hab + (TownDevelopment() * (habitat + 800 + hab
        + Globals->CITY_POP / 2)) / 100;

    return totalhab;
}

// Use this to determine development advantages
// due to connecting roads
int ARegion::RoadDevelopment()
{
    // Road bonus
    int roads = 0;
    for (int i=0; i<NDIRS; i++) if (HasExitRoad(i)) roads++;
    int dbonus = 0;
    if (roads > 0) {
        dbonus = RoadDevelopmentBonus(16, development);
        if (!town) dbonus = dbonus / 2;
    }
    // Maximum bonus of 45 to development for roads
    int bonus = 5;
    int leveloff = 1;
    int plateau = 4;
    int totalb = 0;
    while((dbonus > 0) && (totalb < 46)) {
        dbonus--;
        // reduce development adjustment gradually for large road bonuses
        if (leveloff >= plateau) if (bonus > 1) {
            bonus--;
            leveloff = 1;
            plateau--;
        }
        leveloff++;
        totalb += bonus;
    }
    return totalb;
}

// Measure of the economic development of a town
// on a scale of 0-100. Used for town growth/hab/limits
// and also for raising development through markets.
// Do NOT take roads into account as they are a bonus and
// considered outside of these limits.
int ARegion::TownDevelopment()
{
    int basedev = BaseDev();
    int df = development - basedev;
    if (df < 0) df = 0;
    if (df > 100) df = 100;

    return df;
}

// Checks the growth potential of towns
// and cancels unlimited markets for Starting Cities
// that have been taken over
int ARegion::TownGrowth()
{
    int tarpop = town->pop;

    // Don't update population in Starting Cities
    if (!IsStartingCity()) {
        // Calculate target population from market activity
        int amt = 0;
        int tot = 0;
        for (const auto& m : markets) {
            if (Population() > m->minpop) {
                if (m->type == Market::MarketType::M_BUY) {
                    if (ItemDefs[m->item].type & IT_TRADE) {
                        amt += 5 * m->activity;
                        tot += 5 * m->maxamt;
                        /* regional economic improvement */
                        improvement += 3 * amt;
                    }
                } else { // m->type == M_SELL
                    // Only food items except fish are mandatory
                    // for town growth - other items can
                    // be used in replacement
                    if (ItemDefs[m->item].type & IT_FOOD) {
                        amt += 2 * m->activity;
                    } else amt += m->activity;
                    if ((ItemDefs[m->item].type & IT_FOOD) && (m->item != I_FISH))
                        tot += 2 * m->maxamt;
                    if (ItemDefs[m->item].type & IT_TRADE) {
                        /* regional economic improvement */
                        improvement += 3 * amt;
                    }
                }
            }
        }

        if (amt > tot) amt = tot;

        if (tot) {
            tarpop += (Globals->CITY_POP * amt) / tot;
        }
        // Let's bump tarpop up
        // tarpop = (tarpop * 5) / 4;
        if (tarpop > Globals->CITY_POP) tarpop = Globals->CITY_POP;
    }
    return tarpop;
}

/* Damage region because of pillaging */
void ARegion::Pillage()
{
    wealth = 0;
    int damage = development / 3;
    development -= damage;
    if (Globals->DYNAMIC_POPULATION) {
        // Don't do population damage if population can't recover
        int popdensity = Globals->CITY_POP / 2000;
        AdjustPop(- damage * rng::get_random(popdensity) - rng::get_random(5 * popdensity));
    }
    /* Stabilise at minimal development levels */
    while (Wages() < Globals->MAINTENANCE_COST / 20) development += rng::get_random(5);
}


/* Grow region and town population and set basic
 * migration parameters */
void ARegion::Grow()
{
    // We don't need to grow 0 pop regions
    if (basepopulation == 0) return;

    // growpop is the overall population growth
    int growpop = 0;

    // Init migration parameters
    // immigrants = entering this region,
    // emigrants = leaving
    immigrants = habitat - basepopulation;
    emigrants = population - basepopulation;

    // First, check regional population growth
    // Check resource production activity
    int activity = 0;
    int amount = 0;
    for (const auto& p : products) {
        if (ItemDefs[p->itemtype].type & IT_NORMAL && p->itemtype != I_SILVER) {
            activity += p->activity;
            // bonuses for productivity are calculated from
            // the _baseamount_ of all resources.
            // because trade structures increase the produceable
            // amount and not the baseamount this creates an
            // effective advantage for trade structures
            amount += p->baseamount;
        }
    }

    // Now set the target population for the hex
    // Ant: I'm not sure why population's being subtracted here..
    //      Shouldn't it be something like :
    //          tarpop = habitat + basepopulation?
    int tarpop = habitat - population + basepopulation;

    // Ant: Increase tarpop for any trading that's going on?
    //      Not sure why (habitat - basepopulation) is included
    if (amount) tarpop += ((habitat - basepopulation) * 2 * activity) /
                (3 * amount);

    // diff is the amount we can grow?
    int diff = tarpop - population;
    int adiff = abs(diff);
    //if (adiff < 0) adiff = adiff * (- 1);

    // Adjust basepop?
    // raise basepop depending on production
    // absolute basepop increase
    if (diff > (basepopulation / 20)) {
        int gpop = rng::get_random(Globals->CITY_POP / 600);
        if (gpop > diff / 20) gpop = diff / 20;
        // relative basepop increase
        int relativeg = basepopulation * gpop / 1000;
        if (diff > habitat / 20) basepopulation += gpop + relativeg;
    }
    // lower basepop for extremely low levels of population
    if (population < basepopulation) {
        int depop = (basepopulation - population) / 4;
        basepopulation -= depop + rng::get_random(depop);
    }

    // Limit excessive growth at low pop / hab ratios
    // and avoid overflowing
    // What are grow2 and grow representing? Maybe these formulae
    // could be broken up a bit more and commented?
    long int grow2 = 5 * ((long int) habitat + (3 * (long int) adiff));

    // Ant: In the following formula, dgrow is almost always 0!
    long int dgrow = ((long int) adiff * (long int) habitat ) / grow2;
    if (diff < 0) growpop -= (int) dgrow;
    if (diff > 0) growpop += (int) dgrow;

    // update emigrants - only if region has a decent population level
    if (emigrants > 0) emigrants += diff;


    // Now check town population growth
    if (town) {
        int maxpop = TownGrowth();
        int tgrowth = maxpop - town->pop; // available room to grow
        immigrants += tgrowth;
        // less growth of towns in DYNAMIC_POPULATION
        // to balance town creation and population dynamics
        // through migration
        if (Globals->DYNAMIC_POPULATION) {
            tgrowth = tgrowth / 4;
        } else {
            // With roads can increase wages we need to
            // reduce settlement growth
            tgrowth = tgrowth / 2;
        }
        // Dampen growth curve at high population levels
        // Ant: maybe this formula could be broken up a bit?
        //      also, is (2 * town->hab - town->pop) correct?
        //      it's not meant to be 2 * (town->hab - town->pop)?
        //      ((2 * town->hab) - town->pop) seems clearer
        float increase = tgrowth * (2 * town->hab - town->pop);
        float limitingfactor = (10 * town->hab);

        // Ant: Not sure whether we still need the typecasts here
        growpop += (int) (increase / limitingfactor);
    }

    // Update population
    AdjustPop(growpop);

    /* Initialise the migration variables */
    migdev = 0;
    migfrom.clear();
}


/* Performs a search for each round of Migration for
 * the most attractive valid target region within
 * 2 hexes distance. */
void ARegion::FindMigrationDestination(int round)
{
    // is emigration possible?
    if (emigrants < 0) return;

    int maxattract = 0;
    ARegion *target = this;
    // Check all hexes within 2 hexes
    // range one neighbours
    for (int d=0; d < NDIRS; d++) {
        ARegion *nb = neighbors[d];
        if (!nb) continue;
        if (TerrainDefs[nb->type].similar_type == R_OCEAN) continue;
        int ma = nb->MigrationAttractiveness(development, 1, round);
        // check that we didn't migrate there in previous round
        if ((ma > maxattract) &&
            (!((nb->xloc == target->xloc) && (nb->yloc == target->yloc)))) {
            // set migration target
            target = nb;
            maxattract = ma;
        }
        // range two neighbours
        for (int d2=0; d2 < NDIRS; d2++) {
            ARegion *nb2 = nb->neighbors[d2];
            if (!nb2) continue;
            if (TerrainDefs[nb2->type].similar_type == R_OCEAN) continue;
            ma = nb2->MigrationAttractiveness(development, 2, round);
            // check that we didn't migrate there the previous round
            if ((ma > maxattract) &&
                (!((nb2->xloc == target->xloc) && (nb2->yloc == target->yloc)))) {
                // set migration target
                target = nb2;
                maxattract = ma;
            }
        }
    }
    // do we have a target?
    if (target == this) return;

    // then add this region to the target's migfrom list
    ARegion *self = this;
    target->migfrom.push_back(self);
}

/* Attractiveness of the region as a destination for migrants */
int ARegion::MigrationAttractiveness(int homedev, int range, int round)
{
    int attractiveness = 0;
    int mdev = development;
    /* Is there enough immigration capacity? */
    if (immigrants < 100) return 0;
    /* on the second round, consider as a mid-way target */
    if (round > 1) mdev = migdev;
    /* minimum development difference 8 x range */
    mdev -= 8 * range;
    if (mdev <= homedev) return 0;
    /* available entertainment */
    Production *p = get_production_for_skill(I_SILVER, S_ENTERTAINMENT);
    int entertain = p->activity / 20;
    /* available space */
    float space = 1 / 2;
    int offset = Globals->CITY_POP / 100;
    if (town) {
        space += ((habitat - population) + (town->hab - town->pop) + offset)
            / (habitat + town->hab + offset);
    } else {
        space += (habitat - population + offset) / (habitat + offset);
    }
    /* attractiveness due to development */
    attractiveness += (int) (space * ((float) 100 * (mdev - homedev) / homedev + entertain));

    return attractiveness;
}

/* Performs migration for each region with a migration
 * route pointing to the region (i.e. element of migfrom list),
 * adjusting population for hex of origin and itself */
void ARegion::Migrate()
{
    // calculate total potential migrants
    int totalmig = 0;
    for(const auto r : migfrom) {
        if (!r) continue;
        totalmig += r->emigrants;
    }

    // is there any migration to perform?
    if (totalmig == 0) return;

    // do each migration
    int totalimm = 0;
    for (auto r : migfrom) {
        if (!r) continue;

        // figure range
        int xdist = r->xloc - xloc;
        if (xdist < 0) xdist = - xdist;
        int ydist = r->yloc - yloc;
        if (ydist < 0) ydist = - ydist;
        ydist = (ydist - xdist) / 2;
        int range = xdist + ydist;

        // sanity check - huh?
        if (range < 1) continue;
        int migrants = (int) (immigrants * ((float) (r->emigrants / totalmig)));
        int mdiff = development - 7 - r->development;
        mdiff -= 8 * (range - 1);
        if (mdiff < 0) continue;
        int mmult = 1;
        for (int x=1; x*x < mdiff; x++) mmult = x;
        // adjust migrants according to development difference
        migrants = (int) (migrants * (float) (((mdiff + 100) * mmult) / 500));
        AdjustPop(migrants);
        r->AdjustPop(-migrants);
        r->emigrants -= migrants;
        totalimm += migrants;
        std::string wout = "Migrating from " + std::to_string(r->xloc) + "," + std::to_string(r->yloc) + " to " +
            std::to_string(xloc) + "," + std::to_string(yloc) + ": " + std::to_string(migrants) + " migrants.";
        logger::write(wout);
        // set the region's mid-way migration development
        r->migdev = (development - 8 * (range-1) + r->development) / 2;
        if (r->development > migdev) r->migdev = r->development;
    }
    // reduce possible immigrants
    immigrants -= totalimm;
    // clear migfrom
    migfrom.clear();
}

void ARegion::PostTurn()
{

    /* Check decay */
    if (Globals->DECAY) DoDecayCheck();

    /* Development increase due to player activity */
    // scale improvement
    float imp1 = improvement / 25;
    int imp2 = (improvement * 2 + 15) / 3;
    improvement = (int) (imp1 * imp2);
    // development increase possible?
    if (improvement > development) {
        int diff = improvement - development;
        /* Let road development increase chance of improvement */
        int progress = development - RoadDevelopment();
        /* Three chances to improve */
        for (int a=0; a<3; a++) if (rng::get_random(progress) < diff) development++;
        if (development > maxdevelopment) maxdevelopment = development;
    }

    /* Development increase for very poor regions */
    int recoveryRounds = 1 + earthlore + clearskies;

    if (wealth > 0) recoveryRounds++;

    while (recoveryRounds-- > 0) {
        if (maxdevelopment > development) {
            if (rng::get_random(maxdevelopment) > development) development++;
        }
        if (maxdevelopment > development) {
            if (rng::get_random(maxdevelopment) > development) development++;
        }
        if (maxdevelopment > development) {
            if (rng::get_random(3) == 1) development++;
        }
    }

    /* Check if we were a starting city and got taken over */
    if (IsStartingCity() && !HasCityGuard() && !Globals->SAFE_START_CITIES) {
        // Make sure we haven't already been modified.
        int done = 1;
        for (const auto& m : markets) {
            if (m->minamt == -1) {
                done = 0;
                break;
            }
        }

        if (!done) {
            for (auto& m : markets) delete m; // Free the allocated object
            markets.clear(); // empty the vector.
            SetupCityMarket();
            float ratio = ItemDefs[race].baseprice / (float) (Globals->BASE_MAN_COST * 10);
            // Setup Recruiting
            Market *m = new Market(
                Market::MarketType::M_BUY, race, (int)(Wages() * 4 * ratio), Population() / 25, 0, 10000, 0, 2000
            );
            markets.push_back(m);
            if (Globals->LEADERS_EXIST) {
                ratio = ItemDefs[I_LEADERS].baseprice / (float)Globals->BASE_MAN_COST;
                m = new Market(
                    Market::MarketType::M_BUY, I_LEADERS, (int)(Wages() * 4 * ratio), Population() / 125,
                    0, 10000, 0, 400
                );
                markets.push_back(m);
            }
        }
    }

    /* Set wage income and entertainment */
    if (type != R_NEXUS) {
        SetIncome();
    }

    /* update markets */
    for (auto& m : markets) m->post_turn(Population(), Wages());

    /* update resources */
    UpdateProducts();

    // Set these guys to 0.
    earthlore = 0;
    clearskies = 0;

    for(const auto o : objects) {
        for(const auto u : o->units) {
            u->PostTurn(this);
        }
    }
}
