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
#include "game.h"
#include "gamedata.h"

using namespace std;

static int RandomiseSummonAmount(int num)
{
	int retval, i;

	retval = 0;

	for (i = 0; i < 2 * num; i++)
	{
		if (rng::get_random(2))
			retval++;
	}
	if (retval < 1 && num > 0)
		retval = 1;

	return retval;
}

void Game::ProcessCastOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
	AString * token = o->gettoken();
	if (!token) {
		parse_error( pCheck, u, 0, "CAST: No skill given.");
		return;
	}

	int sk = ParseSkill(token);
	delete token;
	if (sk==-1) {
		parse_error( pCheck, u, 0, "CAST: Invalid skill.");
		return;
	}

	if ( !( SkillDefs[sk].flags & SkillType::MAGIC )) {
		parse_error( pCheck, u, 0, "CAST: That is not a magic skill.");
		return;
	}
	if ( !( SkillDefs[sk].flags & SkillType::CAST )) {
		parse_error( pCheck, u, 0, "CAST: That skill cannot be CAST.");
		return;
	}

	RangeType *rt = NULL;
	if ( !pCheck ) {
		//
		// XXX -- should be error checking spells
		//
		switch(sk) {
			case S_MIND_READING:
				ProcessMindReading(u,o, pCheck );
				break;
			case S_CONSTRUCT_PORTAL:
			case S_ENCHANT_SWORDS:
			case S_ENCHANT_ARMOR:
			case S_ENCHANT_SHIELDS:
			case S_CONSTRUCT_GATE:
			case S_ENGRAVE_RUNES_OF_WARDING:
			case S_SUMMON_IMPS:
			case S_SUMMON_DEMON:
			case S_SUMMON_BALROG:
			case S_SUMMON_SKELETONS:
			case S_RAISE_UNDEAD:
			case S_SUMMON_LICH:
			case S_DRAGON_LORE:
			case S_WOLF_LORE:
			case S_EARTH_LORE:
			case S_SUMMON_WIND:
			case S_CREATE_RING_OF_INVISIBILITY:
			case S_CREATE_CLOAK_OF_INVULNERABILITY:
			case S_CREATE_STAFF_OF_FIRE:
			case S_CREATE_STAFF_OF_LIGHTNING:
			case S_CREATE_AMULET_OF_TRUE_SEEING:
			case S_CREATE_AMULET_OF_PROTECTION:
			case S_CREATE_RUNESWORD:
			case S_CREATE_SHIELDSTONE:
			case S_CREATE_MAGIC_CARPET:
			case S_CREATE_FLAMING_SWORD:
			case S_CREATE_FOOD:
			case S_CREATE_AEGIS:
			case S_CREATE_WINDCHIME:
			case S_CREATE_GATE_CRYSTAL:
			case S_CREATE_STAFF_OF_HEALING:
			case S_CREATE_SCRYING_ORB:
			case S_CREATE_CORNUCOPIA:
			case S_CREATE_BOOK_OF_EXORCISM:
			case S_CREATE_HOLY_SYMBOL:
			case S_CREATE_CENSER:
			case S_BLASPHEMOUS_RITUAL:
			case S_PHANTASMAL_ENTERTAINMENT:
				ProcessGenericSpell(u,sk, pCheck );
				break;
			case S_CLEAR_SKIES:
				rt = FindRange(SkillDefs[sk].range);
				if (rt == NULL)
					ProcessGenericSpell(u, sk, pCheck);
				else
					ProcessRegionSpell(u, o, sk, pCheck);
				break;
			case S_FARSIGHT:
			case S_TELEPORTATION:
			case S_WEATHER_LORE:
				ProcessRegionSpell(u, o, sk, pCheck);
				break;
			case S_BIRD_LORE:
				ProcessBirdLore(u,o, pCheck );
				break;
			case S_INVISIBILITY:
				ProcessInvisibility(u,o, pCheck );
				break;
			case S_GATE_LORE:
				ProcessCastGateLore(u,o, pCheck );
				break;
			case S_PORTAL_LORE:
				ProcessCastPortalLore(u,o, pCheck );
				break;
			case S_CREATE_PHANTASMAL_BEASTS:
				ProcessPhanBeasts(u,o, pCheck );
				break;
			case S_CREATE_PHANTASMAL_UNDEAD:
				ProcessPhanUndead(u,o, pCheck );
				break;
			case S_CREATE_PHANTASMAL_DEMONS:
				ProcessPhanDemons(u,o, pCheck );
				break;
			case S_TRANSMUTATION:
				ProcessTransmutation(u, o, pCheck);
				break;
		}
	}
}

void Game::ProcessMindReading(Unit *u,AString *o, OrdersCheck *pCheck )
{
	UnitId *id = ParseUnit(o);

	if (!id) {
		u->error("CAST: No unit specified.");
		return;
	}

	CastMindOrder *order = new CastMindOrder;
	order->id = id;
	order->spell = S_MIND_READING;
	order->level = 1;

	u->ClearCastOrders();
	u->castorders = order;
}

void Game::ProcessBirdLore(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();

	if (!token) {
		u->error("CAST: Missing arguments.");
		return;
	}

	if (*token == "eagle") {
		CastIntOrder *order = new CastIntOrder;
		order->spell = S_BIRD_LORE;
		order->level = 3;
		u->ClearCastOrders();
		u->castorders = order;
		return;
	}

	if (*token == "direction") {
		delete token;
		token = o->gettoken();

		if (!token) {
			u->error("CAST: Missing arguments.");
			return;
		}

		int dir = ParseDir(token);
		delete token;
		if (dir == -1 || dir > NDIRS) {
			u->error("CAST: Invalid direction.");
			return;
		}

		CastIntOrder *order = new CastIntOrder;
		order->spell = S_BIRD_LORE;
		order->level = 1;
		order->target = dir;
		u->ClearCastOrders();
		u->castorders = order;

		return;
	}

	u->error("CAST: Invalid arguments.");
	delete token;
}

void Game::ProcessInvisibility(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();

	if (!token || !(*token == "units")) {
		u->error("CAST: Must specify units to render invisible.");
		return;
	}
	delete token;

	CastUnitsOrder *order;
	if (u->castorders && u->castorders->type == O_CAST &&
		u->castorders->spell == S_INVISIBILITY && u->castorders->level == 1
	) {
		order = dynamic_cast<CastUnitsOrder *>(u->castorders);
	} else {
		order = new CastUnitsOrder;
		order->spell = S_INVISIBILITY;
		order->level = 1;
		u->ClearCastOrders();
		u->castorders = order;
	}

	UnitId *id = ParseUnit(o);
	while (id) {
		order->units.push_back(id);
		id = ParseUnit(o);
	}
}

void Game::ProcessPhanDemons(Unit *u,AString *o, OrdersCheck *pCheck )
{
	CastIntOrder *order = new CastIntOrder;
	order->spell = S_CREATE_PHANTASMAL_DEMONS;
	order->level = 0;
	order->target = 1;

	AString *token = o->gettoken();

	if (!token) {
		u->error("CAST: Illusion to summon must be given.");
		delete order;
		return;
	}

	if (*token == "imp" || *token == "imps") {
		order->level = 1;
	}

	if (*token == "demon" || *token == "demons") {
		order->level = 2;
	}

	if (*token == "balrog" || *token == "balrogs") {
		order->level = 3;
	}

	delete token;

	if (!order->level) {
		u->error("CAST: Can't summon that illusion.");
		delete order;
		return;
	}

	token = o->gettoken();

	if (!token) {
		order->target = 1;
	} else {
		order->target = token->value();
		delete token;
	}

	u->ClearCastOrders();
	u->castorders = order;
}

