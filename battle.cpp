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


#include "game.h"
#include "battle.h"
#include "army.h"
#include "gamedefs.h"
#include "gamedata.h"
#include "quests.h"
#include "items.h"

using namespace std;

enum StatsCategory {
	ROUND,
	BATTLE
};

void WriteStats(Battle &battle, Army &army, StatsCategory category) {
	auto leaderName = std::string(army.leader->name->Str());
	std::string header = std::string(army.leader->name->Str()) + " army:";

	battle.AddLine(AString(header.c_str()));

	auto stats = category == StatsCategory::ROUND
		? army.stats.roundStats
		: army.stats.battleStats;

	int statLines = 0;
	for (auto &uskv : stats) {
		UnitStat us = uskv.second;
		if (us.attackStats.empty()) {
			continue;
		}

		battle.AddLine(AString("- ") + us.unitName.c_str() + ":");

		for (auto &att : us.attackStats) {
			std::string s = "  - ";

			if (att.weaponIndex != -1) {
				ItemType &weapon = ItemDefs[att.weaponIndex];
				s += std::string(weapon.name) + " [" + weapon.abr + "]";
			}
			else if (att.effect != "") {
				 s += att.effect;
			}
			else {
				 s += "without weapon";
			}

			s += " (";

			switch (att.weaponClass)  {
				case SLASHING: s += "slashing"; break;
				case PIERCING: s += "piercing"; break;
				case CRUSHING: s += "cruhsing"; break;
				case CLEAVING: s += "cleaving"; break;
				case ARMORPIERCING: s += "armor piercing"; break;
				case MAGIC_ENERGY: s += "magic"; break;
				case MAGIC_SPIRIT: s += "magic"; break;
				case MAGIC_WEATHER: s += "magic"; break;

				default: s += "unknown"; break;
			}

			switch (att.attackType) {
				case ATTACK_COMBAT: s += " melee"; break;
				case ATTACK_RIDING: s += " riding"; break;
				case ATTACK_RANGED: s += " ranged"; break;
				case ATTACK_ENERGY: s += " energy"; break;
				case ATTACK_SPIRIT: s += " spirit"; break;
				case ATTACK_WEATHER: s += " weather"; break;

				default: s += " unknown"; break;
			}

			s += " attack)";

			int succeeded = att.attacks - att.failed;
			int reachedTarget = succeeded - att.missed;

			s += ", attacked " + to_string(succeeded) + " of " + to_string(att.attacks) + " " + plural(att.attacks, "time", "times");
			s += ", " + to_string(reachedTarget) + " successful " + plural(reachedTarget, "attack", "attacks");
			s += ", " + to_string(att.blocked) + " blocked by armor";
			s += ", " + to_string(att.hit) + " " + plural(att.killed, "hit", "hits");
			s += ", " + to_string(att.damage) + " total damage";
			s += ", and killed " + to_string(att.killed)  + " " + plural(att.killed, "enemy", "enemies") + ".";

			battle.AddLine(AString(s.c_str()));
		}

		statLines++;
	}

	if (statLines == 0) {
		battle.AddLine(AString("Army made no attacks."));
	}
}

Battle::Battle() { }
Battle::~Battle() { }

// Checks if army A is overwhelmed by army B
bool IsArmyOverwhelmedBy(Army * a, Army * b) {
	if (!Globals->OVERWHELMING) return false;

	int aPower;
	int bPower;
	if (Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_FIGURES) {
		aPower = Globals->OVERWHELMING * a->NumFront();
		bPower = b->NumFront();
	}
	else {
		aPower = Globals->OVERWHELMING * a->NumFrontHits();
		bPower = b->NumFrontHits();
	}

	return bPower > aPower;
}

void Battle::FreeRound(Army * att,Army * def, int ass)
{
	/* Write header */
	AddLine(*(att->leader->name) + " gets a free round of attacks.");

	/* Update both army's shields */
	att->shields.clear();
	UpdateShields(att);

	def->shields.clear();
	UpdateShields(def);

	//
	// Update the attacking armies round counter
	//
	att->round++;

	bool attOverwhelm = IsArmyOverwhelmedBy(def, att);
	if (attOverwhelm) {
		AddLine(*(def->leader->name) + " is overwhelmed.");
	}

	/* Run attacks until done */
	int alv = def->NumAlive();
	while (att->CanAttack() && def->NumAlive()) {
		int num = getrandom(att->CanAttack());
		int behind;
		Soldier * a = att->GetAttacker(num, behind);
		DoAttack(att->round, a, att, def, behind, ass, attOverwhelm, false);
	}
	AddLine("");

	/* Write losses */
	def->Regenerate(this);
	alv -= def->NumAlive();
	AddLine(*(def->leader->name) + " loses " + alv + ".");
	AddLine("");

	if (Globals->BATTLE_LOG_LEVEL == BattleLogLevel::VERBOSE) {
		AddLine("Free round statistics:");
		AddLine("");

		WriteStats(*this, *att, StatsCategory::ROUND);
		AddLine("");

		WriteStats(*this, *def, StatsCategory::ROUND);
		AddLine("");
	}

	att->Reset();
	att->stats.ClearRound();
	def->stats.ClearRound();
}

