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
  if (month_num == 10 && day_num > 20) {
    int count = 0;
    Awrite("Running Halloween monsters...");
    forlist(&regions) {
      ARegion * r = (ARegion *) elem;
      if (r->type == R_OCEAN) continue;

      int spawn = getrandom(100);
      if (spawn > 15) continue;

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

	return;
}
