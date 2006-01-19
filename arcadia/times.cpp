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

#include "game.h"
#include "gamedata.h"
#include "unit.h"

WorldEvent::WorldEvent()
{
    type = -1;
    place = 0;
    reportdelay = 0;
    age = 0;
    fact1 = -1;
    fact2 = -1;
}

WorldEvent::~WorldEvent()
{
}

AString *WorldEvent::Text()
{
    AString *text = 0;
    return text;
}

void Game::CreateBattleEvents()
{
    Battle * b1 = 0;
    Battle * b2 = 0;
    Battle * b3 = 0;
    
    forlist(&battles) {
        Battle *b = (Battle *) elem;
        if(!b1 || b->casualties > b1->casualties) {
            b2 = b1;
            b3 = b2;
            b1 = b;
        } else if(!b2 || b->casualties > b2->casualties) {
            b3 = b2;
            b2 = b;
        } else if(!b3 || b->casualties > b3->casualties) {
            b3 = b;
        }
    }
    //we've picked out the three biggest battles in the world for the turn.
    
    if(b1) {
        WorldEvent * event = new WorldEvent;
        event->type = WorldEvent::BATTLE;
        event->place = new Location;
		event->place->region = b1->region;
		event->place->obj = 0;
		event->place->unit = 0;
		event->fact1 = b1->casualties;
		event->reportdelay = 0;
		worldevents.Add(event);
		b1 = 0;
    }
    if(b2) {
        WorldEvent * event = new WorldEvent;
        event->type = WorldEvent::BATTLE;
        event->place = new Location;
		event->place->region = b2->region;
		event->place->obj = 0;
		event->place->unit = 0;
		event->fact1 = b2->casualties;
		event->reportdelay = 0;
		worldevents.Add(event);
		b2 = 0;
    }
    if(b3) {
        WorldEvent * event = new WorldEvent;
        event->type = WorldEvent::BATTLE;
        event->place = new Location;
		event->place->region = b3->region;
		event->place->obj = 0;
		event->place->unit = 0;
		event->fact1 = b3->casualties;
		event->reportdelay = 0;
		worldevents.Add(event);
		b3 = 0;
    }
}

void Game::WriteRumour(int &rumournum, AString rumour)
{
    int tries = 0;
    int i;
    Aoutfile f;
    Ainfile g;
    AString str;
    
    do {
        str = AString("rumour.") + rumournum;
        rumournum++;
        i = g.OpenByName( str );
        if(i == 0) g.Close();
    } while (i == 0 && tries < 100);
    
    i = f.OpenByName( str );
    if(i == -1) {
        Awrite("Could not write rumour");
        return;
    }
    f.PutStr(rumour);

    f.Close();
}

void Game::WriteTimes(int timesnum, AString times)
{
    Aoutfile f;
    AString str;
    
    str = AString("times.") + timesnum;
    int i = f.OpenByName( str, 1 );
    if(i == -1) {
        Awrite("Could not write times");
        return;
    }
    f.PutStr(times);

    f.Close();
}

