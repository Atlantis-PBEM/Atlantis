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
// Date        Person          Comments
// ----        ------          --------
// 2000/MAR/14 Larry Stanbery  Modified the elimiation message.
// 2000/MAR/14 Davis Kulis     Added a new reporting Template.
// 2001/Feb/18 Joseph Traub    Added Apprentices concept from Lacandon Conquest
// 2001/Feb/21 Joseph Traub    Added a FACLIM_UNLIMITED option
// 2001/Feb/22 Joseph Traub    Modified to always save out faction type values
//                             even in scenarios where they aren't used or
//                             else your city guard and monster facs lose
//                             their NPC status in UNLIMITED
// 2002/AUG/01 Larry Stanbery  Seperated report from template: report.ID and
//                             template.ID are now created
//
#include "gamedata.h"
#include "game.h"

char *as[] = {
	"Hostile",
	"Unfriendly",
	"Neutral",
	"Friendly",
	"Ally"
};

char **AttitudeStrs = as;

char *fs[] = {
	"War",
	"Trade",
	"Magic"
};

char **FactionStrs = fs;

// LLS - fix up the template strings
char *tp[] = {
    "off",
    "short",
    "long",
    "map"
};

char **TemplateStrs = tp;

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

void Attitude::Writeout( Aoutfile *f )
{
	f->PutInt(factionnum);
	f->PutInt(attitude);
}

void Attitude::Readin( Ainfile *f, ATL_VER v )
{
	factionnum = f->GetInt();
	attitude = f->GetInt();
}

Faction::Faction()
{
	exists = 1;
	name = 0;
	for (int i=0; i<NFACTYPES; i++) {
		type[i] = 1;
	}
	lastchange = -6;
	address = 0;
	password = 0;
	times = 0;
	temformat = TEMPLATE_OFF;
	quit = 0;
	defaultattitude = A_NEUTRAL;
	unclaimed = 0;
	pReg = NULL;
	pStartLoc = NULL;
	noStartLeader = 0;
}

Faction::Faction(int n)
{
	exists = 1;
	num = n;
	for (int i=0; i<NFACTYPES; i++) {
		type[i] = 1;
	}
	lastchange = -6;
	name = new AString;
	*name = AString("Faction (") + AString(num) + AString(")");
	address = new AString("NoAddress");
	password = new AString("none");
	times = 1;
	temformat = TEMPLATE_LONG;
	defaultattitude = A_NEUTRAL;
	quit = 0;
	unclaimed = 0;
	bankaccount = 0;
	pReg = NULL;
	pStartLoc = NULL;
	noStartLeader = 0;
}

Faction::~Faction()
{
	if (name) delete name;
	if (address) delete address;
	if (password) delete password;
	attitudes.DeleteAll();
}

void Faction::Writeout( Aoutfile *f )
{
	f->PutInt(num);

	for (int i=0; i<NFACTYPES; i++) {
		f->PutInt(type[i]);
	}

	f->PutInt(lastchange);
	f->PutInt(lastorders);
	f->PutInt(unclaimed);
	f->PutInt(bankaccount);
	f->PutStr(*name);
	f->PutStr(*address);
	f->PutStr(*password);
	f->PutInt(times);
	f->PutInt(temformat);

	skills.Writeout(f);
	f->PutInt(-1);
	items.Writeout(f);
	f->PutInt(defaultattitude);
	f->PutInt(attitudes.Num());
	forlist((&attitudes))
		((Attitude *) elem)->Writeout( f );
}

void Faction::Readin( Ainfile *f, ATL_VER v )
{
	num = f->GetInt();
	int i;

	for (i=0; i<NFACTYPES; i++) {
		type[i] = f->GetInt();
	} 

	lastchange = f->GetInt();
	lastorders = f->GetInt();
	unclaimed = f->GetInt();
	bankaccount = f->GetInt();

	name = f->GetStr();
	address = f->GetStr();
	password = f->GetStr();
	times = f->GetInt();
	temformat = f->GetInt();

	skills.Readin(f);
	defaultattitude = f->GetInt();

	// Is this a new version of the game file
	if(defaultattitude == -1) {
		items.Readin(f);
		defaultattitude = f->GetInt();
	}

	int n = f->GetInt();
	for (i=0; i<n; i++) {
		Attitude * a = new Attitude;
		a->Readin(f,v);
		if (a->factionnum == num) {
			delete a;
		} else {
			attitudes.Add(a);
		}
	}
}

