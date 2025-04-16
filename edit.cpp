#ifdef WIN32
#include <memory.h>  // Needed for memcpy on windows
#endif

#include <string.h>
#include <algorithm> // For std::remove_if, std::find_if, std::for_each

#include "game.h"
#include "unit.h"
#include "astring.h"
#include "gamedata.h"
#include "string_filters.hpp"
#include "string_parser.hpp"
#include <iostream>
#include <unordered_set>

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

		parser::string_parser parser;
		std::cin >> parser;
		Awrite("");

		parser::token token = parser.get_token();
		if (token == "qq") {
			exit = 1;
			Awrite("Quitting without saving.");
		} else if (token == "x") {
			exit = 1;
			*pSaveGame = 1;
			Awrite("Exit and save.");
		} else if (token == "1") {
			ARegion *pReg = EditGameFindRegion();
			if (pReg) EditGameRegion(pReg);
		} else if (token == "2") {
			EditGameFindUnit();
		} else if (token == "3") {
			EditGameCreateUnit();
		} else {
			Awrite("Select from the menu.");
		}

		if (exit) {
			break;
		}
	} while(1);

	return(1);
}

ARegion *Game::EditGameFindRegion()
{
	Awrite("Region coords (x y z):");

	parser::string_parser parser;
	std::cin >> parser;

	parser::token token = parser.get_token();
	if (!token) {
		Awrite("No such region.");
		return nullptr;
	}
	int x = token.get_number().value_or(-1);
	if (x == -1) {
		Awrite("No such region.");
		return nullptr;
	}

	token = parser.get_token();
	if (!token) {
		Awrite("No such region.");
		return nullptr;
	}
	int y = token.get_number().value_or(-1);
	if (y == -1) {
		Awrite("No such region.");
		return nullptr;
	}

	token = parser.get_token();
	int z = token.get_number().value_or(0);

	ARegion *pReg = regions.GetRegion(x, y, z);
	if (!pReg) {
		Awrite("No such region.");
		return nullptr;
	}

	return pReg;
}

void Game::EditGameFindUnit()
{
	Awrite("Which unit number?");

	parser::string_parser parser;
	std::cin >> parser;

	parser::token token = parser.get_token();
	if (!token) {
		Awrite("No such unit!");
		return;
	}
	int num = token.get_number().value_or(-1);
	Unit *pUnit = GetUnit(num);
	if (!pUnit) {
		Awrite("No such unit!");
		return;
	}
	EditGameUnit(pUnit);
}

void Game::EditGameRegion(ARegion *pReg)
{
	parser::string_parser parser;
	do {
		Awrite("Region " + std::to_string(pReg->num) + ": " + std::string(pReg->Print().const_str()));
		Awrite(" 1) Edit objects...");
		Awrite(" 2) Edit terrain...");
		Awrite(" q) Return to previous menu.");

		std::cin >> parser;

		parser::token token = parser.get_token();
		if (token == "1") EditGameRegionObjects(pReg);
		else if (token == "2") EditGameRegionTerrain(pReg);
		else if (token == "q") return;
		else Awrite("Select from the menu.");
	} while(1);
}


