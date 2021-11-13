GAME = arcadia

CPLUS = g++
CC = gcc
CFLAGS = -g -I. -Wall

ENGINE_OBJECTS = alist.o aregion.o army1.o astring.o battle1.o economy.o \
  formation1.o  hexside.o times.o soldier1.o magic.o \
  edit.o faction.o fileio.o game.o gamedata.o gamedefs.o gameio.o \
  genrules.o i_rand.o items.o main.o market.o modify.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o runorders.o \
  shields.o skills.o skillshows.o specials.o spells.o template.o unit.o \
  extra.o map.o monsters.o rules.o world.o

OBJECTS = $(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)) 

$(GAME)-m: objdir $(OBJECTS)
	$(CPLUS) $(CFLAGS) -o $(GAME) $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	if [ -d obj ]; then rmdir obj; fi
	rm -f html/$(GAME).html

rules:
	(./$(GAME) genrules html/intro.html html/styles.css html/$(GAME).html)

objdir:
	if [ ! -d obj ]; then mkdir obj; fi

$(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)): obj/%.o: %.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

