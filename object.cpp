#include "object.h"
#include "items.h"
#include "skills.h"
#include "gamedata.h"
#include "unit.h"
#include "indenter.hpp"
#include "string_parser.hpp"
#include "string_filters.hpp"
#include "strings_util.hpp"

int lookup_object(const strings::ci_string& token)
{
    for (int i = 0; i < NOBJECTS; i++) {
        if (token == ObjectDefs[i].name) return i;
    }
    return -1;
}

/* parse_object checks for matching Object types AND
 * for matching ship-type items (which are also
 * produced using the build order) if the ships
 * argument is given.
 */
int parse_object(const parser::token& token, bool match_ships)
{
    // Check for ship-type items:
    if (match_ships) {
        for (int i = 0; i < NITEMS; i++) {
            if (ItemDefs[i].flags & ItemType::DISABLED) continue;
            if (ItemDefs[i].type & IT_SHIP && (token == ItemDefs[i].name || token == ItemDefs[i].abr))
                return -(i + 1);
        }
    }
    for (int i = O_DUMMY + 1; i < NOBJECTS; i++) {
        if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
        if (token == ObjectDefs[i].name) return i;
    }

    return -1;
}

int ObjectIsShip(int ot)
{
    if (ObjectDefs[ot].capacity) return 1;
    return 0;
}

Object::Object(ARegion *reg)
{
    num = 0;
    type = O_DUMMY;
    name = "Dummy";
    incomplete = 0;
    capacity = 0;
    mages = 0;
    inner = -1;
    runes = 0;
    region = reg;
    prevdir = -1;
    flying = 0;
    destroyed = 0;
    movepoints = Globals->PHASED_MOVE_OFFSET % Globals->MAX_SPEED;
    ships.clear();
}

Object::~Object()
{
    region = (ARegion *)NULL;
    std::for_each(units.begin(), units.end(), [](Unit *unit) { delete unit; });
    units.clear();
}

void Object::Writeout(std::ostream& f)
{
    f << num << '\n';
    f << (type == -1
     ? "NO_OBJECT"
     : (IsFleet() ? ObjectDefs[O_FLEET].name : ObjectDefs[type].name)
    ) << '\n';

    f << incomplete << '\n';
    f << name << '\n';
    f << (describe.empty() ? "none" : describe) << '\n';
    f << inner << '\n';
    f << (Globals->PREVENT_SAIL_THROUGH && !Globals->ALLOW_TRIVIAL_PORTAGE ? prevdir : -1) << '\n';
    f << runes << '\n';
    f << units.size() << '\n';
    for(const auto u : units) u->Writeout(f);
    WriteoutFleet(f);
}

void Object::Readin(std::istream& f, std::list<Faction *>& facs)
{
    f >> num;

    std::string str;
    std::getline(f >> std::ws, str);
    type = lookup_object(str);

    f >> incomplete;

    std::getline(f >> std::ws, str);
    set_name(str | filter::strip_number);


    std::getline(f >> std::ws, str);
    describe = str | filter::legal_characters;
    if (describe == "none") describe.clear();

    f >> inner;
    f >> prevdir;
    f >> runes;

    // Now, fix up a save file if ALLOW_TRIVIAL_PORTAGE is allowed, just
    // in case it wasn't when the save file was made.
    if (Globals->ALLOW_TRIVIAL_PORTAGE) prevdir = -1;
    int i;
    f >> i;
    for (int j = 0; j < i; j++) {
        Unit *temp = new Unit;
        temp->Readin(f, facs);
        if (!temp->faction) continue;
        temp->MoveUnit(this);
        if (!(temp->faction->is_npc)) region->visited = 1;
    }
    mages = ObjectDefs[type].maxMages;
    ReadinFleet(f);
}

void Object::set_name(const std::string& newname, Unit *actor)
{
    if (newname.empty()) return;
    if (type == O_DUMMY) return; // Cannot modify dummy objects, ever
    if (actor && !CanModify()) return;

    std::string temp = newname | filter::legal_characters;
    if (temp.empty()) return;

    name = temp + " [" + std::to_string(num) + "]";
}

void Object::set_description(const std::string& newdescription)
{
    if (!CanModify()) return;

    describe.clear();
    if (!newdescription.empty()) describe = newdescription | filter::legal_characters;
}