/* RegionEdit Patch 030829 BS */
void Game::EditGameRegionObjects( ARegion *pReg )
//template copied from AtlantisDev 030730 post. Modified option a, added option h, m.
{
	do {
		Awrite( AString( "Region: " ) + pReg->ShortPrint() );
		Awrite( "" );
		int i = 0;
		std::string temp;
		for(const auto obj : pReg->objects) {
			temp = std::to_string(i) + ". " + obj->name + " : " + ObjectDefs[obj->type].name;
			Awrite(temp);
			i++;
		}
		Awrite( "" );

		Awrite( " [a] [object type] [dir] to add object" );
		Awrite( " [d] [index] to delete object" );
		Awrite( " [n] [index] [name] to rename object" );
		Awrite( " q) Return to previous menu." );

		parser::string_parser parser;
		std::cin >> parser;

		parser::token token = parser.get_token();
		if (!token) {
			Awrite("Try again.");
			continue;
		}

		if (token == "q") {
			return;
		}

		if (token == "a") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int objType = parse_object(token, false);
			if (objType == -1 || (ObjectDefs[objType].flags & ObjectType::DISABLED)) {
				Awrite("No such object.");
				continue;
			}

			Object *o = new Object(pReg);
			o->type = objType;
			o->incomplete = 0;
			o->inner = -1;
			if (o->IsFleet()) {
				o->num = shipseq++;
				o->set_name("Fleet");
			} else {
				o->num = pReg->buildingseq++;
				o->set_name("Building");
			}
			pReg->objects.push_back(o);
			continue;
		}
		if (token == "d") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int index = token.get_number().value_or(-1);
			if (index < 0 || static_cast<size_t>(index) >= pReg->objects.size()) {
				Awrite("Incorrect index.");
				continue;
			}
			auto oit = pReg->objects.begin();
			for (int i = 0; i < index; i++) { if (oit != pReg->objects.end()) oit++; }
			if (oit == pReg->objects.end()) {
				Awrite("Incorrect index.");
				continue;
			}
			pReg->objects.erase(oit);
			continue;
		}
		if (token == "n") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int index = token.get_number().value_or(-1);
			if (index < 0 || static_cast<size_t>(index) >= pReg->objects.size()) {
				Awrite("Incorrect index.");
				continue;
			}

			auto oit = pReg->objects.begin();
			for (int i = 0; i < index; i++) { if (oit != pReg->objects.end()) oit++; }
			if (oit == pReg->objects.end()) {
				Awrite("Incorrect index.");
				continue;
			}
			Object *tmp = *oit;

			token = parser.get_token();
			if (!token) {
				Awrite("No name given.");
				continue;
			}

			std::string newname = token.get_string() | filter::legal_characters;
			if (!newname.empty()) {
				tmp->set_name(newname);
			}
			continue;
		}
	} while(1);
}

