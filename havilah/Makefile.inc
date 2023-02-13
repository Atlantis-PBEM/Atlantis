# bring in rule set objects
HAVILAH_OBJS := $(addprefix havilah/obj/,$(RULESET))
# map dependencies
HAVILAH_DEP  := $(addprefix dep/havilah/,$(RULSET:.o=.d))

# include dependencies
-include $(HAVILAH_DEP)

# add our dependencies to all
all: dep/havilah havilah/obj havilah

# establish exe
havilah: havilah/havilah

# add our objects to clean
clean::
	@rm -f $(HAVILAH_OBJS) havilah/havilah

# create directories
havilah/obj:
	@mkdir $@

dep/havilah:
	@mkdir $@

$(HAVILAH_OBJS): havilah/obj/%.o: havilah/%.cpp
	@$(CXXBUILD)

havilah/havilah: $(ALL_OBJS) $(HAVILAH_OBJS)
	@$(CXX) -o $@ $^

# nice name to generate rules
.PHONY: havilah-rules
havilah-rules: havilah/html/havilah.html

all-rules: havilah-rules

# generate rules
havilah/html/havilah.html: havilah/havilah_intro.html havilah/html/havilah.css havilah/havilah
	(cd havilah; \
	./havilah genrules havilah_intro.html havilah.css html/havilah.html)

