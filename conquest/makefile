# This is the makefile for Atlantis 4.0
#
# Copyright 1998 by Geoff Dunbar
# MODIFICATIONS
# Date        Person       Comments
# ----        ------       --------
# 2000/MAR/14 Davis Kulis  Added the template code.

#OPTIONS
#
# Define this if you have EASIER_UNDERWORLD set to 1 in the gamedefs struct
#EASY_UNDER = -DEASY_UNDERWORLD

CFLAGS = -g -I. -I.. ${EASY_UNDER}

RULESET_OBJECTS = extra.o rules.o world.o

ENGINE_OBJECTS = alist.o aregion.o army.o astring.o battle.o faction.o \
  fileio.o game.o gamedefs.o gameio.o items.o main.o market.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o runorders.o  shields.o \
  skills.o skillshows.o specials.o spells.o template.o unit.o

OBJECTS = $(patsubst %.o,obj/%.o,$(RULESET_OBJECTS)) \
  $(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)) \
  obj/i_rand.o

all: $(OBJECTS)
	g++ $(CFLAGS) -o conquest $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	rm -f conquest

$(patsubst %.o,obj/%.o,$(RULESET_OBJECTS)): obj/%.o: %.cpp
	g++ $(CFLAGS) -c -o $@ $<

$(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)): obj/%.o: ../%.cpp
	g++ $(CFLAGS) -c -o $@ $<

obj/i_rand.o: ../i_rand.c
	gcc $(CFLAGS) -c -o $@ $<
