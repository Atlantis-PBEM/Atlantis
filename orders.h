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
#ifndef ORDERS_CLASS
#define ORDERS_CLASS

#include "gamedefs.h"
#include "astring.h"
#include <list>
#include <memory>

// Forward declarations
class UnitId;

enum {
	O_ATLANTIS,
	O_END,
	O_UNIT,
	O_ADDRESS,
	O_ADVANCE,
	O_ANNIHILATE,
	O_ARMOR,
	O_ASSASSINATE,
	O_ATTACK,
	O_AUTOTAX,
	O_AVOID,
	O_BEHIND,
	O_BUILD,
	O_BUY,
	O_CAST,
	O_CLAIM,
	O_COMBAT,
	O_CONSUME,
	O_DECLARE,
	O_DESCRIBE,
	O_DESTROY,
	O_DISTRIBUTE,
	O_ENDFORM,
	O_ENDTURN,
	O_ENTER,
	O_ENTERTAIN,
	O_EVICT,
	O_EXCHANGE,
	O_FACTION,
	O_FIND,
	O_FORGET,
	O_FORM,
	O_GIVE,
	O_GUARD,
	O_HOLD,
	O_IDLE,
	O_JOIN,
	O_LEAVE,
	O_MOVE,
	O_NAME,
	O_NOAID,
	O_NOCROSS,
	O_NOSPOILS,
	O_OPTION,
	O_PASSWORD,
	O_PILLAGE,
	O_PREPARE,
	O_PRODUCE,
	O_PROMOTE,
	O_QUIT,
	O_RESTART,
	O_REVEAL,
	O_SACRIFICE,
	O_SAIL,
	O_SELL,
	O_SHARE,
	O_SHOW,
	O_SPOILS,
	O_STEAL,
	O_STUDY,
	O_TAKE,
	O_TAX,
	O_TEACH,
	O_TRANSPORT,
	O_TURN,
	O_WEAPON,
	O_WITHDRAW,
	O_WORK,
	NORDERS
};

enum {
	M_NONE,
	M_WALK,
	M_RIDE,
	M_FLY,
	M_SWIM,
	M_SAIL
};

#define MOVE_PAUSE 97
#define MOVE_IN 98
#define MOVE_OUT 99
/* Enter is MOVE_ENTER + num of object */
#define MOVE_ENTER 100


extern char const ** OrderStrs;

int Parse1Order(AString *);

class Order {
	public:
		Order();
		virtual ~Order();

		int type;
		int quiet;
};

class MoveDir {
public:
	MoveDir(int d) : dir(d) {}
	~MoveDir() {}
	inline int operator==(const MoveDir &d) { return dir == d.dir; }

	int dir;
};

class MoveOrder : public Order {
	public:
		MoveOrder();
		~MoveOrder();

		int advancing;
		std::vector<MoveDir> dirs;
};

class WithdrawOrder : public Order {
	public:
		WithdrawOrder();
		~WithdrawOrder();

		int item;
		int amount;
};

class GiveOrder : public Order {
	public:
		GiveOrder();
		~GiveOrder();

		int item;
		/* if amount == -1, transfer whole unit, -2 means all of item */
		int amount;
		int except;
		int unfinished;
		int merge;

		// This needs special handling because we sometimes make copies of give orders for fleets
		// and without doing this, we can end up with double frees.
		std::shared_ptr<UnitId> target;
};

class StudyOrder : public Order {
	public:
		StudyOrder();
		~StudyOrder();

		int skill;
		int days;
		int level;
};

class TeachOrder : public Order {
	public:
		TeachOrder();
		~TeachOrder();

		std::list<UnitId> targets;
};

class ProduceOrder : public Order {
	public:
		ProduceOrder();
		~ProduceOrder();

		int item;
		int skill; /* -1 for none */
		int productivity;
		int target;
};

class BuyOrder : public Order {
	public:
		BuyOrder();
		~BuyOrder();

