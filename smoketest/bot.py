import random, re

directions = ['n','s','se','sw','ne','nw']

directiondict = {'North':'n', 'Northeast':'ne', 'Northwest':'nw',
                 'South':'s', 'Southeast':'se', 'Southwest':'sw' }

attitudes = ['hostile', 'unfriendly', 'neutral', 'friendly', 'ally',]
                
def generateturn(report, template):
    """Given a report and a template, return a set of orders as a string."""
    firstunit = 'no'
    orders = ''
    maxfactionfound = 0
    
    # Need to concatenate lines in the report if they don't have a . on the end!
    tempreport = []
    temp = ''
    for line in report:
        if len(line) > 2 and (line[-2] == '.' or line[-2] == '-' or line[-2] == ':'): # -2 since we'll have \n's at the end
            withoutspaces = re.sub('\s+',' ',temp+line)
            tempreport.append(withoutspaces)
            temp=''
        else:
            temp += line[:-1] + ' '*(temp!='')
    report = tempreport
    
    #print
    #for line in report:
    #    print line
    #print
    
    # Here we parse the report and template lists to get out relevant info.
    # First, set up some regexps to get out relevant info.
    
    regionpattern = re.compile(r'''(\w+)  # initial region
\s+                     # then a space
\((\d+),(\d+)\)         # x,y coords
\s+in\s+(.*?)           # region name
,\s+(\d+)\s+peasants\s+ # number of peasants
\((.*)\),\s+            # racial type
\$(\d+)\.               # max tax
''', re.VERBOSE)

    citypattern = re.compile(r'''(\w+)  # initial region
\s+                     # then a space
\((\d+),(\d+)\)         # x,y coords
\s+in\s+(.*?)           # region name
,\s+contains\s+(.*)\s+\[(\w+)\]  # city details
,\s+(\d+)\s+peasants\s+ # number of peasants
\((.*)\),\s+            # racial type
\$(\d+)\.               # max tax
''', re.VERBOSE)

    dirpattern = re.compile(r'''\s+   # initial tab
(\w+)                   # Direction
\s+\:\s+
(\w+)                   # Terrain
\s+\(
(\d+,\d+)               # x,y loc
\)\s+in\s+
(\w+)\.                 # province
''', re.VERBOSE)

    dircitypattern = re.compile(r'''\s+   # initial tab
(\w+)                   # Direction
\s+\:\s+
(\w+)                   # Terrain
\s+\(
(\d+,\d+)               # x,y loc
\)\s+in\s+
(\w+)                   # province
,\s+contains\s+
(\w+)
''', re.VERBOSE)

