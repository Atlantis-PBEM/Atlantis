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
void ProcessGenericSpell(Unit *u, int spell, orders_check *checker);
void ProcessRegionSpell(Unit *u, int spell, parser::string_parser& parser, orders_check *checker);

//
// Spell parsing - specific
//
void ProcessCastGateLore(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessCastPortalLore(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessPhanBeasts(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessPhanUndead(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessPhanDemons(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessInvisibility(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessBirdLore(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessMindReading(Unit *u, parser::string_parser& parser, orders_check *checker);
void ProcessTransmutation(Unit *u, parser::string_parser& parser, orders_check *checker);

//
// Spell helpers
//
int GetRegionInRange(ARegion *r, ARegion *tar, Unit *u, int spell);

//
// Spell running
//
int RunDetectGates(ARegion *,Object *,Unit *);
int RunFarsight(ARegion *,Unit *);
int RunGateJump(ARegion *,Object *,Unit *);
int RunTeleport(ARegion *,Object *,Unit *);
int RunLacandonTeleport(ARegion *, Object *, Unit *);
int RunPortalLore(ARegion *,Object *,Unit *);
int RunEarthLore(ARegion *,Unit *);
int RunPhantasmalEntertainment(ARegion *,Unit *);
int RunWeatherLore(ARegion *, Unit *);
int RunClearSkies(ARegion *,Unit *);
int RunPhanBeasts(ARegion *,Unit *);
int RunPhanUndead(ARegion *,Unit *);
int RunPhanDemons(ARegion *,Unit *);
int RunInvisibility(ARegion *,Unit *);
int RunWolfLore(ARegion *,Unit *);
int RunBirdLore(ARegion *,Unit *);
int RunDragonLore(ARegion *,Unit *);
int RunSummonSkeletons(ARegion *,Unit *);
int RunRaiseUndead(ARegion *,Unit *);
int RunSummonLich(ARegion *,Unit *);
int RunSummonImps(ARegion *,Unit *);
int RunSummonDemon(ARegion *,Unit *);
int RunSummonBalrog(ARegion *,Unit *);
int RunCreateArtifact(ARegion *,Unit *,int,int);
int RunEngraveRunes(ARegion *,Object *,Unit *);
int RunConstructGate(ARegion *,Unit *,int);
int RunEnchant(ARegion *,Unit *, int, int);
int RunMindReading(ARegion *,Unit *);
int RunTransmutation(ARegion *,Unit *);
int RunBlasphemousRitual(ARegion *,Unit *);
#endif
