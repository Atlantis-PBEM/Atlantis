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
// Date        Person            Comments
// ----        ------            --------
// 2000/MAR/16 Davis Kulis       Added a new reporting Template.
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

/* Weather Types */
enum {
    W_NORMAL,
    W_WINTER,
    W_MONSOON,
    W_BLIZZARD
};

class TerrainType
{
public:
    char * name;
	int similar_type;

    enum {
        RIDINGMOUNTS = 0x1,
        FLYINGMOUNTS = 0x2,
    };
    int flags;

    int pop;
    int wages;
    int economy;
    int movepoints;
    int prod1;
    int chance1;
    int amt1;
    int prod2;
    int chance2;
    int amt2;
    int prod3;
    int chance3;
    int amt3;
    int prod4;
    int chance4;
    int amt4;
    int prod5;
    int chance5;
    int amt5;
	int prod6;
	int chance6;
	int amt6;
	int prod7;
	int chance7;
	int amt7;
    int race1;
    int race2;
    int race3;
	int race4;
	int coastalrace1;
	int coastalrace2;
	int coastalrace3;
    int wmonfreq;
    int smallmon;
    int bigmon;
    int humanoid;
    int lairChance;
    int lair1;
    int lair2;
    int lair3;
    int lair4;
	int lair5;
	int lair6;
};

extern TerrainType * TerrainDefs;

class Location : public AListElem
{
public:
    Unit * unit;
    Object * obj;
    ARegion * region;
};

Location * GetUnit(AList *,int);

int AGetName( ARegion *pReg, int town );
char *AGetNameString( int name );

class ARegionPtr : public AListElem
{
public:
    ARegion * ptr;
};

ARegionPtr * GetRegion(AList *,int);

class Farsight : public AListElem
{
public:
    Faction *faction;
};

Farsight *GetFarsight(AList *,Faction *);

enum {
  TOWN_VILLAGE,
  TOWN_TOWN,
  TOWN_CITY,
  NTOWNS
};

class TownInfo
{
public:
    TownInfo();
    ~TownInfo();
    
    void Readin(Ainfile *, ATL_VER v );
    void Writeout(Aoutfile * );
    int TownType();
    
    AString * name;
    int pop;
    int basepop;
    int activity;
};

class ARegion : public AListElem
{
    friend class Game;
    friend class ARegionArray;
public:
    ARegion();
    ARegion(int,int);
    ~ARegion();
    void Setup();
	
    void ZeroNeighbors();
    void SetName(char *);
    
    void Writeout(Aoutfile * );
    void Readin(Ainfile *,AList *, ATL_VER v );
    
    int CanMakeAdv(Faction *,int);
    int HasItem(Faction *,int);
    void WriteProducts(Areport *,Faction *);
    void WriteMarkets(Areport *,Faction *);
    void WriteEconomy(Areport *,Faction *);
    void WriteExits(Areport *, ARegionList *pRegs );
    void WriteReport( Areport *f, Faction *fac, int month,
                      ARegionList *pRegions );
    // DK
    void WriteTemplate(Areport *,Faction *, ARegionList *, int);
    // DK
    void WriteTemplateHeader(Areport *, Faction *, ARegionList *, int);
    // DK
    void GetMapLine(char *, int, ARegionList *);
    AString ShortPrint( ARegionList *pRegs );
    AString Print( ARegionList *pRegs );
    
    void Kill(Unit *);
    void ClearHell();
    
    Unit * GetUnit(int);
    Unit * GetUnitAlias(int,int); /* alias, faction number */
    Unit * GetUnitId(UnitId *,int);
    Location * GetLocation(UnitId *,int);
    
    void SetLoc(int,int,int);
    int Present(Faction *);
    AList * PresentFactions();
    int GetObservation(Faction *);
    int GetTrueSight(Faction *);
    
    Object * GetObject(int);
    Object * GetDummy();
    
    int MoveCost(int);
    Unit * Forbidden(Unit *); /* Returns the unit that is forbidding */
    Unit * ForbiddenByAlly(Unit *); /* Returns the unit that is forbidding */
    int CanTax(Unit *);
    int ForbiddenShip(Object *);
    int HasCityGuard();
    void NotifySpell(Unit *,int, ARegionList *pRegs );
	void NotifyCity(Unit *, AString& oldname, AString& newname);
    
    void DefaultOrders();
    void UpdateTown();
    void PostTurn();
    void UpdateProducts();
    void SetWeather( int newWeather );
    int IsCoastal();
    void MakeStartingCity();
    int IsStartingCity();
    int IsSafeRegion();
    int CanBeStartingCity( ARegionArray *pRA );
    int HasShaft();