void Faction::View()
{
	AString temp;
	temp = AString("Faction ") + num + AString(" : ") + *name;
	Awrite(temp);
}

void Faction::SetName(AString * s)
{
	if (s) {
		AString * newname = s->getlegal();
		delete s;
		if (!newname) return;
		delete name;
		*newname += AString(" (") + num + ")";
		name = newname;
	}
}

void Faction::SetNameNoChange( AString *s )
{
	if( s ) {
		delete name;
		name = new AString( *s );
	}
}

void Faction::SetAddress( AString &strNewAddress )
{
	delete address;
	address = new AString( strNewAddress );
}

AString Faction::FactionTypeStr()
{
	AString temp;
	if (IsNPC()) return AString("NPC");

	if( Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED) {
		return (AString("Unlimited"));
	} else if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		return( AString( "Normal" ));
	} else if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
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

void Faction::WriteReport( Areport *f, Game *pGame )
{
	if (IsNPC() && num == 1) {
		if(Globals->GM_REPORT || (pGame->month == 0 && pGame->year == 1)) {
			int i, j;
			// Put all skills, items and objects in the GM report
			shows.DeleteAll();
			for(i = 0; i < NSKILLS; i++) {
				for(j = 1; j < 6; j++) {
					shows.Add(new ShowSkill(i, j));
				}
			}
			if(shows.Num()) {
				f->PutStr("Skill reports:" );
				forlist(&shows) {
					AString *string = ((ShowSkill *)elem)->Report(this);
					if(string) {
						f->PutStr("");
						f->PutStr(*string);
						delete string;
					}
				}
				shows.DeleteAll();
				f->EndLine();
			}

			itemshows.DeleteAll();
			for(i = 0; i < NITEMS; i++) {
				AString *show = ItemDescription(i, 1);
				if(show) {
					itemshows.Add(show);
				}
			}
			if(itemshows.Num()) {
				f->PutStr("Item reports:");
				forlist(&itemshows) {
					f->PutStr("");
					f->PutStr(*((AString *)elem));
				}
				itemshows.DeleteAll();
				f->EndLine();
			}

			objectshows.DeleteAll();
			for(i = 0; i < NOBJECTS; i++) {
				AString *show = ObjectDescription(i);
				if(show) {
					objectshows.Add(show);
				}
			}
			if(objectshows.Num()) {
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
					((ARegionPtr*)elem)->ptr->WriteReport(f, this,
														  pGame->month,
														  &(pGame->regions));
				}
			}
			present_regions.DeleteAll();
		}
		errors.DeleteAll();
		events.DeleteAll();
		battles.DeleteAll();
		return;
	}

	f->PutStr("Atlantis Report For:");
	if((Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) ||
			(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED)) {
		f->PutStr( *name );
	} else if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f->PutStr(*name + " (" + FactionTypeStr() + ")");
	}
	f->PutStr(AString(MonthNames[ pGame->month ]) + ", Year " + pGame->year );
	f->EndLine();

	f->PutStr( AString( "Atlantis Engine Version: " ) +
			ATL_VER_STRING( CURRENT_ATL_VER ));
	f->PutStr( AString( Globals->RULESET_NAME ) + ", Version: " +
			ATL_VER_STRING( Globals->RULESET_VERSION ));
	f->EndLine();

	if (!times) {
		f->PutStr("Note: The Times is not being sent to you.");
		f->EndLine();
	}

	if(*password == "none") {
		f->PutStr("REMINDER: You have not set a password for your faction!");
		f->EndLine();
	}

	if(Globals->MAX_INACTIVE_TURNS != -1) {
		int cturn = pGame->TurnNumber() - lastorders;
		if((cturn >= (Globals->MAX_INACTIVE_TURNS - 3)) && !IsNPC()) {
			cturn = Globals->MAX_INACTIVE_TURNS - cturn;
			f->PutStr( AString("WARNING: You have ") + cturn +
					AString(" turns until your faction is automatically ")+
					AString("removed due to inactivity!"));
			f->EndLine();
		}
	}

	if (!exists) {
		if (quit == QUIT_AND_RESTART) {
			f->PutStr( "You restarted your faction this turn. This faction "
					"has been removed, and a new faction has been started "
					"for you. (Your new faction report will come in a "
					"separate message.)" );
		} else if( quit == QUIT_GAME_OVER ) {
			f->PutStr( "I'm sorry, the game has ended. Better luck in "
					"the next game you play!" );
		} else if( quit == QUIT_WON_GAME ) {
			f->PutStr( "Congratulations, you have won the game!" );
		} else {
			f->PutStr( "I'm sorry, your faction has been eliminated." );
			// LLS
			f->PutStr( "If you wish to restart, please let the "
					"Gamemaster know, and you will be restarted for "
					"the next available turn." );
		}
		f->PutStr( "" );
	}

	f->PutStr("Faction Status:");
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f->PutStr( AString("Mages: ") + nummages + " (" +
				pGame->AllowedMages( this ) + ")");
		if(Globals->APPRENTICES_EXIST) {
			f->PutStr( AString("Apprentices: ") + numapprentices + " (" +
					pGame->AllowedApprentices(this)+ ")");
		}
	} else if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f->PutStr( AString("Tax Regions: ") + war_regions.Num() + " (" +
				pGame->AllowedTaxes( this ) + ")");
		f->PutStr( AString("Trade Regions: ") + trade_regions.Num() + " (" +
				pGame->AllowedTrades( this ) + ")");
		f->PutStr( AString("Mages: ") + nummages + " (" +
				pGame->AllowedMages( this ) + ")");
		if(Globals->APPRENTICES_EXIST) {
			f->PutStr( AString("Apprentices: ") + numapprentices + " (" +
					pGame->AllowedApprentices(this)+ ")");
		}
	}
	f->PutStr("");

	if (errors.Num()) {
		f->PutStr("Errors during turn:");
		forlist((&errors)) {
			f->PutStr(*((AString *) elem));
		}
		errors.DeleteAll();
		f->EndLine();
	}

	if (battles.Num()) {
		f->PutStr("Battles during turn:");
		forlist(&battles) {
			((BattlePtr *) elem)->ptr->Report(f,this);
		}
		battles.DeleteAll();
	}

	if (events.Num()) {
		f->PutStr("Events during turn:");
		forlist((&events)) {
			f->PutStr(*((AString *) elem));
		}
		events.DeleteAll();
		f->EndLine();
	}

	if (shows.Num()) {
		f->PutStr("Skill reports:");
		forlist(&shows) {
			AString * string = ((ShowSkill *) elem)->Report(this);
			if (string) {
				f->PutStr("");
				f->PutStr(*string);
			}
			delete string;
		}
		shows.DeleteAll();
		f->EndLine();
	}

	if (itemshows.Num()) {
		f->PutStr("Item reports:");
		forlist(&itemshows) {
			f->PutStr("");
			f->PutStr(*((AString *) elem));
		}
		itemshows.DeleteAll();
		f->EndLine();
	}

	if(objectshows.Num()) {
		f->PutStr("Object reports:");
		forlist(&objectshows) {
			f->PutStr("");
			f->PutStr(*((AString *)elem));
		}
		objectshows.DeleteAll();
		f->EndLine();
	}

	/* Attitudes */
	AString temp = AString("Declared Attitudes (default ") +
		AttitudeStrs[defaultattitude] + "):";
	f->PutStr(temp);
	for (int i=0; i<NATTITUDES; i++) {
		int j=0;
		temp = AString(AttitudeStrs[i]) + " : ";
		forlist((&attitudes)) {
			Attitude * a = (Attitude *) elem;
			if (a->attitude == i) {
				if (j) temp += ", ";
				temp += *( GetFaction( &( pGame->factions ),
							a->factionnum)->name);
				j = 1;
			}
		}
		if (!j) temp += "none";
		temp += ".";
		f->PutStr(temp);
	}
	f->EndLine();

	temp = AString("Unclaimed silver: ") + unclaimed + ".";
	f->PutStr(temp);
	if (Globals->ALLOW_BANK & GameDefs::BANK_ENABLED) {
		temp = AString("Silver in the Bank: ") + bankaccount;
		if (Globals->ALLOW_BANK & GameDefs::BANK_TRADEINTEREST)
			temp += AString(" (+")+interest+AString(" interest)");
		temp += + ".";
		f->PutStr(temp);
	}	f->PutStr("");

	forlist(&present_regions) {
		((ARegionPtr *) elem)->ptr->WriteReport( f, this, pGame->month, &( pGame->regions ));
	} 
        // LLS - maybe we don't want this -- I'll assume not, for now 
	//f->PutStr("#end");
	f->EndLine();

}

