// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER
// MODIFICATIONS
// Date        Person            Comments
// ----        ------            --------
// 2001/Feb/23 Joseph Traub      Made skill texts runtime generated
//
#include "skills.h"
#include "items.h"
#include "object.h"
#include "gamedata.h"
#include "astring.h"

#define ITEM_ENABLED(X) (!(ItemDefs[(X)].flags & ItemType::DISABLED))
#define ITEM_DISABLED(X) (ItemDefs[(X)].flags & ItemType::DISABLED)
#define SKILL_ENABLED(X) (!(SkillDefs[(X)].flags & SkillType::DISABLED))
#define SKILL_DISABLED(X) (SkillDefs[(X)].flags & SkillType::DISABLED)
#define OBJECT_ENABLED(X) (!(ObjectDefs[(X)].flags & ObjectType::DISABLED))
#define OBJECT_DISABLED(X) (ObjectDefs[(X)].flags & ObjectType::DISABLED)

int SkillCost(int skill, int level, int multiplier)
{
    float cost = (float) SkillDefs[skill].cast_cost;
    cost *= multiplier;
    if(SkillDefs[skill].flags & SkillType::COSTVARIES) {
        float leveleffect = (float) (2 + level ) / 3;
        cost /= leveleffect;
    }
    
    cost += 0.99;

    return (int) cost;
}

AString CostString(int skill, int fromlevel, int tolevel, int multiplier)
{
    AString temp;
    for(int i=fromlevel; i<=tolevel; i++) {
        if(i != fromlevel) temp += ", ";
        if(i == tolevel) temp += "or ";
        temp += AString(SkillCost(skill,i,multiplier));
    }
    return temp;
}

AString EnergyString(int skill, int fromlevel, int tolevel, int multiplier)
{
    AString message;
    message = "will cost a mage ";
    if(SkillDefs[skill].flags & SkillType::COSTVARIES) {
        message += CostString(skill,fromlevel,tolevel,multiplier) + 
            " energy at skill levels " + fromlevel + " to " + tolevel + " respectively," ;
    } else message += AString(SkillCost(skill,fromlevel,multiplier)) + " energy.";
    
    return message;
}

