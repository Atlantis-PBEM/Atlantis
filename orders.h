#pragma once
#ifndef ORDERS_H
#define ORDERS_H

#include "gamedefs.h"
#include <list>
#include "string_parser.hpp"

class UnitId;

enum {
    O_ATLANTIS,
    O_END,
    O_UNIT,
    O_ADDRESS,
    O_ADVANCE,
    O_ANNIHILATE,
    O_ARMOR,
    O_ASSASSINATE,
    O_ATTACK,
    O_AUTOTAX,
    O_AVOID,
    O_BEHIND,
    O_BUILD,
    O_BUY,
    O_CAST,
    O_CLAIM,
    O_COMBAT,
    O_CONSUME,
    O_DECLARE,
    O_DESCRIBE,
    O_DESTROY,
    O_DISTRIBUTE,
    O_ENDFORM,
    O_ENDTURN,
    O_ENTER,
    O_ENTERTAIN,
    O_EVICT,
    O_EXCHANGE,
    O_FACTION,
    O_FIND,
    O_FORGET,
    O_FORM,
    O_GIVE,
    O_GUARD,
    O_HOLD,
    O_IDLE,
    O_JOIN,
    O_LEAVE,
    O_MOVE,
    O_NAME,
    O_NOAID,
    O_NOCROSS,
    O_NOSPOILS,
    O_OPTION,
    O_PASSWORD,
    O_PILLAGE,
    O_PREPARE,
    O_PRODUCE,
    O_PROMOTE,
    O_QUIT,
    O_RESTART,
    O_REVEAL,
    O_SACRIFICE,
    O_SAIL,
    O_SELL,
    O_SHARE,
    O_SHOW,
    O_SPOILS,
    O_STEAL,
    O_STUDY,
    O_TAKE,
    O_TAX,
    O_TEACH,
    O_TRANSPORT,
    O_TURN,
    O_WEAPON,
    O_WITHDRAW,
    O_WORK,
    NORDERS
};

enum {
    M_NONE,
    M_WALK,
    M_RIDE,
    M_FLY,
    M_SWIM,
    M_SAIL
};

#define MOVE_PAUSE 97
#define MOVE_IN 98
#define MOVE_OUT 99
/* Enter is MOVE_ENTER + num of object */
#define MOVE_ENTER 100

extern const std::vector<std::string> OrderStrs;

int Parse1Order(const parser::token& token);

class Order {
  public:
    Order(int type = 0, int quiet = 0);
    virtual ~Order() = default;

    int type;
    int quiet;
};

struct MoveDir {
    int dir = 0;
};

class MoveOrder : public Order {
  public:
    MoveOrder(bool advance = false) : Order(O_MOVE), advancing(advance) {}
    virtual ~MoveOrder();

    bool advancing;
    std::list<MoveDir *> dirs;
};

class WithdrawOrder : public Order {
  public:
    WithdrawOrder(int item, int amount)
        : Order(O_WITHDRAW), item(item), amount(amount) {};
    virtual ~WithdrawOrder() = default;

    int item = 0;
    int amount = 0;
};

class GiveOrder : public Order {
  public:
    GiveOrder(int type = O_GIVE) : Order(type) {}
    virtual ~GiveOrder();

    int item = 0;
    /* if amount == -1, transfer whole unit, -2 means all of item */
    int amount = 0;
    int except = 0;
    int unfinished = 0;
    int merge = 0;

    UnitId *target = nullptr;
};

class StudyOrder : public Order {
  public:
    StudyOrder(int skill)
        : Order(O_STUDY), skill(skill) {}
    virtual ~StudyOrder() = default;

    int skill;
    int days = 0;
    int level = -1;
};

class TeachOrder : public Order {
  public:
    TeachOrder() : Order(O_TEACH) {}
    virtual ~TeachOrder();

    std::list<UnitId *> targets;
};

class ProduceOrder : public Order {
  public:
      ProduceOrder(int item, int skill, int target, int quiet = 0)
          : Order(O_PRODUCE, quiet), item(item), skill(skill), target(target) {
    }
    virtual ~ProduceOrder() = default;

    int item;
    int skill; /* -1 for none */
    int target;
    int productivity = 0;
};

class BuyOrder : public Order {
  public:
    BuyOrder(int item, int num = 0, bool repeating = false)
        : Order(O_BUY), item(item), num(num), repeating(repeating) {
    }

    int item;
    int num;
    bool repeating;
};

class SellOrder : public Order {
  public:
    SellOrder(int item, int num = 0, bool repeating = false)
        : Order(O_SELL), item(item), num(num), repeating(repeating) {
    }

    int item;
    int num;
    bool repeating;
};

class AttackOrder : public Order {
  public:
    AttackOrder() : Order(O_ATTACK) {}
    virtual ~AttackOrder();

    std::list<UnitId *> targets;
};

class BuildOrder : public Order {
  public:
    BuildOrder(UnitId *target = nullptr) : Order(O_BUILD), target(target), needtocomplete(0) {}
    BuildOrder(int needtocomplete) : Order(O_BUILD), target(nullptr), needtocomplete(needtocomplete) {}
    virtual ~BuildOrder();