// LLS - write order template
void Faction::WriteTemplate( Areport *f, Game *pGame )
{
    AString temp;
    int tFormat = temformat;
    if (!IsNPC())
    {
        if (tFormat == TEMPLATE_OFF)
            tFormat = TEMPLATE_SHORT;
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
            ((ARegionPtr *) elem)->ptr->WriteTemplate( f, this, &( pGame->regions ), pGame->month );
	}

	f->PutStr("");
	f->PutStr("#end");
	f->EndLine();
    }
}

void Faction::WriteFacInfo( Aoutfile *file )
{
	file->PutStr( AString( "Faction: " ) + num );
	file->PutStr( AString( "Name: " ) + *name );
	file->PutStr( AString( "Email: " ) + *address );
	file->PutStr( AString( "Password: " ) + *password );
	file->PutStr( AString( "LastOrders: " ) + lastorders );
	file->PutStr( AString( "SendTimes: " ) + times );
        // LLS - write template info to players file
        file->PutStr( AString( "Template: " ) + TemplateStrs[temformat] );

	forlist( &extraPlayers ) {
		AString *pStr = (AString *) elem;
		file->PutStr( *pStr );
	}

	extraPlayers.DeleteAll();
}

void Faction::CheckExist(ARegionList * regs)
{
    if (IsNPC()) return;
	exists = 0;
	forlist(regs) {
		ARegion * reg = (ARegion *) elem;
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

void Faction::SetAttitude(int num,int att)
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

int Faction::CanSee(ARegion * r,Unit * u, int practise)
{
	int detfac = 0;
	if (u->faction == this) return 2;
	if (u->reveal == REVEAL_FACTION) return 2;
	int retval = 0;
	if (u->reveal == REVEAL_UNIT) retval = 1;
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		int dummy = 0;
		if (obj->type == O_DUMMY) dummy = 1;
		forlist((&obj->units)) {
			Unit * temp = (Unit *) elem;
			if (u == temp && dummy == 0) retval = 1;
			if (temp->faction == this) {
				if (temp->GetSkill(S_OBSERVATION) > u->GetSkill(S_STEALTH)) {
					if (practise) {
						temp->Practise(S_OBSERVATION);
						temp->Practise(S_TRUE_SEEING);
						retval = 2;
					}
					else
						return 2;
				} else {
					if (temp->GetSkill(S_OBSERVATION)==u->GetSkill(S_STEALTH)) {
						if (practise) {
							temp->Practise(S_OBSERVATION);
							temp->Practise(S_TRUE_SEEING);
						}
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
		Event(AString("Times reward of ") + Globals->TIMES_REWARD + " silver.");
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
	int seen = items.GetNum(item);
	if(!seen) {
		if(full) {
			items.SetNum(item, 2);
		} else {
			items.SetNum(item, 1);
		}
		force = 1;
	} else {
		if(seen == 1) {
			if(full) {
				items.SetNum(item, 2);
			}
			force = 1;
		} else {
			full = 1;
		}
	}
	if(force) {
		itemshows.Add(ItemDescription(item, full));
	}   
}