void Game::EditGameRegionTerrain( ARegion *pReg )
{
	do {
		Awrite("");
		Awrite("Region: " + std::string(pReg->Print().const_str()));
		Awrite( "" );
		// write pop stuff
		Awrite(std::to_string(pReg->population) + " " + ItemDefs[pReg->race].names + " basepop");
		if (pReg->town) Awrite(std::to_string(pReg->town->pop) + " " + ItemDefs[pReg->race].names + " townpop");
		Awrite(std::to_string(pReg->Population()) + " " + ItemDefs[pReg->race].names + " totalpop");

		// write wages
		Awrite("Wages: " + std::string(pReg->WagesForReport().const_str()) + ".");
		Awrite("Maxwages: " + std::to_string(pReg->maxwages) + ".");

		// write products
		std::string temp = "Products: ";
		bool need_comma = false;
		for (const auto& p : pReg->products) {
			if (p->itemtype == I_SILVER) {
				if (p->skill == S_ENTERTAINMENT) {
					Awrite("Entertainment available: $" + std::to_string(p->amount) + ".");
				}
				continue;
			}
			if (need_comma) temp += ", ";
			temp += p->write_report();
			need_comma = true;
		}
		if (!need_comma) temp += "none";
		temp += ".";
		Awrite(temp);
		Awrite("");

		if (Globals->GATES_EXIST && pReg->gate && pReg->gate != -1) {
			Awrite("There is a Gate here (Gate " + std::to_string(pReg->gate) + " of " +
				std::to_string(regions.numberofgates) + ").");
			if (Globals->GATES_NOT_PERENNIAL)
				Awrite("This gate opens in month " + std::to_string(pReg->gatemonth) + ".");
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

		parser::string_parser parser;
		std::cin >> parser;

		parser::token token = parser.get_token();
		if (!token) {
			Awrite("Try again.");
			continue;
		}
		if (token == "q") {
			return;
		}

		// modify terrain
		if (token == "t") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			AString str = token.get_string();
			int terType = ParseTerrain(&str);
			if (terType == -1) {
				Awrite( "No such terrain." );
				continue;
			}
			pReg->type = terType;
			continue;
		}

		if (token == "r") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int prace = parse_all_items(token);
			if (prace == -1) {
				Awrite( "No such race." );
				continue;
			}
			if (!(ItemDefs[prace].type & IT_MAN) || (ItemDefs[prace].flags & ItemType::DISABLED) ) {
				Awrite( "No such race." );
				continue;
			}
			pReg->race = prace;
			pReg->UpdateEditRegion();
			continue;
		}

		if (token == "dg") {
			if (!Globals->DISPERSE_GATE_NUMBERS && pReg->gate > 0) {
				int numgates = regions.numberofgates;
				for(const auto reg : regions) {
					if (reg->gate == numgates) {
						reg->gate = pReg->gate;
						break;
					}
					Awrite("Error: Could not find last gate");
				}
			}
			pReg->gate = 0;
			regions.numberofgates--;
			continue;
		}

		if (token == "ag") {
			if (pReg->gate > 0) break;
			regions.numberofgates++;
			if (Globals->DISPERSE_GATE_NUMBERS) {
				std::unordered_set<int> used;
				// find the power of 10 that is 2 powers greater than the number of gates
				int ngates= 10;
				while (ngates <= regions.numberofgates) {
					ngates *= 10;
				}
				for(const auto reg : regions) {
					if (reg->gate) used.insert(reg->gate - 1);
				}
				pReg->gate = rng::get_random(ngates);
				while (used.count(pReg->gate))
					pReg->gate = rng::get_random(ngates);
				pReg->gate++;
			} else {
				int gatenum = rng::get_random(regions.numberofgates) + 1;
				if (gatenum != regions.numberofgates) {
					for(const auto reg : regions) {
						if (reg->gate == gatenum) reg->gate = regions.numberofgates;
					}
				}
				pReg->gate = gatenum;
			}
			if (Globals->GATES_NOT_PERENNIAL) pReg->gatemonth = rng::get_random(12);
			continue;
		}

		if (token == "w") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int val = token.get_number().value_or(-1);
			if (val) {
				int change = val - pReg->maxwages;
				pReg->maxwages = val;
				pReg->wages += change;
			}
			pReg->UpdateEditRegion();
			continue;
		}

		if (token == "p") {
			auto removes = remove_if(
				pReg->products.begin(),
				pReg->products.end(),
				[](Production *p) { return p->itemtype != I_SILVER; }
			);
			for_each (removes, pReg->products.end(), [](Production *p) mutable { delete p; });
			pReg->products.erase(removes, pReg->products.end());
			pReg->SetupProds(1);
			continue;
		}

		if (token == "g") {
			if (pReg->town) delete pReg->town;
			pReg->town = NULL;

			for (auto& p : pReg->products) delete p; // Free the allocated object
			pReg->products.clear(); // empty the vector.
			pReg->SetupProds(1);

			for (auto& m : pReg->markets) delete m; // Free the allocated object
			pReg->markets.clear(); // empty the vector.

			pReg->SetupEditRegion();
			pReg->UpdateEditRegion();
			continue;
		}

		if (token == "n") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			pReg->name = token.get_string();
			continue;
		}

		if (token == "tn") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			pReg->town->name = token.get_string();
			continue;
		}


		if (token == "town") {
			if (pReg->race<0) pReg->race = 9;

			std::string townname = "name";
			if (pReg->town) {
				townname = pReg->town->name;
				delete pReg->town;
				for (auto& m : pReg->markets) delete m; // Free the allocated object
				pReg->markets.clear(); // empty the vector.
			}
			pReg->add_town(townname);

			pReg->UpdateEditRegion(); // financial stuff! Does markets
			continue;
		}

		if (token == "deltown") {
			if (pReg->town) {
				delete pReg->town;
				pReg->town = NULL;
				for (auto& m : pReg->markets) delete m; // Free the allocated object
				pReg->markets.clear(); // empty the vector.

				pReg->UpdateEditRegion();
			}
			continue;
		}

		if (token == "v") {
			if (pReg->town) EditGameRegionMarkets(pReg);
			continue;
		}
	} while(1);
}

