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

void Game::ProcessCastOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "CAST: No skill given.");
        return;
    }

    int sk = ParseSkill(token);
    delete token;
    if (sk==-1) {
        ParseError( pCheck, u, 0, "CAST: Invalid skill.");
        return;
    }

    if( !( SkillDefs[sk].flags & SkillType::MAGIC )) {
        ParseError( pCheck, u, 0, "CAST: That is not a magic skill.");
        return;
    }
    if( !( SkillDefs[sk].flags & SkillType::CAST )) {
        ParseError( pCheck, u, 0, "CAST: That skill cannot be CAST.");
        return;
    }

    if( !pCheck ) {
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
			case S_CLEAR_SKIES:
			case S_EARTH_LORE:
			case S_CREATE_RING_OF_INVISIBILITY:
			case S_CREATE_CLOAK_OF_INVULNERABILITY:
			case S_CREATE_STAFF_OF_FIRE:
			case S_CREATE_STAFF_OF_LIGHTNING:
			case S_CREATE_AMULET_OF_TRUE_SEEING:
			case S_CREATE_AMULET_OF_PROTECTION:
			case S_CREATE_RUNESWORD:
			case S_CREATE_SHIELDSTONE:
			case S_CREATE_MAGIC_CARPET:
				ProcessGenericSpell(u,sk, pCheck );
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
			case S_FARSIGHT:
				ProcessFarsight(u,o, pCheck );
				break;
			case S_TELEPORTATION:
				ProcessTeleportation(u,o, pCheck );
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
		}
	}
}

