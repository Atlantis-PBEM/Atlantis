# bring in rule set objects
FRACAS_OBJS := $(addprefix fracas/obj/,$(RULESET))
# map dependencies
FRACAS_DEP  := $(addprefix dep/fracas/,$(RULSET:.o=.d))

# include dependencies
-include $(FRACAS_DEP)

# add our dependencies to all
all: dep/fracas fracas/obj fracas

# establish exe
fracas: fracas/fracas

# add our objects to clean
clean::
	@rm -f $(FRACAS_OBJS) fracas/fracas

# create directories
fracas/obj:
	@mkdir $@

dep/fracas:
	@mkdir $@

$(FRACAS_OBJS): fracas/obj/%.o: fracas/%.cpp
	@$(CXXBUILD)

fracas/fracas: $(ALL_OBJS) $(FRACAS_OBJS)
	@$(CXX) -o $@ $^

# nice name to generate rules
.PHONY: fracas-rules
fracas-rules: fracas/html/fracas.html

all-rules: fracas-rules

# generate rules
fracas/html/fracas.html: fracas/fracas_intro.html fracas/html/fracas.css fracas/fracas
	(cd fracas; \
	./fracas genrules fracas_intro.html fracas.css html/fracas.html)

