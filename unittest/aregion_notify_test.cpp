#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "unit.h"
#include "faction.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	ARegionList *makeRegions()
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surface = new ARegionArray(8, 8);
		surface->SetName("Surface");
		regs->pRegionArrays[1] = surface;
		return regs;
	}

	ARegion *surfaceRegion(ARegionList *regs, char const *name)
	{
		ARegion *r = new ARegion();
		r->SetName(name);
		r->type = R_PLAIN;
		r->SetLoc(2, 2, ARegionArray::LEVEL_SURFACE);
		regs->Add(r);
		return r;
	}

	Unit *addUnit(ARegion *reg, Object *o, int num, Faction *fac, int type = U_NORMAL)
	{
		Unit *u = new Unit(num, fac, 0);
		u->type = type;
		u->MoveUnit(o);
		return u;
	}

	int skillIndex(const char *abbr)
	{
		AString a(abbr);
		return LookupSkill(&a);
	}
}

ut::suite<"ARegion notify"> aregion_notify_suite = []
{
	using namespace ut;

	// --- CanMakeAdv -----------------------------------------------------------------------
	// I_IRON needs mining ("MINI") level 1. A faction with a suitably-skilled unit in the
	// region can make it; an unskilled unit (or a skilled unit of another faction) cannot.

	"CanMakeAdv is true when a faction unit has the item's skill"_test = []
	{
		ARegion *reg = new ARegion();
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *fac = new Faction(1);
		Unit *u = addUnit(reg, o, 100, fac);
		u->items.SetNum(I_LEADERS, 1);      // needs men for SetSkill to take
		u->SetSkill(skillIndex("MINI"), 1); // mining level 1

		expect(reg->CanMakeAdv(fac, I_IRON) == 1_i);
	};

	"CanMakeAdv is false without the skill"_test = []
	{
		ARegion *reg = new ARegion();
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *fac = new Faction(1);
		addUnit(reg, o, 100, fac); // no mining skill

		expect(reg->CanMakeAdv(fac, I_IRON) == 0_i);
	};

	"CanMakeAdv ignores a skilled unit of another faction"_test = []
	{
		ARegion *reg = new ARegion();
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *mine = new Faction(1);
		Faction *other = new Faction(2);
		Unit *u = addUnit(reg, o, 100, other);
		u->items.SetNum(I_LEADERS, 1);
		u->SetSkill(skillIndex("MINI"), 1);

		expect(reg->CanMakeAdv(mine, I_IRON) == 0_i) << "skill belongs to another faction";
	};

	// --- DefaultOrders --------------------------------------------------------------------
	// ARegion::DefaultOrders fans out to Unit::DefaultOrders for every unit; the effect is
	// type-specific, which lets us confirm each unit was visited.

	"DefaultOrders applies per-unit defaults across the region"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN; // not R_NEXUS, so normal units get a produce order
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *fac = new Faction(1);

		Unit *guard = addUnit(reg, o, 1, fac, U_GUARD);
		guard->guard = GUARD_NONE;
		Unit *worker = addUnit(reg, o, 2, fac, U_NORMAL);

		reg->DefaultOrders();

		expect(guard->guard == GUARD_SET) << "idle city guard is set to guard";
		expect(worker->monthorders != nullptr) << "normal unit gets a default produce order";
	};

	// --- NotifyCity -----------------------------------------------------------------------
	// NotifyCity events every OTHER faction present about a rename; the caster's own faction
	// is skipped.

	"NotifyCity notifies other factions but not the caster's"_test = []
	{
		ARegion *reg = new ARegion();
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *casterFac = new Faction(1);
		Faction *otherFac = new Faction(2);
		Unit *caster = addUnit(reg, o, 1, casterFac);
		addUnit(reg, o, 2, otherFac);

		AString oldn("Oldtown"), newn("Newtown");
		reg->NotifyCity(caster, oldn, newn);

		expect(otherFac->events.Num() == 1_i) << "other faction is notified";
		expect(casterFac->events.Num() == 0_i) << "caster's own faction is not notified";
		std::string ev = ((AString *)otherFac->events.First())->Str();
		expect(ev.find("renames") != std::string::npos) << ev;
	};

	// --- NotifySpell ----------------------------------------------------------------------
	// "weather lore" (WEAT) is a NOTIFY skill: NotifySpell events every other faction that
	// has a unit knowing it.

	"NotifySpell notifies factions holding the notifiable skill"_test = []
	{
		ARegionList *regs = makeRegions();
		ARegion *reg = surfaceRegion(regs, "Homeland");
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *casterFac = new Faction(1);
		Faction *watcherFac = new Faction(2);
		Unit *caster = addUnit(reg, o, 1, casterFac);

		Unit *watcher = addUnit(reg, o, 2, watcherFac);
		watcher->items.SetNum(I_LEADERS, 1);
		watcher->SetSkill(skillIndex("WEAT"), 1);

		int ret = reg->NotifySpell(caster, "WEAT", regs);

		expect(ret == 1_i) << "a NOTIFY skill always reports success";
		expect(watcherFac->events.Num() == 1_i) << "the skilled faction is notified";
		std::string ev = ((AString *)watcherFac->events.First())->Str();
		expect(ev.find("weather lore") != std::string::npos) << ev;
	};

	// No other faction knows the skill -> still returns 1 (notifiable), but no events fire.
	"NotifySpell sends no events when nobody holds the skill"_test = []
	{
		ARegionList *regs = makeRegions();
		ARegion *reg = surfaceRegion(regs, "Quietland");
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *casterFac = new Faction(1);
		Faction *watcherFac = new Faction(2);
		Unit *caster = addUnit(reg, o, 1, casterFac);
		addUnit(reg, o, 2, watcherFac); // present but unskilled

		int ret = reg->NotifySpell(caster, "WEAT", regs);
		expect(ret == 1_i);
		expect(watcherFac->events.Num() == 0_i) << "nobody knew the skill";
	};

	// A non-NOTIFY skill takes the other branch: it is not itself notifiable, so NotifySpell
	// only checks (non-notifiable) prerequisites and returns 0 without eventing anyone.
	// Mining ("MINI") is a plain, non-NOTIFY skill.
	"NotifySpell returns 0 for a non-notifiable skill"_test = []
	{
		ARegionList *regs = makeRegions();
		ARegion *reg = surfaceRegion(regs, "Mineland");
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *casterFac = new Faction(1);
		Faction *watcherFac = new Faction(2);
		Unit *caster = addUnit(reg, o, 1, casterFac);
		Unit *miner = addUnit(reg, o, 2, watcherFac);
		miner->items.SetNum(I_LEADERS, 1);
		miner->SetSkill(skillIndex("MINI"), 1);

		int ret = reg->NotifySpell(caster, "MINI", regs);
		expect(ret == 0_i) << "mining is not a NOTIFY skill";
		expect(watcherFac->events.Num() == 0_i) << "no notification for a non-NOTIFY skill";
	};

	// --- CanMakeAdv farsight paths --------------------------------------------------------
	// Besides present units, CanMakeAdv also consults the improved-farsight (farsees) and
	// transit-passer (passers) lists. Both are gated by Globals flags that are off in the
	// unittest ruleset, so we flip them for the test.

	"CanMakeAdv is true via an improved-farsight scout"_test = []
	{
		int saved = Globals->IMPROVED_FARSIGHT;
		Globals->IMPROVED_FARSIGHT = 1;

		ARegion *reg = new ARegion();
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *fac = new Faction(1);
		// A scout with the mining skill, seen via farsight (need not be in this region).
		Unit *scout = new Unit(200, fac, 0);
		scout->items.SetNum(I_LEADERS, 1);
		scout->SetSkill(skillIndex("MINI"), 1);
		Farsight *fs = new Farsight();
		fs->faction = fac;
		fs->unit = scout;
		reg->farsees.Add(fs);

		expect(reg->CanMakeAdv(fac, I_IRON) == 1_i) << "farsight scout's skill counts";

		Globals->IMPROVED_FARSIGHT = saved;
	};

	"CanMakeAdv is true via a transit passer"_test = []
	{
		int saved = Globals->TRANSIT_REPORT;
		Globals->TRANSIT_REPORT =
			GameDefs::REPORT_USE_UNIT_SKILLS | GameDefs::REPORT_SHOW_RESOURCES;

		ARegion *reg = new ARegion();
		Object *o = new Object(reg); o->type = O_DUMMY; reg->objects.Add(o);
		Faction *fac = new Faction(1);
		Unit *passer = new Unit(200, fac, 0);
		passer->items.SetNum(I_LEADERS, 1);
		passer->SetSkill(skillIndex("MINI"), 1);
		Farsight *fs = new Farsight();
		fs->faction = fac;
		fs->unit = passer;
		reg->passers.Add(fs);

		expect(reg->CanMakeAdv(fac, I_IRON) == 1_i) << "transit passer's skill counts";

		Globals->TRANSIT_REPORT = saved;
	};
};