void Game::ProcessPhanUndead(Unit *u,AString *o, OrdersCheck *pCheck)
{
	CastIntOrder *order = new CastIntOrder;
	order->spell = S_CREATE_PHANTASMAL_UNDEAD;
	order->level = 0;
	order->target = 1;

	AString *token = o->gettoken();

	if (!token) {
		u->error("CAST: Must specify which illusion to summon.");
		delete order;
		return;
	}

	if (*token == "skeleton" || *token == "skeletons") {
		order->level = 1;
	}

	if (*token == "undead") {
		order->level = 2;
	}

	if (*token == "lich" || *token == "liches") {
		order->level = 3;
	}

	delete token;

	if (!order->level) {
		u->error("CAST: Must specify which illusion to summon.");
		delete order;
		return;
	}

	token = o->gettoken();

	if (token) {
		order->target = token->value();
		delete token;
	} else {
		order->target = 1;
	}

	u->ClearCastOrders();
	u->castorders = order;
}

void Game::ProcessPhanBeasts(Unit *u,AString *o, OrdersCheck *pCheck )
{
	CastIntOrder *order = new CastIntOrder;
	order->spell = S_CREATE_PHANTASMAL_BEASTS;
	order->level = 0;
	order->target = 1;

	AString *token = o->gettoken();

	if (!token) {
		u->error("CAST: Must specify which illusion to summon.");
		delete order;
		return;
	}

	if (*token == "wolf" || *token == "wolves") {
		order->level = 1;
	}
	if (*token == "eagle" || *token == "eagles") {
		order->level = 2;
	}
	if (*token == "dragon" || *token == "dragon") {
		order->level = 3;
	}

	delete token;
	if (!order->level) {
		delete order;
		u->error("CAST: Must specify which illusion to summon.");
		return;
	}

	token = o->gettoken();
	if (token) {
		order->target = token->value();
		delete token;
	}

	u->ClearCastOrders();
	u->castorders = order;
}

void Game::ProcessGenericSpell(Unit *u,int spell, OrdersCheck *pCheck )
{
	CastOrder *orders = new CastOrder;
	orders->spell = spell;
	orders->level = 1;
	u->ClearCastOrders();
	u->castorders = orders;
}

void Game::ProcessRegionSpell(Unit *u, AString *o, int spell,
		OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	int x = -1;
	int y = -1;
	int z = -1;
	RangeType *range = FindRange(SkillDefs[spell].range);

	if (token) {
		if (*token == "region") {
			delete token;
			token = o->gettoken();
			if (!token) {
				u->error("CAST: Region X coordinate not specified.");
				return;
			}
			x = token->value();
			delete token;

			token = o->gettoken();
			if (!token) {
				u->error("CAST: Region Y coordinate not specified.");
				return;
			}
			y = token->value();
			delete token;

			if (range && (range->flags & RangeType::RNG_CROSS_LEVELS)) {
				token = o->gettoken();
				if (token) {
					z = token->value();
					delete token;
					if (z < 0 || (z >= Globals->UNDERWORLD_LEVELS +
								Globals->UNDERDEEP_LEVELS +
								Globals->ABYSS_LEVEL + 2)) {
						u->error("CAST: Invalid Z coordinate specified.");
						return;
					}
				}
			}
		} else {
			delete token;
		}
	}
	if (x == -1) x = u->object->region->xloc;
	if (y == -1) y = u->object->region->yloc;
	if (z == -1) z = u->object->region->zloc;

	CastRegionOrder *order;
	if (spell == S_TELEPORTATION)
		order = new TeleportOrder;
	else
		order = new CastRegionOrder;
	order->spell = spell;
	order->level = 1;
	order->xloc = x;
	order->yloc = y;
	order->zloc = z;

	u->ClearCastOrders();
	/* Teleports happen late in the turn! */
	if (spell == S_TELEPORTATION)
		u->teleportorders = dynamic_cast<TeleportOrder *>(order);
	else
		u->castorders = order;
}

void Game::ProcessCastPortalLore(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();
	if (!token) {
		u->error("CAST: Requires a target mage.");
		return;
	}
	int gate = token->value();
	delete token;
	token = o->gettoken();

	if (!token) {
		u->error("CAST: No units to teleport.");
		return;
	}

	if (!(*token == "units")) {
		u->error("CAST: No units to teleport.");
		delete token;
		return;
	}

	TeleportOrder *order;

	if (u->teleportorders && u->teleportorders->spell == S_PORTAL_LORE) {
		order = u->teleportorders;
	} else {
		order = new TeleportOrder;
		u->ClearCastOrders();
		u->teleportorders = order;
	}

	order->gate = gate;
	order->spell = S_PORTAL_LORE;
	order->level = 1;

	UnitId *id = ParseUnit(o);
	while(id) {
		order->units.push_back(id);
		id = ParseUnit(o);
	}
}

void Game::ProcessCastGateLore(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();

	if (!token) {
		u->error("CAST: Missing argument.");
		return;
	}

	if ((*token) == "gate") {
		delete token;
		token = o->gettoken();

		if (!token || token->value() < 1) {
			u->error("CAST: Requires a target gate.");
			return;
		}

		TeleportOrder *order;

		if (u->teleportorders && u->teleportorders->spell == S_GATE_LORE &&
				u->teleportorders->gate == token->value()) {
			order = u->teleportorders;
		} else {
			order = new TeleportOrder;
			u->ClearCastOrders();
			u->teleportorders = order;
		}

		order->gate = token->value();
		order->spell = S_GATE_LORE;
		order->level = 3;

		delete token;

		token = o->gettoken();

		if (!token) return;
		if (!(*token == "units")) {
			delete token;
			return;
		}

		UnitId *id = ParseUnit(o);
		while(id) {
			order->units.push_back(id);
			id = ParseUnit(o);
		}
		return;
	}

	if ((*token) == "random") {
		TeleportOrder *order;

		if (u->teleportorders &&
				u->teleportorders->spell == S_GATE_LORE &&
				u->teleportorders->gate == -1 ) {
			order = u->teleportorders;
		} else {
			order = new TeleportOrder;
			u->ClearCastOrders();
			u->teleportorders = order;
		}

		order->gate = -1;
		order->spell = S_GATE_LORE;
		order->level = 1;

		delete token;

		token = o->gettoken();

		if (!token) return;
		if (*token == "level") {
			order->gate = -2;
			order->level = 2;
			delete token;
			token = o->gettoken();
		}
		if (!token) return;
		if (!(*token == "units")) {
			delete token;
			return;
		}

		UnitId *id = ParseUnit(o);
		while(id) {
			order->units.push_back(id);
			id = ParseUnit(o);
		}
		return;
	}

	if ((*token) == "detect") {
		delete token;
		u->ClearCastOrders();
		CastOrder *to = new CastOrder;
		to->spell = S_GATE_LORE;
		to->level = 2;
		u->castorders = to;
		return;
	}

	delete token;
	u->error("CAST: Invalid argument.");
}

