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
class ARegionArray;

#include "gamedefs.h"
#include "gameio.h"
#include "faction.h"
#include "alist.h"
#include "unit.h"
#include "production.h"
#include "market.h"
#include "object.h"
#include "graphs.h"
#include "mapgen.h"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

#include <map>
#include <vector>
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
		char const *name;
		char const *plural;
		char const *type;
		char marker;
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

extern TerrainType *TerrainDefs;

class Location : public AListElem
{
	public:
		Unit *unit;
		Object *obj;
		ARegion *region;
};

Location *GetUnit(AList *, int);

int AGetName(int town, ARegion *r);
char const *AGetNameString(int name);

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
		int observation;
		int exits_used[NDIRS];
};

Farsight *GetFarsight(AList *, Faction *);

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

		void Readin(istream& f);
		void Writeout(ostream& f);
		int TownType();

		AString *name;
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

/**
 * @brief Represents a connection between two regions.
 *
 * This is a tuple of two regions; an edge in the region graph.
 * If the edge is bidirectional, it is not important which region is the left and which is the right.
 *
 * Edge allows to add additional information to the connection between the regions.
 *
 */
class RegionEdge {
private:
    RegionEdge(const bool undirected, const int id, ARegion* left, ARegion* right, const int left_dir, const int right_dir)
        : left(left), right(right), is_undirected(undirected), id(id), left_dir(left_dir), right_dir(right_dir) {}

    ARegion* left;
    ARegion* right;

public:
    using regionType = ARegion *;

    /**
     * @brief Represents the other side of the edge.
     */
    struct side {
        regionType region;
        int direction;
    };

    /**
     * @brief True if the edge is bidirectional, false if it is directed.
     *
     *        If the edge is directed, the left region is the source and the right region is the destination.
     */
    const bool is_undirected;

    /**
     * @brief A unique identifier for this connection.
	 *
	 * This is used to identify the edge in the edges list of the region graph.
	 * Also MOVE order can use this id to specify the destination of the movement.
     *
     */
    const int id;

    /**
     * @brief Direction of the edge from the left region to the right region.
     */
    const int left_dir;

    /**
     * @brief Direction of the edge from the right region to the left region.
     */
    const int right_dir;

    /**
     * @brief The left region of the edge.
     */
    ARegion* get_left() const;

    /**
     * @brief The right region of the edge.
     */
    ARegion* get_right() const;

    /**
     * @brief Returns the other side of the edge connected to the source region.
     *
     * @param source The region to get the other side of.
     * @param side The other side of the edge and the direction from the source to the other side.
     * @return const bool True if the source region is connected to the other side of the edge,
     *                    false otherwise.
     */
    const bool other_side(const regionType source, side &side) const;

    /**
     * @brief Creates a new edge between two regions.
     *
     * This is the only way to create a new edge.
     *
     * @param id A unique identifier for the edge.
     * @param left The left region of the edge.
     * @param right The right region of the edge.
     * @param left_dir The direction of the edge from the left region to the right region.
     * @param right_dir The direction of the edge from the right region to the left region.
     * @return const Edge* The new edge.
     */
    static RegionEdge* create(const int id, regionType left, regionType right, const int dir);

    static RegionEdge* create_directed(const int id, regionType left, regionType right, const int dir);
};

/**
 * @brief Represents connections to other regions and locations from a region.
 */
class RegionEdges {
friend class ARegion;
private:
    RegionEdges();

public:
    using id_type = unsigned int;
    using value_type = const RegionEdge *;
    using map_type = std::unordered_map<id_type, value_type>;
    using region_type = ARegion *;
    using region_array_type = std::array<region_type, NDIRS>;

    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag ;
        using value_type = RegionEdges::value_type;
        using pointer = const value_type *;
        using reference = const value_type &;
        using difference_type = map_type::iterator::difference_type;

        reference operator*() const;
        pointer operator&() const;
        iterator& operator++ ();
        iterator operator++(int);
        bool operator==(const iterator &that) const noexcept;
        bool operator!=(const iterator &that) const noexcept;

    private:
        map_type::const_iterator internal_iterator;

        iterator( map_type::const_iterator iter ) : internal_iterator(iter) {}

        friend RegionEdges;
    };

    RegionEdges(region_type owner);

    /**
     * @brief Returns the number of connections to other regions and locations.
     *
     * @return const int The number of connections.
     */
    const int size() const;

    /**
     * @brief Returns the edge with the given id.
     *
     * @param id The id of the edge to get.
     * @return itemType The edge with the given id or nullptr if there is no such edge.
     */
    value_type get(const id_type id) const;

    /**
     * @brief Adds a new edge to the local list of edges.
     *
     * @param edge The edge to add.
     */
    void add(value_type edge);

    /**
     * @brief Removes an edge from the local list of edges of the region.
     *
     * @param id The id of the edge to remove.
     */
    void remove(const id_type id);

    /**
     * @brief Returns the neighboring region in the given direction.
     *
     * This function is for the compatibility with the old code.
     *
     * @param dir The direction to get the neighbor of.
     * @return ARegion* The neighboring region in the given direction or nullptr if there is no neighbor.
     */
    region_type get_neighbor(const int dir) const;

    /**
     * @brief Returns the edge in the given direction.
     *
     * @param dir The direction to get the edge of.
     * @return itemType The edge in the given direction or nullptr if there is no edge.
     */
    value_type get_edge(const int dir) const;

    iterator begin() const noexcept;
    iterator end() const noexcept;

