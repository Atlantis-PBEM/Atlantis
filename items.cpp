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

const int MonType::getAggression() {
    int aggression = this->hostile;
    aggression *= Globals->MONSTER_ADVANCE_HOSTILE_PERCENT;
    aggression /= 100;
    if (aggression < Globals->MONSTER_ADVANCE_MIN_PERCENT) {
        aggression = Globals->MONSTER_ADVANCE_MIN_PERCENT;
    }

    return aggression;
}

std::optional<std::reference_wrapper<BattleItemType>> FindBattleItem(char const *abbr)
{
    if (abbr == NULL) return std::nullopt;
    strings::ci_string tag(abbr);

    auto it = std::find_if(BattleItemDefs.begin(), BattleItemDefs.end(), [tag](const BattleItemType& battleItem) {
        return tag == battleItem.abbr;
    });
    if (it != BattleItemDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<ArmorType>> FindArmor(char const *abbr)
{
    if (abbr == NULL) return std::nullopt;
    strings::ci_string tag(abbr);

    auto it = std::find_if(ArmorDefs.begin(), ArmorDefs.end(), [tag](const ArmorType& armor) {
        return tag == armor.abbr;
    });
    if (it != ArmorDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<WeaponType>> FindWeapon(char const *abbr)
{
    if (abbr == NULL) return std::nullopt;
    strings::ci_string tag(abbr);

    auto it = std::find_if(WeaponDefs.begin(), WeaponDefs.end(), [tag](const WeaponType& weapon) {
        return tag == weapon.abbr;
    });
    if (it != WeaponDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<ItemType>> FindItem(char const *abbr)
{
    if (abbr == NULL) return std::nullopt;
    strings::ci_string tag(abbr);

    auto it = std::find_if(ItemDefs.begin(), ItemDefs.end(), [tag](const ItemType& item) {
        return tag == item.abr;
    });
    if (it != ItemDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<MountType>> FindMount(char const *abbr)
{
    if (abbr == NULL) return std::nullopt;
    strings::ci_string tag(abbr);

    auto it = std::find_if(MountDefs.begin(), MountDefs.end(), [tag](const MountType& mount) {
        return tag == mount.abbr;
    });
    if (it != MountDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<MonType>> FindMonster(char const *abbr, int illusion)
{
    if (abbr == nullptr) return std::nullopt;
    strings::ci_string tag = (illusion ? "i" : "") + std::string(abbr);

    auto it = std::find_if(MonDefs.begin(), MonDefs.end(), [tag](const MonType& mon) {
        return tag == mon.abbr;
    });
    if (it != MonDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<ManType>> FindRace(char const *abbr)
{
    if (abbr == nullptr) return std::nullopt;
    strings::ci_string tag(abbr);

    auto it = std::find_if(ManDefs.begin(), ManDefs.end(), [tag](const ManType& man) {
        return tag == man.abbr;
    });
    if (it != ManDefs.end()) return std::ref(*it);

    return std::nullopt;
}

AString AttType(int atype)
{
    switch(atype) {
        case ATTACK_COMBAT: return AString("melee");
        case ATTACK_ENERGY: return AString("energy");
        case ATTACK_SPIRIT: return AString("spirit");
        case ATTACK_WEATHER: return AString("weather");
        case ATTACK_RIDING: return AString("riding");
        case ATTACK_RANGED: return AString("ranged");
        case NUM_ATTACK_TYPES: return AString("non-resistable");
        default: return AString("unknown");
    }
}

static AString DefType(int atype)
{
    if (atype == NUM_ATTACK_TYPES) return AString("all");
    return AttType(atype);
}

int lookup_item(const strings::ci_string& name)
{
    for (int i = 0; i < NITEMS; i++) {
        std::string item_name = (ItemDefs[i].type & IT_ILLUSION ? "i" : "") + std::string(ItemDefs[i].abr);
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
        std::string itemAbr = (illusion ? "i" : "") + std::string(ItemDefs[i].abr);

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

string ItemString(int type, int num, int flags)
{
    string temp;
    if (num == 1) {
        if (flags & FULLNUM)
            temp += to_string(num) + " ";
        temp += string((flags & ALWAYSPLURAL) ? ItemDefs[type].names : ItemDefs[type].name) +
            " [" + ItemDefs[type].abr + "]";
    } else {
        if (num == -1) {
            temp += string("unlimited ") + ItemDefs[type].names + " [" + ItemDefs[type].abr + "]";
        } else {
            temp += to_string(num) + " " + ItemDefs[type].names + " [" + ItemDefs[type].abr + "]";
        }
    }
    return temp;
}

static AString EffectStr(char const *effect)
{
    AString temp, temp2;
    int comma = 0;
    int i;

    auto ep = FindEffect(effect).value().get();

    temp += ep.name;

    if (ep.attackVal) {
        temp2 += AString(ep.attackVal) + " to attack";
        comma = 1;
    }

    for (i = 0; i < 4; i++) {
        if (ep.defMods[i].type == -1) continue;
        if (comma) temp2 += ", ";
        temp2 += AString(ep.defMods[i].val) + " versus " + DefType(ep.defMods[i].type) + " attacks";
        comma = 1;
    }

    if (comma) {
        temp += AString(" (") + temp2 + ")";
        temp += (ep.flags & EffectType::EFF_ONESHOT) ? " for their next attack" : " for the rest of the battle";
    }

    temp += ".";

    if (ep.cancelEffect != NULL) {
        if (comma) temp += " ";
        auto up = FindEffect(ep.cancelEffect).value().get();
        temp += AString("This effect cancels out the effects of ") + up.name + ".";
    }
    return temp;
}

static AString AttackDamageDescription(const int damage) {
    AString temp;

    if (damage <= 0) {
        temp = "attack deals no damage";
        return temp;
    }

    // Full skill level damage
    if (damage >= WeaponType::NUM_DAMAGE_SKILL) {
        temp = "attack deals the skill level points of damage";
        return temp;
    }

    // Half skill damage
    if (damage >= WeaponType::NUM_DAMAGE_HALF_SKILL) {
        temp = "attack deals does half the skill level (rounded up) points of damage";
        return temp;
    }

    // Just damage
    temp = AString("attack deals ") + damage + " damage";
    return temp;
}

AString ShowSpecial(char const *special, int level, int expandLevel, int fromItem)
{
    AString temp;
    int comma = 0;
    int i;
    int last = -1;
    int val;

    auto spd = FindSpecial(special).value().get();
    temp += spd.specialname;
    temp += AString(" in battle");
    if (expandLevel)
        temp += AString(" at a skill level of ") + level;
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
            temp += AString(ObjectDefs[spd.buildings[last]].name) + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "or ";
        }
        temp += AString(ObjectDefs[spd.buildings[last]].name) + ".";
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
            temp += ItemString(spd.targets[last], 1, ALWAYSPLURAL) + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "or ";
        }
        temp += ItemString(spd.targets[last], 1, ALWAYSPLURAL) + ".";
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
            temp += AString(ep.name) + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "or ";
        }
        auto ep = FindEffect(spd.effects[last]).value().get();
        temp += AString(ep.name) + ".";
    }
    if (spd.targflags & SpecialType::HIT_ILLUSION) {
        temp += " This ability will only target illusions.";
    }
    if (spd.targflags & SpecialType::HIT_NOMONSTER) {
        temp += " This ability cannot target monsters.";
    }
    if (spd.effectflags & SpecialType::FX_NOBUILDING) {
        temp += AString(" The bonus given to units inside buildings is ") +
            "not effective against this ability.";
    }

    if (spd.effectflags & SpecialType::FX_SHIELD) {
        if (!fromItem)
            temp += " This spell provides a shield against all ";
        else
            temp += AString(" This ability provides the wielder with a defence bonus of ") + level + " against all ";
        comma = 0;
        last = -1;
        for (i = 0; i < 4; i++) {
            if (spd.shield[i] == -1) continue;
            if (last == -1) {
                last = i;
                continue;
            }
            temp += DefType(spd.shield[last]) + ", ";
            last = i;
            comma++;
        }
        if (comma) {
            temp += "and ";
        }
        if (fromItem)
            temp += DefType(spd.shield[last]) + " attacks.";
        else {
            temp += DefType(spd.shield[last]) + " attacks against the entire" +
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

            temp += AString("a defensive bonus of ") + val;
            if (!expandLevel) {
                temp += " per skill level";
            }
            temp += AString(" versus ") + DefType(spd.defs[last].type) +
                " attacks, ";
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
        temp += AString("a defensive bonus of ") + val;
        if (!expandLevel) {
            temp += " per skill level";
        }
        temp += AString(" versus ") + DefType(spd.defs[last].type) +
            " attacks";
        temp += " to the user.";
    }

    /* Now the damages */
    for (i = 0; i < 4; i++) {
        if (spd.damage[i].type == -1) continue;
        temp += AString(" This ability does between ") +
            spd.damage[i].minnum + " and ";
        val = spd.damage[i].value * 2;
        if (expandLevel) {
            if (spd.effectflags & SpecialType::FX_USE_LEV)
                val *= level;
        }
        temp += AString(val);
        if (!expandLevel) {
            temp += " times the skill level of the mage";
        }
        temp += AString(" ") + AttType(spd.damage[i].type) + " attacks and each " + AttackDamageDescription(spd.damage[i].hitDamage) + ".";
        if (spd.damage[i].effect) {
            temp += " Each attack causes the target to be effected by ";
            temp += EffectStr(spd.damage[i].effect);
        }
    }

    return temp;
}

static AString MonResist(int type, int val, int full)
{
    AString temp = "This monster ";
    if (full) {
        temp += AString("has a resistance of ") + val;
    } else {
        temp += "is ";
        if (val < 1) temp += "very susceptible";
        else if (val == 1) temp += "susceptible";
        else if (val > 1 && val < 3) temp += "typically resistant";
        else if (val > 2 && val < 5) temp += "slightly resistant";
        else temp += "very resistant";
    }
    temp += " to ";
    temp += AttType(type);
    temp += " attacks.";
    return temp;
}

static AString FMIResist(int type, int val, int full)
{
    AString temp = "This FMI ";
    if (full) {
        temp += AString("has a resistance of ") + val;
    } else {
        temp += "is ";
        if (val < 1) temp += "very susceptible";
        else if (val == 1) temp += "susceptible";
        else if (val > 1 && val < 3) temp += "typically resistant";
        else if (val > 2 && val < 5) temp += "slightly resistant";
        else temp += "very resistant";
    }
    temp += " to ";
    temp += AttType(type);
    temp += " attacks.";
    return temp;
}

static AString WeapClass(int wclass)
{
    switch(wclass) {
        case SLASHING: return AString("slashing");
        case PIERCING: return AString("piercing");
        case CRUSHING: return AString("crushing");
        case CLEAVING: return AString("cleaving");
        case ARMORPIERCING: return AString("armor-piercing");
        case MAGIC_ENERGY: return AString("energy");
        case MAGIC_SPIRIT: return AString("spirit");
        case MAGIC_WEATHER: return AString("weather");
        default: return AString("unknown");
    }
}

static AString WeapType(int flags, int wclass)
{
    AString type;
    if (flags & WeaponType::RANGED) type = "ranged ";
    if (flags & WeaponType::LONG) type = "long ";
    if (flags & WeaponType::SHORT) type = "short ";
    type += WeapClass(wclass);
    return type;
}

string ShowItem::display_name()
{
    if (ItemDefs[item].type & IT_ILLUSION) {
        return "illusory " + ItemDefs[item].name;
    }
    return ItemDefs[item].name;
}

string ShowItem::display_tag() {
    if (ItemDefs[item].type & IT_ILLUSION) {
        return "I" + string(ItemDefs[item].abr);
    }
    return ItemDefs[item].abr;

}

AString *ItemDescription(int item, int full)
{
    int i;
    AString skname;

    if (ItemDefs[item].flags & ItemType::DISABLED)
        return new AString("");

    AString *temp = new AString;
    int illusion = (ItemDefs[item].type & IT_ILLUSION);

    *temp += AString(illusion?"illusory ":"")+ ItemDefs[item].name + " [" +
            (illusion?"I":"") + ItemDefs[item].abr + "]";

    /* new ship items */
    if (ItemDefs[item].type & IT_SHIP) {
        if (ItemDefs[item].swim > 0) {
            *temp += AString(". This is a ship with a capacity of ") + ItemDefs[item].swim;
        }
        if (ItemDefs[item].fly > 0) {
            *temp += AString(". This is a flying 'ship' with a capacity of ") + ItemDefs[item].fly;
        }
        *temp += " and a speed of ";
        *temp += ItemDefs[item].speed;
        *temp += " hex";
        if (ItemDefs[item].speed != 1)
            *temp += "es";
        *temp += " per month";
        *temp += AString(". This ship requires a total of ") + ItemDefs[item].weight/50 + " levels of sailing skill to sail";
        int objectno = lookup_object(ItemDefs[item].name);
        if (objectno >= 0) {
            if (ObjectDefs[objectno].protect > 0) {
                *temp += ". This ship provides defense to the first ";
                *temp += ObjectDefs[objectno].protect;
                *temp += " men inside it, ";
                // Now add the description to temp
                *temp += AString("giving a defensive bonus of ");

                std::vector<std::string> defences;
                for (int i=0; i<NUM_ATTACK_TYPES; i++) {
                    if (ObjectDefs[objectno].defenceArray[i]) {
                        std::string def = std::to_string(ObjectDefs[objectno].defenceArray[i]) + " against " +
                            std::string(AttType(i).const_str()) + " attacks";
                        defences.push_back(def);
                    }
                }
                *temp += strings::join(defences, ", ", " and ");
            }
            if (Globals->LIMITED_MAGES_PER_BUILDING && ObjectDefs[objectno].maxMages > 0) {
                *temp += ". This ship will allow ";
                if (ObjectDefs[objectno].maxMages > 1) {
                    *temp += "up to ";
                    *temp += ObjectDefs[objectno].maxMages;
                    *temp += " mages";
                } else {
                    *temp += "one mage";
                }
                *temp += " to study above level 2";
            }
        }
    } else {

        *temp += AString(", weight ") + ItemDefs[item].weight;

        if (ItemDefs[item].walk) {
            int cap = ItemDefs[item].walk - ItemDefs[item].weight;
            if (cap) {
                *temp += AString(", walking capacity ") + cap;
            } else {
                *temp += ", can walk";
            }
        }
        if ((ItemDefs[item].hitchItem != -1 )&&
                !(ItemDefs[ItemDefs[item].hitchItem].flags & ItemType::DISABLED)) {
            int cap = ItemDefs[item].walk - ItemDefs[item].weight +
                ItemDefs[item].hitchwalk;
            if (cap) {
                *temp += AString(", walking capacity ") + cap +
                    " when hitched to a " +
                    ItemDefs[ItemDefs[item].hitchItem].name;
            }
        }
        if (ItemDefs[item].ride) {
            int cap = ItemDefs[item].ride - ItemDefs[item].weight;
            if (cap) {
                *temp += AString(", riding capacity ") + cap;
            } else {
                *temp += ", can ride";
            }
        }
        if (ItemDefs[item].swim) {
            int cap = ItemDefs[item].swim - ItemDefs[item].weight;
            if (cap) {
                *temp += AString(", swimming capacity ") + cap;
            } else {
                *temp += ", can swim";
            }
        }
        if (ItemDefs[item].fly) {
            int cap = ItemDefs[item].fly - ItemDefs[item].weight;
            if (cap) {
                *temp += AString(", flying capacity ") + cap;
            } else {
                *temp += ", can fly";
            }
        }
        if (ItemDefs[item].speed) {
            *temp += ", moves ";
            *temp += ItemDefs[item].speed;
            *temp += " hex";
            if (ItemDefs[item].speed != 1)
                *temp += "es";
            *temp += " per month";
        }
    }

    if (ItemDefs[item].flags & ItemType::NOSTEALTH) {
        *temp += ". This item prevents the unit it is with from being stealthy";
    }
    if (ItemDefs[item].flags & ItemType::NO_SHAFT) {
        *temp += ". This item prevents the unit it is with from entering a shaft";
    }
    if (ItemDefs[item].flags & ItemType::SEEK_ALTAR) {
        *temp += ". This item desires to move toward the nearest Ritual Altar";
    }
    if (ItemDefs[item].flags & ItemType::MAINTENANCE) {
        *temp += ". This item requires maintenance each turn of ";
        *temp += ItemDefs[item].baseprice;
        *temp += " silver";
        if (ItemDefs[item].flags & ItemType::SEEK_ALTAR) {
            *temp += ". Moving toward the altar will reduce the maintenance cost in half";
            *temp += ". Moving away from the altar will multiply the maintenance cost 5 time";
        }
    }


    if (Globals->ALLOW_WITHDRAW) {
        if (ItemDefs[item].type & IT_NORMAL && item != I_SILVER) {
            *temp += AString(", costs ") + (ItemDefs[item].baseprice*5/2) +
                " silver to withdraw";
        }
    }
    *temp += ".";

    if (ItemDefs[item].type & IT_MAN) {
        auto mt = FindRace(ItemDefs[item].abr)->get();
        AString mani = "MANI";
        AString last = "";
        int found = 0;
        *temp += " This race may study ";
        unsigned int c;
        unsigned int len = sizeof(mt.skills) / sizeof(mt.skills[0]);
        for (c = 0; c < len; c++) {
            auto pS = FindSkill(mt.skills[c]);
            if (!pS) continue;
            if (mani == pS->get().abbr && Globals->MAGE_NONLEADERS) {
                if (!(last == "")) {
                    if (found)
                        *temp += ", ";
                    *temp += last;
                    found = 1;
                }
                last = "all magical skills";
                continue;
            }
            if (pS->get().flags & SkillType::DISABLED) continue;
            if (!(last == "")) {
                if (found)
                    *temp += ", ";
                *temp += last;
                found = 1;
            }
            last = SkillStrs(pS->get());
        }
        if (!(last == "")) {
            if (found)
                *temp += " and ";
            *temp += last;
            found = 1;
        }
        if (found) {
            *temp += AString(" to level ") + mt.speciallevel +
                " and all other skills to level " +
                mt.defaultlevel + ".";
        } else {
            *temp += AString("all skills to level ") + mt.defaultlevel + ".";
        }
    }

    if ((ItemDefs[item].type & IT_MONSTER) && !(ItemDefs[item].flags & ItemType::MANPRODUCE)) {
        *temp += " This is a monster.";
        auto monster = FindMonster(ItemDefs[item].abr, (ItemDefs[item].type & IT_ILLUSION))->get();
        *temp += AString(" This monster attacks with a combat skill of ") + monster.attackLevel;

        for (int c = 0; c < NUM_ATTACK_TYPES; c++) {
            *temp += AString(" ") + MonResist(c,monster.defense[c], full);
        }

        if (monster.special && monster.special != NULL) {
            *temp += AString(" ") +
                "Monster can cast " +
                ShowSpecial(monster.special, monster.specialLevel, 1, 0);
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
                *temp += " The monster can spawn in the wilderness of " + strings::join(spawnIn, ", ", " and ") + ".";
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
                *temp += " This monster can lair in the " + strings::join(lairIn, ", ", " and ") + ".";
            }
        }

        if (monster.preferredTerrain.empty() && monster.forbiddenTerrain.empty()) {
            *temp += " The monster has no terrain preferences, and it can travel through any terrain.";
        }

        if (!monster.forbiddenTerrain.empty()) {
            std::vector<std::string> list;
            for (auto &terrain : monster.forbiddenTerrain) {
                list.push_back(TerrainDefs[terrain].name);
            }

            *temp += " Monster severely dislikes " + strings::join(list, ",", " and ") + " " +
                strings::plural(monster.forbiddenTerrain.size(), "terrain", "terrains") + " and will never try to enter them.";
        }

        if (!monster.preferredTerrain.empty()) {
            *temp += " Monster prefers to roam the";

            bool isNext = false;
            for (auto &terrain : monster.preferredTerrain) {
                *temp += (isNext ? ", " : " ") + std::string(TerrainDefs[terrain].name);
                isNext = true;
            }

            *temp += " " + strings::plural(monster.preferredTerrain.size(), "terrain", "terrains") + ".";
        }

        const int aggression = monster.getAggression();
        if (aggression >= 100) {
            *temp += " Monster is unbelievably aggressive and will attack player units on sight.";
        }
        else if (aggression >= 75) {
            *temp += " Monster is exceptionally aggressive, and there is a slight chance he will not attack player units.";
        }
        else if (aggression >= 50) {
            *temp += " Monster is very aggressive, but he will not harm player units with good luck.";
        }
        else if (aggression >= 25) {
            *temp += " Monster is aggressive but, in most cases, will leave player units alone.";
        }
        else if (aggression > 0) {
            *temp += " Monster is unfriendly, and the player must be pretty unlucky to be attacked by this monster.";
        }
        else {
            *temp += " Monster is totally peaceful and will never attack player units.";
        }

        if (full) {
            int hits = monster.hits;
            int atts = monster.numAttacks;
            int regen = monster.regen;
            if (!hits) hits = 1;
            if (!atts) atts = 1;
            *temp += " This monster has " + std::to_string(atts) + " melee " +
                strings::plural(atts, "attack", "attacks") + " per round and takes " +
                std::to_string(hits) + " " + strings::plural(hits, "hit", "hits") + " to kill";

            if (atts > 0) {
                *temp += AString(" and each ") + AttackDamageDescription(monster.hitDamage);
            }

            *temp += AString(".");

            if (regen > 0) {
                *temp += AString(" This monsters regenerates ") + regen +
                    " hits per round of battle.";
            }
            *temp += AString(" This monster has a tactics score of ") +
                monster.tactics + ", a stealth score of " + monster.stealth +
                ", and an observation score of " + monster.obs + ".";
        }
        *temp += " This monster might have ";
        if (monster.spoiltype != -1) {
            if (monster.spoiltype & IT_MAGIC) {
                *temp += "magic items and ";
            } else if (monster.spoiltype & IT_ADVANCED) {
                *temp += "advanced items and ";
            } else if (monster.spoiltype & IT_NORMAL) {
                *temp += "normal or trade items and ";
            }
        }
        *temp += "silver as treasure.";
    }

    if(ItemDefs[item].flags & ItemType::MANPRODUCE) {
        *temp += " This is a free-moving-item (FMI).";
        auto monster = FindMonster(ItemDefs[item].abr, (ItemDefs[item].type & IT_ILLUSION))->get();
        *temp += AString(" This FMI attacks with a combat skill of ") + monster.attackLevel + ".";

        for (int c = 0; c < NUM_ATTACK_TYPES; c++) {
            *temp += AString(" ") + FMIResist(c, monster.defense[c], full);
        }

        if (monster.special && monster.special != NULL) {
            *temp += AString(" ") +
                "FMI can cast " +
                ShowSpecial(monster.special, monster.specialLevel, 1, 0);
        }

        if (full) {
            int hits = monster.hits;
            int atts = monster.numAttacks;
            int regen = monster.regen;
            if (!hits) hits = 1;
            if (!atts) atts = 1;
            *temp += AString(" This FMI has ") + atts + " melee " +
                ((atts > 1)?"attacks":"attack") + " per round and takes " +
                hits + " " + ((hits > 1)?"hits":"hit") + " to kill";

            if (atts > 0) {
                *temp += AString(" and each ") + AttackDamageDescription(monster.hitDamage);
            }

            *temp += AString(".");

            if (regen > 0) {
                *temp += AString(" This FMI regenerates ") + regen + " hits per round of battle.";
            }
            *temp += AString(" This FMI has a tactics score of ") +
                monster.tactics + ", a stealth score of " + monster.stealth +
                ", and an observation score of " + monster.obs + ".";
        }

        if (monster.spoiltype != -1) {
            *temp += " This FMI might have ";

            if (monster.spoiltype & IT_MAGIC) {
                *temp += "magic items and ";
            } else if (monster.spoiltype & IT_ADVANCED) {
                *temp += "advanced items and ";
            } else if (monster.spoiltype & IT_NORMAL) {
                *temp += "normal or trade items and ";
            }

            *temp += "silver as treasure.";
        }
    }

    if (ItemDefs[item].type & IT_WEAPON) {
        auto weapon = FindWeapon(ItemDefs[item].abr)->get();
        *temp += " This is a ";
        *temp += WeapType(weapon.flags, weapon.weapClass) + " weapon and each " + AttackDamageDescription(weapon.hitDamage) + ".";
        if (weapon.flags & WeaponType::NEEDSKILL) {
            auto pS = FindSkill(weapon.baseSkill);
            if (pS) {
                *temp += AString(" Knowledge of ") + SkillStrs(pS->get());
                pS = FindSkill(weapon.orSkill);
                if (pS)
                    *temp += AString(" or ") + SkillStrs(pS->get());
                *temp += " is needed to wield this weapon.";
            }
        } else
            *temp += " No skill is needed to wield this weapon.";

        int flag = 0;
        if (weapon.attackBonus != 0) {
            *temp += " This weapon grants a ";
            *temp += ((weapon.attackBonus > 0) ? "bonus of " : "penalty of ");
            *temp += abs(weapon.attackBonus);
            *temp += " on attack";
            flag = 1;
        }
        if (weapon.defenseBonus != 0) {
            if (flag) {
                if (weapon.attackBonus == weapon.defenseBonus) {
                    *temp += " and defense.";
                    flag = 0;
                } else {
                    *temp += " and a ";
                }
            } else {
                *temp += " This weapon grants a ";
                flag = 1;
            }
            if (flag) {
                *temp += ((weapon.defenseBonus > 0)?"bonus of ":"penalty of ");
                *temp += abs(weapon.defenseBonus);
                *temp += " on defense.";
                flag = 0;
            }
        }
        if (flag) *temp += ".";
        if (weapon.mountBonus && full) {
            *temp += " This weapon ";
            if (weapon.attackBonus != 0 || weapon.defenseBonus != 0)
                *temp += "also ";
            *temp += "grants a ";
            *temp += ((weapon.mountBonus > 0)?"bonus of ":"penalty of ");
            *temp += abs(weapon.mountBonus);
            *temp += " against mounted opponents.";
        }

        if (weapon.flags & WeaponType::NOFOOT)
            *temp += " Only mounted troops may use this weapon.";
        else if (weapon.flags & WeaponType::NOMOUNT)
            *temp += " Only foot troops may use this weapon.";

        if (weapon.flags & WeaponType::RIDINGBONUS) {
            *temp += " Wielders of this weapon, if mounted, get their riding "
                "skill bonus on combat attack and defense.";
        } else if (weapon.flags & WeaponType::RIDINGBONUSDEFENSE) {
            *temp += " Wielders of this weapon, if mounted, get their riding "
                "skill bonus on combat defense.";
        }

        if (weapon.flags & WeaponType::NODEFENSE) {
            *temp += " Defenders are treated as if they have an "
                "effective combat skill of 0.";
        }

        if (weapon.flags & WeaponType::NOATTACKERSKILL) {
            *temp += " Attackers do not get skill bonus on defense.";
        }

        if (weapon.flags & WeaponType::ALWAYSREADY) {
            *temp += " Wielders of this weapon never miss a round to ready "
                "their weapon.";
        } else {
            *temp += " There is a 50% chance that the wielder of this weapon "
                "gets a chance to attack in any given round.";
        }

        if (full) {
            int atts = weapon.numAttacks;
            *temp += AString(" This weapon attacks versus the target's ") +
                "defense against " + AttType(weapon.attackType) + " attacks.";
            *temp += AString(" This weapon allows ");
            if (atts > 0) {
                if (atts >= WeaponType::NUM_ATTACKS_HALF_SKILL) {
                    int max = WeaponType::NUM_ATTACKS_HALF_SKILL;
                    char const *attd = "half the skill level (rounded up)";
                    if (atts >= WeaponType::NUM_ATTACKS_SKILL) {
                        max = WeaponType::NUM_ATTACKS_SKILL;
                        attd = "the skill level";
                    }
                    *temp += "a number of attacks equal to ";
                    *temp += attd;
                    *temp += " of the attacker";
                    int val = atts - max;
                    if (val > 0) *temp += AString(" plus ") + val;
                } else {
                    *temp += AString(atts) + ((atts==1)?" attack":" attacks");
                }
                *temp += " per round.";
            } else {
                atts = -atts;
                *temp += "1 attack every ";
                if (atts == 1) *temp += "round .";
                else *temp += AString(atts) + " rounds.";
            }

            for (int i = 0; i < MAX_WEAPON_BM_TARGETS; i++) {
                WeaponBonusMalus *bm = &weapon.bonusMalus[i];
                if (!bm->weaponAbbr) continue;
                if (bm->attackModifer == 0 && bm->defenseModifer == 0) continue;

                auto target = FindItem(bm->weaponAbbr)->get();

                *temp += AString(" Wielders of this weapon will get ");

                if (bm->attackModifer != 0) {
                    *temp += AString((bm->attackModifer > 0) ? "bonus of ":"penalty of ") + abs(bm->attackModifer) + " on combat attack";
                }

                if (bm->defenseModifer != 0) {
                    if (bm->attackModifer != 0) {
                        *temp += AString(" and ");
                    }

                    *temp += AString((bm->defenseModifer > 0) ? "bonus of ":"penalty of ") + abs(bm->defenseModifer) + " on combat defense";
                }

                *temp += AString(" against ") + target.name + " [" + target.abr + "].";
            }
        }
    }

    if (ItemDefs[item].type & IT_ARMOR) {
        *temp += " This is a type of armor.";
        auto armor = FindArmor(ItemDefs[item].abr)->get();
        *temp += " This armor will protect its wearer ";
        for (i = 0; i < NUM_WEAPON_CLASSES; i++) {
            if (i == NUM_WEAPON_CLASSES - 1) {
                *temp += ", and ";
            } else if (i > 0) {
                *temp += ", ";
            }
            int percent = static_cast<int>(
                std::round(static_cast<float>(armor.saves[i]) * 100.0f / static_cast<float>(armor.from))
            );
            *temp += AString(percent) + "% of the time versus " + WeapClass(i) + " attacks";
        }
        *temp += ".";
        if (full) {
            if (armor.flags & ArmorType::USEINASSASSINATE) {
                *temp += " This armor may be worn during assassination "
                    "attempts.";
            }
        }
    }

    if (ItemDefs[item].type & IT_TOOL) {
        int comma = 0;
        int last = -1;
        *temp += " This is a tool.";
        *temp += " This item increases the production of ";
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
                        if (comma > 1) *temp += ",";
                        *temp += " and ";
                    } else {
                        *temp += ", ";
                    }
                }
                comma++;
                if (i == I_SILVER) {
                    *temp += "entertainment";
                } else {
                    *temp += ItemString(i, 1);
                }
                *temp += AString(" by ") + ItemDefs[i].mult_val;
            }
        }
        *temp += ".";
    }

    if (ItemDefs[item].type & IT_TRADE) {
        *temp += " This is a trade good.";
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
                *temp += AString(" This item can be bought for between ") +
                    minbuy + " and " + maxbuy + " silver.";
                *temp += AString(" This item can be sold for between ") +
                    minsell+ " and " + maxsell+ " silver.";
            } else {
                *temp += AString(" This item can be bought and sold for ") +
                    ItemDefs[item].baseprice + " silver.";
            }
        }
    }

    if (ItemDefs[item].type & IT_MOUNT) {
        *temp += " This is a mount.";
        auto mount = FindMount(ItemDefs[item].abr).value().get();
        if (mount.skill == NULL) {
            *temp += " No skill is required to use this mount.";
        } else {
            auto pS = FindSkill(mount.skill);
            if (!pS || (pS->get().flags & SkillType::DISABLED))
                *temp += " This mount is unrideable.";
            else {
                *temp += AString(" This mount requires ") +
                    SkillStrs(pS->get()) + " of at least level " + mount.minBonus +
                    " to ride in combat.";
            }
        }
        *temp += AString(" This mount gives a minimum bonus of +");
        if (Globals->HALF_RIDING_BONUS) {
            *temp += AString(((mount.minBonus + 1) / 2)) + " when ridden into combat.";
        } else {
            *temp +=  AString(mount.minBonus) + " when ridden into combat.";
        }

        *temp += AString(" This mount gives a maximum bonus of +");
        if (Globals->HALF_RIDING_BONUS) {
            *temp += AString(((mount.maxBonus + 1) / 2)) + " when ridden into combat.";
            *temp += AString(" This bonus is calculated as a RIDING skill divided by 2 (rounded up).");
        } else {
            *temp +=  AString(mount.maxBonus) + " when ridden into combat.";
        }

        if (full) {
            if (ItemDefs[item].fly) {
                *temp += AString(" This mount gives a maximum bonus of +");
                if (Globals->HALF_RIDING_BONUS) {
                    *temp += AString(((mount.maxHamperedBonus + 1) / 2)) + " when ridden into combat in ";
                } else {
                    *temp += AString(mount.maxHamperedBonus) + " when ridden into combat in ";
                }
                *temp += "terrain which allows ridden mounts but not flying mounts.";
            }
            if (mount.mountSpecial != NULL) {
                *temp += AString(" When ridden, this mount causes ") +
                    ShowSpecial(mount.mountSpecial, mount.specialLev, 1, 0);
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
                *temp += " This item is a trade resource.";
        }
    }

    if (ItemDefs[item].type & IT_MONEY) {
        *temp += " This is the currency of ";
        *temp += Globals->WORLD_NAME;
        *temp += ".";
    }

    if (!full)
        return temp;

    auto attribtype = FindAttrib("observation");
    if (attribtype) {
        auto ap = attribtype->get();
        for (i = 0; i < 5; i++)
            if (ap.mods[i].flags & AttribModItem::ITEM) {
                AString abbr = ItemDefs[item].abr;
                if (abbr == ap.mods[i].ident &&
                        ap.mods[i].modtype == AttribModItem::CONSTANT) {
                    *temp += " This item grants a ";
                    *temp += ap.mods[i].val;
                    *temp += " point bonus to a unit's observation skill";
                    if (ap.mods[i].flags & AttribModItem::PERMAN) {
                        *temp += " (note that a unit must possess one ";
                        *temp += abbr;
                        *temp += " for each man to gain this bonus)";
                    }
                    *temp += ".";
                }
            }
    }
    attribtype = FindAttrib("stealth");
    if (attribtype) {
        auto ap = attribtype->get();
        for (i = 0; i < 5; i++)
            if (ap.mods[i].flags & AttribModItem::ITEM) {
                AString abbr = ItemDefs[item].abr;
                if (abbr == ap.mods[i].ident &&
                        ap.mods[i].modtype == AttribModItem::CONSTANT) {
                    *temp += " This item grants a ";
                    *temp += ap.mods[i].val;
                    *temp += " point bonus to a unit's stealth skill";
                    if (ap.mods[i].flags & AttribModItem::PERMAN) {
                        *temp += " (note that a unit must possess one ";
                        *temp += abbr;
                        *temp += " for each man to gain this bonus)";
                    }
                    *temp += ".";
                }
            }
    }
    attribtype = FindAttrib("wind");
    if (attribtype) {
        auto ap = attribtype->get();
        for (i = 0; i < 5; i++)
            if (ap.mods[i].flags & AttribModItem::ITEM) {
                AString abbr = ItemDefs[item].abr;
                if (abbr == ap.mods[i].ident &&
                        ap.mods[i].modtype == AttribModItem::CONSTANT) {
                    if (Globals->FLEET_WIND_BOOST > 0) {
                        *temp += " The possessor of this item will add ";
                        *temp += Globals->FLEET_WIND_BOOST;
                        *temp += " movement points to ships requiring up to ";
                        *temp += ap.mods[i].val * 12;
                        *temp += " sailing skill points.";
                        *temp += " This bonus is not cumulative with a mage's summon wind skill.";
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
                *temp += " A Ring of Invisibility has one limitation; "
                    "a unit possessing a RING cannot assassinate, "
                    "nor steal from, a unit with an Amulet of True Seeing.";
            }
            break;
        case I_AMULETOFTS:
            if (!(ItemDefs[I_RINGOFI].flags & ItemType::DISABLED)) {
                *temp += " Also, a unit with an Amulet of True Seeing "
                    "cannot be assassinated by, nor have items "
                    "stolen by, a unit with a Ring of Invisibility "
                    "(note that the unit must have at least one "
                    "Amulet of True Seeing per man in order to repel "
                    "a unit with a Ring of Invisibility).";
            }
            break;
        case I_PORTAL:
            *temp += " This item is required for mages to use the Portal Lore skill.";
            break;
        case I_STAFFOFH:
            *temp += " This item allows its possessor to magically heal "
                "units after battle, as if their skill in Magical Healing "
                "was the highest of their manipulation, pattern, force and "
                "spirit skills, up to a maximum of level 2.";
            break;
        case I_RELICOFGRACE:
            *temp += " This is a token of sacrifice, token of power. The Faction must posess "
                    "60 of them to get a WISH power and win the game. This relics can not "
                    "be given and do not drop in spoils.";
            break;
        case I_HEALPOTION:
            *temp += " This item allows its possessor to heal wounded units after battle."
                " No skill is necessary to use this item; it will be used automatically"
                " when the possessor is involved in a battle. It can heal up to 1"
                " casualties, with a 70 percent success rate. Healing consumes an item.";
            break;
        default:
            break;
    }

    auto pS = FindSkill(ItemDefs[item].grantSkill);
    if (pS && pS->get().flags & SkillType::CAST) {
        *temp += " This item allows its possessor to CAST the ";
        *temp += pS->get().name;
        *temp += " spell as if their skill in ";
        *temp += pS->get().name;
        *temp += " was ";
        if (ItemDefs[item].minGrant < ItemDefs[item].maxGrant) {
            int count, found;
            count = 0;
            for (i = 0; i < 4; i++) {
                pS = FindSkill(ItemDefs[item].fromSkills[i]);
                if (pS && !(pS->get().flags & SkillType::DISABLED))
                    count++;
            }
            if (count > 1)
                *temp += "the highest of ";
            else
                *temp += "that of ";
            *temp += "their ";
            found = 0;
            for (i = 0; i < 4; i++) {
                pS = FindSkill(ItemDefs[item].fromSkills[i]);
                if (!pS || (pS->get().flags & SkillType::DISABLED))
                    continue;
                if (found > 0) {
                    if ((found + 1) == count)
                        *temp += " and ";
                    else
                        *temp += ", ";
                }
                *temp += pS->get().name;
                found++;
            }
            *temp += " skill";
            if (count > 1)
                *temp += "s";
            *temp += ", up to a maximum of";
        }
        *temp += " level ";
        *temp += ItemDefs[item].maxGrant;
        *temp += ".";
        if (ItemDefs[item].minGrant > 1 &&
                ItemDefs[item].minGrant < ItemDefs[item].maxGrant) {
            *temp += " A skill level of at least ";
            *temp += ItemDefs[item].minGrant;
            *temp += " will always be granted.";
        }
    }

    if (ItemDefs[item].type & IT_BATTLE) {
        *temp += " This item is a miscellaneous combat item.";
        auto bt = FindBattleItem(ItemDefs[item].abr);
        if (bt) {
            if (bt->get().flags & BattleItemType::MAGEONLY) {
                *temp += " This item may only be used by a mage";
                if (Globals->APPRENTICES_EXIST) {
                    *temp += " or an ";
                    *temp += Globals->APPRENTICE_NAME;
                }
                *temp += ".";
            }
            if (bt->get().flags & BattleItemType::SHIELD)
                *temp += AString(" ") + "This item provides ";
            else
                *temp += AString(" ") + "This item can cast ";
            *temp += ShowSpecial(bt->get().special, bt->get().skillLevel, 1, bt->get().flags & BattleItemType::SHIELD);
        }
    } else if (ItemDefs[item].type & IT_MAGEONLY) {
        *temp += " This item may only be used by a mage";
        if (Globals->APPRENTICES_EXIST) {
            *temp += " or an ";
            *temp += Globals->APPRENTICE_NAME;
        }
        *temp += ".";
    }
    if (ItemDefs[item].type & IT_FOOD) {
        *temp += " This item can be eaten to provide ";
        *temp += Globals->UPKEEP_FOOD_VALUE;
        *temp += " silver towards a unit's maintenance cost.";
    }
    if (ItemDefs[item].flags & ItemType::CANTGIVE) {
        *temp += " This item cannot be given to other units.";
    }

    if (ItemDefs[item].max_inventory) {
        *temp += AString(" A unit may have at most ") +
            ItemString(item, ItemDefs[item].max_inventory, FULLNUM) + ".";
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

AString Item::Report(int seeillusions)
{
    AString ret = "";
    // special handling of the unfinished ship items
    if (ItemDefs[type].type & IT_SHIP) {
        ret += AString("unfinished ") + ItemDefs[type].name +
            " [" + ItemDefs[type].abr + "] (needs " + num + ")";
    } else ret += ItemString(type,num);
    if (seeillusions && (ItemDefs[type].type & IT_ILLUSION)) {
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

AString ItemList::Report(int obs,int seeillusions,int nofirstcomma)
{
    UncheckAll();
    AString temp;
    for (int s = 0; s < 7; s++) {
        temp += ReportByType(s, obs, seeillusions, nofirstcomma);
        if (temp.Len()) nofirstcomma = 0;
    }
    return temp;
}

AString ItemList::BattleReport()
{
    AString temp;
    for(auto i : items) {
        if (ItemDefs[i->type].combat) {
            temp += ", ";
            temp += i->Report(0);
            if (ItemDefs[i->type].type & IT_MONSTER) {
                auto monster = FindMonster(ItemDefs[i->type].abr, (ItemDefs[i->type].type & IT_ILLUSION))->get();
                temp += AString(" (Combat ") + monster.attackLevel +
                    "/" + monster.defense[ATTACK_COMBAT] + ", Attacks " +
                    monster.numAttacks + ", Hits " + monster.hits +
                    ", Tactics " + monster.tactics + ")";
            }
        }
    }
    return temp;
}

AString ItemList::ReportByType(int type, int obs, int seeillusions,
        int nofirstcomma)
{
    AString temp;
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
                temp += i->Report(seeillusions);
            } else {
                if (ItemDefs[i->type].weight) {
                    if (nofirstcomma) nofirstcomma = 0;
                    else temp += ", ";
                    temp += i->Report(seeillusions);
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
    for (unsigned int i=0; i<(sizeof(skills)/sizeof(skills[0])); i++) {
        if (skills[i] == NULL) continue;
        if (ItemDefs[item].pSkill == skills[i]) return true;
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
        auto armor = FindArmor(ItemDefs[item].abr)->get();
        int p = armor.from / armor.saves[3];
        if (p > 4) {
            // puny armor not used by combative races
            bool mayWearArmor = true;
            for (unsigned int i=0; i<(sizeof(skills)/sizeof(skills[0])); i++) {
                if (skills[i] == NULL) continue;
                auto pS = FindSkill(skills[i]);
                if (!pS) continue;
                if (pS->get() == FindSkill("COMB")->get()) mayWearArmor = false;
            }
            if (mayWearArmor) return 1;
            return 0;
        }
        if (p > 3) return 1;
        // heavy armor not be worn by sailors and sneaky races
        bool mayWearArmor = true;
        for (unsigned int i=0; i<(sizeof(skills)/sizeof(skills[0])); i++) {
            if (skills[i] == NULL) continue;
            auto pS = FindSkill(skills[i]);
            if (!pS) continue;
            if (
                (pS->get() == FindSkill("SAIL")->get()) || (pS->get() == FindSkill("HUNT")->get()) ||
                (pS->get() == FindSkill("STEA")->get()) || (pS->get() == FindSkill("LBOW")->get())
            ) mayWearArmor = false;
        }
        if (mayWearArmor) return true;
        return false;
    }

    return false;
}