void Game::ProcessTransmutation(Unit *u, AString *o, OrdersCheck *pCheck)
{
	CastTransmuteOrder *order;
	AString *token;

	order = new CastTransmuteOrder;
	order->spell = S_TRANSMUTATION;
	order->level = 0;
	order->item = -1;
	order->number = -1;

	token = o->gettoken();
	if (!token) {
		u->error("CAST: You must specify what you wish to create.");
		delete order;
		return;
	}
	if (token->value() > 0) {
		order->number = token->value();
		delete token;
		token = o->gettoken();
	}

	order->item = ParseEnabledItem(token);
	delete token;
	if (order->item == -1) {
		u->error("CAST: You must specify what you wish to create.");
		delete order;
		return;
	}

	switch(order->item) {
		case I_MITHRIL:
		case I_ROOTSTONE:
			order->level = 1;
			break;
		case I_IRONWOOD:
			order->level = 2;
			break;
		case I_FLOATER:
			order->level = 3;
			break;
		case I_YEW:
			order->level = 4;
			break;
		case I_WHORSE:
			order->level = 5;
			break;
		case I_ADMANTIUM:
			order->level = 5;
			break;
		default:
			u->error("CAST: Can't create that by transmutation.");
			delete order;
			return;
	}

	u->ClearCastOrders();
	u->castorders = order;

	return;
}

void Game::RunACastOrder(ARegion * r,Object *o,Unit * u)
{
	int val;
	if (u->type != U_MAGE && u->type != U_APPRENTICE) {
		u->error("CAST: Unit is not a mage.");
		return;
	}

	if (u->castorders->level == 0) {
		u->castorders->level = u->GetSkill(u->castorders->spell);
	}

	if (u->GetSkill(u->castorders->spell) < u->castorders->level || u->castorders->level == 0) {
		u->error("CAST: Skill level isn't that high.");
		return;
	}

	int sk = u->castorders->spell;
	switch (sk) {
		case S_MIND_READING:
			val = RunMindReading(r,u);
			break;
		case S_ENCHANT_ARMOR:
			val = RunEnchant(r, u, sk, I_MPLATE);
			break;
		case S_ENCHANT_SWORDS:
			val = RunEnchant(r, u, sk, I_MSWORD);
			break;
		case S_ENCHANT_SHIELDS:
			val = RunEnchant(r, u, sk, I_MSHIELD);
			break;
		case S_CONSTRUCT_GATE:
			val = RunConstructGate(r,u,sk);
			break;
		case S_ENGRAVE_RUNES_OF_WARDING:
			val = RunEngraveRunes(r,o,u);
			break;
		case S_CONSTRUCT_PORTAL:
			val = RunCreateArtifact(r,u,sk,I_PORTAL);
			break;
		case S_CREATE_RING_OF_INVISIBILITY:
			val = RunCreateArtifact(r,u,sk,I_RINGOFI);
			break;
		case S_CREATE_CLOAK_OF_INVULNERABILITY:
			val = RunCreateArtifact(r,u,sk,I_CLOAKOFI);
			break;
		case S_CREATE_STAFF_OF_FIRE:
			val = RunCreateArtifact(r,u,sk,I_STAFFOFF);
			break;
		case S_CREATE_STAFF_OF_LIGHTNING:
			val = RunCreateArtifact(r,u,sk,I_STAFFOFL);
			break;
		case S_CREATE_AMULET_OF_TRUE_SEEING:
			val = RunCreateArtifact(r,u,sk,I_AMULETOFTS);
			break;
		case S_CREATE_AMULET_OF_PROTECTION:
			val = RunCreateArtifact(r,u,sk,I_AMULETOFP);
			break;
		case S_CREATE_RUNESWORD:
			val = RunCreateArtifact(r,u,sk,I_RUNESWORD);
			break;
		case S_CREATE_SHIELDSTONE:
			val = RunCreateArtifact(r,u,sk,I_SHIELDSTONE);
			break;
		case S_CREATE_MAGIC_CARPET:
			val = RunCreateArtifact(r,u,sk,I_MCARPET);
			break;
		case S_CREATE_FLAMING_SWORD:
			val = RunCreateArtifact(r,u,sk,I_FSWORD);
			break;
		case S_SUMMON_IMPS:
			val = RunSummonImps(r,u);
			break;
		case S_SUMMON_DEMON:
			val = RunSummonDemon(r,u);
			break;
		case S_SUMMON_BALROG:
			val = RunSummonBalrog(r,u);
			break;
		case S_SUMMON_LICH:
			val = RunSummonLich(r,u);
			break;
		case S_RAISE_UNDEAD:
			val = RunRaiseUndead(r,u);
			break;
		case S_SUMMON_SKELETONS:
			val = RunSummonSkeletons(r,u);
			break;
		case S_DRAGON_LORE:
			val = RunDragonLore(r,u);
			break;
		case S_BIRD_LORE:
			val = RunBirdLore(r,u);
			break;
		case S_WOLF_LORE:
			val = RunWolfLore(r,u);
			break;
		case S_INVISIBILITY:
			val = RunInvisibility(r,u);
			break;
		case S_CREATE_PHANTASMAL_DEMONS:
			val = RunPhanDemons(r,u);
			break;
		case S_CREATE_PHANTASMAL_UNDEAD:
			val = RunPhanUndead(r,u);
			break;
		case S_CREATE_PHANTASMAL_BEASTS:
			val = RunPhanBeasts(r,u);
			break;
		case S_GATE_LORE:
			val = RunDetectGates(r,o,u);
			break;
		case S_FARSIGHT:
			val = RunFarsight(r,u);
			break;
		case S_PHANTASMAL_ENTERTAINMENT:
			val = RunPhantasmalEntertainment(r,u);
			break;
		case S_EARTH_LORE:
			val = RunEarthLore(r,u);
			break;
		case S_WEATHER_LORE:
			val = RunWeatherLore(r, u);
			break;
		case S_CLEAR_SKIES:
			val = RunClearSkies(r,u);
			break;
		case S_SUMMON_WIND:
			val = RunCreateArtifact(r, u, sk, I_CLOUDSHIP);
			break;
		case S_CREATE_FOOD:
			val = RunCreateArtifact(r, u, sk, I_FOOD);
			break;
		case S_CREATE_AEGIS:
			val = RunCreateArtifact(r,u,sk,I_AEGIS);
			break;
		case S_CREATE_WINDCHIME:
			val = RunCreateArtifact(r,u,sk,I_WINDCHIME);
			break;
		case S_CREATE_GATE_CRYSTAL:
			val = RunCreateArtifact(r,u,sk,I_GATE_CRYSTAL);
			break;
		case S_CREATE_STAFF_OF_HEALING:
			val = RunCreateArtifact(r,u,sk,I_STAFFOFH);
			break;
		case S_CREATE_SCRYING_ORB:
			val = RunCreateArtifact(r,u,sk,I_SCRYINGORB);
			break;
		case S_CREATE_CORNUCOPIA:
			val = RunCreateArtifact(r,u,sk,I_CORNUCOPIA);
			break;
		case S_CREATE_BOOK_OF_EXORCISM:
			val = RunCreateArtifact(r,u,sk,I_BOOKOFEXORCISM);
			break;
		case S_CREATE_HOLY_SYMBOL:
			val = RunCreateArtifact(r,u,sk,I_HOLYSYMBOL);
			break;
		case S_CREATE_CENSER:
			val = RunCreateArtifact(r,u,sk,I_CENSER);
			break;
		case S_TRANSMUTATION:
			val = RunTransmutation(r, u);
			break;
		case S_BLASPHEMOUS_RITUAL:
			val = RunBlasphemousRitual(r, u);
			break;
	}
	if (val) {
		u->Practice(sk);
		r->NotifySpell(u, SkillDefs[sk].abbr, regions);
	}
}

