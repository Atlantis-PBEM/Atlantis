import random, re

directions = ['n','s','se','sw','ne','nw']

def generateturn(report, template):
    """Given a report and a template, return a set of orders as a string."""
    firstunit = 'no'
    orders = ''
    
    # Need to concatenate lines in the report if they don't have a . on the end!
    tempreport = []
    temp = ''
    for line in report:
        if len(line) > 2 and line[-2] == '.': # -2 since we'll have \n's at the end
            withoutspaces = re.sub('\s+',' ',temp+line)
            tempreport.append(withoutspaces)
            temp=''
        else:
            temp += line[:-1] + ' '*(temp!='')
    report = tempreport
    
    # Here we parse the report and template lists to get out relevant info.
    # First, set up some regexps to get out relevant info.
    
    regionpattern = re.compile(r'''(\w+)  # initial region
\s+                     # then a space
\((\d+),(\d+)\)         # x,y coords
\s+in\s+(\w+)           # region name
,\s+(\d+)\s+peasants\s+ # number of peasants
\((.*)\),\s+            # racial type
\$(\d+)\.               # max tax
''', re.VERBOSE)

    citypattern = re.compile(r'''(\w+)  # initial region
\s+                     # then a space
\((\d+),(\d+)\)         # x,y coords
\s+in\s+(\w+)           # region name
,\s+contains\s+(.*)\s+\[(\w+)\]  # city details
,\s+(\d+)\s+peasants\s+ # number of peasants
\((.*)\),\s+            # racial type
\$(\d+)\.               # max tax
''', re.VERBOSE)


    # Now, go through each region and pull out info
    # Might be easier to do with split/string stuff, too
    region = {}
    units={}
    dictstring=None
    for line in report:
        if line.startswith('--'):
            #the previous line must be the start of a region
            lineindex = report.index(line)
            print "Found a region:",report[lineindex-1]
            
        wibble = regionpattern.search(line[:-1])
        if wibble != None:
            #print line
            #print "***MATCH***: ",wibble.groups()
            
            thisregion = {}
            thisregion['type']=wibble.groups()[0]
            thisregion['x']=wibble.groups()[1]
            thisregion['y']=wibble.groups()[2]
            thisregion['province']=wibble.groups()[3]
            thisregion['settlename']=None
            thisregion['settletype']=None
            thisregion['pop']=wibble.groups()[4]
            thisregion['race']=wibble.groups()[5]
            thisregion['maxtax']=wibble.groups()[6]
            
            dictstring = wibble.groups()[1]+','+wibble.groups()[2]
            region[dictstring]=thisregion
            units[dictstring]=[]
            
        wibble2 = citypattern.search(line[:-1])
        if wibble2 != None:
            #print line
            #print "***MATCH***: ",wibble2.groups()
            
            thisregion = {}
            thisregion['type']=wibble2.groups()[0]
            thisregion['x']=wibble2.groups()[1]
            thisregion['y']=wibble2.groups()[2]
            thisregion['province']=wibble2.groups()[3]
            thisregion['settlename']=wibble2.groups()[4]
            thisregion['settletype']=wibble2.groups()[5]
            thisregion['pop']=wibble2.groups()[6]
            thisregion['race']=wibble2.groups()[7]
            thisregion['maxtax']=wibble2.groups()[8]
            
            dictstring = wibble2.groups()[1]+','+wibble2.groups()[2]
            region[dictstring]=thisregion
            units[dictstring]=[]
        
        # Now for the units...
        if line.startswith('* ') or line.startswith('- '):
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
            elif len(temp)==2:  #Not one of our units
                unit['main'], ignored = line.split('.')
                unit['combatspell']=unit['spells']=unit['weight']=unit['capacity']=unit['skillbit']=None
            elif len(temp)==7:  # Mage unit
                unit['main'], unit['weight'], unit['capacity'], unit['skillbit'], unit['combatspell'], unit['spells'], ignored = line.split('.')
            else:
                #print "Found a wacky unit:",line
                continue
                
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
            
            # Need the region dictstring at this point... should be ok
            if dictstring==None: #we're in the nexus
                if 'nexus' not in units.keys():
                    units['nexus']=[]
                units['nexus'].append(unit)
            else:
                units[dictstring].append(unit)
            #print
    
    #print region
    #print
    #print units
    
    
    # Read each line of the template.
    for line in template:
        if line.startswith('#end'):  #end of file, or end of orders
            orders += "; Found the end!\n"
        
        orders += line  # we'll always want to include the template line
        
        # When it gets to a unit, that unit will either
        if line.startswith("unit"):
            orders += "; Found a unit!\n"
            
            if firstunit == 'no':
                orders += "option template map\n"
                orders += "option notimes\n"
                #orders += "declare default hostile\n"
                #orders += "declare 1 neutral\n"
                #orders += "declare 2 neutral\n"
                firstunit = 'found'
            
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

