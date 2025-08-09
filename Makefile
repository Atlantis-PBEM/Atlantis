GAME ?= standard

CPLUS ?= g++
CC ?= gcc
CFLAGS = -g -I. -I.. -Wextra -Wall -Werror -std=c++20 -pedantic -Wparentheses -Wpointer-arith -Wignored-qualifiers -Wno-psabi

RULESET_OBJECTS = extra.o map.o monsters.o rules.o world.o

ENGINE_OBJECTS = aregion.o army.o battle.o economy.o \
  edit.o faction.o game.o gamedata.o gamedefs.o \
  genrules.o items.o main.o market.o modify.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o quests.o runorders.o \
  skills.o skillshows.o specials.o spells.o unit.o \
  events.o events-battle.o events-assassination.o mapgen.o simplex.o namegen.o \
  indenter.o text_report_generator.o

UNITTEST_SRC = unittest/main.cpp unittest/testhelper.cpp $(wildcard unittest/*_test.cpp)
UNITTEST_OBJECTS = $(patsubst unittest/%.cpp,unittest/obj/%.o,$(UNITTEST_SRC))

OBJECTS =  $(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)) $(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS))

$(GAME)-m: objdir $(OBJECTS)
	$(CPLUS) $(CFLAGS) -o $(GAME)/$(GAME) $(OBJECTS)

all: basic standard fracas kingdoms havilah neworigins unittest

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
	havilah-clean neworigins-clean unittest-clean

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
	if [ -d obj ]; then rm -rf obj; fi
	if [ -d $(GAME)/obj ]; then rm -rf $(GAME)/obj; fi
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

# Some utility tasks to keep the external header libraries up to date if needed.
EXTERNAL_DIR := external
UT_DIR := $(EXTERNAL_DIR)/boost
JSON_DIR := $(EXTERNAL_DIR)/nlohmann

UT_RELEASE_URL := https://github.com/boost-ext/ut/archive/refs/tags
JSON_RELEASE_URL := https://github.com/nlohmann/json/releases/download

.PHONY: check-libraries
check-libraries: check-ut check-json

.PHONY: check-ut
check-ut:
	@NEEDS_UPDATE=false; \
	if [ ! -f "$(UT_DIR)/ut.hpp" ]; then \
		echo "UT library not found. Preparing to download..."; \
		NEEDS_UPDATE=true; \
	else \
		CURRENT_VERSION=$$(cd $(UT_DIR) && grep -m 1 'BOOST_UT_VERSION' ut.hpp | awk '{print $$3}' | sed "s/'/./g"); \
		CURRENT_VERSION=v$$CURRENT_VERSION; \
		LATEST_TAG=$$(curl -s https://api.github.com/repos/boost-ext/ut/releases/latest | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'); \
		if [ "$$CURRENT_VERSION" != "$$LATEST_TAG" ]; then \
			echo "UT library is outdated (current: $$CURRENT_VERSION, latest: $$LATEST_TAG). Preparing to update..."; \
			NEEDS_UPDATE=true; \
		fi; \
	fi; \
	if $$NEEDS_UPDATE; then \
		LATEST_TAG=$$(curl -s https://api.github.com/repos/boost-ext/ut/releases/latest | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'); \
		TEMP_DIR=$$(mktemp -d); \
		curl -L $(UT_RELEASE_URL)/$$LATEST_TAG.tar.gz | tar -xz -C $$TEMP_DIR; \
		rm -rf $(UT_DIR); \
		mkdir -p $(UT_DIR); \
		cp -r $$TEMP_DIR/*/include/boost/ut.hpp $(UT_DIR); \
		rm -rf $$TEMP_DIR; \
		echo "UT library updated from $$CURRENT_VERSION to $$LATEST_TAG."; \
	else \
		echo "UT library is up-to-date at version $$CURRENT_VERSION."; \
	fi

.PHONY: check-json
check-json:
	@NEEDS_UPDATE=false; \
	if [ ! -f "$(JSON_DIR)/json.hpp" ]; then \
		echo "JSON library not found. Preparing to download..."; \
		CURRENT_VERSION="uninstalled"; \
		NEEDS_UPDATE=true; \
	else \
		CURRENT_VERSION=$$(grep -E '^#define NLOHMANN_JSON_VERSION_(MAJOR|MINOR|PATCH)' $(JSON_DIR)/json.hpp | awk '{print $$3}' | tr '\n' '.'); \
		CURRENT_VERSION=v$${CURRENT_VERSION%?}; \
		LATEST_TAG=$$(curl -s https://api.github.com/repos/nlohmann/json/releases/latest | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'); \
		if [ "$$CURRENT_VERSION" != "$$LATEST_TAG" ]; then \
			echo "JSON library is outdated (current: $$CURRENT_VERSION, latest: $$LATEST_TAG). Preparing to update..."; \
			NEEDS_UPDATE=true; \
		fi; \
	fi; \
	if $$NEEDS_UPDATE; then \
		LATEST_TAG=$$(curl -s https://api.github.com/repos/nlohmann/json/releases/latest | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'); \
		mkdir -p $(JSON_DIR); \
		curl -L $(JSON_RELEASE_URL)/$$LATEST_TAG/json.hpp -o $(JSON_DIR)/json.hpp; \
		echo "JSON library updated from $$CURRENT_VERSION to $$LATEST_TAG."; \
	else \
		echo "JSON library is up-to-date at version $$CURRENT_VERSION."; \
	fi