int Game::GetRegionInRange(ARegion *r, ARegion *tar, Unit *u, int spell)
{
	int level = u->GetSkill(spell);
	if (!level) {
		u->error("CAST: You don't know that spell.");
		return 0;
	}

	RangeType *range = FindRange(SkillDefs[spell].range);
	if (range == NULL) {
		u->error("CAST: Spell is not castable at range.");
		return 0;
	}

	int rtype = regions.GetRegionArray(r->zloc)->levelType;
	if ((rtype == ARegionArray::LEVEL_NEXUS) &&
			!(range->flags & RangeType::RNG_NEXUS_SOURCE)) {
		u->error("CAST: Spell does not work from the Nexus.");
		return 0;
	}

	if (!tar) {
		u->error("CAST: No such region.");
		return 0;
	}

	rtype = regions.GetRegionArray(tar->zloc)->levelType;
	if ((rtype == ARegionArray::LEVEL_NEXUS) &&
			!(range->flags & RangeType::RNG_NEXUS_TARGET)) {
		u->error("CAST: Spell does not work to the Nexus.");
		return 0;
	}

	if ((rtype != ARegionArray::LEVEL_SURFACE) &&
			(range->flags & RangeType::RNG_SURFACE_ONLY)) {
		u->error("CAST: Spell can only target regions on the surface.");
		return 0;
	}
	if (!(range->flags&RangeType::RNG_CROSS_LEVELS) && (r->zloc != tar->zloc)) {
		u->error("CAST: Spell is not able to work across levels.");
		return 0;
	}

	int maxdist;
	switch(range->rangeClass) {
		default:
		case RangeType::RNG_ABSOLUTE:
			maxdist = 1;
			break;
		case RangeType::RNG_LEVEL:
			maxdist = level;
			break;
		case RangeType::RNG_LEVEL2:
			maxdist = level * level;
			break;
		case RangeType::RNG_LEVEL3:
			maxdist = level * level * level;
			break;
	}
	maxdist *= range->rangeMult;

	int dist;
	dist = regions.GetPlanarDistance(tar, r, range->crossLevelPenalty, maxdist);
	if (dist > maxdist) {
		u->error("CAST: Target region out of range.");
		return 0;
	}
	return 1;
}

int Game::RunMindReading(ARegion *r,Unit *u)
{
	CastMindOrder *order = dynamic_cast<CastMindOrder *>(u->castorders);
	int level = u->GetSkill(S_MIND_READING);

	Unit *tar = r->GetUnitId(order->id,u->faction->num);
	if (!tar) {
		u->error("No such unit.");
		return 0;
	}

	string temp = "Casts Mind Reading: " + string(tar->name->const_str()) + ", " + tar->faction->name->const_str();

	if (level < 3) {
		u->event(temp + ".", "spell");
		return 1;
	}

	temp += string(tar->items.Report(2,5,0).const_str()) + ". Skills: ";
	temp += string(tar->skills.Report(tar->GetMen()).const_str()) + ".";

	u->event(temp, "spell");
	return 1;
}

int Game::RunEnchant(ARegion *r,Unit *u, int skill, int item)
{
	int level, max, num, i, a;
	unsigned int c;

	level = u->GetSkill(skill);
	max = ItemDefs[item].mOut * level / 100;
	num = max;

	// Figure out how many we can make based on available resources
	for (c=0; c<sizeof(ItemDefs[item].mInput)/sizeof(ItemDefs[item].mInput[0]); c++) {
		if (ItemDefs[item].mInput[c].item == -1)
			continue;
		i = ItemDefs[item].mInput[c].item;
		a = ItemDefs[item].mInput[c].amt;
		if (u->GetSharedNum(i) < num * a) {
			num = u->GetSharedNum(i) / a;
		}
	}

	// collect all the materials
	for (c=0; c<sizeof(ItemDefs[item].mInput)/sizeof(ItemDefs[item].mInput[0]); c++) {
		if (ItemDefs[item].mInput[c].item == -1)
			continue;
		i = ItemDefs[item].mInput[c].item;
		a = ItemDefs[item].mInput[c].amt;
		u->ConsumeShared(i, num * a);
	}

	// Add the created items
	u->items.SetNum(item, u->items.GetNum(item) + num);
	u->event("Enchants " + to_string(num) + " " + ItemDefs[item].names + ".", "spell");
	if (num == 0) return 0;
	return 1;
}

int Game::RunConstructGate(ARegion *r,Unit *u, int spell)
{
	int ngates, log10, *used, i;

	if (TerrainDefs[r->type].similar_type == R_OCEAN) {
		u->error("Gates may not be constructed at sea.");
		return 0;
	}

	if (r->gate) {
		u->error("There is already a gate in that region.");
		return 0;
	}

	if (u->GetSharedMoney() < 1000) {
		u->error("Can't afford to construct a Gate.");
		return 0;
	}

	u->ConsumeSharedMoney(1000);

	int level = u->GetSkill(spell);
	int chance = level * 20;
	if (rng::get_random(100) >= chance) {
		u->event("Attempts to construct a gate, but fails.", "spell");
		return 0;
	}

	u->event("Constructs a Gate in " + string(r->ShortPrint().const_str()) + ".", "spell");
	regions.numberofgates++;
	if (Globals->DISPERSE_GATE_NUMBERS) {
		log10 = 0;
		ngates = regions.numberofgates;
		while (ngates > 0) {
			ngates /= 10;
			log10++;
		}
		ngates = 10;
		while (log10 > 0) {
			ngates *= 10;
			log10--;
		}
		used = new int[ngates];
		for (i = 0; i < ngates; i++)
			used[i] = 0;
		for(const auto reg : regions) {
			if (reg->gate) used[reg->gate - 1] = 1;
		}
		r->gate = rng::get_random(ngates);
		while (used[r->gate])
			r->gate = rng::get_random(ngates);
		delete[] used;
		r->gate++;
	} else {
		r->gate = regions.numberofgates;
	}
	if (Globals->GATES_NOT_PERENNIAL) {
		int dm = Globals->GATES_NOT_PERENNIAL / 2;
		int gm = month + 1 - rng::get_random(dm) - rng::get_random(dm) - rng::get_random(Globals->GATES_NOT_PERENNIAL % 2);
		while(gm < 0) gm += 12;
		r->gatemonth = gm;
	}
	return 1;
}

int Game::RunEngraveRunes(ARegion *r,Object *o,Unit *u)
{
	if (o->IsFleet() || !o->IsBuilding()) {
		u->error("Runes of Warding may only be engraved on a building.");
		return 0;
	}

	if (o->incomplete > 0) {
		u->error( "Runes of Warding may only be engraved on a completed "
				"building.");
		return 0;
	}

	int level = u->GetSkill(S_ENGRAVE_RUNES_OF_WARDING);

	switch (level) {
		case 5:
			if (o->type == O_MCASTLE) break;
			if (o->type == O_MCITADEL) break;
		case 4:
			if (o->type == O_CITADEL) break;
			if (o->type == O_MFORTRESS) break;
		case 3:
			if (o->type == O_CASTLE) break;
			if (o->type == O_MTOWER) break;
		case 2:
			if (o->type == O_FORT) break;
		case 1:
			if (o->type == O_TOWER) break;
		default:
			u->error("Not high enough level to engrave Runes of Warding on "
					"that building.");
			return 0;
	}

	if (u->GetSharedMoney() < 600) {
		u->error("Can't afford to engrave Runes of Warding.");
		return 0;
	}

	u->ConsumeSharedMoney(600);
	if (o->type == O_MCITADEL ) {
		o->runes = 5;
	} else if (o->type == O_MCASTLE) {
		o->runes = 5;
	} else if (o->type == O_MFORTRESS) {
		o->runes = 5;
	} else if (o->type == O_MTOWER) {
		o->runes = 5;
	} else {
		o->runes = 3;
	}
	u->event("Engraves Runes of Warding on " + string(o->name->const_str()) + ".", "spell");
	return 1;
}

