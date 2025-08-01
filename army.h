#pragma once
#ifndef ARMY_H
#define ARMY_H

#include <functional>
#include <map>
#include <vector>
#include <unordered_set>

class Soldier;
class Army;
class Location;

#include "unit.h"
#include "items.h"
#include "object.h"
#include "helper.h"
#include "skills.h"

const WeaponBonusMalus* GetWeaponBonusMalus(const WeaponType& attacker, const WeaponType& target);

struct AttackStat {
    std::string effect;
    int weaponIndex;
    int attackType;
    int weaponClass;

    int soldiers;

    int attacks;
    int failed;
    int missed;
    int blocked;
    int hit;

    int damage;
    int killed;
};


struct UnitStat {
    std::string unitName;
    std::vector<AttackStat> attackStats;
};

namespace unit_stat_control {
    void Clear(UnitStat& us);
    AttackStat* FindStat(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
    void TrackSoldier(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int attackType, int weaponClass);
    void RecordAttack(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
    void RecordAttackFailed(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
    void RecordAttackMissed(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
    void RecordAttackBlocked(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
    void RecordHit(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int damage);
    void RecordKill(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
};

class ArmyStats {
    public:
        // key is unit number
        std::map<int, UnitStat> roundStats;
        std::map<int, UnitStat> battleStats;

        void ClearRound();

        void TrackUnit(Unit *unit);

        void TrackSoldier(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int attackType, int weaponClass);
        void RecordAttack(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
        void RecordAttackFailed(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
        void RecordAttackMissed(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
        void RecordAttackBlocked(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
        void RecordHit(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int damage);
        void RecordKill(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect);
};

class Soldier {
    public:
        Soldier(Unit *unit, Object *object, int regType, int race, int ass=0);

        void SetupSpell();
        void SetupCombatItems();

        //
        // SetupHealing is actually game-specific, and appears in specials.cpp
        //
        void SetupHealing();

        bool has_effect(const std::string& effect);
        void set_effect(const std::string& effect);
        void clear_effect(const std::string& effect);
        void clear_one_time_effects(void);
        bool armor_protect(int weaponClass );

        void RestoreItems();
        void Alive(int);
        void Dead();

        /* Unit info */
        std::string name;
        Unit * unit;
        int race;
        int riding;
        int building;

        /* Healing information */
        int healing;
        int healtype;
        int healitem;
        int canbehealed;
        int regen;

        /* Attack info */
        int weapon;
        int attacktype;
        int askill;
        int attacks;
        int hitDamage;
        char const *special;
        int slevel;

        /* Defense info */
        int dskill[NUM_ATTACK_TYPES];
        int protection[NUM_ATTACK_TYPES];
        int armor;
        int hits;
        int maxhits;
        int damage;

        std::unordered_set<int> battleItems;
        int amuletofi;

        /* Effects */
        std::map<const std::string, bool> effects;
};

typedef Soldier * SoldierPtr;

class Shield {
public:
    int shieldtype;
    int shieldskill;
    Shield(int type, int skill) : shieldtype(type), shieldskill(skill) {}
};

class Army
{
    public:
        Army(Unit *ldr, std::list<Location *>& list, int regtype, int ass = 0);
        ~Army();

        void WriteLosses(Battle *b);
        void Lose(Battle *b, ItemList& spoils);
        void Win(Battle *b, ItemList& spoils);
        void Tie(Battle *b);
        int CanBeHealed();
        void DoHeal(Battle *b);
        void DoHealLevel(Battle *b, int level, int rate, int useItems);
        void Regenerate(Battle *b);

        void GetMonSpoils(ItemList& spoils, int monitem, int free);

        int Broken();
        int NumAlive();
        int NumBehind();
        int NumSpoilers();
        int CanAttack();
        int NumFront();
        int NumFrontHits();
        Soldier *GetAttacker(int, int &);
        int GetEffectNum(char const *effect);
        int GetTargetNum(char const *special = NULL, bool canAttackBehind = false);
        Soldier *GetTarget(int);
        int RemoveEffects(int num, char const *effect);
        int DoAnAttack(
            Battle *, char const *special, int numAttacks, int attackType,
            int attackLevel, int flags, int weaponClass, char const *effect,
            int mountBonus, Soldier *attacker, Army *attackers, bool attackbehind, int attackDamage
        );
        void Kill(int killed, int damage);
        void Reset();

        //
        // These funcs are in specials.cpp
        //
        int CheckSpecialTarget(char const *,int);

        SoldierPtr *soldiers;
        Unit *leader;
        std::vector<std::shared_ptr<Shield>> shields;
        int round;
        int tac;
        int canfront;
        int canbehind;
        int notfront;
        int notbehind;
        int count;
        // Used if set ADVANCED_TACTICS
        int tactics_bonus;

        int hitsalive; // current number of "living hits"
        int hitstotal; // Number of hits at start of battle.

        ArmyStats stats;    // battle statistics
};

#endif // ARMY_H
