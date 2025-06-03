# Atlantis Gamemaster's Guide

This document describes how to compile, setup, modify and run a game of
Atlantis.

The maintainer is Anthony Briggs, who can be contacted at
<abriggs@iinet.net.au>. Send any feedback, suggestions, comments, etc.
to him. This document is heavily based on the previous gamemaster guide,
which was written by JT Traub.

Thanks to: JT Traub, "Mr. Mistofeles", "Draig" and "Iyhael" for their
valuable feedback.


## Table of Contents

1.      Obtaining a copy of Atlantis

2.      Compiling Atlantis
2.1     Unix
2.1.1   A sample compile under UNIX
2.2     Windows (copied directly from the old guide)
2.2.1   Compiling Atlantis with Dev-C++ on Windows
2.3     The gory details (for advanced users, or the terminally curious)

3.      Running a game
3.1     Running a game of Atlantis by hand (the hard way)
3.2     Running a game the easy way (with scripts)

4.      A GM's 'Code of Conduct'

5.      Altering game rules

6.  World Creation Guide
6.1 Basics: world size, land mass and levels
6.2.    Land Mass Distribution
6.3.    Terrain Distribution
6.4.    Towns and Races
6.5 Special: Fractal Generation


## 1. Obtaining a copy of Atlantis

There are two options if you want to get a copy of Atlantis: the first
is to download a release tarball from yahoo groups at
<http://groups.yahoo.com/group/atlantisdev/files/Sources/>.
At the time of writing this Guide, the most recent release was 5.1.0.

The second option is to get a copy of the sources from git. Under a UNIX
system, issue the following commands to obtain a copy of the Atlantis
sources:

git clone https://github.com/Atlantis-PBEM/Atlantis.git

## 2. Compiling Atlantis

### 2.1 Unix

A makefile is included for compilation on Unix, using the GNU gcc
compiler. You can get more information about the GNU compiler at
http://www.gnu.org. This has been tested on Linux and FreeBSD, and
should work on any Unix system. Just type 'make <gamedirname>' at the
top level, where <gamedirname> is one of the various subdirectories,
such as havilah or standard.

You may run into trouble if your system uses a non-gnu make (such as
FreeBSD). In that case, you need to type gmake <gamedirname> instead of
make. The specific error is something like: gcc: No input files
specified, followed by *** Error code 1, and make quitting.

Once the compile has completed, you should find the game specific files
within the game's subdirectory. The main one is the game binary, which
should be named after the game. There will also be a html directory with
the rules in it, if you've also done a 'make <gamedirname>-rules'.

If for some reason you want to clean up the sources afterwards (ie.
remove object files, for reasons of disk space, perhaps) you can use
'make <gamedir>-clean' or 'make GAME=gamedir clean'


### 2.1.1 A sample compile under UNIX

[steve@kudzu atlantis]$ make havilah
make GAME=havilah
make[1]: Entering directory `/home/steve/atlantis'
if [ ! -d obj ]; then mkdir obj; fi
if [ ! -d havilah/obj ]; then mkdir havilah/obj; fi
g++ -g -I. -I.. -Wall -c -o havilah/obj/extra.o havilah/extra.cpp
g++ -g -I. -I.. -Wall -c -o havilah/obj/map.o havilah/map.cpp
g++ -g -I. -I.. -Wall -c -o havilah/obj/monsters.o havilah/monsters.cpp
g++ -g -I. -I.. -Wall -c -o havilah/obj/rules.o havilah/rules.cpp
g++ -g -I. -I.. -Wall -c -o havilah/obj/world.o havilah/world.cpp
g++ -g -I. -I.. -Wall -c -o obj/aregion.o aregion.cpp
.
.
.
g++ -g -I. -I.. -Wall -c -o obj/template.o template.cpp
g++ -g -I. -I.. -Wall -c -o obj/unit.o unit.cpp
g++ -g -I. -I.. -Wall -o havilah/havilah havilah/obj/extra.o havilah/obj/map.o
havilah/obj/monsters.o havilah/obj/rules.o havilah/obj/world.o
obj/aregion.o obj/army.o obj/astring.o obj/battle.o obj/economy.o obj/edit.o
obj/faction.o obj/fileio.o obj/game.o obj/gamedata.o obj/gamedefs.o
obj/genrules.o obj/items.o obj/main.o obj/market.o
obj/modify.o obj/monthorders.o obj/npc.o obj/object.o obj/orders.o
obj/parseorders.o obj/production.o obj/quests.o obj/runorders.o obj/shields.o
obj/skills.o obj/skillshows.o obj/specials.o obj/spells.o obj/template.o
obj/unit.o
make[1]: Leaving directory `/home/steve/atlantis'


### 2.2 Windows (copied directly from the old guide)

I am now using the GNU gcc compiler on Windows as well. This has the
advantage that I can use it on both Unix and Windows, and as an added
bonus the compiler is freely available. As above, just switch into the
rule-set subdirectory and type 'make'. The Win32 port of gcc can be
found at <http://www.cygwin.com/>.

