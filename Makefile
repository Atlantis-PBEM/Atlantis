# This is the makefile for Atlantis 4.0
#
# Copyright 1998 by Geoff Dunbar
# MODIFICATIONS
# Date        Person       Comments
# ----        ------       --------
# 2000/MAR/14 Davis Kulis  Added the template code.
# 2004/MAR/29 Jan Rietema  Added/modified the gamesets

GAME ?= standard

CPLUS = g++
CC = gcc
CFLAGS = -g -I. -I.. -Wall

RULESET_OBJECTS = extra.o map.o monsters.o rules.o world.o 

ENGINE_OBJECTS = alist.o aregion.o army.o astring.o battle.o economy.o \
  edit.o faction.o fileio.o game.o gamedata.o gamedefs.o gameio.o \
  genrules.o i_rand.o items.o main.o market.o modify.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o quests.o runorders.o \
  shields.o skills.o skillshows.o specials.o spells.o template.o unit.o

OBJECTS = $(patsubst %.o,$(GAME)/%.o,$(RULESET_OBJECTS)) \
  $(ENGINE_OBJECTS)

$(GAME)-m: $(OBJECTS)
	$(CPLUS) $(CFLAGS) -o $(GAME)/$(GAME) $(OBJECTS)

.PHONY: all basic standard fracas kingdoms havilah arcadia

all: basic standard fracas kingdoms havilah

arcadia: FORCE
	$(MAKE) GAME=arcadia

basic: FORCE
	$(MAKE) GAME=basic

standard: FORCE
	$(MAKE) GAME=standard
	
kingdoms: FORCE
	$(MAKE) GAME=kingdoms

fracas: FORCE
	$(MAKE) GAME=fracas

havilah: FORCE
	$(MAKE) GAME=havilah

$(GAME)/$(GAME): FORCE
	$(MAKE) GAME=$(GAME)

.PHONY: all-clean basic-clean standard-clean fracas-clean kingdoms-clean havilah-clean arcadia-clean clean

all-clean: basic-clean standard-clean fracas-clean kingdoms-clean \
	havilah-clean

arcadia-clean:
	$(MAKE) GAME=arcadia clean

basic-clean:
	$(MAKE) GAME=basic clean

standard-clean:
	$(MAKE) GAME=standard clean

fracas-clean:
	$(MAKE) GAME=fracas clean
	
kingdoms-clean:
	$(MAKE) GAME=kingdoms clean

havilah-clean:
	$(MAKE) GAME=havilah clean

clean:
	rm -f $(OBJECTS)
	rm -f $(GAME)/html/$(GAME).html
	rm -f $(GAME)/$(GAME)

.PHONY: all-rules basic-rules standard-rules fracas-rules kingdoms-rules havilah-rules arcadia-rules rules

all-rules: basic-rules standard-rules fracas-rules kingdoms-rules \
	havilah-rules

arcadia-rules:
	$(MAKE) GAME=arcadia rules

basic-rules:
	$(MAKE) GAME=basic rules

standard-rules:
	$(MAKE) GAME=standard rules

fracas-rules:
	$(MAKE) GAME=fracas rules
	
kingdoms-rules:
	$(MAKE) GAME=kingdoms rules

havilah-rules:
	$(MAKE) GAME=havilah rules

rules: $(GAME)/$(GAME)
	(cd $(GAME); \
	 ./$(GAME) genrules $(GAME)_intro.html $(GAME).css html/$(GAME).html \
	)

FORCE:


$(patsubst %.o,$(GAME)/%.o,$(RULESET_OBJECTS)): $(GAME)/%.o: $(GAME)/%.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

$(patsubst %.o,%.o,$(ENGINE_OBJECTS)): %.o: %.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

