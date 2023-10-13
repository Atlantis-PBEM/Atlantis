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

#include "gamedata.h"
#include "game.h"
#include "indenter.hpp"
#include <algorithm>
#include <iomanip>

char const *as[] = {
	"Hostile",
	"Unfriendly",
	"Neutral",
	"Friendly",
	"Ally"
};

char const **AttitudeStrs = as;

const std::string F_WAR = "War";
const std::string F_TRADE = "Trade";
const std::string F_MAGIC = "Magic";
const std::string F_MARTIAL = "Martial";

std::vector<std::string> ft { };
std::vector<std::string> *FactionTypes = &ft;

// LLS - fix up the template strings
char const *tp[] = {
	"off",
	"short",
	"long",
	"map"
};

char const **TemplateStrs = tp;

int ParseTemplate(AString *token)
{
	for (int i = 0; i < NTEMPLATES; i++)
		if (*token == TemplateStrs[i]) return i;
	return -1;
}

int ParseAttitude(AString *token)
{
	for (int i=0; i<NATTITUDES; i++)
		if (*token == AttitudeStrs[i]) return i;
	return -1;
}

FactionVector::FactionVector(int size)
{
	vector = new Faction *[size];
	vectorsize = size;
	ClearVector();
}

FactionVector::~FactionVector()
{
	delete vector;
}

void FactionVector::ClearVector()
{
	for (int i=0; i<vectorsize; i++) vector[i] = 0;
}

void FactionVector::SetFaction(int x, Faction *fac)
{
	vector[x] = fac;
}

Faction *FactionVector::GetFaction(int x)
{
	return vector[x];
}

Attitude::Attitude()
{
}

Attitude::~Attitude()
{
}

void Attitude::Writeout(ostream& f)
{
	f << factionnum << '\n';
	f << attitude << '\n';
}

void Attitude::Readin(istream &f)
{
	f >> factionnum;
	f >> attitude;
}

Faction::Faction()
{
	exists = 1;
	name = 0;
	
	for (auto &ft : *FactionTypes) {
		type[ft] = 1;
	}
	
	lastchange = -6;
	address = 0;
	password = 0;
	times = 0;
	showunitattitudes = 0;
	temformat = TEMPLATE_OFF;
	quit = 0;
	defaultattitude = A_NEUTRAL;
	unclaimed = 0;
	pReg = NULL;
	pStartLoc = NULL;
	noStartLeader = 0;
	startturn = 0;
	battleLogFormat = 0;
}

Faction::Faction(int n)
{
	exists = 1;
	num = n;
	
	for (auto &ft : *FactionTypes) {
		type[ft] = 1;
	}

	lastchange = -6;
	name = new AString;
	*name = AString("Faction (") + AString(num) + AString(")");
	address = new AString("NoAddress");
	password = new AString("none");
	times = 1;
	showunitattitudes = 0;
	temformat = TEMPLATE_LONG;
	defaultattitude = A_NEUTRAL;
	quit = 0;
	unclaimed = 0;
	pReg = NULL;
	pStartLoc = NULL;
	noStartLeader = 0;
	startturn = 0;
	battleLogFormat = 0;
}

Faction::~Faction()
{
	if (name) delete name;
	if (address) delete address;
	if (password) delete password;
	attitudes.DeleteAll();
}

void Faction::Writeout(ostream& f)
{
	f << num << '\n';

	for (auto &ft : *FactionTypes) {
		f << type[ft] << '\n';
	}

	f << lastchange << '\n';
	f << lastorders << '\n';
	f << unclaimed << '\n';
	f << *name << '\n';
	f << *address << '\n';
	f << *password << '\n';
	f << times << '\n';
	f << showunitattitudes << '\n';
	f << temformat << '\n';

	skills.Writeout(f);
	items.Writeout(f);
	f << defaultattitude << '\n';
	f << attitudes.Num() << '\n';
	forlist((&attitudes)) ((Attitude *) elem)->Writeout(f);
}