int Game::RunSummonBalrog(ARegion *r,Unit *u)
{
	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	if (u->items.GetNum(I_BALROG) >= ItemDefs[I_BALROG].max_inventory) {
		u->error("Can't control any more balrogs.");
		return 0;
	}

	int level = u->GetSkill(S_SUMMON_BALROG);

	int num = (level * ItemDefs[I_BALROG].mOut + rng::get_random(100)) / 100;
	if (u->items.GetNum(I_BALROG) + num > ItemDefs[I_BALROG].max_inventory)
		num = ItemDefs[I_BALROG].max_inventory - u->items.GetNum(I_BALROG);

	u->items.SetNum(I_BALROG,u->items.GetNum(I_BALROG) + num);
	u->event("Summons " + ItemString(I_BALROG, num) + ".", "spell");
	return 1;
}

int Game::RunSummonDemon(ARegion *r,Unit *u)
{
	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	int level = u->GetSkill(S_SUMMON_DEMON);
	int num = (level * ItemDefs[I_DEMON].mOut + rng::get_random(100)) / 100;
	num = RandomiseSummonAmount(num);
	if (num < 1)
		num = 1;
	u->items.SetNum(I_DEMON,u->items.GetNum(I_DEMON) + num);
	u->event("Summons " + ItemString(I_DEMON, num) + ".", "spell");
	return 1;
}

int Game::RunSummonImps(ARegion *r,Unit *u)
{
	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	int level = u->GetSkill(S_SUMMON_IMPS);
	int num = (level * ItemDefs[I_IMP].mOut + rng::get_random(100)) / 100;
	num = RandomiseSummonAmount(num);

	u->items.SetNum(I_IMP,u->items.GetNum(I_IMP) + num);
	u->event("Summons " + ItemString(I_IMP, num) + ".", "spell");
	return 1;
}

int Game::RunCreateArtifact(ARegion *r,Unit *u,int skill,int item)
{
	int level = u->GetSkill(skill);
	if (level < ItemDefs[item].mLevel) {
		u->error("CAST: Skill level isn't that high.");
		return 0;
	}
	unsigned int c;
	for (c = 0; c < sizeof(ItemDefs[item].mInput)/sizeof(ItemDefs[item].mInput[0]); c++) {
		if (ItemDefs[item].mInput[c].item == -1) continue;
		int amt = u->GetSharedNum(ItemDefs[item].mInput[c].item);
		int cost = ItemDefs[item].mInput[c].amt;
		if (amt < cost) {
			u->error("Doesn't have sufficient " + ItemDefs[ItemDefs[item].mInput[c].item].name + " to create that.");
			return 0;
		}
	}

	// Deduct the costs
	for (c = 0; c < sizeof(ItemDefs[item].mInput)/sizeof(ItemDefs[item].mInput[0]); c++) {
		if (ItemDefs[item].mInput[c].item == -1) continue;
		int cost = ItemDefs[item].mInput[c].amt;
		u->ConsumeShared(ItemDefs[item].mInput[c].item, cost);
	}

	int num = (level * ItemDefs[item].mOut + rng::get_random(100))/100;

	if (ItemDefs[item].type & IT_SHIP) {
		if (num > 0)
			CreateShip(r, u, item);
	} else {
		u->items.SetNum(item,u->items.GetNum(item) + num);
	}
	u->event("Creates " + ItemString(item, num) + ".", "spell");
	if (num == 0) return 0;
	return 1;
}

int Game::RunSummonLich(ARegion *r,Unit *u)
{
	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	int level = u->GetSkill(S_SUMMON_LICH);

	int chance = level * ItemDefs[I_LICH].mOut;
	if (chance < 1)
		chance = level * level * 2;
	int num = (chance + rng::get_random(100))/100;

	u->items.SetNum(I_LICH,u->items.GetNum(I_LICH) + num);
	u->event("Summons " + ItemString(I_LICH, num) + ".", "spell");
	if (num == 0) return 0;
	return 1;
}

int Game::RunRaiseUndead(ARegion *r,Unit *u)
{
	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	int level = u->GetSkill(S_RAISE_UNDEAD);

	int chance = level * ItemDefs[I_UNDEAD].mOut;
	if (chance < 1)
		chance = level * level * 10;
	int num = (chance + rng::get_random(100))/100;
	num = RandomiseSummonAmount(num);

	u->items.SetNum(I_UNDEAD,u->items.GetNum(I_UNDEAD) + num);
	u->event("Raises " + ItemString(I_UNDEAD, num) + ".", "spell");
	if (num == 0) return 0;
	return 1;
}

int Game::RunSummonSkeletons(ARegion *r,Unit *u)
{
	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	int level = u->GetSkill(S_SUMMON_SKELETONS);

	int chance = level * ItemDefs[I_SKELETON].mOut;
	if (chance < 1)
		chance = level * level * 40;
	int num = (chance + rng::get_random(100))/100;
	num = RandomiseSummonAmount(num);

	u->items.SetNum(I_SKELETON,u->items.GetNum(I_SKELETON) + num);
	u->event("Summons " + ItemString(I_SKELETON, num) + ".", "spell");
	if (num == 0) return 0;
	return 1;
}

int Game::RunDragonLore(ARegion *r, Unit *u)
{
	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	int level = u->GetSkill(S_DRAGON_LORE);

	int num = u->items.GetNum(I_DRAGON);
	if (num >= level) {
		u->error("Mage may not summon more dragons.");
		return 0;
	}

	int chance = level * ItemDefs[I_DRAGON].mOut;
	if (chance < 1)
		chance = level * level * 4;
	if (rng::get_random(100) < chance) {
		u->items.SetNum(I_DRAGON,num + 1);
		u->event("Summons a dragon.", "spell");
		num = 1;
	} else {
		u->event("Attempts to summon a dragon, but fails.", "spell");
		num = 0;
	}
	if (num == 0) return 0;
	return 1;
}

int Game::RunBirdLore(ARegion *r,Unit *u)
{
	CastIntOrder *order = dynamic_cast<CastIntOrder *>(u->castorders);
	int type = regions.GetRegionArray(r->zloc)->levelType;

	if (type != ARegionArray::LEVEL_SURFACE) {
		AString error = "CAST: Bird Lore may only be cast on the surface of ";
		error += Globals->WORLD_NAME;
		error += ".";
		u->error(error.Str());
		return 0;
	}

	if (order->level < 3) {
		int dir = order->target;
		ARegion *tar = r->neighbors[dir];
		if (!tar) {
			u->error("CAST: No such region.");
			return 0;
		}

		Farsight *f = new Farsight;
		f->faction = u->faction;
		f->level = u->GetSkill(S_BIRD_LORE);
		tar->farsees.push_back(f);
		u->event("Sends birds to spy on " + string(tar->Print().const_str()) + ".", "spell");
		return 1;
	}

	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	int level = u->GetSkill(S_BIRD_LORE) - 2;
	int max = level * level * 2;
	int num = (level * ItemDefs[I_EAGLE].mOut + rng::get_random(100)) / 100;
	num = RandomiseSummonAmount(num);
	if (num < 1)
		num = 1;

	if (u->items.GetNum(I_EAGLE) >= max) {
		u->error("CAST: Mage can't summon more eagles.");
		return 0;
	}

	if (u->items.GetNum(I_EAGLE) + num > max)
		num = max - u->items.GetNum(I_EAGLE);

	u->items.SetNum(I_EAGLE,u->items.GetNum(I_EAGLE) + num);
	u->event("Summons " + ItemString(I_EAGLE, num) + ".", "spell");
	return 1;
}

