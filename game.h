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
#include "faction.h"
#include "production.h"
#include "object.h"
#include "events.h"
#include "rng.h"
#include "indenter.hpp"
#include "string_parser.hpp"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

#include <map>
#include <string>

#define CURRENT_ATL_VER MAKE_ATL_VER(5, 2, 5)
#define JSON_REPORT_VERSION MAKE_ATL_VER(1, 0, 1) // version 1.0.0 didn't report the version number

class orders_check
{
public:
	orders_check(std::ostream& f) : check_file(f), numshows(0), numerrors(0) { check_file << indent::clear; }

	std::ostream& check_file;
	Unit dummyUnit;
	Faction dummyFaction;
	Order dummyOrder;
	int numshows;
	int numerrors;

	void error(const std::string& err);
};

/// The main game class
/** Currently this doc is here to switch on the class so that
I can see what the Item.cpp docs look like.
*/
class Game
{
	friend class Faction;
	// Rather than making all of the functions & variables public, we can allow this special class to access them.
	// This class won't be defined except in the unit test code, though it could be used in other places if there
	// were a good reason to do so.
	friend class UnitTestHelper;

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
	bool ReadPlayers();
	bool ReadPlayersLine(parser::token& token, parser::string_parser& parser, Faction *fac, bool new_player);

	int ViewMap(const AString &, const AString &);
	// LLS
	void UnitFactionMap();
	int GenRules(const AString &, const AString &, const AString &);
	std::string FactionTypeDescription(Faction &fac);
	int Doorders_check(const AString &strOrders, const AString &strCheck);

	Faction *AddFaction(int noleader = 0, ARegion *pStart = nullptr);

	//
	// Give this particular game a chance to set up the faction. This is in
	// extra.cpp.
	//
	int SetupFaction(Faction *fac);

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
	Unit *parse_gm_unit(std::string tag, Faction *fac);

	int TurnNumber();

	// JLT
	// Functions to allow enabling/disabling parts of the data tables
	void ModifyTablesPerRuleset(void);

	void RecordFact(FactBase *fact);
	void WriteWorldEvents();

private:
	//
	// Game editing functions.
	//
	ARegion *EditGameFindRegion();
	void EditGameFindUnit();
	void EditGameCreateUnit();
	void EditGameRegion(ARegion *pReg);
	void EditGameRegionObjects(ARegion *pReg);
	void EditGameRegionTerrain(ARegion *pReg);
	void EditGameRegionMarkets(ARegion *pReg);
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

	void WriteReport();

	void DeleteDeadFactions();

	//
	// Standard creation functions.
	//
	void CreateCityMons();
	void CreateWMons();
	void CreateLMons();
	void CreateVMons();
	Unit *MakeManUnit(Faction *, int, int, int, int, int, int);

	//
	// Game-specific creation functions (see world.cpp).
	//
	void CreateWorld();
	void CreateNPCFactions();
	void CreateCityMon(ARegion *pReg, int percent, int needmage);
	int MakeWMon(ARegion *pReg);
	void MakeLMon(Object *pObj);

	void WriteSurfaceMap(std::ostream& f, ARegionArray *pArr, int type);
	void WriteUnderworldMap(std::ostream& f, ARegionArray *pArr, int type);
	char GetRChar(ARegion *r);
	AString GetXtraMap(ARegion *, int);

	// LLS
	// Functions to do upgrades to the ruleset -- should be in extras.cpp
	int UpgradeMajorVersion(int savedVersion);
	int UpgradeMinorVersion(int savedVersion);
	int UpgradePatchLevel(int savedVersion);

	// JLT
	// Functions to allow enabling/disabling parts of the data tables
	void EnableSkill(int sk);  // Enabled a disabled skill
	void DisableSkill(int sk); // Prevents skill being studied or used
	void ModifySkillDependancy(int sk, int i, char const *dep, int lev);
	void ModifySkillFlags(int sk, int flags);
	void ModifySkillCost(int sk, int cost);
	void ModifySkillSpecial(int sk, char const *special);
	void ModifySkillRange(int sk, char const *range);

	void EnableItem(int it);  // Enables a disabled item
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
	void ModifyItemProductionSkill(int it, char const *sk, int lev);
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

	void EnableObject(int ob);	// Enables a disabled object
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

	std::list<Faction *> factions;
	std::vector<Battle *> battles;
	ARegionList regions;
	int factionseq;
	unsigned int unitseq;
	Unit **ppUnits;
	unsigned int maxppunits;
	int shipseq;
	int year;
	int month;


	// These hooks are meant to allow the game engine for a custom game to override some basic functions.
	// For our existing case, these are going to be used by the unit test framework to ensure a consistent
	// run (by overriding the seeding of the random number generator).  In general, there should be *very*
	// few of these hooks and they should exist for a very explicit purpose and be used with extreme care.

	// control the random number seed used for new game generation (by default it uses the existing
	// seedrandomrandom function) which uses the current time.
	std::function<void()> init_random_seed = static_cast<void(*)()>(&rng::seed_random);

	enum
	{
		GAME_STATUS_UNINIT,
		GAME_STATUS_NEW,
		GAME_STATUS_RUNNING,
		GAME_STATUS_FINISHED,
	};
	int gameStatus;

	int guardfaction;
	int monfaction;
	int doExtraInit;

	Events *events;

	// We need some way to track game specific data that can be used globally
	// (specifically added for testing of NO7 victory conditions, but generally
	// useful).  I use json here so that it can store arbitrary data in a structured way.
	json rulesetSpecificData;

	//
	// Parsing functions
	//
	void parse_error(orders_check *checker, Unit *unit, Faction *faction, const std::string &error);
	void overwrite_month_warning(std::string type, Unit *u, orders_check *checker);
	UnitId *parse_unit(parser::string_parser& parser);
	int parse_dir(const parser::token& token);