int Object::IsFleet()
{
    if (type == O_FLEET) return 1;
    if (ObjectDefs[type].sailors > 0) return 1;
    if (ships.size() > 0) return 1;
    return 0;
}

int Object::IsBuilding()
{
    if (ObjectDefs[type].protect)
        return 1;
    return 0;
}

int Object::CanModify() // TODO: make bool
{
    return (ObjectDefs[type].flags & ObjectType::CANMODIFY);
}

Unit *Object::GetUnit(int num)
{
    for(const auto u : units)
        if (u->num == num) return u;
    return nullptr;
}

Unit *Object::GetUnitAlias(int alias, int faction)
{
    // First search for units with the 'formfaction'
    for(const auto u : units)
        if (u->alias == alias && u->formfaction->num == faction) return u;

    for(const auto u : units)
        if (u->alias == alias && u->faction->num == faction) return u;
    return nullptr;
}

Unit *Object::GetUnitId(UnitId *id, int faction)
{
    if (id == 0) return 0;
    if (id->unitnum) {
        return GetUnit(id->unitnum);
    } else {
        if (id->faction) {
            return GetUnitAlias(id->alias, id->faction);
        } else {
            return GetUnitAlias(id->alias, faction);
        }
    }
}

int Object::CanEnter(ARegion *reg, Unit *u)
{
    if (!(ObjectDefs[type].flags & ObjectType::CANENTER) &&
            (u->type == U_MAGE || u->type == U_NORMAL ||
             u->type == U_APPRENTICE)) {
        return 0;
    }
    return 1;
}

Unit *Object::ForbiddenBy(ARegion *reg, Unit *u)
{
    Unit *owner = GetOwner();
    if (!owner || type == O_GATEWAY) {
        return(0);
    }

    if (owner->GetAttitude(reg, u) < AttitudeType::FRIENDLY) {
        return owner;
    }
    return 0;
}

Unit *Object::GetOwner()
{
    if(units.empty()) return nullptr;
    return units.front();
}

