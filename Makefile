# This is the makefile for Atlantis 5.0
#
# Copyright 1998 by Geoff Dunbar
# MODIFICATIONS
# Date        Person         Comments
# ----        ------         --------
# 2000/MAR/14 Davis Kulis    Added the template code.
# 2004/MAR/29 Jan Rietema    Added/modified the gamesets

GAME ?= standard

CPLUS = g++
CC = gcc
CFLAGS = -g -I. -I.. -Wall -Werror -std=c++20

RULESET_OBJECTS = extra.o map.o monsters.o rules.o world.o 

ENGINE_OBJECTS = alist.o aregion.o army.o astring.o battle.o economy.o \
  edit.o faction.o fileio.o game.o gamedata.o gamedefs.o gameio.o \
  genrules.o i_rand.o items.o main.o market.o modify.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o quests.o runorders.o \
  shields.o skills.o skillshows.o specials.o spells.o template.o unit.o \
  events.o events-battle.o events-assassination.o mapgen.o simplex.o namegen.o

UNITTEST_SRC = unittest/main.cpp $(wildcard unittest/*_test.cpp)
UNITTEST_OBJECTS = $(patsubst unittest/%.cpp,unittest/obj/%.o,$(UNITTEST_SRC))

OBJECTS =  $(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)) $(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS))

$(GAME)-m: objdir $(OBJECTS)
	$(CPLUS) $(CFLAGS) -o $(GAME)/$(GAME) $(OBJECTS)

all: basic standard fracas kingdoms havilah neworigins

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

neworigins: FORCE
	$(MAKE) GAME=neworigins

$(GAME)/$(GAME): FORCE
	$(MAKE) GAME=$(GAME)

all-clean: basic-clean standard-clean fracas-clean kingdoms-clean \
	havilah-clean neworigins-clean

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

neworigins-clean:
	$(MAKE) GAME=neworigins clean

unittest-clean:
	$(MAKE) GAME=unittest clean

clean:
	rm -f $(OBJECTS)
	rm -f $(UNITTEST_OBJECTS)
	if [ -d obj ]; then rmdir obj; fi
	if [ -d $(GAME)/obj ]; then rmdir $(GAME)/obj; fi
	rm -f $(GAME)/html/$(GAME).html
	rm -f $(GAME)/$(GAME)

all-rules: basic-rules standard-rules fracas-rules kingdoms-rules \
	havilah-rules neworigins-rules

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

neworigins-rules:
	$(MAKE) GAME=neworigins rules

rules: $(GAME)/$(GAME)
	(cd $(GAME); \
	 ./$(GAME) genrules $(GAME)_intro.html $(GAME).css html/$(GAME).html \
	)

.PHONY: unittest
unittest:
	$(MAKE) GAME=unittest unittest-build

unittest-build: unittest-objdir $(filter-out obj/main.o,$(OBJECTS)) $(UNITTEST_OBJECTS)
	$(CPLUS) $(CFLAGS) -o unittest/unittest $(filter-out obj/main.o,$(OBJECTS)) $(UNITTEST_OBJECTS) 

FORCE:

unittest-objdir: objdir
	if [ ! -d unittest/obj ]; then mkdir unittest/obj; fi

objdir:
	if [ ! -d obj ]; then mkdir obj; fi
	if [ ! -d $(GAME)/obj ]; then mkdir $(GAME)/obj; fi


$(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS)): $(GAME)/obj/%.o: $(GAME)/%.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

$(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)): obj/%.o: %.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

# If the boost.hpp file is updated, we need to rebuild the unit test files that include it.
$(UNITTEST_OBJECTS): unittest/obj/%.o: unittest/%.cpp external/boost/ut.hpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