void Battle::DoAttack(int round, Soldier *a, Army *attackers, Army *def,
		int behind, int ass, bool canAttackBehind, bool canAttackFromBehind)
{
	DoSpecialAttack(round, a, attackers, def, behind, canAttackBehind);
	if (!def->NumAlive()) return;

	if (!behind && (a->riding != -1)) {
		MountType *pMt = FindMount(ItemDefs[a->riding].abr);
		if (pMt->mountSpecial != NULL) {
			int i, num, tot = -1;
			SpecialType *spd = FindSpecial(pMt->mountSpecial);
			for (i = 0; i < 4; i++) {
				int times = spd->damage[i].value;
				int hitDamage =  spd->damage[i].hitDamage;

				if (spd->effectflags & SpecialType::FX_USE_LEV)
					times *= pMt->specialLev;
				int realtimes = spd->damage[i].minnum + getrandom(times) +
					getrandom(times);
				num = def->DoAnAttack(this, pMt->mountSpecial, realtimes,
						spd->damage[i].type, pMt->specialLev,
						spd->damage[i].flags, spd->damage[i].dclass,
						spd->damage[i].effect, 0, a, attackers,
						canAttackBehind, hitDamage);
				if (num != -1) {
					if (tot == -1) tot = num;
					else tot += num;
				}
			}
			if (tot != -1) {
				AddLine(a->name + " " + spd->spelldesc + ", " +
						spd->spelldesc2 + tot + spd->spelltarget + ".");
			}
		}
	}
	if (!def->NumAlive()) return;

	int numAttacks = a->attacks;
	if (a->attacks < 0) {
		if (round % ( -1 * a->attacks ) == 1)
			numAttacks = 1;
		else
			numAttacks = 0;
	} else if (ass && (Globals->MAX_ASSASSIN_FREE_ATTACKS > 0) &&
			(numAttacks > Globals->MAX_ASSASSIN_FREE_ATTACKS)) {
		numAttacks = Globals->MAX_ASSASSIN_FREE_ATTACKS;
	}

	for (int i = 0; i < numAttacks; i++) {
		WeaponType *pWep = NULL;
		if (a->weapon != -1)
			pWep = FindWeapon(ItemDefs[a->weapon].abr);

		if (behind && !canAttackFromBehind) {
			if (!pWep) break;
			if (!( pWep->flags & WeaponType::RANGED)) break;
		}

		int flags = 0;
		int attackType = ATTACK_COMBAT;
		int mountBonus = 0;
		int attackClass = SLASHING;
		int hitDamage = a->hitDamage;

		if (pWep) {
			flags = pWep->flags;
			attackType = pWep->attackType;
			mountBonus = pWep->mountBonus;
			attackClass = pWep->weapClass;
		}
		def->DoAnAttack(this, NULL, 1, attackType, a->askill, flags, attackClass,
				NULL, mountBonus, a, attackers, canAttackBehind, hitDamage);
		if (!def->NumAlive()) break;
	}

	a->ClearOneTimeEffects();
}

