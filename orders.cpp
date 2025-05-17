#include "orders.h"
#include "unit.h"

const std::vector<std::string> OrderStrs = {
    "#atlantis",
    "#end",
    "unit",
    "address",
    "advance",
    "annihilate",
    "armor",
    "assassinate",
    "attack",
    "autotax",
    "avoid",
    "behind",
    "build",
    "buy",
    "cast",
    "claim",
    "combat",
    "consume",
    "declare",
    "describe",
    "destroy",
    "distribute",
    "end",
    "endturn",
    "enter",
    "entertain",
    "evict",
    "exchange",
    "faction",
    "find",
    "forget",
    "form",
    "give",
    "guard",
    "hold",
    "idle",
    "join",
    "leave",
    "move",
    "name",
    "noaid",
    "nocross",
    "nospoils",
    "option",
    "password",
    "pillage",
    "prepare",
    "produce",
    "promote",
    "quit",
    "restart",
    "reveal",
    "sacrifice",
    "sail",
    "sell",
    "share",
    "show",
    "spoils",
    "steal",
    "study",
    "take",
    "tax",
    "teach",
    "transport",
    "turn",
    "weapon",
    "withdraw",
    "work",
};


int Parse1Order(const parser::token& token)
{
    for (int i = 0; i < NORDERS; i++)
        if (token == OrderStrs[i]) return i;
    return -1;
}

Order::Order()
{
    type = NORDERS;
    quiet = 0;
}

Order::~Order() {}

ExchangeOrder::ExchangeOrder()
{
    type = O_EXCHANGE;
    exchangeStatus = -1;
}

ExchangeOrder::~ExchangeOrder()
{
    if (target) delete target;
}

TurnOrder::TurnOrder()
{
    type = O_TURN;
    repeating = 0;
}

TurnOrder::~TurnOrder() {}

MoveOrder::MoveOrder() { type = O_MOVE; }

MoveOrder::~MoveOrder() {
    std::for_each(dirs.begin(), dirs.end(), [](MoveDir *dir) { delete dir; });
}

ForgetOrder::ForgetOrder() { type = O_FORGET; }

ForgetOrder::~ForgetOrder() {}

WithdrawOrder::WithdrawOrder() { type = O_WITHDRAW; }

WithdrawOrder::~WithdrawOrder() {}

GiveOrder::GiveOrder()
{
    type = O_GIVE;
    unfinished = 0;
    merge = 0;
}

GiveOrder::~GiveOrder()
{
    if (target) delete target;
}

StudyOrder::StudyOrder() { type = O_STUDY; }

StudyOrder::~StudyOrder() {}

TeachOrder::TeachOrder() { type = O_TEACH; }

TeachOrder::~TeachOrder() {
    std::for_each(targets.begin(), targets.end(), [](UnitId *id) { delete id; });
}

ProduceOrder::ProduceOrder() { type = O_PRODUCE; }

ProduceOrder::~ProduceOrder() {}

BuyOrder::BuyOrder() { type = O_BUY; }

BuyOrder::~BuyOrder() {}

SellOrder::SellOrder() { type = O_SELL; }

SellOrder::~SellOrder() {}

AttackOrder::AttackOrder() { type = O_ATTACK; }

AttackOrder::~AttackOrder() {
    std::for_each(targets.begin(), targets.end(), [](UnitId *id) { delete id; });
}

BuildOrder::BuildOrder()
{
    type = O_BUILD;
    new_building = -1;
    until_complete = false;
    target = nullptr;
    needtocomplete = 0;
}

BuildOrder::~BuildOrder()
{
    if (target) delete target;
}

SailOrder::SailOrder() { type = O_SAIL; }

SailOrder::~SailOrder() {
    std::for_each(dirs.begin(), dirs.end(), [](MoveDir *dir) { delete dir; });
}

FindOrder::FindOrder() { type = O_FIND; }

FindOrder::~FindOrder() {}

StealthOrder::StealthOrder() {}

StealthOrder::~StealthOrder() {}

StealOrder::StealOrder()
{
    type = O_STEAL;
    target = nullptr;
}

StealOrder::~StealOrder()
{
    if (target) delete target;
}

AssassinateOrder::AssassinateOrder() { type = O_ASSASSINATE; }

AssassinateOrder::~AssassinateOrder()
{
    if (target) delete target;
}

CastOrder::CastOrder() { type = O_CAST; }

CastOrder::~CastOrder() {}

CastMindOrder::CastMindOrder() { id = nullptr; }

CastMindOrder::~CastMindOrder() {
    if (id) delete id;
}

TeleportOrder::TeleportOrder() {}

TeleportOrder::~TeleportOrder() {
    std::for_each(units.begin(), units.end(), [](UnitId *id) { delete id; });
}

CastRegionOrder::CastRegionOrder() {}

CastRegionOrder::~CastRegionOrder() {}

CastIntOrder::CastIntOrder() {}

CastIntOrder::~CastIntOrder() {}

CastUnitsOrder::CastUnitsOrder() {}

CastUnitsOrder::~CastUnitsOrder() {
    std::for_each(units.begin(), units.end(), [](UnitId *id) { delete id; });
}

EvictOrder::EvictOrder() { type = O_EVICT; }

EvictOrder::~EvictOrder() {
    std::for_each(targets.begin(), targets.end(), [](UnitId *id) { delete id; });
}

IdleOrder::IdleOrder() { type = O_IDLE; }

IdleOrder::~IdleOrder() {}

TransportOrder::TransportOrder()
{
    type = O_TRANSPORT;
    item = -1;
    amount = 0;
    except = 0;
    target = NULL;
}

TransportOrder::~TransportOrder()
{
    if (target) delete target;
}

CastTransmuteOrder::CastTransmuteOrder() {}

CastTransmuteOrder::~CastTransmuteOrder() {}

JoinOrder::JoinOrder() { type = O_JOIN; }

JoinOrder::~JoinOrder()
{
    if (target) delete target;
}

AnnihilateOrder::AnnihilateOrder() { type = O_ANNIHILATE; }

AnnihilateOrder::~AnnihilateOrder() {}

SacrificeOrder::SacrificeOrder() { type = O_SACRIFICE; }

SacrificeOrder::~SacrificeOrder() {}