int Game::RunWolfLore(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_WOLF_LORE);
	int max = level * level * 4;

	int curr = u->items.GetNum(I_WOLF);
	int num = (level * ItemDefs[I_WOLF].mOut + rng::get_random(100)) / 100;
	num = RandomiseSummonAmount(num);

	if (num + curr > max)
		num = max - curr;
	if (num < 0) num = 0;

	u->event("Casts Wolf Lore, summoning " + ItemString(I_WOLF, num) + ".", "spell");
	u->items.SetNum(I_WOLF, num + curr);
	if (num == 0) return 0;
	return 1;
}

int Game::RunInvisibility(ARegion *r,Unit *u)
{
	CastUnitsOrder *order = dynamic_cast<CastUnitsOrder *>(u->castorders);
	int max = u->GetSkill(S_INVISIBILITY);
	max = max * max;

	int num = 0;
	r->deduplicate_unit_list(order->units, u->faction->num);
	for(const auto id : order->units) {
		Unit *tar = r->GetUnitId(id, u->faction->num);
		if (!tar) continue;
		if (tar->GetAttitude(r,u) < A_FRIENDLY) continue;
		num += tar->GetSoldiers();
	}

	if (num > max) {
		u->error("CAST: Can't render that many men or creatures invisible.");
		return 0;
	}

	if (!num) {
		u->error("CAST: No valid targets to turn invisible.");
		return 0;
	}
	for(const auto id : order->units) {
		Unit *tar = r->GetUnitId(id, u->faction->num);
		if (!tar) continue;
		if (tar->GetAttitude(r,u) < A_FRIENDLY) continue;
		tar->SetFlag(FLAG_INVIS,1);
		tar->event("Is rendered invisible by " + string(u->name->const_str()) + ".", "spell");
	}

	// std::for_each(order->units.begin(), order->units.end(), [&](UnitId *id) { delete id; });
	// order->units.clear();
	u->event("Casts invisibility.", "spell");
	return 1;
}

int Game::RunPhanDemons(ARegion *r,Unit *u)
{
	CastIntOrder *order = dynamic_cast<CastIntOrder *>(u->castorders);
	int level = u->GetSkill(S_CREATE_PHANTASMAL_DEMONS);
	int create,max;

	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	if (order->level < 2) {
		create = I_IIMP;
		max = level * level * 4;
	} else {
		if (order->level < 3) {
			create = I_IDEMON;
			max = level * level;
		} else {
			create = I_IBALROG;
			max = 1;
		}
	}

	if (order->target > max || order->target <= 0) {
		u->error("CAST: Can't create that many Phantasmal Demons.");
		return 0;
	}

	u->items.SetNum(create,order->target);
	u->event("Casts Create Phantasmal Demons.", "spell");
	return 1;
}

int Game::RunPhanUndead(ARegion *r,Unit *u)
{
	CastIntOrder *order = dynamic_cast<CastIntOrder *>(u->castorders);
	int level = u->GetSkill(S_CREATE_PHANTASMAL_UNDEAD);
	int create,max;

	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	if (order->level < 2) {
		create = I_ISKELETON;
		max = level * level * 4;
	} else {
		if (order->level < 3) {
			create = I_IUNDEAD;
			max = level * level;
		} else {
			create = I_ILICH;
			max = level;
		}
	}

	if (order->target > max || order->target <= 0) {
		u->error("CAST: Can't create that many Phantasmal Undead.");
		return 0;
	}

	u->items.SetNum(create,order->target);
	u->event("Casts Create Phantasmal Undead.", "spell");
	return 1;
}

int Game::RunPhanBeasts(ARegion *r,Unit *u)
{
	CastIntOrder *order = dynamic_cast<CastIntOrder *>(u->castorders);
	int level = u->GetSkill(S_CREATE_PHANTASMAL_BEASTS);
	int create,max;

	if (r->type == R_NEXUS) {
		u->error("Can't summon creatures in the nexus.");
		return 0;
	}

	if (order->level < 2) {
		create = I_IWOLF;
		max = level * level * 4;
	} else {
		if (order->level < 3) {
			create = I_IEAGLE;
			max = level * level;
		} else {
			create = I_IDRAGON;
			max = level;
		}
	}

	if (order->target > max || order->target <= 0) {
		u->error("CAST: Can't create that many Phantasmal Beasts.");
		return 0;
	}

	u->items.SetNum(create,order->target);
	u->event("Casts Create Phantasmal Beasts.", "spell");
	return 1;
}

int Game::RunEarthLore(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_EARTH_LORE);

	if (level > r->earthlore) r->earthlore = level;
	int amt = r->Wages() * level * 2 / 10;

	u->items.SetNum(I_SILVER,u->items.GetNum(I_SILVER) + amt);
	u->event("Casts Earth Lore, raising " + to_string(amt) + " silver.", "spell");
	return 1;
}

int Game::RunPhantasmalEntertainment(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_PHANTASMAL_ENTERTAINMENT);

	int amt = level * Globals->ENTERTAIN_INCOME * 20;
	int max_entertainement = 0;

	if (level > r->phantasmal_entertainment) r->phantasmal_entertainment = level;

	Production *p = r->get_production_for_skill(I_SILVER, S_ENTERTAINMENT);
	if (p != NULL) {
		max_entertainement = p->amount;
	}

	if (amt > max_entertainement) {
		amt = max_entertainement;
	}

	u->items.SetNum(I_SILVER, u->items.GetNum(I_SILVER) + amt);
	u->event("Casts Phantasmal Entertainment, raising " + to_string(amt) + " silver.", "spell");
	return 1;
}

int Game::RunClearSkies(ARegion *r, Unit *u)
{
	ARegion *tar = r;
	string temp = "Casts Clear Skies";
	int val;

	CastRegionOrder *order = dynamic_cast<CastRegionOrder *>(u->castorders);

	RangeType *range = FindRange(SkillDefs[S_CLEAR_SKIES].range);
	if (range != NULL) {
		tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
		val = GetRegionInRange(r, tar, u, S_CLEAR_SKIES);
		if (!val) return 0;
		temp += " on " + string(tar->ShortPrint().const_str());
	}
	temp += ".";
	int level = u->GetSkill(S_CLEAR_SKIES);
	if (level > r->clearskies) r->clearskies = level;
	u->event(temp, "spell");
	return 1;
}

int Game::RunWeatherLore(ARegion *r, Unit *u)
{
	ARegion *tar;
	int val, i;

	CastRegionOrder *order = dynamic_cast<CastRegionOrder *>(u->castorders);

	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_WEATHER_LORE);
	if (!val) return 0;

	int level = u->GetSkill(S_WEATHER_LORE);
	int months = 3;
	if (level >= 5) months = 12;
	else if (level >= 3) months = 6;

	string temp = "Casts Weather Lore on " + string(tar->ShortPrint().const_str()) + ". It will be ";
	int weather, futuremonth;
	for (i = 0; i <= months; i++) {
		futuremonth = (month + i)%12;
		weather=regions.GetWeather(tar, futuremonth);
		temp += SeasonNames[weather] + " " + MonthNames[futuremonth];
		if (i < (months-1))
			temp += ", ";
		else if (i == (months-1))
			temp += " and ";
		else
			temp += ".";
	}
	u->event(temp, "spell");
	return 1;
}

