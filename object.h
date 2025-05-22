#pragma once
#ifndef OBJECT_H
#define OBJECT_H

class Object;

#include "gamedefs.h"
#include "faction.h"
#include "items.h"
#include "safe_list.h"
#include "string_parser.hpp"
#include <map>
#include <list>

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

#define I_WOOD_OR_STONE -2

struct SacrificeEffect {
    int granted_item;
    int granted_amount;
    int replace_object;
    bool destroyed;
};

class ObjectType {
    public:
        std::string name;
        enum {
            DISABLED    = 0x001,
            NOMONSTERGROWTH = 0x002,
            NEVERDECAY  = 0x004,
            CANENTER    = 0x008,
            CANMODIFY   = 0x020,
            TRANSPORT   = 0x040,
            GROUP       = 0x080,
            KEYBARRIER  = 0x100,  // Prevents entry to region except to unit with key
            SACRIFICE   = 0x200,  // This object requires the sacrifice command
            GRANTSKILL  = 0x400,  // This object grants a skill to the owner
            NOANNIHILATE = 0x800, // This object cannot be annihilated
        };
        int flags;

        int protect;
        int capacity;
        int sailors;
        int maxMages;

        int item;
        int cost;
        char const *skill;
        int level;

        int key_item; // item needed to enter region containing this object if it's a key barrier
        int sacrifice_item; // item needed to sacrice to this object if it's a sacrifice object
        int sacrifice_amount; // amount of item needed to sacrifice to this object if it's a sacrifice object
        int granted_skill; // skill granted by this object to the owner
        int granted_level; // level of skill granted by this object to the owner
        SacrificeEffect sacrifice_effect; // effect of sacrificing to this object

        int maxMaintenance;
        int maxMonthlyDecay;
        int maintFactor;

        int monster;

        int productionAided;
        int defenceArray[NUM_ATTACK_TYPES];
};

extern std::vector<ObjectType> ObjectDefs;

std::string object_description(int obj);

int lookup_object(const strings::ci_string& token);
int parse_object(const parser::token& token, bool match_ships);

int ObjectIsShip(int);

struct ShowObject {
    int obj;
};


class Object
{
    public:
        Object(ARegion *region);
        ~Object();

        void Readin(std::istream& f, std::list<Faction *>& facs);
        void Writeout(std::ostream& f);
        void build_json_report(json& j, Faction *, int, int, int, int, int, int, int);

        void set_name(const std::string& newname, Unit *actor = nullptr);
        void set_description(const std::string& newdescription);

        Unit *GetUnit(int);
        Unit *GetUnitAlias(int, int); /* alias, faction number */
        Unit *GetUnitId(UnitId *, int);

        // AS
        int IsRoad();

        int IsFleet();
        int IsBuilding();
        int CanModify();
        int CanEnter(ARegion *, Unit *);
        Unit *ForbiddenBy(ARegion *, Unit *);
        Unit *GetOwner();

        void SetPrevDir(int);
        void MoveObject(ARegion *toreg);

        // Fleets
        void ReadinFleet(std::istream &f);
        void WriteoutFleet(std::ostream &f);
        int CheckShip(int);
        int GetNumShips(int);
        void SetNumShips(int, int);
        void AddShip(int);
        AString FleetDefinition();
        int FleetCapacity();
        int FleetLoad();
        int SailThroughCheck(int dir);
        int FleetSailingSkill(int);
        int GetFleetSize();
        int GetFleetSpeed(int);

        std::string name;
        std::string describe;
        ARegion *region;
        int inner;
        int num;
        int type;
        int incomplete;
        int capacity;
        int flying;
        int load;
        int runes;
        int prevdir;
        int mages;
        int shipno;
        int movepoints;
        int destroyed;  // how much points was destroyed so far this turn
        safe::list<Unit *> units;
        ItemList ships;
};

#endif // OBJECT_H
