#!/usr/local/bin/python

# This is a simple script to smoke test a new Atlantis build.

# There are a number of steps:

# 1. cvs update
# 2. make clean
# 3. make
# 4. make a test dir and copy the atlantis binary into it
# 5. make clean again?
# 6. cd to the test dir
# 7. add players into the players.in file
# 8. run a turn
# 9. move the old order files into a directory
# 10. parse reports
# 11. create new orders
# 12. save orders
# 13. move old reports
# 14. unless we've run enough turns, jump to step 8

# At some later stage, we'll probably want to check that orders 
# come back correctly - eg. if we issue a (possibly deliberately)
# bogus move order, we want to make sure that it fails, and issue
# appropriate warnings if it doesn't.

import os, sys, time, random
from checkpassword import setplayerinfo
from bot import generateturn

# First, get our arguments. they'll tell us what parts we want to run

args = sys.argv

if len(args) == 1 or "--help" in args:
    # print usage message and quit
    print "You need to feed smoketest some arguments to tell it what to do:"
    print "  --turns=...      The number of turns that you want to run"
    print "  --players=...    The number of test players in the game"
    print "  --cvs            Download up-to-date sources from the CVS"
    print "  --make           Make a new version of the code"
    print "  --test           Test the code"
    print "  --continue       Continue from a previous run"
    print "  --game=<name>    Make a particular version of the game"
    print "                   (Standard is the default)"
    print "  --all            A combination of the cvs, make and test options"
    sys.exit(0)

# Get the game name, number of turns and number of players if they exist
gamename = None
numturns = None
numplayers = None

for item in args:
    if item.startswith('--game='):
        gamename = item[7:]
    if item.startswith('--turns='):
        numturns = int(item[8:])
        print numturns
    if item.startswith('--players='):
        numplayers = int(item[10:])
        print numplayers

if gamename == None:
    print "Game name defaulted to standard"
    gamename = 'standard'
if numturns == None:
    print "Number of turns defaulted to 10"
    numturns = 10
if numplayers == None:
    print "Number of players defaulted to 10"
    numplayers = 10


# Now start running stuff...
if "--cvs" in args or "--all" in args:
    # Use the shell to update the source
    os.chdir("..")
    os.system('cvs -z3 update')
    os.chdir("smoketest")

if "--make" in args or "--all" in args:
    # we can do a make to redo the code
    os.chdir("..")
    print "now entering directory"
    os.system('pwd')
    os.system('gmake GAME='+gamename)
    if os.access(gamename+'/'+gamename, os.F_OK) != 1:
        print "There was some problem with building the game source"
        sys.exit(2)
    # Try and build the html too, but if we can't, don't halt
    os.system('gmake GAME='+gamename+' rules')
    if os.access(gamename+'/html/'+gamename+'.html', os.F_OK) != 1:
        print "There was some problem with building the game rules into HTML"
    os.chdir("smoketest")

if not ("--test" in args or "--continue" in args or "--all" in args):
    print "Skipping testing ... like a real man"
    sys.exit(0)

# Now, we test
if "--continue" not in args:
    # start by preparing the testing directory
    os.system('rm -r testing')
    os.mkdir("testing")
    os.chdir("testing")
    
    # now copy the game binary and html file
    os.system('cp ../../'+gamename+'/'+gamename+' .')
    os.system('cp ../../'+gamename+'/html/'+gamename+'.html .')
    
    # now run some tests...
    print "You'll need to feed in the values for the size of the game map"
    os.system('./'+gamename+' new')
    
    if os.access('game.out', os.F_OK) != 1:
        print "There was some problem with writing the game.out file"
        sys.exit(3)
    if os.access('players.out', os.F_OK) != 1:
        print "There was some problem with writing the players.out file"
        sys.exit(3)

    os.rename('game.out','game.in')
    os.rename('players.out','players.in')


    # 7. add players into the players.in file
    players = []
    for index in range(numplayers):
        players.append('Player '+str(index+1))
    
    playersfile = open('players.in','a+')
    for player in players:
        entry = "Faction: new\nName: "+player+"\nEmail: nobody@example.com\nPassword: "
        entry += player + "\nLastorders: 0\n"
        playersfile.write(entry)
    playersfile.close()
else:
    os.chdir("testing")

# Now we can start looping and running turns.
print "Running "+str(numturns)+" turns.."

if "--continue" in args:
    # get the current turn from players.in
    tempfile = open( 'players.in', 'r')
    ignored = tempfile.readline()
    ignored = tempfile.readline()
    thisturn = int( tempfile.readline()[12:] )
    tempfile.close()
    numturns = thisturn + numturns
else:
	thisturn = 0

while thisturn <= numturns:
    print "Turn",thisturn,"of",numturns
    
    # Move turn reports and templates
    if os.access(str(thisturn), os.F_OK) == 1:
        os.system('rm -r '+str(thisturn))
    os.system('mkdir '+str(thisturn))
    os.system('mv report.* '+str(thisturn))
    os.system('mv template.* '+str(thisturn))
    
    # run a turn
    os.system('./'+gamename+' run') # > /dev/null')
    
    # 9. move the old order files into a directory
    os.system('mv orders.* '+str(thisturn))
    
    # 11. create new orders
    for index in range(numplayers):
        print "Player "+str(index+1)+":"
        
        if (os.access('report.'+str(index+3), os.F_OK) != 1 or 
           os.access('template.'+str(index+3), os.F_OK) != 1):
            print "The player has been eliminated!"
            continue

        # update the players.in file so that the players don't time out.
        # ie, Set the LastOrders field
        setplayerinfo(index+3, 'lastorders', str(thisturn))
        
        #print "opening template for player",index+1,": template."+str(index+3)
        templatefile = open('template.'+str(index+3), 'r') # readonly
        template = templatefile.readlines()
        
        #print "opening report for player",index+1,": report."+str(index+3)
        reportfile = open('report.'+str(index+3), 'r') # readonly
        report = reportfile.readlines()
        
        orders = generateturn(report, template)
        
        #print "writing orders for player",index+1,": orders."+str(index+3)
        orderfile = open('orders.'+str(index+3), 'w+') # overwrite
        orderfile.write(orders)
        orderfile.close()
    
    # 13. move old reports
    os.system('mv game.in '+str(thisturn))
    os.system('mv game.out game.in')
    os.system('mv players.in '+str(thisturn))
    os.system('mv players.out players.in')
    
    thisturn += 1