void Battle::NormalRound(int round,Army * a,Army * b)
{
	/* Write round header */
	AddLine(AString("Round ") + round + ":");

	if (a->tactics_bonus > b->tactics_bonus) {
		AddLine(*(a->leader->name) + " tactics bonus " + a->tactics_bonus + ".");
	}
	if (b->tactics_bonus > a->tactics_bonus) {
		AddLine(*(b->leader->name) + " tactics bonus " + b->tactics_bonus + ".");
	}

	/* Update both army's shields */
	UpdateShields(a);
	UpdateShields(b);

	/* Initialize variables */
	a->round++;
	b->round++;
	int aalive = a->NumAlive();
	int aialive = aalive;
	int balive = b->NumAlive();
	int bialive = balive;
	int aatt = a->CanAttack();
	int batt = b->CanAttack();

	bool aOverwhelm = IsArmyOverwhelmedBy(b, a);
	if (aOverwhelm) {
		AddLine(*(b->leader->name) + " is overwhelmed.");
	}

	bool bOverwhelm = IsArmyOverwhelmedBy(a, b);
	if (bOverwhelm) {
		AddLine(*(a->leader->name) + " is overwhelmed.");
	}

	/* Run attacks until done */
	while (aalive && balive && (aatt || batt))
	{
		int num = getrandom(aatt + batt);
		int behind;
		if (num >= aatt)
		{
			num -= aatt;

			Soldier * s = b->GetAttacker(num, behind);
			DoAttack(b->round, s, b, a, behind, 0, bOverwhelm, aOverwhelm);
		}
		else
		{
			Soldier * s = a->GetAttacker(num, behind);
			DoAttack(a->round, s, a, b, behind, 0, aOverwhelm, bOverwhelm);
			// ---
		}
		aalive = a->NumAlive();
		balive = b->NumAlive();
		aatt = a->CanAttack();
		batt = b->CanAttack();
	}
	AddLine("");

	/* Finish round */
	a->Regenerate(this);
	b->Regenerate(this);
	aialive -= aalive;
	AddLine(*(a->leader->name) + " loses " + aialive + ".");
	bialive -= balive;
	AddLine(*(b->leader->name) + " loses " + bialive + ".");
	AddLine("");

	if (Globals->BATTLE_LOG_LEVEL == BattleLogLevel::VERBOSE) {
		AddLine(AString("Round ") + round + " statistics:");
		AddLine("");

		WriteStats(*this, *a, StatsCategory::ROUND);
		AddLine("");

		WriteStats(*this, *b, StatsCategory::ROUND);
		AddLine("");
	}

	a->Reset();
	a->stats.ClearRound();

	b->Reset();
	b->stats.ClearRound();
}

void Battle::GetSpoils(std::vector<Location *>& losers, ItemList& spoils, int assassination)
{
	ItemList ships;
	string quest_rewards;

	for(const auto l : losers) {
		Unit * u = l->unit;
		int numalive = u->GetSoldiers();
		int numdead = u->losses;
		if (!numalive) {
			if (quests.check_kill_target(u, spoils, &quest_rewards)) {
				// TODO why doesn't the unit get an event here?
				AddLine(AString("Quest completed! ") + quest_rewards);
			}
		}
		// Since the code below can modify the item list, iterate a copy
		ItemList itemsCopy = u->items;
		for(auto i: itemsCopy) {
			if (IsSoldier(i.type)) continue;
			// ignore incomplete ships
			if (ItemDefs[i.type].type & IT_SHIP) continue;
			// New rule:  Assassins with RINGS cannot get AMTS in spoils
			// This rule is only meaningful with Proportional AMTS usage
			// is enabled, otherwise it has no effect.
			if ((assassination == 2) && (i.type == I_AMULETOFTS)) continue;
			float percent = (float)numdead/(float)(numalive+numdead);
			// incomplete ships:
			if (ItemDefs[i.type].type & IT_SHIP) {
				if (getrandom(100) < percent) {
					u->items.SetNum(i.type, 0);
					if (i.num < ships.GetNum(i.type))
						ships.SetNum(i.type, i.num);
				}
			} else {
				int num = (int)(i.num * percent);
				int num2 = (num + getrandom(2))/2;
				if (ItemDefs[i.type].type & IT_ALWAYS_SPOIL) {
					num2 = num;
				}
				if (ItemDefs[i.type].type & IT_NEVER_SPOIL) {
					num2 = 0;
				}
				spoils.SetNum(i.type, spoils.GetNum(i.type) + num2);
				u->items.SetNum(i.type, i.num - num);
			}
		}
	}
	// add incomplete ships to spoils...
	for (int sh = 0; sh < NITEMS; sh++) {
		if (ItemDefs[sh].type & IT_SHIP) {
			spoils.SetNum(sh, ships.GetNum(sh));
		}
	}
}

