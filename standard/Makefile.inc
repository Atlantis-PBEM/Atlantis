# bring in rule set objects
STANDARD_OBJS := $(addprefix standard/obj/,$(RULESET))
# map dependencies
STANDARD_DEP  := $(addprefix dep/standard/,$(RULSET:.o=.d))

# include dependencies
-include $(STANDARD_DEP)

# add our dependencies to all
all: dep/standard standard/obj standard

# establish exe
standard: standard/standard

# add our objects to clean
clean::
	@rm -f $(STANDARD_OBJS) standard/standard

# create directories
standard/obj:
	@mkdir $@

dep/standard:
	@mkdir $@

$(STANDARD_OBJS): standard/obj/%.o: standard/%.cpp
	@$(CXXBUILD)

standard/standard: $(ALL_OBJS) $(STANDARD_OBJS)
	@$(CXX) -o $@ $^

# nice name to generate rules
.PHONY: standard-rules
standard-rules: standard/html/standard.html

all-rules: standard-rules

# generate rules
standard/html/standard.html: standard/standard_intro.html standard/html/standard.css standard/standard
	(cd standard; \
	./standard genrules standard_intro.html standard.css html/standard.html)