void Object::build_json_report(json& j, Faction *fac, int obs, int truesight,
    int detfac, int passobs, int passtrue, int passdetfac, int present)
{
    // Exit early if no observable.
    if ((type != O_DUMMY) && !present) {
        if (IsFleet() && !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_SHIPS)) return;
        if (IsBuilding() && !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_BUILDINGS)) return;
        if (IsRoad() && !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_ROADS)) return;
    }

    json container = json::object();
    if (type != O_DUMMY) {
        ObjectType& ob = ObjectDefs[type];

        container["name"] = name | filter::strip_number;
        container["number"] = num;
        container["type"] = ob.name;
        if (!describe.empty()) container["description"] = describe;

        if (IsFleet()) {
            container["fleet"] = true;
            if((GetOwner() && fac == GetOwner()->faction) || (obs > 9)){
                container["load"] = FleetLoad();
                container["capacity"] = FleetCapacity();
                container["sailors"] = FleetSailingSkill(1);
                container["fleet_size"] = GetFleetSize();
                container["max_speed"] = GetFleetSpeed(1);
                container["damage_percent"] = incomplete;
            }
            if (
                Globals->PREVENT_SAIL_THROUGH && !Globals->ALLOW_TRIVIAL_PORTAGE
                && (flying < 1) && (TerrainDefs[region->type].similar_type != R_OCEAN)
            ) {
                for (int dir = 0; dir < NDIRS; dir++) {
                    if (SailThroughCheck(dir) == 1) container["sail_directions"][DirectionAbrs[dir]] = true;
                }
            }
            for(auto ship : ships) {
                if (ship->type != -1 && ship->num > 0) {
                    ItemType item_def = ItemDefs[ship->type];
                    if (item_def.flags & ItemType::DISABLED) continue;
                    if (!(item_def.type & IT_SHIP)) continue;
                    container["ships"].push_back(
                        { {"name", item_def.name}, {"number", ship->num}, { "plural", item_def.names } }
                    );
                }
            }
        } else {
            if (incomplete > 0) container["incomplete"] = incomplete;
            if (inner != -1) container["inner_location"] = true;
            if (runes) container["warding_runes"] = true;
            if (!(ob.flags & ObjectType::CANENTER)) container["closed"] = true;
            if (Globals->DECAY && !(ob.flags & ObjectType::NEVERDECAY) && incomplete < 1) {
                if (incomplete > (0 - ob.maxMonthlyDecay)) {
                    container["decaying"] = true;
                } else if (incomplete > (0 - ob.maxMaintenance/2)) {
                    container["needs_maintenance"] = true;
                }
            }
            if (ob.flags & ObjectType::SACRIFICE && (incomplete < 0)) {
                ItemType& def = ItemDefs[ob.sacrifice_item];
                json item_obj = json{ { "name", def.name }, { "tag", def.abr }, { "plural", def.names } };
                item_obj["amount"] = -incomplete;
                container["sacrifice"] = item_obj;
            }
            if (ob.flags & ObjectType::GRANTSKILL) {
                container["grantskill"] = {
                    { "name", SkillDefs[ob.granted_skill].name },
                    { "tag", SkillDefs[ob.granted_skill].abbr },
                    { "level", ob.granted_level }
                };
            }
        }
    }

    json& unit_container = (type == O_DUMMY) ? j["units"] : container["units"];

    // Add units to container
    for(const auto u : units) {
        json unit = json::object();
        AttitudeType attitude = fac->get_attitude(u->faction->num);
        if (u->faction == fac) {
            u->build_json_report(unit, -1, 1, 1, 1, attitude, fac->showunitattitudes);
        } else {
            if (present) {
                u->build_json_report(unit, obs, truesight, detfac, type != O_DUMMY, attitude, fac->showunitattitudes);
            } else {
                if (((type == O_DUMMY) && (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_OUTDOOR_UNITS)) ||
                    ((type != O_DUMMY) && (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_INDOOR_UNITS)) ||
                    ((u->guard == GUARD_GUARD) && (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_GUARDS))) {
                    u->build_json_report(unit, passobs, passtrue, passdetfac, type != O_DUMMY, attitude, fac->showunitattitudes);
                }
            }
        }
        if (unit.empty()) continue;
        unit_container.push_back(unit);
    }

    if (type != O_DUMMY) {
        j["structures"].push_back(container);
    }
}


void Object::SetPrevDir(int newdir)
{
    prevdir = newdir;
}

void Object::MoveObject(ARegion *toreg)
{
    region->objects.remove(this);
    region = toreg;
    toreg->objects.push_back(this);
}

int Object::IsRoad()
{
    if (type >= O_ROADN && type <= O_ROADS) return 1;
    return 0;
}

/* Performs a basic check on items for ship-types.
 * (note: always fails for non-Fleet Objects.)
 */
int Object::CheckShip(int item)
{
    if (item < 0) return 0;
    if (!IsFleet()) return 0;
    if (ItemDefs[item].type & IT_SHIP) return 1;
    return 0;
}

void Object::WriteoutFleet(std::ostream& f)
{
    if (!IsFleet()) return;
    f << ships.size() << "\n";
    for(auto sh : ships) sh->Writeout(f);
}

void Object::ReadinFleet(std::istream &f)
{
    if (type != O_FLEET) return;
    int nships;
    f >> nships;
    for (int i = 0; i < nships; i++) {
        Item ship;
        ship.Readin(f);
        if (ship.type >= 0)
            SetNumShips(ship.type, ship.num);
    }
}

/* Returns the number of component ships of a given
 * type.
 */
int Object::GetNumShips(int type)
{
    if (CheckShip(type) != 0) {
        for(auto ship : ships) {
            if (ship->type == type) {
                return ship->num;
            }
        }
    }
    return 0;
}

/* Erases possible previous entries for ship type
 * and resets the number of ships.
 */
void Object::SetNumShips(int type, int num)
{
    if (CheckShip(type) != 0) {
        if (num > 0) {
            for(auto ship : ships) {
                if (ship->type == type) {
                    ship->num = num;
                    FleetCapacity();
                    return;
                }
            }
            ships.SetNum(type, num);
            FleetCapacity();
        } else {
            ships.SetNum(type, 0);
            FleetCapacity();
            return;
        }
    }
}

/* Adds one ship of the given type.
 */