void AddBattleFact(
	Events* events,
	ARegion* region,
	Unit* attacker,
	Unit* defender,
	Army* attackerArmy,
	Army* defenderArmy,
	int outcome
) {
	if (!Globals->WORLD_EVENTS) return;

	auto fact = new BattleFact();

	fact->location = EventLocation::Create(region);

	fact->attacker.AssignUnit(attacker);
	fact->attacker.AssignArmy(attackerArmy);

	fact->defender.AssignUnit(defender);
	fact->defender.AssignArmy(defenderArmy);

	fact->outcome = outcome;

	std::unordered_set<Unit *> units;
	for (int i = 0; i < defenderArmy->count; i++) {
		auto soldier = defenderArmy->soldiers[i];
		units.emplace(soldier->unit);
	}

	int protect = 0;
	int fortType = -1;
	std::string name;

	for (auto &unit : units) {
		ObjectType& type = ObjectDefs[unit->object->type];
		if (type.flags & ObjectType::GROUP) {
			continue;
		}

		if (unit->object->IsFleet()) {
			continue;
		}

		if (protect >= type.protect) {
			continue;
		}

		protect = type.protect;
		fortType = unit->object->type;
		name = unit->object->name->Str();
	}

	if (!name.empty()) {
		fact->fortification = name;
		fact->fortificationType = fortType;
	}


	events->AddFact(fact);
}

void AddAssassinationFact(
	Events* events,
	ARegion* region,
	Unit* defender,
	Army* defenderArmy,
	int outcome
) {
	if (!Globals->WORLD_EVENTS) return;

	auto fact = new AssassinationFact();

	fact->location = EventLocation::Create(region);

	// fact->victim.AssignUnit(defender);
	// fact->victim.AssignArmy(defenderArmy);

	fact->outcome = outcome;

	events->AddFact(fact);
}

int Battle::Run(
	Events *events,
	ARegion *region,
	Unit *att,
	std::vector<Location *>& atts,
	Unit *tar,
	std::vector<Location *>& defs,
	int ass,
	ARegionList *pRegs
)
{
	Army * armies[2];
	AString temp;
	assassination = ASS_NONE;
	attacker = att->faction;

	armies[0] = new Army(att,atts,region->type,ass);
	armies[1] = new Army(tar,defs,region->type,ass);

	if (ass) {
		FreeRound(armies[0],armies[1], ass);
	} else {
		if (Globals->ADVANCED_TACTICS) {
			int tactics_bonus = 0;
			if (armies[0]->tac > armies[1]->tac) {
				tactics_bonus = armies[0]->tac - armies[1]->tac;
				if (tactics_bonus > 3) tactics_bonus = 3;
				armies[0]->tactics_bonus = tactics_bonus;
			}
			if (armies[1]->tac > armies[0]->tac) {
				tactics_bonus = armies[1]->tac - armies[0]->tac;
				if (tactics_bonus > 3) tactics_bonus = 3;
				armies[1]->tactics_bonus = tactics_bonus;
			}
		} else {
			if (armies[0]->tac > armies[1]->tac) FreeRound(armies[0],armies[1]);
			if (armies[1]->tac > armies[0]->tac) FreeRound(armies[1],armies[0]);
		}
	}

	int round = 1;
	while (!armies[0]->Broken() && !armies[1]->Broken() && round < 101) {
		armies[0]->stats.ClearRound();
		armies[1]->stats.ClearRound();

		NormalRound(round++,armies[0],armies[1]);
	}

	if ((armies[0]->Broken() && !armies[1]->Broken()) ||
		(!armies[0]->NumAlive() && armies[1]->NumAlive())) {
		if (ass) assassination = ASS_FAIL;

		if (armies[0]->NumAlive()) {
			AddLine(*(armies[0]->leader->name) + " is routed!");
			FreeRound(armies[1],armies[0]);
		} else {
			AddLine(*(armies[0]->leader->name) + " is destroyed!");
		}
		AddLine("");

		if (Globals->BATTLE_LOG_LEVEL >= BattleLogLevel::DETAILED) {
			AddLine("Battle statistics:");
			AddLine("");

			WriteStats(*this, *armies[0], StatsCategory::BATTLE);
			AddLine("");

			WriteStats(*this, *armies[1], StatsCategory::BATTLE);
			AddLine("");
		}

		if (!ass) {
			AddBattleFact(events, region, att, tar, armies[0], armies[1], BATTLE_LOST);
		}
		else {
			AddAssassinationFact(events, region, tar, armies[1], BATTLE_WON);
		}

		AddLine("Total Casualties:");
		ItemList spoils;
		armies[0]->Lose(this, spoils);
		GetSpoils(atts, spoils, ass);
		if (spoils.size()) {
			temp = AString("Spoils: ") + spoils.Report(2,0,1) + ".";
		} else {
			temp = "Spoils: none.";
		}

		armies[1]->Win(this, spoils);

		AddLine("");
		AddLine(temp);
		AddLine("");

		delete armies[0];
		delete armies[1];
		return BATTLE_LOST;
	}

	if ((armies[1]->Broken() && !armies[0]->Broken()) ||
		(!armies[1]->NumAlive() && armies[0]->NumAlive())) {
		if (ass) {
			assassination = ASS_SUCC;
			asstext = armies[1]->leader->name->const_str();
			asstext += " is assassinated in ";
			asstext += region->ShortPrint().const_str();
			asstext += "!";
		}
		if (armies[1]->NumAlive()) {
			AddLine(*(armies[1]->leader->name) + " is routed!");
			FreeRound(armies[0],armies[1]);
		} else {
			AddLine(*(armies[1]->leader->name) + " is destroyed!");
		}
		AddLine("");

		if (Globals->BATTLE_LOG_LEVEL >= BattleLogLevel::DETAILED) {
			AddLine("Battle statistics:");
			AddLine("");

			WriteStats(*this, *armies[0], StatsCategory::BATTLE);
			AddLine("");

			WriteStats(*this, *armies[1], StatsCategory::BATTLE);
			AddLine("");
		}

		if (!ass) {
			AddBattleFact(events, region, att, tar, armies[0], armies[1], BATTLE_WON);
		}
		else {
			AddAssassinationFact(events, region, tar, armies[1], BATTLE_LOST);
		}

		AddLine("Total Casualties:");
		ItemList spoils;
		armies[1]->Lose(this, spoils);
		GetSpoils(defs, spoils, ass);
		if (spoils.size()) {
			temp = AString("Spoils: ") + spoils.Report(2,0,1) + ".";
		} else {
			temp = "Spoils: none.";
		}

		armies[0]->Win(this, spoils);
		AddLine("");
		AddLine(temp);
		AddLine("");

		delete armies[0];
		delete armies[1];
		return BATTLE_WON;
	}

	AddLine("The battle ends indecisively.");
	AddLine("");

	if (Globals->BATTLE_LOG_LEVEL >= BattleLogLevel::DETAILED) {
		AddLine("Battle statistics:");
		AddLine("");

		WriteStats(*this, *armies[0], StatsCategory::BATTLE);
		AddLine("");

		WriteStats(*this, *armies[1], StatsCategory::BATTLE);
		AddLine("");
	}

	if (!ass) {
		AddBattleFact(events, region, att, tar, armies[0], armies[1], BATTLE_DRAW);
	}
	else {
		AddAssassinationFact(events, region, tar, armies[1], BATTLE_DRAW);
	}

	AddLine("Total Casualties:");

	armies[0]->Tie(this);
	armies[1]->Tie(this);
	temp = "Spoils: none.";
	AddLine("");
	AddLine(temp);
	AddLine("");

	delete armies[0];
	delete armies[1];
	return BATTLE_DRAW;
}

