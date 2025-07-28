#include "game.h"
#include "battle.h"
#include "army.h"
#include "gamedefs.h"
#include "gamedata.h"
#include "quests.h"
#include "items.h"
#include "strings_util.hpp"

using namespace std;

enum class StatsCategory {
    ROUND,
    BATTLE
};

void WriteStats(Battle &battle, Army &army, StatsCategory category) {
    auto leaderName = army.leader->name;
    std::string header = leaderName + " army:";

    battle.AddLine(header);

    auto stats = category == StatsCategory::ROUND
        ? army.stats.roundStats
        : army.stats.battleStats;

    int statLines = 0;
    for (auto &uskv : stats) {
        UnitStat us = uskv.second;
        if (us.attackStats.empty()) {
            continue;
        }

        battle.AddLine("- " + us.unitName + ":");

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

            s += ", attacked " + to_string(succeeded) + " of " + to_string(att.attacks) + " " + strings::plural(att.attacks, "time", "times");
            s += ", " + to_string(reachedTarget) + " successful " + strings::plural(reachedTarget, "attack", "attacks");
            s += ", " + to_string(att.blocked) + " blocked by armor";
            s += ", " + to_string(att.hit) + " " + strings::plural(att.killed, "hit", "hits");
            s += ", " + to_string(att.damage) + " total damage";
            s += ", and killed " + to_string(att.killed)  + " " + strings::plural(att.killed, "enemy", "enemies") + ".";

            battle.AddLine(s);
        }

        statLines++;
    }

    if (statLines == 0) battle.AddLine("Army made no attacks.");
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
    AddLine(att->leader->name + " gets a free round of attacks.");

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
        AddLine(def->leader->name + " is overwhelmed.");
    }

    /* Run attacks until done */
    int alv = def->NumAlive();
    while (att->CanAttack() && def->NumAlive()) {
        int num = rng::get_random(att->CanAttack());
        int behind;
        Soldier * a = att->GetAttacker(num, behind);
        DoAttack(att->round, a, att, def, behind, ass, attOverwhelm, false);
    }
    AddLine("");

    /* Write losses */
    def->Regenerate(this);
    alv -= def->NumAlive();
    AddLine(def->leader->name + " loses " + std::to_string(alv) + ".");
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
        auto mount = find_mount(ItemDefs[a->riding].abr).value().get();
        if (mount.mountSpecial != NULL) {
            int i, num, tot = -1;
            auto spd = find_special(mount.mountSpecial ? mount.mountSpecial : "").value().get();
            for (i = 0; i < 4; i++) {
                int times = spd.damage[i].value;
                int hitDamage =  spd.damage[i].hitDamage;

                if (spd.effectflags & SpecialType::FX_USE_LEV) times *= mount.specialLev;
                int realtimes = spd.damage[i].minnum + rng::get_random(times) + rng::get_random(times);
                num = def->DoAnAttack(
                    this, mount.mountSpecial, realtimes, spd.damage[i].type, mount.specialLev, spd.damage[i].flags,
                    spd.damage[i].dclass, spd.damage[i].effect, 0, a, attackers, canAttackBehind, hitDamage
                );
                if (num != -1) {
                    if (tot == -1) tot = num;
                    else tot += num;
                }
            }
            if (tot != -1)
                AddLine(a->name + " " + spd.spelldesc + ", " + spd.spelldesc2 + std::to_string(tot) + spd.spelltarget + ".");
        }
    }
    if (!def->NumAlive()) return;

    int numAttacks = a->attacks;
    if (a->attacks < 0) {
        numAttacks = (round % ( -1 * a->attacks ) == 1) ? 1 : 0;
    } else if (ass && (Globals->MAX_ASSASSIN_FREE_ATTACKS > 0) && (numAttacks > Globals->MAX_ASSASSIN_FREE_ATTACKS)) {
        numAttacks = Globals->MAX_ASSASSIN_FREE_ATTACKS;
    }

    for (int i = 0; i < numAttacks; i++) {
        auto weapon_def = (a->weapon != -1) ? find_weapon(ItemDefs[a->weapon].abr) : std::nullopt;

        if (behind && !canAttackFromBehind) {
            if (!weapon_def) break;
            if (!(weapon_def->get().flags & WeaponType::RANGED)) break;
        }

        int flags = 0;
        int attackType = ATTACK_COMBAT;
        int mountBonus = 0;
        int attackClass = SLASHING;
        int hitDamage = a->hitDamage;

        if (weapon_def) {
            flags = weapon_def->get().flags;
            attackType = weapon_def->get().attackType;
            mountBonus = weapon_def->get().mountBonus;
            attackClass = weapon_def->get().weapClass;
        }
        def->DoAnAttack(this, NULL, 1, attackType, a->askill, flags, attackClass,
                NULL, mountBonus, a, attackers, canAttackBehind, hitDamage);
        if (!def->NumAlive()) break;
    }

    a->clear_one_time_effects();
}

