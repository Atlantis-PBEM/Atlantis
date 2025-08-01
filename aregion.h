#pragma once
#ifndef REGION_H
#define REGION_H

class ARegion;
class ARegionList;
class ARegionArray;

#include "gamedefs.h"
#include "logger.hpp"
#include "faction.h"
#include "unit.h"
#include "production.h"
#include "market.h"
#include "object.h"
#include "graphs.h"
#include "mapgen.h"
#include "safe_list.h"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

#include <map>
#include <vector>
#include <list>
#include <set>
#include <functional>

/* Weather Types */
enum {
    W_NORMAL,
    W_WINTER,
    W_MONSOON,
    W_BLIZZARD
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
        const std::string name;
        const std::string plural;
        const std::string type;
        const char marker;
        int similar_type;

        enum {
            RIDINGMOUNTS  = 0x1,
            FLYINGMOUNTS  = 0x2,
            BARREN        = 0x4,
            SHOW_RULES    = 0x8,
            ANNIHILATED   = 0x10,
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

 extern std::vector<TerrainType> TerrainDefs;

class Location
{
    public:
        Unit *unit;
        Object *obj;
        ARegion *region;
};

Location *GetUnit(std::list<Location *>& locs, int unitid);

int AGetName(int town, ARegion *r);
const std::string& AGetNameString(int name);

class Farsight
{
    public:
        Farsight();

        Faction *faction;
        Unit *unit;
        int level;
        int observation;
        int exits_used[NDIRS];
};

Farsight *GetFarsight(std::list<Farsight *>& farsees, Faction *);

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

        void Readin(std::istream& f);
        void Writeout(std::ostream& f);
        int TownType();

        std::string name;
        int pop;
        int activity;
        // achieved settled habitat
        int hab;
        // town's development
        int dev;
};

struct RegionSetup {
    TerrainType* terrain;
    int habitat;
    double prodWeight;
    bool addLair;
    bool addSettlement;
    std::string settlementName;
    int settlementSize;
};

class ARegion
{
    friend class Game;
    friend class ARegionArray;

    public:
        ARegion();
        //ARegion(int, int);
        ~ARegion();

        void Setup();
        void ManualSetup(const RegionSetup& settings);

        void ZeroNeighbors();
        void set_name(const std::string& newname);

        void Writeout(std::ostream& f);
        void Readin(std::istream& f, std::list<Faction *>& factions);

        int CanMakeAdv(Faction *, int);
        int HasItem(Faction *, int);
        json basic_region_data();
        void build_json_report(json& j, Faction *fac, int month, ARegionList& regions);

        std::string short_print();
        std::string print();

        void Kill(Unit *);
        void ClearHell();

        Unit *GetUnit(int);
        Unit *GetUnitAlias(int, int); /* alias, faction number */
        Unit *GetUnitId(UnitId *, int);
        void deduplicate_unit_list(std::list<UnitId *>& list, int factionid);
        Location *GetLocation(UnitId *, int);

        void SetLoc(int, int, int);
        bool Present(Faction *f);
        std::set<Faction *> PresentFactions();
        int GetObservation(Faction *, int);
        int GetTrueSight(Faction *, int);

        Object *GetObject(int);
        Object *GetDummy();
        void CheckFleets();

        int MoveCost(int, ARegion *, int, std::string *road);
        Unit *Forbidden(Unit *); /* Returns unit that is forbidding */
        Unit *ForbiddenByAlly(Unit *); /* Returns unit that is forbidding */
        int CanTax(Unit *);
        int CanGuard(Unit *);
        int CanPillage(Unit *);
        void Pillage();
        int ForbiddenShip(Object *);
        int HasCityGuard();
        bool notify_spell_use(Unit *caster, const std::string& spell, ARegionList& regs);
        void notify_city(Unit *, const std::string& oldname, const std::string& newname);

        void DefaultOrders();
        int TownGrowth();
        void PostTurn();
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
        void DoDecayCheck();
        void DoDecayClicks(Object *o);
        void RunDecayEvent(Object *o);
        std::string get_decay_flavor();
        int GetMaxClicks();
        int PillageCheck();

