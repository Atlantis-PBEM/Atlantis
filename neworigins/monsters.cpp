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
// Date        Person            Comments
// ----        ------            --------
// 2001/Mar/03 Joseph Traub      Moved some of the monster stuff here
//
#include "ctime"
#include "gamedata.h"
#include "game.h"
#include "quests.h"

void Game::CreateVMons()
{
	if (!Globals->LAIR_MONSTERS_EXIST) return;

	return;
}

void Game::GrowVMons()
{
  if (!Globals->LAIR_MONSTERS_EXIST) return;

  std::time_t rawtime;
  struct tm * timeinfo;
  char month [60];
  char day [60];

  time (&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(month, 60, "%m", timeinfo);
  strftime(day, 60, "%d", timeinfo);

  long int day_num = strtol(day, NULL, 10);
  long int month_num = strtol(month, NULL, 10);

  // Halloween monsters
  if (month_num == 10 && day_num > 26) {
    int count = 0;
    Awrite("Running Halloween monsters...");
    forlist(&regions) {
      ARegion * r = (ARegion *) elem;
      if (r->type == R_OCEAN) continue;

      int spawn = getrandom(100);
      if (spawn > 10) continue;

      Faction *mfac = GetFaction(&factions, monfaction);
      Unit *u = GetNewUnit(mfac, 0);
      u->MakeWMon("Headless Horseman", I_HHOR, 1);
      u->MoveUnit(r->GetDummy());
      count++;
    }
    if (count > 0) {
      WriteTimesArticle("Headless Horseman has been spotted roaming around...");
    }
  }

  // Halloween monsters cleanup
  if (month_num == 11 && day_num < 7) {
    int count = 0;
    Awrite("Running Halloween monsters cleanup...");
    forlist(&regions) {
      ARegion *r = (ARegion *) elem;
      forlist (&r->objects) {
        Object *o = (Object *) elem;
        forlist (&o->units) {
          Unit *u = (Unit *) elem;
          int hh = 0;
          forlist(&u->items) {
            Item *i = (Item *) elem;
            if (i->type == I_HHOR) {
              hh = 1;
              break;
            }
          }
          if (hh == 1) {
            count++;
            u->items.SetNum(I_HHOR, 0);
          }
        }
      }
    }
    if (count > 0) {
      WriteTimesArticle("Headless Horsemen had disappeared, where did they go?...");
    }
  }

  if (TurnNumber() == 44) {
    int level = 1;
    int total = 0;
    Quest *q;
    Item *item;
    ARegionArray *pArr = regions.pRegionArrays[level];

		for (int xsec=0; xsec < pArr->x; xsec+=8) {
			for (int ysec=0; ysec < pArr->y; ysec+=12) {
        int found = 0;

				for (int x=0; x < 8; x++) {
					if (x+xsec > pArr->x || found == 1) break;

					for (int y=0; y < 12; y+=2) {
						if (y+ysec > pArr->y) break;

						ARegion *reg = pArr->GetRegion(x+xsec, y+ysec+x%2);
            int rand = getrandom(100);
						if (reg && reg->zloc == level && !reg->town && reg->type != R_OCEAN && rand < 30) {
              Faction *mfac = GetFaction(&factions, monfaction);
              Unit *u = GetNewUnit(mfac, 0);
              u->MakeWMon("Void Fortress", I_VFOR, 1);
              u->MoveUnit(reg->GetDummy());

              q = new Quest;
	            q->type = Quest::SLAY;
              q->times = 0;
              item = new Item;
              item->type = I_RELICOFGRACE;
              item->num = 2;
              q->rewards.Add(item);	
              q->target = u->num;

              total += 1;
              found = 1;
              break;
						}
					}
				}
			}
		}
    if (total > 0) {
      AString tmp = "Black pyramids descended from the sky. Hovering couple feets above the ground ";
      tmp += "they do nothing. In couple of days farmers noticed that ";
      tmp += "land around fortresses started slowly fade and transform into dust...";
      WriteTimesArticle(tmp);
    }
    printf("\n\n TOTAL VOID : %d \n\n", total);
  }

  printf("\n\n TURN : %d \n\n", TurnNumber());

  return;
}