AString *ShowSkill::Report(Faction *f)
{
	if(SkillDefs[skill].flags & SkillType::DISABLED) return NULL;
	AString *str = new AString;
	RangeType *range = NULL;
//Awrite(AString("starting") + skill);
	// Here we pick apart the skill
	switch (skill) {
		case S_FARMING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of grain production.";
			break;
		case S_RANCHING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of livestock "
				"production.";
			break;
		case S_MINING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of extracting raw "
				"metals and gems from the earth. Metals and gems tend to be "
				"found more often in mountainous regions, but may be found "
				"elsewhere as well.";
			break;
		case S_LUMBERJACK:
			if(level > 1) break;
			*str += "This skill deals with all aspects of various wood "
			    "production. Woods are more often found in forests, but "
				"may also be found elsewhere.";
			break;
		case S_BANKING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of depositing and "
				"withdrawing funds from banks.";
			break;
		case S_QUARTERMASTER:
			if (level > 1) break;
			if (!(Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT))
				break;
			*str += "This skill deals with transporting and "
				"distributing goods between non-local units "
				"and transport structures.";
			if (Globals->SHIPPING_COST > 0) {
				*str += " The cost of shipping one weight unit from one "
				  "transport structure to another transport structure is ";
				if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST)
					*str += AString("4-((level+1)/2) * ");
				*str += AString(Globals->SHIPPING_COST) + " silver.";
				if (Globals->FRACTIONAL_WEIGHT) {
					*str += " Items with a normal weight of 0 are "
					  "treated as if ";
					*str += Globals->FRACTIONAL_WEIGHT;
					*str += " of the item in question weigh one weight unit.";
				}
			}

			if (Globals->NONLOCAL_TRANSPORT > 0) {
				*str += " Items may be shipped between two transport "
				  "structures which are up to ";
				*str += Globals->NONLOCAL_TRANSPORT;
				if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST)
					*str += " plus (level+1)/3 ";
				*str += (Globals->NONLOCAL_TRANSPORT != 1) ? "hexes" : "hex";
				*str += " distant from each other.";
			} else if (Globals->NONLOCAL_TRANSPORT == 0) {
				*str += " Items may be instantaneously "
					"shipped between any two transport "
					"structures.";
			}
			if (Globals->LOCAL_TRANSPORT > 0) {
				*str += " Items may be distributed from a transport "
				  "structure to any unit or transported to a transport "
				  "structure by any unit located within ";
				*str += Globals->LOCAL_TRANSPORT;
				*str += (Globals->LOCAL_TRANSPORT != 1) ? " hexes" : " hex";
				*str += " of the transport structure.";
			}
			break;
		case S_QUARRYING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of various stone "
				"production. Mountains are the main producers of stone, but "
				"it may be found in other regions as well.";
			break;
		case S_HUNTING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of animal hide "
				"production.";
			break;
		case S_FISHING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of fish production.";
			break;
		case S_HERBLORE:
			if(level > 1) break;
			*str += "This skill deals with all aspects of herb production.";
			break;
		case S_HORSETRAINING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of horse production.";
			break;
		case S_WEAPONSMITH:
			if(level > 1) break;
			*str += "This skill deals with all aspects of weapon "
				"construction and production.";
			break;
		case S_ARMORER:
			if(level > 1) break;
			*str += "This skill deals with all aspects of armour construction "
				"and production.";
			break;
		case S_CARPENTER:
			if(level > 1) break;
			*str += "This skill deals with all aspects of wood based item "
				"production other than for use as weapons.";
			break;
		case S_BUILDING:
			if(level > 1) break;
			*str += "This skill deals with the construction of "
				"fortifications, roads and other buildings, except for "
				"most trade structures.";
			break;
		case S_SHIPBUILDING:
			if(level > 1) break;
			*str += "This skill deals with the construction of all types "
				"of ships.";
			break;
		case S_CONSTRUCTION:
			if(level > 1) break;
			*str += "This skill deals with the construction of mobile "
				"items and structures, such as wagons or ships.";
			break;
		case S_ENTERTAINMENT:
			if(level > 1) break;
			*str += "A unit with this skill may use the ENTERTAIN order "
				"to generate funds. The amount of silver gained will "
				"be 20 per man, times the level of the entertainers. "
				"This amount is limited by the region that the unit is in.";
			break;
		case S_TACTICS:
			if(level == 1) {
			*str += "Tactics allows a unit to command soldiers during a "
				"battle. The highest level tactitian on each side in a "
				"battle commands their army, and determines their army's overall "
				"strategy to the best of their ability. At skill level 1 "
				"a tactics leader understands the basics of using foot and "
				"ranged formations in battle. In addition, tactics leaders "
                "may act as officers, enabling them to convey the orders of "
                "the overall battle commander to other soldiers in the "
                "battle. At skill level 1, a soldier may control ten "
                "soldiers, including himself, in normal battle conditions.";
			} else if(level == 2) {
			*str += "With tactics skill level 2, a battle commander may "
			    "make use of cavalry formations in battle, comprised of "
                "soldiers with mounts capable of riding.";
            } else if(level == 3) {/*
			*str += "With tactics skill level 3, a battle commander may "
			    "form his troops into square formation, if necessary "
                "to protect small numbers of behind troops. (Not implemented as of 040120)";*/
            } else if(level == 4) {
			*str += "With tactics skill level 4, a battle commander may "
			    "make use of aerial cavalry formations in battle, comprised of "
                "soldiers with mounts capable of flying.";
            } else if(level == 5) {
            }
            if(level>1 && level<5) *str += AString(" In addition, an officer with tactics level ")
                + level + " may control " + (level*(15 + 5*level) / 2) + 
                " soldiers, including himself, in normal battle conditions.";
            if(level >= 5) *str += AString(" An officer with tactics level ")
                + level + " may control " + (level*(15 + 5*level) / 2) + 
                " soldiers, including himself, in normal battle conditions.";
			break;
			
		case S_COMBAT:
			if(level > 1) break;
			*str += "This skill gives the unit a bonus in hand to hand "
				"combat. Also, a unit with this skill may TAX or PILLAGE.";
			break;
		case S_RIDING:
			if(level > 1) break;
			*str += "A unit with this skill, if possessing a mount, may "
				"gain a bonus in combat, if the battle is in a location "
				"where that mount may be utilized and if the skill of the "
				"rider is sufficient to control that mount. The bonus "
				"gained can vary with the mount, the riders skill, and the "
				"terrain.";
			break;
		case S_CROSSBOW:
			if(level > 1) break;
			*str += "A unit with this skill may use a crossbow or other bow "
				"derived from one, either in battle, or to TAX or PILLAGE a "
				"region.";
			break;
		case S_LONGBOW:
			if(level > 1) break;
			*str += "A unit with this skill may use a longbow or other bow "
				"derived from one, either in battle, or to TAX or PILLAGE a "
				"region.";
			break;
		case S_STEALTH:
			if(level > 1) break;
			*str += "A unit with this skill is concealed from being seen";
			if(SKILL_ENABLED(S_OBSERVATION)) {
				*str += ", except by units with an Observation skill greater "
					"than or equal to the stealthy unit's Stealth level";
			}
			*str += ".";
			break;
		case S_OBSERVATION:
			if(level > 1) break;
			*str += "A unit with this skill can see stealthy units or "
				"monsters whose stealth rating is less than or equal to "
				"the observing unit's Observation level. The unit can "
				"also determine the faction owning a unit, provided its "
				"Observation level is higher than the other unit's Stealth "
				"level.";
			break;
		case S_HEALING:
			if(level > 1) break;
			*str += AString("A unit with this skill is able to attempt to heal ") + 
                Globals->HEALS_PER_MAN + " times his skill "
                "level units hurt in a battle which is won. This skill requires "
                "one herb for every healing attempt made.";
			break;
		case S_SAILING:
			if(level > 1) break;
			*str += "A unit with this skill may use the SAIL order to sail "
				"ships.";
			break;
		case S_FORCE:
			if(level > 1) break;
			*str += "The Force skill is not directly useful to a mage, but "
				"is rather one of the Foundation skills on which other "
				"magical skills are based. The Force skill determines the "
				"power of the magical energy that a mage is able to use. "
				"Note that a Force skill level of 0 does not indicate that "
				"a mage cannot use magical energy, but rather can only "
				"perform magical acts that do not require great amounts of "
				"power.";
			break;
		case S_PATTERN:
			if(level > 1) break;
			*str += "The Pattern skill is not directly useful to a mage, but "
				"is rather one of the Foundation skills on which other "
				"magical skills are based. A mage's Pattern skill indicates "
				"the ability to handle complex magical patterns, and is "
				"important for complicated tasks such as healing and "
				"controlling nature.";
			break;
		case S_SPIRIT:
			if(level > 1) break;
			*str += "The Spirit skill is not directly useful to a mage, but "
				"is rather one of the Foundation skills on which other "
				"magical skills are based. Spirit skill indicates the mage's "
				"ability to control and affect magic and other powers beyond "
				"the material world.";
			break;
		case S_FIRE:
			if(level > 1) break;
			break;
		case S_EARTHQUAKE:
			if(level > 1) break;
			break;
		case S_FORCE_SHIELD:
			if(level > 1) break;
			break;
		case S_ENERGY_SHIELD:
			if(level > 1) break;
			break;
		case S_SPIRIT_SHIELD:
			if(level > 1) break;
			break;
		case S_MAGICAL_HEALING:
			/* XXX -- This should be cleaner somehow. */
			if(level == 1) {
				*str += "This skill enables the mage to magically heal units "
					"after battle. ";
            }
            *str += AString("A mage at this level can heal up to ") + HealDefs[level].num + 
					" casualties, with a " + HealDefs[level].rate + 
                    " percent rate of success. No order "
					"is necessary to use this spell, it will be used "
					"automatically when the mage is involved in battle.";
		    if(Globals->ARCADIA_MAGIC && level == 1 && SkillDefs[skill].combat_cost > 0)
		        *str += AString(" A mage will require one energy for every ")
		            + 120/SkillDefs[skill].combat_cost + " casualties he "
		            "attempts to heal.";
			break;
		case S_GATE_LORE:
			/* XXX -- This should be cleaner somehow. */
			if(level == 1) {
				*str += "Gate Lore is the art of detecting and using magical "
					"Gates, which are spread through the world. The Gates are "
					"numbered in order, but spread out randomly, so there is "
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
					"always jumps through the Gate). If only the mage wishes "
                    "to jump, then the syntax CAST Gate_Lore RANDOM is "
                    "sufficient. ";
                if(!Globals->ARCADIA_MAGIC) {
                    *str += "At level 1, the mage "
					    "may carry 15 weight units through the Gate (including "
					    "the weight of the mage).";
	            } else {
	                *str += "A mage may transport up to 3 "
                        "times his level squared weight units per energy used. The mage will always be "
                        "transported, and his weight is included in the total. "
                        "There is no limit to how many units a mage may "
                        "transport, provided he has sufficient "
                        "energy to do so. For instance, a mage at "
                        "level 1 with 15 energy may transport up to 3*1*1*15 = 45 "
                        "weight units.";
	            }
			} else if (level == 2) {
			    if(!Globals->ARCADIA_MAGIC) {
    				*str += "A mage with Gate Lore skill 2 can detect Gates in "
    					"adjacent regions. The mage should use the syntax CAST "
    					"Gate_Lore DETECT in order to detect these nearby Gates. "
    					"Also, at level 2 Gate Lore, the mage may carry 100 "
    					"weight units through a Gate when doing a random jump.";
				} else {
    				*str += AString("A mage with Gate Lore skill 2 can detect Gates in "
    					"adjacent regions. This spell ") + EnergyString(skill,2,6,1) + 
                        " The mage should use the syntax CAST "
    					"Gate_Lore DETECT in order to detect these nearby Gates.";
				}
			} else if(level == 3) {
			    if(!Globals->ARCADIA_MAGIC) {
    				*str += "A mage with Gate Lore skill 3 and higher can step "
    					"through a Gate into another region containing a specific "
    					"Gate. To use this spell, use the syntax CAST Gate_Lore "
    					"GATE <number> UNITS <unit> ... <number> specifies the "
    					"Gate that the mage will jump to. UNITS is followed by a "
    					"list of units to follow the mage through the gate (the "
    					"mage always jumps through the gate). At level 3, the "
    					"mage may carry 15 weight units through the Gate "
    					"(including the mage). Also, a level 3 or higher mage "
    					"doing a random gate jump may carry 1000 weight units "
    					"through the Gate.";
				} else {
    				*str += "A mage with Gate Lore skill 3 and higher can step "
    					"through a Gate into another region containing a specific "
    					"Gate. To use this spell, use the syntax CAST Gate_Lore "
    					"GATE <number> UNITS <unit> ... <number> specifies the "
    					"Gate that the mage will jump to. UNITS is followed by a "
    					"list of units to follow the mage through the gate (the "
    					"mage always jumps through the gate). If only the mage "
                        "is to jump, then the syntax CAST Gate_Lore GATE <number> "
                        "may be used. A mage may transport up to "
                        "his skill level minus two, squared, times 3 weight units "
                        "per energy used. There is no limit on how many units a mage may "
                        "transport provided he has sufficient energy to transport "
                        "both the units and himself. For instance, a mage "
                        "with level 3 gate lore and 40 energy may transport up to "
                        "1*1*3*40 = 120 weight units, including the weight of the "
                        "mage.";
				}
			} else if(level == 4 && !Globals->ARCADIA_MAGIC) {
				*str += "A mage with Gate Lore skill 4 may carry 100 weight "
					"units through a Gate.";
			} else if(level == 5) {
			    if(!Globals->ARCADIA_MAGIC) {
    				*str += "A mage with Gate Lore skill 5 can detect gates two "
    					"regions away.";
				} else {
    				*str += AString("A mage with Gate Lore skill 5 can detect gates up "
                        "to two regions away. The mage should use the syntax 'CAST "
    					"Gate_Lore DETECT LARGE' in order to detect all gates within "
    					"a distance of two regions or less. This spell ") + EnergyString(skill,2,6,1);
				}
			}
			break;
		case S_PORTAL_LORE:
			if(level > 1) break;
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_PORTAL)) break;
			*str += "A mage with the Portal Lore skill may, with the aid of "
				"another mage, make a temporary Gate between two regions, and "
				"send units from one region to another. In order to do this, "
				"both mages (the caster, and the target mage) must have "
				"Portals, and the caster must be trained in Portal Lore. The "
				"caster may teleport units weighing up to 50 weight units "
				"times his skill level, to the target mage's region. ";
			range = FindRange(SkillDefs[skill].range);
			if(range) {
				*str += " The target region must be within ";
				*str += range->rangeMult;
				switch(range->rangeClass) {
					case RangeType::RNG_LEVEL:
						*str += " times the caster's skill level ";
						break;
					case RangeType::RNG_LEVEL2:
						*str += " times the caster's skill level squared ";
						break;
					case RangeType::RNG_LEVEL3:
						*str += " times the caster's skill level cubed ";
						break;
					default:
					case RangeType::RNG_ABSOLUTE:
						break;
				}
				*str += "regions of the caster. ";
			}
			*str += "To use this skill, CAST Portal_Lore <target> UNITS "
				"<unit> ..., where <target> is the unit number of the "
				"target mage, and <unit> is a list of units to be "
				"teleported (the casting mage may teleport himself, if "
				"he so desires).";
			break;
		case S_FARSIGHT:
			if(level == 1) {
    			*str += "A mage with this skill may obtain region reports on "
    				"distant regions. The report will be as if the mage was in "
    				"the distant region himself.";
                if(Globals->ARCADIA_MAGIC) *str += AString(" This skill") + EnergyString(skill,1,6,1);
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
    					*str += " This skill only works on the surface of the "
    						"world.";
    				}
    				*str += " The target region must be within ";
    				*str += range->rangeMult;
    				switch(range->rangeClass) {
    					case RangeType::RNG_LEVEL:
    						*str += " times the caster's skill level ";
    						break;
    					case RangeType::RNG_LEVEL2:
    						*str += " times the caster's skill level squared ";
    						break;
    					case RangeType::RNG_LEVEL3:
    						*str += " times the caster's skill level cubed ";
    						break;
    					default:
    					case RangeType::RNG_ABSOLUTE:
    						break;
    				}
    				*str += "regions of the caster. ";
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
    					*str += "Coordinates of locations not on the surface are "
    						"scaled to the surface coordinates for this "
    						"calculation. Attempting to view across different "
    						"levels increases the distance by ";
    					*str += range->crossLevelPenalty;
    					*str += " per level difference. ";
    					*str += "To use this skill, CAST Farsight REGION <x> <y> "
    						"<z> where <x>, <y>, and <z> are the coordinates of "
    						"the region that you wish to view. If you omit the "
    						"<z> coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
    					if(Globals->UNDERWORLD_LEVELS +
    							Globals->UNDERDEEP_LEVELS == 1) {
    						*str += " The <z> coordinate for the surface is '1' "
    							"and the <z>-coordinate for the underworld is "
    							"'2'.";
    					}
    					*str += " Note that Farsight cannot be used either into "
    						"or out of the Nexus.";
    				} else {
    					*str += "To use this skill, CAST Farsight REGION <x> "
    						"<y>, where <x> and <y> are the coordinates of the "
    						"region that you wish to view.";
    				}
    			}
    			if(Globals->IMPROVED_FARSIGHT) {
    				*str += " Any other skills which the mage has which give "
    					"region information will be used when farsight is used.";
    			} else {
    				*str += " Note that Farsight does not work in conjunction "
    					"with other skills or spells; the mage can only rely on "
    					"his normal facilities while casting Farsight.";
    			}
			} else if(level == 4 && Globals->ARCADIA_MAGIC) {
    			*str += AString("At level 4, a mage may cast farsight on a region and all six "
                        "surrounding regions at once. This skill ") + EnergyString(skill,4,6,4)
                        + "To use the spell in this fashion, "
                        "CAST Farsight LARGE";
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
                        *str += " REGION <x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the central region around which you wish to "
    						"cast farsight. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
			        } else {
    					*str += " REGION <x> <y>, where <x> and <y> are the "
                            "coordinates of the central region around which you "
                            "wish to cast farsight.";
    				}            
    			    *str += " This spell uses the same range calculation as the "
                            "normal farsight spell, but the caster's effective skill "
                            "level is reduced by 2 for calculating the maximum range";
                }
            }
			break;
		case S_MIND_READING:
			/* XXX -- This should be cleaner somehow. */
			if(level == 1) {
				*str += "A mage with Mind Reading skill 1 may cast the spell "
					"and determine the faction affiliation of any unit he can "
					"see. To use the spell in this manner, CAST Mind_Reading "
					"<unit>, where <unit> is the target unit.";
				if(Globals->ARCADIA_MAGIC) *str += " This spell does not "
                    "require any energy to cast.";	
			} else if ((level == 2 && Globals->ARCADIA_MAGIC) || (level == 3 && !Globals->ARCADIA_MAGIC)) {
				*str += AString("A mage with Mind Reading skill ") + level + " will automatically "
					"determine the faction affiliation of any unit he can "
					"see. Usage of this skill is automatic, and no order is "
					"needed to use it.";
			} else if ((level == 3 && Globals->ARCADIA_MAGIC) || (level == 5 && !Globals->ARCADIA_MAGIC)) {
				*str += AString("A mage with Mind Reading skill ") + level + " can get a full "
					"unit report on any unit he can see. To use this skill, "
					"CAST Mind_Reading <unit> where <unit> is the target "
					"unit.";
			}
			break;
		case S_TELEPORTATION:
			if(level > 1) break;
			/* XXX -- This should be cleaner somehow. */
			*str += "A mage with this skill may teleport himself across"
				"great distances, even without the use of a gate. The mage "
				"may teleport up to 50 weight units per skill level";
			if(Globals->ARCADIA_MAGIC) *str += AString(" squared. For every "
                "50 times his skill level, weight units, that are transported, this spell ") + EnergyString(skill,1,6,1);
			else *str += ".";
			range = FindRange(SkillDefs[skill].range);
			if(range) {
				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
					*str += " This skill only works on the surface of the "
						"world.";
				}
				*str += " The target region must be within ";
				*str += range->rangeMult;
				switch(range->rangeClass) {
					case RangeType::RNG_LEVEL:
						*str += " times the caster's skill level ";
						break;
					case RangeType::RNG_LEVEL2:
						*str += " times the caster's skill level squared ";
						break;
					case RangeType::RNG_LEVEL3:
						*str += " times the caster's skill level cubed ";
						break;
					default:
					case RangeType::RNG_ABSOLUTE:
						break;
				}
				*str += "regions of the caster. ";
				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
					*str += "Coordinates of locations not on the surface are "
						"scaled to the surface coordinates for this "
						"calculation. Attempting to view across different "
						"levels increases the distance by ";
					*str += range->crossLevelPenalty;
					*str += " per level difference. ";
					*str += "To use this skill, CAST Teleportation REGION "
						"<x> <y> <z> where <x>, <y>, and <z> are the "
						"coordinates of the region that you wish to "
						"teleport to. If you omit the <z> coordinate, the "
						"<z> coordinate of the caster's current region will "
						"be used.";
					if(Globals->UNDERWORLD_LEVELS +
							Globals->UNDERDEEP_LEVELS == 1) {
						*str += " The <z> coordinate for the surface is '1' "
							"and the <z>-coordinate for the underworld is "
							"'2'.";
					}
					*str += " Note that Teleportation cannot be used either "
						"into or out of the Nexus.";
				} else {
					*str += "To use this skill, CAST Teleportation REGION "
						"<x> <y>, where <x> and <y> are the coordinates of "
						"the region that you wish to teleport to.";
				}
			}
			break;
		case S_WEATHER_LORE:
			if(level > 1) break;
			/* XXX -- This should be templated */
			*str += "Weather Lore is the magic of the weather; a mage with "
				"this skill can predict the weather in nearby regions. "
				"Weather Lore also allows further study into more powerful "
				"areas of magic. The weather may be predicted for 3 months "
				"at level 1, 6 months at level 3 and a full year at level "
				"5.";
			range = FindRange(SkillDefs[skill].range);
			if(range) {
				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
					*str += " This skill only works on the surface of the "
						"world.";
				}
				*str += " The target region must be within ";
				*str += range->rangeMult;
				switch(range->rangeClass) {
					case RangeType::RNG_LEVEL:
						*str += " times the caster's skill level ";
						break;
					case RangeType::RNG_LEVEL2:
						*str += " times the caster's skill level squared ";
						break;
					case RangeType::RNG_LEVEL3:
						*str += " times the caster's skill level cubed ";
						break;
					default:
					case RangeType::RNG_ABSOLUTE:
						break;
				}
				*str += "regions of the caster. ";
				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
					*str += "Coordinates of locations not on the surface are "
						"scaled to the surface coordinates for this "
						"calculation. Attempting to view across different "
						"levels increases the distance by ";
					*str += range->crossLevelPenalty;
					*str += " per level difference. ";
					*str += "To use this skill, CAST Weather_Lore REGION "
						"<x> <y> <z> where <x>, <y>, and <z> are the "
						"coordinates of the region where you wish to "
						"predict the weather. If you omit the <z> "
						"coordinate, the <z> coordinate of the caster's "
						"current region will be used.";
					if(Globals->UNDERWORLD_LEVELS +
							Globals->UNDERDEEP_LEVELS == 1) {
						*str += " The <z> coordinate for the surface is '1' "
							"and the <z>-coordinate for the underworld is "
							"'2'.";
					}
					*str += " Note that Weather Lore cannot be used either "
						"into or out of the Nexus.";
				} else {
					*str += "To use this skill, CAST Weather_Lore REGION "
						"<x> <y>, where <x> and <y> are the coordinates of "
						"the region where you wish to predict the weather.";
				}
			}
			*str += " A mage with Weather Lore skill will perceive the use "
				"of Weather Lore by any other mage in the same region.";
			break;
		case S_SUMMON_WIND:
			if(Globals->ARCADIA_MAGIC) {
    			if(level == 1) {
    				*str += "A mage with knowledge of Summon Wind can summon "
    					   "up the powers of the wind to aid him in sea or "
    					   "air travel. Usage of this spell is automatic. "
    				       "At level 1, the mage may speed a ";
    	            int found = 0;
    	            AString temp = "";
    	            for(int i=0; i<NOBJECTS; i++) {
    	                if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
    	                if(ObjectDefs[i].capacity > 0 && ObjectDefs[i].capacity < 500) {
    	                    if(found++) {
                                *str += temp;
    	                        *str += ", ";
                            }
    	                    temp = ObjectDefs[i].name;
    	                }	            
    	            }
    	            if(found>1) *str += AString("or ") + temp;
    	            else *str += temp;
    	            *str += ", to travel 20% times his skill level faster (rounded down), "
                           "and the ship will not be slowed by winter or monsoons.";
    	            *str += AString(" However, the act of speeding the ship ") +
                           EnergyString(skill,1,6,1) + 
                           " If the mage does not have this much energy, the ship "
                           "speed will not be affected. Only the highest level mage onboard "
                           "(who is able to cast this spell) will cast this spell.";
    				*str += " Also, if the mage is flying, he will receive 2 extra "
    					   "movement points; this does not cost the mage any energy.";
    			} else if (level == 2) {
    				*str += "With level 2 Summon Wind, a mage may speed the passage "
                           "of a ";
                    int found = 0;
    	            AString temp = "";
    	            for(int i=0; i<NOBJECTS; i++) {
    	                if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
    	                if(ObjectDefs[i].capacity > 499 && ObjectDefs[i].capacity < 1201) {
    	                    if(found++) {
                                *str += temp;
    	                        *str += ", ";
                            }
    	                    temp = ObjectDefs[i].name;
    	                }
    	            }
    	            if(found>1) *str += AString("or ") + temp;
    	            else *str += temp;
    				*str += AString(", to travel 20% times his skill level faster, "
                           "without delay by winter or monsoons. This spell ") + 
    				       EnergyString(skill,2,6,2);
    			} else if (level == 3) {
    				*str += "With level 3 Summon Wind, a mage may speed the passage "
                           "of a ";
                    int found = 0;
    	            AString temp = "";
    	            for(int i=0; i<NOBJECTS; i++) {
    	                if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
    	                if(ObjectDefs[i].capacity > 1200) {
    	                    if(found++) {
                                *str += temp;
    	                        *str += ", ";
                            }
    	                    temp = ObjectDefs[i].name;
    	                }
    	            }
    	            if(found>1) *str += AString("or ") + temp;
    	            else *str += temp;
    				*str += AString(", to travel 20% times his skill level faster, "
                           "without delay by winter or monsoons. This spell ") + 
    				       EnergyString(skill,2,6,3);
    			}
    			break;
			} else {
			if(level == 1) {
				*str += "A mage with knowledge of Summon Wind can summon "
					   "up the powers of the wind to aid him in sea or "
					   "air travel. Usage of this spell is automatic.";
				if(OBJECT_ENABLED(O_LONGBOAT)) {
					*str += " At level 1, if the mage is in a Longboat, that "
						   "ship will get 2 extra movement points.";
				}
				*str += " If the mage is flying, he will receive 2 extra "
					   "movement points.";
			} else if (level == 2) {
				if(OBJECT_DISABLED(O_MERCHANT)) break;
				*str += "With level 2 Summon Wind, any ship of Clipper size "
					   "or smaller that the mage is inside will receive a "
					   "2 movement point bonus.";
			} else if (level == 3) {
				*str += "At level 3 of Summon Wind, any ship the mage is in "
					   "will receive a 2 movement point bonus. Note that "
					   "study of Summon Wind beyond level 3 does not "
					   "yield any further powers.";
			}
			break;
			}
		case S_SUMMON_STORM:
			if(level > 1) break;
			break;
		case S_SUMMON_TORNADO:
			if(level > 1) break;
			break;
		case S_CALL_LIGHTNING:
			if(level > 1) break;
			break;
		case S_CLEAR_SKIES:
			/* XXX -- this range stuff needs cleaning up */
			if(level == 1) {
    			if(SkillDefs[skill].flags & SkillType::CAST) {
    				*str += "When cast using the CAST order, this skill causes the "
    					"region to have good weather for the entire month; "
    					"movement is at the normal rate (even if it is winter) "
    					"and the economic and food production of the region is improved "
    					"for a month (this improvement of the economy will "
    					"actually take effect during the turn after the spell "
    					"is cast).";
   					if(Globals->ARCADIA_MAGIC) {
                        *str += " In addition, this spell counteracts the effects "
                            "of fog both in battle and when CAST on the same region, "
                            "by reducing the effectiveness of the fog spell by the "
                            "skill of the mage casting clear skies.";
                        *str += AString(" This spell ") + EnergyString(skill,1,6,1);
                    }
    				range = FindRange(SkillDefs[skill].range);
    				if(range) {
    					if(range->flags & RangeType::RNG_SURFACE_ONLY) {
    						*str += " This skill only works on the surface of "
    							"the world.";
    					}
    					*str += " The target region must be within ";
    					*str += range->rangeMult;
    					switch(range->rangeClass) {
    						case RangeType::RNG_LEVEL:
    							*str += " times the caster's skill level ";
    							break;
    						case RangeType::RNG_LEVEL2:
    							*str += " times the caster's skill level squared ";
    							break;
    						case RangeType::RNG_LEVEL3:
    							*str += " times the caster's skill level cubed ";
    							break;
    						default:
    						case RangeType::RNG_ABSOLUTE:
    							break;
    					}
    					*str += "regions of the caster. ";
    					if(range->flags & RangeType::RNG_CROSS_LEVELS) {
    						*str += "Coordinates of locations not on the surface "
    							"are scaled to the surface coordinates for this "
    							"calculation. Attempting to view across "
    							"different levels increases the distance by ";
    						*str += range->crossLevelPenalty;
    						*str += " per level difference. ";
    						*str += "To use this skill, CAST Clear_Skies REGION "
    							"<x> <y> <z> where <x>, <y>, and <z> are the "
    							"coordinates of the region where you wish to "
    							"improve the weather. If you omit the <z> "
    							"coordinate, the <z> coordinate of the caster's "
    							"current region will be used.";
    						if(Globals->UNDERWORLD_LEVELS +
    								Globals->UNDERDEEP_LEVELS == 1) {
    							*str += " The <z> coordinate for the surface is "
    								"'1' and the <z>-coordinate for the "
    								"underworld is '2'.";
    						}
    						*str += " Note that Clear Skies cannot be used "
    							"either into or out of the Nexus.";
    					} else {
    						*str += "To use this skill, CAST Clear_Skies REGION "
    							"<x> <y>, where <x> and <y> are the coordinates "
    							"of the region where you wish to improve the "
    							"weather.";
    					}
    				}	
				} else {
					*str += " To use the spell in this fashion, CAST "
						"Clear_Skies; no arguments are necessary.";
				}
			} else if(level == 4 && Globals->ARCADIA_MAGIC) {
    			*str += AString("At level 4, a mage may cast clear skies on a region and all six "
                        "surrounding regions at once. This spell ")
                        + EnergyString(skill,4,6,4) + 
                        " To use the spell in this fashion, "
                        "CAST Clear_Skies LARGE";
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
                        *str += " REGION <x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the central region around which you wish to "
    						"cast clear skies. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
			        } else {
    					*str += " REGION <x> <y>, where <x> and <y> are the "
                            "coordinates of the central region around which you "
                            "wish to cast clear skies.";
    				}            
    			    *str += " This spell uses the same range calculation as the "
                            "normal clear skies spell, but the caster's effective skill "
                            "level is reduced by 2 for calculating the maximum range";
                }
                *str += ".";
            }
			break;
		case S_EARTH_LORE:
			if(level == 1) {
    			*str += "Earth Lore is the study of nature, plants, and animals. "
    				"A mage with knowledge of Earth Lore can use his knowledge "
    				"of nature to aid local farmers, raising money for himself, "
    				"and aiding the production of grain or livestock in the "
    				"region. This increased production will take place the turn "
                    "after the spell is cast. Also, a mage with knowledge of Earth "
    				"Lore will detect the use of Earth Lore by any other mage in "
    				"the same region.";
				range = FindRange(SkillDefs[skill].range);
				if(range) {
					if(range->flags & RangeType::RNG_SURFACE_ONLY) {
						*str += " This skill only works on the surface of "
							"the world.";
					}
					*str += " The target region must be within ";
					*str += range->rangeMult;
					switch(range->rangeClass) {
						case RangeType::RNG_LEVEL:
							*str += " times the caster's skill level ";
							break;
						case RangeType::RNG_LEVEL2:
							*str += " times the caster's skill level squared ";
							break;
						case RangeType::RNG_LEVEL3:
							*str += " times the caster's skill level cubed ";
							break;
						default:
						case RangeType::RNG_ABSOLUTE:
							break;
					}
					*str += "regions of the caster. ";
					if(range->flags & RangeType::RNG_CROSS_LEVELS) {
						*str += "Coordinates of locations not on the surface "
							"are scaled to the surface coordinates for this "
							"calculation. Attempting to view across "
							"different levels increases the distance by ";
						*str += range->crossLevelPenalty;
						*str += " per level difference. ";
						*str += "To use this skill, CAST Earth_Lore REGION "
							"<x> <y> <z> where <x>, <y>, and <z> are the "
							"coordinates of the region where you wish to "
							"improve the weather. If you omit the <z> "
							"coordinate, the <z> coordinate of the caster's "
							"current region will be used.";
						if(Globals->UNDERWORLD_LEVELS +
								Globals->UNDERDEEP_LEVELS == 1) {
							*str += " The <z> coordinate for the surface is "
								"'1' and the <z>-coordinate for the "
								"underworld is '2'.";
						}
						*str += " Note that Earth Lore cannot be used "
							"either into or out of the Nexus.";
					} else {
						*str += "To use this skill, CAST Earth_Lore REGION "
							"<x> <y>, where <x> and <y> are the coordinates "
							"of the region where you wish to improve the "
							"weather.";
					}
				} else {
					*str += " To use this skill, CAST Earth_Lore.";
				}
				if(Globals->ARCADIA_MAGIC) {
				    *str += AString(" This spell ") + EnergyString(skill,1,6,1);
                }
			} else if(level == 4) {
    			*str += "At level 4, a mage may cast earth lore on a region and all six "
                        "surrounding regions at once. To use the spell in this fashion, "
                        "CAST Earth_Lore Large";
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
                        *str += " REGION <x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the central region around which you wish to "
    						"cast earth lore. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
			        } else {
    					*str += " REGION <x> <y>, where <x> and <y> are the "
                            "coordinates of the central region around which you "
                            "wish to cast earth lore.";
    				}            
    			    *str += " This spell uses the same range calculation as the "
                            "normal earth lore spell, but the caster's effective skill "
                            "level is reduced by 2 for calculating the maximum range";
                }
				if(Globals->ARCADIA_MAGIC) {
				    *str += AString(" This spell ") + EnergyString(skill,4,6,4);
                }
            }
			break;
		case S_WOLF_LORE:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_WOLF)) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Wolf Lore skill may summon wolves, who will "
    				"fight for him in combat. A mage may summon a number of "
    				"wolves equal to his skill level, and control a total number "
    				"of his skill level squared times 4 wolves; the wolves will "
    				"be placed in the mages inventory. Note, however, that wolves "
    				"may only be summoned in mountain and forest regions. To "
    				"summon wolves, the mage should issue the order CAST "
    				"Wolf_Lore.";
			} else {
    			*str += "A hero with the Wolf Lore skill may tame wolves, who will "
    				"fight for him in combat. A hero may tame a number of "
    				"wolves each month equal to his skill level, and control a total number "
    				"of his skill level squared times 4 wolves; the wolves will "
    				"be placed in the mages inventory. Note, however, that wolves "
    				"may only be summoned in mountain and forest regions. To "
    				"summon wolves, the mage should issue the order CAST "
    				"Wolf_Lore.";
			}	
			break;
		case S_BIRD_LORE:
			/* XXX -- This should be cleaner somehow. */
			if(level == 1) {
				*str += "A hero with Bird Lore learns to control the birds of the "
					"sky. At skill level 1, the hero can tame small "
					"birds, sending them to nearby regions to obtain "
					"reports on those regions. (This skill only works on the "
					"surface of the world, as there are no birds elsewhere). "
					"To use this skill, CAST Bird_Lore DIRECTION <dir>, "
					"where <dir> is the direction the hero wishes the birds "
					"to report on. The hero will recieve reports on the "
                    "the first n regions in that direction, where n is the "
                    "hero's bird lore skill level.";
			} else if (level == 3) {
				if(ITEM_DISABLED(I_EAGLE)) break;
				if(!Globals->ARCADIA_MAGIC) {
    				*str += "A mage with Bird Lore 3 can summon eagles to join "
    					"him, who will aid him in combat, and provide for flying "
    					"transportation. A mage may summon a number of eagles "
    					"equal to his skill level minus 2, squared; the eagles "
    					"will appear in his inventory. To summon an eagle, issue "
    					"the order CAST Bird_Lore EAGLE.";
    			} else {
        			*str += "A hero with Bird Lore 3 can tame eagles "
        				"to join him or her, which will aid in combat and provide "
                        "flying transportation. A hero may tame a total "
                        "number of eagles equal to their skill level minus 2, squared. "
                        " To use this skill, "
        				"the hero should issue the order CAST Bird_Lore EAGLE, "
                        "and the hero will tame an extra eagle in the coming month.";
    			}	
			}/* else if (level == 4) {
				if(Globals->ARCADIA_MAGIC) {
    				*str += AString("A mage with Bird Lore 4 can send small birds to spy "
    					"on all neighbouring regions at once. This skill ")
                         + EnergyString(skill,4,6,4) +
                        " To use this skill in this fashion, CAST Bird_Lore LARGE.";
    			}
			}*/
			break;
		case S_DRAGON_LORE:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_DRAGON)) break;
			*str += "A mage with Dragon Lore skill can summon dragons to "
				"join him, to aid in battle, and provide flying "
				"transportation. A mage at level 1 has a low chance of "
				"successfully summoning a dragon, gradually increasing until "
				"at level 5 he may summon one dragon per turn; the total "
				"number of dragons that a mage may control at one time is "
				"equal to his skill level. To attempt to summon a dragon, "
				"CAST Dragon_Lore.";
			break;
		case S_NECROMANCY:
			if(level > 1) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "Necromancy is the magic of death; a mage versed in "
    				"Necromancy may raise and control the dead, and turn the "
    				"powers of death to his own nefarious purposes. The "
    				"Necromancy skill does not have any direct application, but "
    				"is required to study the more powerful Necromantic skills. "
    				"A mage with knowledge of Necromancy will detect the use of "
    				"Necromancy by any other mage in the same region.";
			} else {
			   *str += "Necromancy is the magic of death; a mage versed in "
                    "Necromancy may raise and control the dead, and turn the "
                    "powers of death to his own nefarious purposes. This skill "
                    "enables the mage to create skeletons from units which have "
                    "died in battle. This spell will be used automatically by "
                    "a mage on the winning side of a battle, and will attempt to "
                    "turn the battle corpses into "
                    "skeletons, with a base success rate of 30%, plus 10% times "
                    "the mage's skill level.";
               if(SkillDefs[skill].combat_cost > 0) *str += AString(" For every ") + 120/SkillDefs[skill].combat_cost + 
                    " corpses which this spell is cast on, a mage will "
                    "lose 1 energy. Note that skeletons, and all other undead, "
                    " have a 6.7% chance of decaying each month.";
               *str + " A mage with knowledge of Necromancy will also detect the use of "
    				"necromancy skills by any other mage in the same region.";
			}
			break;
		case S_SUMMON_SKELETONS:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_SKELETON)) break;
			*str += "A mage with the Summon Skeletons skill may summon "
				"skeletons into his inventory, to aid him in battle. "
				"Skeletons may be given to other units, as they follow "
				"instructions mindlessly; however, they have a 10 percent "
				"chance of decaying each turn. A mage can summon skeletons "
				"at an average rate of 40 percent times his level squared. "
				"To use the spell, use the order CAST Summon_Skeletons, "
				"and the mage will summon as many skeletons as he is able.";
			break;
		case S_RAISE_UNDEAD:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_UNDEAD)) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Raise Undead skill may summon undead "
    				"into his inventory, to aid him in battle. Undead may be "
    				"given to other units, as they follow instructions "
    				"mindlessly; however, they have a 10 percent chance of "
    				"decaying each turn. A mage can summon undead at an average "
    				"rate of 10 percent times his level squared. To use the "
    				"spell, use the order CAST Raise_Undead and the mage will "
    				"summon as many undead as he is able.";
			} else {
    			*str += AString("A mage with the Raise Undead skill may summon undead "
    				"into his inventory, to aid him in combat. Undead may be given "
    				"to other units, as they follow instructions "
    				"mindlessly; however, they have a 6.7% chance of "
    				"decaying each turn. "
    				"For every ten undead summoned, this spell ")
                    + EnergyString(skill,1,6,1) + 
                    " To use this spell, "
    				"the mage should issue the order CAST Raise_Undead <num>, "
                    "where <num> is the number of undead the mage wishes to "
                    "summon. If no number is specified, then the mage will "
                    "summon as many undead as he is able.";
			}
			break;
		case S_SUMMON_LICH:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_LICH)) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Summon Lich skill may summon a lich "
    				"into his inventory, to aid him in battle. Liches may be "
    				"given to other units, as they follow instructions "
    				"mindlessly; however, they have a 10 percent chance of "
    				"decaying each turn. A mage has a 2 percent times his level "
    				"squared chance of summoning a lich; to summon a lich, use "
    				"the order CAST Summon_Lich.";
			} else {
    			*str += AString("A mage with the Summon Lich skill may summon liches "
    				"into his inventory, to aid him in battle. A mage may summon "
                    "one lich per month. Each lich "
                    "has a 6.7% chance of decaying each month. "
                    " This spell ") + EnergyString(skill,1,6,1) + 
                    "To use this spell, the mage should issue the order CAST Summon_Lich.";
			}
			break;
		case S_CREATE_AURA_OF_FEAR:
			if(level > 1) break;
			break;
		case S_SUMMON_BLACK_WIND:
			if(level > 1) break;
			break;
		case S_BANISH_UNDEAD:
			if(level > 1) break;
			break;
		case S_DEMON_LORE:
			if(level > 1) break;
			*str += "Demon Lore is the art of summoning and controlling "
				"demons. The Demon Lore skill does not give the mage any "
				"direct skills, but is required to study further into the "
				"Demonic arts. A mage with knowledge of Demon Lore will "
				"detect the use of Demon Lore by any other mage in the same "
				"region.";
			break;
		case S_SUMMON_IMPS:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_IMP)) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Summon Imps skill may summon imps into "
    				"his inventory, to aid him in combat. A mage may summon one "
    				"imp per skill level; however, the imps have a chance of "
    				"breaking free of the mage's control at the end of each "
    				"turn. This chance is based on the number of imps in the "
    				"mage's control; if the mage has his skill level squared "
    				"times 4 imps, the chance is 5 percent; this chance goes "
    				"up or down quickly if the mage controls more or fewer imps. "
    				"To use this spell, the mage should issue the order CAST "
    				"Summon_Imps, and the mage will summon as many imps as he "
    				"is able.";
			} else {
    			*str += AString("A mage with the Demon Lore skill can summon imps "
    				"into his inventory, to aid him in combat. Imps have a chance of "
    				"breaking free of the mage's control at the end of each "
    				"turn. This chance is based on the number of imps in the "
    				"mage's control; if the mage has his skill level squared "
    				"times 4 imps, the chance is 5 percent; this chance goes "
    				"up or down quickly if the mage controls more or fewer imps. "
    				"For every ten imps summoned, this spell ")
                    + EnergyString(skill,1,6,1) + " In addition, "
                    "the mage will spend energy every turn maintaining control "
                    "over the imps in his possession. This maintenance "
                    "cost is one energy per ten imps at level 1 for a mage, "
                    "decreases with increasing skill level. "
                    "To use this spell, "
    				"the mage should issue the order CAST Demon_Lore <num>, "
                    "where <num> is the number of imps the mage wishes to "
                    "summon. If no number is specified, then the mage will "
                    "summon as many imps as he is able.";
			}
			break;
		case S_SUMMON_DEMON:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_DEMON)) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Summon Demon skill may summon demons "
    				"into his inventory, to aid him in combat. A mage may summon "
    				"one demon each turn; however, the demons have a chance of "
    				"breaking free of the mage's control at the end of each "
    				"turn. This chance is based on the number of demons in the "
    				"mage's control; if the mage has a number of demons equal "
    				"to his skill level squared, the chance is 5 percent; this "
    				"chance goes up or down quickly if the mage controls more or "
    				"fewer demons. To use this spell, the mage should issue the "
    				"order CAST Summon_Demon.";
			} else {
    			*str += AString("A mage with the Summon Demon skill can summon demons "
    				"into his inventory, to aid him in combat. Demons have a chance of "
    				"breaking free of the mage's control at the end of each "
    				"turn. This chance is based on the number of demons in the "
    				"mage's control; if the mage has his skill level squared "
    				"demons, the chance is 5 percent; this chance goes "
    				"up or down quickly if the mage controls more or fewer demons. "
    				"For every ten demons summoned, this spell ")
                    + EnergyString(skill,1,6,1) + " In addition, "
                    "the mage will spend energy every turn maintaining control "
                    "over the demons in his possession. This maintenance "
                    "cost is one energy per demon at level 1, and "
                    "decreases with increasing skill level. "
                    "To use this spell, "
    				"the mage should issue the order CAST Summon_Demon <num>, "
                    "where <num> is the number of demons the mage wishes to "
                    "summon. If no number is specified, then the mage will "
                    "summon as many demons as he is able.";
			}
			break;
		case S_SUMMON_BALROG:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_BALROG)) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Summon Balrog skill may summon a balrog "
    				"into his inventory, to aid him in combat. A mage has a 20 "
    				"percent times his skill level chance of summoning a balrog, "
    				"but may only summon a balrog if one is not already under "
    				"his control. As with other demons, the balrog has a chance "
    				"of breaking free of the mage's control at the end of each "
    				"turn. This chance is equal to 1 over 4 times the mage's "
    				"skill level to the fourth power (or, from 1 over 4 at "
    				"level 1, to 1 over 2500 at level 5). To use this spell, "
    				"the mage should issue the order CAST Summon_Balrog.";
			} else {	
    			*str += AString("A mage with the Summon Balrog skill may summon balrogs "
    				"into his inventory, to aid him in battle. A mage may summon "
                    "one balrog per month, and have up to his skill level divided "
                    "by 2, rounded up, balrogs in his inventory at once. As with "
                    "other demons, balrogs have a chance "
    				"of breaking free of the mage's control at the end of each "
    				"turn. This chance is equal to the number of balrogs, squared, "
                    "over 10 times the mage's skill level squared (or, from 1 "
                    "over 10 with one balrog at level 1, to 9 over 360 with three "
                    "balrogs at level 6). This spell ")
                    + EnergyString(skill,1,6,1) + " If the mage already has a balrog "
                    "under his control, then the mage's skill level will be reduced by "
                    "2 per balrog in energy calculations (thus, summoning a second balrog "
                    "at level 3 costs the same as the first balrog at level 1).In addition, "
                    "the mage will spend energy every turn maintaining control "
                    "over the balrogs in his possession. This maintenance "
                    "cost is seven energy per balrog at level 1, and "
                    "decreases with increasing skill level. "
                    "To use this spell, "
    				"the mage should issue the order CAST Summon_Balrog.";
			}	
			break;
		case S_BANISH_DEMONS:
			if(level > 1) break;
			break;
		case S_ILLUSION:
			if(level > 1) break;
			*str += "Illusion is the magic of creating images of things that "
				"do not actually exist. The Illusion skill does not have any "
				"direct applications, but is required for further study of "
				"Illusionary magic. A mage with knowledge of the Illusion "
				"skill will detect the use of Illusion by any other mage in "
				"the same region.";
			break;
		case S_PHANTASMAL_ENTERTAINMENT:
			/* XXX -- This should be cleaner somehow */
			if(level > 1) break;
			if(Globals->ARCADIA_MAGIC) {
    			*str += "Gladiators are trained to use their battle skills "
                    "for entertainment, and the best gladiators can earn "
                    "far more money than regular entertainers. The "
                    "Gladiator skill effectively grants a hero an "
                    "entertainment bonus equal to ten times his Gladiator "
                    "skill level. To use this skill, use the ENTERTAIN "
                    "order.";
			} else {
    			*str += "A mage with the Phantasmal Entertainment skill may use "
    				"his powers of Illusion to earn money by creating "
    				"illusionary fireworks, puppet shows, etc. In effect, "
    				"Phantasmal Entertainment grants the mage Entertainment "
    				"skill equal to five times his Phantasmal Entertainment "
    				"level. To use this skill, use the ENTERTAIN order.";
			}
			break;
		case S_CREATE_PHANTASMAL_BEASTS:
			/* XXX -- This should be cleaner somehow. */
			if(level == 1) {
				*str += "A mage with Create Phantasmal Beasts may summon "
					"illusionary beasts that appear in the mage's inventory. "
					"These beasts will fight in combat, but do not attack, "
					"and are killed whenever they are attacked.";
				if(ITEM_ENABLED(I_IWOLF)) {
					*str += " Create Phantasmal Beasts at level 1 allows the "
						"mage to summon illusionary wolves; the number the "
						"mage can summon, or have in his inventory at one "
						"time is equal to the mage's skill squared times 4. "
						"To use this spell, the mage should CAST "
						"Create_Phantasmal_Beasts WOLF <number>, where "
						"<number> is the number of wolves that the mage "
						"wishes to have appear in his inventory.";
				}
				*str += " Note: illusionary beasts will appear on reports as "
					"if they were normal items, except on the owner's "
					"report, where they are marked as illusionary. To "
					"reference these items in orders, you must prepend an "
					"'i' to the normal string. (For example: to reference "
					"an illusionary wolf, you would use 'iwolf').";
			} else if (level == 3) {
				if(ITEM_DISABLED(I_IEAGLE)) break;
				*str += "Create Phantasmal Beasts at level 3 allows the mage "
					"to summon illusionary eagles into his inventory. To "
					"summon illusionary eagles, the mage should CAST "
					"Create_Phantasmal_Beasts EAGLE <number>, where <number> "
					"is the number of eagles that the mage wishes to have "
					"appear in his inventory. The number of eagles that a "
					"mage may have in his inventory is equal to his skill "
					"level, minus 2, squared.";
			} else if(level == 5) {
				if(ITEM_DISABLED(I_IDRAGON)) break;
				*str += "Create Phantasmal Beasts at level 5 allows the "
					"mage to summon an illusionary dragon into his "
					"inventory. To summon an illusionary dragon, the mage "
					"should CAST Create_Phantasmal_Beasts DRAGON; the mage "
					"can only have one illusionary dragon in his inventory "
					"at one time.";
			}
			break;
		case S_CREATE_PHANTASMAL_UNDEAD:
			/* XXX -- This should be cleaner somehow. */
			if(level == 1) {
				*str += "A mage with Create Phantasmal Undead may summon "
					"illusionary undead that appear in the mage's inventory. "
					"These undead will fight in combat, but do not attack, "
					"and are killed whenever they are attacked.";
				if(ITEM_ENABLED(I_ISKELETON)) {
					*str += " Create Phantasmal Undead at level 1 allows the "
						"mage to summon illusionary skeletons; the number "
						"the mage can summon, or have in his inventory at "
						"one time is equal to the mage's skill squared times "
						"4. To use this spell, the mage should CAST "
						"Create_Phantasmal_Undead SKELETON <number>, where "
						"<number> is the number of skeletons that the mage "
						"wishes to have appear in his inventory.";
				}
				*str += " Note: illusionary undead will appear on reports as "
					"if they were normal items, except on the owner's "
					"report, where they are marked as illusionary. To "
					"reference these items in orders, you must prepend an "
					"'i' to the normal string. (Example: to reference an "
					"illusionary skeleton, you would use 'iskeleton').";
			} else if(level == 3) {
				if(ITEM_DISABLED(I_IUNDEAD)) break;
				*str += "Create Phantasmal Undead at level 3 allows the mage "
					"to summon illusionary undead into his inventory. To "
					"summon illusionary undead, the mage should CAST "
					"Create_Phantasmal_Undead UNDEAD <number>, where <number> "
					"is the number of undead that the mage wishes to have "
					"appear in his inventory. The number of undead that a "
					"mage may have in his inventory is equal to his skill "
					"level, minus 2, squared.";
			} else if (level == 5) {
				if(ITEM_DISABLED(I_ILICH)) break;
				*str += "Create Phantasmal Undead at level 5 allows the mage "
					"to summon an illusionary lich into his inventory. To "
					"summon an illusionary lich, the mage should CAST "
					"Create_Phantasmal_Undead LICH; the mage can only have "
					"one illusionary lich in his inventory at one time.";
			}
			break;
		case S_CREATE_PHANTASMAL_DEMONS:
			/* XXX -- This should be cleaner somehow. */
			if(level == 1) {
				*str += "A mage with Create Phantasmal Demons may summon "
					    "illusionary demons that appear in the mage's "
						"inventory. These demons will fight in combat, but "
						"do not attack, and are killed whenever they are "
						"attacked.";
				if(ITEM_ENABLED(I_IIMP)) {
					*str += " Create Phantasmal Demons at level 1 allows the "
						"mage to summon illusionary imps; the number the "
						"mage can summon, or have in his inventory at one "
						"time is equal to the mage's skill squared times 4. "
						"To use this spell, the mage should CAST "
						"Create_Phantasmal_Demons IMP <number>, where "
						"<number> is the number of imps that the mage wishes "
						"to have appear in his inventory.";
				}
				*str += " Note: illusionary demons will appear on reports as "
					"if they were normal items, except on the owner's "
					"report, where they are marked as illusionary. To "
					"reference these items in orders, you must prepend an "
					"'i' to the normal string. (Example: to reference an "
					"illusionary imp, you would use 'iimp').";
			} else if (level == 3) {
				if(ITEM_DISABLED(I_IDEMON)) break;
				*str += "Create Phantasmal Demons at level 3 allows the mage "
					"to summon illusionary demons into his inventory. To "
					"summon illusionary demons, the mage should CAST "
					"Create_Phantasmal_Demons DEMON <number>, where <number> "
					"is the number of demons that the mage wishes to have "
					"appear in his inventory. The number of demons that a "
					"mage may have in his inventory is equal to his skill "
					"level, minus 2, squared.";
			} else if (level == 5) {
				if(ITEM_DISABLED(I_IBALROG)) break;
				*str += "Create Phantasmal Demons at level 5 allows the mage "
					"to summon an illusionary balrog into his inventory. To "
					"summon an illusionary balrog, the mage should CAST "
					"Create_Phantasmal_Demons BALROG; the mage can only have "
					"one illusionary balrog in his inventory at one time.";
			}
			break;
		case S_INVISIBILITY:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			*str += "The Invisibility skill allows a mage to render other "
				"units nearly invisible to other factions, giving them a +";
            if(Globals->ARCADIA_MAGIC) *str += AString(2);
            else *str += AString(3);
			*str += " bonus to Stealth. This invisibility will last until the "
				"next Magic round. To cast this spell, use the order CAST "
				"Invisibility UNITS <unit> ..., where <unit> is a list of "
				"the units that the mage wishes to render invisible. A mage "
				"may render invisible a number of men or creatures up to "
				"his skill level squared.";
			if(Globals->ARCADIA_MAGIC) 
			    *str += AString(" For every (skill level) units that the mage casts "
			        "invisibility on, this spell ") + EnergyString(skill,1,6,1) +
                    " In addition, a mage with invisibility will gain a bonus "
                    "to his Stealth skill equal to his Invisibility skill "
                    "divided by 2, rounded up (ie 1 at level 1 up to 3 at level 6).";
			break;
		case S_TRUE_SEEING:
			if(level > 1) break;
			*str += "A mage with the True Seeing spell can see illusions "
				"for what they really are. Whether or not the mage can see "
				"through the illusion depends on his True Seeing skill "
				"being higher that the Illusion skill of the mage casting "
				"the illusion. This spell does not require any order to "
				"use; it is used automatically.";
			if(SKILL_ENABLED(S_OBSERVATION)) {
				*str += "In addition, a mage with the True Seeing skill "
					"receives a bonus to his Observation skill equal to his "
					"True Seeing skill divided by 2, rounded up.";
			}
			break;
		case S_DISPEL_ILLUSIONS:
			if(level > 1) break;
			break;
		case S_ARTIFACT_LORE:
			if(level == 1) {
    			*str += "Artifact Lore is one of the most advanced forms of "
    				"magic; in general, creation of an artifact requires both "
    				"Artifact Lore, and the appropriate skill for the item being "
    				"created. A mage with knowledge of the Artifact Lore skill "
    				"will detect the use of Artifact Lore by any other mage in "
    				"the region.";
    			if(Globals->ARCADIA_MAGIC) {
        			*str += AString(" In addition, a mage at level 1 may create amulets of "
                        "protection, which grant the possessor a "
        				"personal Spirit Shield of 3. A mage may create up to his skill "
        				"level squared of these amulets per turn. For every skill "
                        "level amulets created, this spell ") + EnergyString(skill,1,6,1) + 
                        " To use this spell, CAST Artifact_Lore Protection, and "
                        "the mage will create as many amulets of protection as he "
                        "is able.";
    			}
			} else if(level == 3) {
			    if(Globals->ARCADIA_MAGIC) {
        			*str += AString("A mage with artifact lore at level 3 may create "
                        "shieldstones, which grant the possessor a "
        				"personal Energy Shield of 3. A mage may create up to his skill "
        				"level squared of shieldstones per turn. For every skill "
                        "level shieldstones created, this spell ") + EnergyString(skill,3,6,1) +
                        " To use this spell, CAST Artifact_Lore Shieldstone, and "
                        "the mage will create as many shieldstones as he "
                        "is able.";
    			}
            }
			break;
		case S_CREATE_RING_OF_INVISIBILITY:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_RINGOFI)) break;
			if(level > 1) break;
			*str += "A mage with the Create Ring of Invisibility skill may "
				"create a Ring of Invisibility, which grants a 3 bonus to a "
				"unit's effective Stealth (note that a unit must possess "
				"one ring for each man to gain this bonus).";
			if(ITEM_ENABLED(I_AMULETOFTS)) {
				*str += " A Ring of Invisibility has one limitation; a "
					"unit possessing a ring cannot assassinate, nor steal "
					"from, a unit with an Amulet of True Seeing.";
			}
			if(!Globals->ARCADIA_MAGIC) {    
    			*str += " A mage has a 20 percent times his level chance to "
    				"create a Ring of Invisibility. To use this spell, the mage "
    				"should CAST Create_Ring_of_Invisibility.";
			} else {
    			*str += AString(" This skill ") + 
                    EnergyString(skill,1,6,1) + " To use this "
                    "spell, CAST Create_Ring_of_Invisibility.";
			}
			break;
		case S_CREATE_CLOAK_OF_INVULNERABILITY:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_CLOAKOFI)) break;
			if(level > 1) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Create Cloak of Invulnerability skill "
    				"may create a Cloak of Invulnerability. A mage has a 20 "
    				"percent times his level chance to create a Cloak of "
    				"Invulnerability. To use this spell, the mage should CAST "
    				"Create_Cloak_of_Invulnerability.";
			} else {
    			*str += AString("A mage with the Create Cloak of Invulnerability skill "
    				"may create a Cloak of Invulnerability. This "
                    "skill ") + EnergyString(skill,1,6,1) + " To use this "
                    "spell, CAST Create_Cloak_of_Invulnerability.";
			}
			break;
		case S_CREATE_STAFF_OF_FIRE:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_STAFFOFF)) break;
			if(level > 1) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Create Staff of Fire skill may create a "
    				"Staff of Fire. A Staff of Fire allows any mage to throw "
    				"fireballs in combat as if he had a Fire skill of 3. A mage "
    				"has a 20 percent times his level chance to create a Staff "
    				"of Fire. To use this spell, CAST Create_Staff_of_Fire.";
			} else {
    			*str += AString("A mage with the Create Staff of Fire skill may create a "
    				"Staff of Fire. A Staff of Fire allows any mage to throw "
    				"fireballs in combat as if he had a Fire skill of 3. This "
                    "skill ") + EnergyString(skill,1,6,1) + " To use this "
                    "spell, CAST Create_Staff_of_Fire.";
			}
			break;
		case S_CREATE_STAFF_OF_LIGHTNING:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_STAFFOFL)) break;
			if(level > 1) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Create Staff of Lightning skill may "
    				"create a Staff of Lightning. A Staff of Lightning allows "
    				"any mage to call down lightning bolts as if he had Call "
    				"Lightning skill of 3. A mage has a 20 percent times his "
    				"level chance to create a Staff of Lightning. To use this "
    				"spell, CAST Create_Staff_of_Lightning.";
			} else {
    			*str += AString("A mage with the Create Staff of Lightning skill may "
    				"create a Staff of Lightning. A Staff of Lightning allows "
    				"any mage to call down lightning bolts as if he had Call "
    				"Lightning skill of 3. This skill ") + 
                    EnergyString(skill,1,6,1) + " To use this "
                    "spell, CAST Create_Staff_of_Lightning.";
			}
			break;
		case S_CREATE_AMULET_OF_TRUE_SEEING:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_AMULETOFTS)) break;
			if(level > 1) break;
			*str += "A mage with the Create Amulet of Tree Seeing skill may "
				"create an Amulet of True Seeing. This amulet gives the "
				"possessor a bonus of 2 to his effective Observation skill.";
			if(ITEM_ENABLED(I_RINGOFI)) {
				*str += "Also, a unit with an Amulet of True Seeing cannot "
					"be assassinated by, nor have items stolen by, a unit "
					"with a Ring of Invisibility (Note that the unit must "
					"have at least one Amulet of True Seeing per man to "
					"repel a unit with a Ring of Invisibility).";
			}
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage has a 20 percent times his skill level chance to "
    			   "create an Amulet of True Seeing. To use this spell, CAST "
    			   "Create_Amulet_of_True_Seeing.";
			} else {
    			*str += AString(" This skill ") + 
                    EnergyString(skill,1,6,1) + " To use this "
                    "spell, CAST Create_Amulet_of_True_Seeing.";
			}
			break;
		case S_CREATE_AMULET_OF_PROTECTION:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_AMULETOFP)) break;
			if(level > 1) break;
			*str += "A mage with the Create Amulet of Protection skill may "
				"create Amulets of Protection, which grants the possesor a "
				"personal Spirit Shield of 3. A mage may create his skill "
				"level of these amulets per turn. To use this spell, CAST "
				"Create_Amulet_of_Protection.";
			break;
		case S_CREATE_RUNESWORD:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_RUNESWORD)) break;
			if(level > 1) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Create Runesword skill may create a "
    				"Runesword, which when wielded in combat gives the wielder "
    				"a plus 5 bonus to Combat skill, and also allows the wielder "
    				"to project an Aura of Fear in battle, as if he had Create "
    				"Aura of Fear skill of level 2 (provided the wielder is "
    				"not casting any other combat spells). A mage has a 20 "
    				"percent times his skill level chance of creating a "
    				"Runesword. To cast this spell, CAST Create_Runesword.";
			} else {
			    WeaponType *pW = FindWeapon("RUNE");
		        BattleItemType *bt = FindBattleItem("RUNE");
    			*str += AString("A mage with the Create Runesword skill may create a "
    				"Runesword, which when wielded in combat ");
                if(pW) *str += AString("gives the wielder a plus ") + pW->attackBonus + 
                    " bonus to Combat skill";
                if(pW && bt) *str += ", and also ";
                if(bt) *str += AString("allows the wielder "
    				"to project an Aura of Fear in battle, as if he had Create "
    				"Aura of Fear skill of level ") + bt->skillLevel + " (provided the wielder is "
    				"not casting any other combat spells). This skill " + 
                    EnergyString(skill,1,6,1) + " To use this "
                    "spell, CAST Create_Runesword.";
			}
			break;
		case S_CREATE_SHIELDSTONE:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_SHIELDSTONE)) break;
			if(level > 1) break;
			*str += "A mage with the Create Shieldstone skill may create "
				"Shieldstones, which confers upon the possessor a personal "
				"Energy Shield of 3. A mage may create his skill level in "
				"Shieldstones per turn. To use this spell, CAST "
				"Create_Shieldstone";
			break;
		case S_CREATE_MAGIC_CARPET:
			/* XXX -- This should be cleaner somehow. */
			if(ITEM_DISABLED(I_MCARPET)) break;
			if(level > 1) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Create Magic Carpet skill may create "
    				"Magic Carpets, which provide for flying transportation. A "
    				"Magic Carpet can carry up to 15 weight units in the air. "
    				"Casting this spell allows the mage to create his skill "
    				"level in Magic Carpets. To cast the spell, CAST "
    				"Create_Magic_Carpet.";
			} else {
			    *str += AString("A mage with the Create Magic Carpet skill may create "
    				"Magic Carpets, which provide for flying transportation. A "
    				"Magic Carpet can carry up to ") + ItemDefs[I_MCARPET].fly + 
                    " weight units in the air. A mage may create up to his skill "
        		    "level squared carpets per turn. For every skill "
                    "level carpets created, this spell " + EnergyString(skill,1,6,1) + 
                    " To use this spell, CAST Create_Magic_Carpet, and "
                    "the mage will create as many magic carpets as he "
                    "is able.";
            }
			break;
		case S_ENGRAVE_RUNES_OF_WARDING:
			/* XXX -- This should be cleaner somehow. */
			if(!Globals->ARCADIA_MAGIC) {
    			if(level == 1) {
    				*str += "A mage with the Engrave Runes of Warding may "
    					"engrave runes of warding on a building; these runes "
    					"will give any occupants of the building a personal "
    					"Energy Shield and Spirit Shield, both at level 3. A "
    					"mage has a 20 percent chance per level of succeeding "
    					"with each attempt to cast this spell. To use this "
    					"spell, the mage should CAST Engrave_Runes_of_Warding, "
    					"and be within the building he wishes to engrave runes "
    					"upon. This spell costs 600 silver to cast.";
    				if(OBJECT_ENABLED(O_TOWER)) {
    					*str += " At level 1, the mage may engrave runes of "
    						"warding upon a Tower.";
    				}
    			} else if (level == 2) {
    				int comma = 0;
    				if(OBJECT_DISABLED(O_FORT) && OBJECT_DISABLED(O_MTOWER))
    					break;
    				*str += "At this level, the mage may engrave runes of "
    					"warding upon ";
    				if(OBJECT_ENABLED(O_FORT)) {
    					*str += "a Fort";
    					comma = 1;
    				}
    				if(OBJECT_ENABLED(O_MTOWER)) {
    					if(comma) *str += ", and ";
    					*str += "a Magic Tower";
    				}
    				*str += ".";
    			} else if (level == 3) {
    				if(OBJECT_ENABLED(O_CASTLE)) {
    					*str += "At this level, the mage may engrave runes of "
    						"warding upon a Castle.";
    				}
    			} else if (level == 4) {
    				if(OBJECT_ENABLED(O_CITADEL)) {
    					*str += "At this level, the mage may engrave runes of "
    						"warding upon a Citadel.";
    				}
    			} else if(level == 5) {
    				if(OBJECT_ENABLED(O_MFORTRESS)) {
    					*str += "At this level, the mage may engrave runes of "
    						"warding upon a Magical Fortress, which grants "
    						"the inhabitants an Energy Shield and Spirit "
    						"Shield at level 5.";
    				}
    			}
    			break;
			} else {
    			if(level == 1) {
    				*str += "A mage with the Engrave Runes of Warding may "
    					"engrave runes of warding on a building; these runes "
    					"will increase the defence skill of any occupants "
    					"against energy, spirit and weather attacks by the mage's skill "
                        "level (at time of casting). To use this "
    					"spell, the mage should CAST Engrave_Runes_of_Warding, "
    					"and be within the building he wishes to engrave runes "
    					"upon. At level 1 this spell may be cast on a ";
    					
        	            int found = 0;
        	            AString temp = "";
        	            for(int i=0; i<NOBJECTS; i++) {
        	                if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
        	                if(!(ObjectDefs[i].flags & ObjectType::CANMODIFY)) continue;
        	                if(ObjectDefs[i].protect > 0 && ObjectDefs[i].protect < 40) {
        	                    if(found++) {
                                    *str += temp;
        	                        *str += ", ";
                                }
        	                    temp = ObjectDefs[i].name;
        	                }	            
        	            }
        	            if(found>1) *str += AString("or ") + temp;
        	            else *str += temp;
        	            *str += AString(", which ") + EnergyString(skill,1,6,1);
    					    					
    			} else if (level == 2) {
    				*str += "At level 2 this spell may be cast on a ";
    					
        	            int found = 0;
        	            AString temp = "";
        	            for(int i=0; i<NOBJECTS; i++) {
        	                if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
        	                if(!(ObjectDefs[i].flags & ObjectType::CANMODIFY)) continue;
        	                if(ObjectDefs[i].protect > 39 && ObjectDefs[i].protect < 200) {
        	                    if(found++) {
                                    *str += temp;
        	                        *str += ", ";
                                }
        	                    temp = ObjectDefs[i].name;
        	                }	            
        	            }
        	            if(found>1) *str += AString("or ") + temp;
        	            else *str += temp;
        	            *str += AString(", which ") + EnergyString(skill,2,6,2);
    			} else if (level == 3) {
    				*str += "At level 3 this spell may be cast on a ";
    					
        	            int found = 0;
        	            AString temp = "";
        	            for(int i=0; i<NOBJECTS; i++) {
        	                if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
        	                if(!(ObjectDefs[i].flags & ObjectType::CANMODIFY)) continue;
        	                if(ObjectDefs[i].protect > 199 && ObjectDefs[i].protect < 1000) {
        	                    if(found++) {
                                    *str += temp;
        	                        *str += ", ";
                                }
        	                    temp = ObjectDefs[i].name;
        	                }	            
        	            }
        	            if(found>1) *str += AString("or ") + temp;
        	            else *str += temp;
        	            *str += AString(", which ") + EnergyString(skill,3,6,3);
    			} else if (level == 4) {
    				*str += "At level 4 this spell may be cast on a ";
    					
        	            int found = 0;
        	            AString temp = "";
        	            for(int i=0; i<NOBJECTS; i++) {
        	                if(ObjectDefs[i].flags & ObjectType::DISABLED) continue;
        	                if(!(ObjectDefs[i].flags & ObjectType::CANMODIFY)) continue;
        	                if(ObjectDefs[i].protect > 999) {
        	                    if(found++) {
                                    *str += temp;
        	                        *str += ", ";
                                }
        	                    temp = ObjectDefs[i].name;
        	                }	            
        	            }
        	            if(found>1) *str += AString("or ") + temp;
        	            else *str += temp;
        	            *str += AString(", which ") + EnergyString(skill,4,6,4);
    			}
                break;		
			}
		case S_CONSTRUCT_GATE:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(!Globals->ARCADIA_MAGIC) {
    			*str += "A mage with the Construct Gate skill may construct a "
    				"Gate in a region. The mage has a 20 percent times his "
    				"skill level chance of success, and the attempt costs 1000 "
    				"silver. To use this spell, the mage should issue the order "
    				"CAST Construct_Gate.";
			} else {
    			*str += AString("A mage with the Construct Gate skill may construct a "
    				"Gate in a region. This spell ") + EnergyString(skill,1,6,1) + 
    				" To use this spell, the mage should issue the order "
    				"CAST Construct_Gate.";
			}
			break;
		case S_ENCHANT_SWORDS:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_MSWORD)) break;
			*str += "A mage with the Enchant Swords skill may convert "
				"swords to mithril swords. A mage may convert 5 times his "
				"skill level swords to mithril swords per turn. The mage should "
				"issue the order CAST Enchant_Swords to cast this spell.";
			if(Globals->ARCADIA_MAGIC) *str += AString(" For every two swords "
			    "enchanted, this spell ") + EnergyString(skill,1,6,2);
			break;
		case S_ENCHANT_ARMOR:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_MPLATE)) break;
			*str += "A mage with the Enchant Armour skill may magically "
				"convert plate armour to mithril armour. A mage may create 5 times his skill "
				"level mithril armours per turn. The mage should issue the "
				"order CAST Enchant_Armour to cast this spell.";
			if(Globals->ARCADIA_MAGIC) *str += AString(" For every two armour "
			    "enchanted, this spell ") + EnergyString(skill,1,6,2);
			break;
		case S_CONSTRUCT_PORTAL:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_MPLATE)) break;
			*str += "A mage with the Construct Portal skill may construct a "
				"Portal";
			if(SKILL_ENABLED(S_PORTAL_LORE)) {
				*str += " for use with the Portal Lore skill";
			}
			*str += ". The mage has a 20 percent times his skill level "
				"chance of creating a Portal, and the attempt costs 600 "
				"silver. To use this spell, CAST Construct_Portal.";
			break;
		case S_MANIPULATE:
			if(!Globals->APPRENTICES_EXIST) break;
			if(level > 1) break;
			*str += "A unit with this skill becomes an apprentice mage. "
				"While apprentices cannot cast spells directly, they can "
				"use magic items normally only usable by mages. Continued "
				"study of this skill gives no further advantages.";
			break;
		case S_WEAPONCRAFT:
			if(level > 1) break;
			*str += "The weaponcraft skill is an advanced version of the "
				"weaponsmith skill.";
			break;
		case S_ARMORCRAFT:
			if(level > 1) break;
			*str += "The armourcraft skill is an advanced version of the "
				"armoursmith skill.";
			break;
		case S_CAMELTRAINING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of camel production.";
			break;
		case S_GEMCUTTING:
			if(level > 1) break;
			*str += "This skill enables a unit to fashion higher quality "
				"gems from lower quality ones.";
			break;
		case S_MONSTERTRAINING:
			if(level > 1) break;
			*str += "This skill deals with all aspects of training monster "
				"mounts.";
			break;
		case S_COOKING:
			if(level > 1) break;
			*str += "This skill deals with creating provisions from basic "
					"foodstuffs.  A skilled cook can feed many more people "
					"than a farmer alone.";
			break;
		case S_CREATE_FOOD:
			/* XXX -- This should be cleaner somehow. */
			if(level > 1) break;
			if(ITEM_DISABLED(I_FOOD)) break;
			*str += "A mage with the Create Food skill may magically "
				"create food. A mage may create 5 times his skill level "
				"provisions per turn. The mage should issue the order "
				"CAST Create_Food to cast this spell.";
			break;
		case S_BLIZZARD: //Arcadia
			if(level == 1) {
    			*str += AString("A mage with the Blizzard skill may cast blizzards on distant regions. "
                        "The blizzard will make movement into the region take ten times as "
                        "many movement points as normal, effectively preventing movement into "
                        "the region in almost all cases. This spell ") + EnergyString(skill,1,6,1);
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
    					*str += " This skill only works on the surface of "
    						"the world.";
    				}
    				*str += " The target region must be within ";
    				if(range->rangeMult != 1) {
                        *str += range->rangeMult;
    				    if(range->rangeClass != RangeType::RNG_ABSOLUTE) *str += " times ";
				    }
    				switch(range->rangeClass) {
    					case RangeType::RNG_LEVEL:
    						*str += "the caster's skill level";
    						break;
    					case RangeType::RNG_LEVEL2:
    						*str += "the caster's skill level squared";
    						break;
    					case RangeType::RNG_LEVEL3:
    						*str += "the caster's skill level cubed";
    						break;
    					default:
    					case RangeType::RNG_ABSOLUTE:
    						break;
    				}
    				*str += " regions of the caster. ";
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
    					*str += "Coordinates of locations not on the surface "
    						"are scaled to the surface coordinates for this "
    						"calculation. Attempting to cast across "
    						"different levels increases the distance by ";
    					*str += range->crossLevelPenalty;
    					*str += " per level difference. ";
    					*str += "To use this skill, CAST Blizzard REGION "
    						"<x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the region where you wish to "
    						"cast the blizzard. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
    					if(Globals->UNDERWORLD_LEVELS +
    							Globals->UNDERDEEP_LEVELS == 1) {
    						*str += " The <z> coordinate for the surface is "
    							"'1' and the <z>-coordinate for the "
    							"underworld is '2'.";
    					}
    					*str += " Note that Blizzard cannot be used "
    						"either into or out of the Nexus.";
    				} else {
    					*str += "To use this skill, CAST Blizzard REGION "
    						"<x> <y>, where <x> and <y> are the coordinates "
    						"of the region where you wish to cast the "
    						"blizzard.";
    				}
    			} else {
    				*str += " To use the spell in this fashion, CAST "
    					"Blizzard; no arguments are necessary.";
    			}
            } else if(level == 4) {
    			*str += AString("At level 4, a mage may cast a blizzard on a region and all six "
                        "surrounding regions at once. This spell ") + EnergyString(skill,4,6,4) +
                        " To use the spell in this fashion, "
                        "CAST Blizzard LARGE";
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
                        *str += " REGION <x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the central region around which you wish to "
    						"cast the large blizzard. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
			        } else {
    					*str += " REGION <x> <y>, where <x> and <y> are the "
                            "coordinates of the central region around which you "
                            "wish to cast the large blizzard.";
    				}            
    			    *str += " This spell uses the same range calculation as the "
                            "normal blizzard spell, but the caster's effective skill "
                            "level is reduced by 2 for calculating the maximum range";
                }
                *str += AString(". This skill costs ") + (4*SkillDefs[S_BLIZZARD].cast_cost)
                    + " energy if cast by a " + SkillDefs[SkillDefs[S_BLIZZARD].baseskill].name 
                    + " mage, or " + ((SkillDefs[S_BLIZZARD].cast_cost * 3 * 4)/2)
                    + " energy when cast by any other mage.";
            }
			break;
   		case S_FOG: //Arcadia
			if(level == 1) {
    			*str += AString("A mage with the Fog skill may summon fog into a region, "
                    "concealing all units in the hex from being seen by factions "
                    "other than the faction which owns them for the month after "
                    "the spell is cast. Note that this applies only to the report, "
                    "it does not allow units to steal or assassinate from enemies if "
                    "they would normally not be able to do so, nor prevent exchanges, "
                    "attacks, or give orders, if the target unit number is known. "                    
                    "This spell may be dispelled by "
                    "clear skies cast at equal or higher skill level. To use the spell, "
                    "a mage should CAST Fog, no arguments are necessary. This skill ")
                    + EnergyString(skill,1,6,1) + " When "
                    "used in battle, this spell interferes with the tactical "
                    "effectiveness of enemy officers, by a multiplicative factor of "
                    "0.7 per skill level (ie at skill level 2, the tactics effectiveness "
                    "would be 0.7 squared times their usual effectiveness). This "
                    "is cumulative with any reduction due to the darkness spell. "
                    "If multiple mages cast fog in battle, only the highest level "
                    "spell will have an effect.";
            } else if (level == 4) {
                *str = AString("At skill level 4, a fog mage may summon fog into a region and "
                    "all neighbouring regions at once. The fog need not be centred on the "
                    "mage, it can be centred on any of the six regions surrounding him. "
                    "To use this spell, CAST Fog Large <direction>, where direction is "
                    "the direction in which the fog should be centred, relative to the "
                    "mage. If no direction is specified, then the fog will be centred on "
                    "the region the mage is located in. This spell ") 
                    + EnergyString(skill,4,6,4);
            }
			break;
		case S_SEAWARD: //Arcadia
		    if(level == 1) {
    		    *str += AString("A mage with the Sea Ward skill may harness the furies of the "
                    "wind to keep the ocean from invading a region which would otherwise "
                    "sink. This ward will last for half the mage's skill level, rounded up, months, "
                    "after which, if it is not recast, the region will sink. This ward "
                    "will also be destroyed if another mage rejuvenates this region to "
                    "ocean. This spell ") + EnergyString(skill,1,6,1);
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
    					*str += " This skill only works on the surface of "
    						"the world.";
    				}
    				*str += " The target region must be within ";
    				if(range->rangeMult != 1) {
                        *str += range->rangeMult;
    				    if(range->rangeClass != RangeType::RNG_ABSOLUTE) *str += " times ";
				    }
    				switch(range->rangeClass) {
    					case RangeType::RNG_LEVEL:
    						*str += "the caster's skill level";
    						break;
    					case RangeType::RNG_LEVEL2:
    						*str += "the caster's skill level squared";
    						break;
    					case RangeType::RNG_LEVEL3:
    						*str += "the caster's skill level cubed";
    						break;
    					default:
    					case RangeType::RNG_ABSOLUTE:
    						break;
    				}
    				*str += " regions of the caster. ";
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
    					*str += "Coordinates of locations not on the surface "
    						"are scaled to the surface coordinates for this "
    						"calculation. Attempting to cast across "
    						"different levels increases the distance by ";
    					*str += range->crossLevelPenalty;
    					*str += " per level difference. ";
    					*str += "To use this skill, CAST Sea_Ward REGION "
    						"<x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the region from which you wish to "
    						"ward the sea. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
    					if(Globals->UNDERWORLD_LEVELS +
    							Globals->UNDERDEEP_LEVELS == 1) {
    						*str += " The <z> coordinate for the surface is "
    							"'1' and the <z>-coordinate for the "
    							"underworld is '2'.";
    					}
    					*str += " Note that Sea Ward cannot be used "
    						"either into or out of the Nexus.";
    				} else {
    					*str += "To use this skill, CAST Sea_Ward REGION "
    						"<x> <y>, where <x> and <y> are the coordinates "
    						"of the region from which you wish to ward the "
    						"sea.";
    				}
    				*str += " This spell will fail if cast on a region which "
    				    "is not sinking in the month the spell is cast.";
    			} else {
    				*str += " To use this spell, CAST "
    					"Sea_Ward; no arguments are necessary.";
    			}
            } else if(level == 4) {
    			*str += AString("At level 4, a mage may cast a sea ward on a region and all six "
                        "surrounding regions at once. This spell ") + EnergyString(skill,4,6,3) +
                        " To use the spell in this fashion, "
                        "CAST Sea_Ward LARGE";
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
                        *str += " REGION <x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the central region around which you wish to "
    						"cast the sea ward. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
			        } else {
    					*str += " REGION <x> <y>, where <x> and <y> are the "
                            "coordinates of the central region around which you "
                            "wish to cast the sea ward.";
    				}            
    			    *str += " This spell uses the same range calculation as the "
                            "normal sea ward spell, but the caster's effective skill "
                            "level is reduced by 2 for calculating the maximum range";
                }
                *str += ". This spell will fail if none of the affected regions are "
                    "sinking in the month the spell is cast, though the central region "
                    "need not be sinking itself.";
            }
		    break;
		case S_CONCEALMENT: //Arcadia
			if(level > 1) break;
			*str += "Concealment is a combat spell. When cast in a combat round, "
                    "any formation from the caster's army which attempt to "
                    "flank around the enemy in that round will have a 33% chance, "
                    "times the skill level of the caster, to remain unseen and "
                    "unintercepted. If an enemy mage casts 'dispel illusions' in "
                    "the same combat round, then the effective skill level of this "
                    "spell will be reduced by the skill level of that enemy mage. "
                    "Only the best mage casting concealment has an effect. Unfortunately, "
                    "battle commanders are not yet intelligent enough to factor this "
                    "spell into their decisions, so this spell will not cause "
                    "cavalry to flank if they normally would not due to too much "
                    "enemy cavalry opposition.";
			break;
		case S_ILLUSORY_CREATURES: //Arcadia
			if(level > 4) break;
			//quick coding, doesn't write properly if GM disables any of the illusory creatures.
			//also the maintenance costs of illusory creatures (and summoned creatures) are currently hardcoded.
			if(level == 1) {
				*str += "A mage with Create Illusory Creatures may summon "
					"illusionary creaturess that appear in the mage's inventory. "
					"These beasts will fight in combat, but attack and defend with a skill of -10, "
                    "giving them no chance to hit anything, "
					"nor to survive an attack. At skill "
                    "level 1, a mage may create illusionary";
                if(ITEM_ENABLED(I_IWOLF)) *str += " wolves,";
                if(ITEM_ENABLED(I_IIMP)) *str += " imps,";
                if(ITEM_ENABLED(I_ISKELETON)) *str += " and skeletons.";
                *str += "To use this spell, the mage should CAST "
                    "Create_Illusory_Creatures <creature> <number>, "
                    "where <creature> is the type of illusion the mage "
                    "wishes to summon, and <number> is how many of that "
                    "illusion the mage wishes to have appear in his inventory.";
				*str += " Note: illusionary beasts will appear on reports as "
					"if they were normal items, except on the owner's "
					"report, where they are marked as illusionary. To "
					"reference these items in orders, you must prepend an "
					"'i' to the normal string. (For example: to reference "
					"an illusionary wolf, you would use 'iwolf'). "
					"For every point of energy, a mage can "
                    "create 20 illusions, multiplied by his skill level "
                    "plus two, and divided by three (that is, 20 illusions "
                    "at level 1, up to 53 illusions at level 6). Provided a mage has sufficient energy, "
                    "there is no limit to how many illusions he can create, "
                    "note however that there is an additional 'maintenance' "
                    "cost to sustain these illusions, equal to 1 energy "
                    "per 80 illusions per month at level 1, decreasing "
                    "with skill level.";
			} else if (level == 2) {
				*str += "Create Illusory Creatures at level 2 allows the mage "
					"to summon illusionary eagles, demons and undead into his "
                    "inventory. To summon these illusions, a mage should use the "
                    "usual order for creating illusory creatures, with the appropriate "
                    "type of creature used in the order. These illusions cost "
                    "ten times more to summon and sustain than illusory imps, "
                    "wolves, or skeletons.";
			} else if(level == 3) {
				*str += "Create Illusory Creatures at level 3 allows the mage "
					"to summon illusionary gryffins, balrogs and liches into his "
                    "inventory. To summon these illusions, a mage should use the "
                    "usual order for creating illusory creatures, with the appropriate "
                    "type of creature used in the order. These illusions cost "
                    "forty times more to summon and sustain than illusory imps, "
                    "wolves, or skeletons.";
			} else if(level == 4) {
				*str += "Create Illusory Creatures at level 4 allows the mage "
					"to summon illusionary dragons into his "
                    "inventory. To summon illusory dragons, a mage should use the "
                    "usual order for creating illusory creatures, with 'DRAGON' "
                    "as the creature used in the order. Illusory dragons cost "
                    "160 times more to summon, and 80 times more to sustain, than illusory imps, "
                    "wolves, or skeletons (4 energy per dragon to summon at level 4).";
			}
			break;
		case S_SUMMON_MEN: //Arcadia
			if(level > 1) break;
			*str += AString("A mage with this skill may summon men into a unit belonging "
                "to his faction, and in the same region as the mage. To use "
                "this skill, CAST Summon_Men <number> <race> unit <unit>, where <number> "
                "is the number of men the mage wishes to summon, <race> is the race "
                "the mage wishes to summon, and <unit> is the unit which the men are "
                "to be summoned into. A mage using this skill may not summon leaders, nor "
                "summon men into a unit containing leaders, nor summon men into a unit "
                "controlled by another faction. This skill costs 50 silver "
                "to hire each man which is summoned. A mage may summon at most 10 times "
                "his skill level squared men. For every ten men summoned, this spell ")
                + EnergyString(skill,1,6,1);
			break;
			
		case S_RESURRECTION: //Arcadia
			if(level == 1) {
    			*str += "This skill enables the mage to resurrect men who would "
                    "usually be given up for dead. At higher levels, a mage with this skill may "
                    "attempt to resurrect units from his own or an allied "
                    "faction who have been assassinated in "
                    "the mage's region, or even resurrect him/herself. "
                    "From level 1, in battle a mage may attempt to resurrect "
                    "up to 4 times his skill level squared men who died and could "
                    "not be healed (from 4 corpses at level 1, up to 144 corpses "
                    "at level 6). All resurrection attempts have a 50 percent "
                    "rate of success.";
                if(Globals->ARCADIA_MAGIC && SkillDefs[skill].combat_cost > 0) 
                    *str += AString(" For every skill level times 3 corpses that "
                    "a mage attempts to raise, this spell will cost a "
                    "mage ") + SkillDefs[skill].combat_cost + " energy.";
                *str += " No order is necessary to use this spell, "
                    "it will be used automatically when the mage is involved in "
                    "battle. Note that if a mage is resurrected, he will suffer "
                    "a penalty to his energy levels thereafter.";
            } else if(level == 3) {
                *str += "At level 3, a mage will automatically attempt to resurrect "
                    "men from his own or allied factions which are assassinated in "
                    "the mage's region. Because the mage has only one corpse to focus "
                    "on, his success rate may be higher than that following battles; "
                    "the chance of success is 15% multiplied by the mage's skill level.";
                if(Globals->ARCADIA_MAGIC && SkillDefs[skill].cast_cost > 0) 
                    *str += AString(" This spell ") + EnergyString(skill,3,6,1);            
            } else if(level == 5) {
                *str += "At level 5, a mage can practise the greatest art of resurrection, "
                    "raising himself from the dead when killed in combat. "
                    "This skill will always work, providing the mage has enough "
                    "energy to cast it at the time of his death, but it exacts "
                    "the usual power toll on the mage. The mage will come back to life after the "
                    "end of the battle or assassination he was killed in. ";
                *str += AString("This spell ") + EnergyString(skill,5,6,4);
            }
			break;
		case S_SPIRIT_OF_DEAD: //Arcadia
			if(level > 1) break;
			*str += AString("A mage with this skill may summon the spirit of a long dead mage, "
                "and attempt to convert them to their cause. To use"
                "this skill, CAST Summon_Spirit_of_the_Dead UNITS <unitnum>, where <unitnum> "
                "is the unit number of a dead mage who died at the mage's current "
                "location. If no number is specified, then the mage "
                "will receive a list of all the shades which are at his current "
                "location. The chance of this spell succeeding "
                "is the caster's skill level divided by 6 (ie 1 chance in 6 at level 1). If the target mage was from "
                "a different faction, then it may not be willing to join you; there is "
                "a twenty percent chance per month that a dead mage will lose loyalty "
                "to it's previous faction. Any resurrected mage will suffer a penalty to "
                "their energy supply, equivalent to having N less levels in each "
                "foundation skill, where N is the total number of times they have been "
                "resurrected. For every month that the mage has lain in the land of the "
                "dead, they will lose 30 days of study or experience in a random magic skill. "
                "To resurrect a spirit, this spell ") + EnergyString(skill,1,6,1) +
                " To check if spirits are present in a location, this cost is divided by "
                "five, rounded up.";
            break;
		case S_INNER_STRENGTH: //Arcadia
			if(level > 1) break;
                *str += "A mage with this skill learns to focus inwards, and "
                    "concentrate on harnessing his magics more efficiently. This "
                    "skill gives a bonus to the rate a mage recharges his magic energies, "
                    "equal to 5% of his normal rate (rounded down), plus one energy per skill level per month.";
			break;
		case S_TRANSMUTATION: //Arcadia
			if(level == 1) {
        			*str += AString("This skill deals with the alteration of all objects. "
                        "At level 1, a mage with this skill may magically transmute any "
                        "item into one normal primary item of a different type. "
                        "Primary items are ones produced directly from "
                        "a region, not manufactured from other items. To use this spell, CAST Transmutation "
                        "<to item> <from item>, and the mage will transmute as many items "
                        "in his possession as he is able. A mage is able to transmute up to "
                        "four times his skill level squared "
                        "items. Each eight items which are transmuted, ")
                        + EnergyString(skill,1,6,1);		
			    break;
			} else if(level == 2) {
        			*str += "At level 2, a mage with this skill may magically transmute any "
                        "manufactured item into one normal item of a different type. "
                        "To use this spell, CAST Transmutation "
                        "<to item> <from item>, and the mage will transmute as many items "
                        "in his possession as he is able. A mage is able to transmute up to "
                        "four times his skill level squared "
                        "items. Items which require "
                        "more than one material to manufacture, such as Plate Armour, cannot be "
                        "produced with this spell.";
                break;
            } else if(level == 5) {
    			*str += "At level 5, a mage with this skill may magically transmute any three "
                    "normal primary items into one advanced item of similar type, if one exists. "
                    "The possible conversions which may be made are:";
                if(!(ItemDefs[I_IRON].flags & ItemType::DISABLED) && !(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) )
                    *str += " iron to mithril";
                if(!(ItemDefs[I_WOOD].flags & ItemType::DISABLED) && !(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED) )
                    *str += ", wood to ironwood";
                if(!(ItemDefs[I_WOOD].flags & ItemType::DISABLED) && !(ItemDefs[I_YEW].flags & ItemType::DISABLED) )
                    *str += ", wood to yew";
                if(!(ItemDefs[I_FISH].flags & ItemType::DISABLED) && !(ItemDefs[I_PEARL].flags & ItemType::DISABLED) )
                    *str += ", fish to pearls";
                if(!(ItemDefs[I_STONE].flags & ItemType::DISABLED) && !(ItemDefs[I_ROOTSTONE].flags & ItemType::DISABLED) )
                    *str += ", stone to rootstone";
                if(!(ItemDefs[I_HERBS].flags & ItemType::DISABLED) && !(ItemDefs[I_MUSHROOM].flags & ItemType::DISABLED) )
                    *str += ", herbs to mushrooms";
                if(!(ItemDefs[I_FUR].flags & ItemType::DISABLED) && !(ItemDefs[I_FLOATER].flags & ItemType::DISABLED) )
                    *str += ", furs to floater";
                if(!(ItemDefs[I_HORSE].flags & ItemType::DISABLED) && !(ItemDefs[I_WHORSE].flags & ItemType::DISABLED) )
                    *str += ", horses to winged horses";
                *str += AString(". To use this spell, CAST Transmutation "
                    "<to item>, and the mage will transmute as many items "
                    "in his possession as he is able. A mage is able to transmute up to "
                    "six times his skill level squared items, into two times his "
                    "skill level squared advanced items. "
                    "Each two items which are produced, ")
                    + EnergyString(skill,4,6,1);
                break;
			}
			break;
		case S_MODIFICATION: //Earthsea
			if(level == 1) {
    			*str += AString("This skill allows a mage to harness the magics "
                    "of the land itself, and use it to modify the terrain "
                    "around him. At level 1, a mage may increase the base amount "
                    "of any normal resource that is already present in a "
                    "region, at the cost of reducing the amount of another normal "
                    "resource. For every (4+skill) for which the sought "
                    "resource is increased, the other resource will be "
                    "decreased by 5, resulting in a net increase in "
                    "production at level 2 or higher. "
                    "The amount of the sought resource "
                    "will be increased by 6 times the mage's skill level, "
                    " up to a maximum of "
                    "(1+skill/2) times the minimum amount the region would "
                    "naturally support. For every resource increased, this spell ")
                    + EnergyString(skill,1,6,1);

        			range = FindRange(SkillDefs[skill].range);
        			if(range) {
        				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
        					*str += " This skill only works on the surface of "
        						"the world.";
        				}
        				*str += " The target region must be within ";
        				if(range->rangeMult != 1) {
                            *str += range->rangeMult;
        				    if(range->rangeClass != RangeType::RNG_ABSOLUTE) *str += " times ";
    				    }
        				switch(range->rangeClass) {
        					case RangeType::RNG_LEVEL:
        						*str += "the caster's skill level";
        						break;
        					case RangeType::RNG_LEVEL2:
        						*str += "the caster's skill level squared";
        						break;
        					case RangeType::RNG_LEVEL3:
        						*str += "the caster's skill level cubed";
        						break;
        					default:
        					case RangeType::RNG_ABSOLUTE:
        						break;
        				}
        				*str += " regions of the caster. ";
        				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
        					*str += "Coordinates of locations not on the surface "
        						"are scaled to the surface coordinates for this "
        						"calculation. Attempting to view across "
        						"different levels increases the distance by ";
        					*str += range->crossLevelPenalty;
        					*str += " per level difference. ";
        					*str += "To use this skill, CAST Modification Increase <item to increase> <item to decrease> REGION "
        						"<x> <y> <z> where <x>, <y>, and <z> are the "
        						"coordinates of the region you wish to "
        						"modify. If you omit the <z> "
        						"coordinate, the <z> coordinate of the caster's "
        						"current region will be used.";
        					if(Globals->UNDERWORLD_LEVELS +
        							Globals->UNDERDEEP_LEVELS == 1) {
        						*str += " The <z> coordinate for the surface is "
        							"'1' and the <z>-coordinate for the "
        							"underworld is '2'.";
        					}
        					*str += " Note that Modification cannot be used "
        						"either into or out of the Nexus.";
        				} else {
        					*str += "To use this skill, CAST Modification Increase <item to increase> <item to decrease> REGION "
        						"<x> <y>, where <x> and <y> are the coordinates "
        						"of the region you wish to modify.";
        				}
        			} else {
        				*str += " To use the spell in this fashion, CAST "
        					"Modification Increase <item to increase> <item to decrease>.";
        			}
            } else if(level == 3) {
    			*str += "At level 3, a mage with modification may decrease "
                    "the base amount "
                    "of any normal resource present in a "
                    "region, at the cost of increasing the amount of another normal "
                    "resource. For every (skill) by which the unwanted "
                    "resource is decreased, the other resource will be "
                    "increased by 3, resulting in a net loss of production "
                    "at level 4 or higher. "
                    "The amount of the unwanted resource "
                    "will be decreased by 6 times the mage's skill level, "
                    "to a minimum of 1, provided that the other resource is "
                    "not increased beyond a maximum of "
                    "(1+skill/2) times the minimum amount the region would "
                    "naturally support. The energy cost of this spell is the "
                    "same, per resource decreased, as the cost per resource "
                    "increased when increasing resources.";
                    
        			range = FindRange(SkillDefs[skill].range);
        			if(range) {
        				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
        					*str += " This skill only works on the surface of "
        						"the world.";
        				}
        				*str += " The target region must be within ";
        				if(range->rangeMult != 1) {
                            *str += range->rangeMult;
        				    if(range->rangeClass != RangeType::RNG_ABSOLUTE) *str += " times ";
    				    }
        				switch(range->rangeClass) {
        					case RangeType::RNG_LEVEL:
        						*str += "the caster's skill level";
        						break;
        					case RangeType::RNG_LEVEL2:
        						*str += "the caster's skill level squared";
        						break;
        					case RangeType::RNG_LEVEL3:
        						*str += "the caster's skill level cubed";
        						break;
        					default:
        					case RangeType::RNG_ABSOLUTE:
        						break;
        				}
        				*str += " regions of the caster. ";
        				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
        					*str += "Coordinates of locations not on the surface "
        						"are scaled to the surface coordinates for this "
        						"calculation. Attempting to view across "
        						"different levels increases the distance by ";
        					*str += range->crossLevelPenalty;
        					*str += " per level difference. ";
        					*str += "To use this skill, CAST Modification Decrease "
                                "<item to decrease> <item to increase> REGION "
        						"<x> <y> <z> where <x>, <y>, and <z> are the "
        						"coordinates of the region you wish to "
        						"modify. If you omit the <z> "
        						"coordinate, the <z> coordinate of the caster's "
        						"current region will be used.";
        					if(Globals->UNDERWORLD_LEVELS +
        							Globals->UNDERDEEP_LEVELS == 1) {
        						*str += " The <z> coordinate for the surface is "
        							"'1' and the <z>-coordinate for the "
        							"underworld is '2'.";
        					}
        					*str += " Note that Modification cannot be used "
        						"either into or out of the Nexus.";
        				} else {
        					*str += "To use this skill, CAST Modification Decrease "
                                "<item to decrease> <item to increase> REGION "
        						"<x> <y>, where <x> and <y> are the coordinates "
        						"of the region you wish to modify.";
        				}
        			} else {
        				*str += " To use the spell in this fashion, CAST "
        					"Modification Decrease <item to decrease> <item to increase>.";
        			}
            } else if(level == 4) {
                *str += AString("At skill level 4, a modification mage may increase "
                    "the levels of advanced resources in a region at the "
                    "cost of decreasing a normal resource. This works "
                    "exactly the same as with normal resources, except that "
                    "the mage's effective skill level is reduced by 2 in "
                    "all calculations, and for every resource increased, this spell ")
                    + EnergyString(skill,4,6,4);
                    
            } else if(level == 5) {
                *str += "At skill level 5, a modification mage may decrease "
                    "the levels of advanced resources in a region at the "
                    "cost of increasing another resource. This works "
                    "exactly the same as with decreasing normal resources, except that "
                    "the mage's effective skill level is reduced by 2 in "
                    "all calculations, and the spell cost is as for "
                    "increasing advanced resources.";
            }
			break;
		case S_REJUVENATION: //Earthsea
		    if(level == 2) break;
		    if(level == 4) break;
			if(level == 1) {
    			*str += AString("This skill allows a mage to change the underlying "
                    "magic of the land itself, converting it to a completely "
                    "different type of terrain. At skill level 1, a mage may "
                    "convert any land region into any other type of land "
                    "region, regenerating local products and wages in the process "
                    "(the new region may be the same as the old type, in which "
                    "case only the products and wages will change). Possible "
                    "land types which may be created are: plain, forest, mountain, "
                    "swamp, jungle, desert, tundra. This spell ")
                    + EnergyString(skill,1,6,1);

            	range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
    					*str += " This skill only works on the surface of "
    						"the world.";
    				}
    				*str += " The target region must be within ";
    				if(range->rangeMult != 1) {
                        *str += range->rangeMult;
    				    if(range->rangeClass != RangeType::RNG_ABSOLUTE) *str += " times ";
    			    }
    				switch(range->rangeClass) {
    					case RangeType::RNG_LEVEL:
    						*str += "the caster's skill level";
    						break;
    					case RangeType::RNG_LEVEL2:
    						*str += "the caster's skill level squared";
    						break;
    					case RangeType::RNG_LEVEL3:
    						*str += "the caster's skill level cubed";
    						break;
    					default:
    					case RangeType::RNG_ABSOLUTE:
    						break;
    				}
    				*str += " regions of the caster. ";
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
    					*str += "Coordinates of locations not on the surface "
    						"are scaled to the surface coordinates for this "
    						"calculation. Attempting to view across "
    						"different levels increases the distance by ";
    					*str += range->crossLevelPenalty;
    					*str += " per level difference. ";
    					*str += "To use this skill, CAST Rejuvenation "
                            "<new terrain type> REGION "
    						"<x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the region you wish to "
    						"rejuvenate. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
    					if(Globals->UNDERWORLD_LEVELS +
    							Globals->UNDERDEEP_LEVELS == 1) {
    						*str += " The <z> coordinate for the surface is "
    							"'1' and the <z>-coordinate for the "
    							"underworld is '2'.";
    					}
    					*str += " Note that Rejuvenation cannot be used "
    						"either into or out of the Nexus.";
    				} else {
    					*str += "To use this skill, CAST Rejuvenation "
                            "<new terrain type> REGION "
    						"<x> <y>, where <x> and <y> are the coordinates "
    						"of the region you wish to rejuvenate.";
    				}
    			} else {
    				*str += " To use the spell in this fashion, CAST "
    					"Rejuvenation <new terrain type>.";
    			}




            } else if(level == 3) {
    			*str += AString("At skill level 3, a mage may convert an ocean or lake "
                    "region into a land region of any type. Using this spell "
                    "in this fashion ") + EnergyString(skill,1,4,2);
            } else if(level == 5) {
    			*str += AString("At skill level 5, a mage may convert a region "
                    "into an ocean or lake region. The change will take place "
                    "at the end of the month after the spell is cast, and the "
                    "region will be marked as sinking in the intervening "
                    "report. Any units which remain when the region sinks "
                    "will drown. The region specified should be 'ocean', "
                    "even if it will form a lake when sunk. Casting rejuvenation "
                    "in this manner ") + EnergyString(skill,1,2,3);
            }
			break;
		case S_DIVERSION: //Earthsea
			if(level > 1) break;
			*str += AString("This skill allows a mage to divert the flow of major rivers. "
			    "When cast on a region, rivers will be removed from all edges where "
			    "they are placed, and transferred to all edges which did not "
			    "previously have a river or coastline feature (beach, rocks, harbour). "
			    "In the case whereby two or more rivers join in the region, then the "
			    "edges will be broken into sections according to where rivers enter/exit the "
			    "region. The section with no river will have a river added, and the segment "
			    "clockwise from there will have the river removed. "
			    "Repeated use of this spell allows the path of an entire river to "
			    "be diverted. This spell ") + EnergyString(skill,1,6,1);
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
    					*str += " This skill only works on the surface of "
    						"the world.";
    				}
    				*str += " The target region must be within ";
    				if(range->rangeMult != 1) {
                        *str += range->rangeMult;
    				    if(range->rangeClass != RangeType::RNG_ABSOLUTE) *str += " times ";
				    }
    				switch(range->rangeClass) {
    					case RangeType::RNG_LEVEL:
    						*str += "the caster's skill level";
    						break;
    					case RangeType::RNG_LEVEL2:
    						*str += "the caster's skill level squared";
    						break;
    					case RangeType::RNG_LEVEL3:
    						*str += "the caster's skill level cubed";
    						break;
    					default:
    					case RangeType::RNG_ABSOLUTE:
    						break;
    				}
    				*str += " regions of the caster. ";
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
    					*str += "Coordinates of locations not on the surface "
    						"are scaled to the surface coordinates for this "
    						"calculation. Attempting to view across "
    						"different levels increases the distance by ";
    					*str += range->crossLevelPenalty;
    					*str += " per level difference. ";
    					*str += "To use this skill, CAST Diversion REGION "
    						"<x> <y> <z> where <x>, <y>, and <z> are the "
    						"coordinates of the region where you wish to "
    						"divert the river. If you omit the <z> "
    						"coordinate, the <z> coordinate of the caster's "
    						"current region will be used.";
    					if(Globals->UNDERWORLD_LEVELS +
    							Globals->UNDERDEEP_LEVELS == 1) {
    						*str += " The <z> coordinate for the surface is "
    							"'1' and the <z>-coordinate for the "
    							"underworld is '2'.";
    					}
    					*str += " Note that Diversion cannot be used "
    						"either into or out of the Nexus.";
    				} else {
    					*str += "To use this skill, CAST Diversion REGION "
    						"<x> <y>, where <x> and <y> are the coordinates "
    						"of the region where you wish to divert the "
    						"river.";
    				}
    			} else {
    				*str += " To use the spell in this fashion, CAST "
    					"Diversion; no arguments are necessary.";
    			}			
			break;
		case S_GRYFFIN_LORE: //Earthsea
			if(level > 1) break;
			*str += "A hero with the Gryffin Lore skill can tame gryffins to "
				"join him, to aid in battle and provide flying "
				"transportation. A hero may have up to his skill level divided "
                "by 2, rounded up, gryffins in his inventory at once. When "
                "cast, a hero has a 10% chance, times their effective skill level, "
                "of taming a new gryffin each month. A hero's effective skill level "
                "is equal to their gryffin lore skill, less two times the number "
                "of gryffins they already control. To use this spell, "
				"the hero should issue the order CAST Gryffin_Lore.";
			break;
		case S_HYPNOSIS: //Earthsea
			if(level == 1) {
    			*str += AString("This skill allows a mage to obtain limited control "
                    "over enemy units for a single month. This spell will "
                    "affect only the target units' month long orders. At "
                    "skill level 1 a mage may order a target unit to work, "
                    "tax, or produce. This skill cannot target mages. To use "
                    "this spell, CAST Hypnosis <orders> UNITS <unit> ..., "
                    "where <unit> is a list of the units that the mage wishes "
                    "to hypnotise. A mage may hypnotise a number of men "
                    "equal to his skill level squared, times ten. For every ten "
                    "men times his skill level the mage hypnotises, this spell ") + 
                    EnergyString(skill,1,6,1) + " If the mage "
                    "attempts to hypnotise more men than he is able, then the units "
                    "at the start of the unit list will be hypnotised, up until "
                    "the total number of men exceeds the maximum allowed (if "
                    "this is the case for the first unit, then no units will be "
                    "hypnotised. Valid "
                    "arguments for <orders> are: 'WORK', 'TAX', 'PRODUCE "
                    "<item>'.";
            } else if(level == 2) {
                *str += "At skill level 2, a hypnosis mage may hypnotise "
                    "unit to study. To use this spell in this fashion, use "
                    "'STUDY <skill>' as the order in a hypnotise spell. Note "
                    "that a unit may not be hypnotised into studying magic "
                    "or apprentice skills.";
            } else if(level == 3) {
                *str += "At skill level 3, a hypnosis mage may hypnotise a "
                    "unit to move. To use this spell in this fashion, use"
                    "'MOVE <dirs> ...' as the order in a hypnotise spell, where "
                    "<dirs> is a list of the directions that the unit(s) should "
                    "move.";
            } else if(level == 4) {
                *str += "At skill level 4, a hypnosis mage may hypnotise a "
                    "unit to advance. To use this spell in this fashion, use"
                    "'ADVANCE <dirs> ...' as the order in a hypnosis spell, where "
                    "<dirs> is a list of the directions that the unit(s) should "
                    "advance.";
            } else if(level == 5) {
                *str += "At skill level 5, a hypnosis mage may hypnotise a "
                    "unit to pillage or sail. To use this spell in this fashion, use "
                    "'PILLAGE', or 'SAIL <dirs> ...' as the order in a hypnosis "
                    "spell, where <dirs> is a list of the directions the unit(s) should "
                    "advance.";
            }       
			break;
		case S_BINDING: //Earthsea
			if(level > 1) break;
                *str += "This is a combat skill. It enables a mage to bind an enemy "
                    "mage during combat, preventing him from spellcasting for the "
                    "remainder of the battle. The success of this spell is dependent "
                    "on the power of the casting and target mage, as well as the mage's "
                    "skill level. The chance of success is power * level, out of a total "
                    "of (power * level + enemypower * (3 + enemylevel/2)), where level "
                    "and enemylevel are the greater of the mage's skill in "
                    "either binding or dragon lore. Because this skill is a contest "
                    "of magic between mages, if the caster runs out of energy "
                    "during the battle, he will not cast this spell again, at "
                    "a reduced skill or otherwise.";
			break;
		case S_CREATE_PORTAL: //Earthsea
			if(level > 1) break;
                *str += "This skill allows a mage to create linked portals, "
                    "allowing himself and other units to move directly from "
                    "one land region with a portal to another (portals may "
                    "not be placed in oceans). A portal may be destroyed "
                    "by any unit within it, but is otherwise kept open by the mage "
                    "himself, and will be destroyed if anyone attempts to pass "
                    "through it after the mage dies or runs out "
                    "of energy. To create a portal, a mage should issue the order 'CAST "
                    "Create_Portal'. This will create an unlinked portal in the mage's "
                    "region.";
    			range = FindRange(SkillDefs[skill].range);
   				if(range && (range->flags & RangeType::RNG_CROSS_LEVELS)) {
                    *str += " To link this portal to another, the mage should issue "
                        "the order 'CAST Create_Portal REGION <x> <y> <z>, where "
                        "<x>, <y> and <z> are the coordinates of the region where "
                        "the portal you wish to link to is located.";
                } else {
                    *str += " To link this portal to another, the mage should issue "
                        "the order 'CAST Create_Portal REGION <x> <y>, where "
                        "<x> and <y> are the coordinates of the region where "
                        "the portal you wish to link to is located.";
                }
                *str += " This will create a second portal linked to the first portal, "
                    "through which units may then pass.";
                    //energy cost from units passing through. Hence should protect from enemies.
                    
    			range = FindRange(SkillDefs[skill].range);
    			if(range) {
    				if(range->flags & RangeType::RNG_SURFACE_ONLY) {
    					*str += " This skill only works on the surface of "
    						"the world.";
    				}
    				*str += " The target region must be within ";
    				if(range->rangeMult != 1) {
                        *str += range->rangeMult;
    				    if(range->rangeClass != RangeType::RNG_ABSOLUTE) *str += " times ";
				    }
    				switch(range->rangeClass) {
    					case RangeType::RNG_LEVEL:
    						*str += "the caster's skill level";
    						break;
    					case RangeType::RNG_LEVEL2:
    						*str += "the caster's skill level squared";
    						break;
    					case RangeType::RNG_LEVEL3:
    						*str += "the caster's skill level cubed";
    						break;
    					default:
    					case RangeType::RNG_ABSOLUTE:
    						break;
    				}
    				*str += " regions of the caster. ";
    				if(range->flags & RangeType::RNG_CROSS_LEVELS) {
    					*str += "Coordinates of locations not on the surface "
    						"are scaled to the surface coordinates for this "
    						"calculation. Attempting to link across "
    						"different levels increases the distance by ";
    					*str += range->crossLevelPenalty;
    					*str += " per level difference. ";
    					*str += " Note that Create Portal cannot be used "
    						"either into or out of the Nexus.";
    				}
    			} else {
    				*str += " The range function of this spell seems to be disabled. "
                        "Please contact your GM to have the problem corrected.";
    			}
    		*str += AString("If creating a portal, this spell ") + EnergyString(skill,1,6,1) +
                " In addition, for every (40 * skill) weight units (or part "
                "thereof) which pass through the portal, the mage will lose a point of "
                "energy to maintain the portal connection. "
                "If the mage does not have enough energy, the portal will collapse. Portals "
                "should be kept well guarded, else hostile troops passing through a portal "
                "could quickly drain a mage of energy.";
			break;
		case S_LIGHT: //Earthsea
			if(level > 1) break;
                *str += "This skill allows the mage to bend light "
                    "and concentrate it where he sees fit. If darkness "
                    "is cast in battle, this spell counteracts it, "
                    "reducing the darkness mage's effective skill level "
                    "by the light mage's skill level (to a minimum of zero). ";			
			break;	   			
		case S_DARKNESS: //Earthsea
			if(level > 1) break;		
			*str += "This skill allows a mage to deflect light away from his "
               "location, bringing an unnatural darkness to the surrounding "
               "area. When cast in combat this spell will act like a fog spell "
               "and reduce the tactical effectiveness of tactitians in BOTH "
               "armies by a multiplicative factor of 0.7 per skill level (ie "
               "at skill level 2, the tactics effectiveness will be "
               "0.7 squared). This skill will also reduce the attack "
               "skill and chance-to-attack of all troops in the battle by the "
               "caster's skill level, drastically reducing casualties in the battle. "
               "If multiple mages cast darkness, only the highest level spell "
               "will have an effect. While darkness can be a very powerful "
               "spell, any mage should be careful in using it as it will "
               "affect soldiers on both sides of a battle equally.";
			break;
		case S_DRAGON_TALK:
			if(level == 1) {
                *str += "This skill enabled a mage to converse with dragons. At level "
                    "one, it gives the casting mage a chance to convince a dragon "
                    "to not fight him and his allies, and leave a battle. If the "
                    "mage fails at this, he may instead be able to bind the "
                    "dragon from spellcasting during the battle. Each dragon may "
                    "only be targetted by this spell once per battle, so if "
                    "multiple mages are casting this spell, the highest level "
                    "mage may not be the one to target the dragon. The chance "
                    "of success for this spell is (approximately) dependent on the mage's skill "
                    "level multiplied by the mage's power (maximum energy). If there are "
                    "no dragons present to target, then this spell will revert to "
                    "a normal binding spell, allowing a mage to bind enemy mages. "
                    "Like binding, if the caster runs our of energy during the "
                    "battle, he will not cast any more spells in the battle.";
            } else if (level == 4) {
                *str += "At level 4 of this skill, a mage has an addition chance "
                    "to convince a dragon to join him in his quests. This chance "
                    "increases considerably at higher skill levels.";
            }
            break;			
		case S_BASE_WINDKEY: //Earthsea
			if(level > 1) break;
			*str += "Windkey is one of the Foundation skills on which other "
				"magical skills are based. This skill deals with the mage's ability "
				"to control the wind, weather, and air around them. Mages skilled in windkey are well "
				"known for their ability at sea, where they may calm the waters or "
				"bring wind to speed a ship. Lesser known skills include the "
                "ability to turn air into flame, and to listen on the wind to "
                "things far away.";
            break;
		case S_BASE_ILLUSION: //Earthsea E
			if(level > 1) break;
			*str += "Illusion is one of the Foundation skills on which other "
				"magical skills are based. This skill deals with the mage's ability "
				"to summon illusions, which distort the appearance of the world, but "
				"have no permanent effects. At their most base level, mages skilled "
				"in weaving illusions may be skilled entertainers, but illusions "
				"can be used for more insidious purposes as well. Some mages may devote "
				"their studies not to creating illusions, but to the ability to see "
				"through them and know the world as it truly is. Although these mages "
				"may be found anywhere, they are most common in the east.";
            break;                    
		case S_BASE_SUMMONING: //Earthsea S
			if(level > 1) break;/*
			*str += "Summoning is one of the Foundation skills on which other "
				"magical skills are based. This skill deals with the mage's ability "
				"to summon creatures from other planes of existance into this world, "
				"and control them when summoned. Summoners are usually skilled at "
				"summoning either from the plane of demons or the lands of death, "
				"though some have been known to summon living men as well. Although these mages "
				"may be found anywhere, they are most common in the dark southern lands.";*/
			*str += "Summoning is one of the Foundation skills on which other "
				"magical skills are based. This skill deals with the mage's ability "
				"to summon creatures from other planes of existance into this world, "
				"and control them when summoned. Summoners are usually skilled at "
				"summoning either from the plane of demons or the lands of death, "
				"though some have been known to summon living men as well. And "
                "whilst many have learnt to banish summoned creatures, it is rumoured "
                "that some summoners even learn to banish the sun itself.";
            break; 
   		case S_BASE_PATTERNING: //Earthsea N
			if(level > 1) break;/*
			*str += "Patterning is one of the Foundation skills on which other "
				"magical skills are based. This skill deals with the mage's ability "
				"to interact with nature and the magic intrinsic to the land and creatures "
                "around him. Skilled patterners may control the beasts of the land, "
                "heal creatures or men, or use the "
                "magical energy of the land to modify their surroundings in numerous "
				"ways. Although these mages "
				"may be found anywhere, they are most common in the northern lands.";*/
			*str += "Patterning is one of the Foundation skills on which other "
				"magical skills are based. This skill deals with the mage's ability "
				"to interact with nature and the magic intrinsic to the world and land "
                "around themself. Skilled patterners may alter items in their vicinity, "
                "heal creatures or men, or use the "
                "magical energy of the land to modify their surroundings in numerous "
				"ways.";
			break;
		case S_BASE_MYSTICISM: //Earthsea
			if(level > 1) break;
