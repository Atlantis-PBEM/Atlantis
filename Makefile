# This is the makefile for Atlantis 5.0
#
# Copyright 1998 by Geoff Dunbar
# MODIFICATIONS
# Date        Person         Comments
# ----        ------         --------
# 2000/MAR/14 Davis Kulis    Added the template code.
# 2004/MAR/29 Jan Rietema    Added/modified the gamesets
CXX ?= g++
CXXFLAGS += -g -I. -I.. -Wall -std=c++11 -MP -MMD -O

# allow substitution of dependency file
CXXBUILD = $(CXX) $(CXXFLAGS) -MF $(patsubst %.cpp,dep/%.d,$<) -c -o $@ $<

OBJ := alist.o aregion.o army.o astring.o battle.o economy.o \
  edit.o faction.o fileio.o game.o gamedata.o gamedefs.o gameio.o \
  genrules.o i_rand.o items.o main.o market.o modify.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o quests.o runorders.o \
  shields.o skills.o skillshows.o specials.o spells.o template.o unit.o \
  events.o events-battle.o events-assassination.o mapgen.o simplex.o namegen.o

# objects per rule set
RULESET := extra.o map.o monsters.o rules.o world.o

# sub games
GAMES := basic standard fracas kingdoms havilah neworigins
# arcadia seems different

DEP  := $(addprefix dep/,$(OBJ:.o=.d))
ALL_OBJS := $(addprefix obj/,$(OBJ))

### targets
.PHONY: all
all: dep obj

.PHONY: all-rules

obj:
	@mkdir $@

dep:
	@mkdir $@

-include $(DEP)
-include $(addsuffix /Makefile.inc, $(GAMES))

.PHONY: clean
clean::
	@rm -f $(ALL_OBJS)

.PHONY: all-clean
all-clean:
	$(MAKE) clean

$(ALL_OBJS): obj/%.o: %.cpp
	@$(CXXBUILD)