# Exits:
#  North : jungle (29,81) in Chedaru.
#  Northeast : jungle (30,82) in Chedaru.
#  Southeast : jungle (30,84) in Chedaru, contains Vesler [town].
#  South : jungle (29,85) in Chedaru.
#  Southwest : jungle (28,84) in Chedaru.
#  Northwest : jungle (28,82) in Chedaru.

    # Now, go through each region and pull out info
    # Might be easier to do with split/string stuff, too
    region = {}
    units={}
    unitbynum={}
    dictstring=None
    for line in report:
        lineindex = report.index(line)
        if lineindex+1 < len(report) and report[lineindex+1].startswith('---'):
            #the current line must be the start of a region
            print "Found a region:",report[lineindex]
            
            wibble = regionpattern.search(report[lineindex][:-1])
            if wibble != None:
                #print line
                #print "***MATCH***: ",wibble.groups()
                
                thisregion = {}
                thisregion['terrain']=wibble.groups()[0]
                thisregion['x']=wibble.groups()[1]
                thisregion['y']=wibble.groups()[2]
                thisregion['province']=wibble.groups()[3]
                thisregion['settlename']=None
                thisregion['settletype']=None
                thisregion['pop']=wibble.groups()[4]
                thisregion['race']=wibble.groups()[5]
                thisregion['maxtax']=wibble.groups()[6]
                
                
            if wibble == None:
                wibble = citypattern.search(report[lineindex][:-1])
                if wibble != None:
                    #print line
                    #print "***MATCH***: ",wibble2.groups()
                    
                    thisregion = {}
                    thisregion['type']=wibble.groups()[0]
                    thisregion['x']=wibble.groups()[1]
                    thisregion['y']=wibble.groups()[2]
                    thisregion['province']=wibble.groups()[3]
                    thisregion['settlename']=wibble.groups()[4]
                    thisregion['settletype']=wibble.groups()[5]
                    thisregion['pop']=wibble.groups()[6]
                    thisregion['race']=wibble.groups()[7]
                    thisregion['maxtax']=wibble.groups()[8]
            
            if wibble == None:
                continue
            #Weather
            #print report[lineindex+2][:-1].split(';', 1)
            weather = report[lineindex+2][:-1].split(';', 1)
            if weather[0] == '  The weather was clear last month':
                thisregion['lastweather'] = 'clear'
                #print "Clear last month!"
            else:
                thisregion['lastweather'] = 'not clear'
                #print "Not clear last month!"
            if weather[1] == ' it will be clear next month.':
                thisregion['weather'] = 'clear'
                #print "Clear this month!"
            else:
                thisregion['weather'] = 'not clear'
                #print "Not clear this month!"
            
            #Wages
            if report[lineindex+3][:-1] != ' Wages: $0.':
                wages = re.search('\$(\d+).*?\$(\d+)', report[lineindex+3])
                if wages != None:
                    thisregion['wages'] = int(wages.groups()[0])
                    thisregion['wagesmax'] = int(wages.groups()[1])
                    #print "Wages are $"+str(thisregion['wages'])
                    #print "Wage max is $"+str(thisregion['wagesmax'])
            else:
                #print "No Wages!"
                thisregion['wages'] = 0
                thisregion['wagesmax'] = 0
                
            #Wanted and For Sale
            thisregion['wanted'] = report[lineindex+4]
            thisregion['forsale'] = report[lineindex+5]
            
            #Entertainment
            ente = re.search('\$(\d+)\.', report[lineindex+6])
            if ente != None:
                thisregion['entertain'] = int(ente.groups()[0])
                #print "Entertainment: ", thisregion['entertain']
            #thisregion['entertain'] = report[lineindex+6]
            
            #Products
            ignored,stuff = report[lineindex+7].split(': ', 1)
            itemlist = stuff.split(', ')
            #print itemlist
            thisregion['products'] = itemlist
            
            #Exits
            #exitlist = ['North','Northeast','Southeast','South','Southwest','Northwest']
            hexdirections = {}
            counter = lineindex+9
            while 1:
                try:
                    temp,rest = report[counter].split(' : ', 1)
                    temp = temp.strip()
                    #print ">"+temp+"<"
                    if temp in directiondict.keys():
                        #print "Found an exit!", temp
                        # NB: this assumes that terrain is one word...
                        terrain, coord, ignored, regionname = rest.split(' ', 3)
                        hexdirections[temp]=[coord, terrain, regionname]
                except ValueError:
                    pass
                
                counter += 1
                if report[counter].startswith('*'):
                    thisregion['directions'] = hexdirections
                    break
            
            #Now that we have all the info, commit it to 'memory'
            dictstring = thisregion['x']+','+thisregion['y']
            region[dictstring]=thisregion
            
            # and initialise the list of units
            units[dictstring]=[]
        
        # Now for the units...
        unitstarts = ['= ', ': ', '- ', '% ', '! ', '* ']
        if  line[:2] in unitstarts:                
            unit = {}
            unit['skills']=[]
            unit['items']=[]
            
            if line.startswith('*'):
                unit['mine']='yes'
            else:
                unit['mine']='no'
            
            # If the next line begins with two spaces, 
            # we know we've split a line at the wrong point
            lineindex = report.index(line)+1
            while lineindex<len(report) and report[lineindex].startswith(' '):
                #print "joined up some lines"
                line += report[lineindex]
                lineindex += 1
                
            #print "***MATCH***: found a unit:"
            #print line
            
            # what about comments? split the comment off first,
            # since we might have ., in the comment
            if ';' in line:
                line, ignored = line.split(';')
            
            temp = line.split('.')
            
            if len(temp)==5:  # Normal unit
                unit['main'], unit['weight'], unit['capacity'], unit['skillbit'], ignored = line.split('.')
                unit['combatspell']=unit['spells']=None
                #print "Found a normal unit!"
            elif len(temp)==2:  #Not one of our units
                unit['main'], ignored = line.split('.')
                unit['combatspell']=unit['spells']=unit['weight']=unit['capacity']=unit['skillbit']=None
                #print "Found another faction's unit!"
            elif len(temp)==6:  # Mage unit with no combat spell
                unit['main'], unit['weight'], unit['capacity'], unit['skillbit'], unit['spells'], ignored = line.split('.')
                unit['combatspell'] = None
                #print "Found a mage unit! (no combat spell)"
            elif len(temp)==7:  # Mage unit
                unit['main'], unit['weight'], unit['capacity'], unit['skillbit'], unit['combatspell'], unit['spells'], ignored = line.split('.')
                #print "Found a mage unit!"
            else:
                #print "Found a wacky unit, with "+str(len(temp))+" parts:",line
                unit['main'], ignored = line.split('.')
                unit['combatspell']=unit['spells']=unit['weight']=unit['capacity']=unit['skillbit']=None
                continue
            
            #Grab the unit number and faction number
            wibble = re.search('.*\((\d+)\).*\((\d+)\)', unit['main'])
            if wibble != None and len(wibble.groups()) == 2:
                unit['faction'] = int(wibble.groups()[1])
                unit['unitnum'] = int(wibble.groups()[0])
                #print "Found numbers: FN ==",unit['faction'],"and UN ==",unit['unitnum']
                if unit['faction'] > maxfactionfound:
                    maxfactionfound = unit['faction']
            
            else:
                wibble = re.search('\((\d+)\)', unit['main'])
                if wibble != None and len(wibble.groups()) == 1:
                    unit['unitnum'] = int(wibble.groups()[0])
                    unit['faction'] = 0
                    #print "Found non-revealing unit: UN ==",unit['unitnum']
            
            
            itemlist=unit['main'].split(',')
            #print itemlist
            for thingo in itemlist:
                if '[' in thingo:
                    #print thingo, "is an item!"
                    unit['items'].append(thingo)
            
            if unit['skillbit'] != None:
                itemlist=unit['skillbit'].split(',')
                #print itemlist
                for thingo in itemlist:
                    if '[' in thingo:
                        #print thingo, "is a skill!"
                        unit['skills'].append(thingo)
                        
            # Add the unit to unitbynum
            unitbynum[unit['unitnum']] = unit
            
            # Need the region dictstring at this point... should be ok
            if dictstring==None: #we're in the nexus
                if 'nexus' not in units.keys():
                    units['nexus']=[]
                units['nexus'].append(unit)
            else:
                units[dictstring].append(unit)
                
    #print region
    #print
    #print units
    
    #print unitbynum.keys()
    
    # Read each line of the template.
    for line in template:
        if line.startswith('#end'):  #end of file, or end of orders
            orders += "; Found the end!\n"
        
        orders += line  # we'll always want to include the template line
        
        # When it gets to a unit, that unit will either
        if line.startswith("unit"):
            orders += "; Found a unit!\n"
            ignored, unitnumber = line.split(' ',1)
            unitnumber = int(unitnumber.strip())
            #print ">>>", unitnumber
            #print unitbynum[unitnumber]
            
            if firstunit == 'no':
                orders += "option template map\n"
                orders += "option notimes\n"
                orders += "option showattitudes\n"
                
                #declare towards a random faction
                decfaction = int(random.random()*maxfactionfound)
                orders += "declare "+str(decfaction)+" "
                
                decattitude = int(random.random()*len(attitudes))
                orders += attitudes[decattitude] + "\n"
                
                #orders += "declare default hostile\n"
                #orders += "declare 1 neutral\n"
                #orders += "declare 2 neutral\n"
                firstunit = 'found'
            
            #set units to avoid and behind -- hopefully that way 
            # they'll stay alive long enough for me to check that
            # the '!' is prepended to hostile units ;)
            orders += "avoid 1\nbehind 1\n"
            
            # for a test, we'll move around randomly
            if random.random() >= 0.5:
                temp = int(random.random()*len(directions))
                temp2 = int(random.random()*len(directions))
                orders += "move "+directions[temp]+" "+directions[temp2]+"\n"
            elif random.random() >= 0.1:
                orders += "work\n"
            else:
                orders += "study combat\n"
            # 1. do nothing (ie work/tax if not enough silver)
            #orders += "@work\n"
            # 2. study combat/buy men (if it has enough)
            # 3. form a new unit (if it has more than 10 men?)
            # 50% chance of new unit
            orders += "tax\n"
            if random.random() >= 0.8:
                unitnum = int(random.random()*1000)+1
                #orders += "give new "+str(unitnum)+" 100 silv"
                formstring = "form "+str(unitnum)+"\n"
                formstring += '  name unit "Captain Random"\n  claim 100\n  buy 1 peas\n'
                formstring += 'end\n\n'
                orders += formstring
    return orders