void Game::EditGameRegionMarkets(ARegion *pReg)
{
	/* This only gets called if pReg->town exists! */
	do {
		Awrite("");
		Awrite("Region: " + std::string(pReg->Print().const_str()));
		Awrite("");
		// write pop stuff
		Awrite(std::to_string(pReg->town->pop) + " " + ItemDefs[pReg->race].names + " townpop");

		// write markets
		Awrite("Market Format: ... price(base). minpop/maxpop. minamt/maxamt.");

		Awrite("Wanted: ");
		for (const auto &m : pReg->markets) {
			if (m->type == Market::MarketType::M_SELL) {
				std::string temp = ItemString(m->item, m->amount) + " at $" + std::to_string(m->price) +
					"(" + std::to_string(m->baseprice) + ").";
				temp += " Pop: " + std::to_string(m->minpop) + "/" + std::to_string(m->maxpop) + ".";
				temp += " Amount: " + std::to_string(m->minamt) + "/" + std::to_string(m->maxamt) + ".";
				Awrite(temp);
			}
		}

		Awrite("For Sale: ");
		for (const auto &m : pReg->markets) {
			if (m->type == Market::MarketType::M_BUY) {
				std::string temp = ItemString(m->item, m->amount) + " at $" + std::to_string(m->price) +
					"(" + std::to_string(m->baseprice) + ").";
				temp += " Pop: " + std::to_string(m->minpop) + "/" + std::to_string(m->maxpop) + ".";
				temp += " Amount: " + std::to_string(m->minamt) + "/" + std::to_string(m->maxamt) + ".";
				Awrite(temp);
			}
		}

		Awrite("");
		Awrite(" [g] to regenerate all markets");
		Awrite(" [p] [item] [minpop] [maxpop] to add/modify market population");
		Awrite(" [a] [item] [minamt] [maxamt] to add/modify market amounts");
		Awrite(" [c] [item] [price] [baseprice] to add/modify item prices");
		Awrite(" [s] [item] to swap an item between wanted and sold");
		Awrite(" [d] [item] to delete an item from the market");
		Awrite(" q) Return to previous menu.");

		parser::string_parser parser;
		std::cin >> parser;

		parser::token token = parser.get_token();
		if (!token) {
			Awrite("Try again.");
			continue;
		}

		if (token == "q") {
			break;
		}

		if (token == "g") {
			// regenerate markets
			for (auto& m : pReg->markets) delete m; // Free the allocated object
			pReg->markets.clear(); // empty the vector.

			pReg->SetupCityMarket();
			pReg->UpdateEditRegion();
			continue;
		}

		if (token == "p") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int mitem = parse_enabled_item(token);
			if (mitem < 0) {
				Awrite("No such item");
				continue;
			}

			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int minimum = token.get_number().value_or(0);

			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int maximum = token.get_number().value_or(0);

			if (minimum + 1 > maximum) {
				Awrite("Maximum must be more than minimum");
				continue;
			}

			int population = pReg->Population();
			bool done = false;

			for (auto& m : pReg->markets) {
				if (m->item == mitem) {
					m->minpop = minimum;
					m->maxpop = maximum;

					if (population <= m->minpop) m->amount = m->minamt;
					else if (population >= m->maxpop) m->amount = m->maxamt;
					else {
						m->amount = m->minamt + (m->maxamt - m->minamt) * (population - m->minpop) /
							(m->maxpop - m->minpop);
					}
					done = true;
				}
			}

			if (!done) {
				int price = (ItemDefs[mitem].baseprice * (100 + rng::get_random(50))) / 100;
				Market *m = new Market(Market::MarketType::M_SELL, mitem, price, 0, minimum, maximum, 0, 0);
				pReg->markets.push_back(m);
			}
			continue;
		}

		if (token == "a") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int mitem = parse_enabled_item(token);
			if (mitem < 0) {
				Awrite("No such item");
				continue;
			}

			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int minimum = token.get_number().value_or(0);

			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int maximum = token.get_number().value_or(0);

			if (minimum + 1 > maximum) {
				Awrite("Maximum must be more than minimum");
				continue;
			}

			int population = pReg->Population();
			bool done = false;

			for (auto& m : pReg->markets) {
				if (m->item == mitem) {
					m->minamt = minimum;
					m->maxamt = maximum;

					if (population <= m->minpop) m->amount = m->minamt;
					else if (population >= m->maxpop) m->amount = m->maxamt;
					else {
						m->amount = m->minamt + (m->maxamt - m->minamt) * (population - m->minpop) /
							(m->maxpop - m->minpop);
					}
					done = true;
				}
			}

			if (!done) {
				int price = (ItemDefs[mitem].baseprice * (100 + rng::get_random(50))) / 100;
				int mamount = minimum + (maximum * population / 5000);
				Market *m = new Market(
					Market::MarketType::M_SELL, mitem, price, mamount, 0, 5000, minimum, maximum
				);
				pReg->markets.push_back(m);
			}
			continue;
		}

		if (token == "c") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int mitem = parse_enabled_item(token);
			if (mitem < 0) {
				Awrite("No such item");
				continue;
			}

			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int price = token.get_number().value_or(0);

			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int baseprice = token.get_number().value_or(0);

			if (price < 1 || baseprice < 1) {
				Awrite("Price must be more than zero");
				continue;
			}

			bool done = false;
			for (auto& m : pReg->markets) {
				if (m->item == mitem) {
					m->price = price;
					m->baseprice = baseprice;
					done = true;
				}
			}

			if (!done) {
				Market *m = new Market(Market::MarketType::M_SELL, mitem, price, 0, 0, 5000, 0, 0);
				m->baseprice = baseprice;
				pReg->markets.push_back(m);
			}
			continue;
		}

		if (token == "s") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int mitem = parse_enabled_item(token);
			if (mitem < 0) {
				Awrite("No such item");
				continue;
			}

			// remove all duplicate market items of the same type.
			std::unordered_set<int> s;
			auto dupes = std::remove_if(
				pReg->markets.begin(),
				pReg->markets.end(),
				[&s, mitem](const Market *m) { return m->item == mitem && !s.insert(m->item).second; }
			);
			std::for_each(dupes, pReg->markets.end(), [](Market *m) { delete m; });
			pReg->markets.erase(dupes, pReg->markets.end());

			auto m = std::find_if(
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
			continue;
		}

		if (token == "d") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}
			int mitem = parse_enabled_item(token);
			if (mitem < 0) {
				Awrite("No such item");
				continue;
			}

			auto m = std::find_if(
				pReg->markets.begin(),
				pReg->markets.end(),
				[mitem](const Market *m) { return m->item == mitem; }
			);

			if (m != pReg->markets.end()) {
				auto dupes = std::remove_if(
					pReg->markets.begin(),
					pReg->markets.end(),
					[mitem](const Market *m) { return m->item == mitem; }
				);
				std::for_each(dupes, pReg->markets.end(), [](Market *m) { delete m; });
				pReg->markets.erase(dupes, pReg->markets.end());
			} else {
				Awrite("No such market");
			}
			continue;
		}
	} while(1);
}