    UnitId *target;
    int new_building = -1;
    int needtocomplete;
    bool until_complete = false;
};

class SailOrder : public Order {
  public:
    SailOrder() : Order(O_SAIL) {}
    virtual ~SailOrder();

    std::list<MoveDir *> dirs;
};

class FindOrder : public Order {
  public:
    FindOrder(int find) : Order(O_FIND), find(find) {}
    virtual ~FindOrder() = default;

    int find;
};

class StealthOrder : public Order {
  public:
    StealthOrder(int type = 0, UnitId *target = nullptr);
    virtual ~StealthOrder();

    UnitId *target;
};

class StealOrder : public StealthOrder {
  public:
    StealOrder(UnitId *target, int item)
        : StealthOrder(O_STEAL, target), item(item) {}
    virtual ~StealOrder() = default;

    int item;
};

class AssassinateOrder : public StealthOrder {
  public:
      AssassinateOrder(UnitId * target = nullptr) : StealthOrder(O_ASSASSINATE, target) {}
      virtual ~AssassinateOrder() = default;
};

class ForgetOrder : public Order {
  public:
    ForgetOrder(int skill) : Order(O_FORGET), skill(skill) {}
    virtual ~ForgetOrder() = default;

    int skill;
};

// Add class for exchange
class ExchangeOrder : public Order {
  public:
    ExchangeOrder(UnitId *target = nullptr) : Order(O_EXCHANGE), target(target) {};
    virtual ~ExchangeOrder();

    int giveItem = 0;
    int giveAmount = 0;
    int expectItem = 0;
    int expectAmount = 0;

    int exchangeStatus = -1;

    UnitId *target;
};

class TurnOrder : public Order {
  public:
    TurnOrder(bool repeating = false) : Order(O_TURN), repeating(repeating) {};
    virtual ~TurnOrder() = default;

    bool repeating;
    std::vector<std::string> turnOrders;
};

class CastOrder : public Order {
  public:
    CastOrder(int spell, int level) : Order(O_CAST), spell(spell), level(level) {};
    virtual ~CastOrder() = default;

    int spell;
    int level;
};

class CastMindOrder : public CastOrder {
  public:
    CastMindOrder(int level, UnitId *id = nullptr);
    virtual ~CastMindOrder();

    UnitId *id;
};

class CastRegionOrder : public CastOrder {
  public:
    CastRegionOrder(int spell, int level, int x = 0, int y = 0, int z = 0);
    virtual ~CastRegionOrder() = default;

    int xloc, yloc, zloc;
};

class TeleportOrder : public CastRegionOrder {
  public:
    TeleportOrder(int spell, int level, int x = 0, int y = 0, int z = 0);
    virtual ~TeleportOrder();

    int gate;
    std::list<UnitId *> units;
};

class CastIntOrder : public CastOrder {
  public:
    CastIntOrder(int spell, int level, int target);
    virtual ~CastIntOrder() = default;

    int target;
};

class CastUnitsOrder : public CastOrder {
  public:
    CastUnitsOrder(int level);
    virtual ~CastUnitsOrder();

    std::list<UnitId *> units;
};

class CastTransmuteOrder : public CastOrder {
  public:
    CastTransmuteOrder(int level, int item, int number);
    virtual ~CastTransmuteOrder() = default;

    int item;
    int number;
};

class EvictOrder : public Order {
  public:
    EvictOrder() : Order(O_EVICT) {};
    virtual ~EvictOrder();

    std::list<UnitId *> targets;
};

class IdleOrder : public Order {
  public:
    IdleOrder() : Order(O_IDLE) {};
    virtual ~IdleOrder() = default;
};

class TransportOrder : public Order {
  public:
    TransportOrder(int item, int amount, int except, UnitId *target = nullptr)
        : Order(O_TRANSPORT), item(item), amount(amount), except(except), target(target)
    {
    };
    virtual ~TransportOrder();

    int item;
    // amount == -1 means all available at transport time
    // any other amount is also checked at transport time
    int amount;
    int except;
    int distance = 0;

    enum class TransportPhase {
        UNDEFINED,
        SHIP_TO_QM,
        INTER_QM_TRANSPORT,
        DISTRIBUTE_FROM_QM
    };
    TransportPhase phase = TransportPhase::UNDEFINED;

    UnitId *target;
};

class JoinOrder : public Order {
  public:
    JoinOrder(int overload, int merge, UnitId *target = nullptr)
        : Order(O_JOIN), overload(overload), merge(merge), target(target) {};
    virtual ~JoinOrder();

    int overload;
    int merge;
    UnitId *target;
};

class AnnihilateOrder : public Order {
  public:
    AnnihilateOrder(int x, int y, int z)
        : Order(O_ANNIHILATE), xloc(x), yloc(y), zloc(z) {};
    virtual ~AnnihilateOrder() = default;

    int xloc, yloc, zloc;
};

class SacrificeOrder : public Order {
  public:
    SacrificeOrder(int item = 0, int amount = 0)
        : Order(O_SACRIFICE), item(item), amount(amount) {};
    virtual ~SacrificeOrder() = default;

    int item;
    int amount;
};

#endif // ORDERS_H