void Faction::Readin(istream& f)
{
	f >> num;

	for (auto &ft : *FactionTypes) {
		f >> type[ft];
	}

	f >> lastchange;
	f >> lastorders;
	f >> unclaimed;

	AString tmp;
	f >> ws >> tmp;
	name = new AString(tmp);
	f >> ws >> tmp;
	address = new AString(tmp);
	f >> ws >> tmp;
	password = new AString(tmp);
	f >> times;
	f >> showunitattitudes;
	f >> temformat;

	skills.Readin(f);
	items.Readin(f);

	f >> defaultattitude;
	int n;
	f >> n;
	for (int i = 0; i < n; i++) {
		Attitude* a = new Attitude;
		a->Readin(f);
		if (a->factionnum == num) delete a;
		else attitudes.Add(a);
	}
}

void Faction::View()
{
	AString temp;
	temp = AString("Faction ") + num + AString(" : ") + *name;
	Awrite(temp);
}

void Faction::SetName(AString* s)
{
	if (s) {
		AString* newname = s->getlegal();
		delete s;
		if (!newname) return;
		delete name;
		*newname += AString(" (") + num + ")";
		name = newname;
	}
}

void Faction::SetNameNoChange(AString *s)
{
	if (s) {
		delete name;
		name = new AString(*s);
	}
}

void Faction::SetAddress(AString &strNewAddress)
{
	delete address;
	address = new AString(strNewAddress);
}

AString Faction::FactionTypeStr()
{
	AString temp;
	if (IsNPC()) return AString("NPC");

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED) {
		return (AString("Unlimited"));
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		return(AString("Normal"));
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		bool comma = false;

		for (auto &ft : *FactionTypes) {
			auto value = type[ft];
			if (value) {
				if (comma) {
					temp += ", ";
				} else {
					comma = true;
				}
				temp += AString(ft) + " " + value;
			}
		}
		if (!comma) return AString("none");
	}
	return temp;
}

void PadStrEnd(AString& str, int len) {
	int size = str.Len();
	for (int i = len; i > size; i--) str += AString(" ");
}

void Faction::WriteFactionStats(ostream& f, Game *pGame, int ** citems) {
	f << ";Item                                      Rank  Max        Total\n";
	f << ";=====================================================================\n";
	for (int i = 0; i < NITEMS; i++)
	{
		if (ItemDefs[i].type & IT_SHIP) continue;

		int num = 0;
		forlist(&present_regions)
		{
			ARegionPtr *r = (ARegionPtr *)elem;
			forlist(&r->ptr->objects)
			{
				Object *obj = (Object *)elem;
				forlist(&obj->units)
				{
					Unit *unit = (Unit *)elem;
					if (unit->faction == this)
						num += unit->items.GetNum(i);
				}
			}
		}

		if (!num) continue;
		
		int place = 1;
		int max = 0;
		int total = 0;
		for (int pl = 0; pl < pGame->factionseq; pl++)
		{
			if (citems[pl][i] > num)
				place++;
			if (max < citems[pl][i])
				max = citems[pl][i];
			total += citems[pl][i];
		}
		
		string name = ItemString(i, num).const_str();
		string str = ";" + name;
		if (ItemDefs[i].type & IT_MONSTER && ItemDefs[i].type == IT_ILLUSION) {
			str += " (illusion)";
		}
		f << left << setw(43) << str;
		f << setw(6) << place;
		f << setw(11) << max;
		f << total << right << '\n';
	}
}