void Battle::WriteSides(
	ARegion *r,
	Unit *att,
	Unit *tar,
	std::vector<Location *>& atts,
	std::vector<Location *>& defs,
	int ass,
	ARegionList *pRegs
)
{
	if (ass) {
		AddLine(*att->name + " attempts to assassinate " + *tar->name
				+ " in " + r->ShortPrint() + "!");
	} else {
		AddLine(*att->name + " attacks " + *tar->name + " in " +
				r->ShortPrint() + "!");
	}
	AddLine("");

	int dobs = 0;
	int aobs = 0;
	for(const auto l : defs) {
		int a = l->unit->GetAttribute("observation");
		if (a > dobs) dobs = a;
	}

	AddLine("Attackers:");
	for(const auto l : atts) {
		int a = l->unit->GetAttribute("observation");
		if (a > aobs) aobs = a;
		string temp = l->unit->BattleReport(dobs)->const_str();
		AddLine(temp);
	}
	AddLine("");
	AddLine("Defenders:");
	for(const auto l : defs) {
		string temp = l->unit->BattleReport(aobs)->const_str();
		AddLine(temp);
	}
	AddLine("");
}

void Battle::build_json_report(json& j, Faction *fac) {
	if(assassination == ASS_SUCC && fac != attacker) {
		j["type"] = "assassination";
		j["report"] = json::array();
		j["report"].push_back(asstext);
		j["report"].push_back(""); // All battle reports end with a new line.  Pre-json code added the line here.
		return;
	}
	j["type"] = "battle";
	j["report"] = text;
}

void Battle::AddLine(const AString & s) {
	text.push_back(s.const_str());
}

