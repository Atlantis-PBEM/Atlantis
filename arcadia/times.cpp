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
    }
    if(b3) {
        WorldEvent * event = new WorldEvent;
        event->type = WorldEvent::BATTLE;
        event->place = new Location;
		event->place->region = b2->region;
		event->place->obj = 0;
		event->place->unit = 0;
		event->fact1 = b2->casualties;
		event->reportdelay = 0;
		worldevents.Add(event);
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
        str = AString("rumor.") + rumournum;
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

void Game::CreateTimesReports()
{
    CreateBattleEvents();

    int rumournum = 1;
    AString rumour;
    
    forlist(&worldevents) {
        WorldEvent *event = (WorldEvent *) elem;
        switch(event->type) {
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
            default:
                break;
        }
    }
}