void Game::EditGameUnit(Unit *unit)
{
	do {
		Awrite("Unit: " + unit->name);
		Awrite("Faction: " + std::to_string(unit->faction->num));
		Awrite("  in " + std::string(unit->object->region->ShortPrint().const_str()));
		Awrite("  1) Edit items...");
		Awrite("  2) Edit skills...");
		Awrite("  3) Move unit...");
		Awrite("  4) Edit details...");
		Awrite("  q) Stop editing this unit.");

		parser::string_parser parser;
		std::cin >> parser;

		parser::token token = parser.get_token();
		if (!token) {
			Awrite("Try again.");
			continue;
		}

		if (token == "1") EditGameUnitItems(unit);
		else if (token == "2") EditGameUnitSkills(unit);
		else if (token == "3") EditGameUnitMove(unit);
		else if (token == "4") EditGameUnitDetails(unit);
		else if (token == "q") break;
		else Awrite("Select from the menu.");
	} while(1);
}

void Game::EditGameUnitItems(Unit *pUnit)
{
	do {
		Awrite("Unit items: " + std::string(pUnit->items.Report(2, 1, 1).const_str()));
		Awrite("  [item] [number] to update an item.");
		Awrite("  q) Stop editing the items.");

		parser::string_parser parser;
		std::cin >> parser;

		parser::token token = parser.get_token();
		if (!token) {
			Awrite("Try again.");
			continue;
		}

		if (token == "q") break;

		int itemNum = parse_all_items(token);
		if (itemNum == -1) {
			Awrite("No such item.");
			continue;
		}

		if (ItemDefs[itemNum].flags & ItemType::DISABLED) {
			Awrite("No such item.");
			continue;
		}

		token = parser.get_token();
		int num = token.get_number().value_or(0);

		pUnit->items.SetNum(itemNum, num);
		/* Mark it as known about for 'shows' */
		pUnit->faction->items.SetNum(itemNum, 1);
	} while(1);
}

