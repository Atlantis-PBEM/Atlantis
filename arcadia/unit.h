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
// 2000/MAR/14 Larry Stanbery  Replaced specfic skill bonus functions with
//                             generic function.
//                             Added function to compute production bonus.
// 2001/FEB/07 Joseph Traub    Changes to allow mage support for city guards.
// 2001/Feb/18 Joseph Traub    Added support for Apprentices.
// 2001/Feb/25 Joseph Traub    Added a flag preventing units from crossing
//                             water.

#ifndef UNIT_CLASS
#define UNIT_CLASS

class Unit;
class UnitId;

#include "faction.h"
#include "alist.h"
#include "gameio.h"
#include "orders.h"
#include "fileio.h"
#include "skills.h"
#include "items.h"
#include "object.h"

enum {
	GUARD_NONE,
	GUARD_GUARD,
	GUARD_AVOID,
	GUARD_SET,
	GUARD_ADVANCE
};

enum {
	TAX_NONE,
	TAX_TAX,
	TAX_PILLAGE,
	TAX_AUTO,
};

enum {
	REVEAL_NONE,
	REVEAL_UNIT,
	REVEAL_FACTION
};

enum {
	TACTICS_NONE,
	TACTICS_DEFENSIVE,
	TACTICS_AGGRESSIVE
};

enum {
	U_NORMAL,
	U_SPECIALIST,
	U_LEADER,
	U_MAGE,
	U_GUARD,
	U_WMON,
	U_GUARDMAGE,
	U_APPRENTICE,
	NUNITTYPES
};

#define MAX_READY 4 // maximum number of ready weapons or armors

#define FLAG_BEHIND				0x0001
#define FLAG_NOCROSS_WATER		0x0002
#define FLAG_AUTOTAX			0x0004
#define FLAG_HOLDING			0x0008
#define FLAG_NOAID				0x0010
#define FLAG_INVIS				0x0020
#define FLAG_CONSUMING_UNIT		0x0040
#define FLAG_CONSUMING_FACTION	0x0080
#define FLAG_NOSPOILS			0x0100
#define FLAG_FLYSPOILS			0x0200
#define FLAG_WALKSPOILS			0x0400
#define FLAG_RIDESPOILS			0x0800
#define FLAG_SWIMSPOILS         0x1000
#define FLAG_SAILSPOILS         0x2000
#define FLAG_FIGHTASFOOT		0x4000
#define FLAG_FIGHTASRIDE		0x8000
#define FLAG_VISIB              0x10000
#define FLAG_SHARING			0x20000
#define FLAG_COMMANDER			0x40000


class UnitId : public AListElem {
	public:
		UnitId();
		~UnitId();
		AString Print();

		int unitnum; /* if 0, it is a new unit */
		int alias;
		int faction;
};

class UnitPtr : public AListElem {
	public:
		Unit * ptr;
};
UnitPtr *GetUnitList(AList *, Unit *);

class Unit : public AListElem
{
	public:
		Unit();
		Unit(int,Faction *,int = 0);
		~Unit();

		void SetMonFlags();
		void MakeWMon(char *,int,int);

		void Writeout( Aoutfile *f );
		void Readin( Ainfile *f, AList *, ATL_VER v );

		AString SpoilsReport(void);
		int CanGetSpoil(Item *i);
		void WriteReport(Areport *,int,int,int,int, int);
		AString GetName(int);
		AString MageReport();
		AString ReadyItem();
		AString StudyableSkills();
		AString * BattleReport(int);
		AString TemplateReport();

		void ClearOrders();
		void SafeClearOrders(); // Needed for Arcadia mages.
		void AddCastOrder(CastOrder *order);  //in spells.cpp, arcadian addition
		void ClearCastOrder();
		void ClearTeleportOrders();
		void DefaultOrders(Object *, int peasantfac=0);
		void SetName(AString *);
		void SetDescribe(AString *);
		void PostTurn(ARegion *reg);

		int IsMage();
		int IsLeader();
		int IsNormal();
		int GetMons();
		int GetMen();
//		int GetLeaders();
		int GetSoldiers();
		int GetRealSoldiers(); //excludes illusions
		int GetMen(int);
		void SetMen(int,int);
		int GetMoney();
		void SetMoney(int);
		int IsAlive();
		int IsReallyAlive(); //introduced for Arcadia.
		int IsStationary();  //introduced for SEND
		int GetEthnicity();

		int MaintCost();
		void Short(int, int);
		int SkillLevels();
		void SkillStarvation();
		Skill *GetSkillObject(int);
		int AgeDead(); //ARCADIA_MAGIC patch

		int GetAttackRiding();
		int GetDefenseRiding();

		//
		// These are rule-set specific, in extra.cpp.
		//
		// LLS
		int GetAttribute(char *ident);
		int PracticeAttribute(char *ident);
		int GetProductionBonus(int);

/* Do study stuff so experience gets learnt after study.
  Do production/combat/riding/stealth stuff so total level used.
  Do experience learning
  */