void Game::ProcessMindReading(Unit *u,AString *o, OrdersCheck *pCheck )
{
    UnitId *id = ParseUnit(o);

    if (!id) {
        u->Error("CAST: No unit specified.");
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
        u->Error("CAST: Missing arguments.");
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
            u->Error("CAST: Missing arguments.");
            return;
        }

        int dir = ParseDir(token);
        delete token;
        if (dir == -1 || dir > NDIRS) {
            u->Error("CAST: Invalid direction.");
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

    u->Error("CAST: Invalid arguments.");
    delete token;
}

void Game::ProcessInvisibility(Unit *u,AString *o, OrdersCheck *pCheck )
{
    AString *token = o->gettoken();

    if (!token || !(*token == "units")) {
        u->Error("CAST: Must specify units to render invisible.");
        return;
    }
    delete token;

    CastUnitsOrder *order;
    if (u->castorders && u->castorders->type == O_CAST &&
        ((CastOrder *) u->castorders)->spell == S_INVISIBILITY &&
        ((CastOrder *) u->castorders)->level == 1) {
        order = (CastUnitsOrder *) u->castorders;
    } else {
        order = new CastUnitsOrder;
        order->spell = S_INVISIBILITY;
        order->level = 1;
        u->ClearCastOrders();
        u->castorders = order;
    }

    UnitId *id = ParseUnit(o);
    while (id) {
        order->units.Add(id);
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
        u->Error("CAST: Illusion to summon must be given.");
        delete order;
        return;
    }

    if (*token == "imp" || *token == "imps") {
        order->level = 1;
    }

    if (*token == "demon" || *token == "demons") {
        order->level = 3;
    }

    if (*token == "balrog" || *token == "balrogs") {
        order->level = 5;
    }

    delete token;

    if (!order->level) {
        u->Error("CAST: Can't summon that illusion.");
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
		u->Error("CAST: Must specify which illusion to summon.");
		delete order;
		return;
	}

	if (*token == "skeleton" || *token == "skeletons") {
		order->level = 1;
	}

	if (*token == "undead") {
		order->level = 3;
	}

	if (*token == "lich" || *token == "liches") {
		order->level = 5;
	}

	delete token;

	if (!order->level) {
		u->Error("CAST: Must specify which illusion to summon.");
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
		u->Error("CAST: Must specify which illusion to summon.");
		delete order;
		return;
	}

	if (*token == "wolf" || *token == "wolves") {
		order->level = 1;
	}
	if (*token == "eagle" || *token == "eagles") {
		order->level = 3;
	}
	if (*token == "dragon" || *token == "dragon") {
		order->level = 5;
	}

	delete token;
	if (!order->level) {
		delete order;
		u->Error("CAST: Must specify which illusion to summon.");
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
	u->ClearCastOrders();
	CastOrder *orders = new CastOrder;
	orders->spell = spell;
	orders->level = 1;
	u->castorders = orders;
}

void Game::ProcessTeleportation(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();
	if (!token) {
		u->Error("CAST: Must specify a target region.");
		return;
	}
	if (!(*token == "region")) {
		delete token;
		u->Error("CAST: Must specify a target region.");
		return;
	}
	delete token;

	token = o->gettoken();
	if (!token) {
		u->Error("CAST: Must specify a target region.");
		return;
	}
	int x=token->value();
	delete token;

	token = o->gettoken();
	if (!token) {
		u->Error("CAST: Must specify a target region.");
		return;
	}
	int y=token->value();
	delete token;

	int z = u->object->region->zloc;
	if(Globals->EASIER_UNDERWORLD) {
		token = o->gettoken();
		if(token) {
			z = token->value();
			delete token;
		}
		if(z < 0 || (z > Globals->UNDERWORLD_LEVELS +
				   	Globals->UNDERDEEP_LEVELS +
				   	Globals->ABYSS_LEVEL + 1)) {
			u->Error("CAST: Invalid Z coordinate specified");
			return;
		}
	}

	TeleportOrder *order = new TeleportOrder;
	u->ClearCastOrders();
	order->xloc = x;
	order->yloc = y;
	order->zloc = z;
	order->spell = S_TELEPORTATION;
	order->level = 1;
	u->teleportorders = order;
}

void Game::ProcessFarsight(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();
	if (!token) {
		u->Error("CAST: Region to view must be specified.");
		return;
	}
	if (!(*token == "region")) {
		delete token;
		u->Error("CAST: Region to view must be specified.");
		return;
	}
	delete token;

	/* Should be through REGION crap */
	token = o->gettoken();
	if (!token) {
		u->Error("CAST: Region to view must be specified.");
		return;
	}
	int x = token->value();
	delete token;

	token = o->gettoken();
	if (!token) {
		u->Error("CAST: Region to view must be specified.");
		return;
	}
	int y = token->value();
	delete token;

	int z = u->object->region->zloc;
	if(Globals->EASIER_UNDERWORLD) {
		token = o->gettoken();
		if(token) {
			z = token->value();
			delete token;
		}
		if(z < 0 || (z > Globals->UNDERWORLD_LEVELS +
					Globals->UNDERDEEP_LEVELS +
					Globals->ABYSS_LEVEL + 1)) {
			u->Error("CAST: Invalid Z coordinate specified");
			return;
		}
	}

	CastRegionOrder *order = new CastRegionOrder;
	order->spell = S_FARSIGHT;
	order->level = 1;
	order->xloc = x;
	order->yloc = y;
	order->zloc = z;

	u->ClearCastOrders();
	u->castorders = order;
}

void Game::ProcessCastPortalLore(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();
	if (!token) {
		u->Error("CAST: Requires a target mage.");
		return;
	}
	int gate = token->value();
	delete token;
	token = o->gettoken();

	if (!token) {
		u->Error("CAST: No units to teleport.");
		return;
	}

	if (!(*token == "units")) {
		u->Error("CAST: No units to teleport.");
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
		order->units.Add(id);
		id = ParseUnit(o);
	}
}

void Game::ProcessCastGateLore(Unit *u,AString *o, OrdersCheck *pCheck )
{
	AString *token = o->gettoken();

	if (!token) {
		u->Error("CAST: Missing argument.");
		return;
	}

	if ((*token) == "gate") {
		delete token;
		token = o->gettoken();

		if (!token) {
			u->Error("CAST: Requires a target gate.");
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
			order->units.Add(id);
			id = ParseUnit(o);
		}
		return;
	}

	if ((*token) == "random") {
		TeleportOrder *order;

		if (u->teleportorders && u->teleportorders->spell == S_GATE_LORE &&
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
		if (!(*token == "units")) {
			delete token;
			return;
		}

		UnitId *id = ParseUnit(o);
		while(id) {
			order->units.Add(id);
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
	u->Error("CAST: Invalid argument.");
}

void Game::RunACastOrder(ARegion * r,Object *o,Unit * u)
{
	if (u->type != U_MAGE) {
		u->Error("CAST: Unit is not a mage.");
		return;
	}

	if (u->castorders->level == 0) {
		u->castorders->level = u->GetSkill(u->castorders->spell);
	}

	if (u->GetSkill(u->castorders->spell) < u->castorders->level ||
			u->castorders->level == 0) {
		u->Error("CAST: Skill level isn't that high.");
		return;
	}

	int sk = u->castorders->spell;
	switch (sk) {
		case S_MIND_READING:
			RunMindReading(r,u);
			break;
		case S_ENCHANT_ARMOR:
			RunEnchantArmor(r,u);
			break;
		case S_ENCHANT_SWORDS:
			RunEnchantSwords(r,u);
			break;
		case S_CONSTRUCT_GATE:
			RunConstructGate(r,u);
			break;
		case S_ENGRAVE_RUNES_OF_WARDING:
			RunEngraveRunes(r,o,u);
			break;
		case S_CONSTRUCT_PORTAL:
			RunCreateArtifact(r,u,sk,I_PORTAL);
			break;
		case S_CREATE_RING_OF_INVISIBILITY:
			RunCreateArtifact(r,u,sk,I_RINGOFI);
			break;
		case S_CREATE_CLOAK_OF_INVULNERABILITY:
			RunCreateArtifact(r,u,sk,I_CLOAKOFI);
			break;
		case S_CREATE_STAFF_OF_FIRE:
			RunCreateArtifact(r,u,sk,I_STAFFOFF);
			break;
		case S_CREATE_STAFF_OF_LIGHTNING:
			RunCreateArtifact(r,u,sk,I_STAFFOFL);
			break;
		case S_CREATE_AMULET_OF_TRUE_SEEING:
			RunCreateArtifact(r,u,sk,I_AMULETOFTS);
			break;
		case S_CREATE_AMULET_OF_PROTECTION:
			RunCreateArtifact(r,u,sk,I_AMULETOFP);
			break;
		case S_CREATE_RUNESWORD:
			RunCreateArtifact(r,u,sk,I_RUNESWORD);
			break;
		case S_CREATE_SHIELDSTONE:
			RunCreateArtifact(r,u,sk,I_SHIELDSTONE);
			break;
		case S_CREATE_MAGIC_CARPET:
			RunCreateArtifact(r,u,sk,I_MCARPET);
			break;
		case S_CREATE_FLAMING_SWORD:
			RunCreateArtifact(r,u,sk,I_FSWORD);
			break;
		case S_SUMMON_IMPS:
			RunSummonImps(r,u);
			break;
		case S_SUMMON_DEMON:
			RunSummonDemon(r,u);
			break;
		case S_SUMMON_BALROG:
			RunSummonBalrog(r,u);
			break;
		case S_SUMMON_LICH:
			RunSummonLich(r,u);
			break;
		case S_RAISE_UNDEAD:
			RunRaiseUndead(r,u);
			break;
		case S_SUMMON_SKELETONS:
			RunSummonSkeletons(r,u);
			break;
		case S_DRAGON_LORE:
			RunDragonLore(r,u);
			break;
		case S_BIRD_LORE:
			RunBirdLore(r,u);
			break;
		case S_WOLF_LORE:
			RunWolfLore(r,u);
			break;
		case S_INVISIBILITY:
			RunInvisibility(r,u);
			break;
		case S_CREATE_PHANTASMAL_DEMONS:
			RunPhanDemons(r,u);
			break;
		case S_CREATE_PHANTASMAL_UNDEAD:
			RunPhanUndead(r,u);
			break;
		case S_CREATE_PHANTASMAL_BEASTS:
			RunPhanBeasts(r,u);
			break;
		case S_GATE_LORE:
			RunDetectGates(r,o,u);
			break;
		case S_FARSIGHT:
			RunFarsight(r,u);
			break;
		case S_EARTH_LORE:
			RunEarthLore(r,u);
			break;
		case S_CLEAR_SKIES:
			RunClearSkies(r,u);
			break;
	}
}

void Game::RunMindReading(ARegion *r,Unit *u)
{
	CastMindOrder *order = (CastMindOrder *) u->castorders;
	int level = u->GetSkill(S_MIND_READING);

	Unit *tar = r->GetUnitId(order->id,u->faction->num);
	if (!tar) {
		u->Error("No such unit.");
		return;
	}

	AString temp = AString("Casts Mind Reading: ") + *(tar->name) + ", " +
		*(tar->faction->name);

	if (level < 4) {
		u->Event(temp + ".");
		return;
	}

	temp += tar->items.Report(2,5,0) + ". Skills: ";
	temp += tar->skills.Report(tar->GetMen()) + ".";

	u->Event(temp);
}

void Game::RunEnchantArmor(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_ENCHANT_ARMOR);
	int max = ItemDefs[I_MPLATE].mOut * level;
	int num = 0;
	int count = 0;
	unsigned int c;
	int found;

	// Figure out how many components there are
	for(c=0; c<sizeof(ItemDefs[I_MPLATE].mInput)/sizeof(Materials); c++) {
		if(ItemDefs[I_MPLATE].mInput[c].item != -1) count++;
	}

	while(max) {
		int i, a;
		found = 0;
		// See if we have enough of all items
		for(c=0; c<sizeof(ItemDefs[I_MPLATE].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MPLATE].mInput[c].item;
			a = ItemDefs[I_MPLATE].mInput[c].amt;
			if(i != -1) {
				if(u->items.GetNum(i) >= a) found++;
			}
		}
		// We do not, break.
		if(found != count) break;

		// Decrement our inputs
		for(c=0; c<sizeof(ItemDefs[I_MPLATE].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MPLATE].mInput[c].item;
			a = ItemDefs[I_MPLATE].mInput[c].amt;
			if(i != -1) {
				u->items.SetNum(i, u->items.GetNum(i) - a);
			}
		}
		// We've made one.
		num++;
		max--;
	}

	u->items.SetNum(I_MPLATE,u->items.GetNum(I_MPLATE) + num);
	u->Event(AString("Enchants ") + num + " mithril armor.");
	r->NotifySpell(u,S_ARTIFACT_LORE, &regions );
}

void Game::RunEnchantSwords(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_ENCHANT_SWORDS);
	int max = ItemDefs[I_MSWORD].mOut * level;
	int num = 0;
	int count = 0;
	unsigned int c;
	int found;

	// Figure out how many components there are
	for(c=0; c<sizeof(ItemDefs[I_MSWORD].mInput)/sizeof(Materials); c++) {
		if(ItemDefs[I_MSWORD].mInput[c].item != -1) count++;
	}

	while(max) {
		int i, a;
		found = 0;
		// See if we have enough of all items
		for(c=0; c<sizeof(ItemDefs[I_MSWORD].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MSWORD].mInput[c].item;
			a = ItemDefs[I_MSWORD].mInput[c].amt;
			if(i != -1) {
				if(u->items.GetNum(i) >= a) found++;
			}
		}
		// We do not, break.
		if(found != count) break;

		// Decrement our inputs
		for(c=0; c<sizeof(ItemDefs[I_MSWORD].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MSWORD].mInput[c].item;
			a = ItemDefs[I_MSWORD].mInput[c].amt;
			if(i != -1) {
				u->items.SetNum(i, u->items.GetNum(i) - a);
			}
		}
		// We've made one.
		num++;
		max--;
	}

	u->items.SetNum(I_MSWORD,u->items.GetNum(I_MSWORD) + num);
	u->Event(AString("Enchants ") + num + " mithril swords.");
	r->NotifySpell(u,S_ARTIFACT_LORE, &regions );
}

void Game::RunConstructGate(ARegion *r,Unit *u)
{
	if (TerrainDefs[r->type].similar_type == R_OCEAN) {
		u->Error("Gates may not be constructed at sea.");
		return;
	}

	if (r->gate) {
		u->Error("There is already a gate in that region.");
		return;
	}

	if (u->GetMoney() < 1000) {
		u->Error("Can't afford to construct a Gate.");
		return;
	}

	u->Event(AString("Constructs a Gate in ")+r->ShortPrint( &regions )+".");
	u->SetMoney(u->GetMoney() - 1000);
	r->gate = regions.numberofgates;
	regions.numberofgates++;
	r->NotifySpell(u,S_ARTIFACT_LORE, &regions );
}

void Game::RunEngraveRunes(ARegion *r,Object *o,Unit *u)
{
	if (!o->IsBuilding()) {
		u->Error("Runes of Warding may only be engraved on a building.");
		return;
	}

	if (o->incomplete > 0) {
		u->Error( "Runes of Warding may only be engraved on a completed "
				"building.");
		return;
	}

	int level = u->GetSkill(S_ENGRAVE_RUNES_OF_WARDING);

	switch (level) {
		case 5:
			if (o->type == O_MFORTRESS) break;
		case 4:
			if (o->type == O_CITADEL) break;
		case 3:
			if (o->type == O_CASTLE) break;
		case 2:
			if (o->type == O_FORT) break;
			if (o->type == O_MTOWER) break;
		case 1:
			if (o->type == O_TOWER) break;
		default:
			u->Error("Not high enough level to engrave Runes of Warding on "
					"that building.");
			return;
	}

	if (u->GetMoney() < 600) {
		u->Error("Can't afford to engrave Runes of Warding.");
		return;
	}

	u->SetMoney(u->GetMoney() - 600);
	if( o->type == O_MFORTRESS ) {
		o->runes = 5;
	} else if(o->type == O_MTOWER) {
		o->runes = 4;
	} else {
		o->runes = 3;
	}
	u->Event(AString("Engraves Runes of Warding on ") + *(o->name) + ".");
	r->NotifySpell(u,S_ARTIFACT_LORE, &regions );
}

void Game::RunSummonBalrog(ARegion *r,Unit *u)
{
	if (u->items.GetNum(I_BALROG)) {
		u->Error("Can't control more than one balrog.");
		return;
	}

	int level = u->GetSkill(S_SUMMON_BALROG);

	int num = (level * 20 + getrandom(100)) / 100;

	u->items.SetNum(I_BALROG,u->items.GetNum(I_BALROG) + num);
	u->Event(AString("Summons ") + ItemString(I_BALROG,num) + ".");
	r->NotifySpell(u,S_DEMON_LORE, &regions );
}

void Game::RunSummonDemon(ARegion *r,Unit *u)
{
	u->items.SetNum(I_DEMON,u->items.GetNum(I_DEMON) + 1);
	u->Event(AString("Summons ") + ItemString(I_DEMON,1) + ".");
	r->NotifySpell(u,S_DEMON_LORE, &regions );
}

void Game::RunSummonImps(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_SUMMON_IMPS);

	u->items.SetNum(I_IMP,u->items.GetNum(I_IMP) + level);
	u->Event(AString("Summons ") + ItemString(I_IMP,level) + ".");
	r->NotifySpell(u,S_DEMON_LORE, &regions );
}

void Game::RunCreateArtifact(ARegion *r,Unit *u,int skill,int item)
{
	int level = u->GetSkill(skill);
	unsigned int c;
	for(c = 0; c < sizeof(ItemDefs[item].mInput)/sizeof(Materials); c++) {
		if(ItemDefs[item].mInput[c].item == -1) continue;
		int amt = u->items.GetNum(ItemDefs[item].mInput[c].item);
		int cost = ItemDefs[item].mInput[c].amt;
		if(amt < cost) {
			u->Error(AString("Doesn't have sufficient ") +
					ItemDefs[ItemDefs[item].mInput[c].item].name +
					" to create that.");
			return;
		}
	}

	// Deduct the costs
	for(c = 0; c < sizeof(ItemDefs[item].mInput)/sizeof(Materials); c++) {
		if(ItemDefs[item].mInput[c].item == -1) continue;
		int amt = u->items.GetNum(ItemDefs[item].mInput[c].item);
		int cost = ItemDefs[item].mInput[c].amt;
		u->items.SetNum(ItemDefs[item].mInput[c].item, amt-cost);
	}

	int num = (level * ItemDefs[item].mOut + getrandom(100))/100;

	u->items.SetNum(item,u->items.GetNum(item) + num);
	u->Event(AString("Creates ") + ItemString(item,num) + ".");
	r->NotifySpell(u,S_ARTIFACT_LORE, &regions );
}

void Game::RunSummonLich(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_SUMMON_LICH);

	int num = ((2 * level * level) + getrandom(100))/100;

	u->items.SetNum(I_LICH,u->items.GetNum(I_LICH) + num);
	u->Event(AString("Summons ") + ItemString(I_LICH,num) + ".");
	r->NotifySpell(u,S_NECROMANCY, &regions );
}

void Game::RunRaiseUndead(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_RAISE_UNDEAD);

	int num = ((10 * level * level) + getrandom(100))/100;

	u->items.SetNum(I_UNDEAD,u->items.GetNum(I_UNDEAD) + num);
	u->Event(AString("Raises ") + ItemString(I_UNDEAD,num) + ".");
	r->NotifySpell(u,S_NECROMANCY, &regions );
}

