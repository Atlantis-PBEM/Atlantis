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


AString *ShowSkill::Report(void)
{
	AString *str = new AString;
	int found = 0;
	*str = AString(SkillDefs[skill].name)+" ["+SkillDefs[skill].abbr+"] "+
		  level+": ";

	// Here we pick apart the skill
	switch (skill) {
		case S_FARMING:
			if(level == 1) {
				if(ItemDefs[I_GRAIN].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
				        "to produce grain in designated regions.";
				if(!(ItemDefs[I_BAG].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using bags.";
				}
			} else if(level == 3) {
				if(!(ObjectDefs[O_FARM].flags & ObjectType::DISABLED)) {
					found = 1;
					*str += "A unit with this skill may use the BUILD order "
						   "to build a Farm from any combination of wood or "
						   "stone.";
				}
			}
			break;
		case S_RANCHING:
			if(level == 1) {
				if(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					   "to produce livestock in designated regions.";
				if(!(ItemDefs[I_LASSO].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using lassos.";
				}
			} else if(level == 3) {
				if(!(ObjectDefs[O_RANCH].flags & ObjectType::DISABLED)) {
					found = 1;
					*str += "A unit with this skill may use the BUILD order "
						   "to build a Ranch from any combination of wood or "
						   "stone.";
				}
			}
			break;
		case S_MINING:
			if(level == 1) {
				if(ItemDefs[I_IRON].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					   "to produce iron in designated regions. Iron is "
					   "found primarily in mountain regions, but may "
					   "also be found in other regions.";
				if(!(SkillDefs[S_WEAPONSMITH].flags & SkillType::DISABLED)) {
					*str += " The Weaponsmith skill is used to forge iron "
						   "into weapons.";
				}
				if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using picks.";
				}
			} else if(level == 3) {
				if(!(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED)) {
					found = 1;
					*str += "A unit with this skill may use the PRODUCE "
						"order to produce mithril in designated regions "
						"(Note that a Mining skill of 3 is required to "
						"even determine that mithril may be produced). "
						"One unit of mithril weighs 10 weight units.";
					if(!(SkillDefs[S_WEAPONSMITH].flags&SkillType::DISABLED)) {
						*str += " High level Weaponsmiths may use mithril to "
							"forge mithril weapons.";
					}
					if(!(SkillDefs[S_ARMORER].flags & SkillType::DISABLED)) {
						*str += " High level Armorers may use mithril to "
							"forge mithril armor.";
					}
					if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
						*str += " Production can be increased by using picks.";
					}
				}
				if(!(ObjectDefs[O_MINE].flags & ObjectType::DISABLED)) {
					*str += " A unit with this skill may use the BUILD order "
						   "to build a Mine from any combination of wood or "
						   "stone.";
				}
				if(!(ObjectDefs[O_AMINE].flags & ObjectType::DISABLED) &&
				   !(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED)) {
					*str += " A unit with this skill may use the BUILD order "
						   "to build an Arcane Mine from mithril.";
				}
			} else if(level == 5) {
				if(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE "
						"order to produce admantium in designated regions "
						"(Note that a Mining skill of 5 is required to "
						"even determine that admantium may be produced). "
						"Admantium is the strongest metal known.  One unit "
						"of admantium weighs 10 weight units.";
				if(!(SkillDefs[S_WEAPONCRAFT].flags&SkillType::DISABLED)) {
					*str += " High level weapons crafters may use admantium "
						    "to forge admantium weapons.";
				}
				if(!(SkillDefs[S_ARMORCRAFT].flags & SkillType::DISABLED)) {
					*str += " High level armor crafters may use admantium to "
						    "forge admantium armor.";
				}
				if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using picks.";
				}
			}
			break;
		case S_LUMBERJACK:
			if(level == 1) {
				if(ItemDefs[I_WOOD].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order to "
					   "produce wood in designated regions. Forest regions "
					   "are generally the best producers of wood, though "
					   "other regions may also produce wood.";
				if(!(SkillDefs[S_WEAPONSMITH].flags & SkillType::DISABLED)) {
					*str += " Wood may be made into weapons by weaponsmiths.";
				}
				if(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED)) {
					*str += " Wood may be used to construct ships, using the "
						   "Shipbuilding skill.";
				}
				if(!(SkillDefs[S_WEAPONSMITH].flags & SkillType::DISABLED)) {
					*str += " Wood may be used to contstruct longbows and "
						   "crossbows, using the Weaponsmith skill.";
				}
				if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using axes.";
				}
			} else if (level == 3) {
				if(!(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED)) {
					found = 1;
					*str += "A unit with this skill may use the PRODUCE "
						   "order to produce ironwood in designated regions "
						   "(Note that a Lumberjack skill of 3 is required "
						   "to even determine that ironwood may be "
						   "produced). Ironwood is the strongest wood "
						   "known. One unit of ironwood weighs 10 weight "
						   "units.";
					if(!(SkillDefs[S_SHIPBUILDING].flags&SkillType::DISABLED)){
						*str += " Ironwood may be used by high level "
							"Shipbuilders to construct armored ships.";
					}
					if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
						*str += " Production can be increased by using axes.";
					}
				}
				if(!(ObjectDefs[O_TIMBERYARD].flags & ObjectType::DISABLED)) {
					*str += " A unit with this skill may use the BUILD order "
						   "to build a Timber Yard from any combination of "
						   "wood or stone.";
				}
				if(!(ObjectDefs[O_PRESERVE].flags & ObjectType::DISABLED) &&
				   !(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED)) {
					*str += " A unit with this skill may use the BUILD order "
						   "to build an Preserve from ironwood.";
				}
			} else if (level == 5) {
				if(ItemDefs[I_YEW].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order to "
					   "produce yew in designated regions (Note that a "
					   "Lumberjack skill of 5 is required to even determine "
					   "that yew may be produced). Yew is the lightest, "
					   "most pliable wood known.  One unit of yew weighs 5 "
					   "weight units.";
				if(!(SkillDefs[S_WEAPONSMITH].flags & SkillType::DISABLED)) {
					*str += " High level weaponsmiths may use yew to "
					       "produce powerful bows.";
				}
				if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using axes.";
				}
				if(!(ObjectDefs[O_SACGROVE].flags & ObjectType::DISABLED)) {
					*str += " A unit with this skill may use the BUILD order "
						   "to build a Sacred Grove from yew.";
				}
			}
			break;
		case S_QUARRYING:
			if(level == 1) {
				if(ItemDefs[I_STONE].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					   "to produce stone in designated regions. Mountains "
					   "are the main producers of stone, though other "
					   "regions may also produce stone.";
				if(!(SkillDefs[S_BUILDING].flags & SkillType::DISABLED)) {
					*str += " Stone may be used to construct buildings, "
						   "using the Building skill.";
				}
				if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using picks.";
				}
			} else if(level == 3) {
				if(!(ItemDefs[I_STONE].flags & ItemType::DISABLED)) {
					found = 1;
					*str += "A unit with this skill may use the PRODUCE "
						   "order to produce rootstone, a rare stone found "
						   "deep within the earth. (Note that a Quarrying "
						   "skill of 3 is required to even determine that "
						   "rootstone may be produced.)  A unit of "
						   "rootstone weighs 50 weight units.";
					if(!(SkillDefs[S_BUILDING].flags&SkillType::DISABLED) &&
					   !(ObjectDefs[O_MFORTRESS].flags&ObjectType::DISABLED)){
						*str += " High level builders may use rootstone to "
							   "build magical fortresses.";
					}
					if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
						*str += " Production can be increased by using picks.";
					}
				}
				if(!(ObjectDefs[O_QUARRY].flags & ObjectType::DISABLED)) {
					*str += " A unit with this skill may use the BUILD order "
						   "to build a Quarry from any combination of "
						   "wood or stone.";
				}
				if(!(ObjectDefs[O_MQUARRY].flags & ObjectType::DISABLED) &&
				   !(ItemDefs[I_ROOTSTONE].flags & ItemType::DISABLED)) {
					*str += " A unit with this skill may use the BUILD order "
						   "to build a Mystic Quarry from rootstone.";
				}
			}
			break;
		case S_HUNTING:
			if(level == 1) {
				if(ItemDefs[I_FUR].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					   "to produce furs in designated regions. Furs may be "
					   "produced in many types of regions, generally in "
					   "small quantities.";
				if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using spears.";
				}
			} else if(level == 3) {
				if(ItemDefs[I_FLOATER].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order to "
					   "produce floater hide by hunting floaters, a rare "
					   "and skittish denizen of the swamps. (Note that a "
					   "Hunting skill of 3 is required to even determine "
					   "that floater hides may be produced).  Floaters "
					   "actually weigh less than air, since much of their "
					   "body is made up of gaseous sacs.";
				if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using spears.";
				}
				if(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED)){
					*str += " Floater hide is prized by high level "
						   "shipbuilders who use the hide to make hot air "
						   "balloons.";
				}
				if(!(SkillDefs[S_CARPENTER].flags & SkillType::DISABLED) &&
				   !(ItemDefs[I_GLIDER].flags & ItemType::DISABLED)) {
					*str += " Floater hide is prized by high level "
						   "carpenters who use the hide in making gliders.";
				}
			}
			break;
		case S_FISHING:
			if(level == 1) {
				if(ItemDefs[I_FISH].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					    "to produce fish in designated regions. Fish are "
					    "only available in ocean regions.";
				if(!(ItemDefs[I_NET].flags & ItemType::DISABLED)) {
					*str += " A unit with this skill may also PRODUCE nets "
						    "from herbs. Production can be increased by using "
						    "spinning wheels.";
				}
			}
			break;
		case S_HERBLORE:
			if(level == 1) {
				if(ItemDefs[I_HERBS].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					    "to produce herbs in designated regions.  Herbs "
					    "are found in many types of regions, but most "
					    "frequently in jungles.";
				if(!(SkillDefs[S_HEALING].flags & SkillType::DISABLED)) {
					*str += " Herbs may be used by units with the healing "
						    "skill to heal units killed in battle.";
				}
				if(!(ItemDefs[I_LASSO].flags & ItemType::DISABLED)) {
					*str += " A unit with this skill may also PRODUCE lassos. "
						    "Production of lassos may be increased by using "
							"spinning wheels";
				}
				if(!(ItemDefs[I_BAG].flags & ItemType::DISABLED)) {
					*str += " A unit with this skill may also PRODUCE bags. "
						    "Production of bags can be increased by using "
							"spinning wheels.";
				}
			} else if(level == 3) {
				if(ItemDefs[I_MUSHROOM].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					    "to produce mushrooms in designated areas.  Mushrooms "
						"are found in the underworld.  (Note that an "
						"Herblore skill of 3 is required to even determine "
						"that mushrooms may be produced).";
				if(!(ItemDefs[I_HEALPOTION].flags & ItemType::DISABLED) &&
				   !(SkillDefs[S_HEALING].flags & SkillType::DISABLED)) {
					*str += "  Mushrooms may be used by skilled healers to "
						    "create healing potions.  Healing potions may be "
						    "used even by units without a healing skill and "
						    "each potion can heal up to 10 men with the same"
						    "effects as if a healer with herbs treated them.";
				}
			   	if(!(ItemDefs[I_BAG].flags & ItemType::DISABLED)) {
					*str += " Production of mushrooms may be increased "
						    "by using bags.";
				}
			}
			break;
		case S_HORSETRAINING:
			if(level == 1) {
				if(ItemDefs[I_HORSE].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					   "to produce horses in designated regions. Horses are "
					   "found in plains regions.";
				if(!(ItemDefs[I_LASSO].flags & ItemType::DISABLED)) {
					*str += " Production can by increased by using lassos.";
				}
			} else if(level == 5) {
				if(ItemDefs[I_WHORSE].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order to "
					   "produce winged horses. (Note that a Horse Training "
					   "skill of 5 is required to even determine whether "
					   "winged horses exist in a region.) Winged horses "
					   "weigh 50 weight units, and can carry 20 weight "
					   "units in flight.";
				if(!(ItemDefs[I_LASSO].flags & ItemType::DISABLED)) {
					*str += " Production can by increased by using lassos.";
				}
			}
			break;
		case S_WEAPONSMITH:
			if(level == 1) {
				found = 1;
				*str += "A unit with this skill may construct weapons via "
					   "the PRODUCE order.";
				if(!(ItemDefs[I_IRON].flags & ItemType::DISABLED)) {
					if(!(ItemDefs[I_SWORD].flags & ItemType::DISABLED)) {
						*str += " Swords may be produced from iron.";
					}
					if(!(ItemDefs[I_PICK].flags & ItemType::DISABLED)) {
						*str += " Picks may be produced from iron.";
					}
					if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
						*str += " Hammers may be produced from iron. "
							   " Production of iron-based weapons can be "
							   "increased by using hammers.";
					}
				}
				if(!(ItemDefs[I_WOOD].flags & ItemType::DISABLED)) {
					if(!(ItemDefs[I_LONGBOW].flags & ItemType::DISABLED)) {
						*str += " Longbows may be produced from wood.";
					}
					if(!(ItemDefs[I_CROSSBOW].flags & ItemType::DISABLED)) {
						*str += " Crossbows may be produced from wood.";
					}
					if(!(ItemDefs[I_SPEAR].flags & ItemType::DISABLED)) {
						*str += " Spears may be produced from wood.";
					}
					if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
						*str += " Axes may be produced from wood."
							   " Production of wood-based weapons can be "
							   "increased by using axes.";
					}
				}
			} else if (level == 2) {
				if(ItemDefs[I_BAXE].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_WOOD].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_IRON].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
						"to produce battle axes from 1 unit each of iron and "
						"wood.  Battle axes give units a +4 bonus in combat "
						"but may only strike every other round due to their "
						"mass.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += "  Production can be increased by using hammers.";
				}
			} else if (level == 3) {
				if(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_MSWORD].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					   "to produce mithril swords from mithril. Mithril "
					   "swords give units a +4 bonus in combat rather than "
					   "the usual +2.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += "  Production can be increased by using hammers.";
				}
			} else if(level == 4) {
				if(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_MCROSSBOW].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE "
					"order to produce magic crossbows from ironwood.";
				if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
					*str += "Production can be increased by using axes.";
				}
				*str += ". A magic crossbow fires as if the target has "
					   "a defensive skill of 0, and fires one shot"
					   "per round.  It takes one unit of ironwood to "
					   "produce a magic crossbow, which weighs one unit.";
			} else if(level == 5) {
				if(!(ItemDefs[I_YEW].flags & ItemType::DISABLED) &&
				   !(ItemDefs[I_DOUBLEBOW].flags & ItemType::DISABLED) &&
				   (!(SkillDefs[S_CROSSBOW].flags & SkillType::DISABLED) ||
					!(SkillDefs[S_LONGBOW].flags & SkillType::DISABLED))) {
					found = 1;
					*str += "A unit with this skill may use the PRODUCE "
						"order to produce double bows, one of the best "
						"(natural) dealers of death on a battlefield. Double "
						"bows may be used by units skilled in ";
					if(!(SkillDefs[S_CROSSBOW].flags & SkillType::DISABLED)) {
						*str += "Crossbow";
					}
					if(!(SkillDefs[S_LONGBOW].flags & SkillType::DISABLED)) {
						if(!(SkillDefs[S_CROSSBOW].flags&SkillType::DISABLED)){
							*str += " or ";
						}
						*str += "Longbow";
					}
					*str += ". A double bow fires as if the target has "
						"a defensive skill of 0, and fires a number of "
						"shots, per round, equal to the skill level of the "
						"wielder. It takes one unit of yew to produce a "
						"double bow, which weighs one unit.";
					if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
						*str += "  Production can be increased by using axes.";
					}
				}
			}
			break;
		case S_ARMORER:
			if(level == 1) {
				int needand = 0;
				if((ItemDefs[I_CLOTHARMOR].flags & ItemType::DISABLED) &&
			       (ItemDefs[I_LEATHERARMOR].flags & ItemType::DISABLED) &&
				   (ItemDefs[I_CHAINARMOR].flags & ItemType::DISABLED))
					break;
				if((ItemDefs[I_FUR].flags & ItemType::DISABLED) &&
			       (ItemDefs[I_HERBS].flags & ItemType::DISABLED) &&
				   (ItemDefs[I_IRON].flags & ItemType::DISABLED))
					break;
				if((!(ItemDefs[I_CLOTHARMOR].flags & ItemType::DISABLED) &&
					!(ItemDefs[I_HERBS].flags & ItemType::DISABLED)) ||
				   (!(ItemDefs[I_LEATHERARMOR].flags & ItemType::DISABLED) &&
					!(ItemDefs[I_FUR].flags & ItemType::DISABLED)) ||
				   (!(ItemDefs[I_CHAINARMOR].flags & ItemType::DISABLED) &&
					!(ItemDefs[I_IRON].flags & ItemType::DISABLED))) {
					found = 1;
					*str += "A unit with this skill may PRODUCE ";
					if(!(ItemDefs[I_CLOTHARMOR].flags & ItemType::DISABLED) &&
					   !(ItemDefs[I_HERBS].flags & ItemType::DISABLED)) {
						*str += "cloth armor from herbs (provides a 1/6 "
							    "chance of surviving an attack from a normal "
								"weapon in combat and may be worn during "
								"assassination attacks), ";
						needand = 1;
					}
					if(!(ItemDefs[I_LEATHERARMOR].flags&ItemType::DISABLED) &&
					   !(ItemDefs[I_FUR].flags & ItemType::DISABLED)) {
						*str += "leather armor from furs (provides a 1/4 "
							    "chance of surviving an attack from a normal "
								"weapon in combat), ";
						needand = 1;
					}
					if(!(ItemDefs[I_CHAINARMOR].flags & ItemType::DISABLED) &&
						!(ItemDefs[I_IRON].flags & ItemType::DISABLED)) {
						if (needand) *str += "and ";
						*str += "chain armor from iron (provides a 1/3 "
							    "chance of surviving an attack from a normal "
								"weapon in combat).";
					}
					if((!(ItemDefs[I_SPINNING].flags&ItemType::DISABLED) &&
					   (!(ItemDefs[I_LEATHERARMOR].flags&ItemType::DISABLED)||
						!(ItemDefs[I_CLOTHARMOR].flags&ItemType::DISABLED)))) {
						needand = 0;
						*str += " Production of ";
						if(!(ItemDefs[I_CLOTHARMOR].flags&ItemType::DISABLED)){
							*str += "cloth ";
							needand = 1;
						}
						if(!(ItemDefs[I_LEATHERARMOR].flags&
									ItemType::DISABLED)){
							if(needand) *str += "and ";
							*str += "leather ";
						}
						*str += "armor can be increased by using spinning "
							    "wheels.";
					}
					if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED) &&
					   !(ItemDefs[I_CHAINARMOR].flags & ItemType::DISABLED)) {
						*str += " Production of chain armor can be increased "
							   "by using hammers.";
					}
				}
			} else if (level == 3) {
				if(ItemDefs[I_IRON].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_PLATEARMOR].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may PRODUCE plate armor from "
					   "3 units of iron. Plate armor provides a 2/3 chance "
					   "of surviving a successful attack in battle.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += " Production of plate armor can be increased "
						   "by using hammers.";
				}
			} else if (level == 5) {
				if(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_MPLATE].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may PRODUCE mithril armor "
					   "from one unit of mithril. Mithril armor provides "
					   "a 9/10 chance of surviving a successful attack in "
					   "battle from a normal weapon and a 2/3 chance of "
					   "surviving an attack from a good weapon.  Mithril "
					   "armor weighs one unit.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += " Production of mithril armor can be increased "
						   "by using hammers.";
				}
			}
			break;
		case S_CARPENTER:
			if(level == 1) {
				if(ItemDefs[I_WOOD].flags & ItemType::DISABLED) break;
				if((ItemDefs[I_WAGON].flags & ItemType::DISABLED) &&
				   (ItemDefs[I_SPINNING].flags & ItemType::DISABLED))
					break;
				found = 1;
				*str += "A unit with this skill may PRODUCE ";
				if(!(ItemDefs[I_WAGON].flags & ItemType::DISABLED)) {
					*str += "wagons";
				}
				if(!(ItemDefs[I_SPINNING].flags & ItemType::DISABLED)) {
					if(!(ItemDefs[I_WAGON].flags & ItemType::DISABLED)) {
						*str += " or ";
					}
					*str += "spinning wheels";
				}
				*str += " from wood.";
				if(!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) &&
				   !(ItemDefs[I_HORSE].flags & ItemType::DISABLED)) {
					*str += " A wagon, used with a horse, can carry up to "
						   "200 units of weight.";
				}
				if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using axes.";
				}
			} else if (level == 3) {
				if(ItemDefs[I_MWAGON].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may PRODUCE magic wagons "
					   "from ironwood. A magic wagon ";
				if(!(ItemDefs[I_HORSE].flags & ItemType::DISABLED)) {
					*str += "requires no horse for movement, ";
				}
				*str += "weighs 50 units, and may carry 250 units of "
					   "weight.";
				if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using axes.";
				}
			} else if (level == 5) {
				if(ItemDefs[I_FLOATER].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_GLIDER].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may PRODUCE gliders from "
				       "floater hides. A glider weighs 5 units, and may "
					   "carry 15 units of weight while flying.";
				if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
					*str += " Production can be increased by using axes.";
				}
			}
			break;
		case S_BUILDING:
			if (level == 1) {
				if(!(ItemDefs[I_STONE].flags & ItemType::DISABLED) &&
				   (!(ObjectDefs[O_TOWER].flags & ObjectType::DISABLED) ||
				    !(ObjectDefs[O_FORT].flags & ObjectType::DISABLED) ||
					!(ObjectDefs[O_CASTLE].flags & ObjectType::DISABLED) ||
				    !(ObjectDefs[O_CITADEL].flags & ObjectType::DISABLED) ||
				    !(ObjectDefs[O_STOCKADE].flags & ObjectType::DISABLED))) {
					found = 1;
					*str += "A unit with this skill may use the BUILD order "
						"to construct buildings from stone.  At this level, "
						"the buildings that are able to be built are the ";
					int comma = 0;
					if(!(ObjectDefs[O_TOWER].flags & ObjectType::DISABLED)) {
						*str += "Tower";
						comma = 1;
					}
					if(!(ObjectDefs[O_FORT].flags & ObjectType::DISABLED)) {
						if(comma) *str += ", ";
						*str += "Fort";
						comma = 1;
					}
					if(!(ObjectDefs[O_CASTLE].flags & ObjectType::DISABLED)) {
						if(comma) *str += ", ";
						*str += "Castle";
						comma = 1;
					}
					if(!(ObjectDefs[O_CITADEL].flags & ObjectType::DISABLED)) {
						if(comma) *str += ", and ";
						*str += "Citadel";
					}
					*str += ".";
				}
				if(!(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
				   (!(ObjectDefs[O_STOCKADE].flags & ObjectType::DISABLED))) {
					if(found) *str += " ";
					found = 1;
					*str += "  A unit with this skill may use the BUILD order "
						"to construct Stockades from wood.";
				}
			} else if(level == 2) {
				if(ItemDefs[I_ROOTSTONE].flags & ItemType::DISABLED) break;
				if(ObjectDefs[O_MTOWER].flags & ObjectType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the BUILD order to "
					"construct a Magical Tower from rootstone.";
			} else if(level == 3) {
				if(!(ItemDefs[I_ROOTSTONE].flags & ItemType::DISABLED) &&
				   !(ObjectDefs[O_MFORTRESS].flags & ObjectType::DISABLED)) {
					found = 1;
					*str += "A unit with this skill may use the BUILD order "
						   "to construct magical fortresses from rootstone. "
						   "A magical fortress requires the 160 units of "
						   "rootstone and can protect 250 men. However, "
						   "magical fortresses may be used as the target of "
						   "powerful magical spells";
					if(!(SkillDefs[S_EARTHQUAKE].flags&SkillType::DISABLED)) {
						*str += ", and are not harmed by magical Earthquakes";
					}
					*str += ".";
				}
				if((!(ItemDefs[I_STONE].flags & ItemType::DISABLED) ||
				    !(ItemDefs[I_WOOD].flags & ItemType::DISABLED)) &&
				   !ObjectDefs[O_INN].flags & ObjectType::DISABLED) {
					if(found) *str += " ";
					found = 1;
					*str += "A unit with this skill may use the BUILD order "
					       "to produce an Inn from ";
					if(!(ItemDefs[I_STONE].flags & ItemType::DISABLED)) {
						*str += "stone";
					}
					if(!(ItemDefs[I_WOOD].flags & ItemType::DISABLED)) {
						if(!(ItemDefs[I_STONE].flags & ItemType::DISABLED)) {
							*str += " or ";
						}
						*str += "wood";
					}
					*str += ".";
				}
				if(!(ItemDefs[I_STONE].flags & ItemType::DISABLED)) {
					if(!(ObjectDefs[O_TEMPLE].flags&ObjectType::DISABLED)){
						if(found) *str += " ";
						found = 1;
						*str += "A unit with this skill may use the BUILD "
							"order to produce a Temple from stone.";
					}
					if(!(ObjectDefs[O_ROADN].flags&ObjectType::DISABLED)||
					   !(ObjectDefs[O_ROADNW].flags&ObjectType::DISABLED)||
					   !(ObjectDefs[O_ROADNE].flags&ObjectType::DISABLED)||
					   !(ObjectDefs[O_ROADS].flags&ObjectType::DISABLED)||
					   !(ObjectDefs[O_ROADSW].flags&ObjectType::DISABLED)||
					   !(ObjectDefs[O_ROADSE].flags&ObjectType::DISABLED)){
						if(found) *str += " ";
						found = 1;
						*str += "A unit with this skill may use the BUILD "
							"order to produce Roads from stone.";
					}
				}
				if(!(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED) &&
				   !(ObjectDefs[O_HUT].flags & ObjectType::DISABLED)) {
					if(found) *str += " ";
					found = 1;
					*str += "A unit with this skill may use the BULID order "
						"to produce a Hermits Hut from ironwood.";
				}
			}
			break;
		case S_SHIPBUILDING:
			if(level == 1) {
				if(ItemDefs[I_WOOD].flags & ItemType::DISABLED) break;
				if((ObjectDefs[O_LONGBOAT].flags & ObjectType::DISABLED) &&
				   (ObjectDefs[O_CLIPPER].flags & ObjectType::DISABLED) &&
				   (ObjectDefs[O_GALLEON].flags & ObjectType::DISABLED))
					break;
				found = 1;
				*str += "A unit with this skill may use the BUILD order to "
				       "construct different types of ships from wood.  At "
					   "this level, the ships that may be built are ";
				int comma = 0;
				if(!(ObjectDefs[O_LONGBOAT].flags & ObjectType::DISABLED)) {
					comma = 1;
					*str += "Longboats";
				}
				if(!(ObjectDefs[O_CLIPPER].flags & ObjectType::DISABLED)) {
					if(comma) *str += ", ";
					comma = 1;
					*str += "Clippers";
				}
				if(!(ObjectDefs[O_GALLEON].flags & ObjectType::DISABLED)) {
					if(comma) *str += ", and ";
					comma = 1;
					*str += "Galleons";
				}
				*str += ".";
			} else if (level == 3) {
				if(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED) break;
				if(ObjectDefs[O_AGALLEON].flags & ObjectType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the BUILD order to "
					   "construct armored galleons from 75 units of "
					   "ironwood. An armored galleon can carry 1800 weight "
					   "units, as well as offer protection (in the same "
					   "manner as a building) to the first 200 men within.";
			} else if (level == 5) {
				if(!(ItemDefs[I_FLOATER].flags & ItemType::DISABLED) &&
				   !(ObjectDefs[O_BALLOON].flags & ObjectType::DISABLED)) {
					found = 1;
					*str += "A unit with this skill may use the BUILD order "
						"to construct balloons, using 50 units of floater "
						"hide. A balloon functions in the same manner as a "
						"ship, using the SAIL order to move. However, as "
						"opposed to normal ships, a balloon can sail over "
						"land. A balloon requires 10 sailors to sail, and "
						"has a capacity of 800 weight units.";
				}
				if(!(ItemDefs[I_YEW].flags & ItemType::DISABLED) &&
				   !(ObjectDefs[O_WGALLEON].flags & ObjectType::DISABLED)) {
					if(found) *str += " ";
					found = 1;
					*str += "A unit with this skill may use the BUILD order "
						"to construct war galleons from 100 units of yew. "
						"A war galleon can carry 2500 weight units, as well "
						"as offer protection (in the same manner as a "
						"building) to the first 350 men within.";
				}
				   
			}
			break;
		case S_ENTERTAINMENT:
			if(level == 1) {
				found = 1;
				*str += "A unit with this skill may use the ENTERTAIN order "
					   "to generate funds. The amount of silver gained will "
					   "be 20 per man, times the level of the entertainers. "
					   "This amount is limited by the region that the unit "
					   "is in.";
			}
			break;
		case S_TACTICS:
			if(level == 1) {
				found = 1;
				*str += "Tactics allows the unit, and all allies, to gain a "
					   "free round of attacks during battle. The army with "
					   "the highest level tactician in a battle will "
					   "receive this free round; if the highest levels are "
					   "equal, no free round is awarded.";
			}
			break;
		case S_COMBAT:
			if(level == 1) {
				found = 1;
				*str += "This skill gives the unit a bonus in hand to hand "
					   "combat. Also, a unit with this skill may TAX or "
					   "PILLAGE.";
			}
			break;
		case S_RIDING:
			if(level == 1) {
				if(ItemDefs[I_HORSE].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill, if possessing a horse, "
					   "gains a bonus in combat if the battle is in a "
					   "plain, desert, or tundra. This bonus is equal to "
					   "the skill level, up to a maximum of 3.";
			} else if(level == 3) {
				if(ItemDefs[I_WHORSE].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with Riding 3 or above, if possessing a "
					   "winged horse, gains a bonus in combat, equal to "
					   "the unit's skill level.";
			}
			break;
		case S_CROSSBOW:
			if(level == 1) {
				if(ItemDefs[I_CROSSBOW].flags & ItemType::DISABLED &&
				   ItemDefs[I_MCROSSBOW].flags & ItemType::DISABLED &&
				   ItemDefs[I_DOUBLEBOW].flags & ItemType::DISABLED)
					break;
				found = 1;
				*str += "A unit with this skill may use a crossbow, either "
					   "in battle, or to TAX or PILLAGE a region.";
			}
			break;
		case S_LONGBOW:
			if(level == 1) {
				if(ItemDefs[I_LONGBOW].flags & ItemType::DISABLED &&
				   ItemDefs[I_DOUBLEBOW].flags & ItemType::DISABLED)
					break;
				found = 1;
				*str += "A unit with this skill may use a longbow, either "
					   "in battle, or to TAX or PILLAGE a region.";
			}
			break;
		case S_STEALTH:
			if(level == 1) {
				found = 1;
				*str += "A unit with this skill is concealed from being "
					   "seen, except by units with an Observation skill "
					   "greater than or equal to the stealthy unit's "
					   "Stealth level.";
			}
			break;
		case S_OBSERVATION:
			if(level == 1) {
				found = 1;
				*str += "A unit with this skill can see stealthy units "
					   "whose stealth rating is less than or equal to "
					   "the observing unit's Observation level. The unit "
					   "can also determine the faction owning a unit, "
					   "provided its Observation level is higher than "
					   "the other unit's Stealth level.";
			}
			break;
		case S_HEALING:
			if(level == 1) {
				found = 1;
				*str += "A unit with this skill can use herbs to heal "
					   "units after a battle has been won.";
			} else if(level == 3) {
				if(ItemDefs[I_MUSHROOM].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_HERBS].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_HEALPOTION].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					    "to produce healing potions.  Healing potions may be "
						"used even by units without a healing skill and "
						"each potion can heal up to 10 men with the same "
						"effects as if a healer with herbs treated them.  A "
						"healing potion weighs one weight unit and will be "
						"used automaticlaly if needed.  The potion is "
						"expended if any men are healed using it.";
			}
			break;
		case S_SAILING:
			if(level == 1) {
				found = 1;
				*str += "A unit with this skill may use the SAIL order "
					   "to sail ships.";
			}
			break;
		case S_FORCE:
			if(level == 1) {
				found = 1;
				*str += "The Force skill is not directly useful to a mage, "
					   "but is rather one of the Foundation skills on which "
					   "other magical skills are based. The Force skill "
					   "determines the power of the magical energy that a "
					   "mage is able to use. Note that a Force skill "
					   "level of 0 does not indicate that a mage cannot "
					   "use magical energy, but rather can only perform "
					   "magical acts that do not require great amounts of "
					   "power.";
			}
			break;
		case S_PATTERN:
			if(level == 1) {
				found = 1;
				*str += "The Pattern skill is not directly useful to a mage, "
					   "but is rather one of the Foundation skills on which "
					   "other magical skills are based. A mage's Pattern "
					   "skill indicates the ability to handle complex "
					   "magical patterns, and is important for complicated "
					   "tasks such as healing and controlling nature.";
			}
			break;
		case S_SPIRIT:
			if(level == 1) {
				found = 1;
				*str += "The Spirit skill is not directly useful to a mage, "
					   "but is rather one of the Foundation skills on which "
					   "other magical skills are based. Spirit skill "
					   "indicates the mage's ability to control and affect "
					   "magic and other powers beyond the material world.";
			}
			break;
		case S_FIRE:
			if(level == 1) {
				found = 1;
				*str += "A unit with this magic skill may throw Fireballs "
					   "in combat, inflicting Energy attacks on the enemy. "
					   "The Fireball will inflict from 2 to the mage's Fire "
					   "skill times 10 attacks, at a skill level equal to "
					   "the mage's Fire skill. In order to use this spell, "
					   "the mage should use the COMBAT order to set the "
					   "spell as his combat spell.";
			}
			break;
		case S_EARTHQUAKE:
			if(level == 1) {
				found = 1;
				*str += "A mage with this skill may create violent "
					   "Earthquakes in battle. Earthquakes are dangerous "
					   "only to those enemies within a building. An "
					   "Earthquake will inflict between 2 and the mage's "
					   "Earthquake skill level times 100 attacks, at a "
					   "skill level equal to the mage's Earthquake "
					   "skill; these attacks will only be directed against "
					   "enemies within a building. Note that the defense "
					   "bonus that normally applies to units within a "
					   "building does not apply during an Earthquake "
					   "strike. To use this spell, a mage should set it as "
					   "his combat spell using the COMBAT order.";
			}
			break;
		case S_FORCE_SHIELD:
			if(level == 1) {
				found = 1;
				*str += "A mage with this skill may cast a Force Shield in "
					   "combat.";
				if(!(ItemDefs[I_LONGBOW].flags & ItemType::DISABLED) ||
				   !(ItemDefs[I_CROSSBOW].flags & ItemType::DISABLED) ||
				   !(ItemDefs[I_MCROSSBOW].flags & ItemType::DISABLED) ||
				   !(ItemDefs[I_DOUBLEBOW].flags & ItemType::DISABLED)) {
					*str += " This shield will be effective against all "
						   "bow attacks against the mage's army, at a level "
						   "equal to the mage's skill level.";
				}
				*str += " The mage himself will gain a defensive bonus "
					   "against normal combat attacks, equal to his skill. "
					   "To use this spell in combat, set it using the "
					   "COMBAT order.";
			}
			break;
		case S_ENERGY_SHIELD:
			if(level == 1) {
				found = 1;
				*str += "A mage with this skill may cast an Energy "
					   "Shield in combat. This shield will be effective "
					   "against all Energy attacks against the mage's "
					   "army, at a level equal to the mage's skill level. "
					   "To use this spell in combat, set it using the "
					   "COMBAT order.";
			}
			break;
		case S_SPIRIT_SHIELD:
			if(level == 1) {
				found = 1;
				*str += "A mage with this skill may cast a Spirit "
					   "Shield in combat. This shield will be effective "
					   "against all Spirit attacks against the mage's army, "
					   "at a level equal to the mage's skill level. To use "
					   "this spell in combat, set it using the COMBAT order.";
			}
			break;
		case S_MAGICAL_HEALING:
			if(level == 1) {
				found = 1;
				*str += "A mage with this skill may heal casualties "
					   "after battle";
				if(!(ItemDefs[I_HERBS].flags & ItemType::DISABLED)) {
					*str += " without the aid of herbs.";
					if(!(SkillDefs[S_HEALING].flags & SkillType::DISABLED)) {
						*str += " Other than not needing herbs, this healing "
							   "works the same as normal healing.";
					}
				} else {
					if(!(SkillDefs[S_HEALING].flags & SkillType::DISABLED)) {
						*str += " This healing works the same as normal "
							   "healing.";
					}
				}
				*str += " A mage at this level can heal up to 10 "
					   "casualties, with a 50 percent rate of success. "
					   "No order is necessary to use this spell, it will "
					   "be used automatically when the mage is involved in "
					   "battle.";
			} else if(level == 3) {
				found = 1;
				*str += "A mage with Magical Healing of 3 can work greater "
					   "wonders of healing with his new found powers; he "
					   "may heal up to 25 casualties, with a success "
					   "rate of 75 percent.";
			} else if(level == 5) {
				found = 1;
				*str += "A mage with Magical Healing of 5 can bring "
					   "soldiers back from near death; he may heal up "
					   "to 100 casualties, with a 90 percent rate of success.";
			}
			break;
		case S_GATE_LORE:
			if(level == 1) {
				found = 1;
				*str += "Gate Lore is the art of detecting and using "
					   "magical Gates, which are spread through the "
					   "world. The Gates are numbered in order, but "
					   "spread out randomly, so there is no correlation "
					   "between the Gate number and the Gate's location. "
					   "A mage with skill 1 in Gate Lore can see a Gate "
					   "if one exists in the same region as the mage. "
					   "This detection is automatic; the Gate will appear "
					   "in the region report. A mage with skill 1 in "
					   "Gate Lore may also jump through a Gate into "
					   "another region containing a gate, selected at "
					   "random. To use Gate Lore in this manner, use the "
					   "syntax CAST Gate_Lore RANDOM UNITS <unit> ... "
					   "UNITS is followed by a list of units to follow "
					   "the mage through the Gate (the mage always jumps "
					   "through the Gate). At level 1, the mage may "
					   "carry 15 weight units through the Gate (including "
					   "the weight of the mage).";
			} else if (level == 2) {
				found = 1;
				*str += "A mage with Gate Lore skill 2 can detect Gates "
					   "in adjacent regions. The mage should use the "
					   "syntax CAST Gate_Lore DETECT in order to detect "
					   "these nearby Gates. Also, at level 2 Gate Lore, "
					   "the mage may carry 100 weight units through a "
					   "Gate when doing a random jump.";
			} else if(level == 3) {
				found = 1;
				*str += "A mage with Gate Lore skill 3 and higher can step "
					   "through a Gate into another region containing a "
					   "specific Gate. To use this spell, use the syntax "
					   "CAST Gate_Lore GATE <number> UNITS <unit> ... "
					   "<number> specifies the Gate that the mage will "
					   "jump to. UNITS is followed by a list of units "
					   "to follow the mage through the gate (the mage "
					   "always jumps through the gate). At level 3, the "
					   "mage may carry 15 weight units through the Gate "
					   "(including the mage). Also, a level 3 or higher "
					   "mage doing a random gate jump may carry 1000 weight "
					   "units through the Gate.";
			} else if(level == 4) {
				found = 1;
				*str += "A mage with Gate Lore skill 4 may carry 100 "
					   "weight units through a Gate.";
			} else if(level == 5) {
				found = 1;
				*str += "A mage with Gate Lore skill 5 may carry 1000 "
					   "weight units through a Gate.";
			}
			break;
		case S_PORTAL_LORE:
			if(level == 1) {
				if(ItemDefs[I_PORTAL].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A mage with the Portal Lore skill may, with the "
					   "aid of another mage, make a temporary Gate between "
					   "two regions, and send units from one region to " 
					   "another. In order to do this, both mages (the "
					   "caster, and the target mage) must have Portals, "
					   "and the caster must be trained in Portal Lore. The "
					   "caster may teleport units weighing up to 50 weight "
					   "units times his skill level, to the target mage's "
					   "region, provided the target region is no farther "
					   "than 2 times the caster's skill level squared "
					   "regions away. To use this skill, CAST Portal_Lore "
					   "<target> UNITS <unit> ..., where <target> is the "
					   "unit number of the target mage, and <unit> is a "
					   "list of units to be teleported (the casting mage "
					   "may teleport himself, if he so desires).";
			}
			break;
		case S_FARSIGHT:
			if(level == 1) {
				found = 1;
				if(!Globals->EASIER_UNDERWORLD) {
					*str += "A mage with the Farsight skill may obtain "
						   "region reports on distant regions. This skill "
						   "only works on the surface of the world, but "
						   "the report will be as if the mage was in the "
						   "region himself. The mage may view regions that "
						   "are within the mage's skill level squared "
						   "times 4 regions of the mage's region. To use "
						   "this skill, CAST Farsight REGION <x> <y>, where "
						   "<x> and <y> are the coordinates of the region "
						   "that you wish to view. Note that Farsight does "
						   "not work in conjunction with other spells; the "
						   "mage can only rely on his normal facilities "
						   "while casting Farsight.";
				} else {
					*str += "A mage with the Farsight skill may obtain "
						   "region reports on distant regions. The report "
						   "will be as if the mage was in the region "
						   "himself. The mage may view regions that are "
						   "within the mage's skill level squared times 4 "
						   "regions of the mage's region. Distances in the "
						   "underworld are doubled for this calculation, "
						   "while attempting to view from the surface to "
						   "the underworld (or vice-versa) increases the "
						   "distance by 4. To use this skill, CAST "
						   "Farsight REGION <x> <y> <z>, where <x>, <y> "
						   "and <z> are the coordinates of the region that "
						   "you wish to view. If you omit the <z> "
						   "coordinate the <z> coordinate of the caster's "
						   "current regions will be used. The <z> "
						   "coordinate for the surface is '1' and the "
						   "<z>-coordinate for the underworld is '2'. "
						   "Note that Farsight does not work in or to "
						   "the Nexus or in conjunction with other "
						   "spells; the mage can only rely on his normal "
						   "facilities while casting Farsight.";
				}
			}
			break;
		case S_MIND_READING:
			if(level == 1) {
				found = 1;
				*str += "A mage with Mind Reading skill 1 may cast the "
					   "spell and determine the faction affiliation of "
					   "any unit he can see. To use the spell in this "
					   "manner, CAST Mind_Reading <unit>, where <unit> "
					   "is the target unit.";
			} else if (level == 3) {
				found = 1;
				*str += "A mage with Mind Reading skill 3 will "
					   "automatically determine the faction affiliation "
					   "of any unit he can see. Usage of this skill is "
					   "automatic, and no order is needed to use it.";
			} else if (level == 5) {
				found = 1;
				*str += "A mage with Mind Reading skill 5 can get a full "
					   "unit report on any unit he can see. To use this "
					   "skill, CAST Mind_Reading <unit> where <unit> is "
					   "the target unit.";
			}
			break;
		case S_TELEPORTATION:
			if(level == 1) {
				found = 1;
				if(!Globals->EASIER_UNDERWORLD) {
					*str += "A mage with the Teleportation skill may "
						   "teleport himself across great distances on "
						   "the surface of the world, even without the "
						   "use of a gate. A mage with this skill may "
						   "teleport 15 weight units per skill level to "
						   "a region that is within the mage's skill "
						   "level squared times 2 regions of the mages "
						   "current region. The Teleportation skill only "
						   "works on the surface of the world. The syntax "
						   "to use this skill is CAST Teleportation REGION "
						   "<x> <y>. <x> and <y> are the coordinates of "
						   "the region to teleport to.";
				} else {
					*str += "A mage with the Teleportation skill may "
						   "teleport himself across great distances even "
						   "without the use of a gate. A mage with this "
						   "skill may teleport 15 weight units per skill "
						   "level, to a region that is within the mage's "
						   "skill level squared times 2 regions of the "
						   "mages current region. Distances in the "
						   "underworld are doubled for this calculation, "
						   "while attempting to teleport from the surface "
						   "to the underworld (or vice-versa) increases "
						   "the distance by 4. To use this skill, CAST "
						   "Teleportation REGION <x> <y> <z>, where <x>, "
						   "<y> and <z> are the coordinates of the region "
						   "that you wish to teleport to. If you omit the "
						   "<z> coordinate the <z> coordinate of the "
						   "caster's current regions will be used. The "
						   "<z> coordinate for the surface is '1' and the "
						   "<z>-coordinate for the underworld is '2'. "
						   "Note that the Teleportation skill will not work "
						   "in or out of the Nexus.";
				}
			}
			break;
		case S_WEATHER_LORE:
			if(level == 1) {
				found = 1;
				*str += "Weather Lore is the magic of the weather; the "
					   "skill Weather Lore itself has no use, except to "
					   "allow further study into more powerful areas of "
					   "magic. A mage with Weather Lore skill will perceive "
					   "the use of Weather Lore by any other mage in the "
					   "same region.";
			}
			break;
		case S_SUMMON_WIND:
			if(level == 1) {
				found = 1;
				*str += "A mage with knowledge of Summon Wind can summon "
					   "up the powers of the wind to aid him in sea or "
					   "air travel. Usage of this spell is automatic.";
				if(!(ObjectDefs[O_LONGBOAT].flags & ObjectType::DISABLED)) {
					*str += " At level 1, if the mage is in a Longboat, that "
						   "ship will get 2 extra movement points.";
				}
				*str += " If the mage is flying, he will receive 2 extra "
					   "movement points.";
			} else if (level == 2) {
				if(ObjectDefs[O_CLIPPER].flags & ObjectType::DISABLED) break;
				found = 1;
				*str += "With level 2 Summon Wind, any ship of Clipper size "
					   "or smaller that the mage is inside will receive a "
					   "2 movement point bonus.";
			} else if (level == 3) {
				found = 1;
				*str += "At level 3 of Summon Wind, any ship the mage is in "
					   "will receive a 2 movement point bonus. Note that "
					   "study of Summon Wind beyond level 3 does not "
					   "yield any further powers.";
			}
			break;
		case S_SUMMON_STORM:
			if(level == 1) {
				found = 1;
				*str += "A mage with Summon Storm can summon a storm "
					   "against enemy forces, reducing their combat "
					   "effectiveness. Each round, the storm will affect "
					   "from 2 to the Summon Storm level of the mage "
					   "times 50, and will reduce their effective Combat "
					   "skill by 2 for the duration of the battle. To use "
					   "this spell, the mage should set it as his combat "
					   "spell using the COMBAT order.";
			}
			break;
		case S_SUMMON_TORNADO:
			if(level == 1) {
				found = 1;
				*str += "A mage with Summon Tornado can summon a mighty "
					   "tornado to use against enemy forces in battle. A "
					   "tornado issues a number of Weather attacks from "
					   "2 to the mage's Summon Tornado level times 50, "
					   "of skill level equal to the mage's level. To use "
					   "this spell, use the COMBAT order to set it as "
					   "the mage's combat spell.";
			}
			break;
		case S_CALL_LIGHTNING:
			if(level == 1) {
				found = 1;
				*str += "The Call Lightning spell is a combat spell, "
					   "allowing the mage to summon mighty blasts of "
					   "lightning from the sky against his foes. A "
					   "lightning blast deals a number of attacks from "
					   "4 to the mage's skill level times 120, of skill "
					   "level equal to the mage's level. This spell is "
					   "a bit different than most combat spells, in that "
					   "half of the attacks are treated as Weather "
					   "attacks, and the other half as Energy attacks. To "
					   "use the spell, use the COMBAT order to set it as "
					   "the mage's combat spell.";
			}
			break;
		case S_CLEAR_SKIES:
			if(level == 1) {
				found = 1;
				*str += "The Clear Skies spell has two uses. When cast "
					   "using the CAST order, it causes the region to "
					   "have good weather for the entire month; movement "
					   "is at the normal rate (even if it is winter, and "
					   "movement costs would normally be doubled), and "
					   "the economic production of the region is "
					   "improved for a month (this improvement of the "
					   "economy will actually take effect during the turn "
					   "after the spell is cast). To use the spell in this "
					   "fashion, CAST Clear_Skies; no arguments are "
					   "necessary. The other use of the spell is in "
					   "combat, as a defense against weather attacks. If "
					   "the mage sets this spell as his combat spell "
					   "using the COMBAT order, it will function as a "
					   "shield against weather attacks, at a level equal "
					   "to the mage's Clear Skies skill.";
			}
			break;
		case S_EARTH_LORE:
			if(level == 1) {
				found = 1;
				*str += "Earth Lore is the study of nature, plants, and "
					   "animals. A mage with knowledge of Earth Lore can "
					   "use his knowledge of nature to aid local farmers, "
					   "raising money for himself, and aiding the "
					   "production of grain or livestock in the region. "
					   "To use the spell, CAST Earth_Lore; the mage will "
					   "receive an amount of money based on his level, and "
					   "the economy of the region. Also, a mage with "
					   "knowledge of Earth Lore will detect the use of "
					   "Earth Lore by any other mage in the same region.";
			}
			break;
		case S_WOLF_LORE:
			if(level == 1) {
				found = 1;
				*str += "A mage with Wolf Lore skill may summon wolves, "
					   "who will fight for him in combat. A mage may "
					   "summon a number of wolves equal to his skill "
					   "level, and control a total number of his skill "
					   "level squared times 4 wolves; the wolves will "
					   "be placed in the mages inventory. Note, however, "
					   "that wolves may only be summoned in mountain and "
					   "forest regions. To summon wolves, the mage should "
					   "issue the order CAST Wolf_Lore.";
			}
			break;
		case S_BIRD_LORE:
			if(level == 1) {
				found = 1;
				*str += "A mage with Bird Lore may control the birds of "
					   "the sky. At skill level 1, the mage can control "
					   "small birds, sending them to an adjacent region "
					   "to obtain a report on that region. (This skill "
					   "only works on the surface of the world, as there "
					   "are no birds elsewhere). To use this skill, CAST "
					   "Bird_Lore DIRECTION <dir>, where <dir> is "
					   "the direction the mage wishes the birds to report on.";
			} else if (level == 3) {
				found = 1;
				*str += "A mage with Bird Lore 3 can summon eagles to join "
					   "him, who will aid him in combat, and provide for "
					   "flying transportation. A mage may summon a number "
					   "of eagles equal to his skill level minus 2, "
					   "squared; the eagles will appear in his inventory. "
					   "To summon an eagle, issue the order CAST Bird_Lore "
					   "EAGLE.";
			}
			break;
		case S_DRAGON_LORE:
			if(level == 1) {
				found = 1;
				*str += "A mage with Dragon Lore skill can summon dragons "
					   "to join him, to aid in battle, and provide "
					   "flying transportation. A mage at level 1 has a "
					   "low chance of successfully summoning a dragon, "
					   "gradually increasing until at level 5 he may "
					   "summon one dragon per turn; the total number of "
					   "dragons that a mage may control at one time is "
					   "equal to his skill level. To attempt to summon a "
					   "dragon, CAST Dragon_Lore.";
			}
			break;
		case S_NECROMANCY:
			if(level == 1) {
				found = 1;
				*str += "Necromancy is the magic of death; a mage versed "
					   "in Necromancy may raise and control the dead, and "
					   "turn the powers of death to his own nefarious "
					   "purposes. The Necromancy skill does not have any "
					   "direct application, but is required to study the "
					   "more powerful Necromatic skills. A mage with "
					   "knowledge of Necromancy will detect the use of "
					   "Necromancy by any other mage in the same region.";
			}
			break;
		case S_SUMMON_SKELETONS:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Summon Skeletons skill may "
					   "summon skeletons into his inventory, to aid him "
					   "in battle. Skeletons may be given to other units, "
					   "as they follow instructions mindlessly; however, "
					   "they have a 10 percent chance of decaying each "
					   "turn. A mage can summon skeletons at an average "
					   "rate of 40 percent times his level squared. To use "
					   "the spell, use the order CAST Summon_Skeletons, "
					   "and the mage will summon as many skeletons as he "
					   "is able.";
			}
			break;
		case S_RAISE_UNDEAD:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Raise Undead skill may summon "
					   "undead into his inventory, to aid him in battle. "
					   "Undead may be given to other units, as they follow "
					   "instructions mindlessly; however, they have a 10 "
					   "percent chance of decaying each turn. A mage can "
					   "summon undead at an average rate of 10 percent "
					   "times his level squared. To use the spell, use "
					   "the order CAST Raise_Undead and the mage will "
					   "summon as many undead as he is able.";
			}
			break;
		case S_SUMMON_LICH:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Summon Lich skill may summon a "
					   "lich into his inventory, to aid him in battle. "
					   "Liches may be given to other units, as they "
					   "follow instructions mindlessly; however, they "
					   "have a 10 percent chance of decaying each turn. "
					   "A mage has a 2 percent chance times his level "
					   "squared chance of summoning a lich; to summon a "
					   "lich, use the order CAST Summon_Lich.";
			}
			break;
		case S_CREATE_AURA_OF_FEAR:
			if(level == 1) {
				found = 1;
				*str += "Create Aura of Fear is a combat spell; a mage who "
					   "casts it will project an aura of fear on his "
					   "enemies, reducing their combat effectiveness by "
					   "2. A mage with this skill can strike fear into the "
					   "hearts of 2 to his skill level times 20 men; "
					   "note that this spell does not affect any type "
					   "of creature. To use this spell, the mage should "
					   "set it as his combat spell; the attacks are Spirit "
					   "attacks, and may be blocked by a Spirit shield.";
			}
			break;
		case S_SUMMON_BLACK_WIND:
			if(level == 1) {
				found = 1;
				*str += "This spell allows the mage to summon the black "
					   "wind, a supernatural wind that destroys life. A "
					   "mage who casts this spell in combat will deal from "
					   "2 to his skill level times 200 Spirit attacks, at "
					   "a level equal to the mage's skill level. However, "
					   "the black wind has no effect on undead or demons. "
					   "To use this spell, the mage should set it using "
					   "the COMBAT order.";
			}
			break;
		case S_BANISH_UNDEAD:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Banish Undead skill can banish "
					   "undead in combat. The mage will deal from 2 to "
					   "his skill level times 50 attacks on undead, with no "
					   "defense against these attacks. To use this spell, "
					   "the mage should set it using the COMBAT order.";
			}
			break;
		case S_DEMON_LORE:
			if(level == 1) {
				found = 1;
				*str += "Demon Lore is the art of summoning and controlling "
					   "demons. The Demon Lore skill does not give the mage "
					   "any direct skills, but is required to study further "
					   "into the Demonic arts. A mage with knowledge of "
					   "Demon Lore will detect the use of Demon Lore by any "
					   "other mage in the same region.";
			}
			break;
		case S_SUMMON_IMPS:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Summon Imps skill may summon imps "
					   "into his inventory, to aid him in combat. A mage "
					   "may summon one imp per skill level; however, the "
					   "imps have a chance of breaking free of the mage's "
					   "control at the end of each turn. This chance is "
					   "based on the number of imps in the mage's control; "
					   "if the mage has his skill level squared times 4 "
					   "imps, the chance is 5 percent; this chance goes "
					   "up or down quickly if the mage controls more or "
					   "fewer imps. To use this spell, the mage should "
					   "issue the order CAST Summon_Imps, and the mage will "
					   "summon as many imps as he is able.";
			}
			break;
		case S_SUMMON_DEMON:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Summon Demon skill may summon "
					   "demons into his inventory, to aid him in combat. "
					   "A mage may summon one demon each turn; however, "
					   "the demons have a chance of breaking free of the "
					   "mage's control at the end of each turn. This chance "
					   "is based on the number of demons in the mage's "
					   "control; if the mage has a number of demons equal "
					   "to his skill level squared, the chance is 5 "
					   "percent; this chance goes up or down quickly if "
					   "the mage controls more or fewer demons. To use "
					   "this spell, the mage should issue the order CAST "
					   "Summon_Demon.";
			}
			break;
		case S_SUMMON_BALROG:
			if (level == 1) {
				found = 1;
			   *str += "A mage with the Summon Balrog skill may summon a "
				      "balrog into his inventory, to aid him in combat. "
					  "A mage has a 20 percent times his skill level chance "
					  "of summoning a balrog, but may only control one "
					  "balrog at a time. A with other demons, the balrog "
					  "has a chance of breaking free of the mage's "
					  "control at the end of each turn. This chance is equal "
					  "to 1 over 4 times the mage's skill level squared "
					  "(or, from 1 over 4 at level 1, to 1 over 100 at "
					  "level 5). To use this spell, the mage should "
					  "issue the order CAST Summon_Balrog.";
			}
			break;
		case S_BANISH_DEMONS:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Banish Demons skill can banish "
					   "demons in combat. The mage will deal from 2 to "
					   "his skill level times 50 attacks on demons, with "
					   "no defense against these attacks. To use this "
					   "spell, the mage should set it using the COMBAT order.";
			}
			break;
		case S_ILLUSION:
			if(level == 1) {
				found = 1;
				*str += "Illusion is the magic of creating images of "
					   "things that do not actually exist. The Illusion "
					   "skill does not have any direct applications, but "
					   "is required for further study of Illusionary magic. "
					   "A mage with knowledge of the Illusion skill will "
					   "detect the use of Illusion by any other mage in "
					   "the same region.";
			}
			break;
		case S_PHANTASMAL_ENTERTAINMENT:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Phantasmal Entertainment skill "
					   "may use his powers of Illusion to earn money by "
					   "creating illusionary fireworks, puppet shows, "
					   "etc. In effect, Phantasmal Entertainment grants "
					   "the mage Entertainment skill equal to five times "
					   "his Phantasmal Entertainment level. To use this "
					   "skill, use the ENTERTAIN order.";
			}
			break;
		case S_CREATE_PHANTASMAL_BEASTS:
			if(level == 1) {
				found = 1;
				*str += "A mage with Create Phantasmal Beasts may summon "
					   "illusionary beasts that appear in the mage's "
					   "inventory. These beasts will fight in combat, but "
					   "do not attack, and are killed whenever they are "
					   "attacked. Create Phantasmal Beasts at level 1 "
					   "allows the mage to summon illusionary wolves; the "
					   "number the mage can summon, or have in his "
					   "inventory at one time is equal to the mage's skill "
					   "squared times 4. To use this spell, the mage should "
					   "CAST Create_Phantasmal_Beasts WOLF <number>, "
					   "where <number> is the number of wolves that the "
					   "mage wishes to have appear in his inventory. "
					   "Note: illusionary beasts will appear on reports as "
					   "if they were normal items, except on the owner's "
					   "report, where they are marked as illusionary. To "
					   "reference these items in orders, you must prepend "
					   "an 'i' to the normal string. (Example: to reference "
					   "an illusionary wolf, you would use 'iwolf').";
			} else if (level == 3) {
				found = 1;
				*str += "Create Phantasmal Beasts at level 3 allows the "
					   "mage to summon illusionary eagles into his "
					   "inventory. To summon illusionary eagles, the "
					   "mage should CAST Create_Phantasmal_Beasts EAGLE "
					   "<number>, where <number> is the number of eagles "
					   "that the mage wishes to have appear in his "
					   "inventory. The number of eagles that a mage may "
					   "have in his inventory is equal to his skill level, "
					   "minus 2, squared.";
			} else if(level == 5) {
				found = 1;
				*str += "Create Phantasmal Beasts at level 5 allows the "
					   "mage to summon an illusionary dragon into his "
					   "inventory. To summon an illusionary dragon, the "
					   "mage should CAST Create_Phantasmal_Beasts DRAGON; "
					   "the mage can only have one illusionary dragon in "
					   "his inventory at one time.";
			}
			break;
		case S_CREATE_PHANTASMAL_UNDEAD:
			if(level == 1) {
				found = 1;
				*str += "A mage with Create Phantasmal Undead may summon "
					   "illusionary undead that appear in the mage's "
					   "inventory. These undead will fight in combat, but "
					   "do not attack, and are killed whenever they are "
					   "attacked. Create Phantasmal Undead at level 1 "
					   "allows the mage to summon illusionary skeletons; "
					   "the number the mage can summon, or have in his "
					   "inventory at one time is equal to the mage's skill "
					   "squared times 4. To use this spell, the mage "
					   "should CAST Create_Phantasmal_Undead SKELETON "
					   "<number>, where <number> is the number of skeletons "
					   "that the mage wishes to have appear in his "
					   "inventory. Note: illusionary undead will appear "
					   "on reports as if they were normal items, except "
					   "on the owner's report, where they are marked as "
					   "illusionary. To reference these items in orders, "
					   "you must prepend an 'i' to the normal string. "
					   "(Example: to reference an illusionary skeleton, "
					   "you would use 'iskeleton').";
			} else if(level == 3) {
				found = 1;
				*str += "Create Phantasmal Undead at level 3 allows the "
					   "mage to summon illusionary undead into his "
					   "inventory. To summon illusionary undead, the mage "
					   "should CAST Create_Phantasmal_Undead UNDEAD "
					   "<number>, where <number> is the number of undead "
					   "that the mage wishes to have appear in his "
					   "inventory. The number of undead that a mage may "
					   "have in his inventory is equal to his skill level, "
					   "minus 2, squared.";
			} else if (level == 5) {
				found = 1;
				*str += "Create Phantasmal Undead at level 5 allows the "
					   "mage to summon an illusionary lich into his "
					   "inventory. To summon an illusionary lich, the mage "
					   "should CAST Create_Phantasmal_Undead LICH; the mage "
					   "can only have one illusionary lich in his inventory "
					   "at one time.";
			}
			break;
		case S_CREATE_PHANTASMAL_DEMONS:
			if(level == 1) {
				found = 1;
				*str + "A mage with Create Phantasmal Demons may summon "
					  "illusionary demons that appear in the mage's "
					  "inventory. These demons will fight in combat, but "
					  "do not attack, and are killed whenever they are "
					  "attacked. Create Phantasmal Demons at level 1 "
					  "allows the mage to summon illusionary imps; the "
					  "number the mage can summon, or have in his inventory "
					  "at one time is equal to the mage's skill squared "
					  "times 4. To use this spell, the mage should CAST "
					  "Create_Phantasmal_Demons IMP <number>, where "
					  "<number> is the number of imps that the mage "
					  "wishes to have appear in his inventory. Note: "
					  "illusionary demons will appear on reports as if "
					  "they were normal items, except on the owner's "
					  "report, where they are marked as illusionary. To "
					  "reference these items in orders, you must "
					  "prepend an 'i' to the normal string. (Example: "
					  "to reference an illusionary demon, you would use "
					  "'idemon').";
			} else if (level == 3) {
				found = 1;
				*str += "Create Phantasmal Demons at level 3 allows the "
					   "mage to summon illusionary demons into his "
					   "inventory. To summon illusionary demons, the "
					   "mage should CAST Create_Phantasmal_Demons DEMON "
					   "<number>, where <number> is the number of demons "
					   "that the mage wishes to have appear in his "
					   "inventory. The number of demons that a mage may "
					   "have in his inventory is equal to his skill level, "
					   "minus 2, squared.";
			} else if (level == 5) {
				found = 1;
				*str += "Create Phantasmal Demons at level 5 allows the "
					   "mage to summon an illusionary balrog into his "
					   "inventory. To summon an illusionary balrog, the "
					   "mage should CAST Create_Phantasmal_Demons BALROG; "
					   "the mage can only have one illusionary balrog in "
					   "his inventory at one time.";
			}
			break;
		case S_INVISIBILITY:
			if(level == 1) {
				found = 1;
				*str += "The Invisibility skill allows a mage to render "
					   "other units nearly invisible to other factions, "
					   "giving them a +3 bonus to Stealth. This "
					   "invisibility will last until the next Magic round. "
					   "To cast this spell, use the order CAST Invisibility "
					   "UNITS <unit> ..., where <unit> is a list of the "
					   "units that the mage wishes to render invisible. A "
					   "mage may render invisible a number of men or "
					   "creatures equal to his skill level squared.";
			}
			break;
		case S_TRUE_SEEING:
			if(level == 1) {
				found = 1;
				*str += "A mage with the True Seeing spell can see "
					   "illusions for what they really are. Whether or "
					   "not the mage can see through the illusion depends "
					   "on his True Seeing skill being higher that the "
					   "Illusion skill of the mage casting the illusion. "
					   "This spell does not require any order to use; it "
					   "is used automatically. In addition, a mage with "
					   "the True Seeing skill receives a bonus to his "
					   "Observation skill equal to his True Seeing skill "
					   "divided by 2, rounded up.";
			}
			break;
		case S_DISPEL_ILLUSIONS:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Dispel Illusions spell can dispel "
					   "illusionary foes in combat. Dispelling an "
					   "illusionary foe always destroys the foe; there is "
					   "no defense against it. To use this spell, the mage "
					   "should set it as his combat spell; Dispel "
					   "Illusions will dispel between 2 and the mage's "
					   "level times 100 illusions.";
			}
			break;
		case S_ARTIFACT_LORE:
			if(level == 1) {
				found = 1;
				*str += "Artifact Lore is one of the most advanced forms of "
					   "magic; in general, creation of an artifact requires "
					   "both Artifact Lore, and the appropriate skill for "
					   "the item being created. A mage with knowledge of "
					   "the Artifact Lore skill will detect the use of "
					   "Artifact Lore by any other mage in the region.";
			}
			break;
		case S_CREATE_RING_OF_INVISIBILITY:
			if (level == 1) {
				found = 1;
				*str += "A mage with the Create Ring of Invisibility skill "
					   "may create a Ring of Invisibility, which grants a "
					   "3 bonus to a unit's effective Stealth (note that "
					   "a unit must possess one ring for each man to gain "
					   "this bonus). A Ring of Invisibility has one "
					   "limitation; a unit possessing a ring cannot "
					   "assassinate, nor steal from, a unit with an "
					   "amulet of true seeing. A mage has a 20 percent "
					   "times his level chance to create a Ring of "
					   "Invisibility. To use this spell, the mage should "
					   "CAST Create_Ring_of_Invisibility; it costs 600 "
					   "silver to make the attempt to create a ring.";
			}
			break;
		case S_CREATE_CLOAK_OF_INVULNERABILITY:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Cloak of Invulnerability "
					   "skill may create a Cloak of Invulnerability. A "
					   "Cloak of Invulernerability grants the wearer a "
					   "99 percent chance of surviving any attack. A mage "
					   "has a 20 percent times his level chance to create "
					   "a Cloak of Invulnerability. To use this spell, "
					   "the mage should CAST "
					   "Create_Cloak_of_Invulnerability; it costs 800 "
					   "silver to attempt to create a cloak.";
			}
			break;
		case S_CREATE_STAFF_OF_FIRE:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Staff of Fire skill may "
					   "create a Staff of Fire. A Staff of Fire allows any "
					   "mage to throw fireballs in combat as if he had a "
					   "Fire skill of 3. A mage has a 20 percent times "
					   "his level chance to create a Staff of Fire; the "
					   "attempt costs 600 silver. To use this spell, "
					   "CAST Create_Staff_of_Fire.";
			}
			break;
		case S_CREATE_STAFF_OF_LIGHTNING:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Staff of Lightning skill "
					   "may create a Staff of Lightning. A Staff of "
					   "Lightning allows any mage to call down lightning "
					   "bolts as if he had Call Lightning skill of 3. A "
					   "mage has a 20 percent chance times his level "
					   "chance to create a Staff of Lightning; the attempt "
					   "costs 1000 silver. To use this spell, CAST "
					   "Create_Staff_of_Lightning.";
			}
			break;
		case S_CREATE_AMULET_OF_TRUE_SEEING:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Amulet of Tree Seeing skill "
					   "may create an Amulet of True Seeing. This amulet "
					   "gives the possessor a bonus of 2 to his effective "
					   "Observation skill. Also, a unit with an Amulet of "
					   "True Seeing cannot be assassinated by, nor have "
					   "items stolen by, a unit with a Ring of "
					   "Invisibility (Note that the unit must have at "
					   "least one Amulet of True Seeing per man to repel "
					   "a unit with a Ring of Invisibility). A mage has "
					   "a 20 percent chance times his skill level chance "
					   "to create an Amulet of True Seeing; the attempt "
					   "costs 600 silver. To use this spell, CAST "
					   "Create_Amulet_of_True_Seeing.";
			}
			break;
		case S_CREATE_AMULET_OF_PROTECTION:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Amulet of Protection skill "
					   "may create Amulets of Protection, which grants the "
					   "possesor a personal Spirit Shield of 3. A mage may "
					   "create his skill level of these amulets per turn. "
					   "To use this spell, CAST "
					   "Create_Amulet_of_Protection; this spell costs 200 "
					   "silver to cast.";
			}
			break;
		case S_CREATE_RUNESWORD:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Runesword skill may create "
					   "a Runesword, which when wielded in combat gives "
					   "the wielder a plus 4 bonus to Combat skill, and "
					   "also allows the wielder to project an Aura of Fear "
					   "in battle, as if he had Create Aura of Fear skill "
					   "of level 3 (provided the wielder is not casting "
					   "any other combat spells). A mage has a 20 percent "
					   "times his skill level chance of creating a "
					   "Runesword; the attempt costs 800 silver. To cast "
					   "this spell, CAST Create_Runesword.";
			}
			break;
		case S_CREATE_SHIELDSTONE:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Shieldstone skill may "
					   "create Shieldstones, which confers upon the "
					   "possessor a personal Energy Shield of 3. A mage "
					   "may create his skill level in Shieldstones per "
					   "turn. To use this spell, CAST Create_Shieldstone; "
					   "this spell costs 200 silver to cast.";
			}
			break;
		case S_CREATE_MAGIC_CARPET:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Create Magic Carpet skill may "
					   "create Magic Carpets, which provide for flying "
					   "transportation. A Magic Carpet can carry up to "
					   "15 weight units in the air. Casting this spell "
					   "costs 400 silver, and allows the mage to create "
					   "his skill level in Magic Carpets; to cast the "
					   "spell, CAST Create_Magic_Carpet.";
			}
			break;
		case S_ENGRAVE_RUNES_OF_WARDING:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Engrave Runes of Warding may "
					   "engrave runes of warding on a building; these "
					   "runes will give any occupants of the building "
					   "a personal Energy Shield and Spirit Shield, "
					   "both at level 3.  At level 1 the mage has a 20 "
					   "percent chance of succeeding with each attempt to "
					   "cast this spell. To use this spell, the mage "
					   "should CAST Engrave_Runes_of_Warding, and be "
					   "within the building he wishes to engrave runes "
					   "upon; also, this spell costs 600 silver to cast.";
				if(!(ObjectDefs[O_TOWER].flags & ObjectType::DISABLED)) {
					*str += " At level 1, the mage may engrave runes of "
						   "warding upon a Tower.";
				}
			} else if (level == 2) {
				found = 1;
				*str += "At level 2, the mage has a 40 percent chance of "
					   "success each time he casts this spell.";
				if(!(ObjectDefs[O_FORT].flags & ObjectType::DISABLED)) {
					*str += " At this level, the mage may engrave runes of "
						   "warding upon a Fort.";
				}
			} else if (level == 3) {
				found = 1;
				*str += "At level 3, the mage has a 60 percent chance of "
					   "success each time he casts this spell.";
				if(!(ObjectDefs[O_CASTLE].flags & ObjectType::DISABLED)) {
					*str += " At this level, the mage may engrave runes of "
						   "warding upon a Castle.";
				}
			} else if (level == 4) {
				found = 1;
				*str += "At level 4, the mage has a 80 percent chance of "
					   "success each time he casts this spell.";
				if(!(ObjectDefs[O_CITADEL].flags & ObjectType::DISABLED)) {
					*str += " At this level, the mage may engrave runes of "
						   "warding upon a Citadel.";
				}
			} else if(level == 5) {
				found = 1;
				*str += "At level 5, the mage has a 100 percent chance of "
					   "success each time he casts this spell.";
				if(!(ObjectDefs[O_MFORTRESS].flags & ObjectType::DISABLED)) {
					*str += " At this level, the mage may engrave runes of "
						   "warding upon a Magical Fortress, which grants "
						   "the inhabitants an Energy Shield and Spirit "
						   "Shield at level 5.";
				}
			}
			break;
		case S_CONSTRUCT_GATE:
			if(level == 1) {
				found = 1;
				*str += "A mage with the Construct Gate skill may construct "
					   "a Gate in a region. The mage has a 20 percent times "
					   "his skill level chance of success, and the attempt "
					   "costs 1000 silver. To use this spell, the mage "
					   "should issue the order CAST Construct_Gate.";
			}
			break;
		case S_ENCHANT_SWORDS:
			if(level == 1) {
				if(ItemDefs[I_MSWORD].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_SWORD].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A mage with the Enchant Swords skill may "
					   "enchant normal swords into mithril swords, "
					   "which give a +4 bonus to Combat skill when "
					   "used in combat. A mage may enchant 5 times his "
					   "skill level swords per turn; the swords should "
					   "be in the mage's inventory for him to enchant "
					   "them. The mage should issue the order CAST "
					   "Enchant_Swords to cast this spell.";
			}
			break;
		case S_ENCHANT_ARMOR:
			if(level == 1) {
				if(ItemDefs[I_MPLATE].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_PLATEARMOR].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A mage with the Enchant Armor skill may enchant "
					   "normal plate armor into mithril armor, which give "
					   "the wearer a 9/10 chance of surviving a successful "
					   "attack in battle from a normal weapon and a 2/3 "
					   "chance of surviving an attack from a good weapon. "
					   "A mage may enchant 5 times his skill level "
					   "plate armors per turn; the armor should be in "
					   "the mage's inventory for him to enchant them. The "
					   "mage should issue the order CAST Enchant_Armor "
					   "to cast this spell.";
			}
			break;
		case S_CONSTRUCT_PORTAL:
			if(level == 1) {
				if(ItemDefs[I_PORTAL].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A mage with the Construct Portal skill may "
					   "construct a Portal, for use with the Portal "
					   "Lore skill. The mage has a 20 percent times his "
					   "skill level chance of creating a Portal, and the "
					   "attempt costs 600 silver. To use this spell, CAST "
					   "Construct_Portal.";
			}
			break;
		case S_MANIPULATE:
			if(level == 1) {
				found = 1;
				*str += "A unit with this skill becomes an apprentice mage. "
					   "While apprentices cannot cast spells directly, "
					   "they can use magic items normally only usable by "
					   "mages. Continued study of this skill gives no "
					   "further advantages.";
			}
			break;
		case S_WEAPONCRAFT:
			if (level == 1) {
				*str += "The weaponcraft skill is an advanced version of the "
					    "weaponsmith skill.";
				found = 1;
				if(!(ItemDefs[I_LANCE].flags & ItemType::DISABLED) &&
				   !(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
				   !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
					*str += " A unit with this skill may use the PRODUCE "
						    "order to produce lances from wood.  A lance "
							"attacks as if the target has a defensive skill "
							"of their riding skill (as modified by mount "
							"and terrain) if they are mounted, or 0 if they "
							"are not mounted.   A lance requires riding "
							"skill to wield.  It takes two units of wood "
							"to produce a lance, which weighs two units.  "
							"A unit wielding a lance attacks with a combat "
							"skill equal to their mount bonus + 5.";
					if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
						*str += " Production can be increased by using axes.";
					}
				}
			} else if(level == 2) {
				if(ItemDefs[I_MBAXE].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_IRONWOOD].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
						"to produce mithril battle axes from 1 unit each of "
						"mithril and ironwood.  Mithril battle axes give "
						"units a +6 bonus in combat but may only strike "
						"every other round due to their mass.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += "  Production can be increased by using hammers.";
				}
			} else if (level == 3) {
				if(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_ADSWORD].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					   "to produce admantium swords from admantium.  "
					   "Admantium swords give units a +6 bonus in combat "
					   "rather than the usual +2, and cut more easily "
					   "through armor due to their sharpness.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += "  Production can be increased by using hammers.";
				}
			} else if(level == 4) {
				if(!(ItemDefs[I_YEW].flags & ItemType::DISABLED) &&
				   !(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) &&
				   !(ItemDefs[I_SUPERBOW].flags & ItemType::DISABLED) &&
				   (!(SkillDefs[S_CROSSBOW].flags & SkillType::DISABLED) ||
					!(SkillDefs[S_LONGBOW].flags & SkillType::DISABLED))) {
					found = 1;
					*str += "A unit with this skill may use the PRODUCE "
						"order to produce super bows, the ultimate "
						"(natural) dealer of death on a battlefield.  Super"
						"bows may be used by units skilled in ";
					if(!(SkillDefs[S_CROSSBOW].flags & SkillType::DISABLED)) {
						*str += "Crossbow";
					}
					if(!(SkillDefs[S_LONGBOW].flags & SkillType::DISABLED)) {
						if(!(SkillDefs[S_CROSSBOW].flags&SkillType::DISABLED)){
							*str += " or ";
						}
						*str += "Longbow";
					}
					*str += ". A super bow fires as if the target has "
						"a defensive skill of 0, and fires a number of "
						"shots, per round, equal to the skill level of the "
						"wielder.  Super bows add a +2 to the bow skill of "
						"the unit.  It takes two units of yew and one unit "
						"of mithril to produce a super bow, which weighs "
						"one unit.";
					if(!(ItemDefs[I_AXE].flags & ItemType::DISABLED)) {
						*str += "  Production can be increased by using axes.";
					}
				}
			} else if(level == 5) {
				if(ItemDefs[I_ADBAXE].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_YEW].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
						"to produce admantium battle axes from 1 unit each of "
						"admantium and yew.  Admantium battle axes give "
						"units a +8 bonus in combat but may only strike "
						"every other round due to their mass, however they "
						"do cut more easily through armor due to their "
						"sharpness.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += "  Production can be increased by using hammers.";
				}
			}
			break;
		case S_ARMORCRAFT:
			if(level == 1) {
				*str += "The armorcraft skill is an advanced version of the "
					    "armorsmith skill.";
				found = 1;
				if(!(ItemDefs[I_MITHRIL].flags & ItemType::DISABLED) &&
				   !(ItemDefs[I_MPLATE].flags & ItemType::DISABLED) &&
				   !(ItemDefs[I_IMARM].flags & ItemType::DISABLED)) {
					*str += "  A unit with this skill may PRODUCE improved "
						    "mithril armor from one unit of mithril and an "
							"existing suit of mithril armor.  Improved "
							"mithril armor provides a 9/10 chance of "
							"surviving a successful attack in battle from "
							"a normal weapon, and a 3/4 chance of surviving "
							"an attack from a good weapon. Improved mithril "
							"armor weighs one unit.";
					if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
						*str += " Production of improved mithril armor can be "
							    "increased by using hammers.";
					}
				}
			} else if(level == 3) {
				if(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_ADRING].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may PRODUCE admantium "
						"ring mail from one unit of admantium.  Admantium "
						"ring mail provides a 9/10 chance of "
						"surviving a successful attack in battle from "
						"a normal weapon, and a 4/5 chance of surviving "
						"an attack from a good weapon. Admantium ring mail "
						"weighs one unit.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += " Production of admantium ring mail can be "
							"increased by using hammers.";
				}
			} else if(level == 5) {
				if(ItemDefs[I_ADMANTIUM].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_PLATEARMOR].flags & ItemType::DISABLED) break;
				if(ItemDefs[I_ADPLATE].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may PRODUCE admantium "
						"plate mail from three units of admantium and an "
						"existing suit of plate mail.  Admantium plate mail "
						"provides a 95/100 chance of surviving a successful "
						"attack in battle from a normal weapon, and a 9/10 "
						"chance of surviving an attack from a good "
						"weapon. Admantium plate mail weighs one unit.";
				if(!(ItemDefs[I_HAMMER].flags & ItemType::DISABLED)) {
					*str += " Production of admantium plate mail can be "
							"increased by using hammers.";
				}
			}
			break;
		case S_CAMELTRAINING:
			if(level == 1) {
				if(ItemDefs[I_CAMEL].flags & ItemType::DISABLED) break;
				found = 1;
				*str += "A unit with this skill may use the PRODUCE order "
					    "to produce camels in designated regions.  Camels "
						"can be found in desert regions.   Camels weigh "
						"20 weight units and can carry 70 weight units at "
						"walking or riding speeds.";
				if(!(ItemDefs[I_LASSO].flags & ItemType::DISABLED)) {
					*str += " Production of camels can be increased using "
						    "lassos.";
				}
			}
			break;
	}
	if(!found) *str += "No skill report.";
	return str;
}