void Game::EditGameUnitSkills(Unit *pUnit)
{
	do {
		Awrite("Unit skills: " + std::string(pUnit->skills.Report(pUnit->GetMen()).const_str()));
		Awrite("  [skill] [days] to update a skill.");
		Awrite("  q) Stop editing the skills.");

		parser::string_parser parser;
		std::cin >> parser;

		parser::token token = parser.get_token();
		if (!token) {
			Awrite("Try again.");
			continue;
		}

		if (token == "q") break;

		int skillNum = parse_skill(token);
		if (skillNum == -1) {
			Awrite("No such skill.");
			continue;
		}

		if (SkillDefs[skillNum].flags & SkillType::DISABLED) {
			Awrite("No such skill.");
			continue;
		}

		token = parser.get_token();
		int days = token.get_number().value_or(0);

		if ((SkillDefs[skillNum].flags & SkillType::MAGIC) && (pUnit->type != U_MAGE)) pUnit->type = U_MAGE;
		if ((SkillDefs[skillNum].flags & SkillType::APPRENTICE) && (pUnit->type == U_NORMAL)) pUnit->type = U_APPRENTICE;

		pUnit->skills.SetDays(skillNum, days * pUnit->GetMen());
		int lvl = pUnit->GetRealSkill(skillNum);
		if (lvl > pUnit->faction->skills.GetDays(skillNum)) pUnit->faction->skills.SetDays(skillNum, lvl);
	} while(1);
}

void Game::EditGameUnitMove(Unit *pUnit)
{
	ARegion *pReg = EditGameFindRegion();
	if (!pReg) return;

	pUnit->MoveUnit(pReg->GetDummy());
}

void Game::EditGameUnitDetails(Unit *unit)
{
	do {
		Awrite("Unit: " + unit->name);
		Awrite("Unit faction: " + unit->faction->name);
		std::string temp = " (";
		switch(unit->type) {
			case U_NORMAL: temp += "normal"; break;
			case U_MAGE: temp += "mage"; break;
			case U_GUARD: temp += "guard"; break;
			case U_WMON: temp += "monster"; break;
			case U_GUARDMAGE: temp += "guardmage"; break;
			case U_APPRENTICE: temp += Globals->APPRENTICE_NAME; break;
		}
		temp += ")";
		Awrite("Unit type: " + std::to_string(unit->type) + temp);

		Awrite("");
		Awrite("  [f] [num] to change the unit's faction.");
		Awrite("  [t] [num] to change the unit's type.");
		Awrite("  [q] Go back one screen.");

		parser::string_parser parser;
		std::cin >> parser;

		parser::token token = parser.get_token();
		if (!token) {
			Awrite("Try again.");
			continue;
		}

		if (token == "q") break;

		if (token == "f") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}

			int fnum = token.get_number().value_or(0);
			if (fnum < 1) {
				Awrite("Invalid Faction Number");
				continue;
			}

			Faction *fac = GetFaction(factions, fnum);
			if (fac) unit->faction = fac;
			else Awrite("Cannot Find Faction");
			continue;
		}

		if (token == "t") {
			token = parser.get_token();
			if (!token) {
				Awrite("Try again.");
				continue;
			}

			int newtype = token.get_number().value_or(-1);
			if (newtype < 0 || newtype > NUNITTYPES-1) {
				Awrite("Invalid Type");
				continue;
			}

			unit->type = newtype;
			continue;
		}
	} while(1);
}

void Game::EditGameCreateUnit()
{
	Faction *fac = GetFaction(factions, 1);
	Unit *newunit = GetNewUnit(fac);
	newunit->SetMen(I_LEADERS, 1);
	newunit->reveal = REVEAL_FACTION;
	newunit->MoveUnit(regions.front()->GetDummy());

	EditGameUnit(newunit);
}
