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
// Date		Person			Comments
// ----		------			--------
// 2000/MAR/16 Davis Kulis	   Added a new reporting Template.
// 2000/MAR/21 Azthar Septragen  Added roads.
#ifndef REGION_CLASS
#define REGION_CLASS

class ARegion;
class ARegionList;

#include "gamedefs.h"
#include "gameio.h"
#include "faction.h"
#include "alist.h"
#include "unit.h"
#include "fileio.h"
#include "production.h"
#include "market.h"
#include "object.h"
#include <map>

/* Weather Types */
enum {
	W_NORMAL,
	W_WINTER,
	W_MONSOON,
	W_BLIZZARD
};

enum {
    FL_NULL,
    FL_UNUSED_START_LOC,
    FL_USED_START_LOC
};

struct Product
{
	int product;
	int chance;
	int amount;
};

class TerrainType
{
	public:
		char *name;
		char *type;
		char marker;
		int similar_type;

		enum {
			RIDINGMOUNTS = 0x01,
			FLYINGMOUNTS = 0x02,
			RIDINGLIMITED = 0x04,
			FLYINGLIMITED = 0x08,
			RESTRICTEDFOOT = 0x10,
			RESTRICTEDRANGED = 0x20,
			ENHANCEDRANGED = 0x40, //mutually exclusive with RESTRICTEDRANGED (if both are used, ENHANCEDRANGE will have no effect
			DISABLED = 0x80,   //currently used only in genrules.
		};
		int flags;

		int pop;
		int wages;
		int economy;
		int movepoints;
		Product prods[7];
		// Race information
		// A hex near water will have either one of the normal races or one
		// of the coastal races in it.   Non-coastal hexes will only have one
		// of the normal races.
		int races[4];
		int coastal_races[3];
		int wmonfreq;
		int smallmon;
		int bigmon;
		int humanoid;
		int lairChance;
		int lairs[6];
};

extern TerrainType *TerrainDefs;

AString *TerrainDescription(int type);

class Location : public AListElem
{
	public:
		Unit *unit;
		Object *obj;
		ARegion *region;
};

Location *GetUnit(AList *, int);

int AGetName(int town);
char *AGetNameString(int name);

class ARegionPtr : public AListElem
{
	public:
		ARegion *ptr;
};

ARegionPtr *GetRegion(AList *, int);

class Farsight : public AListElem
{
	public:
		Farsight();

		Faction *faction;
		Unit *unit;
		int level;
		int exits_used[NDIRS];
};

Farsight *GetFarsight(AList *, Faction *);

enum {
	TOWN_VILLAGE,
	TOWN_TOWN,
	TOWN_CITY,
	NTOWNS
};

AString TownString(int i);

class TownInfo
{
	public:
		TownInfo();
		~TownInfo();

		void Readin(Ainfile *, ATL_VER &);
		void Writeout(Aoutfile *);
		int TownType();

		AString *name;
		int pop;
		int basepop;
		int activity;
		
		// Player-Econ stuff
		int growth;
		int mortality;
};

class ARegion : public AListElem
{
	friend class Game;
	friend class ARegionArray;

	public:
		ARegion();
		ARegion(int, int);
		~ARegion();
		void Setup();

		void ZeroNeighbors();
		void SetName(char *);

		void Writeout(Aoutfile *);
		void Readin(Ainfile *, AList *, ATL_VER v);

		int CanMakeAdv(Faction *, int);
		int HasItem(Faction *, int);
		void WriteProducts(Areport *, Faction *, int);
		void WriteMarkets(Areport *, Faction *, int);
		void WriteEconomy(Areport *, Faction *, int);
		void WriteExits(Areport *, ARegionList *pRegs, int *exits_seen);
		void WriteReport(Areport *f, Faction *fac, int month,
				ARegionList *pRegions);
		// DK
		void WriteTemplate(Areport *, Faction *, ARegionList *, int);
		void WriteTemplateHeader(Areport *, Faction *, ARegionList *, int);
		void GetMapLine(char *, int, ARegionList *);

		AString ShortPrint(ARegionList *pRegs);
		AString Print(ARegionList *pRegs);

		void Kill(Unit *);
		void ClearHell();

		Unit *GetUnit(int);
		Unit *GetUnitAlias(int, int); /* alias, faction number */
		Unit *GetUnitId(UnitId *, int);
		Location *GetLocation(UnitId *, int);

		void SetLoc(int, int, int);
		int Present(Faction *);
		AList *PresentFactions();
		int GetObservation(Faction *, int);
		int GetTrueSight(Faction *, int);

		Object *GetObject(int);
		Object *GetDummy();

		int MoveCost(int, ARegion *, int, AString *road);
		Unit *Forbidden(Unit *); /* Returns unit that is forbidding */
		Unit *ForbiddenByAlly(Unit *); /* Returns unit that is forbidding */
		int CanTax(Unit *);
		int CanPillage(Unit *);
		int ForbiddenShip(Object *);
		int HasCityGuard();
		int NotifySpell(Unit *, char *, ARegionList *pRegs);
		void NotifyCity(Unit *, AString& oldname, AString& newname);