void Object::AddShip(int type)
{
    if (CheckShip(type) == 0) return;
    int num = GetNumShips(type);
    num++;
    SetNumShips(type, num);
}

/* Returns the String 'Fleet' for multi-ship fleets
 * and the name of the ship for single ship fleets
 */
std::string Object::FleetDefinition()
{
    std::string fleet;
    int shiptype = -1;
    int num = 0;
    for (int i=0; i<NITEMS; i++) {
        if (ItemDefs[i].type & IT_SHIP) {
            int sn = GetNumShips(i);
            if (sn > 0) {
                num += sn;
                shiptype = i;
            }
        }
    }
    if (num == 1) fleet = ItemDefs[shiptype].name;
    else {
        fleet = ObjectDefs[type].name;
        // report ships:
        for (int item=0; item<NITEMS; item++) {
            num = GetNumShips(item);
            if (num > 0) {
                if (num > 1) {
                    fleet += std::string(", ") + std::to_string(num) + " " + ItemDefs[item].names;
                } else {
                    fleet += std::string(", ") + std::to_string(num) + " " +ItemDefs[item].name;
                }
            }
        }
    }
    return fleet;
}

/* Sets a fleet's sailing capacity.
 */
int Object::FleetCapacity()
{
    capacity = 0;
    // Calculate the maximum number of mages while we're at it
    mages = 0;
    if (!IsFleet()) return 0;
    // Fleets are assumed to be flying, at least until we find any
    // non-flying vessels in them
    flying = 1;
    for (int item=0; item < NITEMS; item++) {
        int num = GetNumShips(item);
        if (num < 1) continue;
        if (ItemDefs[item].fly > 0) {
            capacity += num * ItemDefs[item].fly;
        } else {
            capacity += num * ItemDefs[item].swim;
            flying = 0;
        }
        int ot = lookup_object(ItemDefs[item].name);
        if (ot > 0) mages += num * ObjectDefs[ot].maxMages;
    }
    return capacity;
}

/* Returns a fleet's load or -1 for non-fleet objects.
 */
int Object::FleetLoad()
{
    int load = -1;
    int wgt = 0;
    if (IsFleet()) {
        for(const auto unit : units) wgt += unit->Weight();
        load = wgt;
    }
    return load;
}

/* Return 1 if fleet can sail to a direction without sailing through land, or
 * 0 if it cannot
 */
int Object::SailThroughCheck(int dir)
{
    if (IsFleet()) {
        // if target region doesn't exist, cannot be sailed into
        if (!region->neighbors[dir]) {
            return 0;
        }

        // flying fleets always can sail through
        if (flying == 1) {
            return 1;
        }

        // from ocean sailing is always possible
        if (TerrainDefs[region->type].similar_type == R_OCEAN) {
            return 1;
        }

        // fleet is not flying and it is in a land region. Check that it
        // doesn's sail inland
        if (TerrainDefs[region->neighbors[dir]->type].similar_type != R_OCEAN) {
            return 0;
        }

        // sailing from land into ocean. If sail through is allowed, allow it
        if (!Globals->PREVENT_SAIL_THROUGH) {
            return 1;
        }

        // if the fleet hadn't sailed before, it can go in any direction
        if (prevdir == -1) {
            return 1;
        }

        // Per the rules, you can only exit a region in the direction you came from or an adjacent direction.
        // fleet can always sail backward
        if (prevdir == dir) {
            return 1;
        }

        // Check the adjacent directions as well. (Note: this code has been broken prior to NewOrigins post-V7 code)
        // Prior to V7, the code allowed you to sail through land as long as there was a connected stretch of water
        // between the entry direction and the exit direction.
        int d1 = (prevdir + 1) % NDIRS;
        int d2 = (prevdir - 1 + NDIRS) % NDIRS;

        // Check those directions and make sure they exist and are water.
        if (dir == d1 && region->neighbors[d1] && TerrainDefs[region->neighbors[d1]->type].similar_type == R_OCEAN) {
            return 1;
        }
        if (dir == d2 && region->neighbors[d2] && TerrainDefs[region->neighbors[d2]->type].similar_type == R_OCEAN) {
            return 1;
        }
    }
    return 0;
}

