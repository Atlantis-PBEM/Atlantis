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
// Date        Person          Comment
// ----        ------          -------
// 2000/MAR/14 Larry Stanbery  Added a unit:faction list capability.
// 2000/MAR/25 Larry Stanbery  Added support for ruleset changes.
// 2001/Feb/18 Joseph Traub    Added support for Apprentices
// 2001/Feb/19 Joseph Traub    Upped version to 4.0.5
#ifndef GAME_CLASS
#define GAME_CLASS

class Game;

#include "aregion.h"
#include "alist.h"
#include "faction.h"
#include "production.h"
#include "object.h"

#define CURRENT_ATL_VER MAKE_ATL_VER( 4, 0, 10 )

class OrdersCheck
{
public:
    OrdersCheck();

    Aoutfile *pCheckFile;
    Unit dummyUnit;
    Faction dummyFaction;
    Order dummyOrder;
    int numshows;

    void Error( const AString &error );
};

class Game
{
    friend class Faction;
public:
    Game();
    ~Game();

    int NewGame();
    int OpenGame();
    void DummyGame();

	void DefaultWorkOrder();

    int RunGame();
    int EditGame( int *pSaveGame );
    int SaveGame();
    int WritePlayers();
    int ReadPlayers();
    int ReadPlayersLine( AString *pToken, AString *pLine, Faction *pFac,
                         int newPlayer );
    void WriteNewFac( Faction *pFac );

    int ViewMap(const AString &,const AString &);
    // LLS
    void UnitFactionMap();
	int GenRules(const AString &, const AString &, const AString &);
	int DoOrdersCheck( const AString &strOrders, const AString &strCheck );

    Faction *AddFaction(int setup=1);

    //
    // Give this particular game a chance to set up the faction. This is in
    // extra.cpp.
    //
    int SetupFaction( Faction *pFac );

    void ViewFactions();

    //
    // Get a new unit, with its number assigned.
    //
    Unit *GetNewUnit( Faction *fac, int an = 0 );

    //
    // Setup the array of units.
    //
	void SetupUnitSeq();
    void SetupUnitNums();

	// Fix broken boat numbers
	void FixBoatNums();
	// Fix broken/missing gates
	void FixGateNums();

    //
    // Get a unit by its number.
    //
    Unit *GetUnit( int num );

	// Handle special gm unit modification functions
	Unit *ParseGMUnit(AString *tag, Faction *pFac);

    int TurnNumber();

	// JLT
	// Functions to allow enabling/disabling parts of the data tables
	void ModifyTablesPerRuleset(void);

private:
    //
    // Game editing functions.
    //
    ARegion *EditGameFindRegion();
    void EditGameFindUnit();
    void EditGameRegion( ARegion *pReg );
    void EditGameUnit( Unit *pUnit );
    void EditGameUnitItems( Unit *pUnit );
    void EditGameUnitSkills( Unit *pUnit );
    void EditGameUnitMove( Unit *pUnit );

    void CreateOceanLairs();

    void PreProcessTurn();
    void ReadOrders();
    void RunOrders();
    void ClearOrders(Faction *);
    void MakeFactionReportLists();
    void CountAllMages();
	void CountAllApprentices();
    void WriteReport();
    void DeleteDeadFactions();

    //
    // Standard creation functions.
    //
    void CreateCityMons();
    void CreateWMons();
    void CreateLMons();
	void CreateVMons();

    //
    // Game-specific creation functions (see world.cpp).
    //
    void CreateWorld();
    void CreateNPCFactions();
    void CreateCityMon( ARegion *pReg, int percent, int needmage );
    int MakeWMon( ARegion *pReg );
    void MakeLMon( Object *pObj );

    void WriteSurfaceMap( Aoutfile *f, ARegionArray *pArr, int type );
    void WriteUnderworldMap( Aoutfile *f, ARegionArray *pArr, int type );
    char GetRChar( ARegion *r );
    AString GetXtraMap(ARegion *,int);