void Game::RunSummonSkeletons(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_SUMMON_SKELETONS);

	int num = ((40 * level * level) + getrandom(100))/100;

	u->items.SetNum(I_SKELETON,u->items.GetNum(I_SKELETON) + num);
	u->Event(AString("Summons ") + ItemString(I_SKELETON,num) + ".");
	r->NotifySpell(u,S_NECROMANCY, &regions );
}

void Game::RunDragonLore(ARegion *r, Unit *u)
{
	int level = u->GetSkill(S_DRAGON_LORE);

	int num = u->items.GetNum(I_DRAGON);
	if (num >= level) {
		u->Error("Mage may not summon more dragons.");
		return;
	}

	int chance = level * level * 4;
	if (getrandom(100) < chance) {
		u->items.SetNum(I_DRAGON,num + 1);
		u->Event("Summons a dragon.");
	} else {
		u->Event("Attempts to summon a dragon, but fails.");
	}
	r->NotifySpell(u,S_EARTH_LORE, &regions );
}

void Game::RunBirdLore(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->castorders;
	int type = regions.GetRegionArray(r->zloc)->levelType;

	if(type != ARegionArray::LEVEL_SURFACE) {
		AString error = "CAST: Bird Lore may only be cast on the surface of ";
		error += Globals->WORLD_NAME;
		error += ".";
		u->Error(error.Str());
		return;
	}

	if (order->level < 3) {
		int dir = order->target;
		ARegion *tar = r->neighbors[dir];
		if (!tar) {
			u->Error("CAST: No such region.");
			return;
		}

		Farsight *f = new Farsight;
		f->faction = u->faction;
		f->level = u->GetSkill(S_BIRD_LORE);
		tar->farsees.Add(f);
		u->Event(AString("Sends birds to spy on ") +
				tar->Print( &regions ) + ".");
		r->NotifySpell(u,S_EARTH_LORE, &regions );
		return;
	}

	int level = u->GetSkill(S_BIRD_LORE);
	int max = (level - 2) * (level - 2);

	if (u->items.GetNum(I_EAGLE) >= max) {
		u->Error("CAST: Mage can't summon more eagles.");
		return;
	}

	u->items.SetNum(I_EAGLE,u->items.GetNum(I_EAGLE) + 1);
	u->Event("Summons an eagle.");
	r->NotifySpell(u,S_EARTH_LORE, &regions );
}

