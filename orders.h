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
    Order();
    virtual ~Order();

    int type;
    int quiet;
};

class MoveDir {
  public:
    int dir;
};

class MoveOrder : public Order {
  public:
    MoveOrder();
    ~MoveOrder();

    int advancing;
    std::list<MoveDir *> dirs;
};

class WithdrawOrder : public Order {
  public:
    WithdrawOrder();
    ~WithdrawOrder();

    int item;
    int amount;
};

class GiveOrder : public Order {
  public:
    GiveOrder();
    ~GiveOrder();

    int item;
    /* if amount == -1, transfer whole unit, -2 means all of item */
    int amount;
    int except;
    int unfinished;
    int merge;

    UnitId *target;
};

class StudyOrder : public Order {
  public:
    StudyOrder();
    ~StudyOrder();

    int skill;
    int days;
    int level;
};

class TeachOrder : public Order {
  public:
    TeachOrder();
    ~TeachOrder();

    std::list<UnitId *> targets;
};

class ProduceOrder : public Order {
  public:
    ProduceOrder();
    ~ProduceOrder();

    int item;
    int skill; /* -1 for none */
    int productivity;
    int target;
};

class BuyOrder : public Order {
  public:
    BuyOrder();
    ~BuyOrder();

    int item;
    int num;
    bool repeating;
};

class SellOrder : public Order {
  public:
    SellOrder();
    ~SellOrder();

    int item;
    int num;
    bool repeating;
};

class AttackOrder : public Order {
  public:
    AttackOrder();
    ~AttackOrder();

    std::list<UnitId *> targets;
};

class BuildOrder : public Order {
  public:
    BuildOrder();
    ~BuildOrder();

    UnitId *target;
    int new_building;
    int needtocomplete;
    bool until_complete;
};

class SailOrder : public Order {
  public:
    SailOrder();
    ~SailOrder();

    std::list<MoveDir *> dirs;
};

class FindOrder : public Order {
  public:
    FindOrder();
    ~FindOrder();

    int find;
};

class StealthOrder : public Order {
  public:
    StealthOrder();
    ~StealthOrder();

    UnitId *target;
};

class StealOrder : public StealthOrder {
  public:
    StealOrder();
    ~StealOrder();

    int item;
};

class AssassinateOrder : public StealthOrder {
  public:
    AssassinateOrder();
    ~AssassinateOrder();
};

class ForgetOrder : public Order {
  public:
    ForgetOrder();
    ~ForgetOrder();

    int skill;
};

// Add class for exchange
class ExchangeOrder : public Order {
  public:
    ExchangeOrder();
    ~ExchangeOrder();

    int giveItem;
    int giveAmount;
    int expectItem;
    int expectAmount;

    int exchangeStatus;

    UnitId *target;
};

class TurnOrder : public Order {
  public:
    TurnOrder();
    ~TurnOrder();
    bool repeating;
    std::vector<std::string> turnOrders;
};

class CastOrder : public Order {
  public:
    CastOrder();
    ~CastOrder();

    int spell;
    int level;
};

class CastMindOrder : public CastOrder {
  public:
    CastMindOrder();
    ~CastMindOrder();

    UnitId *id;
};

class CastRegionOrder : public CastOrder {
  public:
    CastRegionOrder();
    ~CastRegionOrder();

    int xloc, yloc, zloc;
};

class TeleportOrder : public CastRegionOrder {
  public:
    TeleportOrder();
    ~TeleportOrder();

    int gate;
    std::list<UnitId *> units;
};

class CastIntOrder : public CastOrder {
  public:
    CastIntOrder();
    ~CastIntOrder();

    int target;
};

class CastUnitsOrder : public CastOrder {
  public:
    CastUnitsOrder();
    ~CastUnitsOrder();

    std::list<UnitId *> units;
};

class CastTransmuteOrder : public CastOrder {
  public:
    CastTransmuteOrder();
    ~CastTransmuteOrder();

    int item;
    int number;
};

class EvictOrder : public Order {
  public:
    EvictOrder();
    ~EvictOrder();

    std::list<UnitId *> targets;
};

class IdleOrder : public Order {
  public:
    IdleOrder();
    ~IdleOrder();
};

class TransportOrder : public Order {
  public:
    TransportOrder();
    ~TransportOrder();

    int item;
    // amount == -1 means all available at transport time
    // any other amount is also checked at transport time
    int amount;
    int except;
    int distance;

    enum class TransportPhase {
        SHIP_TO_QM,
        INTER_QM_TRANSPORT,
        DISTRIBUTE_FROM_QM
    };
    TransportPhase phase;

    UnitId *target;
};

class JoinOrder : public Order {
  public:
    JoinOrder();
    ~JoinOrder();

    UnitId *target;
    int overload;
    int merge;
};

class AnnihilateOrder : public Order {
  public:
    AnnihilateOrder();
    ~AnnihilateOrder();

    int xloc, yloc, zloc;
};

class SacrificeOrder : public Order {
  public:
    SacrificeOrder();
    ~SacrificeOrder();

    int item;
    int amount;
};

#endif // ORDERS_H