void Game::GetDFacs(ARegion *r, Unit *t, std::set<Faction *>& facs)
{
	bool allies_allowed = false;

	// First, check whether allies should assist in this combat
	if (Globals->ALLIES_NOAID == 0) {
		allies_allowed = true;
	} else {
		// Check whether any of the target faction's
		// units aren't set to noaid
		for(const auto obj : r->objects) {
			for(const auto u: obj->units) {
				if (u->IsAlive() && (u->faction == t->faction) && !(u->GetFlag(FLAG_NOAID))) {
					allies_allowed = true;
					break;
				}
			}
			if (allies_allowed) break; // forlist (objects)
		}
	}

	for(const auto obj : r->objects) {
		for(const auto u: obj->units) {
			if (
				u->IsAlive() &&
				(
					u->faction == t->faction ||
					(allies_allowed && u->guard != GUARD_AVOID && u->GetAttitude(r,t) == A_ALLY)
				)
			) {
				facs.insert(u->faction);
			}
		}
	}
}

void Game::GetAFacs(
	ARegion *r, Unit *att, Unit *tar, std::set<Faction *>& dfacs, std::set<Faction *>& afacs,
	std::vector<Location *>& atts
)
{
	for(const auto obj : r->objects) {
		for(const auto u: obj->units) {
			if (u->canattack && u->IsAlive()) {
				bool add = false;
				if (
					(u->faction == att->faction || u->GetAttitude(r,tar) == A_HOSTILE) &&
					(u->guard != GUARD_AVOID || u == att)
				) {
					add = true;
				} else {
					if (u->guard == GUARD_ADVANCE && u->GetAttitude(r,tar) != A_ALLY) {
						add = true;
					} else {
						if (u->attackorders) {
							UnitId tid;
							bool found_target = false;
							Unit *t;
							for (auto id : u->attackorders->targets) {
								t = r->get_unit_id(id, u->faction->num);
								if (!t) continue;
								if (t == tar) {
									found_target = true;
									tid = id;
								}
								if (t->faction == tar->faction) add = true;
							}
							if (found_target) std::erase(u->attackorders->targets, tid);
						}
					}
				}

				if (add && (dfacs.find(u->faction) == dfacs.end())) {
					Location * l = new Location;
					l->unit = u;
					l->obj = obj;
					l->region = r;
					atts.push_back(l);
					afacs.insert(u->faction);
				}
			}
		}
	}
}

bool Game::CanAttack(ARegion *r, std::set<Faction *> afacs, Unit * u)
{
	bool see = false;
	bool ride = false;
	for(auto f: afacs) {
		if (f->CanSee(r, u) == 2) {
			if (ride) return true;
			see = true;
		}
		if (f->CanCatch(r, u)) {
			if (see) return true;
			ride = true;
		}
	}
	return false;
}