void Game::RunWolfLore(ARegion *r,Unit *u)
{
	if (TerrainDefs[r->type].similar_type != R_MOUNTAIN &&
		TerrainDefs[r->type].similar_type != R_FOREST) {
		u->Error("CAST: Can only summon wolves in mountain and "
				 "forest regions.");
		return;
	}

	int level = u->GetSkill(S_WOLF_LORE);
	int max = level * level * 4;

	int num = u->items.GetNum(I_WOLF);
	int summon = max - num;
	if (summon > level) summon = level;
	if (summon < 0) summon = 0;

	u->Event(AString("Casts Wolf Lore, summoning ") +
			ItemString(I_WOLF,summon) + ".");
	u->items.SetNum(I_WOLF,num + summon);
	r->NotifySpell(u,S_EARTH_LORE, &regions );
}

void Game::RunInvisibility(ARegion *r,Unit *u)
{
	CastUnitsOrder *order = (CastUnitsOrder *) u->castorders;
	int max = u->GetSkill(S_INVISIBILITY);
	max = max * max;

	int num = 0;
	forlist (&(order->units)) {
		Unit *tar = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (!tar) continue;
		if (tar->GetAttitude(r,u) < A_FRIENDLY) continue;
		num += tar->GetSoldiers();
	}

	if (num > max) {
		u->Error("CAST: Can't render that many men invisible.");
		return;
	}

	if (!num) {
		u->Error("CAST: No valid targets to turn invisible.");
		return;
	}
	{
		forlist (&(order->units)) {
			Unit *tar = r->GetUnitId((UnitId *) elem,u->faction->num);
			if (!tar) continue;
			if (tar->GetAttitude(r,u) < A_FRIENDLY) continue;
			tar->SetFlag(FLAG_INVIS,1);
			tar->Event(AString("Is rendered invisible by ") +
					*(u->name) + ".");
		}
	}

	r->NotifySpell(u,S_ILLUSION, &regions );
	u->Event("Casts invisibility.");
}