/*			*str += "Mysticism is the most unusual of the magic arts, often spurned by "
				"mages as delving into the dark arts. In truth, like any magic, "
				"knowledge of mysticism can be used for good or bad. However, any "
                "mage who has knowledge of mysticism will be tainted by this knowledge, "
                "whatever spell he may be casting. Mages skilled in mysticism draw "
				"power from outside themselves, and may channel this great power in "
                "their spells. A side effect of this is that many spells cast by "
                "mystics will backfire or have unintended consequences. The likelihood "
                "of this occuring increases as mysticism makes up a large fraction of the mage's "
                "power supply, but decreases slightly as mages become more skilled in "
                "the mystic arts. Mages with an innate talent in mysticism are less "
                "likely to have spells backfire on them than other mages with similar "
                "power drawn from mysticism; however, they will usually also have more "
                "of their power drawn from this supply.";*/
            *str += "Mysticism is one of the Foundation skills on which other "
				"magical skills are based. It is the most unusual of the magic arts, often spurned by "
				"mages as delving into the dark arts. In truth, like any magic, "
				"knowledge of mysticism can be used for good or bad, and offense or defence.";    
			break;	
		case S_BASE_ARTIFACTLORE: //Earthsea
			if(level == 1) {
    			*str += "Artifact Lore is one of the Foundation skills on which other "
    				"magical skills are based. This skill deals with a mage's ability "
    				"to channel magical energy into artifacts, which can be used to "
                    "aid a mage without drawing on their power. Many artifacts "
                    "may also be used by men who are neither mages nor heroes.";
    			*str += AString(" A mage at level 1 may create amulets of "
                    "protection, which grant the possessor a "
    				"personal Spirit Shield of 3. A mage may create up to his skill "
    				"level squared of these amulets per turn. For every skill "
                    "level amulets created, this spell ") + EnergyString(skill,1,6,1) + 
                    " To use this spell, CAST Artifact_Lore Protection, and "
                    "the mage will create as many amulets of protection as he "
                    "is able.";
			} else if(level == 3) {
    			*str += AString("A mage with artifact lore at level 3 may create "
                    "shieldstones, which grant the possessor a "
    				"personal Energy Shield of 3. A mage may create up to his skill "
    				"level squared of shieldstones per turn. For every skill "
                    "level shieldstones created, this spell ") + EnergyString(skill,3,6,1) +
                    " To use this spell, CAST Artifact_Lore Shieldstone, and "
                    "the mage will create as many shieldstones as he "
                    "is able.";
            }
			break;	
		case S_BASE_BATTLETRAINING: //Earthsea
			if(level > 1) break;
			*str += "This skill allows a hero to gain extra proficiency in battle. The "
                "hero's attack skill, and all defence skills, will be increased by "
                "his battletraining skill level in combat.";
			break;
			break;		
		case S_BASE_CHARISMA: //Earthsea
			if(level == 1) {
			*str += "Charisma is a powerful tool, and subsequent skills allow heroes to charm or "
                "persuade others into aiding the hero. Charisma itself gives a hero "
                "the chance to learn from locals of advanced resources in a region, "
                "even if the hero would not normally be able to detect them. At skill "
                "level 1 the hero has a 50% chance of detecting any advanced resource "
                "each month. This skill "
                "is used automatically in the region where the hero ends the month, "
                "providing there are local peasants.";
            }
            else {
            int cycles = level;
            int chance = 1000;
            while(cycles--) chance /= 2;
            chance = 100 - (chance+5)/10;
			*str += AString("At level ") + level + " a hero has a " + chance + "% chance "
			    "of detecting local advanced resources per month.";
            }
			break;		
		case S_TOUGHNESS: //Earthsea
			if(level > 1) break;
			*str += "A hero with toughness learns to survive the most powerful blows. "
                "For each level gained in toughness, the hero will be able to survive "
                "an extra (level) hits in battle. That is, 1 extra hit at level 1, 3 "
                "at level 2, 6 at level 3, etc. In addition, the hero will be less "
                "susceptible to attacks from a distance, gaining a (level):1 chance "
                "to avoid ranged or magic attacks that would otherwise be successful.";
			break;
		case S_UNITY: //Earthsea
			if(level > 1) break;
			*str += "A hero with unity is able to calm followers, and bring harmony where "
                "there was conflict. When used in battle this skill has a 50% chance of eliminating the morale "
                "penalty from mixing ethnic types for each of, the hero's skill "
                "level squared, times fifty, morale affected soldiers.";
			break;
		case S_FRENZY: //Earthsea
			if(level == 1) {
    			*str += "A hero with frenzy can perform amaxing feats in battle. "
                    "At level 1, a hero with frenzy will be more effective against "
                    "armoured opponents. That is, if the hero's attack is not "
                    "normally armour-piercing, it will become so, and if it is "
                    "armour-piercing (due to use of a crossbow or similar) then "
                    "it will ignore the target's armour altogether. In addition, "
                    "the hero will gain two extra physical attacks per round; or if using a weapon "
                    "that only attacks once every 2 rounds, then will make "
                    "two attacks total per round. Note that this bonus is "
                    "an addition, not multiplication - if a weapon would usually "
                    "grant 3 attacks per battle round, then the hero will get "
                    "3+2 = 5 attacks per round.";
            } else {
                *str += AString("At level ") + level + " frenzy, a hero gains a bonus "
                    + 2*level*level + " physical attacks per battle round.";
                if(level == 2) *str += " However, if using a weapon that only attacks once "
                    "every n rounds, then the hero will only get this bonus, divided by n, "
                    "plus 1 attacks per round.";
                *str += AString(" In addition, if the hero scores a hit against a creature "
                    "with multiple hitpoints, that creature will lose ") + level +
                    " hitpoints, instead of just 1.";
            }
			break;		
		case S_SECSIGHT: //Earthsea
			if(level > 1) break;
			*str += "Second Sight conveys upon a hero the ability to avoid theft and "
                "assassinations directed against him or herself. The hero will get "
                "(skill level) attempts to avoid each theft or assassination directed "
                "against him or her. Each attempt has a 50% probability of success. "
                "This skill has no effect on theft or assassination directed at other "
                "units in the region.";
			break;
		case S_SWIFTNESS: //Earthsea
			if(level > 1) break;
			*str += "With this skill, heros may become swift as the wind. For "
                "can-catch purposes only, the hero's swiftness skill is added "
                "to his riding skill, with the following limitations: if the hero's "
                "movement mode is walking, he will not get any defensive bonus, "
                "if it is riding, he will get a defensive bonus of no more than 3. "
                "If the hero does not have an item with sufficient riding capacity "
                "to carry the hero, he will not get an offensive bonus, and if he "
                "does not have an item with sufficient flying capacity to carry "
                "him, he will get an offensive bonus of no more than 3.";
			break;	
		case S_TRADING: //Earthsea
			if(level > 1) break;
			*str += "Heros can become mighty merchants, and bartering for goods is "
                "one of the first skills they learn. For every level in trading, "
                "a hero will gain 5% extra above the market price of every item they sell, "
                "and pay 5% less for every item they buy (rounded down and up, "
                "respectively). Because it is assumed the best person in the "
                "region will do the bartering, this bonus also "
                "affects any other unit in the region from the same "
                "faction or an allied faction. Multiple trading heros do not "
                "get any additional bonus; only the highest skill level is used. "
                "Also note that, as all your men are natural pessimists, they "
                "will never try to buy more items than they could afford at "
                "the normal price.";
			break;	
		case S_MERCHANTRY: //Earthsea
			if(level == 1) {
    			*str += "Skilled merchants can find a customer anywhere, although they "
                    "may not pay well. At skill level 1, this skill allows your hero "
                    "to sell normal, trade, advanced or magical items at 35% "
                    "of their withdraw cost (for non-normal items, this value may "
                    "be found in reference material that comes with the game, or "
                    "through experimentation). Each additional skill level raises "
                    "this price by 5% of the withdraw cost. This price will be "
                    "rounded down, per item. Note that, if a hero attempts to sell "
                    "items that the local market can buy, the items will be sold "
                    "through the market at the market price, even if the hero is "
                    "unable to sell all (or even any) of those items to the local "
                    "market.";
            } else if(level == 4) {
    			*str += "At level 4, merchants may buy any normal item, anywhere. "
                    "However, the item will cost 130% of its withdraw price. Each "
                    "additional level in merchantry reduces the cost by 10% of "
                    "the withdraw price. This price will be rounded up, per item. "
                    "Note that, if a hero attempts to buy "
                    "items that the local market can provide, the items will be bought "
                    "through the market at the market price, even if the hero is "
                    "unable to buy all (or even any) of the wanted items from the local "
                    "market.";
            
            } else if(level == 6) {
    			*str += "At level 6, merchants may purchase advanced items, albeit "
                    "for an arm and a leg. Since spare arms and legs are hard to find, "
                    "they may instead pay 160% of the withdraw cost of the item, "
                    "rounded up.";
            }
			break;
		case S_ARCADIA_QUARTERMASTERY: //Earthsea
			if(level == 1) {
			    *str += "The quartermaster skill enables heros to transport items "
                    "over large distances, free of charge. With this skill, units "
                    "may use the SEND order to send goods to the hero from up to "
                    "(skill level) regions away, and the hero may also send goods "
                    "to other units, up to (skill level) regions away. These "
                    "transfers will be free, providing either the sender or recipient "
                    "has the quartermastery skill.";
            } else if(level == 4) {
                *str += "At level 4, a quartermaster may facilitate the transfer of "
                    "goods between two other units in a single month. To do so, the "
                    "sending unit should issue the command 'SEND UNIT <targetnum> VIA "
                    "<heronum> <numitems> <itemtype>'. The specified goods will then "
                    "be transferred to the target unit for free, provided that the "
                    "distance from the sender to the quartermaster, plus the distance "
                    "from the quartermaster to the receiver, is no more than the "
                    "quartermaster's skill level. Note that, as with the "
                    "receiver, the quartermaster may not move during the month for "
                    "this order to succeed.";
            }
			break;
	}

	AString temp;
	AString temp1;
	AString temp2;
	int comma = 0;
	int comma1 = 0;
	int comma2 = 0;
	int last = -1;
	int last1 = -1;
	int last2 = -1;
	unsigned int c;
	int i;

	if(level == 1 && (SkillDefs[skill].flags & SkillType::FOUNDATION) ) {
	    *str += AString(" A mage whose race does not excel in ") +
	        SkillDefs[skill].name + " will gain an increase to his maximum energy equal to their " +
            SkillDefs[skill].name + " skill squared. "
            "In addition, the mage's energy recharge will be increased by "
            "their " + SkillDefs[skill].name + " skill. If the mage's "
            "race does excel in " + SkillDefs[skill].name + ", these "
            "energy gains will be doubled.";
	}


	// If this is a combat spell, note it.
	if(level == 1 && (SkillDefs[skill].flags & SkillType::COMBAT) && SkillDefs[skill].special) {
		*str += AString(" A mage with this skill can cast ") +
			ShowSpecial(SkillDefs[skill].special, level, 0, 0);
		*str += " In order to use this spell in combat, the mage should use "
			"the COMBAT order to set it as his combat spell.";
		if(Globals->ARCADIA_MAGIC && SkillDefs[skill].combat_first > 0) {
		    if(SkillDefs[skill].combat_first == SkillDefs[skill].combat_cost) {
		        *str += AString(" This spell will cost a mage ") + SkillDefs[skill].combat_cost + 
                    " energy to cast in combat.";
		    } else {
		        *str += AString(" This spell will cost a mage ") + SkillDefs[skill].combat_first + 
                    " energy to cast in the first round of combat, or " + SkillDefs[skill].combat_cost + 
                    " energy to cast in subsequent combat rounds.";
		    }
		}
	}

	// production and ability to see items
	temp += "A unit with this skill is able to determine if a region "
		"contains ";
	temp1 += "A unit with this skill may PRODUCE ";
	temp2 += "A unit with this skill may create ";
	SkillType *sk1, *sk2;
	sk1 = &SkillDefs[skill];
	for(i = NITEMS - 1; i >= 0; i--) {
		if(ITEM_DISABLED(i)) continue;
		sk2 = FindSkill(ItemDefs[i].mSkill);
		if(sk1 == sk2 && ItemDefs[i].mLevel==level && last2==-1) {
			int canmagic = 1;
			for(c = 0; c < sizeof(ItemDefs[i].mInput)/sizeof(Materials); c++) {
				if(ItemDefs[i].mInput[c].item == -1) continue;
				if(ITEM_DISABLED(ItemDefs[i].mInput[c].item)) {
					canmagic = 0;
				}
			}
			if(canmagic) {
				last2 = i;
			}
		}
		sk2 = FindSkill(ItemDefs[i].pSkill);
		if(sk1 == sk2 && ItemDefs[i].pLevel == level) {
			int canmake = 1;
			int resource = 1;
			for(c = 0; c < sizeof(ItemDefs[i].pInput)/sizeof(Materials); c++) {
				if(ItemDefs[i].pInput[c].item == -1) continue;
				resource = 0;
				if(ITEM_DISABLED(ItemDefs[i].pInput[c].item)) {
					canmake = 0;
				}
			}
			if(canmake && last1 == -1) {
				last1 = i;
			}
			if(resource && (ItemDefs[i].type & IT_ADVANCED) && last == -1) {
				last = i;
			}
		}

	}
	for(i = 0; i < NITEMS; i++) {
		if(ITEM_DISABLED(i)) continue;
		int illusion = (ItemDefs[i].type & IT_ILLUSION);
		sk2 = FindSkill(ItemDefs[i].mSkill);
		if(sk1 == sk2 && ItemDefs[i].mLevel == level) {
			int canmagic = 1;
			for(c = 0; c < sizeof(ItemDefs[i].mInput)/sizeof(Materials); c++) {
				if(ItemDefs[i].mInput[c].item == -1) continue;
				if(ITEM_DISABLED(ItemDefs[i].mInput[c].item)) {
					canmagic = 0;
				}
			}
			if(canmagic) {
				if(comma2) {
					if(last2 == i) {
						if(comma2 > 1) temp2 += ",";
						temp2 += " and ";
					} else {
						temp2 += ", ";
					}
				}
				comma2++;
				temp2 += AString(illusion?"illusory ":"") + ItemDefs[i].names;
				if(f) {
					f->DiscoverItem(i, 1, 1);
				}
			}
		}
		sk2 = FindSkill(ItemDefs[i].pSkill);
		if(sk1 == sk2 && ItemDefs[i].pLevel == level) {
			int canmake = 1;
			int resource = 1;
			for(c = 0; c < sizeof(ItemDefs[i].pInput)/sizeof(Materials); c++) {
				if(ItemDefs[i].pInput[c].item == -1) continue;
				resource = 0;
				if(ITEM_DISABLED(ItemDefs[i].pInput[c].item)) {
					canmake = 0;
				}
			}
			if(canmake) {
				if(comma1) {
					if(last1 == i) {
						if(comma1 > 1) temp1 += ",";
						temp1 += " and ";
					} else {
						temp1 += ", ";
					}
				}
				comma1++;
				temp1 += AString(illusion?"illusory ":"") + ItemDefs[i].names;
				if(f) {
					f->DiscoverItem(i, 1, 1);
				}
			}
			if(resource && (ItemDefs[i].type & IT_ADVANCED)) {
				if(comma) {
					if(last == i) {
						if(comma > 1) temp += ",";
						temp += " and ";
					} else {
						temp += ", ";
					}
				}
				comma++;
				temp += AString(illusion?"illusory ":"") + ItemDefs[i].names;
			}
		}
	}
	if(comma1) {
		if(!(*str == "")) *str += " ";
		*str += temp1 + ".";
	}
	if(comma) {
		if(!(*str == "")) *str += " ";
		*str += temp + ".";
	}
	if(comma2) {
		if(!(*str == "")) *str += " ";
		*str += temp2 + " via magic.";
	}

	// Buildings
	comma = 0;
	temp = "A unit with this skill may BUILD the following structures: ";
	for(i = 0; i < NOBJECTS; i++) {
		if(OBJECT_DISABLED(i)) continue;
		AString skname = SkillDefs[skill].abbr;
		if(skname == ObjectDefs[i].skill && ObjectDefs[i].level == level) {
			if(comma) temp += ", ";
			comma = 1;
			temp += ObjectDefs[i].name;
			if(f) {
				f->objectshows.Add(ObjectDescription(i));
			}
		}
	}
	if(comma) {
		if(!(*str == "")) *str += " ";
		*str += temp + ".";
	}

	//Hexsides
	comma = 0;
	temp = "A unit with this skill may BUILD the following terrain edge features: ";
	for(i = 0; i < NHEXSIDES; i++) {
		if(HexsideDefs[i].flags & HexsideType::DISABLED) continue;
		AString skname = SkillDefs[skill].abbr;
		if(skname == HexsideDefs[i].skill && HexsideDefs[i].level == level) {
			if(comma) temp += ", ";
			else comma = 1;
			temp += HexsideDefs[i].name;
			if(f) {
			    f->terrainshows.Add(HexsideDescription(i));
			}
		}
	}
	if(comma) {
		if(!(*str == "")) *str += " ";
		*str += temp + ".";
	}


	// Required skills
	SkillType *lastpS = NULL;
	last = -1;
	if(level == 1) {
		comma = 0;
		int found = 0;
		temp = "This skill requires ";
		for(c=0; c<sizeof(SkillDefs[skill].depends)/sizeof(SkillDepend); c++) {
			SkillType *pS = FindSkill(SkillDefs[skill].depends[c].skill);
			if (!pS || (pS->flags & SkillType::DISABLED)) continue;
			found = 1;
			if(lastpS == NULL) {
				lastpS = pS;
				last = c;
				continue;
			}
			temp += SkillStrs(lastpS) + " " +
				SkillDefs[skill].depends[last].level + ", ";
			lastpS = pS;
			last = c;
			comma++;
		}
		if(comma) {
			if(SkillDefs[skill].flags & SkillType::STUDYOR) temp += "or "; 
            else temp += "and ";
		}
		if (found) {
			temp += SkillStrs(lastpS) + " " +
				SkillDefs[skill].depends[last].level;
		}

		if(found) {
			if(!(*str == "")) *str += " ";
			*str += temp + " to begin to study.";
		}
	}

	if(level == 1) {
		if(SkillDefs[skill].cost) {
			if(!(*str == "")) *str += " ";
			*str += AString("This skill costs ") + SkillDefs[skill].cost +
				" silver per month of study.";
		}
		if(SkillDefs[skill].flags & SkillType::SLOWSTUDY) {
			if(!(*str == "")) *str += " ";
			*str += "This skill is studied at one half the normal speed.";
		}
	}

	// Tell whether this skill can be taught, studied, or gained through exp
	if(SkillDefs[skill].flags & SkillType::NOSTUDY) {
		if(!(*str == "")) *str += " ";
		*str += "This skill cannot be studied via normal means.";
	}
	if(SkillDefs[skill].flags & SkillType::NOTEACH) {
		if(!(*str == "")) *str += " ";
		*str += "This skill cannot be taught to other units.";
	}
	if((Globals->SKILL_PRACTICE_AMOUNT > 0)) {
		if(!(*str == "")) *str += " ";
		*str += "This skill cannot be increased through experience.";
	}

	temp1 = SkillStrs(skill) + " " + level + ": ";
	if(*str == "") {
		*str = temp1 + "No skill report.";
	} else {
		*str = temp1 + *str;
	}
	return str;
}
