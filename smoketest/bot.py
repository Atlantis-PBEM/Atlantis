def generateturn(report, template):
    """Given a report and a template, return a set of orders as a string."""
    firstunit = 'no'
    orders = ''
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
            temp = int(random.random()*len(directions))
            temp2 = int(random.random()*len(directions))
            orders += "move "+directions[temp]+" "+directions[temp2]+"\n"
            
            # 1. do nothing (ie work/tax if not enough silver)
            #orders += "@work\n"
            # 2. study combat/buy men (if it has enough)
            # 3. form a new unit (if it has more than 10 men?)
            # 50% chance of new unit
            if random.random() >= 0.5:
                unitnum = int(random.random()*1000)+1
                #orders += "give new "+str(unitnum)+" 100 silv"
                formstring = "form "+str(unitnum)+"\n"
                formstring += '  name unit "Captain Random"\n  claim 100\n  buy 1 peas\n'
                formstring += 'end\n\n'
                orders += formstring
    return orders

