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

RULESET_OBJECTS = extra.o map.o monsters.o rules.o world.o

ENGINE_OBJECTS = alist.o aregion.o army.o astring.o battle.o faction.o \
  fileio.o game.o gamedata.o gamedefs.o gameio.o genrules.o items.o main.o \
  market.o modify.o monthorders.o npc.o object.o orders.o parseorders.o \
  production.o runorders.o shields.o skills.o skillshows.o specials.o \
  spells.o template.o unit.o

OBJECTS = $(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS)) \
  $(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)) \
  obj/i_rand.o

$(GAME)-m: objdir $(OBJECTS)
	$(CPLUS) $(CFLAGS) -o $(GAME)/$(GAME) $(OBJECTS)

all: conquest ceran realms standard wyreth fracas

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

fracas: FORCE
	$(MAKE) GAME=fracas

$(GAME)/$(GAME): FORCE
	$(MAKE) GAME=$(GAME)

all-clean: conquest-clean ceran-clean realms-clean standard-clean wyreth-clean \
  fracas-clean

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

fracas-clean:
	$(MAKE) GAME=fracas clean

clean:
	rm -f $(OBJECTS)
	if [ -d obj ]; then rmdir obj; fi
	if [ -d $(GAME)/obj ]; then rmdir $(GAME)/obj; fi
	rm -f $(GAME)/html/$(GAME).html
	rm -f $(GAME)/$(GAME)

all-rules: conquest-rules ceran-rules realms-rules standard-rules wyreth-rules \
  fracas-rules

conquest-rules:
	$(MAKE) GAME=conquest rules

ceran-rules:
	$(MAKE) GAME=ceran rules

realms-rules:
	$(MAKE) GAME=realms rules

standard-rules:
	$(MAKE) GAME=standard rules

wyreth-rules:
	$(MAKE) GAME=wyreth rules

fracas-rules:
	$(MAKE) GAME=fracas rules

rules: $(GAME)/$(GAME)
	(cd $(GAME); \
	 ./$(GAME) genrules $(GAME)_intro.html $(GAME).css html/$(GAME).html \
	)

FORCE:

objdir:
	if [ ! -d obj ]; then mkdir obj; fi
	if [ ! -d $(GAME)/obj ]; then mkdir $(GAME)/obj; fi


$(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS)): $(GAME)/obj/%.o: $(GAME)/%.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

$(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)): obj/%.o: %.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

obj/i_rand.o: i_rand.c
	$(CC) $(CFLAGS) -c -o $@ $<
