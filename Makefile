# This is the makefile for Atlantis 4.0
#
# Copyright 1998 by Geoff Dunbar
# MODIFICATIONS
# Date        Person       Comments
# ----        ------       --------
# 2000/MAR/14 Davis Kulis  Added the template code.
# 2004/MAR/29 Jan Rietema  Added/modified the gamesets

GAME ?= standard

CXX ?= g++
CXXFLAGS = -g -Wall

RULESET_OBJECTS = extra.o map.o monsters.o rules.o world.o 

ENGINE_OBJECTS = alist.o aregion.o army.o astring.o battle.o economy.o \
  edit.o faction.o fileio.o game.o gamedata.o gamedefs.o gameio.o \
  genrules.o i_rand.o items.o main.o market.o modify.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o quests.o runorders.o \
  shields.o skills.o skillshows.o specials.o spells.o template.o unit.o

OBJECTS = $(patsubst %.o,$(GAME)/%.o,$(RULESET_OBJECTS)) \
  $(ENGINE_OBJECTS)

# default target
$(GAME)/$(GAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(GAME)/$(GAME) $(OBJECTS)

# Arcadia stuff
ARCADIA_ENGINE_SOURCES = \
		alist.cpp     i_rand.cpp

ARCADIA_SOURCES = \
		astring.cpp   edit.cpp      fileio.cpp      \
		gamedata.cpp  genrules.cpp  items.cpp        map.cpp         \
		monsters.cpp  object.cpp    production.cpp   shields.cpp     \
		soldier1.cpp  template.cpp  world.cpp        aregion.cpp     \
		battle1.cpp   extra.cpp     formation1.cpp   gamedefs.cpp    \
		hexside.cpp   magic.cpp     market.cpp       monthorders.cpp \
		orders.cpp    rules.cpp     skills.cpp       specials.cpp    \
		times.cpp     army1.cpp     economy.cpp      faction.cpp     \
		game.cpp      gameio.cpp    main.cpp        \
		modify.cpp    npc.cpp       parseorders.cpp  runorders.cpp   \
		skillshows.cpp  spells.cpp  unit.cpp

ARCADIA_OBJECTS = $(patsubst %.cpp,arcadia/%.o,$(ARCADIA_SOURCES)) $(patsubst %.cpp,%.o,$(ARCADIA_ENGINE_SOURCES))

arcadia/arcadia: $(ARCADIA_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: all basic standard fracas kingdoms havilah arcadia

all: basic standard fracas kingdoms havilah

arcadia: arcadia/arcadia

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


.PHONY: all-clean basic-clean standard-clean fracas-clean kingdoms-clean havilah-clean arcadia-clean clean

all-clean: basic-clean standard-clean fracas-clean kingdoms-clean \
	havilah-clean

arcadia-clean:
	rm -f $(ARCADIA_OBJECTS)
	rm -f arcadia/html/arcadia.html
	rm -f arcadia/arcadia


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

arcadia-rules: arcadia/arcadia
	(cd arcadia; \
	 ./arcadia genrules arcadia_intro.html arcadia.css html/arcadia.html \
	)


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