    // LLS
    // Functions to do upgrades to the ruleset -- should be in extras.cpp
    int UpgradeMajorVersion(int savedVersion);
    int UpgradeMinorVersion(int savedVersion);
    int UpgradePatchLevel(int savedVersion);

	// JLT
	// Functions to allow enabling/disabling parts of the data tables
	void EnableSkill(int sk); // Enabled a disabled skill
	void DisableSkill(int sk);  // Prevents skill being studied or used
	void ModifySkillDependancy(int sk, int i, int dep, int lev);
	void ModifySkillFlags(int sk, int flags);
	void ModifySkillCost(int sk, int cost);
	void ModifySkillSpecial(int sk, int special);
	void ModifySkillRange(int sk, int range);

	void EnableItem(int it); // Enables a disabled item
	void DisableItem(int it); // Prevents item being generated/produced
	void ModifyItemFlags(int it, int flags);
	void ModifyItemType(int it, int type);
	void ModifyItemWeight(int it, int weight);
	void ModifyItemBasePrice(int it, int price);
	void ModifyItemCapacities(int it, int walk, int ride, int fly, int swim);
	void ModifyItemProductionBooster(int it, int item, int bonus);
	void ModifyItemHitch(int it, int item, int bonus);
	void ModifyItemProductionSkill(int it, int sk, int lev);
	void ModifyItemProductionOutput(int it, int months, int count);
	void ModifyItemProductionInput(int it, int i, int input, int amount);
	void ModifyItemMagicSkill(int it, int sk, int lev);
	void ModifyItemMagicOutput(int it, int count);
	void ModifyItemMagicInput(int it, int i, int input, int amount);

	void ModifyRaceSkillLevels(int race, int special, int def);
	void ModifyRaceSkills(int race, int i, int sk);

	void ModifyMonsterAttackLevel(int mon, int lev);
	void ModifyMonsterDefense(int mon, int defenseType, int level);
	void ModifyMonsterAttacksAndHits(int mon, int numattacks, int hits,
			int regen);
	void ModifyMonsterSkills(int mon, int tact, int stealth, int obs);
	void ModifyMonsterSpecial(int mon, int special, int lev);
	void ModifyMonsterSpoils(int mon, int silver, int spoilType);
	void ModifyMonsterThreat(int mon, int num, int hostileChance);

	void ModifyWeaponSkills(int weap, int baseSkill, int orSkill);
	void ModifyWeaponFlags(int weap, int flags);
	void ModifyWeaponAttack(int weap, int wclass, int attackType, int numAtt);
	void ModifyWeaponBonuses(int weap, int attack, int defense, int vsMount);

	void ModifyArmorFlags(int armor, int flags);
	void ModifyArmorSaveFrom(int armor, int from);
	void ModifyArmorSaveValue(int armor, int wclass, int val);

	void ModifyMountSkill(int mount, int skill);
	void ModifyMountBonuses(int mount, int min, int max, int hampered);
	void ModifyMountSpecial(int mount, int special, int level);

	void EnableObject(int ob); // Enables a disabled object
	void DisableObject(int ob); // Prevents object being built
	void ModifyObjectFlags(int ob, int flags);
	void ModifyObjectDecay(int ob, int maxMaint, int maxMonthDecay, int mFact);
	void ModifyObjectProduction(int ob, int it);
	void ModifyObjectMonster(int ob, int monster);
	void ModifyObjectConstruction(int ob, int it, int num, int sk, int lev);
	void ModifyObjectManpower(int ob, int prot, int cap, int sail, int mages);

	void ClearTerrainRaces(int t);
	void ModifyTerrainRace(int t, int i, int r);
	void ModifyTerrainCoastRace(int t, int i, int r);
	void ClearTerrainItems(int t);
	void ModifyTerrainItems(int t, int i, int p, int c, int a);
	void ModifyTerrainWMons(int t, int freq, int smon, int bigmon, int hum);
	void ModifyTerrainLairChance(int t, int chance);
	void ModifyTerrainLair(int t, int i, int lair);
	void ModifyTerrainEconomy(int t, int pop, int wages, int econ, int move);