private:
    region_type owner;
    map_type _edges;
    region_array_type neighbors;
};

class GameEdges {
public:
    using id_type = unsigned int;
    using value_type = const RegionEdge *;
    using map_type = std::unordered_map<id_type, value_type>;

    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag ;
        using value_type = GameEdges::value_type;
        using pointer = const value_type *;
        using reference = const value_type &;
        using difference_type = map_type::iterator::difference_type;

        reference operator*() const;
        pointer operator&() const;
        iterator& operator++ ();
        iterator operator++(int);
        bool operator==(const iterator &that) const noexcept;
        bool operator!=(const iterator &that) const noexcept;

    private:
        map_type::const_iterator internal_iterator;

        iterator( map_type::const_iterator iter ) : internal_iterator(iter) {}

        friend GameEdges;
    };

    iterator begin() const noexcept;
    iterator end() const noexcept;

    ~GameEdges();

    const std::size_t size() const;

    const id_type get_last_edge_id() const;
    void set_last_edge_id(const id_type id);

    value_type get(const id_type id) const;
    value_type create(ARegion *left, ARegion *right, const int dir);
    value_type create_directed(ARegion *left, ARegion *right, const int dir);
    void add(value_type edge);
    value_type find(const ARegion *left, const ARegion *right) const;
    const bool has(const ARegion *left, const ARegion *right) const;
    const bool has(const id_type id) const;
    const bool remove(const id_type id);
    const bool remove(value_type edge);

private:
    id_type _last_edge_id;
    map_type _edges;

    const id_type next_edge_id();
};

class ARegion : public AListElem
{
	friend class Game;
	friend class ARegionArray;

	public:
		ARegion();
		//ARegion(int, int);
		~ARegion();

		void Setup();
		void ManualSetup(const RegionSetup& settings);

		void SetName(char const *);

		void Writeout(ostream& f);
		void Readin(istream& f, AList *);

		int CanMakeAdv(Faction *, int);
		int HasItem(Faction *, int);
		json basic_region_data();
		void build_json_report(json& j, Faction *fac, int month, ARegionList *pRegions);

		AString ShortPrint();
		AString Print();

		void Kill(Unit *);
		void ClearHell();

		Unit *GetUnit(int);
		Unit *GetUnitAlias(int, int); /* alias, faction number */
		Unit *GetUnitId(UnitId *, int);
		void DeduplicateUnitList(AList *, int);
		Location *GetLocation(UnitId *, int);

		void SetLoc(int, int, int);
		int Present(Faction *);
		AList *PresentFactions();
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
		int NotifySpell(Unit *, char const *, ARegionList *pRegs);
		void NotifyCity(Unit *, AString& oldname, AString& newname);

		void DefaultOrders();
		int TownGrowth();
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
		void FindMigrationDestination(int round);
		int MigrationAttractiveness(int, int, int);
		void Migrate();
		void SetTownType(int);
		int DetermineTownSize();
		int TraceConnectedRoad(int, int, AList *, int, int);
		int RoadDevelopmentBonus(int, int);
		int BaseDev();
		int ProdDev();
		int TownHabitat();
		int RoadDevelopment();
		int TownDevelopment();
		int CheckSea(int, int, int);

        // TODO: Functions below should be belong to the map generator not the Region class.
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
		AString WagesForReport();
		int Population();

		// ruleset specific movment checks
		const char *movement_forbidden_by_ruleset(Unit *unit, ARegion *origin, ARegionList *regions);

		AString *name;
		int num;
		int type;
		int buildingseq;
		int weather;
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
		int elevation;
		int humidity;
		int temperature;
		int vegetation;
		int culture;
		// migration origins
		AList migfrom;
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

        /**
         * The edges of the region graph that connect this region to other regions and locations.
         */
        RegionEdges edges;

        /**
         * @brief Returns the neighboring region in the given direction.
         *
         * This function is for the compatibility with the old code.
         *
         * @param dir The direction to get the neighbor of.
         * @return ARegion* The neighboring region in the given direction or nullptr if there is no neighbor.
         */
        ARegion* neighbors(const int dir);

        AList objects;
		map<int,int> newfleets;
		int fleetalias;
		AList hell; /* Where dead units go */
		AList farsees;
		// List of units which passed through the region
		AList passers;
		std::vector<Production *> products;
		std::vector<Market *> markets;
		int xloc, yloc, zloc;
		int visited;

		// Used for calculating distances using an A* search
        // TODO: Remove this! A* distance calculation should be done outside of the region class.
		int distance;
		ARegion *next;

		/**
		 * @brief A link to the region's level to make some things easier.
		 */
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
		int GetNearestProd(int);
		void SetupCityMarket();
		void AddTown();
		void AddTown(int);
		void AddTown(AString *);
		void AddTown(int, AString *);
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
		void SetName(char const *name);

