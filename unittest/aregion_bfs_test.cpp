#include "external/boost/ut.hpp"

#include <unordered_map>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "graphs.h"
#include "orders.h" // direction constants

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	ARegion *node(int num)
	{
		ARegion *r = new ARegion();
		r->num = num;
		return r;
	}

	// Wire a<->b through a direction slot each. breadthFirstSearch only walks the neighbors[]
	// array, so the exact directions are immaterial as long as the link is mutual.
	void link(ARegion *a, int da, ARegion *b, int db)
	{
		a->neighbors[da] = b;
		b->neighbors[db] = a;
	}
}

// breadthFirstSearch (aregion.cpp:4129, declared in aregion.h) is the public region-graph BFS
// used by events.cpp. It returns a came-from map keyed by every region it reaches. It was
// previously uncovered. The subtle branch is the distance cutoff: a node is *expanded* only
// while its distance <= maxDistance, but that expansion still *records* the node's neighbors
// one step further out -- so the map reaches exactly maxDistance + 1.
ut::suite<"ARegion breadthFirstSearch"> aregion_bfs_suite = []
{
	using namespace ut;

	// Linear chain A-B-C-D-E-F. From A with maxDistance 2, the search expands A(0), B(1), C(2)
	// and records D(3) as C's neighbor without expanding it. E and F are never reached.
	"breadthFirstSearch reaches maxDistance + 1 on a chain"_test = []
	{
		ARegion *a = node(0), *b = node(1), *c = node(2);
		ARegion *d = node(3), *e = node(4), *f = node(5);
		link(a, D_SOUTH, b, D_NORTH);
		link(b, D_SOUTH, c, D_NORTH);
		link(c, D_SOUTH, d, D_NORTH);
		link(d, D_SOUTH, e, D_NORTH);
		link(e, D_SOUTH, f, D_NORTH);

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
		link(a, D_SOUTH, b, D_NORTH);
		link(b, D_SOUTH, c, D_NORTH);

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
		link(a, D_SOUTHEAST, b, D_NORTHWEST);
		link(b, D_SOUTHWEST, c, D_NORTHEAST);
		link(c, D_NORTH, a, D_SOUTH);

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
