#include <stdlib.h>
#include "items.h"
#include "skills.h"
#include "object.h"
#include "gamedata.h"
#include "string_parser.hpp"
#include "strings_util.hpp"
#include <ranges>
#include <cmath>

using namespace std;

int MonType::getAggression() const {
    int aggression = this->hostile;
    aggression *= Globals->MONSTER_ADVANCE_HOSTILE_PERCENT;
    aggression /= 100;
    if (aggression < Globals->MONSTER_ADVANCE_MIN_PERCENT) {
        aggression = Globals->MONSTER_ADVANCE_MIN_PERCENT;
    }

    return aggression;
}

std::optional<std::reference_wrapper<BattleItemType>> find_battle_item(const strings::ci_string& abbr)
{
    auto it = std::find_if(BattleItemDefs.begin(), BattleItemDefs.end(), [abbr](const BattleItemType& battleItem) {
        return abbr == battleItem.abbr;
    });
    if (it != BattleItemDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<ArmorType>> find_armor(const strings::ci_string& abbr)
{
    if (abbr.empty()) return std::nullopt;

    auto it = std::find_if(ArmorDefs.begin(), ArmorDefs.end(), [abbr](const ArmorType& armor) {
        return abbr == armor.abbr;
    });
    if (it != ArmorDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<WeaponType>> find_weapon(const strings::ci_string& abbr)
{
    if (abbr.empty()) return std::nullopt;

    auto it = std::find_if(WeaponDefs.begin(), WeaponDefs.end(), [abbr](const WeaponType& weapon) {
        return abbr == weapon.abbr;
    });
    if (it != WeaponDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<ItemType>> find_item(const strings::ci_string& abbr)
{
    if (abbr.empty()) return std::nullopt;

    auto it = std::find_if(ItemDefs.begin(), ItemDefs.end(), [abbr](const ItemType& item) {
        return abbr == item.abr;
    });
    if (it != ItemDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<MountType>> find_mount(const strings::ci_string& abbr)
{
    if (abbr.empty()) return std::nullopt;

    auto it = std::find_if(MountDefs.begin(), MountDefs.end(), [abbr](const MountType& mount) {
        return abbr == mount.abbr;
    });
    if (it != MountDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<MonType>> find_monster(const strings::ci_string& abbr, int illusion)
{
    if (abbr.empty()) return std::nullopt;
    strings::ci_string tag = (illusion ? "i" : "") + abbr;

    auto it = std::find_if(MonDefs.begin(), MonDefs.end(), [tag](const MonType& mon) {
        return tag == mon.abbr;
    });
    if (it != MonDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<ManType>> find_race(const strings::ci_string& abbr)
{
    if (abbr.empty()) return std::nullopt;

    auto it = std::find_if(ManDefs.begin(), ManDefs.end(), [abbr](const ManType& man) {
        return abbr == man.abbr;
    });
    if (it != ManDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::string attack_type(int atype)
{
    switch(atype) {
        case ATTACK_COMBAT: return "melee";
        case ATTACK_ENERGY: return "energy";
        case ATTACK_SPIRIT: return "spirit";
        case ATTACK_WEATHER: return "weather";
        case ATTACK_RIDING: return "riding";
        case ATTACK_RANGED: return "ranged";
        case NUM_ATTACK_TYPES: return "non-resistable";
        default: return "unknown";
    }
}

static std::string defense_type(int atype)
{
    if (atype == NUM_ATTACK_TYPES) return "all";
    return attack_type(atype);
}

int lookup_item(const strings::ci_string& name)
{
    for (int i = 0; i < NITEMS; i++) {
        std::string item_name = (ItemDefs[i].type & IT_ILLUSION ? "i" : "") + ItemDefs[i].abr;
        if (name == item_name) return i;
    }
    return -1;
}

std::optional<int> parse_item_category(const parser::token& str)
{
    if (str == "normal") return -IT_NORMAL;
    if (str == "advanced") return -IT_ADVANCED;
    if (str == "trade") return -IT_TRADE;
    if (str == "man" || str == "men") return -IT_MAN;
    if (str == "monster" || str == "monsters") return -IT_MONSTER;
    if (str == "magic") return -IT_MAGIC;
    if (str == "weapon" || str == "weapons") return -IT_WEAPON;
    if (str == "armor") return -IT_ARMOR;
    if (str == "mount" || str == "mounts") return -IT_MOUNT;
    if (str == "battle") return -IT_BATTLE;
    if (str == "special") return -IT_SPECIAL;
    if (str == "food") return -IT_FOOD;
    if (str == "tool" || str == "tools") return -IT_TOOL;
    if (str == "item" || str == "items") return -NITEMS;
    if (str == "ship" || str == "ships") return -IT_SHIP;

    return std::nullopt;  // Not a category
}

int parse_all_items(const parser::token& token, int flags)
{
    for (int i = 0; i < NITEMS; i++) {
        if (ItemDefs[i].flags & flags) continue;
        bool illusion = ItemDefs[i].type & IT_ILLUSION;
        std::string itemName = (illusion ? "i" : "") + ItemDefs[i].name;
        std::string itemNames = (illusion ? "i" : "") + ItemDefs[i].names;
        std::string itemAbr = (illusion ? "i" : "") + ItemDefs[i].abr;

        if (token == itemName || token == itemNames || token == itemAbr) return i;
    }
    return -1;
}

int parse_enabled_item(const parser::token& token, int flags)
{
    int item = parse_all_items(token, flags | ItemType::DISABLED);
    return item;
}

int parse_giveable_item(const parser::token& token, int flags)
{
    int item = parse_enabled_item(token, flags | ItemType::CANTGIVE);
    return item;
}

int parse_transportable_item(const parser::token& token, int flags)
{
    int item = parse_giveable_item(token, ItemType::NOTRANSPORT);
    return item;
}

string item_string(int type, int num, int flags)
{
    string temp;
    if (num == 1) {
        if (flags & FULLNUM)
            temp += std::to_string(num) + " ";
        temp += (flags & ALWAYSPLURAL ? ItemDefs[type].names : ItemDefs[type].name) + " [" + ItemDefs[type].abr + "]";
    } else {
        if (num == -1) {
            temp += "unlimited " + ItemDefs[type].names + " [" + ItemDefs[type].abr + "]";
        } else {
            temp += std::to_string(num) + " " + ItemDefs[type].names + " [" + ItemDefs[type].abr + "]";
        }
    }
    return temp;
}

static std::string effect_description(char const *effect)
{
    std::string temp, temp2;
    int comma = 0;
    int i;

    auto ep = FindEffect(effect).value().get();

    temp += ep.name;

    if (ep.attackVal) {
        temp2 += std::to_string(ep.attackVal) + " to attack";
        comma = 1;
    }

    for (i = 0; i < 4; i++) {
        if (ep.defMods[i].type == -1) continue;
        if (comma) temp2 += ", ";
        temp2 += std::to_string(ep.defMods[i].val) + " versus " + defense_type(ep.defMods[i].type) + " attacks";
        comma = 1;
    }

    if (comma) {
        temp += " (" + temp2 + ")";
        temp += (ep.flags & EffectType::EFF_ONESHOT) ? " for their next attack" : " for the rest of the battle";
    }

    temp += ".";

    if (ep.cancelEffect != NULL) {
        if (comma) temp += " ";
        auto up = FindEffect(ep.cancelEffect).value().get();
        temp += "This effect cancels out the effects of " + up.name + ".";
    }
    return temp;
}

static std::string attack_damage_description(const int damage) {
    if (damage <= 0) {
        return "attack deals no damage";
    }

    // Full skill level damage
    if (damage >= WeaponType::NUM_DAMAGE_SKILL) {
        return "attack deals the skill level points of damage";
    }

    // Half skill damage
    if (damage >= WeaponType::NUM_DAMAGE_HALF_SKILL) {
        return "attack deals does half the skill level (rounded up) points of damage";
    }

    // Just damage
    return "attack deals " + std::to_string(damage) + " damage";
}

std::string show_special(const std::string& special, int level, int expandLevel, int fromItem)
{
    std::string temp;
    int comma = 0;
    int i;
    int last = -1;
    int val;

    auto spd = find_special(special).value().get();
    temp += spd.specialname;
    temp += " in battle";
    if (expandLevel)
        temp += " at a skill level of " + std::to_string(level);
    temp += ".";

    if ((spd.targflags & SpecialType::HIT_BUILDINGIF) ||
            (spd.targflags & SpecialType::HIT_BUILDINGEXCEPT)) {
        temp += " This ability will ";
        if (spd.targflags & SpecialType::HIT_BUILDINGEXCEPT) {
            temp += "only target units inside structures, with the exception of";
        } else {
            temp += "only target units which are inside";
        }

        temp += " the following structures: ";
        for (i = 0; i < SPECIAL_BUILDINGS; i++) {
            if (spd.buildings[i] == -1) continue;
            if (ObjectDefs[spd.buildings[i]].flags & ObjectType::DISABLED)
                continue;
            if (last == -1) {
                last = i;
                continue;
            }
            temp += ObjectDefs[spd.buildings[last]].name + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "or ";
        }
        temp += ObjectDefs[spd.buildings[last]].name + ".";
    }
    if ((spd.targflags & SpecialType::HIT_SOLDIERIF) ||
            (spd.targflags & SpecialType::HIT_SOLDIEREXCEPT) ||
            (spd.targflags & SpecialType::HIT_MOUNTIF) ||
            (spd.targflags & SpecialType::HIT_MOUNTEXCEPT)) {
        temp += " This ability will ";
        if ((spd.targflags & SpecialType::HIT_SOLDIEREXCEPT) ||
                (spd.targflags & SpecialType::HIT_MOUNTEXCEPT)) {
            temp += "not ";
        } else {
            temp += "only ";
        }
        temp += "target ";
        if ((spd.targflags & SpecialType::HIT_MOUNTIF) ||
                (spd.targflags & SpecialType::HIT_MOUNTEXCEPT)) {
            temp += "units mounted on ";
        }
        comma = 0;
        last = -1;
        for (i = 0; i < 7; i++) {
            if (spd.targets[i] == -1) continue;
            if (ItemDefs[spd.targets[i]].flags & ItemType::DISABLED) continue;
            if (last == -1) {
                last = i;
                continue;
            }
            temp += item_string(spd.targets[last], 1, ALWAYSPLURAL) + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "or ";
        }
        temp += item_string(spd.targets[last], 1, ALWAYSPLURAL) + ".";
    }
    if ((spd.targflags & SpecialType::HIT_EFFECTIF) ||
            (spd.targflags & SpecialType::HIT_EFFECTEXCEPT)) {
        temp += " This ability will ";
        if (spd.targflags & SpecialType::HIT_EFFECTEXCEPT) {
            temp += "not ";
        } else {
            temp += "only ";
        }
        temp += "target creatures which are currently affected by ";
        for (i = 0; i < 3; i++) {
            if (spd.effects[i] == NULL) continue;
            if (last == -1) {
                last = i;
                continue;
            }
            auto ep = FindEffect(spd.effects[last]).value().get();
            temp += ep.name + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "or ";
        }
        auto ep = FindEffect(spd.effects[last]).value().get();
        temp += ep.name + ".";
    }
    if (spd.targflags & SpecialType::HIT_ILLUSION) {
        temp += " This ability will only target illusions.";
    }
    if (spd.targflags & SpecialType::HIT_NOMONSTER) {
        temp += " This ability cannot target monsters.";
    }
    if (spd.effectflags & SpecialType::FX_NOBUILDING) {
        temp += " The bonus given to units inside buildings is not effective against this ability.";
    }

    if (spd.effectflags & SpecialType::FX_SHIELD) {
        if (!fromItem)
            temp += " This spell provides a shield against all ";
        else
            temp += " This ability provides the wielder with a defence bonus of " + std::to_string(level) +
                " against all ";
        comma = 0;
        last = -1;
        for (i = 0; i < 4; i++) {
            if (spd.shield[i] == -1) continue;
            if (last == -1) {
                last = i;
                continue;
            }
            temp += defense_type(spd.shield[last]) + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "and ";
        }
        if (fromItem)
            temp += defense_type(spd.shield[last]) + " attacks.";
        else {
            temp += defense_type(spd.shield[last]) + " attacks against the entire" +
                " army at a level equal to the skill level of the ability.";
        }
    }
    if (spd.effectflags & SpecialType::FX_DEFBONUS) {
        temp += " This ";
        if (fromItem)
            temp += "ability";
        else
            temp += "spell";
        temp += " provides ";
        comma = 0;
        last = -1;
        for (i = 0; i < 4; i++) {
            if (spd.defs[i].type == -1) continue;
            if (last == -1) {
                last = i;
                continue;
            }
            val = spd.defs[last].val;
            if (expandLevel) {
                if (spd.effectflags & SpecialType::FX_USE_LEV)
                    val *= level;
            }

            temp += "a defensive bonus of " + std::to_string(val);
            if (!expandLevel) {
                temp += " per skill level";
            }
            temp += " versus " + defense_type(spd.defs[last].type) + " attacks, ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "and ";
        }
        val = spd.defs[last].val;
        if (expandLevel) {
            if (spd.effectflags & SpecialType::FX_USE_LEV)
                val *= level;
        }
        temp += "a defensive bonus of " + std::to_string(val);
        if (!expandLevel) {
            temp += " per skill level";
        }
        temp += " versus " + defense_type(spd.defs[last].type) + " attacks";
        temp += " to the user.";
    }

    /* Now the damages */
    for (i = 0; i < 4; i++) {
        if (spd.damage[i].type == -1) continue;
        temp += " This ability does between " + std::to_string(spd.damage[i].minnum) + " and ";
        val = spd.damage[i].value * 2;
        if (expandLevel) {
            if (spd.effectflags & SpecialType::FX_USE_LEV)
                val *= level;
        }
        temp += std::to_string(val);
        if (!expandLevel) {
            temp += " times the skill level of the mage";
        }
        temp += " " + attack_type(spd.damage[i].type) + " attacks and each " +
            attack_damage_description(spd.damage[i].hitDamage) + ".";
        if (spd.damage[i].effect) {
            temp += " Each attack causes the target to be effected by ";
            temp += effect_description(spd.damage[i].effect);
        }
    }

    return temp;
}

static std::string resistance_description(int type, int val, int full)
{
    std::string temp;
    if (full) {
        temp = "has a resistance of " + std::to_string(val);
    } else {
        temp = "is ";
        if (val < 1) temp += "very susceptible";
        else if (val == 1) temp += "susceptible";
        else if (val > 1 && val < 3) temp += "typically resistant";
        else if (val > 2 && val < 5) temp += "slightly resistant";
        else temp += "very resistant";
    }
    temp += " to " + attack_type(type) + " attacks.";
    return temp;
}

static std::string weapon_class(int wclass)
{
    switch(wclass) {
        case SLASHING: return "slashing";
        case PIERCING: return "piercing";
        case CRUSHING: return "crushing";
        case CLEAVING: return "cleaving";
        case ARMORPIERCING: return "armor-piercing";
        case MAGIC_ENERGY: return "energy";
        case MAGIC_SPIRIT: return "spirit";
        case MAGIC_WEATHER: return "weather";
    }
    return "unknown";
}

static std::string weapon_type(int flags, int wclass)
{
    std::string type;
    if (flags & WeaponType::RANGED) type = "ranged ";
    if (flags & WeaponType::LONG) type = "long ";
    if (flags & WeaponType::SHORT) type = "short ";
    type += weapon_class(wclass);
    return type;
}

std::string ShowItem::display_name()
{
    if (ItemDefs[item].type & IT_ILLUSION) {
        return "illusory " + ItemDefs[item].name;
    }
    return ItemDefs[item].name;
}

std::string ShowItem::display_tag() {
    if (ItemDefs[item].type & IT_ILLUSION) {
        return "I" + ItemDefs[item].abr;
    }
    return ItemDefs[item].abr;

}

std::string item_description(int item, int full)
{
    int i;
    std::string skname;

    if (ItemDefs[item].flags & ItemType::DISABLED) return "";

    std::string temp;
    int illusion = (ItemDefs[item].type & IT_ILLUSION);

    temp += (illusion?"illusory ":"")+ ItemDefs[item].name + " [" + (illusion?"I":"") + ItemDefs[item].abr + "]";

    /* new ship items */
    if (ItemDefs[item].type & IT_SHIP) {
        if (ItemDefs[item].swim > 0) {
            temp += ". This is a ship with a capacity of " + std::to_string(ItemDefs[item].swim);
        }
        if (ItemDefs[item].fly > 0) {
            temp += ". This is a flying 'ship' with a capacity of " + std::to_string(ItemDefs[item].fly);
        }
        temp += " and a speed of " + std::to_string(ItemDefs[item].speed) + " hex";
        if (ItemDefs[item].speed != 1)
            temp += "es";
        temp += " per month. This ship requires a total of " + std::to_string(ItemDefs[item].weight/50) +
            " levels of sailing skill to sail";
        int objectno = lookup_object(ItemDefs[item].name);
        if (objectno >= 0) {
            if (ObjectDefs[objectno].protect > 0) {
                temp += ". This ship provides defense to the first " + std::to_string(ObjectDefs[objectno].protect) +
                    " men inside it, giving a defensive bonus of ";

                std::vector<std::string> defences;
                for (int i=0; i<NUM_ATTACK_TYPES; i++) {
                    if (ObjectDefs[objectno].defenceArray[i]) {
                        std::string def = std::to_string(ObjectDefs[objectno].defenceArray[i]) + " against " +
                            attack_type(i) + " attacks";
                        defences.push_back(def);
                    }
                }
                temp += strings::join(defences, ", ", " and ");
            }
            if (Globals->LIMITED_MAGES_PER_BUILDING && ObjectDefs[objectno].maxMages > 0) {
                temp += ". This ship will allow ";
                if (ObjectDefs[objectno].maxMages > 1) {
                    temp += "up to " + std::to_string(ObjectDefs[objectno].maxMages) + " mages";
                } else {
                    temp += "one mage";
                }
                temp += " to study above level 2";
            }
        }
    } else {

        temp += ", weight " + std::to_string(ItemDefs[item].weight);

        if (ItemDefs[item].walk) {
            int cap = ItemDefs[item].walk - ItemDefs[item].weight;
            if (cap) {
                temp += ", walking capacity " + std::to_string(cap);
            } else {
                temp += ", can walk";
            }
        }
        if ((ItemDefs[item].hitchItem != -1 )&&
                !(ItemDefs[ItemDefs[item].hitchItem].flags & ItemType::DISABLED)) {
            int cap = ItemDefs[item].walk - ItemDefs[item].weight +
                ItemDefs[item].hitchwalk;
            if (cap) {
                temp += ", walking capacity " + std::to_string(cap) + " when hitched to a " +
                    ItemDefs[ItemDefs[item].hitchItem].name;
            }
        }
        if (ItemDefs[item].ride) {
            int cap = ItemDefs[item].ride - ItemDefs[item].weight;
            if (cap) {
                temp += ", riding capacity " + std::to_string(cap);
            } else {
                temp += ", can ride";
            }
        }
        if (ItemDefs[item].swim) {
            int cap = ItemDefs[item].swim - ItemDefs[item].weight;
            if (cap) {
                temp += ", swimming capacity " + std::to_string(cap);
            } else {
                temp += ", can swim";
            }
        }
        if (ItemDefs[item].fly) {
            int cap = ItemDefs[item].fly - ItemDefs[item].weight;
            if (cap) {
                temp += ", flying capacity " + std::to_string(cap);
            } else {
                temp += ", can fly";
            }
        }
        if (ItemDefs[item].speed) {
            temp += ", moves " + std::to_string(ItemDefs[item].speed) +
                strings::plural(ItemDefs[item].speed, " hex", " hexes") + " per month";
        }
    }

    if (ItemDefs[item].flags & ItemType::NOSTEALTH) {
        temp += ". This item prevents the unit it is with from being stealthy";
    }
    if (ItemDefs[item].flags & ItemType::NO_SHAFT) {
        temp += ". This item prevents the unit it is with from entering a shaft";
    }
    if (ItemDefs[item].flags & ItemType::SEEK_ALTAR) {
        temp += ". This item desires to move toward the nearest Ritual Altar";
    }
    if (ItemDefs[item].flags & ItemType::MAINTENANCE) {
        temp += ". This item requires maintenance each turn of " + std::to_string(ItemDefs[item].baseprice) + " silver";
        if (ItemDefs[item].flags & ItemType::SEEK_ALTAR) {
            temp += ". Moving toward the altar will reduce the maintenance cost in half. Moving away from the altar will multiply the maintenance cost 5 time";
        }
    }


    if (Globals->ALLOW_WITHDRAW) {
        if (ItemDefs[item].type & IT_NORMAL && item != I_SILVER) {
            temp += ", costs " + std::to_string(ItemDefs[item].baseprice*5/2) + " silver to withdraw";
        }
    }
    temp += ".";

    if (ItemDefs[item].type & IT_MAN) {
        auto mt = find_race(ItemDefs[item].abr)->get();
        std::string mani = "MANI";
        std::string last = "";
        bool found = false;
        temp += " This race may study ";
        unsigned int c;
        unsigned int len = sizeof(mt.skills) / sizeof(mt.skills[0]);
        for (c = 0; c < len; c++) {
            auto pS = FindSkill(mt.skills[c] ? mt.skills[c]->c_str() : nullptr);
            if (!pS) continue;
            if (mani == pS->get().abbr && Globals->MAGE_NONLEADERS) {
                if (!(last == "")) {
                    if (found)
                        temp += ", ";
                    temp += last;
                    found = true;
                }
                last = "all magical skills";
                continue;
            }
            if (pS->get().flags & SkillType::DISABLED) continue;
            if (!last.empty()) {
                if (found)
                    temp += ", ";
                temp += last;
                found = true;
            }
            last = SkillStrs(pS->get());
        }
        if (!last.empty()) {
            if (found)
                temp += " and ";
            temp += last;
            found = true;
        }
        if (found) {
            temp += " to level " + std::to_string(mt.speciallevel) +
                " and all other skills to level " + std::to_string(mt.defaultlevel) + ".";
        } else {
            temp += "all skills to level " + std::to_string(mt.defaultlevel) + ".";
        }
    }

    if ((ItemDefs[item].type & IT_MONSTER) && !(ItemDefs[item].flags & ItemType::MANPRODUCE)) {
        temp += " This is a monster.";
        auto monster = find_monster(ItemDefs[item].abr, (ItemDefs[item].type & IT_ILLUSION))->get();
        temp += " This monster attacks with a combat skill of " + std::to_string(monster.attackLevel);

        for (int c = 0; c < NUM_ATTACK_TYPES; c++) {
            temp += " This monster " + resistance_description(c, monster.defense[c], full);
        }

        if (monster.special && monster.special != NULL) {
            temp += " Monster can cast " + show_special(monster.special ? monster.special : "", monster.specialLevel, 1, 0);
        }

        {
            std::vector<std::string> spawnIn;
            for (const auto& terrain : TerrainDefs) {
                if (!(terrain.flags & TerrainType::SHOW_RULES)) {
                    continue;
                }

                if (terrain.smallmon == item || terrain.bigmon == item || terrain.humanoid == item) {
                    spawnIn.push_back(terrain.plural);
                }
            }

            if (!spawnIn.empty()) {
                temp += " The monster can spawn in the wilderness of " + strings::join(spawnIn, ", ", " and ") + ".";
            }
        }

        {
            std::vector<std::string> lairIn;
            for (int i = 0; i < NOBJECTS; i++) {
                ObjectType &obj = ObjectDefs[i];
                if (obj.flags & ObjectType::DISABLED) {
                    continue;
                }

                if (obj.monster == item) {
                    lairIn.push_back(obj.name);
                }
            }

            if (!lairIn.empty()) {
                temp += " This monster can lair in the " + strings::join(lairIn, ", ", " and ") + ".";
            }
        }

        if (monster.preferredTerrain.empty() && monster.forbiddenTerrain.empty()) {
            temp += " The monster has no terrain preferences, and it can travel through any terrain.";
        }

        if (!monster.forbiddenTerrain.empty()) {
            std::vector<std::string> list;
            for (auto &terrain : monster.forbiddenTerrain) {
                list.push_back(TerrainDefs[terrain].name);
            }

            temp += " Monster severely dislikes " + strings::join(list, ",", " and ") + " " +
                strings::plural(monster.forbiddenTerrain.size(), "terrain", "terrains") + " and will never try to enter them.";
        }

        if (!monster.preferredTerrain.empty()) {
            temp += " Monster prefers to roam the";

            bool isNext = false;
            for (auto &terrain : monster.preferredTerrain) {
                temp += (isNext ? ", " : " ") + TerrainDefs[terrain].name;
                isNext = true;
            }

            temp += " " + strings::plural(monster.preferredTerrain.size(), "terrain", "terrains") + ".";
        }

        const int aggression = monster.getAggression();
        if (aggression >= 100) {
            temp += " Monster is unbelievably aggressive and will attack player units on sight.";
        }
        else if (aggression >= 75) {
            temp += " Monster is exceptionally aggressive, and there is a slight chance he will not attack player units.";
        }
        else if (aggression >= 50) {
            temp += " Monster is very aggressive, but he will not harm player units with good luck.";
        }
        else if (aggression >= 25) {
            temp += " Monster is aggressive but, in most cases, will leave player units alone.";
        }
        else if (aggression > 0) {
            temp += " Monster is unfriendly, and the player must be pretty unlucky to be attacked by this monster.";
        }
        else {
            temp += " Monster is totally peaceful and will never attack player units.";
        }

        if (full) {
            int hits = monster.hits;
            int atts = monster.numAttacks;
            int regen = monster.regen;
            if (!hits) hits = 1;
            if (!atts) atts = 1;
            temp += " This monster has " + std::to_string(atts) + " melee " +
                strings::plural(atts, "attack", "attacks") + " per round and takes " +
                std::to_string(hits) + " " + strings::plural(hits, "hit", "hits") + " to kill";

            if (atts > 0) {
                temp += " and each " + attack_damage_description(monster.hitDamage);
            }

            temp += ".";

            if (regen > 0) {
                temp += " This monsters regenerates " + std::to_string(regen) + " hits per round of battle.";
            }
            temp += " This monster has a tactics score of " + std::to_string(monster.tactics) +
                ", a stealth score of " + std::to_string(monster.stealth) +
                ", and an observation score of " + std::to_string(monster.obs) + ".";
        }
        temp += " This monster might have ";
        if (monster.spoiltype != -1) {
            if (monster.spoiltype & IT_MAGIC) {
                temp += "magic items and ";
            } else if (monster.spoiltype & IT_ADVANCED) {
                temp += "advanced items and ";
            } else if (monster.spoiltype & IT_NORMAL) {
                temp += "normal or trade items and ";
            }
        }
        temp += "silver as treasure.";
    }

    if(ItemDefs[item].flags & ItemType::MANPRODUCE) {
        temp += " This is a free-moving-item (FMI).";
        auto monster = find_monster(ItemDefs[item].abr, (ItemDefs[item].type & IT_ILLUSION))->get();
        temp += " This FMI attacks with a combat skill of " + std::to_string(monster.attackLevel) + ".";

        for (int c = 0; c < NUM_ATTACK_TYPES; c++) {
            temp += " This FMI " + resistance_description(c, monster.defense[c], full);
        }

        if (monster.special && monster.special != NULL) {
            temp += " FMI can cast " + show_special(monster.special ? monster.special : "", monster.specialLevel, 1, 0);
        }

        if (full) {
            int hits = monster.hits;
            int atts = monster.numAttacks;
            int regen = monster.regen;
            if (!hits) hits = 1;
            if (!atts) atts = 1;
            temp += " This FMI has " + std::to_string(atts) + " melee " +
                ((atts > 1)?"attacks":"attack") + " per round and takes " +
                std::to_string(hits) + " " + ((hits > 1)?"hits":"hit") + " to kill";

            if (atts > 0) {
                temp += " and each " + attack_damage_description(monster.hitDamage);
            }

            temp += ".";

            if (regen > 0) {
                temp += " This FMI regenerates " + std::to_string(regen) + " hits per round of battle.";
            }
            temp += " This FMI has a tactics score of " + std::to_string(monster.tactics) +
                ", a stealth score of " + std::to_string(monster.stealth) +
                ", and an observation score of " + std::to_string(monster.obs) + ".";
        }

        if (monster.spoiltype != -1) {
            temp += " This FMI might have ";

            if (monster.spoiltype & IT_MAGIC) {
                temp += "magic items and ";
            } else if (monster.spoiltype & IT_ADVANCED) {
                temp += "advanced items and ";
            } else if (monster.spoiltype & IT_NORMAL) {
                temp += "normal or trade items and ";
            }

            temp += "silver as treasure.";
        }
    }

    if (ItemDefs[item].type & IT_WEAPON) {
        auto weapon = find_weapon(ItemDefs[item].abr)->get();
        temp += " This is a ";
        temp += weapon_type(weapon.flags, weapon.weapClass) + " weapon and each " +
            attack_damage_description(weapon.hitDamage) + ".";
        if (weapon.flags & WeaponType::NEEDSKILL) {
            auto pS = FindSkill(weapon.baseSkill);
            if (pS) {
                temp += " Knowledge of " + SkillStrs(pS->get());
                pS = FindSkill(weapon.orSkill);
                if (pS)
                    temp += " or " + SkillStrs(pS->get());
                temp += " is needed to wield this weapon.";
            }
        } else
            temp += " No skill is needed to wield this weapon.";

        bool flag = false;
        if (weapon.attackBonus != 0) {
            temp += " This weapon grants a ";
            temp += (weapon.attackBonus > 0 ? "bonus of " : "penalty of ") + std::to_string(std::abs(weapon.attackBonus)) +
                " on attack";
            flag = true;
        }
        if (weapon.defenseBonus != 0) {
            if (flag) {
                if (weapon.attackBonus == weapon.defenseBonus) {
                    temp += " and defense.";
                    flag = false;
                } else {
                    temp += " and a ";
                }
            } else {
                temp += " This weapon grants a ";
                flag = true;
            }
            if (flag) {
                temp += (weapon.defenseBonus > 0 ? "bonus of " : "penalty of ") + std::to_string(std::abs(weapon.defenseBonus)) +
                    " on defense.";
                flag = false;
            }
        }
        if (flag) temp += ".";
        if (weapon.mountBonus && full) {
            temp += " This weapon ";
            if (weapon.attackBonus != 0 || weapon.defenseBonus != 0)
                temp += "also ";
            temp += "grants a ";
            temp += (weapon.mountBonus > 0 ? "bonus of " : "penalty of ") + std::to_string(std::abs(weapon.mountBonus)) +
                " against mounted opponents.";
        }

        if (weapon.flags & WeaponType::NOFOOT)
            temp += " Only mounted troops may use this weapon.";
        else if (weapon.flags & WeaponType::NOMOUNT)
            temp += " Only foot troops may use this weapon.";

        if (weapon.flags & WeaponType::RIDINGBONUS) {
            temp += " Wielders of this weapon, if mounted, get their riding skill bonus on combat attack and defense.";
        } else if (weapon.flags & WeaponType::RIDINGBONUSDEFENSE) {
            temp += " Wielders of this weapon, if mounted, get their riding skill bonus on combat defense.";
        }

        if (weapon.flags & WeaponType::NODEFENSE) {
            temp += " Defenders are treated as if they have an effective combat skill of 0.";
        }

        if (weapon.flags & WeaponType::NOATTACKERSKILL) {
            temp += " Attackers do not get skill bonus on defense.";
        }

        if (weapon.flags & WeaponType::ALWAYSREADY) {
            temp += " Wielders of this weapon never miss a round to ready their weapon.";
        } else {
            temp += " There is a 50% chance that the wielder of this weapon gets a chance to attack in any given round.";
        }

        if (full) {
            int atts = weapon.numAttacks;
            temp += " This weapon attacks versus the target's defense against " +
                attack_type(weapon.attackType) + " attacks.";
            temp += " This weapon allows ";
            if (atts > 0) {
                if (atts >= WeaponType::NUM_ATTACKS_HALF_SKILL) {
                    int max = WeaponType::NUM_ATTACKS_HALF_SKILL;
                    std::string attd = "half the skill level (rounded up)";
                    if (atts >= WeaponType::NUM_ATTACKS_SKILL) {
                        max = WeaponType::NUM_ATTACKS_SKILL;
                        attd = "the skill level";
                    }
                    temp += "a number of attacks equal to ";
                    temp += attd;
                    temp += " of the attacker";
                    int val = atts - max;
                    if (val > 0) temp += " plus " + std::to_string(val);
                } else {
                    temp += std::to_string(atts) + ((atts==1)?" attack":" attacks");
                }
                temp += " per round.";
            } else {
                atts = -atts;
                temp += "1 attack every ";
                if (atts == 1) temp += "round .";
                else temp += std::to_string(atts) + " rounds.";
            }

            for (int i = 0; i < MAX_WEAPON_BM_TARGETS; i++) {
                WeaponBonusMalus *bm = &weapon.bonusMalus[i];
                if (!bm->weaponAbbr) continue;
                if (bm->attackModifer == 0 && bm->defenseModifer == 0) continue;

                auto target = find_item(bm->weaponAbbr)->get();

                temp += " Wielders of this weapon will get ";

                if (bm->attackModifer != 0) {
                    temp += ((bm->attackModifer > 0) ? "bonus of ":"penalty of ") +
                        std::to_string(std::abs(bm->attackModifer)) + " on combat attack";
                }

                if (bm->defenseModifer != 0) {
                    if (bm->attackModifer != 0) {
                        temp += " and ";
                    }

                    temp += ((bm->defenseModifer > 0) ? "bonus of ":"penalty of ") +
                        std::to_string(std::abs(bm->defenseModifer)) + " on combat defense";
                }

                temp += " against " + target.name + " [" + target.abr + "].";
            }
        }
    }

    if (ItemDefs[item].type & IT_ARMOR) {
        temp += " This is a type of armor.";
        auto armor = find_armor(ItemDefs[item].abr)->get();
        temp += " This armor will protect its wearer ";
        for (i = 0; i < NUM_WEAPON_CLASSES; i++) {
            if (i == NUM_WEAPON_CLASSES - 1) {
                temp += ", and ";
            } else if (i > 0) {
                temp += ", ";
            }
            int percent = static_cast<int>(
                std::round(static_cast<float>(armor.saves[i]) * 100.0f / static_cast<float>(armor.from))
            );
            temp += std::to_string(percent) + "% of the time versus " + weapon_class(i) + " attacks";
        }
        temp += ".";
        if (full) {
            if (armor.flags & ArmorType::USEINASSASSINATE) {
                temp += " This armor may be worn during assassination attempts.";
            }
        }
    }

    if (ItemDefs[item].type & IT_TOOL) {
        int comma = 0;
        int last = -1;
        temp += " This is a tool. This item increases the production of ";
        for (i = NITEMS - 1; i > 0; i--) {
            if (ItemDefs[i].flags & ItemType::DISABLED) continue;
            if (ItemDefs[i].mult_item == item) {
                last = i;
                break;
            }
        }
        for (i = 0; i < NITEMS; i++) {
            if (ItemDefs[i].flags & ItemType::DISABLED) continue;
            if (ItemDefs[i].mult_item == item) {
                if (comma) {
                    if (last == i) {
                        if (comma > 1) temp += ",";
                        temp += " and ";
                    } else {
                        temp += ", ";
                    }
                }
                comma++;
                if (i == I_SILVER) {
                    temp += "entertainment";
                } else {
                    temp += item_string(i, 1);
                }
                temp += " by " + std::to_string(ItemDefs[i].mult_val);
            }
        }
        temp += ".";
    }

    if (ItemDefs[item].type & IT_TRADE) {
        temp += " This is a trade good.";
        if (full) {
            if (Globals->RANDOM_ECONOMY) {
                int maxbuy, minbuy, maxsell, minsell;
                if (Globals->MORE_PROFITABLE_TRADE_GOODS) {
                    minsell = (ItemDefs[item].baseprice*250)/100;
                    maxsell = (ItemDefs[item].baseprice*350)/100;
                    minbuy = (ItemDefs[item].baseprice*100)/100;
                    maxbuy = (ItemDefs[item].baseprice*190)/100;
                } else {
                    minsell = (ItemDefs[item].baseprice*150)/100;
                    maxsell = (ItemDefs[item].baseprice*200)/100;
                    minbuy = (ItemDefs[item].baseprice*100)/100;
                    maxbuy = (ItemDefs[item].baseprice*150)/100;
                }
                temp += " This item can be bought for between " + std::to_string(minbuy) + " and " +
                    std::to_string(maxbuy) + " silver.";
                temp += " This item can be sold for between " + std::to_string(minsell) + " and " +
                    std::to_string(maxsell) + " silver.";
            } else {
                temp += " This item can be bought and sold for " + std::to_string(ItemDefs[item].baseprice) + " silver.";
            }
        }
    }

    if (ItemDefs[item].type & IT_MOUNT) {
        temp += " This is a mount.";
        auto mount = find_mount(ItemDefs[item].abr).value().get();
        if (mount.skill == NULL) {
            temp += " No skill is required to use this mount.";
        } else {
            auto pS = FindSkill(mount.skill);
            if (!pS || (pS->get().flags & SkillType::DISABLED))
                temp += " This mount is unrideable.";
            else {
                temp += " This mount requires " + SkillStrs(pS->get()) + " of at least level " +
                    std::to_string(mount.minBonus) + " to ride in combat.";
            }
        }
        temp += " This mount gives a minimum bonus of +";
        if (Globals->HALF_RIDING_BONUS) {
            temp += std::to_string((mount.minBonus + 1) / 2) + " when ridden into combat.";
        } else {
            temp += std::to_string(mount.minBonus) + " when ridden into combat.";
        }

        temp += " This mount gives a maximum bonus of +";
        if (Globals->HALF_RIDING_BONUS) {
            temp += std::to_string((mount.maxBonus + 1) / 2) + " when ridden into combat.";
            temp += " This bonus is calculated as a RIDING skill divided by 2 (rounded up).";
        } else {
            temp += std::to_string(mount.maxBonus) + " when ridden into combat.";
        }

        if (full) {
            if (ItemDefs[item].fly) {
                temp += " This mount gives a maximum bonus of +";
                if (Globals->HALF_RIDING_BONUS) {
                    temp += std::to_string((mount.maxHamperedBonus + 1) / 2) + " when ridden into combat in ";
                } else {
                    temp += std::to_string(mount.maxHamperedBonus) + " when ridden into combat in ";
                }
                temp += "terrain which allows ridden mounts but not flying mounts.";
            }
            if (mount.mountSpecial != NULL) {
                temp += " When ridden, this mount causes " + show_special(mount.mountSpecial ? mount.mountSpecial : "", mount.specialLev, 1, 0);
            }
        }
    } else {
        int found;

        auto pS = FindSkill(ItemDefs[item].pSkill);
        if (pS && !(pS->get().flags & SkillType::DISABLED)) {
            found = 0;
            for (i = 0; i < 4; i++)
                if (ItemDefs[item].pInput[i].item != -1)
                    found = 1;
            if (!found)
                temp += " This item is a trade resource.";
        }
    }

    if (ItemDefs[item].type & IT_MONEY) {
        temp += " This is the currency of " + Globals->WORLD_NAME + ".";
    }

    if (!full)
        return temp;

    auto attribtype = FindAttrib("observation");
    if (attribtype) {
        auto ap = attribtype->get();
        for (i = 0; i < 5; i++)
            if (ap.mods[i].flags & AttribModItem::ITEM) {
                std::string abbr = ItemDefs[item].abr;
                if (abbr == ap.mods[i].ident &&
                        ap.mods[i].modtype == AttribModItem::CONSTANT) {
                    temp += " This item grants a " + std::to_string(ap.mods[i].val) +
                        " point bonus to a unit's observation skill";
                    if (ap.mods[i].flags & AttribModItem::PERMAN) {
                        temp += " (note that a unit must possess one " + abbr + " for each man to gain this bonus)";
                    }
                    temp += ".";
                }
            }
    }
    attribtype = FindAttrib("stealth");
    if (attribtype) {
        auto ap = attribtype->get();
        for (i = 0; i < 5; i++)
            if (ap.mods[i].flags & AttribModItem::ITEM) {
                std::string abbr = ItemDefs[item].abr;
                if (abbr == ap.mods[i].ident && ap.mods[i].modtype == AttribModItem::CONSTANT) {
                    temp += " This item grants a " + std::to_string(ap.mods[i].val) +
                        " point bonus to a unit's stealth skill";
                    if (ap.mods[i].flags & AttribModItem::PERMAN) {
                        temp += " (note that a unit must possess one " + abbr + " for each man to gain this bonus)";
                    }
                    temp += ".";
                }
            }
    }
    attribtype = FindAttrib("wind");
    if (attribtype) {
        auto ap = attribtype->get();
        for (i = 0; i < 5; i++)
            if (ap.mods[i].flags & AttribModItem::ITEM) {
                std::string abbr = ItemDefs[item].abr;
                if (abbr == ap.mods[i].ident && ap.mods[i].modtype == AttribModItem::CONSTANT) {
                    if (Globals->FLEET_WIND_BOOST > 0) {
                        temp += " The possessor of this item will add " + std::to_string(Globals->FLEET_WIND_BOOST) +
                            " movement points to ships requiring up to " + std::to_string(ap.mods[i].val * 12) +
                            " sailing skill points. This bonus is not cumulative with a mage's summon wind skill.";
                    }
                }
            }
    }

    // special descriptions for items whose behaviour isn't fully
    // described by the data tables.  In an ideal world there
    // wouldn't be any of these...
    switch (item) {
        case I_RINGOFI:
            if (!(ItemDefs[I_AMULETOFTS].flags & ItemType::DISABLED)) {
                temp += " A Ring of Invisibility has one limitation; a unit possessing a RING cannot assassinate, nor steal from, a unit with an Amulet of True Seeing.";
            }
            break;
        case I_AMULETOFTS:
            if (!(ItemDefs[I_RINGOFI].flags & ItemType::DISABLED)) {
                temp += " Also, a unit with an Amulet of True Seeing cannot be assassinated by, nor have items stolen by, a unit with a Ring of Invisibility (note that the unit must have at least one Amulet of True Seeing per man in order to repel a unit with a Ring of Invisibility).";
            }
            break;
        case I_PORTAL:
            temp += " This item is required for mages to use the Portal Lore skill.";
            break;
        case I_STAFFOFH:
            temp += " This item allows its possessor to magically heal units after battle, as if their skill in Magical Healing was the highest of their manipulation, pattern, force and spirit skills, up to a maximum of level 2.";
            break;
        case I_RELICOFGRACE:
            temp += " This is a token of sacrifice, token of power. The Faction must posess 60 of them to get a WISH power and win the game. This relics can not be given and do not drop in spoils.";
            break;
        case I_HEALPOTION:
            temp += " This item allows its possessor to heal wounded units after battle. No skill is necessary to use this item; it will be used automatically when the possessor is involved in a battle. It can heal up to 1 casualties, with a 70 percent success rate. Healing consumes an item.";
            break;
        default:
            break;
    }

    auto pS = FindSkill(ItemDefs[item].grantSkill);
    if (pS && pS->get().flags & SkillType::CAST) {
        temp += " This item allows its possessor to CAST the " + pS->get().name + " spell as if their skill in " +
            pS->get().name + " was ";
        if (ItemDefs[item].minGrant < ItemDefs[item].maxGrant) {
            int count, found;
            count = 0;
            for (i = 0; i < 4; i++) {
                pS = FindSkill(ItemDefs[item].fromSkills[i]);
                if (pS && !(pS->get().flags & SkillType::DISABLED))
                    count++;
            }
            if (count > 1)
                temp += "the highest of ";
            else
                temp += "that of ";
            temp += "their ";
            found = 0;
            for (i = 0; i < 4; i++) {
                pS = FindSkill(ItemDefs[item].fromSkills[i]);
                if (!pS || (pS->get().flags & SkillType::DISABLED))
                    continue;
                if (found > 0) {
                    if ((found + 1) == count)
                        temp += " and ";
                    else
                        temp += ", ";
                }
                temp += pS->get().name;
                found++;
            }
            temp += " skill";
            if (count > 1)
                temp += "s";
            temp += ", up to a maximum of";
        }
        temp += " level " + std::to_string(ItemDefs[item].maxGrant) + ".";
        if (ItemDefs[item].minGrant > 1 &&
                ItemDefs[item].minGrant < ItemDefs[item].maxGrant) {
            temp += " A skill level of at least " + std::to_string(ItemDefs[item].minGrant) + " will always be granted.";
        }
    }

    if (ItemDefs[item].type & IT_BATTLE) {
        temp += " This item is a miscellaneous combat item.";
        auto bt = find_battle_item(ItemDefs[item].abr);
        if (bt) {
            if (bt->get().flags & BattleItemType::MAGEONLY) {
                temp += " This item may only be used by a mage";
                if (Globals->APPRENTICES_EXIST) {
                    temp += " or an " + Globals->APPRENTICE_NAME;
                }
                temp += ".";
            }
            if (bt->get().flags & BattleItemType::SHIELD)
                temp += " This item provides ";
            else
                temp += " This item can cast ";
            temp += show_special(bt->get().special ? bt->get().special : "", bt->get().skillLevel, 1, bt->get().flags & BattleItemType::SHIELD);
        }
    } else if (ItemDefs[item].type & IT_MAGEONLY) {
        temp += " This item may only be used by a mage";
        if (Globals->APPRENTICES_EXIST) {
            temp += " or an " + Globals->APPRENTICE_NAME;
        }
        temp += ".";
    }
    if (ItemDefs[item].type & IT_FOOD) {
        temp += " This item can be eaten to provide " + std::to_string(Globals->UPKEEP_FOOD_VALUE) +
            " silver towards a unit's maintenance cost.";
    }
    if (ItemDefs[item].flags & ItemType::CANTGIVE) {
        temp += " This item cannot be given to other units.";
    }

    if (ItemDefs[item].max_inventory) {
        temp += " A unit may have at most " + item_string(item, ItemDefs[item].max_inventory, FULLNUM) + ".";
    }

    return temp;
}

int IsSoldier(int item)
{
    if (ItemDefs[item].type & IT_MAN || ItemDefs[item].type & IT_MONSTER)
        return 1;
    return 0;
}


Item::Item()
{
    selling = 0;
    checked = 0;
}

Item::~Item()
{
}

std::string Item::report(bool see_illusions)
{
    std::string ret = "";
    // special handling of the unfinished ship items
    if (ItemDefs[type].type & IT_SHIP) {
        ret += "unfinished " + ItemDefs[type].name + " [" + ItemDefs[type].abr + "] (needs " + std::to_string(num) + ")";
    } else ret += item_string(type,num);
    if (see_illusions && (ItemDefs[type].type & IT_ILLUSION)) {
        ret = ret + " (illusion)";
    }
    return ret;
}

void Item::Writeout(ostream& f)
{
    if (type != -1) {
        f << num << " ";
        if (ItemDefs[type].type & IT_ILLUSION) f << "i";
        f << ItemDefs[type].abr << '\n';
    } else
        f << "-1 NO_ITEM\n";
}

void Item::Readin(istream &f)
{
    std::string temp;
    f >> ws >> num >> temp;
    type = lookup_item(temp);
}

void ItemList::Writeout(ostream& f)
{
    f << size() << "\n";
    for(auto i : items) i->Writeout(f);
}

void ItemList::Readin(istream &f)
{
    int i;
    f >> i;
    for (int j = 0; j < i; j++) {
        Item *temp = new Item;
        temp->Readin(f);
        if (temp->type < 0 || temp->num < 1 || ItemDefs[temp->type].flags & ItemType::DISABLED) {
            delete temp;
            continue;
        }
        items.push_back(temp);
    }
}

int ItemList::GetNum(int t)
{
    for(auto i : items) {
        if (i->type == t) return i->num;
    }
    return 0;
}

int ItemList::Weight()
{
    int wt = 0;
    int frac = 0;
    for(auto i : items) {
        // Exempt unfinished ships from weight calculations: these just get removed when the unit moves.
        if (ItemDefs[i->type].type & IT_SHIP) continue;
        if (ItemDefs[i->type].weight == 0) frac += i->num;
        else wt += ItemDefs[i->type].weight * i->num;
    }
    if (Globals->FRACTIONAL_WEIGHT > 0 && frac != 0) wt += (frac/Globals->FRACTIONAL_WEIGHT);
    return wt;
}

int ItemList::CanSell(int t)
{
    for(auto i : items) {
        if (i->type == t) return i->num - i->selling;
    }
    return 0;
}

void ItemList::Selling(int t, int n)
{
    for(auto i : items) {
        if (i->type == t) i->selling += n;
    }
}

void ItemList::UncheckAll()
{
    for(auto i : items) i->checked = 0;
}

std::string ItemList::report(int obs,int seeillusions,int nofirstcomma)
{
    UncheckAll();
    std::string temp;
    for (int s = 0; s < 7; s++) {
        temp += report_by_type(s, obs, seeillusions, nofirstcomma);
        if (!temp.empty()) nofirstcomma = 0;
    }
    return temp;
}

std::string ItemList::battle_report()
{
    std::string temp;
    for(auto i : items) {
        if (ItemDefs[i->type].combat) {
            temp += ", ";
            temp += i->report(false);
            if (ItemDefs[i->type].type & IT_MONSTER) {
                auto monster = find_monster(ItemDefs[i->type].abr, (ItemDefs[i->type].type & IT_ILLUSION))->get();
                temp += " (Combat " + std::to_string(monster.attackLevel) + "/" +
                    std::to_string(monster.defense[ATTACK_COMBAT]) + ", Attacks " + std::to_string(monster.numAttacks) +
                    ", Hits " + std::to_string(monster.hits) + ", Tactics " + std::to_string(monster.tactics) + ")";
            }
        }
    }
    return temp;
}

std::string ItemList::report_by_type(int type, int obs, int seeillusions, int nofirstcomma)
{
    std::string temp;
    for(auto i : items) {
        int report = 0;
        if (i->checked) continue;

        bool battle_item = (
            (ItemDefs[i->type].type & IT_WEAPON) || (ItemDefs[i->type].type & IT_BATTLE) ||
            (ItemDefs[i->type].type & IT_ARMOR) || (ItemDefs[i->type].type & IT_MAGIC)
        );

        switch (type) {
            case 0:
                if (ItemDefs[i->type].type & IT_MAN) report = 1;
                break;
            case 1:
                if (ItemDefs[i->type].type & IT_MONSTER) report = 1;
                break;
            case 2:
                if (battle_item) report = 1;
                break;
            case 3:
                if (ItemDefs[i->type].type & IT_MOUNT) report = 1;
                break;
            case 4:
                if ((i->type == I_WAGON) || (i->type == I_MWAGON)) report = 1;
                break;
            case 5:
                report = 1;
                if (ItemDefs[i->type].type & IT_MAN) report = 0;
                if (ItemDefs[i->type].type & IT_MONSTER) report = 0;
                if (i->type == I_SILVER) report = 0;
                if (battle_item) report = 0;
                if (ItemDefs[i->type].type & IT_MOUNT) report = 0;
                if ((i->type == I_WAGON) || (i->type == I_MWAGON)) report = 0;
                break;
            case 6:
                if (i->type == I_SILVER) report = 1;
        }
        if (report) {
            if (obs == 2) {
                if (nofirstcomma) nofirstcomma = 0;
                else temp += ", ";
                temp += i->report(seeillusions);
            } else {
                if (ItemDefs[i->type].weight) {
                    if (nofirstcomma) nofirstcomma = 0;
                    else temp += ", ";
                    temp += i->report(seeillusions);
                }
            }
            i->checked = 1;
        }
    }
    return temp;
}

void ItemList::SetNum(int t,int n)
{
    // sanity check: does this item type exist?
    if ((t<0) || (t>=NITEMS)) return;
    if (n) {
        for(auto i : items) {
            if (i->type == t) {
                i->num = n;
                return;
            }
        }
        Item *i = new Item;
        i->type = t;
        i->num = n;
        items.push_back(i);
    } else {
        // this is safe *only* because we return
        for(auto i : items) {
            if (i->type == t) {
                std::erase(items, i);
                delete i;
                return;
            }
        }
    }
}

bool ManType::CanProduce(int item)
{
    if (ItemDefs[item].flags & ItemType::DISABLED) return false;
    if (!ItemDefs[item].pSkill) return false;
    for (unsigned int i=0; i<(sizeof(skills)/sizeof(skills[0])); i++) {
        if (skills[i] == std::nullopt) continue;
        if (skills[i] == ItemDefs[item].pSkill) return true;
    }
    return false;
}

bool ManType::CanUse(int item)
{
    if (ItemDefs[item].flags & ItemType::DISABLED) return false;

    // Check if the item is a mount
    if (ItemDefs[item].type & IT_MOUNT) return true;

    // Check if the item is a weapon
    if (ItemDefs[item].type & IT_WEAPON) return true;

    // Check if the item is a tool
    if (ItemDefs[item].type & IT_TOOL) return true;

    // Check if the item is an armor
    if (ItemDefs[item].type & IT_ARMOR) {
        auto armor = find_armor(ItemDefs[item].abr)->get();
        int p = armor.from / armor.saves[3];
        if (p > 4) {
            // puny armor not used by combative races
            bool mayWearArmor = true;
            for (unsigned int i = 0; i < (sizeof(skills) / sizeof(skills[0])); i++) {
                if (skills[i] == std::nullopt) continue;
                auto pS = FindSkill(skills[i]->c_str());
                if (!pS) continue;
                if (pS->get().abbr == "COMB") mayWearArmor = false;
            }
            if (mayWearArmor) return true;
            return false;
        }
        if (p > 3) return true;
        // heavy armor not be worn by sailors and sneaky races
        bool mayWearArmor = true;
        for (unsigned int i = 0; i<(sizeof(skills) / sizeof(skills[0])); i++) {
            if (skills[i] == std::nullopt) continue;
            auto pS = FindSkill(skills[i]->c_str());
            if (!pS) continue;
            std::string skill_tag = pS->get().abbr;
            if (skill_tag == "SAIL" || skill_tag == "HUNT" || skill_tag == "STEA" || skill_tag == "LBOW")
                mayWearArmor = false;
        }
        if (mayWearArmor) return true;
        return false;
    }

    return false;
}