    // AS
    int HasRoad();
    int HasExitRoad(int realDirection);
    int CountConnectingRoads();
    int HasConnectingRoad(int realDirection);
    int GetRoadDirection(int realDirection);
    int GetRealDirComp(int realDirection);
    void DoDecayCheck();
    void DoDecayClicks(Object *o);
    void RunDecayEvent(Object *o);
    AString GetDecayFlavor();
    int GetMaxClicks();
    int PillageCheck();
    
    int CountWMons();
    int IsGuarded();
    
    int Wages();
    AString WagesForReport();
    int Population();
    
    AString * name;
    int num;
    int type;
    int buildingseq;
    int weather;
    int gate;
    
    TownInfo * town;
    int race;
    int population;
    int basepopulation;
    int wages;
    int maxwages;
    int money;
    
    /* Potential bonuses to economy */
    int clearskies;
    int earthlore;
    
    ARegion * neighbors[NDIRS];
    AList objects;
    AList hell; /* Where dead units go */
    AList farsees;
    ProductionList products;
    MarketList markets;
    int xloc,yloc,zloc;
    
private:
    /* Private Setup Functions */
    void SetupPop();
    void SetupProds();
    int GetNearestProd(int);
    void SetupCityMarket();
    void AddTown();
    void MakeLair(int);
    void LairCheck();
};

class ARegionArray
{
public:
    ARegionArray(int,int);
    ~ARegionArray();
    
    void SetRegion(int,int,ARegion *);
    ARegion * GetRegion(int,int);
	void SetName( char *name );

    int x;
    int y;
    ARegion ** regions;
    AString *strName;

    enum {
        LEVEL_NEXUS,
        LEVEL_SURFACE,
        LEVEL_UNDERWORLD,
    };
    int levelType;
};

class ARegionFlatArray
{
public:
    ARegionFlatArray(int);
    ~ARegionFlatArray();
    
    void SetRegion(int,ARegion *);
    ARegion * GetRegion(int);
    
    int size;
    ARegion ** regions;
};

class ARegionList : public AList 
{
public:
    ARegionList();
    ~ARegionList();

    ARegion * GetRegion(int);
    ARegion * GetRegion(int,int,int);
    void ReadRegions( Ainfile *f, AList *, ATL_VER v );
    void WriteRegions( Aoutfile *f );
    Location * FindUnit(int);
    
    void ChangeStartingCity(ARegion *,int);
    ARegion *GetStartingCity( ARegion *AC, 
                              int num, 
                              int level,
                              int maxX,
                              int maxY );
    
    ARegion *FindGate(int);
    int GetDistance(ARegion *,ARegion *);
    int GetPlanarDistance(ARegion *,ARegion *);
    int GetWeather( ARegion *pReg, int month );

    int numberofgates;

    ARegionArray *GetRegionArray( int level );

    int numLevels;
    ARegionArray **pRegionArrays;

public:
    //
    // Public world creation stuff
    //
    void CreateLevels( int numLevels );

	void CreateAbyssLevel( int level, char *name );
    void CreateNexusLevel( int level, int xSize, int ySize, char *name );
    void CreateSurfaceLevel( int level, 
                             int xSize, 
                             int ySize,
                             int percentOcean,
                             int continentSize, char *name );
    void CreateIslandLevel( int level,
                            int nPlayers,
                            char *name );
    void CreateUnderworldLevel( int level, int xSize, int ySize, char *name );

    void MakeShaftLinks( int levelFrom, int levelTo, int odds );
    void SetACNeighbors( int levelSrc, 
                         int levelTo, 
                         int maxX,
                         int maxY );
    void InitSetupGates( int level );
    void FinalSetupGates();

    void CalcDensities();

private:
    //
    // Private world creation stuff
    //
    void MakeRegions( int level, int xSize, int ySize );
    void SetupNeighbors( ARegionArray *pRegs );
    void NeighSetup( ARegion *r, ARegionArray *ar );

    void SetRegTypes( ARegionArray *pRegs, int newType );
    void MakeLand( ARegionArray *pRegs,
                   int percentOcean,
                   int continentSize );
    void MakeCentralLand( ARegionArray *pRegs );

    void SetupAnchors( ARegionArray *pArr );
    void GrowTerrain( ARegionArray *pArr, int growOcean );
    void RandomTerrain( ARegionArray *pArr );
    void MakeUWMaze( ARegionArray *pArr );
    void MakeIslands( ARegionArray *pArr, int nPlayers );
    void MakeOneIsland( ARegionArray *pRegs, int xx, int yy );

    void AssignTypes( ARegionArray *pArr );
    void FinalSetup(ARegionArray *);

    void MakeShaft( ARegion *reg, ARegionArray *pFrom, ARegionArray *pTo );

    //
    // Game-specific world stuff (see world.cpp)
    //
    int GetRegType( ARegion *pReg );
    int CheckRegionExit( int nDir, ARegion *pFrom, ARegion *pTo );
};

#endif