		void DefaultOrders(int peasantfac);
		void UpdateTown();
		void PostTurn(ARegionList *pRegs);
		void UpdateProducts();
		void SetWeather(int newWeather);
		int IsCoastal();
		int IsCoastalOrLakeside();
		void MakeStartingCity();
		int IsStartingCity();
		int IsSafeRegion();
		int CanBeStartingCity(ARegionArray *pRA);
		int HasShaft();

		// AS
		int HasRoad();
		int HasExitRoad(int realDirection);
		int CountConnectingRoads();
		int HasConnectingRoad(int realDirection);
		int GetRoadDirection(int realDirection);
		int GetRealDirComp(int realDirection);
		void DoDecayCheck(ARegionList *pRegs);
		void DoDecayClicks(Object *o, ARegionList *pRegs);
		void RunDecayEvent(Object *o, ARegionList *pRegs);
		AString GetDecayFlavor();
		int GetMaxClicks();
		int PillageCheck();

		// JR
		int GetPoleDistance(int dir);
		void SetGateStatus(int month);
		void DisbandInRegion(int, int);
		void Recruit(int);
		int IsNativeRace(int);
		void AdjustPop(int);
		void Migrate();
		void CheckTownIncrease();
		int TraceConnectedRoad(int, int, AList *, int);
		int CountRoadConnectedTowns(int);
		int TownHabitat();
		int Development();
		int TownDevelopment();
		int CheckSea(int, int, int);
		int Slope();
		int SurfaceWater();
		int Soil();
		int Winds();
		int TerrainFactor(int, int);
		int TerrainProbability(int);
		
		int CountWMons();
		int IsGuarded();

		int Wages();
		int LakeEffect();
		AString WagesForReport();
		int Population();

		AString *name;
		int num;
		int type;
		int buildingseq;
		int weather;
		int gate;
		int gatemonth;
		int gateopen;
		int willsink; //number of turns until the region will sink (decreased at end of runorders). This should be set to 0 (will never sink) or a positive number. Negative numbers should be ok, but I make no promises!
		int untaxed;  //amount of silver NOT taxed (used in guard regeneration).
		void SinkRegion(ARegionList *pRegs);
		void OceanToLand();
		void Fake();
		void SetEthnicity(int ethnicity, ARegionList *pRegs);
		int GetEthnicity();
		void RedoAs(int tertype, int town, ARegionList *pRegs);
		void Event(const AString &);
		AList events;

		TownInfo *town;
		int race;
		int population;
		int basepopulation;
		int migration;
		int wages;
		int maxwages;
		int money;
		
		/* Player-run economy */
		int habitat;
		int development;
		int growth;
		int mortality;
		int elevation;
		int humidity;
		int temperature;
		int vegetation;
		int culture;

		/* Potential bonuses to economy */
		int clearskies;
		int earthlore;
		/* Potential malus */
		int fog;

		ARegion *neighbors[NDIRS];
		Hexside *hexside[NDIRS];
		AList objects;
		AList hell; /* Where dead units go */
		AList farsees;
		// List of units which passed through the region
		AList passers;
		ProductionList products;
		MarketList markets;
		int xloc, yloc, zloc;
		
		int dynamicexits;  //are the exits to this region the same for every unit present? Usually ... yes

		void AddEditTown(AString * = 0);
		void UpdateEditRegion();
		void SetupEditRegion(int canmakecity = 1);
		void EditAdjustAreaName(AString name);
//		int GetArcadianTradeAveraged(int numtrade, int producing);
		int GetArcadianTrade(int numtrade, int producing);
		int marker; //temporary variable you can use for eg marking regions visited during a routine. Clear before using.
		int flagpole; //variable, needs to be saved with region. Used in Arcadia for triggering region-specific events.
		int timesmarker; //variable for times events.

	private:
		/* Private Setup Functions */
		void SetupPop();
		void SetupProds();
		int GetNearestProd(int);
		void SetupCityMarket();
		void AddTown();
		void MakeLair(int);
		void LairCheck();
		
		void WagesFromDevelopment();
};

class ARegionArray
{
	public:
		ARegionArray(int, int);
		~ARegionArray();

		void SetRegion(int, int, ARegion *);
		ARegion *GetRegion(int, int);
		void SetName(char *name);

		int x;
		int y;
		ARegion **regions;
		AString *strName;

		enum {
			LEVEL_NEXUS,
			LEVEL_SURFACE,
			LEVEL_UNDERWORLD,
			LEVEL_UNDERDEEP,
			LEVEL_QUEST,
		};
		int levelType;
};

class ARegionFlatArray
{
	public:
		ARegionFlatArray(int);
		~ARegionFlatArray();

		void SetRegion(int, ARegion *);
		ARegion *GetRegion(int);

		int size;
		ARegion **regions;
};

struct Geography
{
	int elevation;
	int humidity;
	int temperature;
	int vegetation;
	int culture;
};

