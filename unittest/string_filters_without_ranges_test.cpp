#ifndef CPP20_RANGES_ARE_BROKEN
#define CPP20_RANGES_ARE_BROKEN
#endif
#include "string_filters.hpp"

#include "external/boost/ut.hpp"
#include "external/nlohmann/json.hpp"

using json = nlohmann::json;

#include <string>
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

using namespace std;

// This suite tests the string_filters.hpp utility functions for string manipulation
ut::suite<"StringFiltersWithoutRanges"> string_filters_without_ranges_suite = []
{
  using namespace ut;

  "canonicalize adds underscores"_test = []
  {
    string input = "Hello Beautiful World";
    string expected = "Hello_Beautiful_World";
    string actual = filter::canonicalize(input);
    expect(actual == expected);
  };

  "canonicalize adds capitals"_test = []
  {
    string input = "hello world";
    string expected = "Hello_World";
    string actual = filter::canonicalize(input);
    expect(actual == expected);
  };
};