void Game::GetSides(
	ARegion *r, std::set<Faction *> afacs, std::set<Faction *>dfacs,
	std::vector<Location *>& atts, std::vector<Location *>& defs,
	Unit *att, Unit *tar, bool ass, bool adv
)
{
	if (ass) {
		/* Assassination attempt */
		Location * l = new Location;
		l->unit = att;
		l->obj = r->GetDummy();
		l->region = r;
		atts.push_back(l);

		l = new Location;
		l->unit = tar;
		l->obj = r->GetDummy();
		l->region = r;
		defs.push_back(l);

		return;
	}

	int j=NDIRS;
	int noaida = 0, noaidd = 0;

	for (int i=-1;i<j;i++) {
		ARegion * r2 = r;

		// Check if neighbor exists and assign r2 to a neighbor
		if (i>=0) {
			r2 = r->neighbors[i];
			if (!r2) continue;
		}

		// Define block to avoid conflicts with another forlist local created variables
		for(const auto o : r2->objects) {
			// Can't get building bonus in another region without EXTENDED_FORT_DEFENCE
			if (i>=0 && !Globals->EXTENDED_FORT_DEFENCE) {
				o->capacity = 0;
				o->shipno = o->ships.size();
				continue;
			}

			/* Set building capacity */
			if (o->incomplete < 1 && o->IsBuilding()) {
				o->capacity = ObjectDefs[o->type].protect;
				o->shipno = 0;
				// AddLine("Fortification bonus added for ", o->name);
				// AddLine("");
			} else if (o->IsFleet()) {
				o->capacity = 0;
				o->shipno = 0;
			}
		}

		for(const auto o : r2->objects) {
			for(const auto u: o->units) {
				int add = 0;

#define ADD_ATTACK 1
#define ADD_DEFENSE 2
				/* First, can the unit be involved in the battle at all? */
				if ((i==-1 || u->GetFlag(FLAG_HOLDING) == 0) && u->IsAlive()) {
					if (afacs.find(u->faction) != afacs.end()) {
						/*
						 * The unit is on the attacking side, check if the
						 * unit should be in the battle
						 */
						if (i == -1 || (!noaida)) {
							if (u->canattack &&
									(u->guard != GUARD_AVOID || u==att) &&
									u->CanMoveTo(r2,r) &&
									!::GetUnit(atts, u->num)) {
								add = ADD_ATTACK;
							}
						}
					} else {
						/* The unit is not on the attacking side */
						/*
						 * First, check for the noaid flag; if it is set,
						 * only units from this region will join on the
						 * defensive side
						 */
						if (!(i != -1 && noaidd)) {
							if (u->type == U_GUARD) {
								/* The unit is a city guardsman */
								if (i == -1 && adv == 0)
									add = ADD_DEFENSE;
							} else if (u->type == U_GUARDMAGE) {
								/* the unit is a city guard support mage */
								if (i == -1 && adv == 0)
									add = ADD_DEFENSE;
							} else {
								/*
								 * The unit is not a city guardsman, check if
								 * the unit is on the defensive side
								 */
								if (dfacs.find(u->faction) != dfacs.end()) {
									if (u->guard == GUARD_AVOID) {
										/*
										 * The unit is avoiding, and doesn't
										 * want to be in the battle if he can
										 * avoid it
										 */
										if (u == tar ||
												(u->faction == tar->faction &&
												 i==-1 &&
												 CanAttack(r, afacs, u))) {
											add = ADD_DEFENSE;
										}
									} else {
										/*
										 * The unit is not avoiding, and wants
										 * to defend, if it can
										 */
										if (u->CanMoveTo(r2,r)) {
											add = ADD_DEFENSE;
										}
									}
								}
							}
						}
					}
				}

				if (add == ADD_ATTACK) {
					Location * l = new Location;
					l->unit = u;
					l->obj = o;
					l->region = r2;
					atts.push_back(l);
				} else if (add == ADD_DEFENSE) {
						Location * l = new Location;
						l->unit = u;
						l->obj = o;
						l->region = r2;
						defs.push_back(l);
				}
			}
		}
		//
		// If we are in the original region, check for the noaid status of
		// the units involved
		//
		if (i == -1) {
			noaida = 1;
			for(const auto l : atts) {
				if (!l->unit->GetFlag(FLAG_NOAID)) {
					noaida = 0;
					break;
				}
			}
		}

		noaidd = 1;
		{
			for(const auto l : defs) {
				if (!l->unit->GetFlag(FLAG_NOAID)) {
					noaidd = 0;
					break;
				}
			}
		}
	}
}

int Game::KillDead(Location * l, Battle *b, int max_susk, int max_rais)
{
	int uncontrolled = 0;
	int skel, undead;
	AString tmp;

	if (!l->unit->IsAlive()) {
		l->region->Kill(l->unit);
		uncontrolled += l->unit->raised;
		l->unit->raised = 0;
	} else {
		if (l->unit->advancefrom) {
			l->unit->MoveUnit( l->unit->advancefrom->GetDummy() );
		}
		// Raise undead/skel if mage is in battle
		if (l->unit->raised > 0 && max_susk > 0) {
			// Raise UNDE only if mage has RAISE_UNDEAD skill
			undead = 0;
			if (max_rais > 0) {
				undead = getrandom(l->unit->raised * 2 / 3 + 1);
			}

			skel = l->unit->raised - undead;
			tmp = ItemString(I_SKELETON, skel);
			if (undead > 0) {
				tmp += " and ";
				tmp += ItemString(I_UNDEAD, undead);
			}
			tmp += " rise";
			if ((skel + undead) == 1)
				tmp += "s";
			tmp += " from the grave to join ";
			tmp += *l->unit->name;
			tmp += ".";
			l->unit->items.SetNum(I_SKELETON, l->unit->items.GetNum(I_SKELETON) + skel);
			l->unit->items.SetNum(I_UNDEAD, l->unit->items.GetNum(I_UNDEAD) + undead);
			b->AddLine(tmp);
			b->AddLine("");
			l->unit->raised = 0;
		}
	}

	return uncontrolled;
}

