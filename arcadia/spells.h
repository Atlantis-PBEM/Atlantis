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
void ProcessGenericSpell(Unit *, int, OrdersCheck *pCheck, int isquiet);
void ProcessRegionSpell(Unit *, AString *, int, OrdersCheck *pCheck, int isquiet);

//
// Spell parsing - specific
//
void ProcessCastGateLore(Unit *,AString *, OrdersCheck *pCheck, int isquiet );
void ProcessCastPortalLore(Unit *,AString *, OrdersCheck *pCheck, int isquiet );
void ProcessPhanBeasts(Unit *,AString *, OrdersCheck *pCheck, int isquiet );
void ProcessPhanUndead(Unit *,AString *, OrdersCheck *pCheck, int isquiet );
void ProcessPhanDemons(Unit *,AString *, OrdersCheck *pCheck, int isquiet );
void ProcessBirdLore(Unit *,AString *, OrdersCheck *pCheck, int isquiet );
void ProcessMindReading(Unit *,AString *, OrdersCheck *pCheck, int isquiet );
//Arcadia
void ProcessSummonCreaturesSpell(Unit *, AString *, int spell, OrdersCheck *pCheck, int isquiet); //wolf, imp, demon, undead
void ProcessSummonMen(Unit *, AString *, OrdersCheck *pCheck, int isquiet); //SummonMen
void ProcessUnitsSpell(Unit *,AString *, int spell, OrdersCheck *pCheck, int isquiet ); //Invis, SummonSpirit
void ProcessChangeSpell(Unit *,AString *, int spell, OrdersCheck *pCheck, int isquiet ); //Transmutation
void ProcessModificationSpell(Unit *,AString *, OrdersCheck *pCheck, int isquiet ); //Modification
void ProcessRejuvenationSpell(Unit *,AString *, OrdersCheck *pCheck, int isquiet ); //Rejuvenation
void ProcessArtifactSpell(Unit *,AString *, OrdersCheck *pCheck, int isquiet ); //Artifact_Lore
void ProcessHypnosisSpell(Unit *,AString *, OrdersCheck *pCheck, int isquiet ); //
void ProcessFogSpell(Unit *, AString *, OrdersCheck *pCheck, int isquiet ); //Fog
void ProcessPhanCreatures(Unit *,AString *, OrdersCheck *pCheck, int isquiet ); //Illusory Creatures

//
// Spell helpers
//
int GetRegionInRange(ARegion *r, ARegion *tar, Unit *u, int spell, int quiet, int penalty = 0);

//
// Spell running
//
int RunDetectGates(ARegion *,Object *,Unit *);  //done skillshows, cost, experience, and 1/1 mevents.
int RunFarsight(ARegion *,Unit *);           //done skillshows, cost, experience, and 3/3 mevents.
int RunGateJump(ARegion *,Object *,Unit *);  //done skillshows, cost, experience, and 2/2 mevents.
int RunTeleport(ARegion *,Object *,Unit *);  //done skillshows, cost, experience, and 2/2 mevents.
int RunPortalLore(ARegion *,Object *,Unit *);   //disabled in ES
int RunEarthLore(ARegion *,Unit *);          //done skillshows, cost, experience, and 1/1 mevents.
int RunWeatherLore(ARegion *, Unit *);  //disabled in ES
int RunClearSkies(ARegion *,Unit *);        //done skillshows, cost, experience, and 3/3 mevents.
int RunPhanBeasts(ARegion *,Unit *);    //disabled in ES
int RunPhanUndead(ARegion *,Unit *);    //disabled in ES
int RunPhanDemons(ARegion *,Unit *);    //disabled in ES
int RunInvisibility(ARegion *,Unit *);     //done skillshows, cost, experience, and 3/3 mevents.
int RunWolfLore(ARegion *,Unit *, int quiet);         //disabled in ES (summoncreatures used instead)
int RunBirdLore(ARegion *,Unit *);           //done skillshows, cost, experience, and 2/2,1/1,2/2 mevents.
int RunDragonLore(ARegion *,Unit *, int quiet);    //disabled in ES
int RunSummonSkeletons(ARegion *,Unit *);//disabled in ES
int RunRaiseUndead(ARegion *,Unit *);   //disabled in ES (summoncreatures used instead)
int RunSummonLich(ARegion *,Unit *);    //disabled in ES (summoncreatures used instead)
int RunSummonImps(ARegion *,Unit *);    //disabled in ES (summoncreatures used instead)
int RunSummonDemon(ARegion *,Unit *);   //disabled in ES (summoncreatures used instead)
int RunSummonBalrog(ARegion *,Unit *, int quiet);  //disabled in ES (summoncreatures used instead)
int RunCreateArtifact(ARegion *,Unit *,int,int);   //done skillshows, cost, experience, and 1/1 mevents.
int RunEngraveRunes(ARegion *,Object *,Unit *, int quiet);    //done skillshows, cost, experience, and 3/3 mevents.
int RunConstructGate(ARegion *,Unit *,int, int quiet);     //done skillshows, cost, experience, and 2/2 mevents.
int RunEnchantSwords(ARegion *,Unit *);        //done skillshows, cost, experience
int RunEnchantArmor(ARegion *,Unit *);         //done skillshows, cost, experience
int RunMindReading(ARegion *,Unit *);
int RunCreateFood(ARegion *,Unit *); //disabled in ES
//Arcadia
int RunPhanCreatures(ARegion *,Unit *);     //done skillshows, cost, experience, and 3/3 mevents.
int RunPhanEntertainment(ARegion *,Unit *); //done skillshows, cost, experience, and 3/3 mevents.
int RunBlizzard(ARegion *,Unit *);          //done skillshows, cost, experience, and 3/3 mevents.
int RunSeaward(ARegion *,Unit *);           //done skillshows, cost, experience, and 4/4 mevents.
int RunDiversion(ARegion *,Unit *);         //done skillshows, cost, experience, and 1/1 mevents.  Complicated 3-case code, but I don't think there's a simpler solution.
int RunSummonCreatures(ARegion *, Unit *, int spell, int item, int max); //wolves, imps, undead and demons. done skillshows, cost, experience, and 3/3 mevents.
int RunSummonHigherCreature(ARegion *, Unit *, int spell, int item); //balrogs, liches and gryffins. done skillshows, cost, experience, and 3/3 mevents.
int RunFog(ARegion *,Unit *);               //done skillshows, cost, experience, and 3/3 mevents.
int RunSummonMen(ARegion *,Unit *);         //done skillshows, cost, experience, and 2/2 mevents.
int RunTransmutation(ARegion *,Unit *);     //done skillshows, cost, experience, and 4/4 mevents.
int RunModification(ARegion *,Unit *);      //done skillshows, cost, experience, and 1/3 mevents*.
int RunRejuvenation(ARegion *, Unit *);     //done skillshows, cost, experience, and 4/4 mevents.
int RunSpiritOfDead(ARegion *, Unit *);     //done skillshows, cost, experience, and 3/3 mevents.
int RunHypnosis(ARegion *, Unit *);         //done skillshows, cost, experience, and 3/4 mevents*.
int RunCreatePortal(ARegion *, Unit *);     //done skillshows, cost, experience, and 3/3 mevents.
void DoMerchantBuy(Unit *u, BuyOrder *o);
void DoMerchantSell(Unit *u, SellOrder *o);


//portal

#endif