	void ModifyBattleItemFlags(int item, int flags);
	void ModifyBattleItemSpecial(int item, int special, int level);

	void ModifySpecialTargetFlags(int special, int targetflags);
	void ModifySpecialTargetObjects(int special, int index, int obj);
	void ModifySpecialTargetItems(int special, int index, int item);
	void ModifySpecialTargetEffects(int special, int index, int effect);
	void ModifySpecialEffectFlags(int special, int effectflags);
	void ModifySpecialShields(int special, int index, int type);
	void ModifySpecialDefenseMods(int special, int index, int type, int val);
	void ModifySpecialDamage(int special, int index, int type, int min,
			int val, int flags, int cls, int effect);

	void ModifyEffectFlags(int effect, int flags);
	void ModifyEffectAttackMod(int effect, int val);
	void ModifyEffectDefenseMod(int effect, int index, int type, int val);
	void ModifyEffectCancelEffect(int effect, int uneffect);

	void ModifyRangeFlags(int range, int flags);
	void ModifyRangeClass(int range, int rclass);
	void ModifyRangeMultiplier(int range, int mult);
	void ModifyRangeLevelPenalty(int range, int pen);

    AList factions;
    AList newfactions; /* List of strings */
    AList battles;
    ARegionList regions;
    int factionseq;
    unsigned int unitseq;
    Unit **ppUnits;
	unsigned int maxppunits;
    int shipseq;
    int year;
    int month;

    enum {
        GAME_STATUS_UNINIT,
        GAME_STATUS_NEW,
        GAME_STATUS_RUNNING,
        GAME_STATUS_FINISHED,
    };
    int gameStatus;

    int guardfaction;
    int monfaction;
    int doExtraInit;

    //
    // Parsing functions
    //
    void ParseError( OrdersCheck *pCheck, Unit *pUnit, Faction *pFac,
                     const AString &strError );
    UnitId *ParseUnit(AString * s);
    int ParseDir(AString * token);


    void ParseOrders(int faction, Aorders *ordersFile, OrdersCheck *pCheck );
    void ProcessOrder( int orderNum, Unit *unit, AString *order,
                       OrdersCheck *pCheck );
    void ProcessMoveOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAdvanceOrder(Unit *,AString *, OrdersCheck *pCheck );
    Unit *ProcessFormOrder( Unit *former, AString *order,
                            OrdersCheck *pCheck );
    void ProcessAddressOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAvoidOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessGuardOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessNameOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessDescribeOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessBehindOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessGiveOrder(Unit *,AString *, OrdersCheck *pCheck );
	void ProcessWithdrawOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessDeclareOrder(Faction *,AString *, OrdersCheck *pCheck );
    void ProcessStudyOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessTeachOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessWorkOrder(Unit *, OrdersCheck *pCheck );
    void ProcessProduceOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessBuyOrder( Unit *, AString *, OrdersCheck *pCheck );
    void ProcessSellOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAttackOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessBuildOrder( Unit *, AString *, OrdersCheck *pCheck );
    void ProcessSailOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessEnterOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessLeaveOrder(Unit *, OrdersCheck *pCheck );
    void ProcessPromoteOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessEvictOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessTaxOrder(Unit *, OrdersCheck *pCheck );
    void ProcessPillageOrder(Unit *, OrdersCheck *pCheck );
    void ProcessConsumeOrder(Unit *, AString *, OrdersCheck *pCheck );
    void ProcessRevealOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessFindOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessDestroyOrder(Unit *, OrdersCheck *pCheck );
    void ProcessQuitOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessRestartOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAssassinateOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessStealOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessFactionOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessClaimOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessCombatOrder(Unit *,AString *, OrdersCheck *pCheck );
	void ProcessPrepareOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessWeaponOrder(Unit *u, AString *o, OrdersCheck *pCheck);
	void ProcessArmorOrder(Unit *u, AString *o, OrdersCheck *pCheck);
    void ProcessCastOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessEntertainOrder(Unit *, OrdersCheck *pCheck );
    void ProcessForgetOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessReshowOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessHoldOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessNoaidOrder(Unit *,AString *, OrdersCheck *pCheck );
	void ProcessNocrossOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessNospoilsOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessSpoilsOrder(Unit *, AString *, OrdersCheck *pCheck);
    void ProcessAutoTaxOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessOptionOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessPasswordOrder(Unit *, AString *, OrdersCheck *pCheck );
	void ProcessExchangeOrder(Unit *, AString *, OrdersCheck *pCheck);
	AString *ProcessTurnOrder(Unit *, Aorders *, OrdersCheck *pCheck, int);

