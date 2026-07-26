#include "external/boost/ut.hpp"

#include <algorithm>
#include <vector>
#include <unordered_map>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "graphs.h" // graphs::Location2D
#include "orders.h" // direction constants (breadthFirstSearch suite)

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	using graphs::Location2D;

	struct Grid {
		ARegionArray *arr;
		ARegion *c; // center, at (2,2)
		ARegion *n; // north neighbor, at (2,0)
		ARegion *s; // south neighbor, at (2,4)
	};

	// Build a tiny surface array with a center region wired to two neighbors (north and
	// south). Each region is stored in the array at its own (xloc,yloc), because
	// ARegionGraph::get() looks regions up by grid coordinate while ARegionGraph::neighbors()
	// reports each neighbor's own (xloc,yloc) -- the two must agree for a consistent graph.
	// Cells are chosen on even (x+y) parity, the only cells ARegionArray stores.
	Grid buildGrid()
	{
		ARegionArray *arr = new ARegionArray(6, 6);

		ARegion *c = new ARegion(); c->SetLoc(2, 2, ARegionArray::LEVEL_SURFACE);
		ARegion *n = new ARegion(); n->SetLoc(2, 0, ARegionArray::LEVEL_SURFACE);
		ARegion *s = new ARegion(); s->SetLoc(2, 4, ARegionArray::LEVEL_SURFACE);

		arr->SetRegion(2, 2, c);
		arr->SetRegion(2, 0, n);
		arr->SetRegion(2, 4, s);

		// The constructor already zeroes neighbor slots; wire only the two we exercise.
		c->neighbors[D_NORTH] = n;
		c->neighbors[D_SOUTH] = s;

		return { arr, c, n, s };
	}

	bool contains(const std::vector<Location2D> &v, Location2D loc)
	{
		return std::find(v.begin(), v.end(), loc) != v.end();
	}
}

ut::suite<"ARegionGraph"> aregion_graph_suite = []
{
	using namespace ut;

	// get() delegates to ARegionArray::GetRegion, so it returns the stored region for a
	// populated cell and null for an empty one.
	"get resolves a location to its stored region"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		expect(graph.get({2, 2}) == g.c);
		expect(graph.get({2, 0}) == g.n);
		expect(graph.get({2, 4}) == g.s);
		expect(graph.get({0, 0}) == nullptr) << "an unpopulated (even-parity) cell is null";
	};

	// A freshly constructed graph uses the default cost function, which is a flat 1 for
	// every edge regardless of the endpoints.
	"default cost is a flat 1 per edge"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		expect(graph.cost({2, 0}, {2, 2}) == 1.0_d);
		expect(graph.cost({2, 2}, {2, 4}) == 1.0_d);
	};

	// neighbors() walks the region's neighbors[] array, skips null directions, and reports
	// each present neighbor's own coordinates. The default inclusion function admits all.
	"neighbors lists the coordinates of all linked neighbors"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		std::vector<Location2D> nb = graph.neighbors({2, 2});
		expect(nb.size() == 2_ul) << "center has exactly two wired neighbors";
		expect(contains(nb, {2, 0})) << "north neighbor present";
		expect(contains(nb, {2, 4})) << "south neighbor present";
	};

	// A region with no wired neighbors yields an empty list (all NDIRS slots are null).
	"neighbors is empty for a region with no links"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		std::vector<Location2D> nb = graph.neighbors({2, 0}); // the north region, unwired
		expect(nb.size() == 0_ul);
	};

	// setInclusion installs a predicate that filters which neighbors are traversable. Here
	// we exclude the south region by coordinate; only the north neighbor survives.
	"setInclusion filters the neighbor list"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		graph.setInclusion([](ARegion *current, ARegion *next) {
			(void)current;
			return next->yloc < 3; // north (y=0) included, south (y=4) excluded
		});

		std::vector<Location2D> nb = graph.neighbors({2, 2});
		expect(nb.size() == 1_ul);
		expect(contains(nb, {2, 0}));
		expect(!contains(nb, {2, 4})) << "south is filtered out";
	};

	// An inclusion predicate that rejects everything empties the neighbor list entirely.
	"setInclusion rejecting all yields no neighbors"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		graph.setInclusion([](ARegion *, ARegion *) { return false; });
		expect(graph.neighbors({2, 2}).size() == 0_ul);
	};

	// setCost installs a custom edge-cost function; cost() then reports its value.
	"setCost overrides the edge cost"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		graph.setCost([](ARegion *, ARegion *) { return 5.0; });
		expect(graph.cost({2, 0}, {2, 2}) == 5.0_d);
		expect(graph.cost({2, 2}, {2, 4}) == 5.0_d);
	};

	// cost() passes get(current) as the first argument and get(next) as the second. We probe
	// the ordering with a cost function that reads yloc of each endpoint distinctly.
	"cost passes current then next to the cost function"_test = []
	{
		Grid g = buildGrid();
		ARegionGraph graph(g.arr);

		graph.setCost([](ARegion *current, ARegion *next) {
			return (double)(current->yloc * 10 + next->yloc);
		});

		// current = north (y=0), next = center (y=2) -> 0*10 + 2 = 2
		expect(graph.cost({2, 0}, {2, 2}) == 2.0_d);
		// current = center (y=2), next = south (y=4) -> 2*10 + 4 = 24
		expect(graph.cost({2, 2}, {2, 4}) == 24.0_d);
	};
};

