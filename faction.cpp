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

char const *as[] = {
	"Hostile",
	"Unfriendly",
	"Neutral",
	"Friendly",
	"Ally"
};

char const **AttitudeStrs = as;

char const *fs[] = {
	"War",
	"Trade",
	"Magic"
};

char const **FactionStrs = fs;

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

void Attitude::Writeout(Aoutfile *f)
{
	f->PutInt(factionnum);
	f->PutInt(attitude);
}

void Attitude::Readin(Ainfile *f, ATL_VER v)
{
	factionnum = f->GetInt();
	attitude = f->GetInt();
}

Faction::Faction()
{
	exists = 1;
	name_ = nullptr;
	for (int i=0; i<NFACTYPES; i++) {
		type[i] = 1;
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
}

Faction::Faction(int n)
{
	exists = 1;
	num = n;
	for (int i=0; i<NFACTYPES; i++) {
		type[i] = 1;
	}
	lastchange = -6;
	name_ = new AString;
	*name_ = AString("Faction");
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
}

Faction::~Faction()
{
	if (name_) delete name_;
	if (address) delete address;
	if (password) delete password;
	attitudes.DeleteAll();
}

void Faction::Writeout(Aoutfile *f)
{
	f->PutInt(num);

	for (int i=0; i<NFACTYPES; i++) f->PutInt(type[i]);

	f->PutInt(lastchange);
	f->PutInt(lastorders);
	f->PutInt(unclaimed);
	f->PutStr(*name_);
	f->PutStr(*address);
	f->PutStr(*password);
	f->PutInt(times);
	f->PutInt(showunitattitudes);
	f->PutInt(temformat);

	skills.Writeout(f);
	items.Writeout(f);
	f->PutInt(defaultattitude);
	f->PutInt(attitudes.Num());
	forlist((&attitudes)) ((Attitude *) elem)->Writeout(f);
}

void Faction::Readin(Ainfile *f, ATL_VER v)
{
	num = f->GetInt();
	int i;

	for (i=0; i<NFACTYPES; i++) type[i] = f->GetInt();

	lastchange = f->GetInt();
	lastorders = f->GetInt();
	unclaimed = f->GetInt();

	name_ = f->GetStr();
	address = f->GetStr();
	password = f->GetStr();
	times = f->GetInt();
	showunitattitudes = f->GetInt();
	temformat = f->GetInt();

	skills.Readin(f);
	items.Readin(f);

	defaultattitude = f->GetInt();
	int n = f->GetInt();
	for (i=0; i<n; i++) {
		Attitude* a = new Attitude;
		a->Readin(f, v);
		if (a->factionnum == num) delete a;
		else attitudes.Add(a);
	}

	// if (skills.GetDays(S_BUILDING) > 1)
	//	shows.Add(new ShowSkill(S_BUILDING, 2));
}

void Faction::View()
{
	AString temp;
	temp = AString("Faction ") + num + AString(" : ") + name();
	Awrite(temp);
}

void Faction::SetName(AString* s)
{
	if (s) {
		AString* newname = s->getlegal();
		delete s;
		if (!newname) return;
		delete name_;
		name_ = newname;
	}
}

AString Faction::name() const
{
	return *name_ + AString(" (") + num + ")";
}

void Faction::SetNameNoChange(AString *s)
{
	if (s) {
		delete name_;
		name_ = new AString(*s);
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
		int comma = 0;
		for (int i=0; i<NFACTYPES; i++) {
			if (type[i]) {
				if (comma) {
					temp += ", ";
				} else {
					comma = 1;
				}
				temp += AString(FactionStrs[i]) + " " + type[i];
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

void Faction::WriteFactionStats(Areport *f, Game *pGame, int ** citems) {
	f->PutStr(";Item                                      Rank  Max        Total");
	f->PutStr(";=====================================================================");

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
		
		AString str = AString(";") + ItemString(i, num);
		if (ItemDefs[i].type & IT_MONSTER && ItemDefs[i].type == IT_ILLUSION)
		{
			str += AString(" (illusion)");
		}
		str += AString(" ");
		PadStrEnd(str, 43);

		str += AString(place);
		PadStrEnd(str, 49);
		
		str += AString(max);
		PadStrEnd(str, 60);

		str += AString(total);
		f->PutStr(str);
	}
}

void Faction::WriteReport(JsonReport &of, Game *pGame, int ** citems)
{
	Areport *f = of.PlainFile();
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
				f->PutStr("Skill reports:");
				forlist(&shows) {
					AString *string = ((ShowSkill *)elem)->Report(this);
					if (string) {
						f->PutStr("");
						f->PutStr(*string);
						delete string;
					}
				}
				shows.DeleteAll();
				f->EndLine();
			}

			itemshows.DeleteAll();
			for (i = 0; i < NITEMS; i++) {
				AString *show = ItemDescription(i, 1);
				if (show) {
					itemshows.Add(show);
				}
			}
			if (itemshows.Num()) {
				f->PutStr("Item reports:");
				forlist(&itemshows) {
					f->PutStr("");
					f->PutStr(*((AString *)elem));
				}
				itemshows.DeleteAll();
				f->EndLine();
			}

			objectshows.DeleteAll();
			for (i = 0; i < NOBJECTS; i++) {
				AString *show = ObjectDescription(i);
				if (show) {
					objectshows.Add(show);
				}
			}
			if (objectshows.Num()) {
				f->PutStr("Object reports:");
				forlist(&objectshows) {
					f->PutStr("");
					f->PutStr(*((AString *)elem));
				}
				objectshows.DeleteAll();
				f->EndLine();
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
					((ARegionPtr*)elem)->ptr->WriteReport(of, this,
																pGame->month,
																&(pGame->regions));
				}
			}
			present_regions.DeleteAll();
		}
		errors.DeleteAll();
		battles.DeleteAll();
		return;
	}

	if (Globals->FACTION_STATISTICS) {
		f->PutStr(";Treasury:");
		f->PutStr(";");
		this->WriteFactionStats(f, pGame, citems);
		f->EndLine();
	}

	f->PutStr("Atlantis Report For:");

	of.StartDict("faction");
	of.PutPairStr("name", name_->Str());
	of.PutPairInt("number", num);

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED) {
		f->PutStr(name());
		of.PutPairStr("type", "Unlimited");
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f->PutStr(name());
		of.PutPairStr("type", "Normal");
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f->PutStr(name() + " (" + FactionTypeStr() + ")");
		of.StartDict("type");
		for (int i = 0; i < NFACTYPES; ++i)
		{
			of.PutPairInt(FactionStrs[i], type[i]);
		}
		of.EndDict(); // faction::type
	}
	of.EndDict(); // faction

	f->PutStr(AString(MonthNames[ pGame->month ]) + ", Year " + pGame->year);
	f->EndLine();
	of.StartDict("date");
	of.PutPairStr("month", MonthNames[pGame->month]);
	of.PutPairInt("year", pGame->year);
	of.EndDict();

	AString atl_ver_str = ATL_VER_STRING(CURRENT_ATL_VER);
	f->PutStr(AString("Atlantis Engine Version: ") +
			atl_ver_str);
	AString rules_ver_str = ATL_VER_STRING(Globals->RULESET_VERSION);
	f->PutStr(AString(Globals->RULESET_NAME) + ", Version: " +
			rules_ver_str);
	f->EndLine();
	of.StartDict("engine");
	of.PutPairStr("version", atl_ver_str.Str());
	of.StartDict("ruleset");
	of.PutPairStr("name", Globals->RULESET_NAME);
	of.PutPairStr("version", rules_ver_str.Str());
	of.EndDict(); // engine::ruleset
	of.EndDict(); // engine

	if (!times) {
		f->PutStr("Note: The Times is not being sent to you.");
		f->EndLine();
	}
	of.PutPairInt("newssheet", times);

	if (!password || (*password == "none")) {
		f->PutStr("REMINDER: You have not set a password for your faction!");
		f->EndLine();
	}
	of.PutPairStr("password", password->Str());

	int count_down = -1;
	const int cturn = pGame->TurnNumber() - lastorders;
	if (Globals->MAX_INACTIVE_TURNS != -1) {
		if (pGame->TurnNumber() != lastorders)
			count_down = Globals->MAX_INACTIVE_TURNS - cturn;

		if ((cturn >= (Globals->MAX_INACTIVE_TURNS - 3)) && !IsNPC()) {
			f->PutStr(AString("WARNING: You have ") + count_down +
					AString(" turns until your faction is automatically ")+
					AString("removed due to inactivity!"));
			f->EndLine();
		}
	}
	of.PutPairInt("turnCountdown", count_down);

	int tmp_quit = 0;
	if (!exists) {
		if (quit == QUIT_AND_RESTART) {
			f->PutStr("You restarted your faction this turn. This faction "
					"has been removed, and a new faction has been started "
					"for you. (Your new faction report will come in a "
					"separate message.)");
		} else if (quit == QUIT_GAME_OVER) {
			f->PutStr("I'm sorry, the game has ended. Better luck in "
					"the next game you play!");
		} else if (quit == QUIT_WON_GAME) {
			f->PutStr("Congratulations, you have won the game!");
		} else {
			f->PutStr("I'm sorry, your faction has been eliminated.");
			// LLS
			f->PutStr("If you wish to restart, please let the "
					"Gamemaster know, and you will be restarted for "
					"the next available turn.");
		}
		tmp_quit = quit != 0 ? quit : QUIT_BY_ORDER;

		f->PutStr("");
	}
	of.PutPairInt("quit", tmp_quit);

	f->PutStr("Faction Status:");
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		of.StartArray("factionStatus");
		f->PutStr(AString("Mages: ") + nummages + " (" +
				pGame->AllowedMages(this) + ")");
		of.StartDict(NULL);
		of.PutPairStr("key", "Mages");
		of.PutPairInt("amount", nummages);
		of.PutPairInt("max", pGame->AllowedMages(this));
		of.EndDict();

		if (Globals->APPRENTICES_EXIST) {
			AString temp;
			temp = (char) toupper(Globals->APPRENTICE_NAME[0]);
			temp += Globals->APPRENTICE_NAME + 1;
			temp += "s: ";
			temp += numapprentices;
			temp += " (";
			temp += pGame->AllowedApprentices(this);
			temp += ")";
			f->PutStr(temp);
			of.StartDict(NULL);
			of.PutPairStr("key", "Apprentices");
			of.PutPairInt("amount", numapprentices);
			of.PutPairInt("max", pGame->AllowedApprentices(this));
			of.EndDict();
		}
		of.EndArray();
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		of.StartArray("factionStatus");
		f->PutStr(AString("Tax Regions: ") + war_regions.Num() + " (" +
				pGame->AllowedTaxes(this) + ")");
		of.StartDict(NULL);
		of.PutPairStr("key", "Tax Regions");
		of.PutPairInt("amount", war_regions.Num());
		of.PutPairInt("max", pGame->AllowedTaxes(this));
		of.EndDict();

		f->PutStr(AString("Trade Regions: ") + trade_regions.Num() + " (" +
				pGame->AllowedTrades(this) + ")");
		of.StartDict(NULL);
		of.PutPairStr("key", "Trade Regions");
		of.PutPairInt("amount", trade_regions.Num());
		of.PutPairInt("max", pGame->AllowedTrades(this));
		of.EndDict();

		if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
			f->PutStr(AString("Quartermasters: ") + numqms + " (" +
					pGame->AllowedQuarterMasters(this) + ")");
			of.StartDict(NULL);
			of.PutPairStr("key", "Quartermasters");
			of.PutPairInt("amount", numqms);
			of.PutPairInt("max", pGame->AllowedQuarterMasters(this));
			of.EndDict();
		}

		if (Globals->TACTICS_NEEDS_WAR) {
			f->PutStr(AString("Tacticians: ") + numtacts + " (" +
					pGame->AllowedTacticians(this) + ")");
			of.StartDict(NULL);
			of.PutPairStr("key", "Tacticians");
			of.PutPairInt("amount", numtacts);
			of.PutPairInt("max", pGame->AllowedTacticians(this));
			of.EndDict();
		}

		f->PutStr(AString("Mages: ") + nummages + " (" +
				pGame->AllowedMages(this) + ")");
		of.StartDict(NULL);
		of.PutPairStr("key", "Mages");
		of.PutPairInt("amount", nummages);
		of.PutPairInt("max", pGame->AllowedMages(this));
		of.EndDict();

		if (Globals->APPRENTICES_EXIST) {
			AString temp;
			temp = (char) toupper(Globals->APPRENTICE_NAME[0]);
			temp += Globals->APPRENTICE_NAME + 1;
			temp += "s: ";
			temp += numapprentices;
			temp += " (";
			temp += pGame->AllowedApprentices(this);
			temp += ")";
			f->PutStr(temp);

			of.StartDict(NULL);
			of.PutPairStr("key", "Apprentices");
			of.PutPairInt("amount", numapprentices);
			of.PutPairInt("max", pGame->AllowedApprentices(this));
			of.EndDict();
		}
		of.EndArray();
	}
	f->EndLine();

	if (errors.Num()) {
		// TODO: computer friendly errors
		f->PutStr("Errors during turn:");
		of.StartArray("errors");
		forlist((&errors)) {
			AString *p = (AString*)elem;
			f->PutStr(*p);
			of.PutStr(p->Str());
		}
		of.EndArray();
		errors.DeleteAll();
		f->EndLine();
	}

	if (battles.Num()) {
		f->PutStr("Battles during turn:");
		of.StartArray("battles");
		forlist(&battles) {
			BattlePtr *b = (BattlePtr*)elem;
			b->ptr->Report(f, this);
			b->ptr->Creport(of, this);
		}
		of.EndArray();
		battles.DeleteAll();
	}

	if (!events_.empty()) {
		f->PutStr("Events during turn:");
		of.StartArray("events");
		for(const auto &e : events_) {
			const std::string msg = e->str(pGame->regions);
			f->PutStr(msg.c_str());
			e->writeJson(of, pGame->regions);
		}
		of.EndArray();
		f->EndLine();
	}

	if (shows.Num()) {
		// TODO: computer friendly skill shows
		f->PutStr("Skill reports:");
		of.StartArray("skillReports");
		forlist(&shows) {
			AString* string = ((ShowSkill *) elem)->Report(this);
			if (string) {
				f->PutStr("");
				f->PutStr(*string);
				of.PutStr(string->Str());
			}
			delete string;
		}
		of.EndArray();
		shows.DeleteAll();
		f->EndLine();
	}

	if (itemshows.Num()) {
		// TODO: computer friendly item shows
		f->PutStr("Item reports:");
		of.StartArray("itemReports");
		forlist(&itemshows) {
			f->PutStr("");
			AString *e = (AString*)elem;
			f->PutStr(*e);
			of.PutStr(e->Str());
		}
		of.EndArray();
		itemshows.DeleteAll();
		f->EndLine();
	}

	if (objectshows.Num()) {
		// TODO: computer friendly object shows
		f->PutStr("Object reports:");
		of.StartArray("objectReports");
		forlist(&objectshows) {
			f->PutStr("");
			AString *e = (AString*)elem;
			f->PutStr(*e);
			of.PutStr(e->Str());
		}
		of.EndArray();
		objectshows.DeleteAll();
		f->EndLine();
	}

	/* Attitudes */
	AString temp = AString("Declared Attitudes (default ") +
		AttitudeStrs[defaultattitude] + "):";
	f->PutStr(temp);
	of.StartDict("attitudes");
	of.PutPairStr("default", AttitudeStrs[defaultattitude]);
	for (int i=0; i<NATTITUDES; i++) {
		bool has_this_attitude = false;
		int j=0;
		temp = AString(AttitudeStrs[i]) + " : ";
		forlist((&attitudes)) {
			Attitude* a = (Attitude *) elem;
			if (a->attitude == i) {
				if (j) temp += ", ";
				AString fac_name = GetFaction(&(pGame->factions), a->factionnum)->name();
				temp += fac_name;
				j = 1;
				if (!has_this_attitude) {
					has_this_attitude = true;
					of.StartArray(AttitudeStrs[i]);
				}
				of.PutStr(fac_name.Str());
			}
		}
		if (has_this_attitude)
		{
			of.EndArray();
		}
		if (!j) temp += "none";
		temp += ".";
		f->PutStr(temp);
	}
	f->EndLine();
	of.EndDict();

	temp = AString("Unclaimed silver: ") + unclaimed + ".";
	f->PutStr(temp);
	f->PutStr("");
	of.PutPairInt("unclaimedSilver", unclaimed);

	of.StartArray("regions");
	forlist(&present_regions) {
		ARegionPtr *reg = (ARegionPtr*)elem;
		reg->ptr->WriteReport(of, this, pGame->month, &(pGame->regions));
	} 
	of.EndArray();
	// LLS - maybe we don't want this -- I'll assume not, for now 
	//f->PutStr("#end");
	f->EndLine();
	of.Close();
}