	void RemoveInactiveFactions();

    //
    // Game running functions
    //

    //
    // This can be called by parse functions
    //
    int CountMages(Faction *);
	int CountApprentices(Faction *);
    void FindDeadFactions();
    void DeleteEmptyUnits();
    void DeleteEmptyInRegion(ARegion *);
    void EmptyHell();
    void DoGuard1Orders();
    void DoGiveOrders();
	void DoWithdrawOrders();

	void DoExchangeOrders();
	void DoExchangeOrder(ARegion *,Unit *,ExchangeOrder *);

    //
    // Faction limit functions.
    //
    // The first 3 are game specific and can be found in extra.cpp. They
    // may return -1 to indicate no limit.
    //
    int AllowedMages( Faction *pFac );
	int AllowedApprentices(Faction *pFact);
    int AllowedTaxes( Faction *pFac );
    int AllowedTrades( Faction *pFac );
    int TaxCheck( ARegion *pReg, Faction *pFac );
    int TradeCheck( ARegion *pReg, Faction *pFac );

    //
    // The DoGiveOrder returns 0 normally, or 1 if no more GIVE orders
    // should be allowed
    //
    int DoGiveOrder(ARegion *,Unit *,GiveOrder *);
    //
    // The DoWithdrawOrder returns 0 normally, or 1 if no more WITHDRAW
	// orders should be allowed
    //
    int DoWithdrawOrder(ARegion *,Unit *,WithdrawOrder *);

    //
    // These are game specific, and can be found in extra.cpp
    //
    void CheckUnitMaintenance( int consume );
    void CheckFactionMaintenance( int consume );
    void CheckAllyMaintenance();

	// Similar to the above, but for minimum food requirements
	void CheckUnitHunger();
	void CheckFactionHunger();
	void CheckAllyHunger();

    void CheckUnitMaintenanceItem(int item, int value, int consume );
    void CheckFactionMaintenanceItem(int item, int value, int consume );
    void CheckAllyMaintenanceItem(int item, int value);

	// Hunger again
	void CheckUnitHungerItem(int item, int value);
	void CheckFactionHungerItem(int item, int value);
	void CheckAllyHungerItem(int item, int value);

    void AssessMaintenance();

    void GrowWMons(int);
    void GrowLMons(int);
	void GrowVMons();
    void PostProcessUnit(ARegion *,Unit *);
	void MidProcessUnit(ARegion *, Unit *);

    //
    // Mid and PostProcessUnitExtra can be used to provide game-specific
	// unit processing at the approrpriate times.
    //
	void MidProcessUnitExtra(ARegion *, Unit *);
	void MidProcessTurn();
    void PostProcessUnitExtra(ARegion *,Unit *);
    void PostProcessTurn();

	// Handle escaped monster check
	void MonsterCheck(ARegion *r, Unit *u);

    //
    // CheckVictory is used to see if the game is over.
    //
    Faction *CheckVictory();