namespace {
	ARegion *node(int num)
	{
		ARegion *r = new ARegion();
		r->num = num;
		return r;
	}

	// Wire a<->b through a direction slot each. breadthFirstSearch only walks the neighbors[]
	// array, so the exact directions are immaterial as long as the link is mutual.
	void linkNodes(ARegion *a, int da, ARegion *b, int db)
	{
		a->neighbors[da] = b;
		b->neighbors[db] = a;
	}
}

// breadthFirstSearch (declared in aregion.h) is the public region-graph BFS used by events.cpp.
// It returns a came-from map keyed by every region it reaches. The subtle branch is the distance
// cutoff: a node is *expanded* only while its distance <= maxDistance, but that expansion still
// *records* the node's neighbors one step further out -- so the map reaches exactly
// maxDistance + 1.
ut::suite<"ARegion breadthFirstSearch"> aregion_bfs_suite = []
{
	using namespace ut;

	// Linear chain A-B-C-D-E-F. From A with maxDistance 2, the search expands A(0), B(1), C(2)
	// and records D(3) as C's neighbor without expanding it. E and F are never reached.
	"breadthFirstSearch reaches maxDistance + 1 on a chain"_test = []
	{
		ARegion *a = node(0), *b = node(1), *c = node(2);
		ARegion *d = node(3), *e = node(4), *f = node(5);
		linkNodes(a, D_SOUTH, b, D_NORTH);
		linkNodes(b, D_SOUTH, c, D_NORTH);
		linkNodes(c, D_SOUTH, d, D_NORTH);
		linkNodes(d, D_SOUTH, e, D_NORTH);
		linkNodes(e, D_SOUTH, f, D_NORTH);

		auto came = breadthFirstSearch(a, 2);

		expect(came.count(a) == 1_ul) << "start is recorded";
		expect(came.count(b) == 1_ul);
		expect(came.count(c) == 1_ul);
		expect(came.count(d) == 1_ul) << "the node one past maxDistance is still recorded";
		expect(came.count(e) == 0_ul) << "maxDistance + 2 is never reached";
		expect(came.count(f) == 0_ul);
		// The start maps to itself at distance 0.
		expect(came.at(a).key == a);
		expect(came.at(a).distance == 0_i);
	};

	// maxDistance 0: A is expanded (its neighbors recorded), but none of them are expanded, so
	// only A and its immediate neighbors appear.
	"breadthFirstSearch with maxDistance 0 records only the start and its neighbors"_test = []
	{
		ARegion *a = node(0), *b = node(1), *c = node(2);
		linkNodes(a, D_SOUTH, b, D_NORTH);
		linkNodes(b, D_SOUTH, c, D_NORTH);

		auto came = breadthFirstSearch(a, 0);

		expect(came.count(a) == 1_ul);
		expect(came.count(b) == 1_ul) << "immediate neighbor recorded";
		expect(came.count(c) == 0_ul) << "two hops out is not";
	};

	// A cyclic graph must terminate and visit each node once (the cameFrom.find guard prevents
	// re-adding a region already seen).
	"breadthFirstSearch terminates on a cycle and visits each node once"_test = []
	{
		ARegion *a = node(0), *b = node(1), *c = node(2);
		linkNodes(a, D_SOUTHEAST, b, D_NORTHWEST);
		linkNodes(b, D_SOUTHWEST, c, D_NORTHEAST);
		linkNodes(c, D_NORTH, a, D_SOUTH);

		auto came = breadthFirstSearch(a, 10); // must return, not hang

		expect(came.size() == 3_ul) << "triangle visited exactly once each";
	};

	// A lone region with no neighbors yields a map containing only itself.
	"breadthFirstSearch on an isolated region returns just the start"_test = []
	{
		ARegion *a = node(0);
		auto came = breadthFirstSearch(a, 5);
		expect(came.size() == 1_ul);
		expect(came.count(a) == 1_ul);
	};
};
