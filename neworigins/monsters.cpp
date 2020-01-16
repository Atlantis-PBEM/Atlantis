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

  if (TurnNumber() >= 47) {
    // Transform into Void
    forlist(&regions) {
      ARegion *r = (ARegion *) elem;
      int d = getrandom(100);

      forlist (&r->objects) {
        Object *o = (Object *) elem;
        forlist (&o->units) {
          Unit *u = (Unit *) elem;
          forlist(&u->items) {
            Item *i = (Item *) elem;
            if (i->type == I_VFOR) {
              if (d > 90) {
                // Do transform region
                printf("\n\n TRANSFORM %d,%d,%d \n\n", r->xloc, r->yloc, r->zloc);
                r->development = 0;
                r->maxdevelopment = 0;
                r->habitat = 0;
                r->improvement = 0;
                r->type = R_VOID;
                r->town = NULL;
                r->SetName("Void");
                r->products.DeleteAll();
                r->SetupProds();
                r->markets.DeleteAll();
                r->population = 0;
                r->basepopulation = 0;
                r->wages = 0;
                r->maxwages = 0;
              }
              break;
            }
          }
        }
      }
    }
  }

  if (TurnNumber() >= 47) {
    Awrite("Running Spread Void...");
    int transform = 0;
    int monsters = 0;

    forlist(&regions) {
      ARegion *r = (ARegion *) elem;

      int vfor = 0;
      int avat = 0;
      forlist (&r->objects) {
        Object *o = (Object *) elem;
        forlist (&o->units) {
          Unit *u = (Unit *) elem;
          forlist(&u->items) {
            Item *i = (Item *) elem;
            if (i->type == I_VFOR) {
              vfor = 1;
            }
            if (i->type == I_AVAT) {
              printf("\n\n AVATAR FOUND \n\n");
              avat = 1;
            }
          }
        }
      }
      if (vfor == 0 && avat == 0) continue;

      int d = getrandom(100);
      if (d > 50 || avat == 1) {
        printf("\n\n CREATURES SPAWNED \n\n");
        Faction *mfac = GetFaction(&factions, monfaction);
        Unit *u = GetNewUnit(mfac, 0);
        u->MakeWMon("Creatures of the Void", I_NOOGLE, 1);
        u->MoveUnit(r->GetDummy());
        monsters++;
      }

      if (avat == 1 && r && r->type != R_VOID) {
        printf("\n\n AVATAR TRANSFORMS VOID \n\n");
        r->development = 0;
        r->maxdevelopment = 0;
        r->habitat = 0;
        r->improvement = 0;
        r->type = R_VOID;
        r->town = NULL;
        r->SetName("Void");
        r->products.DeleteAll();
        r->SetupProds();
        r->markets.DeleteAll();
        r->population = 0;
        r->basepopulation = 0;
        r->wages = 0;
        r->maxwages = 0;
      }

      Awrite("Found Void with Fortress...");
      for (int i=0; i<NDIRS; i++) {
        ARegion *r2 = r->neighbors[i];
        int d = getrandom(100);
        if (d > 90 && r2) {
          transform++;
          printf("\n\n TRANSFORM neighbor %d,%d,%d \n\n", r2->xloc, r2->yloc, r2->zloc);
          r2->development = 0;
          r2->maxdevelopment = 0;
          r2->habitat = 0;
          r2->improvement = 0;
          r2->type = R_VOID;
          r2->town = NULL;
          r2->SetName("Void");
          r2->products.DeleteAll();
          r2->SetupProds();
          r2->markets.DeleteAll();
          r2->population = 0;
          r2->basepopulation = 0;
          r2->wages = 0;
          r2->maxwages = 0;
        }
      }
    }
    if (transform > 0) {
      AString tmp = "Void spreads... ";
      tmp += "There are rumors about demigods, avatars ";
      tmp += "who posesses power to stop this world turn into dust.";
      WriteTimesArticle(tmp);
    }
    if (monsters > 0) {
      WriteTimesArticle("Creatures appeared from the Void...");
    }

    Faction *mfac = GetFaction(&factions, monfaction);
    mfac->SetAttitude(15, A_ALLY);
    mfac->SetAttitude(18, A_ALLY);
    mfac->SetAttitude(30, A_ALLY);
    mfac->SetAttitude(43, A_ALLY);
    mfac->SetAttitude(44, A_ALLY);
    mfac->SetAttitude(60, A_ALLY);

    // {
    //   int level = 2;
    //   int total = 0;
    //   ARegionArray *pArr = regions.pRegionArrays[level];

    //   for (int xsec = 0; xsec < pArr->x; xsec += 8)
    //   {
    //     for (int ysec = 0; ysec < pArr->y; ysec += 12)
    //     {
    //       int found = 0;

    //       for (int x = 0; x < 8; x++)
    //       {
    //         if (x + xsec > pArr->x || found == 1)
    //           break;

    //         for (int y = 0; y < 12; y += 2)
    //         {
    //           if (y + ysec > pArr->y)
    //             break;

    //           ARegion *reg = pArr->GetRegion(x + xsec, y + ysec + x % 2);
    //           int rand = getrandom(100);
    //           if (reg && reg->zloc == level && !reg->town && reg->type != R_OCEAN && rand < 30)
    //           {
    //             Faction *mfac = GetFaction(&factions, monfaction);
    //             Unit *u = GetNewUnit(mfac, 0);
    //             u->MakeWMon("Void Fortress", I_VFOR, 1);
    //             u->MoveUnit(reg->GetDummy());
    //             found = 1;
    //             total++;
    //             break;
    //           }
    //         }
    //       }
    //     }
    //   }

    //   printf("\n TOTAL VFORT: %d \n", total);
    //   AString tmp = "Giant portals opened undergroud and black piramids appered.";
    //   WriteTimesArticle(tmp);
    // }

  }

  printf("\n\n TURN : %d \n\n", TurnNumber());

  return;
}