void Game::RunPhanDemons(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->castorders;
	int level = u->GetSkill(S_CREATE_PHANTASMAL_DEMONS);
	int create,max;

	if (order->level < 3) {
		create = I_IIMP;
		max = level * level * 4;
	} else {
		if (order->level < 5) {
			create = I_IDEMON;
			max = (level - 2) * (level - 2);
		} else {
			create = I_IBALROG;
			max = 1;
		}
	}

	if (order->target > max || order->target < 0) {
		u->Error("CAST: Can't create that many Phantasmal Demons.");
		return;
	}

	u->items.SetNum(create,order->target);
	u->Event("Casts Create Phantasmal Demons.");
	r->NotifySpell(u,S_ILLUSION, &regions );
}

void Game::RunPhanUndead(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->castorders;
	int level = u->GetSkill(S_CREATE_PHANTASMAL_UNDEAD);
	int create,max;

	if (order->level < 3) {
		create = I_ISKELETON;
		max = level * level * 4;
	} else {
		if (order->level < 5) {
			create = I_IUNDEAD;
			max = (level - 2) * (level - 2);
		} else {
			create = I_ILICH;
			max = 1;
		}
	}

	if (order->target > max || order->target < 0) {
		u->Error("CAST: Can't create that many Phantasmal Undead.");
		return;
	}

	u->items.SetNum(create,order->target);
	u->Event("Casts Create Phantasmal Undead.");
	r->NotifySpell(u,S_ILLUSION, &regions );
}