/* Returns the total skill level of all sailors.
 * If report is not 0, returns the total skill level of all
 * units regardless if they have sail orders (for report
 * purposes).
 */
int Object::FleetSailingSkill(int report)
{
    int skill = -1;
    int slvl = 0;
    if (IsFleet()) {
        for(const auto unit : units) {
            if ((report != 0) || (unit->monthorders && unit->monthorders->type == O_SAIL)) {
                slvl += unit->GetSkill(S_SAILING) * unit->GetMen();
            }
        }
        skill = slvl;
    }
    return skill;
}

/* Returns fleet size - which is the total of
 * sailors needed to move the fleet.
 */
int Object::GetFleetSize()
{
    if (!IsFleet()) return 0;
    int inertia = 0;
    for (int item=0; item<NITEMS; item++) {
        int num = GetNumShips(item);
        if (num > 0) inertia += num * ItemDefs[item].weight;
    }
    return (inertia / 50);
}

/* Returns the fleet speed - theoretical if report
 * argument is greater than zero (which means all
 * potential sailors issued a SAIL command). The
 * latter is mainly for report purposes. Game
 * functions for moving the fleet provide a zero
 * argument.
 */
int Object::GetFleetSpeed(int report)
{
    int tskill = FleetSailingSkill(report);
    int speed = Globals->MAX_SPEED;
    int weight = 0;
    int capacity = 0;
    int bonus;
    int windbonus = 0;

    if (!IsFleet()) return 0;

    for (int item = 0; item < NITEMS; item++) {
        int num = GetNumShips(item);
        if (num > 0) {
            weight += num * ItemDefs[item].weight;
            if (ItemDefs[item].fly > 0) {
                capacity += num * ItemDefs[item].fly;
            } else {
                capacity += num * ItemDefs[item].swim;
                flying = 0;
            }
            // Fleets travel as fast as their slowest ship
            if (ItemDefs[item].speed < speed)
                speed = ItemDefs[item].speed;
        }
    }
    // no ships no speed
    if (weight < 1) return 0;

    // check for sufficient sailing skill!
    if (tskill < (weight / 50)) return 0;

    // count wind mages
    for(const auto unit : units) {
        int wb = unit->GetAttribute("wind");
        if (wb > 0) {
            windbonus += wb * 12 * Globals->FLEET_WIND_BOOST;
        }
    }
    // speed gain through wind:
    bonus = windbonus / (weight / 50);
    if (bonus > Globals->FLEET_WIND_BOOST)
        bonus = Globals->FLEET_WIND_BOOST;
    speed += bonus;

    // speed bonus due to more / better skilled sailors:
    bonus = 0;
    while (tskill >= (weight / 25)) {
        bonus++;
        tskill /= 2;
    }
    if (bonus > Globals->FLEET_CREW_BOOST)
        bonus = Globals->FLEET_CREW_BOOST;
    speed += bonus;

    // check for being overloaded
    if (FleetLoad() > capacity) return 0;

    // speed bonus due to low load:
    int loadfactor = (capacity / FleetLoad());
    bonus = 0;
    while (loadfactor >= 2) {
        bonus++;
        loadfactor /= 2;
    }
    if (bonus > Globals->FLEET_LOAD_BOOST)
        bonus = Globals->FLEET_LOAD_BOOST;
    speed += bonus;

    // Cap everything at max speed
    if (speed > Globals->MAX_SPEED) speed = Globals->MAX_SPEED;

    return speed;
}

