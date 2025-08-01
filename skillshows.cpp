#include "skills.h"
#include "items.h"
#include "object.h"
#include "gamedata.h"

static const std::string describe_escape_parameters(int item)
{
    if (item < 0 || item >= NITEMS || !ItemDefs[item].escape) return "";

    std::string desc;
    if (!(ItemDefs[item].escape & ItemType::LOSS_CHANCE) && !(ItemDefs[item].escape & ItemType::HAS_SKILL)) {
        desc += "Each " + ItemDefs[item].name + " has a chance to escape equal to " +
            (
                (ItemDefs[item].escape & ItemType::ESC_NUM_SQUARE)
                ? "the total number of " + ItemDefs[item].names + " under the mage's control"
                : "1"
            ) + " in " + (ItemDefs[item].esc_val > 1 ? std::to_string(ItemDefs[item].esc_val) + " times ": "")
            + "the mage's skill level";

        desc += (
            (ItemDefs[item].escape & ItemType::ESC_LEV_SQUARE) ? " squared" :
            (ItemDefs[item].escape & ItemType::ESC_LEV_CUBE) ? " cubed" :
            (ItemDefs[item].escape & ItemType::ESC_LEV_QUAD) ? " to the fourth power" :
            "" // None of the above apply
        );
    }

    if (ItemDefs[item].escape & ItemType::LOSE_LINKED) {
        desc += ". If any one does escape, its first action will be to release its companions";
    }
    desc += ". ";

    return desc;
}

