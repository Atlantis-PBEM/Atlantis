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
#include "orders.h"

char * od[] = {
  "#atlantis",
  "#end",
  "unit",
  "address",
  "advance",
  "assassinate",
  "attack",
  "autotax",
  "avoid",
  "behind",
  "build",
  "buy",
  "cast",
  "claim",
  "combat",
  "consume",
  "declare",
  "describe",
  "destroy",
  "end",
  "enter",
  "entertain",
  "faction",
  "find",
  "forget",
  "form",
  "give",
  "guard",
  "hold",
  "leave",
  "move",
  "name",
  "noaid",
  "nocross",
  "option",
  "password",
  "pillage",
  "produce",
  "promote",
  "quit",
  "restart",
  "reveal",
  "sail",
  "sell",
  "show",
  "steal",
  "study",
  "tax",
  "teach",
  "work"
};

char ** OrderStrs = od;

int Parse1Order(AString * token) {
  for (int i=0; i<NORDERS; i++)
    if (*token == OrderStrs[i]) return i;
  return -1;
}

Order::Order() {
  type = NORDERS;
}

Order::~Order() {
}

MoveOrder::MoveOrder() {
  type = O_MOVE;
}

MoveOrder::~MoveOrder() {
}

ForgetOrder::ForgetOrder() {
  type = O_FORGET;
}

ForgetOrder::~ForgetOrder() {
}

GiveOrder::GiveOrder() {
  type = O_GIVE;
}

GiveOrder::~GiveOrder() {
  delete target;
}

StudyOrder::StudyOrder() {
  type = O_STUDY;
}

StudyOrder::~StudyOrder() {
}

TeachOrder::TeachOrder() {
  type = O_TEACH;
}

TeachOrder::~TeachOrder() {
}

ProduceOrder::ProduceOrder() {
  type = O_PRODUCE;
}

ProduceOrder::~ProduceOrder() {
}

BuyOrder::BuyOrder() {
  type = O_BUY;
}

BuyOrder::~BuyOrder() {
}

SellOrder::SellOrder() {
  type = O_SELL;
}

SellOrder::~SellOrder() {
}

AttackOrder::AttackOrder() {
  type = O_ATTACK;
}

AttackOrder::~AttackOrder() {
}

BuildOrder::BuildOrder() {
  type = O_BUILD;
}

BuildOrder::~BuildOrder() {
}

SailOrder::SailOrder() {
  type = O_SAIL;
}

SailOrder::~SailOrder() {
}

FindOrder::FindOrder() {
  type = O_FIND;
}

FindOrder::~FindOrder() {
}

StealOrder::StealOrder() {
  type = O_STEAL;
}

StealOrder::~StealOrder() {
  if (target) delete target;
}

AssassinateOrder::AssassinateOrder() {
  type = O_ASSASSINATE;
}

AssassinateOrder::~AssassinateOrder() {
  if (target) delete target;
}

CastOrder::CastOrder() {
  type = O_CAST;
}

CastOrder::~CastOrder() {
}

CastMindOrder::CastMindOrder() {
  id = 0;
}

CastMindOrder::~CastMindOrder() {
  delete id;
}

TeleportOrder::TeleportOrder() {
}

TeleportOrder::~TeleportOrder() {
}

CastRegionOrder::CastRegionOrder() {
}

CastRegionOrder::~CastRegionOrder() {
}

CastIntOrder::CastIntOrder() {
}

CastIntOrder::~CastIntOrder() {
}

CastUnitsOrder::CastUnitsOrder() {
}

CastUnitsOrder::~CastUnitsOrder() {
}