        // JR
        int GetPoleDistance(int dir);
        void SetGateStatus(int month);
        void DisbandInRegion(int, int);
        void Recruit(int);
        int IsNativeRace(int);
        void AdjustPop(int);
        void FindMigrationDestination(int round);
        int MigrationAttractiveness(int, int, int);
        void Migrate();
        void SetTownType(int);
        int DetermineTownSize();
        int TraceConnectedRoad(int dir, int sum, std::list<ARegion *>& con, int range, int dev);
        int RoadDevelopmentBonus(int, int);
        int BaseDev();
        int ProdDev();
        int TownHabitat();
        int RoadDevelopment();
        int TownDevelopment();
        int CheckSea(int, int, int);
        int Slope();
        int SurfaceWater();
        int Soil();
        int Winds();
        int TerrainFactor(int, int);
        int TerrainProbability(int);
        void AddFleet(Object *);
        int ResolveFleetAlias(int);

        int CountWMons();
        int IsGuarded();

        int Wages();
        std::string wages_for_report();
        int Population();

        // ruleset specific movment checks
        const std::optional<std::string> movement_forbidden_by_ruleset(Unit *unit, ARegion *origin, ARegionList& regions);

        std::string name;
        int num;
        int type;
        int buildingseq;
        int weather = W_NORMAL;
        int gate;
        int gatemonth;
        int gateopen;

        TownInfo *town;
        int race;
        int population;
        int basepopulation;
        int wages;
        int maxwages;
        int wealth;

        /* Economy */
        int habitat;
        int development;
        int maxdevelopment;
        int elevation = -1;
        int humidity = -1;
        int temperature = -1;
        int vegetation = -1;
        int culture = -1;
        // migration origins
        std::list<ARegion *> migfrom;
        // mid-way migration development
        int migdev;
        int immigrants;
        int emigrants;
        // economic improvement
        int improvement;

        /* Potential bonuses to economy */
        int clearskies;
        int earthlore;
        int phantasmal_entertainment;

        ARegion *neighbors[NDIRS];
        safe::list<Object *> objects;
        std::map<int,int> newfleets;
        int fleetalias;
        std::list<Unit *> hell; /* Where dead units go */
        std::list<Farsight *> farsees;
        // List of units which passed through the region
        std::list<Farsight *>passers;
        std::vector<Production *> products;
        std::vector<Market*> markets;
        int xloc, yloc, zloc;
        int visited;

        // Used for calculating distances using an A* search
        int distance;
        ARegion *next;

        // A link to the region's level to make some things easier.
        ARegionArray *level;

        // find a production for a certain skill.
        Production *get_production_for_skill(int item, int skill);
        int produces_item(int item);

        // Editing functions
        void UpdateEditRegion();
        void SetupEditRegion();
    private:
        /* Private Setup Functions */
        void SetupPop();
        void SetupProds(double weight);
        void SetIncome();
        void Grow();
        void SetupCityMarket();
        void add_town();
        void add_town(int size);
        void add_town(const std::string& name);
        void add_town(int size, const std::string& name);
        void MakeLair(int);
        void LairCheck();
        std::vector<int> GetPossibleLairs();
        void SetupHabitat(TerrainType* terrain);
        void SetupEconomy();
};

class ARegionArray
{
    public:
        ARegionArray(int, int);
        ~ARegionArray();

        void SetRegion(int, int, ARegion *);
        ARegion *GetRegion(int, int);
        void set_name(const std::string& name);

        std::vector<ARegion *> get_starting_region_candidates(int terrain);

        int x;
        int y;
        ARegion **regions = nullptr;
        std::string strName;

        enum {
            LEVEL_NEXUS,
            LEVEL_SURFACE,
            LEVEL_UNDERWORLD,
            LEVEL_UNDERDEEP,
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
        std::map<long int, Geography> geomap;

};

class ARegionList
{
    std::vector<ARegion *> regions;

    public:
        using iterator = typename std::vector<ARegion *>::iterator;

        ~ARegionList();

        ARegion *GetRegion(int);
        ARegion *GetRegion(int, int, int);
        int ReadRegions(std::istream &f, std::list<Faction *>& facs);
        void WriteRegions(std::ostream&  f);
        Location *FindUnit(int);
        Location *GetUnitId(UnitId *id, int faction, ARegion *cur);

        void ChangeStartingCity(ARegion *, int);
        ARegion *GetStartingCity(ARegion *AC, int num, int level, int maxX,
                int maxY);

        ARegion *FindGate(int);
        int GetPlanarDistance(ARegion *one, ARegion *two, int penalty, int maxdist = -1);
        int get_connected_distance(ARegion *start, ARegion *target, int penalty, int maxdist = -1);
        int GetWeather(ARegion *pReg, int month);

        ARegionArray *GetRegionArray(int level);
        ARegionArray *get_first_region_array_of_type(int type);

        int numberofgates = 0;
        int numLevels = 0;
        ARegionArray **pRegionArrays = nullptr;