int Game::RunFarsight(ARegion *r,Unit *u)
{
	ARegion *tar;
	int val;

	CastRegionOrder *order = dynamic_cast<CastRegionOrder *>(u->castorders);

	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_FARSIGHT);
	if (!val) return 0;

	Farsight *f = new Farsight;
	f->faction = u->faction;
	f->level = u->GetSkill(S_FARSIGHT);
	f->unit = u;
	f->observation = u->GetAttribute("observation");
	tar->farsees.push_back(f);
	u->event("Casts Farsight on " + string(tar->ShortPrint().const_str()) + ".", "spell");
	return 1;
}

int Game::RunDetectGates(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_GATE_LORE);

	if (level == 1) {
		u->error("CAST: Casting Gate Lore at level 1 has no effect.");
		return 0;
	}

	u->event("Casts Gate Lore, detecting nearby Gates:", "spell");
	int found = 0;
	if ((r->gate) && (!r->gateopen)) {
		u->event("Identified local gate number " + to_string(r->gate) + " in " +
			string(r->ShortPrint().const_str()) + ".", "spell");
	}
	for (int i=0; i<NDIRS; i++) {
		ARegion *tar = r->neighbors[i];
		if (tar) {
			if (tar->gate) {
				if (Globals->DETECT_GATE_NUMBERS) {
					u->event(string(tar->Print().const_str()) +	" contains Gate " +
						to_string(tar->gate) + ".", "spell");
				} else {
					u->event(string(tar->Print().const_str()) +	" contains a Gate.", "spell");
				}
				found = 1;
			}
		}
	}
	if (!found) u->event("There are no nearby Gates.", "spell");
	return 1;
}

int Game::RunTeleport(ARegion *r,Object *o,Unit *u)
{
	ARegion *tar;
	int val;

	CastRegionOrder *order = (CastRegionOrder *)u->teleportorders;

	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_TELEPORTATION);
	if (!val) return 0;

	int level = u->GetSkill(S_TELEPORTATION);
	int maxweight = level * 50;

	if (u->Weight() > maxweight) {
		u->error("CAST: Can't carry that much when teleporting.");
		return 0;
	}

	// Check for any keybarrier objects in the target region
	for(const auto o : tar->objects) {
		if (ObjectDefs[o->type].flags & ObjectType::KEYBARRIER) {
			if (u->items.GetNum(ObjectDefs[o->type].key_item) < 1) {
				u->error("CAST: A mystical barrier prevents teleporting to that location.");
				return 0;
			}
		}
	}

	const char *prevented = tar->movement_forbidden_by_ruleset(u, r, regions);
	if (prevented != nullptr) {
		u->error("CAST: " + string(prevented) + " prevents teleporting to that location.");
		return 0;
	}

	// Presume they had to open the portal to see if target is ocean
	if (TerrainDefs[tar->type].similar_type == R_OCEAN) {
		// If the unit has enough capacity to swim in the ocean, let them teleport there.
		// We use CanReallySwim rather than CanSwim because the latter also allows flying
		// units which would break the 'flying units must end on land' rule.
		if (!u->CanReallySwim()) {
			u->error(string("CAST: ") + tar->Print().const_str() + " is ocean.");
			return 1;
		}
	}
	u->DiscardUnfinishedShips();
	u->event("Teleports to " + string(tar->Print().const_str()) + ".", "spell");
	u->MoveUnit(tar->GetDummy());
	return 1;
}

int Game::RunGateJump(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_GATE_LORE);
	int nexgate = 0;
	if ( !level ) {
		u->error( "CAST: Unit doesn't have that skill." );
		return 0;
	}

	TeleportOrder *order = u->teleportorders;

	if ((order->gate > 0 && level < 2) || (order->gate == -2 && level < 2)) {
		u->error("CAST: Unit Doesn't know Gate Lore at that level.");
		return 0;
	}

	nexgate = Globals->NEXUS_GATE_OUT && (TerrainDefs[r->type].similar_type == R_NEXUS);
	if (!r->gate && !nexgate) {
		u->error("CAST: There is no gate in that region.");
		return 0;
	}

	if (!r->gateopen) {
		u->error("CAST: Gate not open at this time of year.");
		return 0;
	}

	int maxweight = 10;

	// -2 means random jump between levels
	// We need to reduce capacity by 1 level
	if (order->gate == -2) {
		level = level - 1;
	}

	// -1 means no gate selected - random jump
	// -2 means random jump between levels
	if (order->gate == -1 || order->gate == -2) {
		switch (level) {
			case 1:
				maxweight = 15;
				break;
			case 2:
				maxweight = 500;
				break;
			case 3:
				maxweight = 1500;
				break;
			case 4:
				maxweight = 3000;
				break;
			case 5:
				maxweight = 6000;
				break;
		}
	} else {
		// Gate selected
		switch (level) {
			case 1:
				maxweight = 0;
				break;
			case 2:
				maxweight = 15;
				break;
			case 3:
				maxweight = 500;
				break;
			case 4:
				maxweight = 1500;
				break;
			case 5:
				maxweight = 3000;
				break;
		}
	}

	int weight = u->Weight();

	r->deduplicate_unit_list(order->units, u->faction->num);
	for(const auto id : order->units) {
		Unit *taru = r->GetUnitId(id, u->faction->num);
		if (taru && taru != u) weight += taru->Weight();
	}

	if (weight > maxweight) {
		u->error( "CAST: Can't carry that much weight through a Gate.");
		return 0;
	}

	ARegion *tar;
	if (order->gate < 0) {
		int good = 0;

		do {
			tar = regions.FindGate(-1);
			if (!tar) continue;

			if (tar->zloc == r->zloc) good = 1;
			if (order->gate == -2) good = 1;
			if (nexgate && tar->zloc == ARegionArray::LEVEL_SURFACE) good = 1;
			if (!tar->gateopen) good = 0;
		} while (!good);

		string jump_text = "Casts Random Gate Jump. Capacity: " + to_string(weight) + "/" + to_string(maxweight) + ".";
		u->event(jump_text, "spell");
	} else {
		tar = regions.FindGate(order->gate);
		if (!tar) {
			u->error("CAST: No such target gate.");
			return 0;
		}
		if (!tar->gateopen) {
			u->error("CAST: Target gate is not open at this time of year.");
			return 0;
		}

		string jump_text = "Casts Gate Jump. Capacity: " + to_string(weight) + "/" + to_string(maxweight) + ".";
		u->event(jump_text, "spell");
	}

	int comma = 0;
	string unitlist;
	for(const auto id : order->units) {
		Location *loc = r->GetLocation(id, u->faction->num);
		if (loc) {
			/* Don't do the casting unit yet */
			if (loc->unit == u) {
				delete loc;
				continue;
			}

			if (loc->unit->GetAttitude(r,u) < A_ALLY) {
				u->error("CAST: Unit is not allied.");
			} else {
				unitlist += (comma ? ", " : "") + to_string(loc->unit->num);
				comma = 1;
				loc->unit->DiscardUnfinishedShips();
				loc->unit->event("Is teleported through a Gate to " + string(tar->Print().const_str()) +
					" by " + string(u->name->const_str()) + ".", "spell");
				loc->unit->MoveUnit(tar->GetDummy());
				if (loc->unit != u) loc->unit->ClearCastOrders();
			}
			delete loc;
		} else {
			u->error("CAST: No such unit.");
		}
	}
	u->DiscardUnfinishedShips();
	u->event("Jumps through a Gate to " + string(tar->Print().const_str()) + ".", "spell");
	if (comma) {
		u->event(unitlist + " follow through the Gate.", "spell");
	}
	u->MoveUnit(tar->GetDummy());
	return 1;
}

