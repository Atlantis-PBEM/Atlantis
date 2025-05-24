#pragma once
#ifndef UNIT_H
#define UNIT_H

class Unit;
class UnitId;
enum class AttitudeType;

#include "faction.h"
#include "logger.hpp"
#include "orders.h"
#include "skills.h"
#include "items.h"
#include "object.h"
#include <set>
#include <string>
#include <list>

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

enum {
    GUARD_NONE,
    GUARD_GUARD,
    GUARD_AVOID,
    GUARD_SET,
    GUARD_ADVANCE
};

enum {
    TAX_NONE,
    TAX_TAX,
    TAX_PILLAGE,
    TAX_AUTO,
};

enum {
    REVEAL_NONE,
    REVEAL_UNIT,
    REVEAL_FACTION
};

enum {
    U_NORMAL,
    U_MAGE,
    U_GUARD,
    U_WMON,
    U_GUARDMAGE,
    U_APPRENTICE,
    NUNITTYPES
};

#define MAX_READY 4 // maximum number of ready weapons or armors

#define FLAG_BEHIND         0x0001
#define FLAG_NOCROSS_WATER      0x0002
#define FLAG_AUTOTAX            0x0004
#define FLAG_HOLDING            0x0008
#define FLAG_NOAID          0x0010
#define FLAG_INVIS          0x0020
#define FLAG_CONSUMING_UNIT     0x0040
#define FLAG_CONSUMING_FACTION      0x0080
#define FLAG_NOSPOILS           0x0100
#define FLAG_FLYSPOILS          0x0200
#define FLAG_WALKSPOILS         0x0400
#define FLAG_RIDESPOILS         0x0800
#define FLAG_SHARING            0x1000
#define FLAG_SWIMSPOILS         0x2000
#define FLAG_SAILSPOILS         0x4000

class UnitId {
    public:
        UnitId();
        ~UnitId();
        std::string Print();

        int unitnum; /* if 0, it is a new unit */
        int alias;
        int faction;

        bool operator==(const UnitId& other) const {
            return unitnum == other.unitnum && alias == other.alias && faction == other.faction;
        }
};

// Specialize std::hash for UnitId
namespace std {
    template <>
    struct hash<UnitId> {
        std::size_t operator()(const UnitId& uid) const {
            return std::hash<int>()(uid.unitnum) ^ std::hash<int>()(uid.alias) ^ std::hash<int>()(uid.faction);
        }
    };
}

class Unit {
    public:
        Unit();
        Unit(int,Faction *,int = 0);
        ~Unit();

        void SetMonFlags();
        void MakeWMon(char const *,int,int);

        void Writeout(std::ostream& f);
        void Readin(std::istream& f, std::list<Faction *>& facs);

        AString SpoilsReport(void);
        int CanGetSpoil(Item *i);
        json build_json_descriptor();
        void build_json_report(
            json& j, int obs, int truesight, int detfac, int autosee, AttitudeType attitude, bool showattitudes
        );
        json write_json_orders();
        std::string get_name(int observation);
        AString MageReport();
        AString ReadyItem();
        AString StudyableSkills();
        std::string battle_report(int observation);

        void ClearOrders();
        void ClearCastOrders();
        void DefaultOrders(Object *);
        void set_name(const std::string& newname);
        void set_description(const std::string& newdescription);
        void PostTurn(ARegion *reg);

        int IsLeader();
        int IsNormal();
        int GetMons();
        int GetMen();
        int GetLeaders();
        int GetSoldiers();
        int GetMen(int);
        void SetMen(int,int);
        int GetMoney();
        void SetMoney(int);
        int GetSharedNum(int);
        void ConsumeShared(int,int);
        int GetSharedMoney();
        void ConsumeSharedMoney(int);
        int IsAlive();

        int MaintCost(ARegionList& regions, ARegion *current_region);
        void Short(int, int);
        int SkillLevels();
        void SkillStarvation();
        Skill *GetSkillObject(int);

        int GetAttackRiding();
        int GetDefenseRiding();

        //
        // These are rule-set specific, in extra.cpp.
        //
        // LLS
        int GetAttribute(char const *ident);
        int PracticeAttribute(char const *ident);
        int GetProductionBonus(int);