void Faction::WriteReport(ostream& f, Game *pGame, int ** citems)
{
	// make the output automatically wrap at 70 characters
	f << indent::wrap;

	if (IsNPC() && num == 1) {
		if (Globals->GM_REPORT || (pGame->month == 0 && pGame->year == 1)) {
			int i, j;
			// Put all skills, items and objects in the GM report
			shows.DeleteAll();
			for (i = 0; i < NSKILLS; i++) {
				for (j = 1; j < 6; j++) {
					shows.Add(new ShowSkill(i, j));
				}
			}
			if (shows.Num()) {
				f << "Skill reports:\n";
				forlist(&shows) {
					AString *string = ((ShowSkill *)elem)->Report(this);
					if (string) {
						f << '\n' << string->const_str() << '\n';
						delete string;
					}
				}
				shows.DeleteAll();
				f << '\n';
			}

			itemshows.DeleteAll();
			for (i = 0; i < NITEMS; i++) {
				AString *show = ItemDescription(i, 1);
				if (show) {
					itemshows.Add(show);
				}
			}
			if (itemshows.Num()) {
				f << "Item reports:\n";
				forlist(&itemshows) {
					f << '\n' << ((AString *)elem)->const_str() << '\n';
				}
				itemshows.DeleteAll();
				f << '\n';
			}

			objectshows.DeleteAll();
			for (i = 0; i < NOBJECTS; i++) {
				AString *show = ObjectDescription(i);
				if (show) {
					objectshows.Add(show);
				}
			}
			if (objectshows.Num()) {
				f << "Object reports:\n";
				forlist(&objectshows) {
					f << '\n' << ((AString *)elem)->const_str() << '\n';
				}
				objectshows.DeleteAll();
				f << '\n';
			}

			present_regions.DeleteAll();
			forlist(&(pGame->regions)) {
				ARegion *reg = (ARegion *)elem;
				ARegionPtr *ptr = new ARegionPtr;
				ptr->ptr = reg;
				present_regions.Add(ptr);
			}
			{
				forlist(&present_regions) {
					((ARegionPtr*)elem)->ptr->WriteReport(f, this, pGame->month, &(pGame->regions));
				}
			}
			present_regions.DeleteAll();
		}
		errors.DeleteAll();
		events.DeleteAll();
		battles.DeleteAll();
		return;
	}

	if (Globals->FACTION_STATISTICS) {
		f << ";Treasury:\n";
		f << ";\n";
		this->WriteFactionStats(f, pGame, citems);
		f << '\n';
	}

	f << "Atlantis Report For:\n";
	if ((Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) ||
			(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED)) {
		f << name->const_str() << '\n';
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f << name->const_str() << " (" << FactionTypeStr().const_str() << ")\n";
	}
	f << MonthNames[pGame->month] << ", Year " << pGame->year << "\n\n";

	f << "Atlantis Engine Version: " <<	ATL_VER_STRING(CURRENT_ATL_VER) << '\n';
	f << Globals->RULESET_NAME << ", Version: " << ATL_VER_STRING(Globals->RULESET_VERSION) << "\n\n";

	if (!times) {
		f << "Note: The Times is not being sent to you.\n\n";
	}

	if (!password || (*password == "none")) {
		f << "REMINDER: You have not set a password for your faction!\n\n";
	}

	if (Globals->MAX_INACTIVE_TURNS != -1) {
		int cturn = pGame->TurnNumber() - lastorders;
		if ((cturn >= (Globals->MAX_INACTIVE_TURNS - 3)) && !IsNPC()) {
			cturn = Globals->MAX_INACTIVE_TURNS - cturn;
			f << "WARNING: You have " << cturn
			  << " turns until your faction is automatically removed due to inactivity!\n\n";
		}
	}

	if (!exists) {
		if (quit == QUIT_AND_RESTART) {
			f << "You restarted your faction this turn. This faction "
			  << "has been removed, and a new faction has been started "
			  << "for you. (Your new faction report will come in a "
			  << "separate message.)\n";
		} else if (quit == QUIT_GAME_OVER) {
			f << "I'm sorry, the game has ended. Better luck in "
			  << "the next game you play!\n";
		} else if (quit == QUIT_WON_GAME) {
			f << "Congratulations, you have won the game!\n";
		} else {
			f << "I'm sorry, your faction has been eliminated.\n"
			  << "If you wish to restart, please let the "
			  << "Gamemaster know, and you will be restarted for "
			  << "the next available turn.\n";
		}
		f << '\n';
	}

	f << "Faction Status:\n";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f << "Mages: " << nummages << " (" << pGame->AllowedMages(this) << ")\n";
		
		if (Globals->APPRENTICES_EXIST) {
			string name = Globals->APPRENTICE_NAME;
			name[0] = toupper(name[0]);
			f << name << "s: " << numapprentices << " (" << pGame->AllowedApprentices(this) << ")\n";
		}
	}
	else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (Globals->FACTION_ACTIVITY != FactionActivityRules::DEFAULT) {
			int currentCost = GetActivityCost(FactionActivity::TAX);
			int maxAllowedCost = pGame->AllowedMartial(this);

			bool isMerged = Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED;
			f << (isMerged ? "Regions: " : "Activity: ") << currentCost << " (" << maxAllowedCost << ")\n";
		} else {			
			int taxRegions = GetActivityCost(FactionActivity::TAX);
			int tradeRegions = GetActivityCost(FactionActivity::TRADE);
			f << "Tax Regions: " << taxRegions << " (" << pGame->AllowedTaxes(this) << ")\n";
			f << "Trade Regions: " << tradeRegions << " (" << pGame->AllowedTrades(this) << ")\n";
		}

		if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
			f << "Quartermasters: " << numqms << " (" << pGame->AllowedQuarterMasters(this) << ")\n";
		}

		if (Globals->TACTICS_NEEDS_WAR) {
			f << "Tacticians: " << numtacts << " (" << pGame->AllowedTacticians(this) << ")\n";
		}

		f << "Mages: " << nummages << " (" << pGame->AllowedMages(this) << ")\n";

		if (Globals->APPRENTICES_EXIST) {
			string name = Globals->APPRENTICE_NAME;
			name[0] = toupper(name[0]);
			f << name << "s: " << numapprentices << " (" << pGame->AllowedApprentices(this) << ")\n";
		}
	}
	f << '\n';

	if (errors.Num()) {
		f << "Errors during turn:\n";
		forlist((&errors)) {
			f << ((AString *)elem)->const_str() << '\n';
		}
		errors.DeleteAll();
		f << '\n';
	}

	if (battles.Num()) {
		f << "Battles during turn:\n";
		forlist(&battles) {
			((BattlePtr *) elem)->ptr->Report(f, this);
		}
		battles.DeleteAll();
	}

	if (events.Num()) {
		f << "Events during turn:\n";
		forlist((&events)) {
			f << ((AString *) elem)->const_str() << '\n';
		}
		events.DeleteAll();
		f << '\n';
	}

	if (shows.Num()) {
		f << "Skill reports:\n";
		forlist(&shows) {
			AString* string = ((ShowSkill *) elem)->Report(this);
			if (string) {
				f << '\n' << string->const_str() << '\n';
			}
			delete string;
		}
		shows.DeleteAll();
		f << '\n';
	}

	if (itemshows.Num()) {
		f << "Item reports:\n";
		forlist(&itemshows) {
			f << '\n' << ((AString *) elem)->const_str() << '\n';
		}
		itemshows.DeleteAll();
		f << '\n';
	}

	if (objectshows.Num()) {
		f << "Object reports:\n";
		forlist(&objectshows) {
			f << '\n' << ((AString *) elem)->const_str() << '\n';
		}
		objectshows.DeleteAll();
		f << '\n';
	}

	/* Attitudes */
	f << "Declared Attitudes (default " << AttitudeStrs[defaultattitude] << "):\n";
	for (int i=0; i<NATTITUDES; i++) {
		int j=0;
		f << AttitudeStrs[i] << " : ";
		forlist((&attitudes)) {
			Attitude* a = (Attitude *) elem;
			if (a->attitude == i) {
				if (j) f << ", ";
				f << GetFaction(&(pGame->factions), a->factionnum)->name->const_str();
				j = 1;
			}
		}
		if (!j) f << "none";
		f << ".\n";
	}
	f << '\n';

	f << "Unclaimed silver: " << unclaimed << ".\n\n";

	forlist(&present_regions) {
		((ARegionPtr *) elem)->ptr->WriteReport(f, this, pGame->month, &(pGame->regions));
	} 
	f << '\n';
}

