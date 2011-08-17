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

#include "quests.h"
#include "object.h"

QuestList quests;

Quest::Quest()
{
	type = -1;
	target = -1;
	objective.type = -1;
	objective.num = 0;
	building = -1;
	regionnum = -1;
	regionname = "-";
}

Quest::~Quest()
{
	rewards.DeleteAll();
}

int QuestList::ReadQuests(Ainfile *f)
{
        int count, rewards;
	Quest *quest;
	AString *name;
	Item *item;

	quests.DeleteAll();

        count = f->GetInt();
	if (count < 0)
		return 0;
	while (count-- > 0) {
		quest = new Quest();
		quest->type = f->GetInt();
		switch (quest->type) {
			case Quest::SLAY:
				quest->target = f->GetInt();
				break;
			case Quest::HARVEST:
				quest->objective.Readin(f);
				quest->regionnum = f->GetInt();
				break;
			default:
				quest->target = f->GetInt();
				quest->objective.Readin(f);
				name = f->GetStr();
				quest->building = LookupObject(name);
				delete name;
				quest->regionnum = f->GetInt();
				name = f->GetStr();
				quest->regionname = *name;
				delete name;
				break;
		}
		rewards = f->GetInt();
		while (rewards-- > 0) {
			item = new Item();
			item->Readin(f);
			if (-1 == item->type)
				return 0;
			quest->rewards.Add(item);
		}
		quests.Add(quest);
	}

        return 1;
}

void QuestList::WriteQuests(Aoutfile *f)
{
	Quest *q;
	Item *i;

        f->PutInt(quests.Num());
	forlist(this) {
		q = (Quest *) elem;
		f->PutInt(q->type);
		switch(q->type) {
			case Quest::SLAY:
				f->PutInt(q->target);
				break;
			case Quest::HARVEST:
				q->objective.Writeout(f);
				f->PutInt(q->regionnum);
				break;
			default:
				f->PutInt(q->target);
				q->objective.Writeout(f);
				if (q->building != -1)
					f->PutStr(ObjectDefs[q->building].name);
				else
					f->PutStr("NO_OBJECT");
				f->PutInt(q->regionnum);
				f->PutStr(q->regionname);
				break;
		}
		f->PutInt(q->rewards.Num());
		forlist(&q->rewards) {
			i = (Item *) elem;
			i->Writeout(f);
		}
	}

	f->PutInt(0);

        return;
}

int QuestList::CheckQuestKillTarget(Unit * u, ItemList *reward)
{
	Quest *q;
	Item *i;

	forlist(this) {
		q = (Quest *) elem;
		if (q->type == Quest::SLAY && q->target == u->num) {
			// This dead thing was the target of a quest!
			forlist (&q->rewards) {
				i = (Item *) elem;
				reward->SetNum(i->type, reward->GetNum(i->type) + i->num);
			}
			quests.Remove(q);
			delete q;
			return 1;
		}
	}

	return 0;
}

int QuestList::CheckQuestHarvestTarget(ARegion *r,
		int item, int harvested, int max,
		ItemList *reward)
{
	Quest *q;
	Item *i;

	forlist(this) {
		q = (Quest *) elem;
		if (q->type == Quest::HARVEST &&
				q->regionnum == r->num &&
				q->objective.type == item) {
			if (getrandom(max) < harvested) {
				forlist (&q->rewards) {
					i = (Item *) elem;
					reward->SetNum(i->type, reward->GetNum(i->type) + i->num);
				}
				quests.Remove(q);
				delete q;
				return 1;
			}
		}
	}

	return 0;
}

