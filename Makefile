# This is the makefile for Atlantis 4.0
#
# Copyright 1998 by Geoff Dunbar
# MODIFICATIONS
# Date        Person       Comments
# ----        ------       --------
# 2000/MAR/14 Davis Kulis  Added the template code.

GAME ?= standard

CPLUS = g++
CC = gcc
CFLAGS = -g -I. -I.. -Wall

RULESET_OBJECTS = extra.o monsters.o rules.o world.o

ENGINE_OBJECTS = alist.o aregion.o army.o astring.o battle.o faction.o \
  fileio.o game.o gamedata.o gamedefs.o gameio.o genrules.o items.o main.o \
  market.o modify.o monthorders.o npc.o object.o orders.o parseorders.o \
  production.o runorders.o shields.o skills.o skillshows.o specials.o \
  spells.o template.o unit.o

OBJECTS = $(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS)) \
  $(patsubst %.o,$(GAME)/obj/%.o,$(ENGINE_OBJECTS)) \
  $(GAME)/obj/i_rand.o

$(GAME)-m: objdir $(OBJECTS)
	$(CPLUS) $(CFLAGS) -o $(GAME)/$(GAME) $(OBJECTS)

all: conquest ceran realms standard wyreth

conquest: FORCE
	$(MAKE) GAME=conquest

ceran: FORCE
	$(MAKE) GAME=ceran

realms: FORCE
	$(MAKE) GAME=realms

standard: FORCE
	$(MAKE) GAME=standard

wyreth: FORCE
	$(MAKE) GAME=wyreth

all-clean: conquest-clean ceran-clean realms-clean standard-clean wyreth-clean

conquest-clean:
	$(MAKE) GAME=conquest clean

ceran-clean:
	$(MAKE) GAME=ceran clean

realms-clean:
	$(MAKE) GAME=realms clean

standard-clean:
	$(MAKE) GAME=standard clean

wyreth-clean:
	$(MAKE) GAME=wyreth clean

clean:
	rm -f $(OBJECTS)
	rm -f $(GAME)/$(GAME)
	rmdir $(GAME)/obj

FORCE:

objdir:
	if [ ! -d $(GAME)/obj ]; then mkdir $(GAME)/obj; fi


$(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS)): $(GAME)/obj/%.o: $(GAME)/%.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

$(patsubst %.o,$(GAME)/obj/%.o,$(ENGINE_OBJECTS)): $(GAME)/obj/%.o: %.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

$(GAME)/obj/i_rand.o: i_rand.c
	$(CC) $(CFLAGS) -c -o $@ $<