void Game::RunPhanBeasts(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->castorders;
	int level = u->GetSkill(S_CREATE_PHANTASMAL_BEASTS);
	int create,max;

	if (order->level < 3) {
		create = I_IWOLF;
		max = level * level * 4;
	} else {
		if (order->level < 5) {
			create = I_IEAGLE;
			max = (level - 2) * (level - 2);
		} else {
			create = I_IDRAGON;
			max = 1;
		}
	}

	if (order->target > max || order->target < 0) {
		u->Error("CAST: Can't create that many Phantasmal Beasts.");
		return;
	}

	u->items.SetNum(create,order->target);
	u->Event("Casts Create Phantasmal Beasts.");
	r->NotifySpell(u,S_ILLUSION, &regions );
}

void Game::RunEarthLore(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_EARTH_LORE);

	int amt = r->Wages() * level * 2;
	if (level > r->earthlore) r->earthlore = level;

	u->items.SetNum(I_SILVER,u->items.GetNum(I_SILVER) + amt);
	u->Event(AString("Casts Earth Lore, raising ") + amt + " silver.");
	r->NotifySpell(u,S_EARTH_LORE, &regions );
}

void Game::RunClearSkies(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_CLEAR_SKIES);

	if (level > r->clearskies) r->clearskies = level;

	u->Event(AString("Casts Clear Skies."));
	r->NotifySpell(u,S_WEATHER_LORE, &regions );
}

