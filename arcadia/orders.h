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

class Order;
class AttackOrder;
class MoveOrder;
class WithdrawOrder;
class WishdrawOrder;
class WishskillOrder;
class MasteryOrder;
class GiveOrder;
class SendOrder;
class StudyOrder;
class TeachOrder;
class SellOrder;
class BuyOrder;
class ProduceOrder;
class BuildOrder;
class BuildHexsideOrder;
class SailOrder;
class FindOrder;
class StealOrder;
class AssassinateOrder;
class CastOrder;
class CastMindOrder;
class CastRegionOrder;
class TeleportOrder;
class ForgetOrder;
class EvictOrder;
class BankOrder;
class IdleOrder;
class TransportOrder;

#include "unit.h"
#include "gamedefs.h"
#include "astring.h"
#include "alist.h"

enum {
	O_ATLANTIS,
	O_END,
	O_UNIT,
	O_ADDRESS,
	O_ADVANCE,
	O_ARMOR,
	O_ASSASSINATE,
	O_ATTACK,
	O_AUTOTAX,
	O_AVOID,
	O_BANK,
	O_BEHIND,
	O_BUILD,
	O_BUILDHEXSIDE,
	O_BUY,
	O_CAST,
	O_CLAIM,
	O_COMBAT,
	O_COMMAND,
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
	O_FIGHTAS,
	O_FIND,
	O_FOLLOW,
	O_FORGET,
	O_FORM,
	O_GIVE,
	O_GUARD,
	O_HOLD,
	O_IDLE,
	O_LEAVE,
	O_MASTER,
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
	O_SAIL,
	O_SELL,
	O_SEND,
	O_SHARE,
	O_SHOW,
	O_SPOILS,
	O_STEAL,
	O_STUDY,
	O_TACTICS,
	O_TAX,
	O_TEACH,
	O_TRANSPORT,
	O_TURN,
	O_WEAPON,
	O_WISHDRAW,
	O_WISHSKILL,
	O_WITHDRAW,
	O_WORK,
	NORDERS
};

enum {
	M_NONE,
	M_WALK,
	M_RIDE,
	M_FLY,
	M_SAIL
};

#define MOVE_IN 98
#define MOVE_OUT 99
/* Enter is MOVE_ENTER + num of object */
#define MOVE_ENTER 100

extern char ** OrderStrs;

int Parse1Order(AString *);

class Order : public AListElem {
	public:
		Order();
		virtual ~Order();

		int type;
		int quiet;
};

class MoveDir : public AListElem {
	public:
		int dir;
};

class MoveOrder : public Order {
	public:
		MoveOrder();
		~MoveOrder();

		int advancing;
		AList dirs;
};

class WithdrawOrder : public Order {
	public:
		WithdrawOrder();
		~WithdrawOrder();

		int item;
		int amount;
};

class WishdrawOrder : public Order {
	public:
		WishdrawOrder();
		~WishdrawOrder();

		int item;
		int amount;
};

class WishskillOrder : public Order {
	public:
		WishskillOrder();
		~WishskillOrder();

		int skill;
		int knowledge;
		int experience;
};

class MasterOrder : public Order {
    public:
        MasterOrder();
        ~MasterOrder();
};

class FollowOrder : public Order {
	public:
		FollowOrder();
		~FollowOrder();

		int ship;
		UnitId *targetid;
		
		Unit *target;
		int dir;
		int advancing;
};

class GiveOrder : public Order {
	public:
		GiveOrder();
		~GiveOrder();

		int item;
		/* if amount == -1, transfer whole unit, -2 means all of item */
		int amount;
		int except;

		UnitId *target;
};

class SendOrder : public Order {
	public:
		SendOrder();
		~SendOrder();

		int item;
		/* if amount == -2, send all of item */
		int amount;
		int except;
		int direction;

		UnitId *target;
		UnitId *via;
};

class StudyOrder : public Order {
	public:
		StudyOrder();
		~StudyOrder();

		int skill;
		int days;
		int level;  //STUDY order mod
};

class TeachOrder : public Order {
	public:
		TeachOrder();
		~TeachOrder();

		AList targets;
};

class ProduceOrder : public Order {
	public:
		ProduceOrder();
		~ProduceOrder();

		int item;
		int skill; /* -1 for none */
		int productivity;
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

		AList targets;
};

class BuildOrder : public Order {
	public:
		BuildOrder();
		~BuildOrder();

		UnitId * target;
};

class BuildHexsideOrder : public Order {
	public:
		BuildHexsideOrder();
		~BuildHexsideOrder();

		int terrain;
		int direction;
};

class SailOrder : public Order {
	public:
		SailOrder();
		~SailOrder();

		AList dirs;
};

class FindOrder : public Order {
	public:
		FindOrder();
		~FindOrder();

		int find;
};

class StealOrder : public Order {
	public:
		StealOrder();
		~StealOrder();

		UnitId *target;
		int item;
};

class AssassinateOrder : public Order {
	public:
		AssassinateOrder();
		~AssassinateOrder();

		UnitId *target;
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

		UnitId *target;
};

class TurnOrder : public Order {
	public:
		TurnOrder();
		~TurnOrder();
		int repeating;
		AList turnOrders;
};

class CastOrder : public Order {
	public:
		CastOrder();
		~CastOrder();

		int spell;
		int level;
		int extracost; //first spell is 0, second is 1, third is 2.
};

class CastMindOrder : public CastOrder {
	public:
		CastMindOrder();
		~CastMindOrder();

		UnitId *id;
};

class CastRegionOrder : public CastOrder {
	public:
		CastRegionOrder();
		~CastRegionOrder();

		int xloc, yloc, zloc;
};

class TeleportOrder : public CastRegionOrder {
	public:
		TeleportOrder();
		~TeleportOrder();

		int gate;
		AList units;
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

		AList units;
};

class CastChangeOrder : public CastOrder {
	public:
		CastChangeOrder();
		~CastChangeOrder();

		int fromitem;
		int toitem;
		AList units;
};

class CastModifyOrder : public CastOrder {
	public:
		CastModifyOrder();
		~CastModifyOrder();

		int fromitem;
		int toitem;
		int xloc, yloc, zloc;
};

class CastMenOrder : public CastOrder {
    public:
        CastMenOrder();
        ~CastMenOrder();
        
        int men;
        int race;
        AList units;
};

class CastHypnosisOrder : public CastOrder {
    public:
        CastHypnosisOrder();
        ~CastHypnosisOrder();
        
        AList units;
        Order *monthorder;
        int taxing;
};

class EvictOrder : public Order {
	public:
		EvictOrder();
		~EvictOrder();

		AList targets;
};

class BankOrder : public Order {
	public:

		BankOrder();
		~BankOrder();

		int what; // 1 == withdraw; 2 == deposit
		int amount;
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
		int amount;
		int except;

		UnitId *target;
};

#endif