// LLS - write order template
void Faction::WriteTemplate(ostream& f, Game *pGame)
{
	AString temp;
	if (temformat == TEMPLATE_OFF) return;
	if (IsNPC()) return;

	f << indent::wrap;
	f << '\n';
	switch (temformat) {
		case TEMPLATE_SHORT:
			f << "Orders Template (Short Format):\n";
			break;
		case TEMPLATE_LONG:
			f << "Orders Template (Long Format):\n";
			break;
		case TEMPLATE_MAP:
			f << "Orders Template (Map Format):\n";
			break;
	}
	f << '\n';
	f << "#atlantis " << num;
	if (!(*password == "none")) {
		f << " \"" << *password << "\"";
	}
	f << '\n';

	forlist((&present_regions)) {
		((ARegionPtr *) elem)->ptr->WriteTemplate(f, this, &(pGame->regions), pGame->month);
	}

	f << "\n#end\n\n";
}

void Faction::WriteFacInfo(ostream &f)
{
	f << "Faction: " << num << '\n';
	f << "Name: " << name->const_str() << '\n';
	f << "Email: " << address->const_str() << '\n';
	f << "Password: " << password->const_str() << '\n';
	f << "LastOrders: " << lastorders << '\n';
	f << "FirstTurn: " << startturn << '\n';
	f << "SendTimes: " << times << '\n';
	f << "Template: " << TemplateStrs[temformat] << '\n';
	f << "Battle: na\n";
	forlist(&extraPlayers) {
		f << ((AString *) elem)->const_str() << '\n';
	}
	extraPlayers.DeleteAll();
}

