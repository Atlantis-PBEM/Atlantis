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
// Date        Person         Comments
// ----        ------         --------
// 2000/MAR/14 Davis Kulis    Added a new reporting Template.
// 2001/Feb/18 Joseph Traub   Added Apprentices from Lacandon Conquest
#ifndef FACTION_CLASS
#define FACTION_CLASS

class Faction;
class Game;

#include "gameio.h"
#include "aregion.h"
#include "fileio.h"
#include "unit.h"
#include "battle1.h"
#include "skills.h"
#include "items.h"
#include "alist.h"
#include "astring.h"

enum {
    A_HOSTILE,
    A_UNFRIENDLY,
    A_NEUTRAL,
    A_FRIENDLY,
    A_ALLY,
    NATTITUDES
};

enum {
    F_WAR,
    F_TRADE,
    F_MAGIC,
    NFACTYPES
};

// DK
// LLS - make templates cleaner for save/restore
enum {
    TEMPLATE_OFF,
    TEMPLATE_SHORT,
    TEMPLATE_LONG,
    TEMPLATE_MAP,
    NTEMPLATES
};

enum {
    QUIT_NONE,
    QUIT_BY_ORDER,
    QUIT_BY_GM,
    QUIT_AND_RESTART,
    QUIT_WON_GAME,
    QUIT_GAME_OVER,
};

extern char ** AttitudeStrs;
extern char ** FactionStrs;

// LLS - include strings for the template enum
extern char **TemplateStrs;
int ParseTemplate(AString *);

int ParseAttitude(AString *);

int MagesByFacType(int);

class FactionVector {
public:
  FactionVector(int);
  ~FactionVector();

  void ClearVector();
  void SetFaction(int, Faction *);
  Faction *GetFaction(int);

  Faction **vector;
  int vectorsize;
};
  
class Attitude : public AListElem {
public:
  Attitude();
  ~Attitude();
  void Writeout(Aoutfile * );
  void Readin( Ainfile *, ATL_VER version );
  
  int factionnum;
  int attitude;
};

class Statistic {
public:
    Statistic();
    ~Statistic();

    int Value() const { return value; }
    void AddValue(int addvalue) {value += addvalue; }   //not used at the moment
    int value;
    int rank;
    int maxvalue;
};

class FormTemplate : public AListElem
{
    public:
        FormTemplate();
        ~FormTemplate();
        void Writeout(Aoutfile * );
        void Readin( Ainfile * );
        AString * GetLine(int);
        
        AString * name;
        AList orders;
};

class FactionPtr : public AListElem {
public:
  Faction * ptr;
};

class Faction : public AListElem
{
public:
    Faction();
    Faction(int);
    ~Faction();
    
    void Readin( Ainfile *, ATL_VER version );
    void Writeout( Aoutfile * );
    void View();
    
    void SetName(AString *);
    void SetNameNoChange( AString *str );
    void SetAddress( AString &strNewAddress );
    
    void CheckExist(ARegionList *);
    void Error(const AString &);
    void Event(const AString &);
    void Message(const AString &); //BS mod.
    
    AString FactionTypeStr();
    void WriteReport( Areport *f, Game *pGame );
    // LLS - write order template
    void WriteTemplate(Areport *f, Game *pGame);
    void WriteFormTemplates(Areport *f);
    void WriteFacInfo(Aoutfile *);
    
    void SetAttitude(int,int); /* faction num, attitude */
    /* if attitude == -1, clear it */
    int GetAttitude(int);
    void RemoveAttitude(int);
    
    int CanCatch(ARegion *,Unit *);
    int CanCatchAtSea(ARegion *,Unit *);
	/* Return 1 if can see, 2 if can see faction */
    int CanSee(ARegion *,Unit *, int practice = 0);
    
    void DefaultOrders();
    void TimesReward();
    
    void SetNPC();
    int IsNPC();

	void DiscoverItem(int item, int force, int full);

    int num;

    //
    // The type is only used if Globals->FACTION_LIMIT_TYPE ==
    // FACLIM_FACTION_TYPES
    //
    int type[NFACTYPES];
    int ethnicity;

    int lastchange;
    int lastorders;
    int unclaimed;
	int bankaccount;
	int interest; // not written to game.out
    AString * name;
    AString * address;
    AString * password;
    int times;
    int showunitattitudes;
    int temformat;
    char exists;
    int quit;
    int numshows;
    
    int nummages;
	int numapprentices;
	int numqms;
	int numtacts;
	Statistic guardedcities;
	Statistic totalsilver;
	Statistic itemnetworth;
	Statistic skillnetworth;
	Statistic totalnetworth;
	Statistic nummen;
	Statistic magepower;
	int labryinth; // Arcadia only, "labryinth counter". Needs to be saved in the gamefile.
    AList war_regions;
    AList trade_regions;

    /* Used when writing reports */
    AList present_regions;
    
    int defaultattitude;
    AList attitudes;
    SkillList skills;
	ItemList items;
	//List of templates
	AList formtemplates; //FORM_TEMPLATES
	AList labeltemplates; //FORM_TEMPLATES
	
    //
    // Both are lists of AStrings
    //
    AList extraPlayers;
    AList messages; // BS mod, extra list for the report.
    AList errors;
    AList events;
    AList battles;
    AList shows;
    AList itemshows;
	AList objectshows;
	AList terrainshows;

	// These are used for 'granting' units to a faction via the players.in
	// file
	ARegion *pReg;
	ARegion *pStartLoc;
	int start;
	int noStartLeader;
};

Faction * GetFaction(AList *,int);
Faction * GetFaction2(AList *,int); /*This AList is a list of FactionPtr*/

#endif
