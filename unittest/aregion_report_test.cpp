#include "external/boost/ut.hpp"

#include <cstdio>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "faction.h"
#include "fileio.h"
#include "production.h"
#include "market.h"
#include "object.h"
#include "unit.h"
#include "orders.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	// Build a region list with a nexus level (0) and a named surface level (1). ShortPrint,
	// Print and WriteExits all reach through pRegs->pRegionArrays[zloc] for the level name,
	// so any region we print must live on a level this list knows about.
	ARegionList *makeRegions(char const *surfaceName)
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surface = new ARegionArray(8, 8);
		surface->levelType = ARegionArray::LEVEL_SURFACE;
		surface->SetName(surfaceName); // null clears it
		regs->pRegionArrays[1] = surface;
		return regs;
	}

	ARegion *surfaceRegion(int x, int y, char const *name)
	{
		ARegion *r = new ARegion();
		r->SetName(name);
		r->type = R_PLAIN;
		r->SetLoc(x, y, ARegionArray::LEVEL_SURFACE);
		return r;
	}

	// Run a report-writing call against an Areport backed by a temp file, then return the
	// file's contents as a string. This is the general technique for unit-testing any of the
	// Write*/report methods: capture the emitted text and assert on it.
	std::string capture(std::function<void(Areport *)> emit)
	{
		const char *scratch = "aregion_report.tmp";
		std::remove(scratch);

		Areport rep;
		rep.OpenByName(scratch);
		emit(&rep);
		rep.Close();

		std::ifstream in(scratch);
		std::stringstream ss;
		ss << in.rdbuf();
		in.close();
		std::remove(scratch);
		return ss.str();
	}

	bool has(const std::string &haystack, const std::string &needle)
	{
		return haystack.find(needle) != std::string::npos;
	}

	Unit *addUnit(ARegion *reg, int num, Faction *fac)
	{
		Object *o = new Object(reg);
		o->type = O_DUMMY;
		reg->objects.Add(o);
		Unit *u = new Unit(num, fac, 0);
		u->MoveUnit(o);
		return u;
	}

	// A deterministic resource production (the ctor jitters `amount` under RANDOM_ECONOMY,
	// so we pin it).
	Production *resourceProd(int item, int amount)
	{
		Production *p = new Production(item, amount);
		p->amount = amount;
		p->productivity = amount;
		return p;
	}
}

