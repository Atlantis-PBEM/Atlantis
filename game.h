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

#ifndef GAME_CLASS
#define GAME_CLASS

class Game;

#include "aregion.h"
#include "alist.h"
#include "faction.h"
#include "production.h"
#include "object.h"

#define CURRENT_ATL_VER MAKE_ATL_VER(5, 2, 4)

class OrdersCheck
{
public:
	OrdersCheck();

	Aoutfile *pCheckFile;
	Unit dummyUnit;
	Faction dummyFaction;
	Order dummyOrder;
	int numshows;
	int numerrors;

	void Error(const AString &error);
};

/// The main game class
/** Currently this doc is here to switch on the class so that
I can see what the modify.cpp docs look like.
*/
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
	int EditGame(int *pSaveGame);
	int SaveGame();
	int WritePlayers();
	int ReadPlayers();
	int ReadPlayersLine(AString *pToken, AString *pLine, Faction *pFac,
						 int newPlayer);
	void WriteNewFac(Faction *pFac);

	int ViewMap(const AString &, const AString &);
	// LLS
	void UnitFactionMap();
	int GenRules(const AString &, const AString &, const AString &);
	int DoOrdersCheck(const AString &strOrders, const AString &strCheck);

	Faction *AddFaction(int noleader=0, ARegion *pStart = NULL);

	//
	// Give this particular game a chance to set up the faction. This is in
	// extra.cpp.
	//
	int SetupFaction(Faction *pFac);

	void ViewFactions();

	//
	// Get a new unit, with its number assigned.
	//
	Unit *GetNewUnit(Faction *fac, int an = 0);

	//
	// Setup the array of units.
	//
	void SetupUnitSeq();
	void SetupUnitNums();

	//
	// Get a unit by its number.
	//
	Unit *GetUnit(int num);

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
	void EditGameCreateUnit();
	void EditGameRegion(ARegion *pReg);
	void EditGameRegionObjects(ARegion *pReg);
	void EditGameRegionTerrain(ARegion *pReg );
	void EditGameRegionMarkets(ARegion *pReg );
	void EditGameUnit(Unit *pUnit);
	void EditGameUnitItems(Unit *pUnit);
	void EditGameUnitSkills(Unit *pUnit);
	void EditGameUnitMove(Unit *pUnit);
	void EditGameUnitDetails(Unit *pUnit);
	
	void PreProcessTurn();
	void ReadOrders();
	void RunOrders();
	void ClearOrders(Faction *);
	void MakeFactionReportLists();
	void CountAllSpecialists();
	//void CountAllMages();
	//void CountAllApprentices();
	//void CountAllQuarterMasters();
	//void CountAllTacticians();
	void WriteReport();
	// LLS - write order templates
	void WriteTemplates();

	void DeleteDeadFactions();

	//
	// Standard creation functions.
	//
	void CreateCityMons();
	void CreateWMons();
	void CreateLMons();
	void CreateVMons();
	Unit *MakeManUnit(Faction*, int, int, int, int, int, int);

	//
	// Game-specific creation functions (see world.cpp).
	//
	void CreateWorld();
	void CreateNPCFactions();
	void CreateCityMon(ARegion *pReg, int percent, int needmage);
	int MakeWMon(ARegion *pReg);
	void MakeLMon(Object *pObj);

	void WriteSurfaceMap(Aoutfile *f, ARegionArray *pArr, int type);
	void WriteUnderworldMap(Aoutfile *f, ARegionArray *pArr, int type);
	char GetRChar(ARegion *r);
	AString GetXtraMap(ARegion *, int);

	// LLS
	// Functions to do upgrades to the ruleset -- should be in extras.cpp
	int UpgradeMajorVersion(int savedVersion);
	int UpgradeMinorVersion(int savedVersion);
	int UpgradePatchLevel(int savedVersion);

	// JLT
	// Functions to allow enabling/disabling parts of the data tables
	void EnableSkill(int sk); // Enabled a disabled skill
	void DisableSkill(int sk);  // Prevents skill being studied or used
	void ModifySkillDependancy(int sk, int i, char const *dep, int lev);
	void ModifySkillFlags(int sk, int flags);
	void ModifySkillCost(int sk, int cost);
	void ModifySkillSpecial(int sk, char const *special);
	void ModifySkillRange(int sk, char const *range);

	void EnableItem(int it); // Enables a disabled item
	void DisableItem(int it); // Prevents item being generated/produced
	void ModifyItemName(int it, char const *name, char const *names);
	void ModifyItemFlags(int it, int flags);
	void ModifyItemType(int it, int type);
	void ModifyItemWeight(int it, int weight);
	void ModifyItemBasePrice(int it, int price);
	void ModifyItemCapacities(int it, int walk, int ride, int fly, int swim);
	void ModifyItemSpeed(int it, int speed);
	void ModifyItemProductionBooster(int it, int item, int bonus);
	void ModifyItemHitch(int it, int item, int bonus);
	void ModifyItemProductionSkill(int it, char *sk, int lev);
	void ModifyItemProductionOutput(int it, int months, int count);
	void ModifyItemProductionInput(int it, int i, int input, int amount);
	void ModifyItemMagicSkill(int it, char *sk, int lev);
	void ModifyItemMagicOutput(int it, int count);
	void ModifyItemMagicInput(int it, int i, int input, int amount);
	void ModifyItemEscape(int it, int escape, char const *skill, int val);

	void ModifyRaceSkillLevels(char const *race, int special, int def);
	void ModifyRaceSkills(char const *race, int i, char const *sk);

	void ModifyMonsterAttackLevel(char const *mon, int lev);
	void ModifyMonsterDefense(char const *mon, int defenseType, int level);
	void ModifyMonsterAttacksAndHits(char const *mon, int num, int hits, int regen, int hitDamage);
	void ModifyMonsterSkills(char const *mon, int tact, int stealth, int obs);
	void ModifyMonsterSpecial(char const *mon, char const *special, int lev);
	void ModifyMonsterSpoils(char const *mon, int silver, int spoilType);
	void ModifyMonsterThreat(char const *mon, int num, int hostileChance);

	void ModifyWeaponSkills(char const *weap, char *baseSkill, char *orSkill);
	void ModifyWeaponFlags(char const *weap, int flags);
	void ModifyWeaponAttack(char const *weap, int wclass, int attackType, int numAtt, int hitDamage);
	void ModifyWeaponBonuses(char const *weap, int attack, int defense, int vsMount);
	void ModifyWeaponBonusMalus(char const *weap, int index, char *weaponAbbr, int attackModifer, int defenseModifer);

	void ModifyArmorFlags(char const *armor, int flags);
	void ModifyArmorSaveFrom(char const *armor, int from);
	void ModifyArmorSaveValue(char const *armor, int wclass, int val);

	void ModifyMountSkill(char const *mount, char *skill);
	void ModifyMountBonuses(char const *mount, int min, int max, int hampered);
	void ModifyMountSpecial(char const *mount, char const *special, int level);

	void EnableObject(int ob); // Enables a disabled object
	void DisableObject(int ob); // Prevents object being built
	void ModifyObjectFlags(int ob, int flags);
	void ModifyObjectDecay(int ob, int maxMaint, int maxMonthDecay, int mFact);
	void ModifyObjectProduction(int ob, int it);
	void ModifyObjectMonster(int ob, int monster);
	void ModifyObjectConstruction(int ob, int it, int num, char const *sk, int lev);
	void ModifyObjectManpower(int ob, int prot, int cap, int sail, int mages);
	void ModifyObjectDefence(int ob, int co, int en, int sp, int we, int ri, int ra);
	void ModifyObjectName(int ob, char const *name);

	void ClearTerrainRaces(int t);
	void ModifyTerrainRace(int t, int i, int r);
	void ModifyTerrainCoastRace(int t, int i, int r);
	void ClearTerrainItems(int t);
	void ModifyTerrainItems(int t, int i, int p, int c, int a);
	void ModifyTerrainWMons(int t, int freq, int smon, int bigmon, int hum);
	void ModifyTerrainLairChance(int t, int chance);
	void ModifyTerrainLair(int t, int i, int lair);
	void ModifyTerrainEconomy(int t, int pop, int wages, int econ, int move);
	void ModifyTerrainFlags(int t, int flags);

	void ModifyBattleItemFlags(char const *item, int flags);
	void ModifyBattleItemSpecial(char const *item, char const *special, int level);

	void ModifySpecialTargetFlags(char const *special, int targetflags);
	void ModifySpecialTargetObjects(char const *special, int index, int obj);
	void ModifySpecialTargetItems(char const *special, int index, int item);
	void ModifySpecialTargetEffects(char const *special, int index, char const *effect);
	void ModifySpecialEffectFlags(char const *special, int effectflags);
	void ModifySpecialShields(char const *special, int index, int type);
	void ModifySpecialDefenseMods(char const *special, int index, int type, int val);
	void ModifySpecialDamage(char const *special, int index, int type, int min,
			int val, int flags, int cls, char const *effect, int hitDamage);

	void ModifyEffectFlags(char const *effect, int flags);
	void ModifyEffectAttackMod(char const *effect, int val);
	void ModifyEffectDefenseMod(char const *effect, int index, int type, int val);
	void ModifyEffectCancelEffect(char const *effect, char *uneffect);

	void ModifyRangeFlags(char const *range, int flags);
	void ModifyRangeClass(char const *range, int rclass);
	void ModifyRangeMultiplier(char const *range, int mult);
	void ModifyRangeLevelPenalty(char const *range, int pen);

	void ModifyAttribMod(char const *mod, int index, int flags, char const *ident,
			int type, int val);
	void ModifyHealing(int level, int patients, int success);

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
	void ParseError(OrdersCheck *pCheck, Unit *pUnit, Faction *pFac,
					 const AString &strError);
	UnitId *ParseUnit(AString *s);
	int ParseDir(AString *token);


	void ParseOrders(int faction, Aorders *ordersFile, OrdersCheck *pCheck);
	void ProcessOrder(int orderNum, Unit *unit, AString *order,
					   OrdersCheck *pCheck);
	void ProcessMoveOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessAdvanceOrder(Unit *, AString *, OrdersCheck *pCheck);
	Unit *ProcessFormOrder(Unit *former, AString *order,
					   OrdersCheck *pCheck, int atsign);
	void ProcessAddressOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessAvoidOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessGuardOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessNameOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessDescribeOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessBehindOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessGiveOrder(int, Unit *, AString *, OrdersCheck *pCheck);
	void ProcessWithdrawOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessDeclareOrder(Faction *, AString *, OrdersCheck *pCheck);
	void ProcessStudyOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessTeachOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessWorkOrder(Unit *, int quiet, OrdersCheck *pCheck);
	void ProcessProduceOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessBuyOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessSellOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessAttackOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessBuildOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessSailOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessEnterOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessLeaveOrder(Unit *, OrdersCheck *pCheck);
	void ProcessPromoteOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessEvictOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessTaxOrder(Unit *, OrdersCheck *pCheck);
	void ProcessPillageOrder(Unit *, OrdersCheck *pCheck);
	void ProcessConsumeOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessRevealOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessFindOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessDestroyOrder(Unit *, OrdersCheck *pCheck);
	void ProcessQuitOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessRestartOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessAssassinateOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessStealOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessFactionOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessClaimOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessCombatOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessPrepareOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessWeaponOrder(Unit *u, AString *o, OrdersCheck *pCheck);
	void ProcessArmorOrder(Unit *u, AString *o, OrdersCheck *pCheck);
	void ProcessCastOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessEntertainOrder(Unit *, OrdersCheck *pCheck);
	void ProcessForgetOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessReshowOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessHoldOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessNoaidOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessNocrossOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessNospoilsOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessSpoilsOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessAutoTaxOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessOptionOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessPasswordOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessExchangeOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessIdleOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessTransportOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessDistributeOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessShareOrder(Unit *, AString *, OrdersCheck *pCheck);
	AString *ProcessTurnOrder(Unit *, Aorders *, OrdersCheck *pCheck, int);
	void ProcessJoinOrder(Unit *, AString *, OrdersCheck *pCheck);

	void RemoveInactiveFactions();

	//
	// Game running functions
	//

	//
	// This can be called by parse functions
	//
	int CountMages(Faction *);
	int CountApprentices(Faction *);
	int CountQuarterMasters(Faction *);
	int CountTacticians(Faction *);
	
	void FindDeadFactions();
	void DeleteEmptyUnits();
	void DeleteEmptyInRegion(ARegion *);
	void EmptyHell();
	void DoGuard1Orders();
	void DoGiveOrders();
	void DoWithdrawOrders();

	void WriteTimesArticle(AString);

	void DoExchangeOrders();
	void DoExchangeOrder(ARegion *, Unit *, ExchangeOrder *);

	//
	// Faction limit functions.
	//
	// The first 4 are game specific and can be found in extra.cpp. They
	// may return -1 to indicate no limit.
	//
	int AllowedMages(Faction *pFac);
	int AllowedApprentices(Faction *pFact);
	int AllowedQuarterMasters(Faction *pFact);
	int AllowedTacticians(Faction *pFact);
	int AllowedTaxes(Faction *pFac);
	int AllowedTrades(Faction *pFac);
	int TaxCheck(ARegion *pReg, Faction *pFac);
	int TradeCheck(ARegion *pReg, Faction *pFac);

	//
	// The DoGiveOrder returns 0 normally, or 1 if no more GIVE orders
	// should be allowed
	//
	int DoGiveOrder(ARegion *, Unit *, GiveOrder *);
	//
	// The DoWithdrawOrder returns 0 normally, or 1 if no more WITHDRAW
	// orders should be allowed
	//
	int DoWithdrawOrder(ARegion *, Unit *, WithdrawOrder *);

	//
	// These are game specific, and can be found in extra.cpp
	//
	void CheckUnitMaintenance(int consume);
	void CheckFactionMaintenance(int consume);
	void CheckAllyMaintenance();

	// Similar to the above, but for minimum food requirements
	void CheckUnitHunger();
	void CheckFactionHunger();
	void CheckAllyHunger();

	void CheckUnitMaintenanceItem(int item, int value, int consume);
	void CheckFactionMaintenanceItem(int item, int value, int consume);
	void CheckAllyMaintenanceItem(int item, int value);

	// Hunger again
	void CheckUnitHungerItem(int item, int value);
	void CheckFactionHungerItem(int item, int value);
	void CheckAllyHungerItem(int item, int value);

	void AssessMaintenance();

	void GrowWMons(int);
	void GrowLMons(int);
	void GrowVMons();
	void PostProcessUnit(ARegion *, Unit *);
	void MidProcessUnit(ARegion *, Unit *);

	//
	// Mid and PostProcessUnitExtra can be used to provide game-specific
	// unit processing at the approrpriate times.
	//
	void MidProcessUnitExtra(ARegion *, Unit *);
	void MidProcessTurn();
	void PostProcessUnitExtra(ARegion *, Unit *);
	void PostProcessTurn();

	// Processing regions grow after production phase
	void ProcessEconomics();
	
	// Migration effects for alternate player-driven economy
	void ProcessMigration();
	
	// Run setup and equilibration turns (econ-only) at start
	void DevelopTowns();
	void Equilibrate();

	// Handle escaped monster check
	void MonsterCheck(ARegion *r, Unit *u);

	//
	// CheckVictory is used to see if the game is over.
	//
	Faction *CheckVictory();

	void EndGame(Faction *pVictor);

	void RunBuyOrders();
	void DoBuy(ARegion *, Market *);
	int GetBuyAmount(ARegion *, Market *);
	void RunSellOrders();
	void DoSell(ARegion *, Market *);
	int GetSellAmount(ARegion *, Market *);
	void DoAttackOrders();
	void CheckWMonAttack(ARegion *, Unit *);
	Unit *GetWMonTar(ARegion *, int, Unit *);
	int CountWMonTars(ARegion *, Unit *);
	void AttemptAttack(ARegion *, Unit *, Unit *, int, int=0);
	void DoAutoAttacks();
	void DoAdvanceAttack(ARegion *, Unit *);
	void DoAutoAttack(ARegion *, Unit *);
	void DoMovementAttacks(AList *);
	void DoMovementAttack(ARegion *, Unit *);
	void DoAutoAttackOn(ARegion *, Unit *);
	void RemoveEmptyObjects();
	void RunEnterOrders(int);
	void Do1EnterOrder(ARegion *, Object *, Unit *);
	void Do1JoinOrder(ARegion *, Object *, Unit *);
	void RunPromoteOrders();
	void Do1PromoteOrder(Object *, Unit *);
	void Do1EvictOrder(Object *, Unit *);
	void RunPillageOrders();
	int CountPillagers(ARegion *);
	void ClearPillagers(ARegion *);
	void RunPillageRegion(ARegion *);
	void RunTaxOrders();
	void RunTaxRegion(ARegion *);
	int FortTaxBonus(Object *, Unit *);
	int CountTaxes(ARegion *);
	void RunFindOrders();
	void RunFindUnit(Unit *);
	void RunDestroyOrders();
	void Do1Destroy(ARegion *, Object *, Unit *);
	void RunQuitOrders();
	void RunForgetOrders();
	void Do1Quit(Faction *);
	void SinkUncrewedFleets();
	void DrownUnits();
	void RunStealOrders();
	void RunTransportOrders();
	void CheckTransportOrders();
	AList *CanSeeSteal(ARegion *, Unit *);
	void Do1Steal(ARegion *, Object *, Unit *);
	void Do1Assassinate(ARegion *, Object *, Unit *);
	void AdjustCityMons(ARegion *pReg);
	void AdjustCityMon(ARegion *pReg, Unit *u);

	//
	// Month long orders
	//
	void RunMoveOrders();
	Location *DoAMoveOrder(Unit *, ARegion *, Object *);
	void DoMoveEnter(Unit *, ARegion *, Object **);
	void RunMonthOrders();
	void RunStudyOrders(ARegion *);
	void Do1StudyOrder(Unit *, Object *);
	void RunTeachOrders();
	void Do1TeachOrder(ARegion *, Unit *);
	void RunProduceOrders(ARegion *);
	void RunIdleOrders(ARegion *);
	int ValidProd(Unit *, ARegion *, Production *);
	int FindAttemptedProd(ARegion *, Production *);
	void RunAProduction(ARegion *, Production *);
	void RunUnitProduce(ARegion *, Unit *);
	void Run1BuildOrder(ARegion *, Object *, Unit *);
	void RunBuildShipOrder(ARegion *, Object *, Unit *);
	void AddNewBuildings(ARegion *);
	void RunBuildHelpers(ARegion *);
	int ShipConstruction(ARegion *, Unit *, Unit *, int, int, int);
	void CreateShip(ARegion *, Unit *, int);
	void RunSailOrders();
	void RunMovementOrders();
	Location *Do1SailOrder(ARegion *, Object *, Unit *);
	void ClearCastEffects();
	void RunCastOrders();
	void RunACastOrder(ARegion *, Object *, Unit *);
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
	int KillDead(Location *, Battle *);
	int RunBattle(ARegion *, Unit *, Unit *, int = 0, int = 0);
	void GetSides(ARegion *, AList &, AList &, AList &, AList &, Unit *, Unit *,
				  int = 0, int = 0);
	int CanAttack(ARegion *, AList *, Unit *);
	void GetAFacs(ARegion *, Unit *, Unit *, AList &, AList &, AList &);
	void GetDFacs(ARegion *, Unit *, AList &);
};

#endif