		std::vector<ARegion *> get_starting_region_candidates(int terrain);

		int x;
		int y;
		ARegion **regions;
		AString *strName;

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
		map<long int,Geography> geomap;

};

class ARegionList : public AList
{
	public:
		ARegionList();
		~ARegionList();

		ARegion *GetRegion(int);
		ARegion *GetRegion(int, int, int);
		int ReadRegions(istream &f, AList *);
		void WriteRegions(ostream&  f);
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

		int numberofgates;
		int numLevels;
		ARegionArray **pRegionArrays;

	public:
        using iterator = AListIterator<ARegion>;

		//
		// Public world creation stuff
		//
		void CreateLevels(int numLevels);
		void CreateAbyssLevel(int level, char const *name);
		void CreateNexusLevel(int level, int xSize, int ySize, char const *name);
		void CreateSurfaceLevel(int level, int xSize, int ySize, char const *name);
		void CreateNaturalSurfaceLevel(Map* map);
		void CreateIslandRingLevel(int level, int xSize, int ySize, char const *name);
		void CreateIslandLevel(int level, int nPlayers, char const *name);
		void CreateUnderworldLevel(int level, int xSize, int ySize, char const *name);
		void CreateUnderworldRingLevel(int level, int xSize, int ySize, char const *name);
		void CreateUnderdeepLevel(int level, int xSize, int ySize, char const *name);

		void MakeShaftLinks(int levelFrom, int levelTo, int odds);
		void SetACNeighbors(int levelSrc, int levelTo, int maxX, int maxY);
		ARegion *FindConnectedRegions(ARegion *r, ARegion *tail, int shaft);
		ARegion *FindNearestStartingCity(ARegion *r, int *dir);
		int FindDistanceToNearestObject(int object, ARegion *r);
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

        iterator begin() { return iterator((ARegion *) this->First()); }
        iterator end() { return iterator(); }

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

        // Region Edges
        GameEdges edges;
};

int LookupRegionType(AString *);
int ParseTerrain(AString *);

using ARegionCostFunction = std::function<double(ARegion*, ARegion*)>;
using ARegionInclusionFunction = std::function<bool(ARegion*, ARegion*)>;

template <class TLoc, class TContainer>
class BaseRegionGraph : public graphs::Graph<TLoc, ARegion*> {
public:
    BaseRegionGraph(TContainer* regions) : regions(regions) {
        this->costFn = [](ARegion* current, ARegion* next) { return 1; };
        this->includeFn = [](ARegion* current, ARegion* next) { return true; };
    }

    virtual double cost(TLoc current, TLoc next) {
        return this->costFn(get(current), get(next));
    }

    virtual void setCost(ARegionCostFunction costFn) {
        this->costFn = costFn;
    }

    virtual void setInclusion(ARegionInclusionFunction includeFn) {
        this->includeFn = includeFn;
    }

protected:
    TContainer* regions;
    ARegionCostFunction costFn;
    ARegionInclusionFunction includeFn;
};

class RegionGraph : public BaseRegionGraph<graphs::Location3D, ARegionList> {
public:
    RegionGraph(ARegionList* regions) :
        BaseRegionGraph(regions),
        follow_inner_location(false),
        follow_gates(false),
        follow_nexus_gate(true),
        stay_in_same_level(false),
        ignore_objects({})
        { }

    ARegion* get(graphs::Location3D id);
    std::vector<graphs::Location3D> neighbors(graphs::Location3D id);

    /**
     * @brief If true, the graph will follow the inner location of the region.
     *
     *        Default is `false`.
     */
    bool follow_inner_location;

    /**
     * @brief If true, the graph will follow the gates of the region.
     *
     *         Default is `false`.
     */
    bool follow_gates;

    /**
     * @brief If true, the graph will follow the Nexus gate and connect
     *        the regions in the Nexus level to all other regions with a gate.
     *
     *        Default is `true`.
     */
    bool follow_nexus_gate;

    /**
     * @brief If true, the graph will stay in the same level when following the gates.
     *
     *        Default is `false`.
     */
    bool stay_in_same_level;

    /**
     * @brief List of object types to ignore when following the inner locations.
     */
    std::vector<int> ignore_objects;
};

class SingleLayerRegionGraph : public BaseRegionGraph<graphs::Location2D, ARegionArray> {
public:
    SingleLayerRegionGraph(ARegionArray* regions) : BaseRegionGraph(regions) { }

    ARegion* get(graphs::Location2D id);
	std::vector<graphs::Location2D> neighbors(graphs::Location2D id);
};

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

/**
 * @brief Performs a breadth-first search from the start region.
 *
 * @param start The region to start the search from.
 * @param maxDistance The maximum distance to search. -1 means no limit.
 * @return const std::unordered_map<ARegion*, graphs::Node<ARegion*>>& Distance map from the start region.
 */
const std::unordered_map<ARegion*, graphs::Node<ARegion*>> &breadthFirstSearch(ARegion* start, const int maxDistance);

#endif