std::string object_description(int obj)
{
    if (ObjectDefs[obj].flags & ObjectType::DISABLED) return "";

    ObjectType *o = &ObjectDefs[obj];
    std::string temp;
    if (ObjectDefs[obj].flags & ObjectType::GROUP) {
        temp += "This is a group of ships.";
    } else if (o->capacity) {
        temp += "This is a ship.";
    } else {
        temp += "This is a building.";
    }

    if (o->flags & ObjectType::SACRIFICE) {
        temp += " This structure requires a sacrifice of " + std::to_string(o->sacrifice_amount) + " " +
            strings::plural(o->sacrifice_amount, ItemDefs[o->sacrifice_item].name, ItemDefs[o->sacrifice_item].names) +
            " [" + ItemDefs[o->sacrifice_item].abr + "].";
    }

    if (o->flags & ObjectType::GRANTSKILL) {
        temp += " This structure grants the owner the skill " + SkillDefs[o->granted_skill].name + " [" +
            SkillDefs[o->granted_skill].abbr + "] at a skill level of " + std::to_string(o->granted_level) + ".";
    }

    if (Globals->LAIR_MONSTERS_EXIST && (o->monster != -1)) {
        temp += " " + (ItemDefs[o->monster].names | filter::capitalize) + " can potentially lair in this structure.";
        if (o->flags & ObjectType::NOMONSTERGROWTH) temp += " Monsters in this structures will never regenerate.";
    }

    if (o->flags & ObjectType::CANENTER) temp += " Units may enter this structure.";

    if (o->protect) {
        temp += " This structure provides defense to the first " + std::to_string(o->protect) + " men inside it. " +
            "This structure gives a defensive bonus of ";
        std::vector<std::string> defences;
        for (int i=0; i<NUM_ATTACK_TYPES; i++) {
            if (o->defenceArray[i]) {
                std::string def = std::to_string(o->defenceArray[i]) + " against " + attack_type(i) + " attacks";
                defences.push_back(def);
            }
        }
        temp += strings::join(defences, ", ", " and ") + (defences.size() > 0 ? "." : "");

        // Capacity check prevents showing wrong details for ships
        if (Globals->EXTENDED_FORT_DEFENCE && !o->capacity) {
            temp += " This structure also protects in all adjacent regions.";
        }
    }

    for (auto const &spd : SpecialDefs) {
        std::string effect = "are";
        int match = 0;
        if (!(spd.targflags & SpecialType::HIT_BUILDINGIF) && !(spd.targflags & SpecialType::HIT_BUILDINGEXCEPT)) continue;
        for (int j = 0; j < SPECIAL_BUILDINGS; j++)
            if (spd.buildings[j] == obj) match = 1;
        if (!match) continue;
        if (spd.targflags & SpecialType::HIT_BUILDINGEXCEPT) effect += " not";
        temp += " Units in this structure " + effect + " affected by " + spd.specialname + ".";
    }

    if (o->sailors) {
        temp += " This ship requires " + std::to_string(o->sailors) + " total levels of sailing skill to sail.";
    }
    if (o->maxMages && Globals->LIMITED_MAGES_PER_BUILDING) {
        temp += " This structure will allow ";
        if (o->maxMages > 1) {
            temp += "up to " + std::to_string(o->maxMages) + " mages";
        } else {
            temp += "one mage";
        }
        temp += " to study above level 2.";
    }
    int buildable = 1;
    auto pS = FindSkill(o->skill);
    if (o->item == -1 || o->skill == nullptr || !pS || pS->get().flags & SkillType::DISABLED) buildable = 0;
    if (o->item != I_WOOD_OR_STONE && (ItemDefs[o->item].flags & ItemType::DISABLED)) buildable = 0;
    if (
        o->item == I_WOOD_OR_STONE && (ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
        (ItemDefs[I_STONE].flags & ItemType::DISABLED)
    ) {
        buildable = 0;
    }
    if (!buildable && !(ObjectDefs[obj].flags & ObjectType::GROUP)) {
        temp += " This structure cannot be built by players.";
    }

    if (o->productionAided != -1 && !(ItemDefs[o->productionAided].flags & ItemType::DISABLED)) {
        temp += " This trade structure increases the amount of " +
            (o->productionAided == I_SILVER ? "entertainment" : ItemDefs[o->productionAided].names) +
            " available in the region.";
    }

    if (Globals->DECAY) {
        if (o->flags & ObjectType::NEVERDECAY) {
            temp += " This structure will never decay.";
        } else {
            temp += " This structure can take " + std::to_string(o->maxMaintenance) +
                " units of damage before it begins to decay.";
            temp += " Damage can occur at a maximum rate of " + std::to_string(o->maxMonthlyDecay) + " units per month.";
            if (buildable) {
                temp += " Repair of damage is accomplished at a rate of " + std::to_string(o->maintFactor) +
                    " damage units per unit of " +
                    (o->item == I_WOOD_OR_STONE ? "wood or stone." : ItemDefs[o->item].name);
            }
        }
    }

    return temp;
}