        int GetActiveSkill(int);         //checks if a skill has been player-disabled.
		int GetSkill(int);
		void SetSkill(int,int);
		void SetSkill(int,int,int); /* REAL_EXPERIENCE Patch */
		int IsASpeciality(int skill);
		int GetSkillKnowledgeMax(int);
		int GetSkillExperMax(int); /* REAL_EXPERIENCE Patch */
		int GetRealSkill(int);
		int GetDaysSkill(int);
		int GetExperSkill(int);  		
		void ForgetSkill(int);
		int CheckDepend(int,SkillDepend &s);
		int CanStudy(int);
		int Study(int,int, int overflow = 1); /* Returns 1 if it succeeds */
		void Experience(int sk,int exp,int group = 1, int dividenonspecials = 1); //if(group), multiplies experience by nummen.
		int Practice(int);
		void AdjustSkills(int overflow = 0);

		/* Return 1 if can see, 2 if can see faction */
		int CanSee(ARegion *,Unit *, int practice = 0);
		int CanCatch(ARegion *,Unit *);
		int AmtsPreventCrime(Unit *);
		int GetAttitude(ARegion *,Unit *); /* Get this unit's attitude toward
											  the Unit parameter */
		int Hostile();
		int Forbids(ARegion *,Unit *);
		int Weight();
	        int FlyingCapacity();
	        int RidingCapacity();
	        int SwimmingCapacity();
	        int WalkingCapacity();
		int CanFly(int);
		int CanRide(int);
		int CanWalk(int);
		int CanFly();
		int CanSwim();
		int CanReallySwim();
		int TryToSwim();
		int MoveType();
		int CalcMovePoints();
		int CanMoveTo(ARegion *,ARegion *);
		int GetFlag(int);
		void SetFlag(int,int);
		void CopyFlags(Unit *);
		int GetBattleItem(AString &itm);
		int GetArmor(AString &itm, int ass);
		int GetMount(AString &itm, int canFly, int canRide, int &bonus, int &type);
		int GetWeapon(AString &itm, int riding, int ridingBonus,
				int &attackBonus, int &defenseBonus, int &attacks);
		int CanUseWeapon(WeaponType *pWep, int riding);
		int CanUseWeapon(WeaponType *pWep);
		int Taxers(int);

		void MoveUnit( Object *newobj );
		int HasBoat(ARegion *reg); //in runorders.cpp

		void Event(const AString &);
		void Error(const AString &);
		void Message(const AString &); //BS mod

		Faction *faction;
		Faction *formfaction;
		Object *object;
		AString *name;
		AString *describe;
		int num;
		int type;
		int GetEnergy(int transferring = 0);
		int energy;  // this is only for mages in ARCADIA_MAGIC
		int dead;  // this is only for mages in ARCADIA_MAGIC
		int resurrects;  // this is only for mages in ARCADIA_MAGIC
	    int mastery;    // this is only for mages in ARCADIA_MAGIC
	    int transferred; //this is for portal mages in ARCADIA_MAGIC. It need not be saved in the gamefile; should be reset to zero every month.
		int foggy; // Has this unit got fog following him around?     	
		int MaxEnergy();  // this is only for mages in ARCADIA_MAGIC
		int EnergyRecharge();
		int EnergyMaintenance(int maxallowed);
		int MysticEvent(); // returns 0 if no mystic event, or 1-4 if there is (4 most serious).
		int GetCastCost(int spell, int extracost, int multiplier = 1, int levelpenalty = 0);
		int GetCombatCost(int spell, int multiplier = 1);
		int GetFirstCombatCost(int spell, int multiplier = 1);
		void WanderingExperience(int message);
		int GetSeniority();
		
		int alias;
		int gm_alias; /* used for gm manual creation of new units */
		int guard; /* Also, avoid- see enum above */
		int reveal;
		int tactics;
		int flags;
		int taxing;
		int movepoints;
		int canattack;
		int nomove;
		SkillList skills;
		ItemList items;
		ItemList itemsintransit;
		int combat;
		int readyItem;
		int readyWeapon[MAX_READY];
		int readyArmor[MAX_READY];
		AList oldorders;
		int needed; /* For assessing maintenance */
		int hunger;
		int stomach_space;
		int losses;
		int free;
		int practiced; // Has this unit practiced a skill this turn
		
		int crossbridge; //Used in combat code to see if a unit needs to cross a bridge to get to a combat. Set to zero in Game::GetSides.
		int marker; //use wherever, clear before using.
		void CrossHexside(ARegion *from, ARegion *to);

        //Set of routines to allow sharing of items/money between units, when wanted.
        int GetSharedNum(int);
        int GetSharedNum(int, int);
		int ConsumeShared(int,int);
		int GetSharedMoney();
		int GetSharedMoney(int);
		int ConsumeSharedMoney(int);
        
		/* Orders */
		int destroy;
		int enter;
		Object *build;
		int leftShip;
		UnitId *promote;
		AList findorders;
		AList giveorders;
		AList sendorders;
		AList withdraworders;
		AList wishdraworders;
		AList wishskillorders;
		AList bankorders;
		AList buyorders;
		AList sellorders;
		AList forgetorders;
		AList castlistorders;
		CastOrder *activecastorder;
		TeleportOrder *teleportorders;
		Order *stealorders;
		Order *monthorders;
		StudyOrder *herostudyorders;
		AttackOrder *attackorders;
		EvictOrder *evictorders;
		ARegion *advancefrom;

		AList exchangeorders;
		AList turnorders;
		int inTurnBlock;
		Order *presentMonthOrders;
		int presentTaxing;
		AList transportorders;
		Unit *former;
};

#endif