void Game::RunFarsight(ARegion *r,Unit *u)
{
	CastRegionOrder *order = (CastRegionOrder *) u->castorders;

	int level = u->GetSkill(S_FARSIGHT);
	if (!level) {
		u->Error("CAST: You don't know that spell.");
		return;
	}

	int range = 4 * level * level;

	if(Globals->EASIER_UNDERWORLD) {
		if(regions.GetRegionArray(r->zloc)->levelType ==
				ARegionArray::LEVEL_NEXUS) {
			u->Error("CAST: Farsight does not work from or to the Nexus.");
			return;
		}
	} else {
		if( regions.GetRegionArray( r->zloc )->levelType !=
				ARegionArray::LEVEL_SURFACE) {
			AString error = "CAST: Farsight only works on the surface of ";
			error += Globals->WORLD_NAME;
			error += ".";
			u->Error(error.Str());
			return;
		}
	}

	ARegion *tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	if (!tar) {
		u->Error("CAST: No such region.");
		return;
	}

	int dist;
	if(Globals->EASIER_UNDERWORLD) {
		dist = regions.GetPlanarDistance(tar, r);
	} else {
		dist = regions.GetDistance(tar, r);
	}

	if (dist > range) {
		u->Error("CAST: Can't view a region that distant.");
		return;
	}

	Farsight *f = new Farsight;
	f->faction = u->faction;
	f->level = level;
	f->unit = u;
	tar->farsees.Add(f);
	u->Event("Casts Farsight.");
}

void Game::RunDetectGates(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_GATE_LORE);

	if (level == 1) {
		u->Error("CAST: Casting Gate Lore at level 1 has no effect.");
		return;
	}

	u->Event("Casts Gate Lore, detecting nearby Gates:");
	int found = 0;
	for (int i=0; i<NDIRS; i++) {
		ARegion *tar = r->neighbors[i];
		if (tar) {
			if (tar->gate) {
				u->Event((tar->Print( &regions )) + " contains a Gate.");
				found = 1;
			}
		}
	}
	if (!found)
		u->Event("There are no nearby Gates.");
}

void Game::RunTeleport(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_TELEPORTATION);
	TeleportOrder *order = u->teleportorders;

	if (!level) {
		u->Error("CAST: Unit doesn't know teleportation.");
		return;
	}

	if(Globals->EASIER_UNDERWORLD) {
		if(regions.GetRegionArray(r->zloc)->levelType ==
				ARegionArray::LEVEL_NEXUS) {
			u->Error("CAST: Teleportation does not work to or from the Nexus.");
			return;
		}
	} else {
		if( regions.GetRegionArray( r->zloc )->levelType !=
				ARegionArray::LEVEL_SURFACE ) {
			u->Error("CAST: Teleportation only works on the surface.");
			return;
		}
	}

	int maxweight = level * 15;

	if (u->Weight() > maxweight) {
		u->Error("CAST: Can't carry that much when teleporting.");
		return;
	}

	ARegion *tar = regions.GetRegion( order->xloc, order->yloc, order->zloc );
	if (!tar) {
		u->Error("CAST: No such region.");
		return;
	}

	int maxdist = level * level * 2;
	int dist;

	if(Globals->EASIER_UNDERWORLD)
		dist = regions.GetPlanarDistance(r, tar);
	else
		dist = regions.GetDistance(r, tar);

	if (dist > maxdist) {
		u->Error("CAST: Can't teleport that far.");
		return;
	}

	if (TerrainDefs[tar->type].similar_type == R_OCEAN) {
		u->Error(AString("CAST: ") + tar->Print( &regions ) + " is an ocean.");
		return;
	}

	u->Event(AString("Teleports to ") + tar->Print( &regions ) + ".");
	u->MoveUnit( tar->GetDummy() );
}