        int GetSkill(int);
        void SetSkill(int,int);
        int GetSkillMax(int);
        int GetAvailSkill(int);
        int GetRealSkill(int);
        void ForgetSkill(int);
        int CheckDepend(int,SkillDepend &s);
        int CanStudy(int);
        int Study(int,int); /* Returns 1 if it succeeds */
        int Practice(int);
        void AdjustSkills();

        /* Return 1 if can see, 2 if can see faction */
        int CanSee(ARegion *,Unit *, int practice = 0);
        int CanCatch(ARegion *,Unit *);
        int AmtsPreventCrime(Unit *);
        AttitudeType GetAttitude(ARegion *,Unit *); /* Get this unit's attitude toward
                                              the Unit parameter */
        int Hostile();
        int Forbids(ARegion *,Unit *);
        int Weight();
        int FlyingCapacity();
        int RidingCapacity();
        int SwimmingCapacity();
        int WalkingCapacity();
        int CanFly(int);
        int CanRide(int);
        int CanWalk(int);
        int CanFly();
        int CanSwim();
        int CanReallySwim();
        int MoveType(ARegion *r = 0);
        int CalcMovePoints(ARegion *r = 0);
        int CanMoveTo(ARegion *,ARegion *);
        int GetFlag(int);
        void SetFlag(int,int);
        void CopyFlags(Unit *);
        int get_battle_item(const std::string &item);
        int get_armor(const std::string &item, int ass);
        int get_mount(const std::string &item, int canFly, int canRide, int &bonus);
        int get_weapon(
            const std::string &item, int riding, int ridingBonus, int &attackBonus, int &defenseBonus,
            int &attacks, int &hitDamage
        );
        int CanUseWeapon(const WeaponType& weapon, int riding);
        int Taxers(int);

        void MoveUnit( Object *newobj );
        void DiscardUnfinishedShips();

        void event(const std::string& message, const std::string& category, ARegion *r = nullptr);
        void error(const std::string& message);

        Faction *faction;
        Faction *formfaction;
        Object *object;
        std::string name;
        std::string describe;
        int num;
        int type;
        int alias;
        int gm_alias; /* used for gm manual creation of new units */
        int guard; /* Also, avoid- see enum above */
        int reveal;
        int flags;
        int taxing;
        int movepoints;
        int canattack;
        int nomove;
        int routed;
        SkillList skills;
        ItemList items;
        ItemList transport_items;
        int combat;
        int readyItem;
        int readyWeapon[MAX_READY];
        int readyArmor[MAX_READY];
        std::list<std::string> oldorders;
        int needed; /* For assessing maintenance */
        int hunger;
        int stomach_space;
        int losses;
        int free;
        int practiced; // Has this unit practiced a skill this turn
        int moved;
        int phase;
        int savedmovement;
        int savedmovedir;

        /* Orders */
        int destroy;
        int enter;
        int build;
        UnitId *promote;
        std::list<FindOrder *> findorders;
        std::list<GiveOrder *> giveorders;
        std::list<WithdrawOrder *> withdraworders;
        std::list<BuyOrder *> buyorders;
        std::list<SellOrder *> sellorders;
        std::list<ForgetOrder *> forgetorders;
        CastOrder *castorders;
        TeleportOrder *teleportorders;
        StealthOrder *stealthorders;
        Order *monthorders;
        AttackOrder *attackorders;
        EvictOrder *evictorders;
        SacrificeOrder *sacrificeorders;
        std::list<AnnihilateOrder *>annihilateorders;
        ARegion *advancefrom;

        std::list<ExchangeOrder *> exchangeorders;
        std::list<TurnOrder *> turnorders;
        int inTurnBlock;
        Order *presentMonthOrders;
        int presentTaxing;
        std::list<TransportOrder *>transportorders;
        JoinOrder *joinorders;
        Unit *former;
        bool form_repeated;

        // Used for tracking VISIT quests
        std::set<std::string> visited;
        int raised;

        // Used for tracking movement for NO7 victory condition
        ARegion *initial_region;
};

#endif // UNIT_H
