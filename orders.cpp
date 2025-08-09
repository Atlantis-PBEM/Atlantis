#include "orders.h"
#include "unit.h"
#include "gamedata.h"

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

Order::Order(int type, int quiet)
    : type(type), quiet(quiet)
{
}

ExchangeOrder::~ExchangeOrder()
{
    if (target) delete target;
}

MoveOrder::~MoveOrder() {
    std::for_each(dirs.begin(), dirs.end(), [](MoveDir *dir) { delete dir; });
}

GiveOrder::~GiveOrder()
{
    if (target) delete target;
}

TeachOrder::~TeachOrder() {
    std::for_each(targets.begin(), targets.end(), [](UnitId *id) { delete id; });
}

AttackOrder::~AttackOrder() {
    std::for_each(targets.begin(), targets.end(), [](UnitId *id) { delete id; });
}

BuildOrder::~BuildOrder()
{
    if (target) delete target;
}

SailOrder::~SailOrder() {
    std::for_each(dirs.begin(), dirs.end(), [](MoveDir *dir) { delete dir; });
}

StealthOrder::StealthOrder(int type, UnitId *target)
    : Order(type), target(target)
{
}

StealthOrder::~StealthOrder() {
    if (target) delete target;
}

CastMindOrder::CastMindOrder(int level, UnitId *id)
    : CastOrder(S_MIND_READING, level), id(id) {
}

CastMindOrder::~CastMindOrder() {
    if (id) delete id;
}

TeleportOrder::TeleportOrder(int spell, int level, int x, int y, int z)
    : CastRegionOrder(spell, level, x, y, z), gate(0) {
}

TeleportOrder::~TeleportOrder() {
    std::for_each(units.begin(), units.end(), [](UnitId *id) { delete id; });
}

CastUnitsOrder::CastUnitsOrder(int level)
    : CastOrder(S_INVISIBILITY, level) {
}

CastUnitsOrder::~CastUnitsOrder() {
    std::for_each(units.begin(), units.end(), [](UnitId *id) { delete id; });
}

EvictOrder::~EvictOrder() {
    std::for_each(targets.begin(), targets.end(), [](UnitId *id) { delete id; });
}

TransportOrder::~TransportOrder()
{
    if (target) delete target;
}

JoinOrder::~JoinOrder()
{
    if (target) delete target;
}

CastTransmuteOrder::CastTransmuteOrder(int level, int item, int number)
    : CastOrder(S_TRANSMUTATION, level), item(item), number(number) {
}

CastRegionOrder::CastRegionOrder(int spell, int level, int x, int y, int z)
    : CastOrder(spell, level), xloc(x), yloc(y), zloc(z) {
}

CastIntOrder::CastIntOrder(int spell, int level, int target)
    : CastOrder(spell, level), target(target) {
}