	void ParseOrders(int faction, std::istream& ordersFile, orders_check *checker);
	void ProcessOrder(int order, Unit *unit, parser::string_parser& parser, orders_check *checker, bool repeating);
	void ProcessMoveOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessAdvanceOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	Unit *ProcessFormOrder(Unit *former, parser::string_parser& order, orders_check *checker, bool repeating);
	void ProcessAddressOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessAvoidOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessGuardOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessNameOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessDescribeOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessBehindOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessGiveOrder(int, Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessWithdrawOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessDeclareOrder(Faction *f, parser::string_parser& parser, orders_check *checker);
	void ProcessStudyOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessTeachOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessWorkOrder(Unit *u, int quiet, orders_check *checker);
	void ProcessProduceOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessBuyOrder(Unit *u, parser::string_parser& parser, orders_check *checker, bool repeating);
	void ProcessSellOrder(Unit *u, parser::string_parser& parser, orders_check *checker, bool repeating);
	void ProcessAttackOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessBuildOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	// Helper functions for ProcessBuildOrder
	BuildOrder *ProcessBuildHelp(Unit *u, parser::string_parser& parser, orders_check *checker);
	BuildOrder *ProcessBuildObject(Unit *u, int object_type, orders_check *checker);
	BuildOrder *ProcessBuildShip(Unit *u, int object_type, orders_check *checker);
	BuildOrder *ProcessBuildStructure(Unit *u, int object_type, orders_check *checker);
	BuildOrder *ProcessContinuedBuild(Unit *u, orders_check *checker);
	//bool ProcessBuildComplete(Unit *u, BuildOrder *order, parser::token token, orders_check *checker);
	//void CheckBuildOrderConflicts(Unit *u, orders_check *checker);
	void ProcessSailOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessEnterOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessLeaveOrder(Unit *u, orders_check *checker);
	void ProcessPromoteOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessEvictOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessTaxOrder(Unit *u, orders_check *checker);
	void ProcessPillageOrder(Unit *, orders_check *checker);
	void ProcessConsumeOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessRevealOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessFindOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessDestroyOrder(Unit *u, orders_check *checker);
	void ProcessQuitOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessRestartOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessAssassinateOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessStealOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessFactionOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessClaimOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessCombatOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessPrepareOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessWeaponOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessArmorOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessCastOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessEntertainOrder(Unit *u, orders_check *checker);
	void ProcessForgetOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessReshowOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessHoldOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessNoaidOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessNocrossOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessNospoilsOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessSpoilsOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessAutoTaxOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessOptionOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessPasswordOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessExchangeOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessIdleOrder(Unit *u, orders_check *checker);
	void ProcessTransportOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessShareOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	std::string ProcessTurnOrder(Unit *u, std::istream& f, orders_check *checker, bool repeating);
	void ProcessJoinOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessAnnihilateOrder(Unit *u, parser::string_parser& parser, orders_check *checker);
	void ProcessSacrificeOrder(Unit *u, parser::string_parser& parser, orders_check *checker);

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
	int AllowedMartial(Faction *pFac);
	bool ActivityCheck(ARegion *pReg, Faction *pFac, FactionActivity activity);

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
	void AttemptAttack(ARegion *, Unit *, Unit *, int, int = 0);
	void DoAutoAttacks();
	void DoAdvanceAttack(ARegion *, Unit *);
	void DoAutoAttack(ARegion *, Unit *);
	void DoMovementAttacks(std::list<Location *>& locs);
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
	void RunStealthOrders();
	void RunTransportOrders();
	void RunTransportPhase(TransportOrder::TransportPhase phase);
	void RunAnnihilateOrders();
	void RunSacrificeOrders();
	void CollectInterQMTransportItems();
	void CheckTransportOrders();
	std::list<Faction *>CanSeeSteal(ARegion *r, Unit *u);
	void Do1Steal(ARegion *, Object *, Unit *);
	void Do1Assassinate(ARegion *, Object *, Unit *);
	void Do1Annihilate(ARegion *reg);
	void AdjustCityMons(ARegion *pReg);
	void AdjustCityMon(ARegion *pReg, Unit *u);

	//
	// Month long orders
	//
	void RunMoveOrders();
	Location *DoAMoveOrder(Unit *, ARegion *, Object *);
	void DoMoveEnter(Unit *unit, ARegion *region);
	void RunMonthOrders();
	void RunStudyOrders(ARegion *);
	void Do1StudyOrder(Unit *, Object *);
	void RunTeachOrders();
	void Do1TeachOrder(ARegion *, Unit *);
	void RunProduceOrders(ARegion *);
	void RunIdleOrders(ARegion *);
	int consume_production_inputs(Unit *u, int item, int maxproduced);
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
	int KillDead(Location *, Battle *, int, int);
	int RunBattle(ARegion *, Unit *, Unit *, int = 0, int = 0);
	void GetSides(
		ARegion *r, std::set<Faction *>& afacs, std::set<Faction *>& dfacs, std::list<Location *>& atts,
		std::list<Location *>& defs, Unit *att, Unit *tar, int ass = 0, int adv = 0
	);
	int CanAttack(ARegion *r, std::set<Faction *>& afacs, Unit * u);
	void GetAFacs(
		ARegion *f, Unit *att, Unit *tar, std::set<Faction *>& dfacs,
		std::set<Faction *>& afacs, std::list<Location *> &atts
	);
	void GetDFacs(ARegion *r, Unit *t, std::set<Faction*>& facs);

	// For faction statistics
	void CountItems(size_t **citems);
	int CountItem(Faction *, int);
};

#endif