void Game::CreateTimesReports()
{
    CreateBattleEvents();

    int rumournum = 1;
    int timesnum = 1;
    AString rumour;
    AString times;
    Faction *pFac;
    
    int elfwritten = 0;
    int humanwritten = 0;    
    int dwarfwritten = 0;
    int independentwritten = 0;

    forlist(&factions) {
        Faction *f = (Faction *) elem;
        //This section writes an opening intro for each faction
        
        if(f->pStartLoc) {
            //ie on turn faction is created.
            //get hero name
            AString hero;
            AString race;
            forlist(&f->pStartLoc->objects) {
		        Object *o = (Object *) elem;
		        forlist(&o->units) {
		            Unit *u = (Unit *) elem;
		            if(u->faction == f && u->type == U_MAGE) {
                        hero = *u->name;
                        race = EthnicityString(u->GetEthnicity());
                    }
		        }
		    }
		    AString *fname = new AString(*f->name);
		    AString *token = fname->gettoken();
		    int the = 0;
		    if(*token == "the") the = 1;
		    delete fname;
		    fname = 0;
		    delete token;
		    token = 0;
		    
		    times = hero + ", a " + race + " hero from the ";
		    if(f->pStartLoc->town) {
		        times += TownString(f->pStartLoc->town->TownType()) + " ";
		        times += *f->pStartLoc->town->name;
		    } else {
		    /////
		    }
		    times += ", has emerged from obscurity. As leader of "; 
            if(!the) times += "the ";
            times += *f->name;
            times += AString(", ") + hero + " may yet achieve dominance over Xanaxor.";            
            WriteTimes(f->num, times);
        }
    }
    
    forlist_reuse(&regions) {
		ARegion *r = (ARegion *) elem;
		
		//This section writes a message every time a city guard/mage unit is killed
		if(r->timesmarker == 1) {
            times = "The guards of ";
            if(r->town) times += (*r->town->name);
            else times += *r->name;
            times += " have been callously slain!";
            
    		switch(r->GetEthnicity()) {
    		    case RA_HUMAN:
    		        timesnum = guardfaction;
    		        humanwritten = 1;
    		        break;
    		    case RA_ELF:
    		        timesnum = elfguardfaction;
    		        elfwritten = 1;
    		        break;
    		    case RA_DWARF:
    		        timesnum = dwarfguardfaction;
    		        dwarfwritten = 1;
    		        break;
    		    case RA_OTHER:
    		        timesnum = independentguardfaction;
    		        independentwritten = 1;
    		        break;
    		    default:
    		        timesnum = 0;
    		        break;
    		}
    		
    		if(timesnum) WriteTimes(timesnum, times);
        } 
    }

    forlist_reuse(&worldevents) {
        WorldEvent *event = (WorldEvent *) elem;
        switch(event->type) {
        //This writes a rumour for <the biggest 3> ALL? battles around the world
            case WorldEvent::BATTLE:
                rumour = AString("A battle was fought in ") + *event->place->region->name + 
                    ", ";
                if(event->fact1 < 8) rumour += "with less than 10 deaths";
                else if(event->fact1 < 80) {
                    rumour += "with about ";
                    rumour += ((event->fact1+3+getrandom(5))/10)*10;
                    rumour += " deaths.";
                } else {
                    rumour += "with about ";
                    rumour += ((event->fact1+30+getrandom(41))/100)*100;
                    rumour += " deaths.";
                }
                WriteRumour(rumournum, rumour);
                break;
            case WorldEvent::CONVERSION:
                pFac = GetFaction(&factions, event->fact1);
                if(!pFac) break;
                if(pFac->ethnicity != event->fact2) break;
                times = AString(*pFac->name) + " has converted to the ";
        		switch(pFac->ethnicity) {
        		    case RA_HUMAN:
        		        times += "human cause!";
        		        timesnum = guardfaction;
        		        humanwritten = 1;
        		        break;
        		    case RA_ELF:
        		        times += "elven cause!";
        		        timesnum = elfguardfaction;
        		        elfwritten = 1;
        		        break;
        		    case RA_DWARF:
        		        times += "dwarven cause!";
        		        timesnum = dwarfguardfaction;
        		        dwarfwritten = 1;
        		        break;
        		    case RA_OTHER:
        		        times += "independent cause!";
        		        timesnum = independentguardfaction;
    		            independentwritten = 1;
        		        break;
        		    case RA_NA:
        		        times = AString("The leadership of ") + *pFac->name + " is in chaos.";
        		        timesnum = peasantfaction;
        		        break;
        		    default:
        		        times += "!@#$ please alert your GM !@#$";
        		        timesnum = guardfaction;
        		        humanwritten = 1;
        		        break;    		
        		}
                WriteTimes(timesnum, times);
                break;
            default:
                break;
        }
    }
    
    //use this if your script doesn't add on the faction name when compiling times.
    if(humanwritten) {
        Faction *fac = GetFaction(&factions, guardfaction);
        WriteTimes(guardfaction, AString(" "));
        WriteTimes(guardfaction, AString(*fac->name));
    }
    if(elfwritten) {
        Faction *fac = GetFaction(&factions, elfguardfaction);
        WriteTimes(elfguardfaction, AString(" "));
        WriteTimes(elfguardfaction, AString(*fac->name));
    }
    if(dwarfwritten) {
        Faction *fac = GetFaction(&factions, dwarfguardfaction);
        WriteTimes(dwarfguardfaction, AString(" "));
        WriteTimes(dwarfguardfaction, AString(*fac->name));
    }
    if(independentwritten) {
        Faction *fac = GetFaction(&factions, independentguardfaction);
        WriteTimes(independentguardfaction, AString(" "));
        WriteTimes(independentguardfaction, AString(*fac->name));
    }
    
}