void Game::RunGateJump(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_GATE_LORE);
	int nexgate = 0;
	if( !level ) {
		u->Error( "CAST: Unit doesn't have that skill." );
		return;
	}

	TeleportOrder *order = u->teleportorders;

	if (order->gate != -1 && level < 3) {
		u->Error("CAST: Unit Doesn't know Gate Lore at that level.");
		return;
	}

	nexgate = Globals->NEXUS_GATE_OUT &&
		(TerrainDefs[r->type].similar_type == R_NEXUS);
	if (!r->gate && !nexgate) {
		u->Error("CAST: There is no gate in that region.");
		return;
	}

	int maxweight = 10;
	if (order->gate != -1) level -= 2;
	switch (level) {
		case 1:
			maxweight = 15;
			break;
		case 2:
			maxweight = 100;
			break;
		case 3:
		case 4:
		case 5:
			maxweight = 1000;
			break;
	}

	int weight = u->Weight();

	forlist (&(order->units)) {
		Unit *taru = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (taru && taru != u) weight += taru->Weight();
	}

	if (weight > maxweight) {
		u->Error( "CAST: That mage cannot carry that much weight through "
				"a Gate." );
		return;
	}

	ARegion *tar;
	if (order->gate == -1) {
		int good = 0;
		int gatenum = getrandom(regions.numberofgates);
		tar = regions.FindGate(gatenum+1);

		if(tar && tar->zloc == r->zloc) good = 1;
		if(tar && nexgate && tar->zloc == ARegionArray::LEVEL_SURFACE)
			good = 1;

		while( !good ) {
			gatenum = getrandom(regions.numberofgates);
			tar = regions.FindGate(gatenum+1);
			if(tar && tar->zloc == r->zloc) good = 1;
			if(tar && nexgate && tar->zloc == ARegionArray::LEVEL_SURFACE)
				good = 1;
		}

		u->Event("Casts Random Gate Jump.");
	} else {
		if (order->gate < 1 || order->gate > regions.numberofgates) {
			u->Error("CAST: No such target gate.");
			return;
		}

		tar = regions.FindGate(order->gate);
		if (!tar) {
			u->Error("CAST: No such target gate.");
			return;
		}

		u->Event("Casts Gate Jump.");
	}

	int comma = 0;
	AString unitlist; {
		forlist(&(order->units)) {
			Location *loc = r->GetLocation((UnitId *) elem,u->faction->num);
			if (loc) {
				/* Don't do the casting unit yet */
				if (loc->unit == u) {
					delete loc;
					continue;
				}

				if (loc->unit->GetAttitude(r,u) < A_ALLY) {
					u->Error("CAST: Unit is not allied.");
				} else {
					if (comma) {
						unitlist += AString(", ") + AString(loc->unit->num);
					} else {
						unitlist += AString(loc->unit->num);
						comma = 1;
					}

					loc->unit->Event(AString("Is teleported through a ") +
							"Gate to " + tar->Print(&regions) + " by " +
							*u->name + ".");
					loc->unit->MoveUnit( tar->GetDummy() );
					if (loc->unit != u) loc->unit->ClearCastOrders();
				}
				delete loc;
			} else {
				u->Error("CAST: No such unit.");
			}
		}
	}

	u->Event(AString("Jumps through a Gate to ") +
			tar->Print( &regions ) + ".");
	if (comma) {
		u->Event(unitlist + " follow through the Gate.");
	}
	u->MoveUnit( tar->GetDummy() );
}

void Game::RunPortalLore(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_PORTAL_LORE);
	TeleportOrder *order = u->teleportorders;

	if (!level) {
		u->Error("CAST: Doesn't know Portal Lore.");
		return;
	}

	if (!u->items.GetNum(I_PORTAL)) {
		u->Error("CAST: Unit doesn't have a Portal.");
		return;
	}

	int maxweight = 50 * level;
	int maxdist = 2 * level * level;
	int weight = 0;
	forlist (&(order->units)) {
		Unit *taru = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (taru) weight += taru->Weight();
	}

    if (weight > maxweight) {
		u->Error("CAST: That mage cannot teleport that much weight through a "
				"Portal.");
		return;
	}

	Location *tar = regions.FindUnit(order->gate);
	if (!tar) {
		u->Error("CAST: No such target mage.");
		return;
	}

	if (tar->unit->faction->GetAttitude(u->faction->num) < A_FRIENDLY) {
		u->Error("CAST: Target mage is not friendly.");
		return;
	}

	if (tar->unit->type != U_MAGE) {
		u->Error("CAST: Target is not a mage.");
		return;
	}

	if (!tar->unit->items.GetNum(I_PORTAL)) {
		u->Error("CAST: Target does not have a Portal.");
		return;
	}

	if (regions.GetDistance(r,tar->region) > maxdist) {
		u->Error("CAST: Can't Portal Jump that far.");
		return;
	}

	u->Event("Casts Portal Jump.");

	{
		forlist(&(order->units)) {
			Location *loc = r->GetLocation((UnitId *) elem,u->faction->num);
			if (loc) {
				if (loc->unit->GetAttitude(r,u) < A_ALLY) {
					u->Error("CAST: Unit is not allied.");
				} else {
					loc->unit->Event(AString("Is teleported to ") +
							tar->region->Print( &regions ) +
							" by " + *u->name + ".");
					loc->unit->MoveUnit( tar->obj );
					if (loc->unit != u) loc->unit->ClearCastOrders();
				}
				delete loc;
			} else {
				u->Error("CAST: No such unit.");
			}
		}
	}

	delete tar;
}

void Game::RunTeleportOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			int foundone = 1;
			while (foundone) {
				foundone = 0;
				forlist(&o->units) {
					Unit * u = (Unit *) elem;
					if (u->teleportorders) {
						foundone = 1;
						switch (u->teleportorders->spell) {
							case S_GATE_LORE:
								RunGateJump(r,o,u);
								break;
							case S_TELEPORTATION:
								RunTeleport(r,o,u);
								break;
							case S_PORTAL_LORE:
								RunPortalLore(r,o,u);
								break;
						}
						delete u->teleportorders;
						u->teleportorders = 0;
						break;
					}
				}
			}
		}
	}
}