int Game::RunPortalLore(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_PORTAL_LORE);
	TeleportOrder *order = u->teleportorders;

	if (!level) {
		u->error("CAST: Doesn't know Portal Lore.");
		return 0;
	}

	if (!u->items.GetNum(I_PORTAL)) {
		u->error("CAST: Unit doesn't have a Portal.");
		return 0;
	}

	int maxweight = 800 * level;
	r->deduplicate_unit_list(order->units, u->faction->num);
	int weight = 0;
	for(const auto id : order->units) {
		Unit *taru = r->GetUnitId(id, u->faction->num);
		if (taru) weight += taru->Weight();
	}

	if (weight > maxweight) {
		u->error("CAST: That mage cannot teleport that much weight through a "
				"Portal.");
		return 0;
	}

	Location *tar = regions.FindUnit(order->gate);
	if (!tar) {
		u->error("CAST: No such target mage.");
		return 0;
	}

	if (tar->unit->faction->get_attitude(u->faction->num) < A_FRIENDLY) {
		u->error("CAST: Target mage is not friendly.");
		return 0;
	}

	if (tar->unit->type != U_MAGE && tar->unit->type != U_APPRENTICE) {
		u->error("CAST: Target is not a mage.");
		return 0;
	}

	if (!tar->unit->items.GetNum(I_PORTAL)) {
		u->error("CAST: Target does not have a Portal.");
		return 0;
	}

	if (!GetRegionInRange(r, tar->region, u, S_PORTAL_LORE)) return 0;

	// Check for any keybarrier objects in the target region
	for(const auto o : tar->region->objects) {
		if (ObjectDefs[o->type].flags & ObjectType::KEYBARRIER) {
			if (u->items.GetNum(ObjectDefs[o->type].key_item) < 1) {
				u->error("CAST: A mystical barrier prevents portalling to that location.");
				return 0;
			}
		}
	}

	const char *prevented = tar->region->movement_forbidden_by_ruleset(u, r, regions);
	if (prevented != nullptr) {
		u->error("CAST: " + string(prevented) + " prevents portalling to that location.");
		return 0;
	}

	u->event("Casts Portal Jump.", "spell");

	for(const auto id : order->units) {
		Location *loc = r->GetLocation(id, u->faction->num);
		if (loc) {
			if (loc->unit->GetAttitude(r,u) < A_ALLY) {
				u->error("CAST: Unit is not allied.");
			} else {
				loc->unit->DiscardUnfinishedShips();
				loc->unit->event("Is teleported to " + string(tar->region->Print().const_str()) +
					" by " + string(u->name->const_str()) + ".", "spell");
				loc->unit->MoveUnit( tar->obj );
				if (loc->unit != u) loc->unit->ClearCastOrders();
			}
			delete loc;
		} else {
			u->error("CAST: No such unit.");
		}
	}

	delete tar;
	return 1;
}

int Game::RunTransmutation(ARegion *r, Unit *u)
{
	CastTransmuteOrder *order;
	int level, num, source;

	order = dynamic_cast<CastTransmuteOrder *>(u->castorders);
	level = u->GetSkill(S_TRANSMUTATION);
	if (!level) {
		u->error("CAST: Unit doesn't have that skill.");
		return 0;
	}
	if (level < order->level) {
		u->error("CAST: Can't create that by transmutation.");
		return 0;
	}

	switch(order->item) {
		case I_ADMANTIUM:
		case I_MITHRIL:
			source = I_IRON;
			break;
		case I_ROOTSTONE:
			source = I_STONE;
			break;
		case I_FLOATER:
			source = I_FUR;
			break;
		case I_IRONWOOD:
		case I_YEW:
			source = I_WOOD;
			break;
		case I_WHORSE:
			source = I_HORSE;
			break;
	}

	num = u->GetSharedNum(source);
	if (num > ItemDefs[order->item].mOut * level)
		num = ItemDefs[order->item].mOut * level;
	if (order->number != -1 && num > order->number)
		num = order->number;
	if (num < order->number)
		u->error("CAST: Can't create that many.");
	u->ConsumeShared(source, num);
	u->items.SetNum(order->item, u->items.GetNum(order->item) + num);
	u->event("Transmutes " + ItemString(source, num) + " into " + ItemString(order->item, num) + ".", "spell");

	return 1;
}

int Game::RunBlasphemousRitual(ARegion *r, Unit *mage)
{
	int level, num, sactype, sacrifices, i, sac, relics;
	Object *tower;
	Unit *victim;
	AString message;

	level = mage->GetSkill(S_BLASPHEMOUS_RITUAL);
	if (level < 1) {
		mage->error("CAST: Unit doesn't have that skill.");
		return 0;
	}
	if (TerrainDefs[r->type].similar_type == R_OCEAN) {
		mage->error("CAST: Can't cast Ritual on water.");
		return 0;
	}
	num = level;
	tower = 0;
	sactype = IT_LEADER;
	sacrifices = 0;

	for(const auto o : r->objects) {
		if (o->type == O_BKEEP && !o->incomplete) {
			tower = o;
		}

		for(const auto u : o->units) {
			if (u->faction->num == mage->faction->num) {
				for(auto item : u->items) {
					if (ItemDefs[item->type].type & sactype) {
						sacrifices += item->num;
					}
				}
			}
		}
	}

	if (tower == 0) {
		mage->error("CAST: Can't cast Ritual: no Black Tower in a region.");
		return 0;
	}

	if (num > sacrifices) {
		num = sacrifices;
	}

	while (num-- > 0) {
		victim = 0;
		i = rng::get_random(sacrifices);
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				if (u->faction->num == mage->faction->num) {
					for(auto item : u->items) {
						if (ItemDefs[item->type].type & sactype) {
							if (!victim && i < item->num) {
								victim = u;
								sac = item->type;
							}
							i -= item->num;
						}
					}
				}
			}
		}

		victim->SetMen(sac, victim->GetMen(sac) - 1);
		sacrifices--;

		// Write article with a details
		message = "Vile ritual has been performed at ";
		message += r->ShortPrint();
		message += "!";
		WriteTimesArticle(message);

		mage->event("Sacrifices " + ItemDefs[sac].name + " from " + victim->name->const_str(), "spell");
		if (!victim->GetMen())
			r->Kill(victim);
		if (!mage->GetMen())
			break;

		relics = mage->items.GetNum(I_RELICOFGRACE);
		mage->items.SetNum(I_RELICOFGRACE, relics + 1);
	}

	return 1;
}

void Game::RunTeleportOrders()
{
	int val = 1;
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			int foundone = 1;
			while (foundone) {
				foundone = 0;
				for(const auto u : o->units) {
					if (u->teleportorders) {
						foundone = 1;
						switch (u->teleportorders->spell) {
							case S_GATE_LORE:
								val = RunGateJump(r,o,u);
								break;
							case S_TELEPORTATION:
								val = RunTeleport(r,o,u);
								break;
							case S_PORTAL_LORE:
								val = RunPortalLore(r,o,u);
								break;
						}
						if (val) u->Practice(u->teleportorders->spell);
						std::for_each(
							u->teleportorders->units.begin(),
							u->teleportorders->units.end(),
							[&](UnitId *id) { delete id; }
						);
						u->teleportorders->units.clear();
						delete u->teleportorders;
						u->teleportorders = 0;
						break;
					}
				}
			}
		}
	}
}
