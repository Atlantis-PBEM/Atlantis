# bring in rule set objects
BASIC_OBJS := $(addprefix basic/obj/,$(RULESET))
# map dependencies
BASIC_DEP  := $(addprefix dep/basic/,$(RULSET:.o=.d))

# include dependencies
-include $(BASIC_DEP)

# add our dependencies to all
all: dep/basic basic/obj basic

# establish exe
basic: basic/basic

# add our objects to clean
clean::
	@rm -f $(BASIC_OBJS) basic/basic

# create directories
basic/obj:
	@mkdir $@

dep/basic:
	@mkdir $@

$(BASIC_OBJS): basic/obj/%.o: basic/%.cpp
	@$(CXXBUILD)

basic/basic: $(ALL_OBJS) $(BASIC_OBJS)
	@$(CXX) -o $@ $^

# nice name to generate rules
.PHONY: basic-rules
basic-rules: basic/html/basic.html

all-rules: basic-rules

# generate rules
basic/html/basic.html: basic/basic_intro.html basic/html/basic.css basic/basic
	(cd basic; \
	./basic genrules basic_intro.html basic.css html/basic.html)