void Battle::NormalRound(int round,Army * a,Army * b)
{
    /* Write round header */
    AddLine("Round " + to_string(round) + ":");

    if (a->tactics_bonus > b->tactics_bonus) {
        AddLine(a->leader->name + " tactics bonus " + std::to_string(a->tactics_bonus) + ".");
    }
    if (b->tactics_bonus > a->tactics_bonus) {
        AddLine(b->leader->name + " tactics bonus " + std::to_string(b->tactics_bonus) + ".");
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
    if (aOverwhelm) AddLine(b->leader->name + " is overwhelmed.");

    bool bOverwhelm = IsArmyOverwhelmedBy(a, b);
    if (bOverwhelm) AddLine(a->leader->name + " is overwhelmed.");

    /* Run attacks until done */
    while (aalive && balive && (aatt || batt))
    {
        int num = rng::get_random(aatt + batt);
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
    AddLine(a->leader->name + " loses " + std::to_string(aialive) + ".");
    bialive -= balive;
    AddLine(b->leader->name + " loses " + std::to_string(bialive) + ".");
    AddLine("");

    if (Globals->BATTLE_LOG_LEVEL == BattleLogLevel::VERBOSE) {
        AddLine("Round " + std::to_string(round) + " statistics:");
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

void Battle::GetSpoils(std::list<Location *>& losers, ItemList& spoils, int ass)
{
    string quest_rewards;

    for(const auto l : losers) {
        Unit *u = l->unit;
        int numalive = u->GetSoldiers();
        int numdead = u->losses;
        if (!numalive) {
            if (quests.check_kill_target(u, spoils, &quest_rewards)) {
                // TODO why doesn't the unit get an event here?
                AddLine("Quest completed! " + quest_rewards);
            }
        }
        for(auto it = u->items.begin(); it != u->items.end();) {
            Item *i = *it;
            // We are going to advance the iterator here so that even if the item under the current iterator
            // gets deleted by setting it's amount to 0, the iterator is still good.
            ++it;
            if (IsSoldier(i->type)) continue;
            // ignore incomplete ships
            if (ItemDefs[i->type].type & IT_SHIP) continue;
            // New rule:  Assassins with RINGS cannot get AMTS in spoils
            // This rule is only meaningful with Proportional AMTS usage
            // is enabled, otherwise it has no effect.
            if ((ass == 2) && (i->type == I_AMULETOFTS)) continue;
            float percent = (float)numdead/(float)(numalive + numdead);
            // incomplete ships:
            if (ItemDefs[i->type].type & IT_SHIP) {
                if (rng::get_random(100) < percent) {
                    u->items.SetNum(i->type, 0);
                    if (i->num < spoils.GetNum(i->type)) spoils.SetNum(i->type, i->num);
                }
            } else {
                int num = (int)(i->num * percent);
                int num2 = (num + rng::get_random(2))/2;
                if (ItemDefs[i->type].type & IT_ALWAYS_SPOIL) num2 = num;
                if (ItemDefs[i->type].type & IT_NEVER_SPOIL) num2 = 0;
                spoils.SetNum(i->type, spoils.GetNum(i->type) + num2);
                u->items.SetNum(i->type, i->num - num);
            }
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

    auto fact = new BattleFact;

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
        name = unit->object->name;
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
    Events* events, ARegion *region, Unit *att, std::list<Location *>& atts,
    Unit *tar, std::list<Location *>& defs, int ass
)
{
    Army * armies[2];
    std::string temp;
    assassination = ASS_NONE;
    attacker = att->faction;

    armies[0] = new Army(att, atts, region->type, ass);
    armies[1] = new Army(tar, defs, region->type, ass);

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
            AddLine(armies[0]->leader->name + " is routed!");
            FreeRound(armies[1],armies[0]);
        } else {
            AddLine(armies[0]->leader->name + " is destroyed!");
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
            temp = "Spoils: " + spoils.report(2,0,1) + ".";
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
            asstext = armies[1]->leader->name + " is assassinated in " + region->short_print() + "!";
        }
        if (armies[1]->NumAlive()) {
            AddLine(armies[1]->leader->name + " is routed!");
            FreeRound(armies[0],armies[1]);
        } else {
            AddLine(armies[1]->leader->name + " is destroyed!");
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
            temp = "Spoils: " + spoils.report(2,0,1) + ".";
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
    ARegion * r, Unit * att, Unit * tar, std::list<Location *>& atts, std::list<Location *>& defs, int ass
)
{
    if (ass) AddLine(att->name + " attempts to assassinate " + tar->name + " in " + r->short_print() + "!");
    else AddLine(att->name + " attacks " + tar->name + " in " + r->short_print() + "!");
    AddLine("");

    int dobs = 0;
    int aobs = 0;
    for(const auto d : defs) {
        int a = d->unit->GetAttribute("observation");
        if (a > dobs) dobs = a;
    }

    AddLine("Attackers:");
    for(const auto at : atts) {
        int a = at->unit->GetAttribute("observation");
        if (a > aobs) aobs = a;
        AddLine(at->unit->battle_report(dobs));
    }

    AddLine("");
    AddLine("Defenders:");
    for(const auto de : defs) AddLine(de->unit->battle_report(aobs));
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

void Battle::AddLine(const std::string& line) {
    text.push_back(line);
}

void Game::GetDFacs(ARegion * r, Unit * t, std::set<Faction *>& facs)
{
    int AlliesIncluded = 0;

    // First, check whether allies should assist in this combat
    if (Globals->ALLIES_NOAID == 0) {
        AlliesIncluded = 1;
    } else {
        // Check whether any of the target faction's
        // units aren't set to noaid
        for(const auto obj : r->objects) {
            for(const auto u : obj->units) {
                if (u->IsAlive() && u->faction == t->faction && u->GetFlag(FLAG_NOAID) == 0) {
                    AlliesIncluded = 1;
                    break;
                }
                if (AlliesIncluded == 1) break; // forlist(units)
            }
            if (AlliesIncluded == 1) break; // forlist (objects)
        }
    }

    for(const auto obj : r->objects) {
        for(const auto u : obj->units) {
            if (u->IsAlive() &&
                (u->faction == t->faction ||
                    (AlliesIncluded == 1 && u->guard != GUARD_AVOID && u->GetAttitude(r,t) == AttitudeType::ALLY)
                )
            ) {
                facs.insert(u->faction);
            }
        }
    }
}

void Game::GetAFacs(
    ARegion *r, Unit *att, Unit *tar, std::set<Faction *>& dfacs,
    std::set<Faction *>& afacs, std::list<Location *>& atts
)
{
    for(const auto obj : r->objects) {
        for(const auto u : obj->units) {
            if (u->canattack && u->IsAlive()) {
                int add = 0;
                if (
                    (u->faction == att->faction || u->GetAttitude(r,tar) == AttitudeType::HOSTILE) &&
                    (u->guard != GUARD_AVOID || u == att)
                ) {
                    add = 1;
                } else if (u->guard == GUARD_ADVANCE && u->GetAttitude(r,tar) != AttitudeType::ALLY) {
                    add = 1;
                } else if (u->attackorders) {
                    for(auto it = u->attackorders->targets.begin(); it != u->attackorders->targets.end();) {
                        UnitId *id = *it;
                        Unit *t = r->GetUnitId(id, u->faction->num);
                        if (!t) { ++it; continue; }
                        if (t == tar) {
                            it = u->attackorders->targets.erase(it);
                            delete id;
                        } else {
                            ++it;
                        }
                        if (t->faction == tar->faction) add = 1;
                    }
                }

                if (add && dfacs.find(u->faction) == dfacs.end()) {
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

int Game::CanAttack(ARegion *r, std::set<Faction *>& afacs, Unit * u)
{
    int see = 0;
    int ride = 0;
    for(const auto f : afacs) {
        if (f->CanSee(r, u) == 2) {
            if (ride == 1) return 1;
            see = 1;
        }
        if (f->CanCatch(r, u)) {
            if (see == 1) return 1;
            ride = 1;
        }
    }
    return 0;
}

void Game::GetSides(
    ARegion *r, std::set<Faction *>& afacs, std::set<Faction *>& dfacs, std::list<Location *>& atts,
    std::list<Location *>& defs, Unit *att, Unit *tar, int ass, int adv
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
        {
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
        }

        for(const auto o : r2->objects) {
            for(const auto u : o->units) {
                int add = 0;

#define ADD_ATTACK 1
#define ADD_DEFENSE 2
                /* First, can the unit be involved in the battle at all? */
                if ((i==-1 || u->GetFlag(FLAG_HOLDING) == 0) && u->IsAlive()) {
                    if(afacs.find(u->faction) != afacs.end()) {
                        /*
                         * The unit is on the attacking side, check if the
                         * unit should be in the battle
                         */
                        if (i == -1 || (!noaida)) {
                            if (u->canattack &&
                                (u->guard != GUARD_AVOID || u==att) && u->CanMoveTo(r2,r) && !::GetUnit(atts, u->num)
                            ) {
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
                                if (i == -1 && adv == 0) add = ADD_DEFENSE;
                            } else if (u->type == U_GUARDMAGE) {
                                /* the unit is a city guard support mage */
                                if (i == -1 && adv == 0) add = ADD_DEFENSE;
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
                                        if (u == tar || (u->faction == tar->faction && i==-1 && CanAttack(r, afacs,u))) {
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
        for(const auto l : defs) {
            if (!l->unit->GetFlag(FLAG_NOAID)) {
                noaidd = 0;
                break;
            }
        }
    }
}

int Game::KillDead(Location * l, Battle *b, int max_susk, int max_rais)
{
    int uncontrolled = 0;
    int skel, undead;
    std::string tmp;

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
                undead = rng::get_random(l->unit->raised * 2 / 3 + 1);
            }

            skel = l->unit->raised - undead;
            tmp = item_string(I_SKELETON, skel);
            if (undead > 0) {
                tmp += " and ";
                tmp += item_string(I_UNDEAD, undead);
            }
            tmp += " rise";
            if ((skel + undead) == 1)
                tmp += "s";
            tmp += " from the grave to join ";
            tmp += l->unit->name;
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

int Game::RunBattle(ARegion * r,Unit * attacker,Unit * target,int ass,
                     int adv)
{
    std::set<Faction *> afacs, dfacs;
    std::list<Location *> atts, defs;
    int result;

    if (ass) {
        if (attacker->GetAttitude(r,target) == AttitudeType::ALLY) {
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
        if (attacker->GetAttitude(r,target) == AttitudeType::ALLY) {
            attacker->error("ATTACK: Can't attack an ally.");
            if (adv) {
                attacker->canattack = 0;
                if (attacker->advancefrom) {
                    attacker->MoveUnit( attacker->advancefrom->GetDummy() );
                }
            }
            return BATTLE_IMPOSSIBLE;
        }
        GetDFacs(r, target, dfacs);
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
        logger::write("Cannot find any attackers!");
        return BATTLE_IMPOSSIBLE;
    }
    if (defs.size() <= 0) {
        // This shouldn't happen, but just in case
        logger::write("Cannot find any defenders!");
        return BATTLE_IMPOSSIBLE;
    }

    Battle *b = new Battle;
    b->WriteSides(r, attacker, target, atts, defs, ass);

    battles.push_back(b);
    {
        for(const auto f : factions) {
            if(afacs.find(f) != afacs.end() || dfacs.find(f) != dfacs.end() || r->Present(f)) {
                f->battles.push_back(b);
            }
        }
    }
    result = b->Run(events, r, attacker, atts, target, defs,ass);

    int attaker_max_susk = 0;
    int attaker_max_rais = 0;
    for(const auto l : atts) {
        int susk_level = l->unit->GetAttribute("susk");
        int rais_level = l->unit->GetAttribute("rais");
        if (susk_level > attaker_max_susk) {
            attaker_max_susk = susk_level;
        }
        if (rais_level > attaker_max_rais) {
            attaker_max_rais = rais_level;
        }
    }

    int defender_max_susk = 0;
    int defender_max_rais = 0;
    for(const auto l : defs) {
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
        int undead = rng::get_random(uncontrolled * 2 / 3 + 1);
        int skel = uncontrolled - undead;
        std::string tmp = item_string(I_SKELETON, skel);
        if (undead > 0) {
            tmp += " and ";
            tmp += item_string(I_UNDEAD, undead);
        }
        tmp += " rise";
        if ((skel + undead) == 1)
            tmp += "s";
        tmp += " from the grave to seek vengeance.";
        Faction *monfac = GetFaction(factions, monfaction);
        Unit *u = GetNewUnit(monfac, 0);
        u->MakeWMon("Undead", I_SKELETON, skel);
        u->items.SetNum(I_UNDEAD, undead);
        u->MoveUnit(r->GetDummy());
        b->AddLine(tmp);
        b->AddLine("");
    }
    return result;
}