    void EndGame( Faction *pVictor );

    void RunBuyOrders();
    void DoBuy(ARegion *,Market *);
    int GetBuyAmount(ARegion *,Market *);
    void RunSellOrders();
    void DoSell(ARegion *,Market *);
    int GetSellAmount(ARegion *,Market *);
    void DoAttackOrders();
    void CheckWMonAttack(ARegion *,Unit *);
    Unit *GetWMonTar(ARegion *,int,Unit *);
    int CountWMonTars(ARegion *,Unit *);
    void AttemptAttack(ARegion *,Unit *,Unit *,int,int=0);
    void DoAutoAttacks();
    void DoAutoAttacksRegion(ARegion *);
    void DoAdvanceAttack(ARegion *,Unit *);
    void DoAutoAttack(ARegion *,Unit *);
    void DoAdvanceAttacks(AList *);
    void DoAutoAttackOn(ARegion *,Unit *);
    void RemoveEmptyObjects();
    void RunEnterOrders();
    void Do1EnterOrder(ARegion *,Object *,Unit *);
    void RunPromoteOrders();
    void Do1PromoteOrder(Object *,Unit *);
    void Do1EvictOrder(Object *,Unit *);
    void RunPillageOrders();
    int CountPillagers(ARegion *);
    void ClearPillagers(ARegion *);
    void RunPillageRegion(ARegion *);
    void RunTaxOrders();
    void RunTaxRegion(ARegion *);
    int CountTaxers(ARegion *);
    void RunFindOrders();
    void RunFindUnit(Unit *);
    void RunDestroyOrders();
    void Do1Destroy(ARegion *,Object *,Unit *);
    void RunQuitOrders();
    void RunForgetOrders();
    void Do1Quit(Faction *);
    void SinkUncrewedShips();
	void DrownUnits();
    void RunStealOrders();
    AList * CanSeeSteal(ARegion *,Unit *);
    void Do1Steal(ARegion *,Object *,Unit *);
    void Do1Assassinate(ARegion *,Object *,Unit *);
    void AdjustCityMons( ARegion *pReg );
    void AdjustCityMon( ARegion *pReg, Unit *u );

    //
    // Month long orders
    //
    void RunMoveOrders();
    Location * DoAMoveOrder(Unit *,ARegion *,Object *);
    void DoMoveEnter(Unit *,ARegion *,Object **);
    void RunMonthOrders();
    void RunStudyOrders(ARegion *);
    void Do1StudyOrder(Unit *,Object *);
    void RunTeachOrders();
    void Do1TeachOrder(ARegion *,Unit *);
    void RunProduceOrders(ARegion *);
    int ValidProd(Unit *,ARegion *,Production *);
    int FindAttemptedProd(ARegion *,Production *);
    void RunAProduction(ARegion *,Production *);
    void RunUnitProduce(ARegion *,Unit *);
    void Run1BuildOrder(ARegion *,Object *,Unit *);
	void RunBuildHelpers(ARegion *);
    void RunSailOrders();
    ARegion * Do1SailOrder(ARegion *,Object *,Unit *);
    void ClearCastEffects();
    void RunCastOrders();
    void RunACastOrder(ARegion *,Object *,Unit *);
    void RunTeleportOrders();

    //
    // include spells.h for spell function declarations
    //
#define GAME_SPELLS
#include "spells.h"
#undef GAME_SPELLS

    //
    // Battle function
    //
    void KillDead(Location *);
    int RunBattle(ARegion *,Unit *,Unit *,int = 0,int = 0);
    void GetSides(ARegion *,AList &,AList &,AList &,AList &,Unit *,Unit *,
                  int = 0,int = 0);
    int CanAttack(ARegion *,AList *,Unit *);
    void GetAFacs(ARegion *,Unit *,Unit *,AList &,AList &,AList &);
    void GetDFacs(ARegion *,Unit *,AList &);
};

#endif