const std::string ShowSkill::Report(Faction *f) const
{
    if (SkillDefs[skill].flags & SkillType::DISABLED) return "";

    std::string str;
    std::optional<std::reference_wrapper<RangeType>> range;
    int max;

    // Here we pick apart the skill
    switch (skill) {
        case S_FARMING:
            if (level > 1) break;
            str += "This skill deals with all aspects of grain production.";
            break;
        case S_RANCHING:
            if (level > 1) break;
            str += "This skill deals with all aspects of livestock production.";
            break;
        case S_MINING:
            if (level > 1) break;
            if (!(ItemDefs[I_IRON].flags & ItemType::DISABLED) ||
                !(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) ||
                !(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED) ||
                !(ItemDefs[I_GEMS].flags & ItemType::DISABLED)) {
                str += "This skill deals with all aspects of extracting raw ";
                if (!(ItemDefs[I_IRON].flags & ItemType::DISABLED) ||
                    !(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) ||
                    !(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED)) {
                    str += "metals";
                    if (!(ItemDefs[I_GEMS].flags & ItemType::DISABLED)) {
                        str += " and gems";
                    }
                } else {
                    str += "gems";
                }
                str += " from the earth. They tend to be found more often "
                    "in mountainous regions, but may be found "
                    "elsewhere as well.";
            } else {
                str += "The mining skill is overrated.";
            }
            break;
        case S_LUMBERJACK:
            if (level > 1) break;
            str += "This skill deals with all aspects of various wood "
                "production. Wood is most often found in forests, but "
                "may also be found elsewhere.";
            break;
        case S_QUARTERMASTER:
            if (level > 1) break;
            if (!(Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT))
                break;
            str += "This skill deals with transporting and "
                "distributing goods between non-local units "
                "and transport structures.";
            if (Globals->SHIPPING_COST > 0) {
                str += " The cost of shipping one weight unit from one "
                    "transport structure to another transport structure is ";
                if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST) str += "4-((level+1)/2) * ";
                str += std::to_string(Globals->SHIPPING_COST) + " silver.";
                if (Globals->FRACTIONAL_WEIGHT) {
                    str += " Items with a normal weight of 0 are "
                        "treated as if ";
                    str += std::to_string(Globals->FRACTIONAL_WEIGHT);
                    str += " of the item in question weigh one weight unit.";
                }
            }

            if (Globals->NONLOCAL_TRANSPORT > 0) {
                str += " Items may be shipped between two transport "
                    "structures which are up to ";
                str += std::to_string(Globals->NONLOCAL_TRANSPORT);
                if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST)
                    str += " plus (level+1)/3 ";
                str += (Globals->NONLOCAL_TRANSPORT != 1) ? "hexes" : "hex";
                str += " distant from each other.";
            } else if (Globals->NONLOCAL_TRANSPORT == 0) {
                str += " Items may be instantaneously "
                    "shipped between any two transport "
                    "structures.";
            }
            if (Globals->LOCAL_TRANSPORT > 0) {
                str += " Items may be distributed from a transport "
                    "structure to any unit or transported to a transport "
                    "structure by any unit located within ";
                str += std::to_string(Globals->LOCAL_TRANSPORT);
                str += (Globals->LOCAL_TRANSPORT != 1) ? " hexes" : " hex";
                str += " of the transport structure.";
            }
            break;
        case S_QUARRYING:
            if (level > 1) break;
            str += "This skill deals with all aspects of various stone "
                "production. Mountains are the main producers of stone, but "
                "it may be found in other regions as well.";
            break;
        case S_HUNTING:
            if (level > 1) break;
            str += "This skill deals with all aspects of animal hide "
                "production.";
            break;
        case S_FISHING:
            if (level > 1) break;
            str += "This skill deals with all aspects of fish production.";
            break;
        case S_HERBLORE:
            if (level > 1) break;
            str += "This skill deals with all aspects of herb production.";
            break;
        case S_HORSETRAINING:
            if (level > 1) break;
            str += "This skill deals with all aspects of horse production.";
            break;
        case S_WEAPONSMITH:
            if (level > 1) break;
            str += "This skill deals with all aspects of weapon "
                "construction and production.";
            break;
        case S_ARMORER:
            if (level > 1) break;
            str += "This skill deals with all aspects of armor construction "
                "and production.";
            break;
        case S_CARPENTER:
            if (level > 1) break;
            str += "This skill deals with all aspects of wood based item "
                "production other than for use as weapons.";
            break;
        case S_BUILDING:
            if (level > 1) break;
            str += "This skill deals with the construction of "
                "fortifications, roads and other buildings, except for "
                "most trade structures.";
            break;
        case S_SHIPBUILDING:
            if (level > 1) break;
            str += "This skill deals with the constructions of all types "
                "of ships.";
            break;
        case S_ENTERTAINMENT:
            if (level > 1) break;
            str += "A unit with this skill may use the ENTERTAIN order "
                "to generate funds. The amount of silver gained will "
                "be ";
            str += std::to_string(Globals->ENTERTAIN_INCOME);
            str +=  " per man, times the level of the entertainers. "
                "This amount is limited by the region that the unit is in.";
            break;
        case S_TACTICS:
            if (level > 1) break;
            if (Globals->ADVANCED_TACTICS) {
                str += "Tactics allows the unit, and all allies, to gain a "
                    "bonus to their attack and defense during first battle "
                    "round. Bonus equals to the difference in skills but can be "
                    "+3 at most. The army with the highest level tactician "
                    "in a battle will receive this bonus; if the highest "
                    "levels are equal, no bonus is awarded.";
            } else {
                str += "Tactics allows the unit, and all allies, to gain a "
                    "free round of attacks during battle. The army with the "
                    "highest level tactician in a battle will receive this free "
                    "round; if the highest levels are equal, no free round is "
                    "awarded. Only one free round total will be awarded for any "
                    "reason.";
            }
            break;
        case S_COMBAT:
            if (level > 1) break;
            str += "This skill gives the unit a bonus in hand to hand "
                "combat. Also, a unit with this skill may TAX or PILLAGE.";
            break;
        case S_ENDURANCE:
            if (level == 1) {
                str += "A unit with this skill has begun the process of building "
                    "on their combat experience to learn how to survive wounds "
                    "that would lay low a less grizzled warrior. This is an "
                    "arduous process, and doesn't yet provide any advantage "
                    "at this skill level.";
            } else if (level == 3) {
                str += "The process of building up combat endurance is starting "
                    "to yield results. At this level the men in the unit can "
                    "survive one extra hit in combat before being overcome.";
            } else if (level == 5) {
                str += "The men of this unit are now hardened veterans, and can "
                    "survive three hits in combat before falling.";
            }
            break;
        case S_RIDING:
            if (level > 1) break;
            str += "A unit with this skill, if possessing a mount, may "
                "gain a bonus in combat, if the battle is in a location "
                "where that mount may be utilized and if the skill of the "
                "rider is sufficient to control that mount. The bonus "
                "gained can vary with the mount, the riders skill, and the "
                "terrain.";
            break;
        case S_CROSSBOW:
            if (level > 1) break;
            str += "A unit with this skill may use a crossbow or other bow "
                "derived from one, either in battle, or to TAX or PILLAGE a "
                "region.";
            break;
        case S_LONGBOW:
            if (level > 1) break;
            str += "A unit with this skill may use a longbow or other bow "
                "derived from one, either in battle, or to TAX or PILLAGE a "
                "region.";
            break;
        case S_STEALTH:
            if (level > 1) break;
            str += "A unit with this skill is concealed from being seen";
            if (!(SkillDefs[S_OBSERVATION].flags & SkillType::DISABLED)) {
                str += ", except by units with an Observation skill greater than or equal to the stealthy unit's Stealth level";
            }
            str += ".";
            break;
        case S_OBSERVATION:
            if (level > 1) break;
            str += "A unit with this skill can see stealthy units or "
                "monsters whose stealth rating is less than or equal to "
                "the observing unit's Observation level. The unit can "
                "also determine the faction owning a unit, provided its "
                "Observation level is higher than the other unit's Stealth "
                "level.";
            break;
        case S_HEALING:
            str += "A unit with this skill is able to use herbs [HERB] to "
                "heal units hurt in battle.";
            if (HealDefs[level].num != HealDefs[level - 1].num ||
                    HealDefs[level].rate != HealDefs[level - 1].rate) {
                str += " A unit at this level of skill can ";
                str += "bring soldiers back from near death, healing";
                str += " up to ";
                str += std::to_string(HealDefs[level].num * Globals->HEALS_PER_MAN);
                str += " casualties, with a ";
                str += std::to_string(HealDefs[level].rate);
                str += " percent success rate.";
            }
            break;
        case S_SAILING:
            if (level > 1) break;
            str += "A unit with this skill may use the SAIL order to sail "
                "ships.";
            break;
        case S_FORCE:
            if (level > 1) break;
            str += "The Force skill is not directly useful to a mage, but "
                "is rather one of the Foundation skills on which other "
                "magical skills are based. The Force skill determines the "
                "power of the magical energy that a mage is able to use. "
                "Note that a Force skill level of 0 does not indicate that "
                "a mage cannot use magical energy, but rather can only "
                "perform magical acts that do not require great amounts of "
                "power.";
            break;
        case S_PATTERN:
            if (level > 1) break;
            str += "The Pattern skill is not directly useful to a mage, but "
                "is rather one of the Foundation skills on which other "
                "magical skills are based. A mage's Pattern skill indicates "
                "the ability to handle complex magical patterns, and is "
                "important for complicated tasks such as healing and "
                "controlling nature.";
            break;
        case S_SPIRIT:
            if (level > 1) break;
            str += "The Spirit skill is not directly useful to a mage, but "
                "is rather one of the Foundation skills on which other "
                "magical skills are based. Spirit skill indicates the mage's "
                "ability to control and affect magic and other powers beyond "
                "the material world.";
            break;
        case S_FIRE:
            if (level > 1) break;
            break;
        case S_EARTHQUAKE:
            if (level > 1) break;
            break;
        case S_FORCE_SHIELD:
            if (level > 1) break;
            break;
        case S_ENERGY_SHIELD:
            if (level > 1) break;
            break;
        case S_SPIRIT_SHIELD:
            if (level > 1) break;
            break;
        case S_MAGICAL_HEALING:
            if (level == 1) {
                str += "This skill enables the mage to magically heal units "
                    "after battle. No order is necessary to use this spell; "
                    "it will be used automatically when the mage is involved "
                    "in a battle. ";
            }
            if (MagicHealDefs[level].num != MagicHealDefs[level - 1].num ||
                    MagicHealDefs[level].rate != MagicHealDefs[level - 1].rate) {
                str += "A mage at this level of skill can ";
                if (level > 4) {
                    str += "bring soldiers back from near death, healing";
                } else if (level > 2) {
                    str += "work wonders of healing with his new-found powers; he may heal";
                } else {
                    str += "heal";
                }
                str += " up to ";
                str += std::to_string(MagicHealDefs[level].num);
                str += " casualties, with a ";
                str += std::to_string(MagicHealDefs[level].rate);
                str += " percent success rate.";
            }
            break;
        case S_GATE_LORE:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "Gate Lore is the art of detecting and using magical "
                    "Gates, which are spread through the world. The Gates are ";
                if (!Globals->DISPERSE_GATE_NUMBERS)
                    str += "numbered in order, but ";
                str += "spread out randomly, so there is "
                    "no correlation between the Gate number and the Gate's "
                    "location. A mage with skill 1 in Gate Lore can see a "
                    "Gate if one exists in the same region as the mage. This "
                    "detection is automatic; the Gate will appear in the "
                    "region report. A mage with skill 1 in Gate Lore may "
                    "also jump through a Gate into another region on the same "
                    "level containing a gate, selected at random. To use "
                    "Gate Lore in this manner, use the syntax CAST Gate_Lore "
                    "RANDOM UNITS <unit> ... UNITS is followed by a list "
                    "of units to follow the mage through the Gate (the mage "
                    "always jumps through the Gate). At level 1, the mage "
                    "may carry 15 weight units through the Gate (including "
                    "the weight of the mage).";
            } else if (level == 2) {
                str += "A mage with Gate Lore skill 2 can detect Gates in "
                    "adjacent regions. The mage should use the syntax CAST "
                    "Gate_Lore DETECT in order to detect these nearby Gates. "
                    "Also at level 2 Gate Lore, the mage may perform a "
                    "random gate jump without being restricted to the same "
                    "level; use CAST Gate_Lore RANDOM LEVEL UNITS <unit> ... "
                    "to use this option; when calculating weight for multi "
                    "level jump, skill level is reduced by 1. The mage may "
                    "also now carry 500 weight units through a Gate when doing "
                    "a random jump. "
                    "A mage with Gate Lore skill 2 and higher can step "
                    "through a Gate into another region containing a specific "
                    "Gate. To use this spell, use the syntax CAST Gate_Lore "
                    "GATE <number> UNITS <unit> ... <number> specifies the "
                    "Gate that the mage will jump to. UNITS is followed by a "
                    "list of units to follow the mage through the gate (the "
                    "mage always jumps through the gate). At level 2, the "
                    "mage may carry 15 weight units through the specific Gate "
                    "(including the mage).";
            } else if (level == 3) {
                str += "A mage with Gate Lore skill 3 may carry 500 weight "
                    "units through a Gate. Also, a level 3 mage doing a random "
                    "gate jump may carry 1500 weight units through the Gate.";
            } else if (level == 4) {
                str += "A mage with Gate Lore skill 4 may carry 1500 weight "
                    "units through a Gate. Also, a level 4 mage doing a random "
                    "gate jump may carry 3000 weight units through the Gate.";
            } else if (level == 5) {
                str += "A mage with Gate Lore skill 5 may carry 3000 weight "
                    "units through a Gate. Also, a level 5 mage doing a random "
                    "gate jump may carry 6000 weight units through the Gate.";
            }
            break;
        case S_PORTAL_LORE:
            if (level > 1) break;
            /* XXX -- This should be cleaner somehow. */
            if (ItemDefs[I_PORTAL].flags & ItemType::DISABLED) break;
            str += "A mage with the Portal Lore skill may, with the aid of "
                "another mage";
            if (Globals->APPRENTICES_EXIST) {
                str += " or ";
                str += Globals->APPRENTICE_NAME;
            }
            str += ", make a temporary Gate between two regions, and "
                "send units from one region to another. In order to do this, "
                "both mages (the caster, and the target mage) must have "
                "Portals, and the caster must be trained in Portal Lore. The "
                "caster may teleport units weighing up to 800 weight units "
                "times his skill level, to the target mage's region. ";
            range = find_range(SkillDefs[skill].range.value_or(""));
            if (range) {
                str += " The target region must be within ";
                str += std::to_string(range->get().rangeMult);
                switch(range->get().rangeClass) {
                    case RangeType::RNG_LEVEL:
                        str += " times the caster's skill level ";
                        break;
                    case RangeType::RNG_LEVEL2:
                        str += " times the caster's skill level squared ";
                        break;
                    case RangeType::RNG_LEVEL3:
                        str += " times the caster's skill level cubed ";
                        break;
                    default:
                    case RangeType::RNG_ABSOLUTE:
                        break;
                }
                str += "regions of the caster. ";
            }
            str += "To use this skill, CAST Portal_Lore <target> UNITS "
                "<unit> ..., where <target> is the unit number of the "
                "target mage, and <unit> is a list of units to be "
                "teleported (the casting mage may teleport himself, if "
                "he so desires).";
            break;
        case S_FARSIGHT:
            if (level > 1) break;
            str += "A mage with this skill may obtain region reports on "
                "distant regions. The report will be as if the mage was in "
                "the distant region himself.";
            range = find_range(SkillDefs[skill].range.value_or(""));
            if (range) {
                if (range->get().flags & RangeType::RNG_SURFACE_ONLY) {
                    str += " This skill only works on the surface of the "
                        "world.";
                }
                str += " The target region must be within ";
                str += std::to_string(range->get().rangeMult);
                switch(range->get().rangeClass) {
                    case RangeType::RNG_LEVEL:
                        str += " times the caster's skill level ";
                        break;
                    case RangeType::RNG_LEVEL2:
                        str += " times the caster's skill level squared ";
                        break;
                    case RangeType::RNG_LEVEL3:
                        str += " times the caster's skill level cubed ";
                        break;
                    default:
                    case RangeType::RNG_ABSOLUTE:
                        break;
                }
                str += "regions of the caster. ";
                if (range->get().flags & RangeType::RNG_CROSS_LEVELS) {
                    str += "Coordinates of locations not on the surface are "
                        "scaled to the surface coordinates for this "
                        "calculation. Attempting to view across different "
                        "levels increases the distance by ";
                    str += std::to_string(range->get().crossLevelPenalty);
                    str += " per level difference. ";
                    str += "To use this skill, CAST Farsight REGION <x> <y> "
                        "<z> where <x>, <y>, and <z> are the coordinates of "
                        "the region that you wish to view. If you omit the "
                        "<z> coordinate, the <z> coordinate of the caster's "
                        "current region will be used.";
                    if (Globals->UNDERWORLD_LEVELS +
                            Globals->UNDERDEEP_LEVELS == 1) {
                        str += " The <z> coordinate for the surface is '1' "
                            "and the <z>-coordinate for the underworld is "
                            "'2'.";
                    }
                    str += " Note that Farsight cannot be used either into "
                        "or out of the Nexus.";
                } else {
                    str += "To use this skill, CAST Farsight REGION <x> "
                        "<y>, where <x> and <y> are the coordinates of the "
                        "region that you wish to view.";
                }
            }
            if (Globals->IMPROVED_FARSIGHT) {
                str += " Any other skills which the mage has which give "
                    "region information will be used when farsight is used.";
            } else {
                str += " Note that Farsight does not work in conjunction "
                    "with other skills or spells; the mage can only rely on "
                    "his normal facilities while casting Farsight.";
            }
            break;
        case S_MIND_READING:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "A mage with Mind Reading skill 1 may cast the spell "
                    "and determine the faction affiliation of any unit he can "
                    "see. To use the spell in this manner, CAST Mind_Reading "
                    "<unit>, where <unit> is the target unit.";
            } else if (level == 2) {
                str += "A mage with Mind Reading skill 2 will automatically "
                    "determine the faction affiliation of any unit he can "
                    "see. Usage of this skill is automatic, and no order is "
                    "needed to use it.";
            } else if (level == 3) {
                str += "A mage with Mind Reading skill 3 can get a full "
                    "unit report on any unit he can see. To use this skill, "
                    "CAST Mind_Reading <unit> where <unit> is the target "
                    "unit.";
            }
            break;
        case S_TELEPORTATION:
            if (level > 1) break;
            /* XXX -- This should be cleaner somehow. */
            str += "A mage with this skill may teleport himself across "
                "great distances, even without the use of a gate. The mage "
                "may teleport up to 50 weight units per skill level.";
            range = find_range(SkillDefs[skill].range.value_or(""));
            if (range) {
                if (range->get().flags & RangeType::RNG_SURFACE_ONLY) {
                    str += " This skill only works on the surface of the "
                        "world.";
                }
                str += " The target region must be within ";
                str += std::to_string(range->get().rangeMult);
                switch(range->get().rangeClass) {
                    case RangeType::RNG_LEVEL:
                        str += " times the caster's skill level ";
                        break;
                    case RangeType::RNG_LEVEL2:
                        str += " times the caster's skill level squared ";
                        break;
                    case RangeType::RNG_LEVEL3:
                        str += " times the caster's skill level cubed ";
                        break;
                    default:
                    case RangeType::RNG_ABSOLUTE:
                        break;
                }
                str += "regions of the caster. ";
                if (range->get().flags & RangeType::RNG_CROSS_LEVELS) {
                    str += "Coordinates of locations not on the surface are "
                        "scaled to the surface coordinates for this "
                        "calculation. Attempting to view across different "
                        "levels increases the distance by ";
                    str += std::to_string(range->get().crossLevelPenalty);
                    str += " per level difference. ";
                    str += "To use this skill, CAST Teleportation REGION "
                        "<x> <y> <z> where <x>, <y>, and <z> are the "
                        "coordinates of the region that you wish to "
                        "teleport to. If you omit the <z> coordinate, the "
                        "<z> coordinate of the caster's current region will "
                        "be used.";
                    if (Globals->UNDERWORLD_LEVELS +
                            Globals->UNDERDEEP_LEVELS == 1) {
                        str += " The <z> coordinate for the surface is '1' "
                            "and the <z>-coordinate for the underworld is "
                            "'2'.";
                    }
                    str += " Note that Teleportation cannot be used either "
                        "into or out of the Nexus.";
                } else {
                    str += "To use this skill, CAST Teleportation REGION "
                        "<x> <y>, where <x> and <y> are the coordinates of "
                        "the region that you wish to teleport to.";
                }
            }
            break;
        case S_WEATHER_LORE:
            if (level > 1) break;
            /* XXX -- This should be templated */
            str += "Weather Lore is the magic of the weather; a mage with "
                "this skill can predict the weather in nearby regions. "
                "Weather Lore also allows further study into more powerful "
                "areas of magic. The weather may be predicted for 3 months "
                "at level 1, 6 months at level 3 and a full year at level "
                "5.";
            range = find_range(SkillDefs[skill].range.value_or(""));
            if (range) {
                if (range->get().flags & RangeType::RNG_SURFACE_ONLY) {
                    str += " This skill only works on the surface of the "
                        "world.";
                }
                str += " The target region must be within ";
                str += std::to_string(range->get().rangeMult);
                switch(range->get().rangeClass) {
                    case RangeType::RNG_LEVEL:
                        str += " times the caster's skill level ";
                        break;
                    case RangeType::RNG_LEVEL2:
                        str += " times the caster's skill level squared ";
                        break;
                    case RangeType::RNG_LEVEL3:
                        str += " times the caster's skill level cubed ";
                        break;
                    default:
                    case RangeType::RNG_ABSOLUTE:
                        break;
                }
                str += "regions of the caster. ";
                if (range->get().flags & RangeType::RNG_CROSS_LEVELS) {
                    str += "Coordinates of locations not on the surface are "
                        "scaled to the surface coordinates for this "
                        "calculation. Attempting to view across different "
                        "levels increases the distance by ";
                    str += std::to_string(range->get().crossLevelPenalty);
                    str += " per level difference. ";
                    str += "To use this skill, CAST Weather_Lore REGION "
                        "<x> <y> <z> where <x>, <y>, and <z> are the "
                        "coordinates of the region where you wish to "
                        "predict the weather. If you omit the <z> "
                        "coordinate, the <z> coordinate of the caster's "
                        "current region will be used.";
                    if (Globals->UNDERWORLD_LEVELS +
                            Globals->UNDERDEEP_LEVELS == 1) {
                        str += " The <z> coordinate for the surface is '1' "
                            "and the <z>-coordinate for the underworld is "
                            "'2'.";
                    }
                    str += " Note that Weather Lore cannot be used either "
                        "into or out of the Nexus.";
                } else {
                    str += "To use this skill, CAST Weather_Lore REGION "
                        "<x> <y>, where <x> and <y> are the coordinates of "
                        "the region where you wish to predict the weather.";
                }
            }
            str += " A mage with Weather Lore skill will perceive the use "
                "of Weather Lore by any other mage in the same region.";
            break;
        case S_SUMMON_WIND:
            if (level == 1) {
                str += "A mage with knowledge of Summon Wind can summon "
                    "up the powers of the wind to aid him in sea or "
                    "air travel. Usage of this spell is automatic. ";
                if (Globals->FLEET_WIND_BOOST > 0) {
                    str += "A mage with this skill will add ";
                    str += std::to_string(Globals->FLEET_WIND_BOOST);
                    str += " movement points to ships requiring up to "
                        "12 sailing skill points per skill level "
                        "of Summon Wind. ";
                }

                str += "The effects of all such mages in a fleet are cumulative. ";
            }
            break;
        case S_SUMMON_STORM:
            if (level > 1) break;
            break;
        case S_SUMMON_TORNADO:
            if (level > 1) break;
            break;
        case S_CALL_LIGHTNING:
            if (level > 1) break;
            break;
        case S_CLEAR_SKIES:
            /* XXX -- this range stuff needs cleaning up */
            if (level > 1) break;
            if (SkillDefs[skill].flags & SkillType::CAST) {
                str += "When cast using the CAST order, it causes the "
                    "region to have good weather for the entire month; "
                    "movement is at the normal rate (even if it is winter) "
                    "and the economic production of the region is improved "
                    "for a month (this improvement of the economy will "
                    "actually take effect during the turn after the spell "
                    "is cast).";
                range = find_range(SkillDefs[skill].range.value_or(""));
                if (range) {
                    if (range->get().flags & RangeType::RNG_SURFACE_ONLY) {
                        str += " This skill only works on the surface of "
                            "the world.";
                    }
                    str += " The target region must be within ";
                    str += std::to_string(range->get().rangeMult);
                    switch(range->get().rangeClass) {
                        case RangeType::RNG_LEVEL:
                            str += " times the caster's skill level ";
                            break;
                        case RangeType::RNG_LEVEL2:
                            str += " times the caster's skill level squared ";
                            break;
                        case RangeType::RNG_LEVEL3:
                            str += " times the caster's skill level cubed ";
                            break;
                        default:
                        case RangeType::RNG_ABSOLUTE:
                            break;
                    }
                    str += "regions of the caster. ";
                    if (range->get().flags & RangeType::RNG_CROSS_LEVELS) {
                        str += "Coordinates of locations not on the surface "
                            "are scaled to the surface coordinates for this "
                            "calculation. Attempting to view across "
                            "different levels increases the distance by ";
                        str += std::to_string(range->get().crossLevelPenalty);
                        str += " per level difference. ";
                        str += "To use this skill, CAST Clear_Skies REGION "
                            "<x> <y> <z> where <x>, <y>, and <z> are the "
                            "coordinates of the region where you wish to "
                            "improve the weather. If you omit the <z> "
                            "coordinate, the <z> coordinate of the caster's "
                            "current region will be used.";
                        if (Globals->UNDERWORLD_LEVELS +
                                Globals->UNDERDEEP_LEVELS == 1) {
                            str += " The <z> coordinate for the surface is "
                                "'1' and the <z>-coordinate for the "
                                "underworld is '2'.";
                        }
                        str += " Note that Clear Skies cannot be used "
                            "either into or out of the Nexus.";
                    } else {
                        str += "To use this skill, CAST Clear_Skies REGION "
                            "<x> <y>, where <x> and <y> are the coordinates "
                            "of the region where you wish to improve the "
                            "weather.";
                    }
                } else {
                    str += " To use the spell in this fashion, CAST "
                        "Clear_Skies; no arguments are necessary.";
                }
            }
            break;
        case S_EARTH_LORE:
            if (level > 1) break;
            str += "Earth Lore is the study of nature, plants, and animals. "
                "A mage with knowledge of Earth Lore can use his knowledge "
                "of nature to aid local farmers, raising money for himself, "
                "and aiding the production of grain or livestock in the "
                "region. To use the spell, CAST Earth_Lore; the mage will "
                "receive an amount of money based on his level, and the "
                "economy of the region. Also, a mage with knowledge of Earth "
                "Lore will detect the use of Earth Lore by any other mage in "
                "the same region.";
            break;
        case S_WOLF_LORE:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_WOLF].flags & ItemType::DISABLED) break;
            str += "A mage with Wolf Lore skill may summon wolves, who will "
                "fight for him in combat. A mage may summon a number of "
                "wolves averaging ";
            str += std::to_string(ItemDefs[I_WOLF].mOut);
            str += " percent times his skill level, and control a total number "
                "of his skill level squared times 4 wolves; the wolves will "
                "be placed in the mages inventory. To "
                "summon wolves, the mage should issue the order CAST "
                "Wolf_Lore.";
            break;
        case S_BIRD_LORE:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "A mage with Bird Lore may control the birds of the "
                    "sky. At skill level 1, the mage can control small "
                    "birds, sending them to an adjacent region to obtain a "
                    "report on that region. (This skill only works on the "
                    "surface of the world, as there are no birds elsewhere)."
                    " To use this skill, CAST Bird_Lore DIRECTION <dir>, "
                    "where <dir> is the direction the mage wishes the birds "
                    "to report on.";
            } else if (level == 3) {
                if (ItemDefs[I_EAGLE].flags & ItemType::DISABLED) break;
                str += "A mage with Bird Lore 3 can summon eagles to join "
                    "him, who will aid him in combat, and provide for flying "
                    "transportation. A mage may summon ";
                if (ItemDefs[I_EAGLE].mOut > 0) {
                    str += "an average of ";
                    str += std::to_string(ItemDefs[I_EAGLE].mOut);
                    str += " percent times his skill level minus 2 eagles";
                }
                else
                    str += "one eagle";
                str += " per month, and may control a number equal to "
                    "his skill level minus 2, squared, times two. "
                    "To summon an eagle, issue the order CAST "
                    "Bird_Lore EAGLE; the eagles will appear in his inventory.";
            }
            break;
        case S_DRAGON_LORE:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_DRAGON].flags & ItemType::DISABLED) break;
            str += "A mage with Dragon Lore skill can summon dragons to "
                "join him, to aid in battle, and provide flying "
                "transportation. ";
            if (ItemDefs[I_DRAGON].mOut > 0) {
                str += "A mage has a ";
                str += std::to_string(ItemDefs[I_DRAGON].mOut);
                str += "% times his skill level chance to summon a dragon";
            }
            else
                str += "A mage at level 1 has a low chance of "
                    "successfully summoning a dragon, gradually increasing "
                    "until at level 5 he may summon one dragon per turn";
            str += "; the total number of dragons that a mage may control at one "
                "time is equal to his skill level. To attempt to summon a "
                "dragon, CAST Dragon_Lore.";
            break;
        case S_NECROMANCY:
            if (level > 1) break;
            str += "Necromancy is the magic of death; a mage versed in "
                "Necromancy may raise and control the dead, and turn the "
                "powers of death to his own nefarious purposes. The "
                "Necromancy skill does not have any direct application, but "
                "is required to study the more powerful Necromantic skills. "
                "A mage with knowledge of Necromancy will detect the use of "
                "Necromancy by any other mage in the same region.";
            break;
        case S_SUMMON_SKELETONS:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_SKELETON].flags & ItemType::DISABLED) break;
            str += "A mage with the Summon Skeletons skill may summon "
                "skeletons into his inventory, to aid him in battle. "
                "Skeletons may be given to other units, as they follow "
                "instructions mindlessly; however, they have a " + std::to_string(ItemDefs[I_SKELETON].esc_val) +
                " percent chance of decaying each turn. A mage can summon skeletons "
                "at an average rate of ";
            if (ItemDefs[I_SKELETON].mOut > 0) {
                str += std::to_string(ItemDefs[I_SKELETON].mOut);
                str += " percent times his skill level.";
            }
            else
                str += "40 percent times his level squared.";
            str += " To use the spell, use the order CAST Summon_Skeletons, "
                "and the mage will summon as many skeletons as he is able.";
            break;
        case S_RAISE_UNDEAD:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_UNDEAD].flags & ItemType::DISABLED) break;
            str += "A mage with the Raise Undead skill may summon undead "
                "into his inventory, to aid him in battle. Undead may be "
                "given to other units, as they follow instructions "
                "mindlessly; however, they have a " + std::to_string(ItemDefs[I_UNDEAD].esc_val) +
                " percent chance of decaying each turn. A mage can summon undead at an average "
                "rate of ";
            if (ItemDefs[I_UNDEAD].mOut > 0) {
                str += std::to_string(ItemDefs[I_UNDEAD].mOut);
                str += " percent times his skill level.";
            }
            else
                str += "10 percent times his level squared.";
            str += " To use the spell, use the order CAST Raise_Undead and the "
                "mage will summon as many undead as he is able.";
            break;
        case S_SUMMON_LICH:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_LICH].flags & ItemType::DISABLED) break;
            str += "A mage with the Summon Lich skill may summon a lich "
                "into his inventory, to aid him in battle. Liches may be "
                "given to other units, as they follow instructions "
                "mindlessly; however, they have a " + std::to_string(ItemDefs[I_LICH].esc_val) +
                " percent chance of decaying each turn. A mage has a ";
            if (ItemDefs[I_LICH].mOut > 0) {
                str += std::to_string(ItemDefs[I_LICH].mOut);
                str += " percent times his skill level";
            }
            else
                str += "2 percent times his level squared";
            str += " chance of summoning a lich; to summon a lich, use "
                "the order CAST Summon_Lich.";
            break;
        case S_CREATE_AURA_OF_FEAR:
            if (level > 1) break;
            break;
        case S_SUMMON_BLACK_WIND:
            if (level > 1) break;
            break;
        case S_BANISH_UNDEAD:
            if (level > 1) break;
            break;
        case S_DEMON_LORE:
            if (level > 1) break;
            str += "Demon Lore is the art of summoning and controlling "
                "demons. The Demon Lore skill does not give the mage any "
                "direct skills, but is required to study further into the "
                "Demonic arts. A mage with knowledge of Demon Lore will "
                "detect the use of Demon Lore by any other mage in the same "
                "region.";
            break;
        case S_SUMMON_IMPS:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_IMP].flags & ItemType::DISABLED) break;
            str += "A mage with the Summon Imps skill may summon imps into "
                "his inventory, to aid him in combat. A mage may summon ";
            if (ItemDefs[I_IMP].mOut > 0) {
                str += std::to_string(ItemDefs[I_IMP].mOut);
                str += " percent times his skill level imps";
            }
            else
                str += "one imp per skill level";
            str += "; however, the imps have a chance of "
                "breaking free of the mage's control at the end of each "
                "turn. ";
            str += describe_escape_parameters(I_IMP);
            str += "To use this spell, the mage should issue the order CAST "
                "Summon_Imps, and the mage will summon as many imps as he "
                "is able.";
            break;
        case S_SUMMON_DEMON:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_DEMON].flags & ItemType::DISABLED) break;
            str += "A mage with the Summon Demon skill may summon demons "
                "into his inventory, to aid him in combat. A mage may summon ";
            if (ItemDefs[I_DEMON].mOut > 0) {
                str += std::to_string(ItemDefs[I_DEMON].mOut);
                str += " percent times his skill level demons";
            }
            else
                str += "one demon";
            str += " each turn; however, the demons have a chance of "
                "breaking free of the mage's control at the end of each "
                "turn. ";
            str += describe_escape_parameters(I_DEMON);
            str += "To use this spell, the mage should issue the "
                "order CAST Summon_Demon.";
            break;
        case S_SUMMON_BALROG:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_BALROG].flags & ItemType::DISABLED) break;
            str += "A mage with the Summon Balrog skill may summon a balrog "
                "into his inventory, to aid him in combat. A mage has a ";
            str += std::to_string(ItemDefs[I_BALROG].mOut);
            str += " percent times his skill level chance of summoning a balrog, "
                "but may only summon a balrog if one is not already under "
                "his control. As with other demons, the balrog has a chance "
                "of breaking free of the mage's control at the end of each "
                "turn. ";
            str += describe_escape_parameters(I_BALROG);
            str += "To use this spell, "
                "the mage should issue the order CAST Summon_Balrog.";
            break;
        case S_BANISH_DEMONS:
            if (level > 1) break;
            break;
        case S_ILLUSION:
            if (level > 1) break;
            str += "Illusion is the magic of creating images of things that "
                "do not actually exist. The Illusion skill does not have any "
                "direct applications, but is required for further study of "
                "Illusionary magic. A mage with knowledge of the Illusion "
                "skill will detect the use of Illusion by any other mage in "
                "the same region.";
            break;
        case S_PHANTASMAL_ENTERTAINMENT:
            /* XXX -- This should be cleaner somehow */
            if (level > 1) break;
            str += "A mage with the Phantasmal Entertainment skill may use "
                "his powers of Illusion to earn money by creating "
                "illusionary fireworks, puppet shows, etc. In effect, "
                "Phantasmal Entertainment grants the mage Entertainment "
                "skill equal to ";
            str += std::to_string(Globals->ENTERTAIN_INCOME * 20);
            str += " silver times his Phantasmal Entertainment "
                "level. To use this skill, the mage should CAST Phantasmal_Entertainment.";
            break;
        case S_CREATE_PHANTASMAL_BEASTS:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "A mage with Create Phantasmal Beasts may summon "
                    "illusionary beasts that appear in the mage's inventory. "
                    "These beasts will fight in combat, but do not attack, "
                    "and are killed whenever they are attacked.";
                if (!(ItemDefs[I_WOLF].flags & ItemType::DISABLED)) {
                    str += " Create Phantasmal Beasts at level 1 allows the "
                        "mage to summon illusionary wolves; the number the "
                        "mage can summon, or have in his inventory at one "
                        "time is equal to the mage's skill squared times 4. "
                        "To use this spell, the mage should CAST "
                        "Create_Phantasmal_Beasts WOLF <number>, where "
                        "<number> is the number of wolves that the mage "
                        "wishes to have appear in his inventory.";
                }
                str += " Note: illusionary beasts will appear on reports as "
                    "if they were normal items, except on the owner's "
                    "report, where they are marked as illusionary. To "
                    "reference these items in orders, you must prepend an "
                    "'i' to the normal string. (For example: to reference "
                    "an illusionary wolf, you would use 'iwolf').";
            } else if (level == 2) {
                if (ItemDefs[I_EAGLE].flags & ItemType::DISABLED) break;
                str += "Create Phantasmal Beasts at level 2 allows the mage "
                    "to summon illusionary eagles into his inventory. To "
                    "summon illusionary eagles, the mage should CAST "
                    "Create_Phantasmal_Beasts EAGLE <number>, where <number> "
                    "is the number of eagles that the mage wishes to have "
                    "appear in his inventory. The number of eagles that a "
                    "mage may have in his inventory is equal to mage's skill squared.";
            } else if (level == 3) {
                if (ItemDefs[I_DRAGON].flags & ItemType::DISABLED) break;
                str += "Create Phantasmal Beasts at level 3 allows the "
                    "mage to summon illusionary dragons into his "
                    "inventory. To summon illusionary dragons, the mage "
                    "should CAST Create_Phantasmal_Beasts DRAGON <number>. The number of dragons that a "
                    "mage may have in his inventory is equal to mage's skill level.";
            }
            break;
        case S_CREATE_PHANTASMAL_UNDEAD:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "A mage with Create Phantasmal Undead may summon "
                    "illusionary undead that appear in the mage's inventory. "
                    "These undead will fight in combat, but do not attack, "
                    "and are killed whenever they are attacked.";
                if (!(ItemDefs[I_SKELETON].flags & ItemType::DISABLED)) {
                    str += " Create Phantasmal Undead at level 1 allows the "
                        "mage to summon illusionary skeletons; the number "
                        "the mage can summon, or have in his inventory at "
                        "one time is equal to the mage's skill squared times "
                        "4. To use this spell, the mage should CAST "
                        "Create_Phantasmal_Undead SKELETON <number>, where "
                        "<number> is the number of skeletons that the mage "
                        "wishes to have appear in his inventory.";
                }
                str += " Note: illusionary undead will appear on reports as "
                    "if they were normal items, except on the owner's "
                    "report, where they are marked as illusionary. To "
                    "reference these items in orders, you must prepend an "
                    "'i' to the normal string. (Example: to reference an "
                    "illusionary skeleton, you would use 'iskeleton').";
            } else if (level == 2) {
                if (ItemDefs[I_UNDEAD].flags & ItemType::DISABLED) break;
                str += "Create Phantasmal Undead at level 2 allows the mage "
                    "to summon illusionary undead into his inventory. To "
                    "summon illusionary undead, the mage should CAST "
                    "Create_Phantasmal_Undead UNDEAD <number>, where <number> "
                    "is the number of undead that the mage wishes to have "
                    "appear in his inventory. The number of undead that a "
                    "mage may have in his inventory is equal to mage's skill squared.";
            } else if (level == 3) {
                if (ItemDefs[I_LICH].flags & ItemType::DISABLED) break;
                str += "Create Phantasmal Undead at level 3 allows the mage "
                    "to summon illusionary liches into his inventory. To "
                    "summon illusionary liches, the mage should CAST "
                    "Create_Phantasmal_Undead LICH <number>; The number of liches that a "
                    "mage may have in his inventory is equal to mage's skill level.";
            }
            break;
        case S_CREATE_PHANTASMAL_DEMONS:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "A mage with Create Phantasmal Demons may summon "
                        "illusionary demons that appear in the mage's "
                        "inventory. These demons will fight in combat, but "
                        "do not attack, and are killed whenever they are "
                        "attacked.";
                if (!(ItemDefs[I_IMP].flags & ItemType::DISABLED)) {
                    str += " Create Phantasmal Demons at level 1 allows the "
                        "mage to summon illusionary imps; the number the "
                        "mage can summon, or have in his inventory at one "
                        "time is equal to the mage's skill squared times 4. "
                        "To use this spell, the mage should CAST "
                        "Create_Phantasmal_Demons IMP <number>, where "
                        "<number> is the number of imps that the mage wishes "
                        "to have appear in his inventory.";
                }
                str += " Note: illusionary demons will appear on reports as "
                    "if they were normal items, except on the owner's "
                    "report, where they are marked as illusionary. To "
                    "reference these items in orders, you must prepend an "
                    "'i' to the normal string. (Example: to reference an "
                    "illusionary imp, you would use 'iimp').";
            } else if (level == 2) {
                if (ItemDefs[I_DEMON].flags & ItemType::DISABLED) break;
                str += "Create Phantasmal Demons at level 2 allows the mage "
                    "to summon illusionary demons into his inventory. To "
                    "summon illusionary demons, the mage should CAST "
                    "Create_Phantasmal_Demons DEMON <number>, where <number> "
                    "is the number of demons that the mage wishes to have "
                    "appear in his inventory. The number of demons that a "
                    "mage may have in his inventory is equal to mage's skill squared.";
            } else if (level == 3) {
                if (ItemDefs[I_BALROG].flags & ItemType::DISABLED) break;
                str += "Create Phantasmal Demons at level 3 allows the mage "
                    "to summon an illusionary balrog into his inventory. To "
                    "summon an illusionary balrog, the mage should CAST "
                    "Create_Phantasmal_Demons BALROG; the mage can only have "
                    "one illusionary balrog in his inventory at one time.";
            }
            break;
        case S_INVISIBILITY:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            str += "The Invisibility skill allows a mage to render other "
                "units nearly invisible to other factions, giving them a +3 "
                "bonus to Stealth. This invisibility will last until the "
                "next Magic round. To cast this spell, use the order CAST "
                "Invisibility UNITS <unit> ..., where <unit> is a list of "
                "the units that the mage wishes to render invisible. A mage "
                "may render invisible a number of men or creatures equal to "
                "his skill level squared.";
            if (Globals->FULL_INVIS_ON_SELF) {
                str += " A mage automatically gets his Invisibility skill added to his stealth.";
            }
            break;
        case S_TRUE_SEEING:
            if (level > 1) break;
            str += "A mage with the True Seeing spell can see illusions "
                "for what they really are. Whether or not the mage can see "
                "through the illusion depends on his True Seeing skill "
                "being higher that the Illusion skill of the mage casting "
                "the illusion. This spell does not require any order to "
                "use; it is used automatically.";
            if (!(SkillDefs[S_OBSERVATION].flags & SkillType::DISABLED)) {
                if (Globals->FULL_TRUESEEING_BONUS) {
                    str += " In addition, a mage with the True Seeing skill "
                        "receives a bonus to his Observation skill equal to his "
                        "True Seeing skill.";
                } else {
                    str += " In addition, a mage with the True Seeing skill "
                        "receives a bonus to his Observation skill equal to his "
                        "True Seeing skill divided by 2, rounded up.";
                }
            }
            break;
        case S_DISPEL_ILLUSIONS:
            if (level > 1) break;
            break;
        case S_ARTIFACT_LORE:
            if (level > 1) break;
            str += "Artifact Lore is one of the most advanced forms of "
                "magic; in general, creation of an artifact requires both "
                "Artifact Lore, and the appropriate skill for the item being "
                "created. A mage with knowledge of the Artifact Lore skill "
                "will detect the use of Artifact Lore by any other mage in "
                "the region.";
            break;
        case S_ENGRAVE_RUNES_OF_WARDING:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "A mage with the Engrave Runes of Warding may "
                    "engrave runes of warding on a building; these runes "
                    "will give any occupants of the building a personal "
                    "Energy Shield and Spirit Shield, both at level 3. "
                    "A mage has a 20 percent chance per level of succeeding "
                    "with each attempt to cast this spell. To use this "
                    "spell, the mage should CAST Engrave_Runes_of_Warding, "
                    "and be within the building he wishes to engrave runes "
                    "upon. This spell costs 600 silver to cast.";
                if (!(ObjectDefs[O_TOWER].flags & ObjectType::DISABLED)) {
                    str += " At level 1, the mage may engrave runes of "
                        "warding upon a ";
                    str += ObjectDefs[O_TOWER].name;
                    str += ".";
                }
            } else if (level == 2) {
                if (!(ObjectDefs[O_FORT].flags & ObjectType::DISABLED)) {
                    str += "At this level, the mage may engrave runes of "
                        "warding upon a ";
                    str += ObjectDefs[O_FORT].name;
                    str += ".";
                }
            } else if (level == 3) {
                if (
                    !(ItemDefs[I_ROOTSTONE].flags & ItemType::DISABLED) &&
                    (
                        !(ObjectDefs[O_MTOWER].flags & ObjectType::DISABLED) ||
                        !(ObjectDefs[O_MFORTRESS].flags & ObjectType::DISABLED) ||
                        !(ObjectDefs[O_MCASTLE].flags & ObjectType::DISABLED) ||
                        !(ObjectDefs[O_MCITADEL].flags & ObjectType::DISABLED)
                    )
                ) {
                    str += "At this level, the mage improves the level of the Shields "
                        "granted by these runes to level 5 for buildings that are "
                        "made from rootstone. ";
                } else if (!(ObjectDefs[O_CASTLE].flags & ObjectType::DISABLED)) {
                    str += "At this level, the mage may engrave runes of "
                        "warding upon a ";
                    str += ObjectDefs[O_CASTLE].name;
                    str += ".";
                    break;
                }
                if (
                    (ObjectDefs[O_CASTLE].flags & ObjectType::DISABLED) &&
                    (ObjectDefs[O_MTOWER].flags & ObjectType::DISABLED)
                ) {
                    break;
                }
                int comma = 0;
                str += "The mage may now engrave runes of "
                    "warding upon ";
                if (!(ObjectDefs[O_CASTLE].flags & ObjectType::DISABLED)) {
                    str += "a ";
                    str += ObjectDefs[O_CASTLE].name;
                    comma = 1;
                }
                if (!(ObjectDefs[O_MTOWER].flags & ObjectType::DISABLED)) {
                    if (comma) str += " or ";
                    str += "a ";
                    str += ObjectDefs[O_MTOWER].name;
                }
                str += ".";
            } else if (level == 4) {
                int comma = 0;
                if (
                    (ObjectDefs[O_CITADEL].flags & ObjectType::DISABLED) &&
                    (ObjectDefs[O_MFORTRESS].flags & ObjectType::DISABLED)
                ) {
                    break;
                }
                str += "At this level, the mage may engrave runes of "
                    "warding upon ";
                if (!(ObjectDefs[O_CITADEL].flags & ObjectType::DISABLED)) {
                    str += "a ";
                    str += ObjectDefs[O_CITADEL].name;
                    comma = 1;
                }
                if (!(ObjectDefs[O_MFORTRESS].flags & ObjectType::DISABLED)) {
                    if (comma) str += " or ";
                    str += "a ";
                    str += ObjectDefs[O_MFORTRESS].name;
                }
                str += ".";
            } else if (level == 5) {
                int comma = 0;
                if (
                    (ObjectDefs[O_MCASTLE].flags & ObjectType::DISABLED) &&
                    (ObjectDefs[O_MCITADEL].flags & ObjectType::DISABLED)
                ) {
                    break;
                }
                str += "At this level, the mage may engrave runes of "
                    "warding upon ";
                if (!(ObjectDefs[O_MCASTLE].flags & ObjectType::DISABLED)) {
                    str += "a ";
                    str += ObjectDefs[O_MCASTLE].name;
                    comma = 1;
                }
                if (!(ObjectDefs[O_MCITADEL].flags & ObjectType::DISABLED)) {
                    if (comma) str += " or ";
                    str += "a ";
                    str += ObjectDefs[O_MCITADEL].name;
                }
                str += ".";
            }
            break;
        case S_CONSTRUCT_GATE:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            str += "A mage with the Construct Gate skill may construct a "
                "Gate in a region. The mage has a 20 percent times his "
                "skill level chance of success, and the attempt costs 1000 "
                "silver. To use this spell, the mage should issue the order "
                "CAST Construct_Gate.";
            break;
        case S_CONSTRUCT_PORTAL:
            /* XXX -- This should be cleaner somehow. */
            if (level > 1) break;
            if (ItemDefs[I_PORTAL].flags & ItemType::DISABLED) break;
            str += "A mage with the Construct Portal skill may "
                "construct a Portal";
            if (!(SkillDefs[S_PORTAL_LORE].flags & SkillType::DISABLED)) {
                str += " for use with the Portal Lore skill";
            }
            str += ".";
            break;
        case S_TRANSMUTATION:
            /* XXX -- This should be cleaner somehow. */
            if (level == 1) {
                str += "A mage with Transmutation may transform basic resources into more esoteric ones. ";
                std::vector<std::string> options;
                if (
                    !(ItemDefs[I_STONE].flags & ItemType::DISABLED) &&
                    !(ItemDefs[I_ROOTSTONE].flags & ItemType::DISABLED)
                ) {
                    std::string option = item_string(I_STONE, ItemDefs[I_ROOTSTONE].mOut) +
                        " times the skill level into " + item_string(I_ROOTSTONE, 1);
                    options.push_back(option);
                }
                if (
                    !(ItemDefs[I_IRON].flags & ItemType::DISABLED) &&
                    !(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED)
                ) {
                    std::string option = item_string(I_IRON, ItemDefs[I_MITHRIL].mOut) +
                        " times the skill level into " + item_string(I_MITHRIL, 1);
                    options.push_back(option);
                }
                if (options.size() > 0) {
                    str += "At level 1 the mage may transmute ";
                    str += strings::join(options, ", ", " or ");
                    str += ". ";
                }

                str += "To use this spell, the mage should issue the order CAST Transmutation <material>, "
                    "where <material> is the resource you wish to create. Should you wish to create fewer than "
                    "maximum, you may CAST Transmutation [number] <material> instead.";
            } else if (level == 2) {
                if (
                    !(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
                    !(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED)
                ) {
                    str += "At this level the mage may transmute " + item_string(I_WOOD, ItemDefs[I_IRONWOOD].mOut) +
                        " times the skill level into " + item_string(I_IRONWOOD, 1) + ".";
                }
            } else if (level == 3) {
                if (
                    !(ItemDefs[I_FUR].flags & ItemType::DISABLED) && !(ItemDefs[I_FLOATER].flags & ItemType::DISABLED)
                ) {
                    str += "At this level the mage may transmute " + item_string(I_FUR, ItemDefs[I_FLOATER].mOut) +
                        " times the skill level into " + item_string(I_FLOATER, 1) + ".";
                }
            } else if (level == 4) {
                if (
                    !(ItemDefs[I_WOOD].flags & ItemType::DISABLED) && !(ItemDefs[I_YEW].flags & ItemType::DISABLED)
                ) {
                    str += "At this level the mage may transmute " + item_string(I_WOOD, ItemDefs[I_YEW].mOut) +
                        " times the skill level into " + item_string(I_YEW, 1) + ".";
                }
            } else if (level == 5) {
                if (
                    !(ItemDefs[I_HORSE].flags & ItemType::DISABLED) &&
                    !(ItemDefs[I_WHORSE].flags & ItemType::DISABLED)
                ) {
                    str += "At this level the mage may transmute " +
                        item_string(I_HORSE, ItemDefs[I_WHORSE].mOut, ALWAYSPLURAL) + " times the skill level into " +
                        item_string(I_WHORSE, 1, ALWAYSPLURAL) + ".";
                }
                if (
                    !(ItemDefs[I_IRON].flags & ItemType::DISABLED) &&
                    !(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED)
                ) {
                    str += " At this level the mage may transmute " +
                        item_string(I_IRON, ItemDefs[I_ADMANTIUM].mOut, ALWAYSPLURAL) + " times the skill level into " +
                        item_string(I_ADMANTIUM, 1, ALWAYSPLURAL) + ".";
                }
            }
            break;
        case S_BLASPHEMOUS_RITUAL:
            if (level > 1) break;
            if (ObjectDefs[O_BKEEP].flags & ObjectType::DISABLED) break;
            str += "A mage with the Blasphemous Ritual skill may "
                "perform a blasphemous ritual to get a WISH power and "
                "win the game. ";
            str += "This ritual requires 60";
            str += " sacrifice of a randomly selected "
                "leader belonging to the mage's faction.";
            if (ObjectDefs[O_BKEEP].cost > 1) {
                str += " Mage must be inside a Black Keep to cast this spell. "
                    "60 such sacrifices will be "
                    "necessary to complete the ritual; "
                    "the caster will attempt to perform "
                    "as many sacrifices as their skill "
                    "level in Blasphemous Ritual.";
            }
            break;
        case S_MANIPULATE:
            if (!Globals->APPRENTICES_EXIST) break;
            if (level == 1) {
                str += "A unit with this skill becomes an ";
                str += Globals->APPRENTICE_NAME;
                str += ". While ";
                str += Globals->APPRENTICE_NAME;
                str += "s cannot cast spells directly, they can "
                    "use magic items normally only usable by mages.";
            }
            max = 1;
            if (!(ItemDefs[I_CORNUCOPIA].flags & ItemType::DISABLED)) max = 2;
            if (
                !(ItemDefs[I_GATE_CRYSTAL].flags & ItemType::DISABLED) ||
                !(ItemDefs[I_STAFFOFH].flags & ItemType::DISABLED) ||
                !(ItemDefs[I_SCRYINGORB].flags & ItemType::DISABLED)
            ) {
                max = 3;
            }
            if (level == max) str += "Continued study of this skill gives no further advantages.";
            break;
        case S_WEAPONCRAFT:
            if (level > 1) break;
            str += "The weaponcraft skill is an advanced version of the weaponsmith skill.";
            break;
        case S_ARMORCRAFT:
            if (level > 1) break;
            str += "The armorcraft skill is an advanced version of the armorsmith skill.";
            break;
        case S_CAMELTRAINING:
            if (level > 1) break;
            str += "This skill deals with all aspects of camel production.";
            break;
        case S_GEMCUTTING:
            if (level > 1) break;
            str += "This skill enables a unit to fashion higher quality gems from lower quality ones.";
            break;
        case S_MONSTERTRAINING:
            if (level > 1) break;
            str += "This skill deals with all aspects of training monster mounts.";
            break;
        case S_COOKING:
            if (level > 1) break;
            str += "This skill deals with creating provisions from basic foodstuffs. A skilled cook can feed "
                "many more people than a farmer alone.";
            break;
        case S_ANNIHILATION:
            if (level > 1) break;
            range = find_range(SkillDefs[skill].range.value_or(""));
            str += "A unit with access to the Annihilation skill may destroy a region and all surrounding "
                "regions.  Regions destroyed in this way will become barren.  All units and structures except for "
                "shafts and anomalies in the region and the surrounding regions will be destroyed as will production "
                "and cities.";
            if (!(ObjectDefs[O_ACTIVE_MONOLITH].flags & ObjectType::DISABLED)) {
                if (ObjectDefs[O_ACTIVE_MONOLITH].flags & ObjectType::GRANTSKILL) {
                    str += " This skill is only available to the unit which controls the World-breaker Monolith. "
                        "To use this skill, the owner of the World-breaker Monolith ";
                }
            } else {
                str += " To use this skill, the unit ";
            }
            str += "must issue the order ANNIHILATE REGION <x> <y>";
            if (range && range->get().flags & RangeType::RNG_SURFACE_ONLY) {
                str += ".";
            } else {
                str += " <z>. If the z coordinate is not specified, it will default to the same z coordinate as "
                    "the unit utilizing the skill.";
            }

            if (range && range->get().flags & RangeType::RNG_SURFACE_ONLY) {
                str += " This skill can only target a region on the surface of the world.";
            } else {
                str += " This skill can target a region anywhere in the world.";
            }
            break;
    }

    std::string temp;
    std::string temp1;
    std::string temp2;
    std::string temp3;
    std::string temp4;
    int comma = 0;
    int comma1 = 0;
    int count;
    int last = -1;
    int last1 = -1;
    unsigned int c;
    int i;
    int build = 0;

    // If this is a combat spell, note it.
    if (level == 1 && (SkillDefs[skill].flags & SkillType::COMBAT)) {
        str += " A mage with this skill can cast " + show_special(SkillDefs[skill].special.value_or("").c_str(), level, 0, 0) +
            " In order to use this spell in combat, the mage should use the COMBAT order to set it as his combat spell.";
    }

    // production and ability to see items
    temp = "A unit with this skill is able to determine if a region contains ";
    temp1 = "A unit with this skill may PRODUCE ";
    temp2 = "";
    // for the new ship items
    temp3 = "A unit with this skill may BUILD ";

    const SkillType& sk1 = SkillDefs[skill];
    for (i = NITEMS - 1; i >= 0; i--) {
        if (ItemDefs[i].flags & ItemType::DISABLED) continue;
        auto sk2 = FindSkill(ItemDefs[i].pSkill);
        if (sk2 && sk1 == sk2->get() && ItemDefs[i].pLevel == level) {
            int canmake = 1;
            int resource = 1;
            for (c = 0; c < sizeof(ItemDefs[i].pInput)/sizeof(ItemDefs[i].pInput[0]); c++) {
                if (ItemDefs[i].pInput[c].item == -1) continue;
                resource = 0;
                if (ItemDefs[ItemDefs[i].pInput[c].item].flags & ItemType::DISABLED) {
                    canmake = 0;
                }
            }
            if (canmake && last1 == -1) {
                last1 = i;
            }
            if (resource && (ItemDefs[i].type & IT_ADVANCED) && last == -1) {
                last = i;
            }
        }

    }
    for (i = 0; i < NITEMS; i++) {
        if (ItemDefs[i].flags & ItemType::DISABLED) continue;
        int illusion = (ItemDefs[i].type & IT_ILLUSION);
        auto sk2 = FindSkill(ItemDefs[i].mSkill);
        if (sk2 && sk1 == sk2->get() && ItemDefs[i].mLevel == level) {
            int canmagic = 1;
            for (c = 0; c < sizeof(ItemDefs[i].mInput)/sizeof(ItemDefs[i].mInput[0]); c++) {
                if (ItemDefs[i].mInput[c].item == -1) continue;
                if (ItemDefs[ItemDefs[i].mInput[c].item].flags & ItemType::DISABLED) {
                    canmagic = 0;
                }
            }
            if (ItemDefs[i].mOut == 0)
                canmagic = 0;
            if (canmagic) {
                temp2 += "A mage with this skill ";
                if (ItemDefs[i].mOut < 100) {
                    temp2 += "has a ";
                    temp2 += std::to_string(ItemDefs[i].mOut);
                    temp2 += " percent times their level chance to create a";
                    if (illusion) {
                        temp2 += "n illusory ";
                    } else {
                        switch (ItemDefs[i].name[0]) {
                            case 'a':
                            case 'e':
                            case 'i':
                            case 'o':
                            case 'u':
                            case 'A':
                            case 'E':
                            case 'I':
                            case 'O':
                            case 'U':
                                temp2 += "n";
                                break;
                            default:
                                break;
                        }
                        temp2 += " ";
                    }
                    temp2 += ItemDefs[i].name;
                } else {
                    temp2 += "may create ";
                    if (ItemDefs[i].mOut > 100) {
                        temp2 += std::to_string(ItemDefs[i].mOut / 100);
                        temp2 += " times ";
                    }
                    temp2 += "their level in ";
                    temp2 += std::string(illusion?"illusory ":"") + ItemDefs[i].names;
                }
                temp2 += " [";
                temp2 += ItemDefs[i].abr;
                temp2 += "] via magic";
                count = 0;
                for (c = 0; c < sizeof(ItemDefs[i].mInput)/sizeof(ItemDefs[i].mInput[0]); c++) {
                    if (ItemDefs[i].mInput[c].item == -1) continue;
                    count++;
                }
                if (count > 0) {
                    temp2 += " at a cost of ";
                    temp4 = "";
                    count = 0;
                    for (c = 0; c < sizeof(ItemDefs[i].mInput)/sizeof(ItemDefs[i].mInput[0]); c++) {
                        if (ItemDefs[i].mInput[c].item == -1) continue;
                        if (!temp4.empty()) {
                            if (count > 0)
                                temp2 += ", ";
                            temp2 += temp4;
                            count++;
                        }
                        temp4 = item_string(ItemDefs[i].mInput[c].item, ItemDefs[i].mInput[c].amt);
                    }
                    if (count > 0)
                        temp2 += " and ";
                    temp2 += temp4;
                }
            }
            if (f) {
                f->DiscoverItem(i, 1, 1);
            }
        }
        sk2 = FindSkill(ItemDefs[i].pSkill);
        if (sk2 && sk1 == sk2->get() && ItemDefs[i].pLevel == level) {
            int canmake = 1;
            int resource = 1;
            for (c = 0; c < sizeof(ItemDefs[i].pInput)/sizeof(ItemDefs[i].pInput[0]); c++) {
                if (ItemDefs[i].pInput[c].item == -1) continue;
                resource = 0;
                if (ItemDefs[ItemDefs[i].pInput[c].item].flags & ItemType::DISABLED) {
                    canmake = 0;
                }
            }
            if (canmake) {
                // IT_SHIP: switch to BUILD description
                if ((ItemDefs[i].type & IT_SHIP) && (build == 0)) {
                    temp1 = temp3;
                    build = 1;
                }
                if (comma1) {
                    if (last1 == i) {
                        if (comma1 > 1) temp1 += ",";
                        temp1 += " and ";
                    } else {
                        temp1 += ", ";
                    }
                }
                comma1++;
                if (ItemDefs[i].flags & ItemType::SKILLOUT) {
                    temp1 += "a number of ";
                }
                if (ItemDefs[i].flags & ItemType::SKILLOUT_HALF) {
                    temp1 += "a number of ";
                }
                temp1 += std::string(illusion?"illusory ":"") + ItemDefs[i].names;
                temp1 += " [";
                temp1 += ItemDefs[i].abr;
                temp1 += "]";
                if (ItemDefs[i].flags & ItemType::SKILLOUT) {
                    temp1 += " equal to their skill level";
                }
                if (ItemDefs[i].flags & ItemType::SKILLOUT_HALF) {
                    temp1 += " equal to skill level divided by 2, rounded up";
                }
                if (!resource) {
                    temp1 += " from ";
                    if (ItemDefs[i].flags & ItemType::ORINPUTS)
                        temp1 += "any of ";
                    temp4 = "";
                    count = 0;
                    for (c = 0; c < sizeof(ItemDefs[i].pInput)/sizeof(ItemDefs[i].pInput[0]); c++) {
                        if (ItemDefs[i].pInput[c].item == -1) continue;
                        if (!temp4.empty()) {
                            if (count > 0)
                                temp1 += ", ";
                            temp1 += temp4;
                            count++;
                        }
                        temp4 = item_string(ItemDefs[i].pInput[c].item,
                            ItemDefs[i].type & IT_SHIP ?
                                ItemDefs[i].pMonths :
                                ItemDefs[i].pInput[c].amt);
                    }
                    if (count > 0)
                        temp1 += " and ";
                    temp1 += temp4;
                }
                if (!build) {
                    temp1 += " at a rate of ";
                    temp1 += std::to_string(ItemDefs[i].pOut);
                    temp1 += " per ";
                    if (ItemDefs[i].pMonths == 1) {
                        temp1 += "man-month";
                    } else {
                        temp1 += std::to_string(ItemDefs[i].pMonths);
                        temp1 += " man-months";
                    }
                }
                if (f) {
                    f->DiscoverItem(i, 1, 1);
                }
            }
            if (resource && (ItemDefs[i].type & IT_ADVANCED)) {
                if (comma) {
                    if (last == i) {
                        if (comma > 1) temp += ",";
                        temp += " and ";
                    } else {
                        temp += ", ";
                    }
                }
                comma++;
                temp += std::string(illusion?"illusory ":"") + ItemDefs[i].names;
            }
        }
    }
    if (comma1) {
        if (!str.empty()) str += " ";
        str += temp1 + ".";
    }
    if (comma) {
        if (!str.empty()) str += " ";
        str += temp + ".";
    }
    if (!temp2.empty()) {
        if (!str.empty()) str += " ";
        str += temp2;
        str += ". To use this spell, the mage should CAST ";
        str += sk1.name | filter::canonicalize;
        str += ".";
    }

    // Buildings
    comma = 0;
    temp = "";
    temp2 = "";
    for (i = 0; i < NOBJECTS; i++) {
        if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
        if (ObjectDefs[i].item == -1) continue;
        if (ObjectDefs[i].item != I_WOOD_OR_STONE &&
                (ItemDefs[ObjectDefs[i].item].flags & ItemType::DISABLED))
            continue;
        if (ObjectDefs[i].item == I_WOOD_OR_STONE &&
                (ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
                (ItemDefs[I_STONE].flags & ItemType::DISABLED))
            continue;
        std::string skname = SkillDefs[skill].abbr;
        if (skname == ObjectDefs[i].skill && ObjectDefs[i].level == level) {
            if (comma) {
                temp += ", ";
            }
            temp += temp2;
            if (!temp.empty()) comma = 1;
            temp2 = "a";
            switch (ObjectDefs[i].name[0]) {
                case 'a':
                case 'e':
                case 'i':
                case 'o':
                case 'u':
                case 'A':
                case 'E':
                case 'I':
                case 'O':
                case 'U':
                    temp2 += "n";
                    break;
                default:
                    break;
            }
            temp2 += " ";
            temp2 += ObjectDefs[i].name;
            temp2 += " from ";
            temp2 += std::to_string(ObjectDefs[i].cost);
            temp2 += " ";
            if (ObjectDefs[i].item == I_WOOD_OR_STONE) {
                if (ItemDefs[I_WOOD].flags & ItemType::DISABLED) {
                    temp2 += ItemDefs[I_STONE].name;
                    temp2 += " [";
                    temp2 += ItemDefs[I_STONE].abr;
                    temp2 += "]";
                } else if (ItemDefs[I_STONE].flags & ItemType::DISABLED) {
                    temp2 += ItemDefs[I_WOOD].name;
                    temp2 += " [";
                    temp2 += ItemDefs[I_WOOD].abr;
                    temp2 += "]";
                } else {
                    temp2 += ItemDefs[I_WOOD].name;
                    temp2 += " [";
                    temp2 += ItemDefs[I_WOOD].abr;
                    temp2 += "] or ";
                    temp2 += ItemDefs[I_STONE].name;
                    temp2 += " [";
                    temp2 += ItemDefs[I_STONE].abr;
                    temp2 += "]";
                }
            } else {
                temp2 += ItemDefs[ObjectDefs[i].item].name;
                    temp2 += " [";
                    temp2 += ItemDefs[ObjectDefs[i].item].abr;
                    temp2 += "]";
            }
            if (f) {
                f->objectshows.push_back({.obj = i});
            }
        }
    }
    if (!temp2.empty()) {
        temp = std::string("A unit with this skill may BUILD ") + temp;
        if (comma) {
            temp += " or ";
        }
        temp += temp2;
        if (!str.empty()) str += " ";
        str += temp + ".";
    }

    // Required skills
    last = -1;
    if (level == 1) {
        std::optional<std::reference_wrapper<SkillType>> lastpS = std::nullopt;
        comma = 0;
        int found = 0;
        temp = "This skill requires ";
        for (c=0; c<sizeof(SkillDefs[skill].depends)/sizeof(SkillDefs[skill].depends[0]); c++) {
            auto pS = FindSkill(SkillDefs[skill].depends[c].skill);
            if (!pS || (pS->get().flags & SkillType::DISABLED)) continue;
            found = 1;
            if (!lastpS) {
                lastpS = pS;
                last = c;
                continue;
            }
            if (comma) temp += ", ";
            temp += SkillStrs(lastpS->get()) + " " + std::to_string(SkillDefs[skill].depends[last].level);
            lastpS = pS;
            last = c;
            comma++;
        }
        if (comma) {
            temp += " and ";
        }
        if (found) {
            temp += SkillStrs(lastpS->get()) + " " + std::to_string(SkillDefs[skill].depends[last].level);
        }

        if (found) {
            if (!str.empty()) str += " ";
            str += temp + " to begin to study.";
        }
    }

    if (level == 1) {
        if (SkillDefs[skill].cost) {
            if (!str.empty()) str += " ";
            str += "This skill costs " + std::to_string(SkillDefs[skill].cost) +
                " silver per month of study.";
        }
        if (SkillDefs[skill].flags & SkillType::SLOWSTUDY) {
            if (!str.empty()) str += " ";
            str += "This skill is studied at one half the normal speed.";
        }
    }

    // Tell whether this skill can be taught, studied, or gained through exp
    if (SkillDefs[skill].flags & SkillType::NOSTUDY) {
        if (!str.empty()) str += " ";
        str += "This skill cannot be studied via normal means.";
    }
    if (SkillDefs[skill].flags & SkillType::NOTEACH) {
        if (!str.empty()) str += " ";
        str += "This skill cannot be taught to other units.";
    }
    if ((Globals->SKILL_PRACTICE_AMOUNT > 0) &&
            (SkillDefs[skill].flags & SkillType::NOEXP)) {
        if (!str.empty()) str += " ";
        str += "This skill cannot be increased through experience.";
    }

    if (str.empty()) {
        return("No skill report.");
    }

    return str;
}
