# Thera - A series of tools to run an atlantis game.
# Copyright (C) 2001 Anthony Briggs
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# The author may be contacted via email at abriggs@sourceforge.net.au


# This is the checkpassword script. Given a game name, along with
# a faction name or number, and a password, it'll go through players.in and
# return 'ok' if there's a player with that name/number and password in
# that game, and something else if there isn't, or if the password's wrong.
#
# Note that it won't check the gamename, other than returning 'no' if the 
# gamename is wrong.
#
# In addition, the calling script is responsible for doing lockfile stuff.

import re
#from mylog import log
#from config import server, game

def setplayerinfo(factionnumber, attribute, value):
    """ Sets a piece of information for a specific player, in a specific game.
    Returns either ok, 'invalid' if the attribute is wrong, 
    or Unknown, if there's no such player."""
    
    # This is a butchered version, specifically for the smoketest
    
    startstring = { 'name':'Name: ', 
                    'email':'Email: ', 
                    'password':'Password: ', 
                    'lastorders':'LastOrders: ',
                    'sendtimes':'SendTimes: ',
                    'ready':'Ready: ',
                    'timesreward':'RewardTimes: '}
    
    if attribute not in startstring.keys():
        return 'invalid'
    
    if factionnumber == 1 or factionnumber == 2:
        return 'invalid'
        
    playersfile = open('players.out', 'r')
    searchstring = 'Faction: '+str(factionnumber)
    
    output = ''
    #Now read through the players.in file
    while 1:
        tempstring = playersfile.readline()
        output = output + tempstring
        if tempstring == '':
            return 'unknown'
        if re.search(searchstring, tempstring) != None:
            #info['name'] = playersfile.readline()[6:-1]
            break
    
    #Right, we're into the relevant player bit
    lookingfor = startstring[attribute]
    foundit = 'no'
    tempstring = ''

    while 1:    
        tempstring = playersfile.readline()
        if tempstring[:len(lookingfor)] == lookingfor:
            output = output + startstring[attribute] + str(value) + '\n'
            foundit = 'yes'
            #Since we haven't read in all of the rest, we can safely break here.
            break
        if tempstring[:9] == 'Faction: ':
            #got to the end of the player's settings
            break
        if tempstring == '':
            #got to the end of the file
            output = output + '\n'
            break
        #otherwise, it's a setting that we're not looking for
        output = output + tempstring
    
    if foundit == 'no':
        # if we haven't found the string, it must not be in here
        # so we have to create it ourselves
        output = output + startstring[attribute] + str(value) + '\n'
        # And we'll still have 'Faction: ' in the tempstring...
        output = output + tempstring
    
    #Get all of the rest of the players.in file
    while 1:
        tempstring = playersfile.readline()
        output = output + tempstring
        if tempstring == '':
            if output[-1] != '\n':
                output = output + '\n'
            playersfile.close()
            outputfile = open('players.out', 'w')
            outputfile.write(output)
            outputfile.close()
            return 'ok'

    #won't get here...

