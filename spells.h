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
#ifdef GAME_SPELLS
//
// If GAME_SPELLS is defined, this is being included from inside the Game
// class in game.h
//

//
// Spell parsing - generic
//
void ProcessGenericSpell(Unit *, int, OrdersCheck *pCheck);
void ProcessRegionSpell(Unit *, AString *, int, OrdersCheck *pCheck);

//
// Spell parsing - specific
//
void ProcessCastGateLore(Unit *,AString *, OrdersCheck *pCheck );
void ProcessCastPortalLore(Unit *,AString *, OrdersCheck *pCheck );
void ProcessPhanBeasts(Unit *,AString *, OrdersCheck *pCheck );
void ProcessPhanUndead(Unit *,AString *, OrdersCheck *pCheck );
void ProcessPhanDemons(Unit *,AString *, OrdersCheck *pCheck );
void ProcessInvisibility(Unit *,AString *, OrdersCheck *pCheck );
void ProcessBirdLore(Unit *,AString *, OrdersCheck *pCheck );
void ProcessMindReading(Unit *,AString *, OrdersCheck *pCheck );
void ProcessLacandonTeleport(Unit *, AString *, OrdersCheck *pCheck);

//
// Spell helpers
//
int GetRegionInRange(ARegion *r, ARegion *tar, Unit *u, int spell);

//
// Spell running
//
void RunDetectGates(ARegion *,Object *,Unit *);
void RunFarsight(ARegion *,Unit *);
void RunGateJump(ARegion *,Object *,Unit *);
void RunTeleport(ARegion *,Object *,Unit *);
void RunLacandonTeleport(ARegion *, Object *, Unit *);
void RunPortalLore(ARegion *,Object *,Unit *);
void RunEarthLore(ARegion *,Unit *);
void RunWeatherLore(ARegion *, Unit *);
void RunClearSkies(ARegion *,Unit *);
void RunPhanBeasts(ARegion *,Unit *);
void RunPhanUndead(ARegion *,Unit *);
void RunPhanDemons(ARegion *,Unit *);
void RunInvisibility(ARegion *,Unit *);
void RunWolfLore(ARegion *,Unit *);
void RunBirdLore(ARegion *,Unit *);
void RunDragonLore(ARegion *,Unit *);
void RunSummonSkeletons(ARegion *,Unit *);
void RunRaiseUndead(ARegion *,Unit *);
void RunSummonLich(ARegion *,Unit *);
void RunSummonImps(ARegion *,Unit *);
void RunSummonDemon(ARegion *,Unit *);
void RunSummonBalrog(ARegion *,Unit *);
void RunCreateArtifact(ARegion *,Unit *,int,int);
void RunEngraveRunes(ARegion *,Object *,Unit *);
void RunConstructGate(ARegion *,Unit *);
void RunEnchantSwords(ARegion *,Unit *);
void RunEnchantArmor(ARegion *,Unit *);
void RunMindReading(ARegion *,Unit *);
#endif
