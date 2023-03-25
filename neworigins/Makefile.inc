# bring in rule set objects
NEWORIGINS_OBJS := $(addprefix neworigins/obj/,$(RULESET))
# map dependencies
NEWORIGINS_DEP  := $(addprefix dep/neworigins/,$(RULSET:.o=.d))

# include dependencies
-include $(NEWORIGINS_DEP)

# add our dependencies to all
all: dep/neworigins neworigins/obj neworigins

# establish exe
neworigins: neworigins/neworigins

# add our objects to clean
clean::
	@rm -f $(NEWORIGINS_OBJS) neworigins/neworigins

# create directories
neworigins/obj:
	@mkdir $@

dep/neworigins:
	@mkdir $@

$(NEWORIGINS_OBJS): neworigins/obj/%.o: neworigins/%.cpp
	@$(CXXBUILD)

neworigins/neworigins: $(ALL_OBJS) $(NEWORIGINS_OBJS)
	@$(CXX) -o $@ $^

# nice name to generate rules
.PHONY: neworigins-rules
neworigins-rules: neworigins/html/neworigins.html

all-rules: neworigins-rules

# generate rules
neworigins/html/neworigins.html: neworigins/neworigins_intro.html neworigins/html/neworigin.css neworigins/neworigins
	(cd neworigins; \
	./neworigins genrules neworigins_intro.html neworigin.css html/neworigins.html)