		int item;
		int num;
};

class SellOrder : public Order {
	public:
		SellOrder();
		~SellOrder();

		int item;
		int num;
};

class AttackOrder : public Order {
	public:
		AttackOrder();
		~AttackOrder();

		std::list<UnitId> targets;
};

class BuildOrder : public Order {
	public:
		BuildOrder();
		~BuildOrder();

		std::shared_ptr<UnitId> target;
		int new_building;
		int needtocomplete;
};

class SailOrder : public Order {
	public:
		SailOrder();
		~SailOrder();

		std::vector<MoveDir> dirs;
};

class FindOrder : public Order {
	public:
		FindOrder();
		~FindOrder();

		int find;
};

class StealthOrder : public Order {
	public:
		StealthOrder();
		virtual ~StealthOrder();

		std::shared_ptr<UnitId> target;
};

class StealOrder : public StealthOrder {
	public:
		StealOrder();
		~StealOrder();

		int item;
};

class AssassinateOrder : public StealthOrder {
	public:
		AssassinateOrder();
		~AssassinateOrder();

		std::shared_ptr<UnitId> target;
};

class ForgetOrder : public Order {
	public:
		ForgetOrder();
		~ForgetOrder();

		int skill;
};

// Add class for exchange
class ExchangeOrder : public Order {
	public:
		ExchangeOrder();
		~ExchangeOrder();

		int giveItem;
		int giveAmount;
		int expectItem;
		int expectAmount;

		int exchangeStatus;

		std::shared_ptr<UnitId> target;
};

class TurnOrder : public Order {
	public:
		TurnOrder();
		~TurnOrder();
		int repeating;
		std::vector<std::string> turnOrders;
};

class CastOrder : public Order {
	public:
		CastOrder();
		virtual ~CastOrder();

		int spell;
		int level;
};

class CastMindOrder : public CastOrder {
	public:
		CastMindOrder();
		~CastMindOrder();

		std::shared_ptr<UnitId> id;
};

class CastRegionOrder : public CastOrder {
	public:
		CastRegionOrder();
		virtual ~CastRegionOrder();

		int xloc, yloc, zloc;
};

class TeleportOrder : public CastRegionOrder {
	public:
		TeleportOrder();
		~TeleportOrder();

		int gate;
		std::list<UnitId> units;
};

class CastIntOrder : public CastOrder {
	public:
		CastIntOrder();
		~CastIntOrder();

		int target;
};

class CastUnitsOrder : public CastOrder {
	public:
		CastUnitsOrder();
		~CastUnitsOrder();

		std::list<UnitId> units;
};

class CastTransmuteOrder : public CastOrder {
	public:
		CastTransmuteOrder();
		~CastTransmuteOrder();

		int item;
		int number;
};

class EvictOrder : public Order {
	public:
		EvictOrder();
		~EvictOrder();

		std::list<UnitId> targets;
};

class IdleOrder : public Order {
	public:
		IdleOrder();
		~IdleOrder();
};

class TransportOrder : public Order {
	public:
		TransportOrder();
		~TransportOrder();

		int item;
		// amount == -1 means all available at transport time
		// any other amount is also checked at transport time
		int amount;
		int except;
		int distance;

		enum class TransportPhase {
			SHIP_TO_QM,
			INTER_QM_TRANSPORT,
			DISTRIBUTE_FROM_QM
		};
		TransportPhase phase;

		std::shared_ptr<UnitId> target;
};

class JoinOrder : public Order {
	public:
		JoinOrder();
		~JoinOrder();

		std::shared_ptr<UnitId> target;
		int overload;
		int merge;
};

class AnnihilateOrder : public Order {
	public:
		AnnihilateOrder();
		~AnnihilateOrder();

		int xloc, yloc, zloc;
};

class SacrificeOrder : public Order {
	public:
		SacrificeOrder();
		~SacrificeOrder();

		int item;
		int amount;
};

#endif