void Faction::CheckExist(ARegionList* regs)
{
	if (IsNPC()) return;
	exists = 0;
	forlist(regs) {
		ARegion* reg = (ARegion *) elem;
		if (reg->Present(this)) {
			exists = 1;
			return;
		}
	}
}

void Faction::Error(const AString &s)
{
	if (IsNPC()) return;
	if (errors.Num() > 1000) {
		if (errors.Num() == 1001) {
			errors.Add(new AString("Too many errors!"));
		}
		return;
	}

	AString *temp = new AString(s);
	errors.Add(temp);
}

void Faction::Event(const AString &s)
{
	if (IsNPC()) return;
	AString *temp = new AString(s);
	events.Add(temp);
}

void Faction::RemoveAttitude(int f)
{
	forlist((&attitudes)) {
		Attitude *a = (Attitude *) elem;
		if (a->factionnum == f) {
			attitudes.Remove(a);
			delete a;
			return;
		}
	}
}

int Faction::GetAttitude(int n)
{
	if (n == num) return A_ALLY;
	forlist((&attitudes)) {
		Attitude *a = (Attitude *) elem;
		if (a->factionnum == n)
			return a->attitude;
	}
	return defaultattitude;
}

void Faction::SetAttitude(int num, int att)
{
	forlist((&attitudes)) {
		Attitude *a = (Attitude *) elem;
		if (a->factionnum == num) {
			if (att == -1) {
				attitudes.Remove(a);
				delete a;
				return;
			} else {
				a->attitude = att;
				return;
			}
		}
	}
	if (att != -1) {
		Attitude *a = new Attitude;
		a->factionnum = num;
		a->attitude = att;
		attitudes.Add(a);
	}
}

int Faction::CanCatch(ARegion *r, Unit *t)
{
	if (TerrainDefs[r->type].similar_type == R_OCEAN) return 1;

	int def = t->GetDefenseRiding();

	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u == t && o->type != O_DUMMY) return 1;
			if (u->faction == this && u->GetAttackRiding() >= def) return 1;
		}
	}
	return 0;
}