        inline iterator begin() { return regions.begin(); }
        inline iterator end() { return regions.end(); }
        inline iterator erase(iterator it) { return regions.erase(it); }
        inline size_t size() { return regions.size(); }
        inline void clear() { regions.clear(); }
        inline ARegion *front() { return regions.front(); }

        //
        // Public world creation stuff
        //
        void create_levels(int numLevels);
        void create_abyss_level(int level, const std::string& name);
        void create_nexus_level(int level, int xSize, int ySize, const std::string& name);
        void create_surface_level(int level, int xSize, int ySize, const std::string& name);
        void create_natural_surface_level(Map* map);
        void create_island_ring_level(int level, int xSize, int ySize, const std::string& name);
        void create_island_level(int level, int nPlayers, const std::string& name);
        void create_underworld_level(int level, int xSize, int ySize, const std::string& name);
        void create_underworld_ring_level(int level, int xSize, int ySize, const std::string& name);
        void create_underdeep_level(int level, int xSize, int ySize, const std::string& name);

        void MakeShaftLinks(int levelFrom, int levelTo, int odds);
        void SetACNeighbors(int levelSrc, int levelTo, int maxX, int maxY);
        ARegion *FindConnectedRegions(ARegion *r, ARegion *tail, int shaft);
        ARegion *FindNearestStartingCity(ARegion *r, int *dir);
        int FindDistanceToNearestObject(int object, ARegion *r);
        int find_distance_between_regions(ARegion *start, ARegion *target);
        void FixUnconnectedRegions();
        void InitSetupGates(int level);
        void FinalSetupGates();

        // JR
        void InitGeographicMap(ARegionArray *pRegs);
        void CleanUpWater(ARegionArray *pRegs);
        void RemoveCoastalLakes(ARegionArray *pRegs);
        void SeverLandBridges(ARegionArray *pRegs);
        void RescaleFractalParameters(ARegionArray *pArr);
        void SetFractalTerrain(ARegionArray *pArr);
        void NameRegions(ARegionArray *pArr);
        void UnsetRace(ARegionArray *pRegs);
        void RaceAnchors(ARegionArray *pRegs);
        void GrowRaces(ARegionArray *pRegs);

        void TownStatistics();
        void ResourcesStatistics();

        void CalcDensities();
        int GetLevelXScale(int level);
        int GetLevelYScale(int level);

        void AddHistoricalBuildings(ARegionArray* arr, const int w, const int h);

    private:
        //
        // Private world creation stuff
        //
        void MakeRegions(int level, int xSize, int ySize);
        void SetupNeighbors(ARegionArray *pRegs);
        void NeighSetup(ARegion *r, ARegionArray *ar);
        void MakeIcosahedralRegions(int level, int xSize, int ySize);
        void SetupIcosahedralNeighbors(ARegionArray *pRegs);
        void IcosahedralNeighSetup(ARegion *r, ARegionArray *ar);

        void SetRegTypes(ARegionArray *pRegs, int newType);
        void MakeLand(ARegionArray *pRegs, int percentOcean, int continentSize);
        void MakeCentralLand(ARegionArray *pRegs);
        void MakeRingLand(ARegionArray *pRegs, int minDistance, int maxDistance);

        void SetupAnchors(ARegionArray *pArr);
        void GrowTerrain(ARegionArray *pArr, int growOcean);
        void RandomTerrain(ARegionArray *pArr);
        void MakeUWMaze(ARegionArray *pArr);
        void PlaceVolcanos(ARegionArray *pArr);
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

};

int parse_terrain(const strings::ci_string& token);

using ARegionCostFunction = std::function<double(ARegion*, ARegion*)>;
using ARegionInclusionFunction = std::function<bool(ARegion*, ARegion*)>;

class ARegionGraph : public graphs::Graph<graphs::Location2D, ARegion*> {
public:
    ARegionGraph(ARegionArray* regions);
    ~ARegionGraph();

    ARegion* get(graphs::Location2D id);
    std::vector<graphs::Location2D> neighbors(graphs::Location2D id);
    double cost(graphs::Location2D current, graphs::Location2D next);

    void setCost(ARegionCostFunction costFn);
    void setInclusion(ARegionInclusionFunction includeFn);

private:
    ARegionArray* regions;
    ARegionCostFunction costFn;
    ARegionInclusionFunction includeFn;
};

const std::unordered_map<ARegion*, graphs::Node<ARegion*>> breadthFirstSearch(ARegion* start, const int maxDistance);

#endif // REGION_H