int Game::RunBattle(ARegion *r, Unit *attacker, Unit *target, bool ass, bool adv)
{
	std::set<Faction *> afacs, dfacs;
	std::vector<Location *> atts, defs;
	int result;

	if (ass) {
		if (attacker->GetAttitude(r,target) == A_ALLY) {
			attacker->error("ASSASSINATE: Can't assassinate an ally.");
			return BATTLE_IMPOSSIBLE;
		}
		/* Assassination attempt */
		afacs.insert(attacker->faction);
		dfacs.insert(target->faction);
	} else {
		if ( r->IsSafeRegion() ) {
			attacker->error("ATTACK: No battles allowed in safe regions.");
			return BATTLE_IMPOSSIBLE;
		}
		if (attacker->GetAttitude(r,target) == A_ALLY) {
			attacker->error("ATTACK: Can't attack an ally.");
			if (adv) {
				attacker->canattack = 0;
				if (attacker->advancefrom) {
					attacker->MoveUnit( attacker->advancefrom->GetDummy() );
				}
			}
			return BATTLE_IMPOSSIBLE;
		}
		GetDFacs(r,target,dfacs);
		if (dfacs.find(attacker->faction) != dfacs.end()) {
			attacker->error("ATTACK: Can't attack an ally.");
			if (adv) {
				attacker->canattack = 0;
				if (attacker->advancefrom) {
					attacker->MoveUnit( attacker->advancefrom->GetDummy() );
				}
			}
			return BATTLE_IMPOSSIBLE;
		}
		GetAFacs(r, attacker, target, dfacs, afacs, atts);
	}

	GetSides(r, afacs, dfacs, atts, defs, attacker, target, ass, adv);

	if (atts.size() <= 0) {
		// This shouldn't happen, but just in case
		Awrite(AString("Cannot find any attackers!"));
		return BATTLE_IMPOSSIBLE;
	}
	if (defs.size() <= 0) {
		// This shouldn't happen, but just in case
		Awrite(AString("Cannot find any defenders!"));
		return BATTLE_IMPOSSIBLE;
	}

	Battle * b = new Battle;
	b->WriteSides(r, attacker, target, atts, defs, ass, &regions);

	battles.push_back(b);
	for(const auto& fac: factions) {
		Faction *f = fac.get();
		if ((afacs.find(f) != afacs.end()) || (dfacs.find(f) != dfacs.end()) || r->Present(f)) {
			f->battles.push_back(b);
		}
	}
	result = b->Run(events, r,attacker, atts, target, defs, ass, &regions );

	int attaker_max_susk = 0;
	int attaker_max_rais = 0;
	{
		for(const auto l : atts) {
			// int necr_level = ((Location *)elem)->unit->GetAttribute("necromancy");
			int susk_level = l->unit->GetAttribute("susk");
			int rais_level = l->unit->GetAttribute("rais");
			if (susk_level > attaker_max_susk) {
				attaker_max_susk = susk_level;
			}
			if (rais_level > attaker_max_rais) {
				attaker_max_rais = rais_level;
			}
		}
	}

	int defender_max_susk = 0;
	int defender_max_rais = 0;
	for(const auto l : defs) {
		// int necr_level = ((Location *)elem)->unit->GetAttribute("necromancy");
		int susk_level = l->unit->GetAttribute("susk");
		int rais_level = l->unit->GetAttribute("rais");
		if (susk_level > defender_max_susk) {
			defender_max_susk = susk_level;
		}
		if (rais_level > defender_max_rais) {
			defender_max_rais = rais_level;
		}
	}

	/* Remove all dead units */
	int uncontrolled = 0;
	for(const auto l : atts) {
		uncontrolled += KillDead(l, b, attaker_max_susk, attaker_max_rais);
	}
	for(const auto l : defs) {
		uncontrolled += KillDead(l, b, defender_max_susk, defender_max_rais);
	}
	if (uncontrolled > 0 && monfaction > 0) {
		// Number of UNDE or SKEL raised as uncontrolled
		int undead = getrandom(uncontrolled * 2 / 3 + 1);
		int skel = uncontrolled - undead;
		AString tmp = ItemString(I_SKELETON, skel);
		if (undead > 0) {
			tmp += " and ";
			tmp += ItemString(I_UNDEAD, undead);
		}
		tmp += " rise";
		if ((skel + undead) == 1)
			tmp += "s";
		tmp += " from the grave to seek vengeance.";
		Faction *monfac = get_faction(factions, monfaction);
		Unit *u = GetNewUnit(monfac, 0);
		u->MakeWMon("Undead", I_SKELETON, skel);
		u->items.SetNum(I_UNDEAD, undead);
		u->MoveUnit(r->GetDummy());
		b->AddLine(tmp);
		b->AddLine("");
	}
	return result;
}