int Faction::CanSee(ARegion* r, Unit* u, int practice)
{
	int detfac = 0;
	if (u->faction == this) return 2;
	if (u->reveal == REVEAL_FACTION) return 2;
	int retval = 0;
	if (u->reveal == REVEAL_UNIT) retval = 1;
	if (u->guard == GUARD_GUARD) retval = 1;
	forlist((&r->objects)) {
		Object* obj = (Object *) elem;
		int dummy = 0;
		if (obj->type == O_DUMMY) dummy = 1;
		forlist((&obj->units)) {
			Unit* temp = (Unit *) elem;
			if (u == temp && dummy == 0) retval = 1;

			// penalty of 2 to stealth if assassinating and 1 if stealing
			// TODO: not sure about the reasoning behind the IMPROVED_AMTS part
			int stealpenalty = 0;
			if (Globals->HARDER_ASSASSINATION && u->stealorders){
				if (u->stealorders->type == O_STEAL) {
					stealpenalty = 1;
				} else if (u->stealorders->type == O_ASSASSINATE) {
					if (Globals->IMPROVED_AMTS){
						stealpenalty = 1;
					} else {
						stealpenalty = 2;
					}
				}
			}

			if (temp->faction == this) {
				if (temp->GetAttribute("observation") >
						u->GetAttribute("stealth") - stealpenalty) {
					if (practice) {
						temp->PracticeAttribute("observation");
						retval = 2;
					}
					else
						return 2;
				} else {
					if (temp->GetAttribute("observation") ==
							u->GetAttribute("stealth") - stealpenalty) {
						if (practice) temp->PracticeAttribute("observation");
						if (retval < 1) retval = 1;
					}
				}
				if (temp->GetSkill(S_MIND_READING) > 1) detfac = 1;
			}
		}
	}
	if (retval == 1 && detfac) return 2;
	return retval;
}

void Faction::DefaultOrders()
{
	activity.clear();
	numshows = 0;
}

void Faction::TimesReward()
{
	if (Globals->TIMES_REWARD) {
		Event(AString("Times reward of ") + Globals->TIMES_REWARD + " silver.");
		unclaimed += Globals->TIMES_REWARD;
	}
}

void Faction::SetNPC()
{
	for (auto &ft : *FactionTypes) {
		type[ft] = -1;
	}
}

int Faction::IsNPC()
{
	if (type[F_WAR] == -1 || type[F_MARTIAL] == -1) return 1;
	return 0;
}

Faction *GetFaction(AList *facs, int n)
{
	forlist(facs)
		if (((Faction *) elem)->num == n)
			return (Faction *) elem;
	return 0;
}

Faction *GetFaction2(AList *facs, int n)
{
	forlist(facs)
		if (((FactionPtr *) elem)->ptr->num == n)
			return ((FactionPtr *) elem)->ptr;
	return 0;
}

void Faction::DiscoverItem(int item, int force, int full)
{
	int seen, skill, i;
	AString skname;

	seen = items.GetNum(item);
	if (!seen) {
		if (full) {
			items.SetNum(item, 2);
		} else {
			items.SetNum(item, 1);
		}
		force = 1;
	} else {
		if (seen == 1) {
			if (full) {
				items.SetNum(item, 2);
			}
			force = 1;
		} else {
			full = 1;
		}
	}
	if (force) {
		itemshows.Add(ItemDescription(item, full));
		if (!full)
			return;
		// If we've found an item that grants a skill, give a
		// report on the skill granted (if we haven't seen it
		// before)
		skname = ItemDefs[item].grantSkill;
		skill = LookupSkill(&skname);
		if (skill != -1 && !(SkillDefs[skill].flags & SkillType::DISABLED)) {
			for (i = 1; i <= ItemDefs[item].maxGrant; i++) {
				if (i > skills.GetDays(skill)) {
					skills.SetDays(skill, i);
					shows.Add(new ShowSkill(skill, i));
				}
			}
		}
	}
}

int Faction::GetActivityCost(FactionActivity type) {
	int count = 0;
	for (auto &kv : this->activity) {
		auto regionActivity = kv.second;

		if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL) {
			// do not care on particular activity type, but each activity consumes one point
			count += regionActivity.size();
		} else if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED) {
			// Activity array item can be present due to some logic like trying to do
			// activity unsuccessfully because of different reasons
			if (regionActivity.size() > 0) {
				count++;
			}
		} else {
			// standard logic, each activity is counted separately
			if (regionActivity.find(type) != regionActivity.end()) {
				count++;
			}
		}
	}

	return count;
}

void Faction::RecordActivity(ARegion *region, FactionActivity type) {
	this->activity[region].insert(type);
}

bool Faction::IsActivityRecorded(ARegion *region, FactionActivity type) {
	auto regionActivity = this->activity[region];

	if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED) {
		if (regionActivity.size() > 0) {
			return true;
		}
	}

	return regionActivity.find(type) != std::end(regionActivity);
}