// LLS - write order template
void Faction::WriteTemplate(Areport *f, Game *pGame)
{
	AString temp;
	if (temformat == TEMPLATE_OFF)
		return;
	if (!IsNPC()) {
		f->PutStr("");
		switch (temformat) {
			case TEMPLATE_SHORT:
				f->PutStr("Orders Template (Short Format):");
				break;
			case TEMPLATE_LONG:
				f->PutStr("Orders Template (Long Format):");
				break;
			// DK
			case TEMPLATE_MAP:
				f->PutStr("Orders Template (Map Format):");
				break;
		}

		f->PutStr("");
		temp = AString("#atlantis ") + num;
		if (!(*password == "none")) {
			temp += AString(" \"") + *password + "\"";
		}
		f->PutStr(temp);
		forlist((&present_regions)) {
			// DK
			((ARegionPtr *) elem)->ptr->WriteTemplate(f, this, &(pGame->regions), pGame->month);
		}

		f->PutStr("");
		f->PutStr("#end");
		f->EndLine();
	}
}

void Faction::WriteFacInfo(Aoutfile *file)
{
	file->PutStr(AString("Faction: ") + num);
	file->PutStr(AString("Name: ") + *name_);
	file->PutStr(AString("Email: ") + *address);
	file->PutStr(AString("Password: ") + *password);
	file->PutStr(AString("LastOrders: ") + lastorders);
	file->PutStr(AString("FirstTurn: ") + startturn);
	file->PutStr(AString("SendTimes: ") + times);

	// LLS - write template info to players file
	file->PutStr(AString("Template: ") + TemplateStrs[temformat]);

	forlist(&extraPlayers) {
		AString *pStr = (AString *) elem;
		file->PutStr(*pStr);
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

void Faction::LogEvent(IEvent *e)
{
	if (!IsNPC())
		events_.push_back(e);
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
				if (temp->GetSkill(S_MIND_READING) > 2) detfac = 1;
			}
		}
	}
	if (retval == 1 && detfac) return 2;
	return retval;
}

void Faction::DefaultOrders()
{
	war_regions.DeleteAll();
	trade_regions.DeleteAll();
	numshows = 0;
}

void Faction::TimesReward()
{
	if (Globals->TIMES_REWARD) {
		AString msg("Times reward of ");
		msg += AString(Globals->TIMES_REWARD) + " silver.";
		LogEvent(new StrEvent(msg.Str()));
		unclaimed += Globals->TIMES_REWARD;
	}
}

void Faction::SetNPC()
{
	for (int i=0; i<NFACTYPES; i++) type[i] = -1;
}

int Faction::IsNPC()
{
	if (type[F_WAR] == -1) return 1;
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
