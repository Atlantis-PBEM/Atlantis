# bring in rule set objects
KINGDOMS_OBJS := $(addprefix kingdoms/obj/,$(RULESET))
# map dependencies
KINGDOMS_DEP  := $(addprefix dep/kingdoms/,$(RULSET:.o=.d))

# include dependencies
-include $(KINGDOMS_DEP)

# add our dependencies to all
all: dep/kingdoms kingdoms/obj kingdoms

# establish exe
kingdoms: kingdoms/kingdoms

# add our objects to clean
clean::
	@rm -f $(KINGDOMS_OBJS) kingdoms/kingdoms

# create directories
kingdoms/obj:
	@mkdir $@

dep/kingdoms:
	@mkdir $@

$(KINGDOMS_OBJS): kingdoms/obj/%.o: kingdoms/%.cpp
	@$(CXXBUILD)

kingdoms/kingdoms: $(ALL_OBJS) $(KINGDOMS_OBJS)
	@$(CXX) -o $@ $^

# nice name to generate rules
.PHONY: kingdoms-rules
kingdoms-rules: kingdoms/html/kingdoms.html

all-rules: kingdoms-rules

# generate rules
kingdoms/html/kingdoms.html: kingdoms/kingdoms_intro.html kingdoms/html/kingdoms.css kingdoms/kingdoms
	(cd kingdoms; \
	./kingdoms genrules kingdoms_intro.html kingdoms.css html/kingdoms.html)