ut::suite<"ARegion report"> aregion_report_suite = []
{
	using namespace ut;

	// ShortPrint returns "<terrain> (x,y,<level name>) in <region name>", folding in the
	// level name only when the level array has one. This is a plain AString, so no capture
	// is needed.
	"ShortPrint includes the level name when the level is named"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Testville");
		regs->Add(r);

		std::string out = r->ShortPrint(regs).Str();
		expect(eq(out, std::string("plain (2,2,Surface) in Testville")));
	};

	"ShortPrint omits the level name when the level is unnamed"_test = []
	{
		ARegionList *regs = makeRegions(nullptr); // no level name
		ARegion *r = surfaceRegion(3, 1, "Nowhere");
		regs->Add(r);

		std::string out = r->ShortPrint(regs).Str();
		expect(eq(out, std::string("plain (3,1) in Nowhere")));
	};

	// Print == ShortPrint for a region with no town (the town clause is appended only when a
	// TownInfo is present).
	"Print equals ShortPrint for a townless region"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Testville");
		regs->Add(r);

		std::string shortForm = r->ShortPrint(regs).Str();
		std::string longForm = r->Print(regs).Str();
		expect(eq(longForm, shortForm)) << "no town -> Print adds nothing";
	};

	// WriteEconomy emits the wage line, then markets, then products. On an empty region with
	// present == 1 the wage is $0 and there are no products.
	"WriteEconomy reports zero wages and no products for an empty region"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "Testville");
		Faction *fac = new Faction(1);

		std::string out = capture([&](Areport *rep) {
			r->WriteEconomy(rep, fac, /*present*/ 1);
		});

		expect(has(out, "Wages: $0.")) << "empty region pays no wages\n" << out;
		expect(has(out, "Products: none.")) << "empty region makes nothing\n" << out;
	};

	// WagesForReport formats the region's silver production as "$<prod/10>.<prod%10> (Max:
	// $<amount>)". The expected string is taken verbatim from a real NewOrigins report
	// (snapshot-tests/neworigins_turns/turn_0/report.1) to keep the format honest.
	"WagesForReport formats the silver production and its cap"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "Testville");
		// GetProd(I_SILVER, -1) matches itemtype AND skill, so the wage production's skill
		// must be -1 (as the engine sets it for the base wage production). NOTE: the
		// Production ctor bumps `amount` by getrandom() when RANDOM_ECONOMY is on, so we set
		// the reported fields explicitly to keep the expectation deterministic.
		Production *wage = new Production(I_SILVER, 595);
		wage->amount = 595;       // max wage pool (override the RANDOM_ECONOMY jitter)
		wage->productivity = 135; // -> $13.5
		wage->skill = -1;
		r->products.Add(wage);

		std::string out = r->WagesForReport().Str();
		expect(eq(out, std::string("$13.5 (Max: $595)")));
	};

	// WriteExits lists each neighbor whose exit has been seen; with none seen it prints
	// "none". The listed neighbor is rendered via its own Print(pRegs).
	"WriteExits lists a seen neighbor and prints none otherwise"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *center = surfaceRegion(2, 2, "Center");
		ARegion *north = surfaceRegion(2, 0, "Northshire");
		regs->Add(center);
		regs->Add(north);
		center->neighbors[D_NORTH] = north;

		int seen[NDIRS];
		for (int i = 0; i < NDIRS; i++) seen[i] = 0;
		seen[D_NORTH] = 1;

		std::string shown = capture([&](Areport *rep) {
			center->WriteExits(rep, regs, seen);
		});
		expect(has(shown, "Exits:")) << shown;
		expect(has(shown, "plain (2,0,Surface) in Northshire")) << shown;

		// Nothing seen -> "none".
		int none[NDIRS];
		for (int i = 0; i < NDIRS; i++) none[i] = 0;
		std::string empty = capture([&](Areport *rep) {
			center->WriteExits(rep, regs, none);
		});
		expect(has(empty, "none")) << empty;
	};

	// --- WriteProducts: every branch ------------------------------------------------------

	"WriteProducts prints none for an empty region"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		std::string out = capture([&](Areport *rep){ r->WriteProducts(rep, fac, 1); });
		expect(has(out, "Products: none.")) << out;
	};

	"WriteProducts lists a basic resource when present"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		r->products.Add(resourceProd(I_IRON, 20));
		std::string out = capture([&](Areport *rep){ r->WriteProducts(rep, fac, 1); });
		expect(has(out, "iron")) << "resource listed when present\n" << out;
		expect(!has(out, "Products: none.")) << out;
	};

	// The silver+entertainment production is a special branch: it prints its own line.
	"WriteProducts prints entertainment separately"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		Production *ent = new Production(I_SILVER, 77);
		ent->amount = 77;
		ent->skill = S_ENTERTAINMENT;
		r->products.Add(ent);
		std::string out = capture([&](Areport *rep){ r->WriteProducts(rep, fac, 1); });
		expect(has(out, "Entertainment available: $77.")) << out;
	};

	// Not present and no REPORT_SHOW_RESOURCES: the resource is skipped -> "none".
	"WriteProducts hides resources in a transit report"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		r->products.Add(resourceProd(I_IRON, 20));
		std::string out = capture([&](Areport *rep){ r->WriteProducts(rep, fac, 0); });
		expect(has(out, "Products: none.")) << "resource hidden when not present\n" << out;
	};

	// --- WriteMarkets: every branch -------------------------------------------------------

	"WriteMarkets prints none for an empty region"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		std::string out = capture([&](Areport *rep){ r->WriteMarkets(rep, fac, 1); });
		expect(has(out, "Wanted: none.")) << out;
		expect(has(out, "For Sale: none.")) << out;
	};

	"WriteMarkets lists a sell market under Wanted"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		r->markets.Add(new Market(M_SELL, I_IRON, 10, 5, 0, 10000, 0, 100));
		std::string out = capture([&](Areport *rep){ r->WriteMarkets(rep, fac, 1); });
		expect(has(out, "iron")) << "sell market listed\n" << out;
		expect(has(out, "For Sale: none.")) << "no buy markets\n" << out;
	};

	"WriteMarkets lists a buy market under For Sale"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		r->markets.Add(new Market(M_BUY, I_IRON, 8, 3, 0, 10000, 0, 100));
		std::string out = capture([&](Areport *rep){ r->WriteMarkets(rep, fac, 1); });
		expect(has(out, "Wanted: none.")) << "no sell markets\n" << out;
		expect(has(out, "For Sale: ")) << out;
		// The item appears in the For Sale section.
		expect(has(out, "iron")) << out;
	};

	"WriteMarkets hides markets in a transit report"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Faction *fac = new Faction(1);
		r->markets.Add(new Market(M_SELL, I_IRON, 10, 5, 0, 10000, 0, 100));
		std::string out = capture([&](Areport *rep){ r->WriteMarkets(rep, fac, 0); });
		expect(has(out, "Wanted: none.")) << "market hidden when not present\n" << out;
	};

	// --- WriteReport: presence / population / nexus branches ------------------------------

	// No unit of the faction present and no farsight -> nothing is written at all.
	"WriteReport writes nothing when the faction cannot see the region"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Hidden");
		regs->Add(r);
		Faction *fac = new Faction(1);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(out.empty()) << "invisible region produces no report\n" << out;
	};

	// A present faction sees the full block: header, peasants + race + wealth, economy, exits.
	"WriteReport writes the full block for a present, populated region"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		r->population = 100;
		r->race = I_LEADERS; // RACES_EXIST is on, so race must be valid
		r->wealth = 500;
		regs->Add(r);
		Faction *fac = new Faction(1);
		addUnit(r, 100, fac); // makes the faction present

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "plain (2,2,Surface) in Homeland")) << out;
		expect(has(out, "100 peasants")) << out;
		expect(has(out, "$500")) << out;
		expect(has(out, "Wages:")) << out;
		expect(has(out, "Exits:")) << out;
	};

	// Present but unpopulated: the peasants clause is omitted, economy/exits still print.
	"WriteReport omits peasants when population is zero"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Empty");
		r->population = 0;
		regs->Add(r);
		Faction *fac = new Faction(1);
		addUnit(r, 100, fac);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(!has(out, "peasants")) << "no peasants line for population 0\n" << out;
		expect(has(out, "Wages:")) << out;
	};

	// The R_NEXUS branch emits the Atlantis nexus description block.
	"WriteReport emits the nexus description for a Nexus region"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = new ARegion();
		r->SetName("The Void");
		r->type = R_NEXUS;
		r->SetLoc(0, 0, ARegionArray::LEVEL_NEXUS); // nexus level (index 0)
		regs->Add(r);
		Faction *fac = new Faction(1);
		addUnit(r, 100, fac);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "nexus")) << "nexus terrain named in the header\n" << out;
		expect(has(out, "Atlantis")) << "nexus description mentions the world name\n" << out;
	};

	// --- WriteTemplate: header / no-header branches ---------------------------------------

	// No unit of the faction -> the header is never emitted, output is empty.
	"WriteTemplate writes nothing without a unit of the faction"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "R");
		regs->Add(r);
		Faction *fac = new Faction(1);

		std::string out = capture([&](Areport *rep){ r->WriteTemplate(rep, fac, regs, 0); });
		expect(out.empty()) << out;
	};

	// A unit of the faction with a LONG template -> region header + per-unit line.
	"WriteTemplate writes a region header and unit line"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		regs->Add(r);
		Faction *fac = new Faction(1);
		fac->temformat = TEMPLATE_LONG;
		addUnit(r, 42, fac);

		std::string out = capture([&](Areport *rep){ r->WriteTemplate(rep, fac, regs, 0); });
		expect(has(out, "Homeland")) << "region header printed\n" << out;
		expect(has(out, "unit 42")) << "unit line printed\n" << out;
	};

	// --- advanced-item branches -----------------------------------------------------------

	// An advanced product (mithril) is listed only when the faction can make it -- i.e. has a
	// unit skilled enough (CanMakeAdv). Mithril needs mining level 3.
	"WriteProducts lists an advanced product only for a capable faction"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Object *o = new Object(r); o->type = O_DUMMY; r->objects.Add(o);
		Faction *fac = new Faction(1);
		r->products.Add(resourceProd(I_MITHRIL, 10));

		// Without the skill, the advanced product is hidden.
		std::string hidden = capture([&](Areport *rep){ r->WriteProducts(rep, fac, 1); });
		expect(has(hidden, "Products: none.")) << "advanced product hidden\n" << hidden;

		// Give a unit mining level 3 -> CanMakeAdv -> the product appears.
		Unit *miner = new Unit(100, fac, 0);
		miner->MoveUnit(o);
		miner->items.SetNum(I_LEADERS, 1);
		miner->SetSkill(LookupSkill(new AString("MINI")), 3);
		std::string shown = capture([&](Areport *rep){ r->WriteProducts(rep, fac, 1); });
		expect(has(shown, "mithril")) << "advanced product shown to a capable faction\n" << shown;
	};

	// An advanced sell-market is hidden unless MARKETS_SHOW_ADVANCED_ITEMS (off here) or the
	// faction already holds the item. We give the faction the item so HasItem admits it.
	"WriteMarkets shows an advanced market when the faction holds the item"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Object *o = new Object(r); o->type = O_DUMMY; r->objects.Add(o);
		Faction *fac = new Faction(1);
		r->markets.Add(new Market(M_SELL, I_MITHRIL, 100, 3, 0, 10000, 0, 100));

		std::string hidden = capture([&](Areport *rep){ r->WriteMarkets(rep, fac, 1); });
		expect(has(hidden, "Wanted: none.")) << "advanced market hidden without the item\n" << hidden;

		Unit *u = new Unit(100, fac, 0);
		u->MoveUnit(o);
		u->items.SetNum(I_MITHRIL, 1); // HasItem -> market becomes visible
		std::string shown = capture([&](Areport *rep){ r->WriteMarkets(rep, fac, 1); });
		expect(has(shown, "mithril")) << "advanced market shown when holding the item\n" << shown;
	};

	// The weather paragraph of WriteReport is gated by WEATHER_EXISTS (off in the unittest
	// ruleset). Flip it to reach the "It was ... last month" block.
	"WriteReport prints a weather paragraph when weather exists"_test = []
	{
		int saved = Globals->WEATHER_EXISTS;
		Globals->WEATHER_EXISTS = 1;

		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		r->weather = W_NORMAL;
		regs->Add(r);
		Faction *fac = new Faction(1);
		addUnit(r, 100, fac);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "last month")) << "weather paragraph emitted\n" << out;
		expect(has(out, "next month")) << out;

		Globals->WEATHER_EXISTS = saved;
	};

	// --- WriteReport gate display ---------------------------------------------------------

	// With GATES_EXIST and an open gate, a faction that can see it (a present unit with gate
	// lore) gets the "There is a Gate here" line.
	"WriteReport shows an open gate to a gate-lore unit"_test = []
	{
		int saved = Globals->GATES_EXIST;
		Globals->GATES_EXIST = 1;

		ARegionList *regs = makeRegions("Surface");
		regs->numberofgates = 12;
		ARegion *r = surfaceRegion(2, 2, "Portland");
		r->gate = 5;
		r->gateopen = 1;
		regs->Add(r);
		Faction *fac = new Faction(1);
		Unit *u = addUnit(r, 100, fac);
		u->items.SetNum(I_LEADERS, 1);
		u->SetSkill(S_GATE_LORE, 1); // lets the faction see the gate

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "There is a Gate here (Gate 5")) << out;

		Globals->GATES_EXIST = saved;
	};

	// A closed gate is shown only when SHOW_CLOSED_GATES is enabled.
	"WriteReport shows a closed gate when configured"_test = []
	{
		int savedG = Globals->GATES_EXIST;
		int savedC = Globals->SHOW_CLOSED_GATES;
		Globals->GATES_EXIST = 1;
		Globals->SHOW_CLOSED_GATES = 1;

		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Portland");
		r->gate = 5;
		r->gateopen = 0; // closed
		regs->Add(r);
		Faction *fac = new Faction(1);
		Unit *u = addUnit(r, 100, fac);
		u->items.SetNum(I_LEADERS, 1);
		u->SetSkill(S_GATE_LORE, 1);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "There is a closed Gate here.")) << out;

		Globals->SHOW_CLOSED_GATES = savedC;
		Globals->GATES_EXIST = savedG;
	};

	// --- WriteTemplate order content ------------------------------------------------------

	// Old month-long orders are echoed, and (because a month order is present) the unit's
	// turn-order block is wrapped in TURN/ENDTURN.
	"WriteTemplate echoes old orders and wraps a turn block"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		regs->Add(r);
		Faction *fac = new Faction(1);
		fac->temformat = TEMPLATE_LONG;
		Unit *u = addUnit(r, 42, fac);
		u->oldorders.Add(new AString("work")); // O_WORK -> a month order

		TurnOrder *t = new TurnOrder();
		t->repeating = 0;
		t->turnOrders.Add(new AString("build"));
		u->turnorders.Add(t);

		std::string out = capture([&](Areport *rep){ r->WriteTemplate(rep, fac, regs, 0); });
		expect(has(out, "work")) << "old order echoed\n" << out;
		expect(has(out, "TURN")) << "turn block opened\n" << out;
		expect(has(out, "build")) << "turn order line written\n" << out;
		expect(has(out, "ENDTURN")) << "turn block closed\n" << out;
	};

	// A repeating turn order with NO month order is emitted as an @TURN block.
	"WriteTemplate emits @TURN for a repeating order without a month order"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		regs->Add(r);
		Faction *fac = new Faction(1);
		fac->temformat = TEMPLATE_LONG;
		Unit *u = addUnit(r, 42, fac);

		TurnOrder *t = new TurnOrder();
		t->repeating = 1;
		t->turnOrders.Add(new AString("move 1"));
		u->turnorders.Add(t);

		std::string out = capture([&](Areport *rep){ r->WriteTemplate(rep, fac, regs, 0); });
		expect(has(out, "@TURN")) << "repeating order without a month order\n" << out;
	};

	// The O_TAX case of the month-order switch only counts as a month order when
	// TAX_PILLAGE_MONTH_LONG is set -- which then wraps the turn block.
	"WriteTemplate treats tax as a month order under TAX_PILLAGE_MONTH_LONG"_test = []
	{
		int saved = Globals->TAX_PILLAGE_MONTH_LONG;
		Globals->TAX_PILLAGE_MONTH_LONG = 1;

		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		regs->Add(r);
		Faction *fac = new Faction(1);
		fac->temformat = TEMPLATE_LONG;
		Unit *u = addUnit(r, 42, fac);
		u->oldorders.Add(new AString("tax"));

		TurnOrder *t = new TurnOrder();
		t->repeating = 0;
		t->turnOrders.Add(new AString("build"));
		u->turnorders.Add(t);

		std::string out = capture([&](Areport *rep){ r->WriteTemplate(rep, fac, regs, 0); });
		expect(has(out, "tax")) << out;
		expect(has(out, "ENDTURN")) << "tax counted as a month order -> wrapped block\n" << out;

		Globals->TAX_PILLAGE_MONTH_LONG = saved;
	};

	// The TEMPLATE_MAP format routes through WriteTemplateHeader instead of the plain header.
	"WriteTemplate uses the map header for TEMPLATE_MAP"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		regs->Add(r);
		Faction *fac = new Faction(1);
		fac->temformat = TEMPLATE_MAP;
		addUnit(r, 42, fac);

		std::string out = capture([&](Areport *rep){ r->WriteTemplate(rep, fac, regs, 0); });
		expect(has(out, "plain (2,2,Surface) in Homeland")) << "map header includes the print\n" << out;
		expect(has(out, "unit 42")) << out;
	};

	// An NPC faction lists advanced products without needing the making skill (the IsNPC
	// alternative to CanMakeAdv). A faction is NPC when a required type is -1.
	"WriteProducts lists advanced products for an NPC faction"_test = []
	{
		ARegion *r = surfaceRegion(2, 2, "R");
		Object *o = new Object(r); o->type = O_DUMMY; r->objects.Add(o);
		Faction *npc = new Faction(1);
		npc->type[F_WAR] = -1; // makes IsNPC() true
		expect(fatal(npc->IsNPC() == 1_i));
		r->products.Add(resourceProd(I_MITHRIL, 10));

		std::string out = capture([&](Areport *rep){ r->WriteProducts(rep, npc, 1); });
		expect(has(out, "mithril")) << "NPC sees the advanced product without the skill\n" << out;
	};

	// A present unit with mind reading (> 1) sets detfac, which flows into the unit-report
	// section. We exercise that branch and confirm the report is still produced.
	"WriteReport runs the detfac (mind reading) branch"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Homeland");
		regs->Add(r);
		Faction *fac = new Faction(1);
		Unit *u = addUnit(r, 100, fac);
		u->items.SetNum(I_LEADERS, 1);
		u->SetSkill(S_MIND_READING, 2); // > 1 -> detfac = 1

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "plain (2,2,Surface) in Homeland")) << "report produced with detfac set\n" << out;
	};

	// A region seen only through improved farsight (no present unit) still shows its gate when
	// the watching unit knows gate lore -- the farsight branch of the gate-visibility logic.
	"WriteReport shows a gate via a farsight gate-lore watcher"_test = []
	{
		int savedG = Globals->GATES_EXIST;
		int savedF = Globals->IMPROVED_FARSIGHT;
		Globals->GATES_EXIST = 1;
		Globals->IMPROVED_FARSIGHT = 1;

		ARegionList *regs = makeRegions("Surface");
		regs->numberofgates = 4;
		ARegion *r = surfaceRegion(2, 2, "Farland");
		r->gate = 3;
		r->gateopen = 1;
		regs->Add(r);
		Faction *fac = new Faction(1); // NOT present in the region

		// A farsight watcher of this faction whose unit knows gate lore.
		Unit *scout = new Unit(200, fac, 0);
		scout->items.SetNum(I_LEADERS, 1);
		scout->SetSkill(S_GATE_LORE, 1);
		Farsight *fs = new Farsight();
		fs->faction = fac;
		fs->unit = scout;
		r->farsees.Add(fs);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "There is a Gate here (Gate 3")) << "gate shown via farsight lore\n" << out;

		Globals->IMPROVED_FARSIGHT = savedF;
		Globals->GATES_EXIST = savedG;
	};

	// --- WriteReport transit / farsight-only display variants -----------------------------
	// These cover the "region seen without a present unit" paths: the report body is entered
	// via a passers/farsees Farsight, and the display is driven by TRANSIT_REPORT flags.

	// Transit "used exits": with REPORT_SHOW_USED_EXITS and a passer that recorded a used
	// exit, only that exit is shown.
	"WriteReport shows only used exits in a transit report"_test = []
	{
		int saved = Globals->TRANSIT_REPORT;
		Globals->TRANSIT_REPORT = GameDefs::REPORT_SHOW_USED_EXITS;

		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Center");
		ARegion *north = surfaceRegion(2, 0, "Northshire");
		regs->Add(r);
		regs->Add(north);
		r->neighbors[D_NORTH] = north;

		Faction *fac = new Faction(1); // NOT present in r
		Unit *scout = new Unit(200, fac, 0);
		Farsight *pass = new Farsight();
		pass->faction = fac;
		pass->unit = scout;
		pass->exits_used[D_NORTH] = 1; // it came through the north exit
		r->passers.Add(pass);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "Northshire")) << "the used north exit is shown\n" << out;

		Globals->TRANSIT_REPORT = saved;
	};

	// Farsight-only economy: a region seen through farsight (not present) still shows its
	// peasants and its real wealth (the `farsight` disjuncts in the peasant/money conditions).
	"WriteReport shows peasants and wealth via farsight"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Farview");
		r->population = 200;
		r->race = I_LEADERS;
		r->wealth = 700;
		regs->Add(r);

		Faction *fac = new Faction(1); // NOT present
		Unit *scout = new Unit(200, fac, 0);
		Farsight *fs = new Farsight();
		fs->faction = fac;
		fs->unit = scout;
		r->farsees.Add(fs);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "200 peasants")) << out;
		expect(has(out, "$700")) << "farsight reveals real wealth\n" << out;
	};

	// Transit economy: with REPORT_SHOW_PEASANTS but not REPORT_SHOW_REGION_MONEY, a passer
	// sees peasants but the money is masked as $0 (the else branch of the money condition).
	"WriteReport masks region money as $0 in a transit report"_test = []
	{
		int saved = Globals->TRANSIT_REPORT;
		Globals->TRANSIT_REPORT = GameDefs::REPORT_SHOW_PEASANTS;

		ARegionList *regs = makeRegions("Surface");
		ARegion *r = surfaceRegion(2, 2, "Transitville");
		r->population = 200;
		r->race = I_LEADERS;
		r->wealth = 700;
		regs->Add(r);

		Faction *fac = new Faction(1); // NOT present, NOT farsight
		Unit *scout = new Unit(200, fac, 0);
		Farsight *pass = new Farsight();
		pass->faction = fac;
		pass->unit = scout;
		r->passers.Add(pass);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "200 peasants")) << out;
		expect(has(out, "$0")) << "transit report masks the money\n" << out;
		expect(!has(out, "$700")) << "real wealth is hidden\n" << out;

		Globals->TRANSIT_REPORT = saved;
	};

	// Transit gate lore: a passer whose unit knows gate lore sees the gate, via the
	// REPORT_USE_UNIT_SKILLS passer branch of the gate-visibility logic.
	"WriteReport shows a gate via a transit passer with gate lore"_test = []
	{
		int savedG = Globals->GATES_EXIST;
		int savedT = Globals->TRANSIT_REPORT;
		Globals->GATES_EXIST = 1;
		Globals->TRANSIT_REPORT = GameDefs::REPORT_USE_UNIT_SKILLS;

		ARegionList *regs = makeRegions("Surface");
		regs->numberofgates = 8;
		ARegion *r = surfaceRegion(2, 2, "Gateway");
		r->gate = 6;
		r->gateopen = 1;
		regs->Add(r);

		Faction *fac = new Faction(1); // NOT present
		Unit *scout = new Unit(200, fac, 0);
		scout->items.SetNum(I_LEADERS, 1);
		scout->SetSkill(S_GATE_LORE, 1);
		Farsight *pass = new Farsight();
		pass->faction = fac;
		pass->unit = scout;
		r->passers.Add(pass);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(has(out, "There is a Gate here (Gate 6")) << "gate shown via transit gate lore\n" << out;

		Globals->TRANSIT_REPORT = savedT;
		Globals->GATES_EXIST = savedG;
	};
};