class GeoMap
{
	public:
		GeoMap(int, int);
		void Generate(int spread, int smoothness);
		int GetElevation(int, int);
		int GetHumidity(int, int);
		int GetTemperature(int, int);
		int GetVegetation(int, int);
		int GetCulture(int, int);
		void ApplyGeography(ARegionArray *pArr);
		
		int size, xscale, yscale, xoff, yoff;
		map<long int,Geography> geomap;
		
};

class ARegionList : public AList
{
	public:
		ARegionList();
		~ARegionList();

		ARegion *GetRegion(int);
		ARegion *GetRegion(int, int, int);
		int ReadRegions(Ainfile *f, AList *, ATL_VER v);
		void WriteRegions(Aoutfile *f);
		Location *FindUnit(int);
		Location *GetUnitId(UnitId *id, int faction, ARegion *cur);

		void ChangeStartingCity(ARegion *, int);
		ARegion *GetStartingCity(ARegion *AC, int num, int level, int maxX,
				int maxY);

		ARegion *FindGate(int);
		int GetDistance(ARegion *, ARegion *);
		int GetPlanarDistance(ARegion *, ARegion *, int penalty);
		int GetWeather(ARegion *pReg, int month);

		ARegionArray *GetRegionArray(int level);

		int numberofgates;
		int numLevels;
		ARegionArray **pRegionArrays;

	public:
		//
		// Public world creation stuff
		//
		void CreateLevels(int numLevels);
		void CreateAbyssLevel(int level, char *name);
		void CreateNexusLevel(int level, int xSize, int ySize, char *name);
		void CreateSurfaceLevel(int level, int xSize, int ySize, char *name);
		void CreateIslandLevel(int level, int nPlayers, char *name);
		void CreateUnderworldLevel(int level, int xSize, int ySize, char *name);
		void CreateUnderdeepLevel(int level, int xSize, int ySize, char *name);
		void AddQuestLevel(int xSize, int ySize, char *name, int type);

		void MakeShaftLinks(int levelFrom, int levelTo, int odds);
		void SetACNeighbors(int levelSrc, int levelTo, int maxX, int maxY);
		void InitSetupGates(int level);
		void FinalSetupGates();

		// JR
		void InitGeographicMap(ARegionArray *pRegs);
		void CleanUpWater(ARegionArray *pRegs);
		void RemoveCoastalLakes(ARegionArray *pRegs);
		void SeverLandBridges(ARegionArray *pRegs);
		void AddBeaches(ARegionArray *pRegs);
		void AddRivers(ARegionArray *pRegs);
		int WaterDistance(ARegion *reg);
        void ClearRiverNums(ARegionArray *pRegs);
        int AddRiverSegment(ARegion *reg,int hexside,int rivernum, int tries);	
        void CreateRiverSegment(ARegion *reg, int hexside, int rivernum);
        void DeleteRiverSegment(ARegion *reg, int hexside);
		void RescaleFractalParameters(ARegionArray *pArr);
		void SetFractalTerrain(ARegionArray *pArr);
		void NameRegions(ARegionArray *pArr);
		void UnsetRace(ARegionArray *pRegs);
		void RaceAnchors(ARegionArray *pRegs);
		void GrowRaces(ARegionArray *pRegs);

		void CalcDensities();
		int GetLevelXScale(int level);
		int GetLevelYScale(int level);
		
		void EditNeighSetup(ARegion *r, ARegionArray *ar); //calls the private neighsetup function for the game editor

	private:
		//
		// Private world creation stuff
		//
		void MakeRegions(int level, int xSize, int ySize);
		void SetupNeighbors(ARegionArray *pRegs);
		void AddHexsides(ARegionArray *pRegs);
		void MakeIcosahedralRegions(int level, int xSize, int ySize);
		void SetupIcosahedralNeighbors(ARegionArray *pRegs);
		void NeighSetup(ARegion *r, ARegionArray *ar);

		void SetRegTypes(ARegionArray *pRegs, int newType);
		void MakeLand(ARegionArray *pRegs, int percentOcean, int continentSize);
		void MakeCentralLand(ARegionArray *pRegs);

		void SetupAnchors(ARegionArray *pArr);
		void GrowTerrain(ARegionArray *pArr, int growOcean);
		void RandomTerrain(ARegionArray *pArr);
		void MakeUWMaze(ARegionArray *pArr);
		void MakeIslands(ARegionArray *pArr, int nPlayers);
		void MakeOneIsland(ARegionArray *pRegs, int xx, int yy);

		void AssignTypes(ARegionArray *pArr);
		void FinalSetup(ARegionArray *);

		void MakeShaft(ARegion *reg, ARegionArray *pFrom, ARegionArray *pTo);

		//
		// Game-specific world stuff (see world.cpp)
		//
		int GetRegType(ARegion *pReg);
		int CheckRegionExit(ARegion *pFrom, ARegion *pTo);
		
		void CheckHexsides(ARegionArray *pArr);

};

int LookupRegionType(AString *);
int ParseTerrain(AString *);

#endif