I'm no longer using Visual C++, so I don't include a Visual C++
makefile. If someone wants to create a makefile for others to use, I'll
be happy to put it on the download page.

However, it is very easy to make a VC project to compile Atlantis:
1. Choose a rule-set that you want to compile. For instance, the
   Atlantis Conquest rule-set is in the subdirectory conquest.
2. Create a 'Console Application' Workspace in the rule-set
   directory.
3. Add all of the Atlantis engine source files to the project. These
   are all of the .cpp files located in the main directory.
4. Add all of the rule-set specific source files; these are all of
   the .cpp files in the rule-set directory.
5. Update the include path to include both the main directory and the
   rule-set directory.
6. That's it; you should be able to compile Atlantis at this point.
   You'll probably want to compile the 'Checked' version in case you
   want to debug.

2.2.1 Compiling Atlantis with Dev-C++ on Windows
      (Submitted by malakh@subdimension.com)
      (Added to by gmfangs@asmrb.org)

First, you need to have the open source compiler Dev-C++ version 5
installed (homepage: <http://www.bloodshed.net/dev/devcpp.html>, this
howto tested with the version "Dev-C++ 5.0 beta 9 (4.9.9.1) (7.6 MB)
with Mingw/GCC 3.3.1").

I recommend installing it to "C:\Dev-Cpp\" if you are using anything
prior to Windows XP -- this will help from running into a limitation on
the length of commands in DOS that may prevent the linker from working.
Otherwise it may be installed anywhere.

Next, you'll need to download the Atlantis source code. One of the best
ways to do this is to use open source Tortoise CVS (homepage:
<http://www.tortoisecvs.org/>, current version:
<http://prdownloads.sourceforge.net/tortoisecvs/TortoiseCVS-1.8.11.exe>).

Once Tortoise CVS is installed, select the "C:\Dev-Cpp\" folder, right-
click and select the "CVS Checkout..." menu item. In the checkout
dialog, select for the Protocol: "Password server (:pserver)", for the
Server: enter "cvs.dragoncat.net", for the Repository folder: enter
"/data/cvs", for the User name: enter "guest", for the Module: enter
"atlantis", and leave the port blank. If you want the Atlantis 4 stable
branch, click on the Revision tab, select "Choose branch or tag", and
enter "RELEASE_410" for the Branch. Then click "Ok" and the current
version of Atlantis 5, (or Atlantis 4.10 if you selected that branch)
will be downloaded into the folder "C:\Dev-Cpp\atlantis\".

If you don't want to use Tortoise CVS, you can downloaded the Atlantis
source code, decompressed it (remembering to preserve the files as unix
files), and then copy the atlantis folder inside "C:\Dev-Cpp\".

To compile Atlantis, you must first change some compiler options. Open
Dev-C++, and select the menu Tools -> Compiler Options, and then click
on the "Directories" tab. Make sure that there is a path to your
binaries at "C:\Dev-Cpp\bin" (they should be set there by default, but
I've run into cases where the Dev-C++ beta installer leaves this empty).

Next click on the "C++ Includes" tab, and add the path to the
"C:\Dev-Cpp\include\c++\3.3.1\mingw32\bits" folder. Now you can close
the compiler options.

If you are compiling Atlantis 5.0 project, start a new C++ console
project. Save the *.dev in the same folder as the *.cpp source files for
Atlantis if you are using anything prior to Windows XP (again this keeps
the paths short.) Otherwise, you can create a folder anywhere, for
example "C:\Dev-Cpp\standard"

When you first create a project, it starts with a pre-generated main.cpp
file. Close it without saving it. You should now have an empty project.

Next go to Project -> Add to Project, and click on the popup "Files of
Type" so that you see only C++ sources. Then click to select the first
c++ source file in the "C:\Dev-Cpp\atlantis" folder (normally
aregion.cpp), and then shift-click on the last file (normally unit.h).
Then click "Open" to add all of these files to your project.

Next go to Project -> Add to Project, and now select all the .cpp files
that are in the sub-folder for the rules variant you plan to use, for
"C:\Dev-Cpp\atlantis\standard", and click on "Open" to add all of these
files to the project.

Now go to Project -> Project Options menu, and click on "Directories"
tab then click on the "Include Directories" sub-tab and add in the
paths for your atlantis sources (i.e. "C:\Dev-Cpp\atlantis") and also a
path to your rules set sub-folder (i.e. "C:\Dev-Cpp\atlantis\standard").
Then click OK.

Finally go to Execute -> Compile. Be patient, this will take some time to
complete. You will know it is done compiling when the Compile Log
minimizes.

You should now have an .exe of your project inside "C:\Dev-Cpp\standard".

You can quickly test your .exe build by creating a shortcut to the .exe
file, then right-click to select file properties, and append to the
"Target:" field the text " new", i.e. "C:\Dev-Cpp\test-build\atlantis-
standard.exe new". Then rename this shortcut "new". When you double-
click on the shortcut, you should be asked how wide the map should be,
and how tall the map should be. If there are no errors, there should be
a game.out and a players.out file in your test folder.

If you want to test further, you can now copy this shortcut and change
the appended Target parameter from " new" to " edit" and you can now
edit your generated world (if you make changes a new game.out file will
be created with the changes.)

I like to use Atlantis Advisor (home page
<http://gnawer.byte-force.yar.ru/advisor/>, announcements at
<http://games.groups.yahoo.com/group/atlantis_advisor_en>, and current
version at <http://atlantis.chol.ru/advisor_3.24.00.exe>) to test new
builds.

Atlantis Advisor allows you to run a local copy of the Atlantis
completely inside Windows. Run Atlantis Advisor, and select "Options"
and add the path to your .exe file, and then select "New Game" and click
on the "Local game" box. Atlantis Advisor will create a new world, allow
you to enter orders for a single faction, then you can select "Run next
local turn" from the file menu to run your commands. It only runs turns
for single user, but it is an excellent way to test out any changes that
you may have made to your rules files.


### 2.3 The gory details (for advanced users, or the terminally curious)

Atlantis is a standard C++ program, and is deliberately written with no
UI or other platform specific features. It does not handle email,
running turns, or adding players, which is all left up to the GM.

If you are building from your own project or makefile, you will need to
include both sets of source files in the makefile.  As such, simply
putting all of the source files into a project or makefile and compiling
it as a command-line application is about all there is to it.

Atlantis has a number of variant games, with different races, monsters
and features. For each of these games there is a subdirectory for all of
the rule set specific files; the common Atlantis engine files live in
the main directory.


## 3. Running a Game of Atlantis

The Atlantis program is actually very simple in terms of input and
output. It takes a set of files as input, runs the turn, and writes out
a set of files to the same directory. The Atlantis program does not do
anything in terms of scanning for email, sending email, or anything of
that nature. It is up to the gamemaster to either make sure the files
are in the right place, and the right emails get sent out, or he must
find or write a program to do that.

Explanation: Atlantis is designed to be a very generic program, that
will run on many computer systems, and in different ways. Different
computer systems handle email in different ways; making Atlantis work
on all of these different systems would be quite a task, and in my
opinion one that is best separated from the actual game-running program.

Further, nothing about Atlantis requires that it be an email game at
all; the engine can be used in many different ways. For example, a web
based version of Atlantis is available at <http://grelth.army-of-
darkness.it/>


### 3.1 Running a game of Atlantis by hand (the hard way)

I'll give this as a list of commented UNIX commands. The Windows
commands are very similar: move instead of mv, copy instead of cp, etc.

Firstly, you'll need to create a game directory outside your source
directory, to avoid it being cluttered up with random files. Assuming
that you want to run a havilah variant, and you're in the havilah
directory:

```
    bash-2.04$ mkdir ../../mygame
    bash-2.04$ cp havilah ../../mygame
    bash-2.04$ cd ../../mygame
    bash-2.04$ ./havilah new
```

At this point, you'll be asked how big the map should be. For a first
test game, you'll probably want a 16x16 world:

    Atlantis Engine Version: 5.1.0 (beta)
    Havilah, Version: 1.0.0 (beta)

    How wide should the map be?
    80
    How tall should the map be?
    80
    Making a level...

    ...Lots of other stuff about the world

So, what have you got now? You should have the game info for your new
game, stored in game.out, and the player info, stored in players.out.

```
    bash-2.04$ ls
    game.out  havilah  names.out  players.out`
```

In order to run our first turn, we need to do two things: feed havilah
the game and players file, and add ourselves as a player. First, the
easy part:

```
    mv game.out game.in
    mv players.out players.in
```

Atlantis will automatically read game.in and players.in when it runs.
Now for the second bit. Edit the players.in file with your favorite text
editor, and make it look like this:

    AtlantisPlayerStatus
    Version: 327936
    TurnNumber: 0
    GameStatus: New

    Faction: 1
    Name: The Guardsmen (1)
    Email: NoAddress
    Password: none
    LastOrders: 0
    FirstTurn: 0
    SendTimes: 1
    Template: long
    Faction: 2
    Name: Creatures (2)
    Email: NoAddress
    Password: none
    LastOrders: 0
    FirstTurn: 0
    SendTimes: 1
    Template: long
    Faction: new
    Name: Anthony
    Email: anthony@beastie.house
    Password: Test

Note: The first two factions are reserved for town guards and monsters,
which can cause some confusion. The first player faction will start as
faction number 3, the next as number 4, and so on.

Only the addition of the "Faction: new" line is necessary, but adding
yor faction name, email address and password will make your first turn
tidier.

Now when the game runs, it'll add you (well, me in this example) to the
game. Let's run it and see what happens:

```
    bash-2.04$ ./havilah run
    Atlantis Engine Version: 5.1.0
    Havilah, Version: 1.0.0 (beta)

    Saved Game Engine Version: 5.1.0
    Saved Rule-Set Version: 1.0.0 (beta)
    Reading the regions...
    Setting up the neighbors...
    Setting Up Turn...
    Reading the Gamemaster File...
    Reading the Orders File...
    QUITting Inactive Factions...
    Running the Turn...
    Running FIND Orders...
    Etcetera...
```

Now if you look in the directory, you should see the following:

```
    bash-2.04$ ls
    game.in   havilah    players.in   report.1  template.3
    game.out  names.out  players.out  report.3  times.<some number>
```

The game has updated the world, and stored it in game.out and players.
out. If you look at report.3, you should see your turn:

```
    bash-2.04$ more report.3
    Atlantis Report For:
    Anthony (3) (War 1, Trade 1, Magic 1)
    January, Year 1
```

If you want to submit orders for your first turn, you'd leave your
orders in a file called orders.3 in the directory, and follow similar
steps to the ones you just did to run your first turn. Move game.in,
players.in and any report files out of the way:

```
    mkdir 0
    mv game.in players.in report.* 0
```

And then do exactly what you did previously:

```
    mv game.out game.in
    mv players.out players.in
    ./havilah run
```

In later turns, you'll want to move old orders out of the way too.
The template files serves as a template for the orders for the next
turn.  This includes any @ recurring orders and any tern/endturn orders
you gave.  How this is made available to the players varies.  Some games
concatenate it to the end of the report file before sending that to the
player.  Some tools can work with that concetantion.


### 3.2 Running a game the easy way (with scripts)

Of course, this all gets very tedious, especially when running a game of
more than four or five people. There are a number of scripts that can
help you out. In it's simplest form, a script will look like this:

```
    #!/bin/bash
    #The argument tells you what turn is running
    gameturn=$1

    mkdir $gameturn
    mv game.in players.in report.* times.* $gameturn

    mv players.out players.in
    mv game.out game.in

    ./havilah run
    mv orders.* $gameturn
```

under windows, this script will look more like:

```
set ARGUMENT=%1

    mkdir %ARGUMENT%
    move game.in %ARGUMENT%
    move players.in %ARGUMENT%
    move report.* %ARGUMENT%
    move times.* %ARGUMENT%

    move players.out players.in
    move game.out game.in

    ./havilah run
    move orders.* %ARGUMENT%
```

These scripts will help you run turns by hand, by automating the tedious
moving and copying of files. Just run it as ./runturn <gameturn> (after
doing a chmod u+x runturn, of course).

Once you've got a handle on that, there are a number of scripts
available to help you automate most, if not all of the email side of
things too, which is how the 'big boys' run games of several hundred
players.

There are many scripts in the files/GMTools section of yahoo groups:
<http://groups.yahoo.com/group/atlantisdev/files/GM%20Tools/>. Getting
them up and running is outside the scope of this FAQ, and the authors
will do a better job of it too, so you should ask them if you are in any
doubt.


## 4. A GM's 'Code of Conduct'

Here is a list of things you should consider before running a game:
[thanks to Antony Briggs for posting this on atlantisdev]

Backups
The GM should keep backups of the game, preferably on a separate machine
or hard drive. Offsite backups are even better. Oh yeah, and they should
be automated if at all possible. If you don't know how to make backups,
or even what backups are, you shouldn't be running a game.

Back up *everything* - source code, executables, log files and your
scripts, as well as the game files, turn reports, etc. If you do have a
mishap, you don't want to have to remember your changes and then
recompile all of your code.

Automation
While it is possible to run an Atlantis game manually, it is far better
to use an automated script to respond to order submissions, and running
turns in general. Humans are fallible - you *will* send the wrong orders
out at 1AM. Computers won't, unless you mistakenly tell them to.

Email
Every effort should be made to make the game as accessible as possible,
but the GM is *not* responsible for the vagaries of email. Not even
slightly. If you're trying to get your turn in 5 minutes before the
deadline, and its mysteriously delayed, I am not going to rerun the turn.

Impartiality
The GM should make an effort to appear impartial, particularly when
handling disuptes between players. Playing in your own game, while
possible to do fairly, is generally frowned upon, and it makes it much
harder to punish people who are rude to you or your friends.

Communication
Generally it's a good idea to keep players in the loop. Group mailing
lists such as yahoogroups (and others) are ideal for this. You can just
maintain a list of email addresses for people who are playing, but this
requires effort on your part, which is always to be avoided.

You should always inform your players if:
   you need to rerun the turn,
   you need to delay the turn,
   you need to change the day or time that it runs,
   there is a change in the rules,
   a bug is discovered that affects play, or
   you need to ban someone.

It's generally considered good manners to inform the other players when
someone is banned, so that they can avoid a similar fate. Plus it helps
to keep them on their toes...

Finding bugs and bug reports
Bugs are generally difficult to avoid, and crop up even in well tested
code. There's always some player who wants to do things differently, and
when they do, they'll usually tickle some bug or other. When they give
you a bug report, make sure that you verify that the bug exists as they
have reported it - don't just take their word for it!

Fixing bugs
Once you know that it's definitely a bug, take steps to fix it. In a
test game it's normally straightforward, but in a running game you
should make the *minimum* change necessary to fix the problem. Don't be
tempted to 'fix' anything else - you'll introduce more problems. Also
don't be tempted to rush in a quick fix to get the game going again.
You're just as likely to create two more bugs if you do that.

Test your bug fixes
Above all, once you think that you've fixed the bug, test your fix
thoroughly to make sure that it all works. This normally means rerunning
the turn locally and verifying it before you set the turns going, as
well as checking the other turns too.

Don't just check the fix itself, either - check that you haven't broken
something else in the process of fixing the first bug. Nothing annoys
players more than having to rerun the turn three times because you
didn't test stuff - or worse, you fixed the bug but broke something
else, or fixed the wrong bug.

A final point - any bug fix that affects game play should be reported to
the players, so that they don't get caught out.

Rerunning turns
A rerun takes a lot of effort, inconveniences players and uses up
valuable network bandwidth. It should only be done as a last resort when
all other options, such as replacing units or skills, or compensation
paid into a player's unclaimed silver, have been exhausted.

A player messing up their orders, or not getting their turn in on time,
or misreading the rules is not grounds for a rerun.

All of the players should be informed as soon as humanly possible of the
rerun, the reason for the rerun, and if necessary, the revised deadline
for turn submissions (a really good idea). When the turn is actually
rerun, the subject line of everything that you send out should clearly
state that it is a rerun, so that players don't get confused.


## 5. Altering game rules

There may come a time when you ask yourself "Why do War factions get so
much darned silver?" or "Why are Balrogs so darned powerful?" In this
case, you've probably played and run a couple of games, and are looking
to tweak the rules! Welcome to the club ;)

The first step is to create your own subdirectory to store your game.
It's easier if you base your game on an existing variant. So do
something like 'cp -r havilah myhavilah', except swap 'myhavilah' for
something a little more original.

Once you've done that, have a closer look inside the subdirectory. The
two files that will interest you are extra.cpp and rules.cpp. They
contain all of the goodies that you'll want to tamper with. Don't worry
about tampering with forces beyond your ken. That's all part of the fun.

Assume that you wanted to make the two changes above. First, change the
default tax amount for war factions. Edit rules.cpp, and change this:

        50, /* TAX_INCOME */

to this:

        30, /* TAX_INCOME */

Hah! That'll learn those pesky war factions!

Balrogs are a little more tricky. You could add the following line to
the end of the ModifyTablesPerRuleset function in extra.cpp:

    DisableItem(I_BALROG);

This will disable Balrogs. Of course, none of the mages in your game
will be very amused by your little joke when they finally reach SUBA,
so we'd better do something a little more sensible. How about:

    modify_monster_attacks_and_hits(MONSTER_BALROG, 100, 100, 0);

Now any balrogs will have 100 hits, and do 100 melee attacks, instead of
200. That's pretty cool, but now they're just easier to harvest. D'oh!
Well, we can do something about that, too. Just add the following line:

    modify_monster_spoils(MONSTER_BALROG, 20000, IT_ADVANCED);

Cool - now Balrogs will only give out advanced spoils, in addition to
being wussier. It's important to balance these sorts of things out. A
wussy creature that explodes with treasure is just asking for some sort
of abuse ;)

Note also that we're using different 'values' for these functions -
DisableItem uses I_BALROG, but modify_monster uses MONSTER_BALROG instead.
The difference is which table it's indexing into.  The I_ values affect
things in the ItemDefs table. The spoils and treasure type of the
monster are in the MonDefs table. Enable/DisableItem work on Items
(and thus I_ is correct there).

Similarly, if you were modifying weapon characteristics, you would use
WEAPON_ for things like weapon class, etc. The name of the function tells
you which table you are working with. If you're in doubt, check out the
gamedata.h file in the main source tree.

Most settings in the game will either be boolean (ie true/false) or
integer values, but some will be enum types. What this means is that you
can set individual bits of a value. A good example is the new TRANSPORT
variable, used to set the behaviour of quartermasters:

    enum {
        ALLOW_TRANSPORT = 0x01, // Do we allow transport/distribute?
        QM_AFFECT_COST = 0x02, // QM level affect shipping cost?
        QM_AFFECT_DIST = 0x04, // QM level affect longrange dist?
    };
    int TRANSPORT;

If you wanted to switch on quartermasters, and have their level affect
shipping costs, but not distance, you would set the variable in rules.cpp
like so:

    GameDefs::ALLOW_TRANSPORT | GameDefs::QM_AFFECT_COST, // TRANSPORT

Note that the different values are set using a boolean OR |, not a
logical OR ||.

Once you've finished twisting the game to your own evil purposes, be sure
to let your players know of all the wonderful modifications you've made
to the game, so they can plan their tactics accordingly.

One final note on modifying Atlantis games: from time to time, Gamedefs
will be added to the source, usually in gamedefs.h and rules.cpp. If you
update your source, but not your custom game's rules.cpp file, your game
may not run any more, or will run strangely, which will make you sad.
Adding the new gamedefs in will make you happy again ;)



## 6. World Creation Guide

The following is not a technical guide but a description of the options
involved in Atlantis world creation. None of the information here is
relevant if you merely want to run a standard game. However if you as
GM wish to tweak the generation process to your wishes then the following
guide is meant as an introduction. World creation is currently an area
under code development - I strongly suggest you experiment with the
aspects of world creation that interest you most to find the settings
that are optimal for your game. Welcome to the art and science of Atlantis
world creation!

All of the following assumes that you have access to the GM report in order
to view the game map after creation - at least while you're experimenting.
You will have to make sure the gamedef GM_REPORT is set to 1. The GM_REPORT
will be created as 'report.1' in the atlantis directory - it's an omniscient
turn report.


### 6.1. Basics: world size, land mass and levels

The most important consideration for any game is world size as this will
determine the number of players that can comfortably join a game and the
amount of competition.

The amount of land hexes per player will give you a good indication whether
your game is too large or too small. You are prompted for the width and
height of the world when you run the program to generate a new world.

`# of surface hexes = width * (height / 2)`

The gamedef OCEAN_PERCENT is a rough estimate of how much of these will be
land mass:

`# of surface land hexes = width * (height / 2) * OCEAN_PERCENT / 100`

For instance with a default OCEAN_PERCENT of 60, a 128 x 128 map will have
8192 surface hexes and about 4915 land hexes. In standard Atlantis, a faction
can make use of 100 maximum. Reduce that somewhat for mixed factions, magic
factions and a little bit of friction and you might want to aim for 50 land
hexes per player. That means that a game this size would allow up to 100
players to expand and play comfortably (note: this is just a very rough
sketch and things can vary considerably on the play style chosen by your
players and other rules you may or may not have enforced in your game).

Underworld levels, while not being available to players immediately at
start, also have an effect on the total number of regions available for
play. They are set by the gamedefs UNDERWORLD_LEVELS (the number of
underworld levels beneath the surface), UNDERDEEP_LEVELS (the number of
underdeep levels) and ABYSS_LEVEL (this is just an on/off switch).
The underworld levels are only 1/2 the size in both coordinates of the
surface level (except the top one if there's more than one), underdeep
levels are 1/4th the size in both coordinates (except the topmost if
there's more than one - that will be 1/2 size), and the Abyss is just
a tiny area with a Black Keep for the standard victory conditions.


### 6.2. Land Mass Distribution

The distribution of land mass determines whether players will be able to
expand on a large continental mass and how important ships and sailing
will be in the game. Here's a description of the gamedefs that govern
land mass distribution and shape:

CONTINENT_SIZE: this gamedef will control how much land is allocated to
a continent at a time. The default is 16 (and less is recommended should
you decide to use the SEA_LIMIT gamedef). For smaller worlds this value
should be decreased or you'll find that all your land mass is allocated
to one or two continents only. For huge worlds it might be increased
correspondingly if you want to have continents fit for your world size.

ARCHIPELAGO: this gamedef controls how often land is created not as a
continent but as a small island chain. The size of the islands can range
from very small to a value that's linked with continent size but even
the bigger islands will tend to be much smaller than your continents.
So while an archipelago world might be created by setting CONTINENT_SIZE
to a really low value, setting ARCHIPELAGO will allow your world to have
both solid continents and islands at the same time. The value of
ARCHIPELAGO is treated as a percent chance that any attempt at creating
new land mass will end up forming an archipelago. Note that because
archipelagos are much smaller than continents, the overall land mass
allocated to them will be smaller than this gamedef value (i.e. an
ARCHIPELAGO setting of 50% might only allocate 30% of total land mass
to archipelagos).

SEA_LIMIT: standard Atlantis maps tend to show either a web of land
hexes against a backdrop of water or continents with a shotgun
pattern of ocean sprinkled in between. The SEA_LIMIT gamedef controls
an option that erases ocean hexes caught in a continental land mass
without a connection to the main ocean. The erased hexes get filled in
with land so continents will get blockier if you opt for this (which in
turn can be adjusted by tuning CONTINENT_SIZE downwards).
The value of SEA_LIMIT tells the game to erase any ocean hex that cannot
trace at least <SEA_LIMIT> number of ocean hexes in any direction.
Setting SEA_LIMIT to 3 will therefore erase all inland seas smaller than
three hexes in diameter but leave the larger ones intact. A value of 10
will erase most inland seas. The algorithm that implements SEA_LIMIT will
have to work considerably harder for higher values so that the time
needed to generate the world will rise exponentially with higher settings.
Please be advised to use sane values here and consider your computer's
processing power.

SEVER_LAND_BRIDGES: this option finishes what SEA_LIMIT begins - it
cuts the web-like strands of land between continents that riddle standard
Atlantis maps. The setting here governs the chance that any such  1-hex
wide links get severed. A setting between 15 and 40 is recommended here if
this option is desired. The effect of this is that Atlantis Ocean
becomes a much more continuous area and that the importance of sailing
is emphasised. Especially if PREVENT_SAIL_THROUGH is enabled it is
suggested that this gamedef is used as well.


### 6.3. Terrain Distribution

Terrain distribution is probably more a matter of style and flavor than
the aspects discussed in previous chapters. It can become a matter of
resource availability if not much land is available either due to
world or continent size.

Terrain types are allocated in 'world.cpp' in the function
GetRegType(ARegion *pReg). Depending on the region's level and distance
to the equator a different table of terrain type choices is used. If you
want to have more or less of a specific terrain or add in new types you'll
need to edit these tables. For most games using the standard terrain set
you won't need to look into this. (The exception to this are lakes which
are currently handled by their own gamedef - see below)

Terrain is distributed in the following fashion: in a first step a certain
number of 'seed' hexes are given a terrain type. In the second step the
allocated terrain grows outwards from these seed hexes into neighboring,
unallocated land hexes. Obviously the distance of these seed hexes will
determine how big or small the 'patches' of hexes of similar terrain will
be.

TERRAIN_GRANULARITY: this gamedef controls how distant to each other the
'seed' hexes are placed. A setting of 0 is default Atlantis behavior -
which means that exactly one seed hex is placed in each 4 x 4 area.
A setting higher than 0 means that each square of the settings' height
and width in hexes will have one seed hex (on average). A setting of '1'
will therefore create a different terrain in every hex, while a value
of '4' means that 1 out of 10 hexes will be a seed hex, and at TERRAIN_
GRANULARITY setting of '8' 1 out of 18 hexes will be a seed hex.
It's important to remember that seed hexes are placed randomly in their
area of placement, not in any determined pattern. It's therefore entirely
possible (if unlikely) that at a setting of '4' one patch of terrain will
extend 12 or more hexes in one direction (because the seeds are placed as far
apart as possible), while another is only 1 hex wide because the seeds
were placed right next to each other. There is also no guarantee that
neighboring seeds actually have different terrain types and any large
patch of one terrain could have been caused by a number of seeds of the
same type.
In short, TERRAIN_GRANULARITY is very useful to tailor your terrain detail
to world size. Remember: small values mean lots of terrain variety and
smaller patches of equal terrain, larger values mean considerably larger
stretches of terrain.

LAKES: lakes are a terrain type that is impassable by foot but can be
sailed on or flown over just like ocean. They are meant to introduce
added variety and some hindrance to land movement on the continents
where inland sea masses have been erased. They can be set so that all
adjacent hexes act as coastal terrain (having similar races and possibly
resources) by using the LAKESIDE_IS_COASTAL gamedef. Lakes can also
provide some economic benefit to adjacent land hexes, this is handled by
the LAKE_WAGE_EFFECT setting. Here we are concerned with lake placement.

The LAKES gamedef sets the chance that any given land hex is allocated
as lake and not as any other type of terrain. The basic chance for this
to happen depends on whether this hex has previously been an ocean hex
that has been filled in via the SEA_LIMIT option. In case it has, the
percent chance is equal to the LAKES setting. In all other cases, the
chance is LAKES / 10 + 1 %.
For instance, a LAKES setting of 16 will create a lake from a filled-in
inland sea hex with a 16% chance, and with a 2% chance from all others.
This creates areas with a higher abundance of lakes, usually on
main continents and not islands.

ODD_TERRAIN: this gamedef adds a sprinkle of chaos to your maps by
inserting single hexes of different terrain. The terrain type is still
allocated in the normal way - it might just be different from the patch
of terrain around it.
The value of ODD_TERRAIN is the chance that this happens, per hex, in
0.1%. A setting of '4' will therefore create interspersed terrain in
0.4% of all hexes. Note that the terrain type can of course end up to
be the same as that of the surrounding hexes.


### 6.4 Towns and Races

Towns are always a focus of economic activities in Atlantis and are
strategically important locations. The number and placement of towns
can therefore have a strong effect on the competition and feel of a
game. Races are a bit more of a flavor aspect but with a growing
trend towards custom games with special races and possibly racial
leaders their distribution will become much more important.

TOWN_PROBABILITY: this is the chance for creating towns compared to
standard Atlantis. '100' is the default setting. A value of '80'
means that only 80% of the usual number of towns will be created.

TOWN_SPREAD: normally towns are much more likely in plains than anywhere
else. This might or might not suit your game - why should your
dwarves have less towns just because they live in the mountains?
At the default value of '0', the chance for a town is directly
proportional to the terrain's economy value. At a TOWN_SPREAD setting
of '100' at the other end of the scale the chance is exactly the same
regardless of terrain. The effect is that other terrain types - which
are already economically less interesting than plains - become much
more viable as a base for factions.

TOWNS_NOT_ADJACENT: ever seen too many towns in one spot, especially
on those already rich plains? This gamedef is a possible check against
town clustering. TOWNS_NOT_ADJACENT is the percent chance that a
town will not be created if another one has already been created next
to it. A setting of '50' for instance will decrease the likelihood by
50%, and a setting of '100' and above will prevent towns from being
adjacent. Note that starting cities are an exception to this since
they are created by a different part of the code and are necessary
for the game.

LESS_ARCTIC_TOWNS: use this gamedef to reduce chances of towns near the
polar caps (within 9 hexes of the map border). The town chance is
reduced by approximately the LESS_ARCTIC_TOWNS setting x 3% times
9 minus the distance from the polar cap. For instance, depending on the
distance from the polar caps, the chances are reduced as follows:
Setting Chance at six hexes Chance at three hexes
1       - 22%                   - 53%
2       - 36%                   - 69%
3       - 46%                   - 77%
4       - 53%                   - 82%
5       - 58%                   - 85%
Towns that are created will also be considerably smaller than usual.

GROW_RACES: in standard Atlantis, a random race is allocated per hex.
With the GROW_RACES gamedef it's now possible to have populations
stretch out over several hexes. This alternative way of distributing
races works much like terrain distribution works now - races are set
up in 'seed' hexes and are afterwards 'grown' from them. The only
difference is that a race has a lower chance to grow into a terrain
that it isn't native to and when it does, it will have a lower
population than normal. This means that dwarves can live in the plains,
only that they don't settle there in great numbers and won't build
big cities there.
The GROW_RACES setting works much like TERRAIN_GRANULARITY works for
terrains, only that race anchors are set up regularly rather than
randomly. A greater setting for GROW_RACES means that a population
will spread over a wider area. Seafaring races can also spread across
ocean hexes, so that nearby islands often have the same race type.


### 6.5. Special: Fractal Generation

This chapter addresses the code developments for an alternate world
creation mechanism that is used by the Kingdoms gameset. The
alternate code can be found in the 'map.cpp' file in the gameset.

Fractal generation is an attempt to use a method that's popular for
realistic map generation (the 'diamond-square' method) in Atlantis
world generation. Before doing anything else the fractal world gen
creates a two dimensional graph of geographic parameters such as
elevation, humidity, temperature and vegetation. It then assigns
land and water mass as well as terrain types based on these geo-
graphic factors. The effect is that coastlines will look more
natural and that terrains are not just placed completely at random
but alternate according to local parameters. This creates more
variety while at the same time the map has a more realistic feel
to it. So much for the theory...

To use fractal generation in your game you must make sure the right
'map.cpp' file is in your gameset directory. The Kingdoms game uses
fractal map generation by default. Just copy the 'map.cpp' from
there into your game directory before compiling.

Fractal map generation uses all of the gamedefs and options
mentioned in the previous chapters. There are ways to tweak the
process but you'll need to edit the code directly.

In the currently supplied version of the code, the coastline is
generated in the standard way and only the terrain is fractal. This
method of map generation is still under construction and I found
the standard method of generating the coastline more customisable and
more useful for the game. This might be due to change as the code
develops. You can test the fractal land mass distribution by doing
the following: In 'map.cpp', near the top of the
ARegionList::MakeLand(..) function set the variable 'fractal' to 1.

Terrain distribution works by assigning each hex a probability value
per terrain type according to it's geographic parameters. The amount
of each terrain can be easily tweaked by adjusting the so-called
terrain type limits. In 'map.cpp', in the center loop of the function
ARegionList::SetFractalTerrain(..) you can find the limit variable
being set to '100' for each terrain. Setting the 'plain amount' to
'80' for instance will reduce the number of plains hexes. Similarly,
increasing a value will increase the overall number of that kind of
hex. Since terrain types sort of 'compete' for hexes with certain
other kinds regarding the geographic parameters, other terrain types
will be greatly affected by the changes you make. For instance, lowering
the value for 'jungles' will probably increase forests and swamps
greatly, and deserts and mountains to a lesser extent.

One problem with fractal terrain distribution is that sometimes
'fluke' worlds would have very few hexes of a single terrain - such
as 128 x 128 worlds with only four mountain hexes! This is due to
the partly random, partly deterministic nature of fractal generation
and I recommend you check the region count from the Atlantis console
output to check whether this has ocurred before using the generated
map for an actual game.

At the time this guide was written, a decent method for region naming
under the fractal world generation had not yet been implemented,
so you'll just find a different name for each region.

The author of fractal world generation, Jan Rietema, would be
grateful for your comments and suggestions and can be contacted at
<ravanrooke@yahoo.de>.
