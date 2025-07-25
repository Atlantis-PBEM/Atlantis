#include "external/boost/ut.hpp"
#include "external/nlohmann/json.hpp"

using json = nlohmann::json;

#include <string>
#include "testhelper.hpp"
#include "string_filters.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite tests the string_filters.hpp utility functions for string manipulation
ut::suite<"StringFilters"> string_filters_suite = []
{
  using namespace ut;

  "legal_characters removes illegal characters"_test = []
  {
    std::string input = "Hello, World! [@#$\n^&*]";
    std::string expected = "Hello, World! @#$^&*";
    std::string actual = filter::legal_characters(input);
    expect(actual == expected);
  };

  "legal_characters returns empty string for spaces-only result"_test = []
  {
    std::string input = "   \t\n\r";
    std::string expected = "";
    std::string actual = filter::legal_characters(input);
    expect(actual == expected);
  };

  "strip_number removes text after bracket/parenthesis and one preceeding space"_test = []
  {
    std::string input = "Hello, World! (123)";
    std::string expected = "Hello, World!";
    std::string actual = filter::strip_number(input);
    expect(actual == expected);

    input = "Test String [456]";
    expected = "Test String";
    actual = filter::strip_number(input);
    expect(actual == expected);

    // Test with multiple spaces before bracket
    input = "Multiple   Spaces    (before)";
    expected = "Multiple   Spaces   ";
    actual = filter::strip_number(input);
    expect(actual == expected);

    // Test with no space before bracket
    input = "NoSpace(before)";
    expected = "NoSpace";
    actual = filter::strip_number(input);
    expect(actual == expected);
  };

  "strip_number handles no brackets/parentheses"_test = []
  {
    std::string input = "Hello, World!";
    std::string expected = "Hello, World!";
    std::string actual = filter::strip_number(input);
    expect(actual == expected);
  };

  "strip_number handles brackets/parentheses at start"_test = []
  {
    std::string input = "(Hello) World!";
    std::string expected = "";
    std::string actual = filter::strip_number(input);
    expect(actual == expected);
  };

  "pipe syntax works for legal_characters"_test = []
  {
    std::string input = "Hello, World! [@#$]";
    std::string expected = "Hello, World! @#$";
    std::string actual = input | filter::legal_characters;
    expect(actual == expected);
  };

  "pipe syntax works for strip_number"_test = []
  {
    std::string input = "Hello, World! (123)";
    std::string expected = "Hello, World!";
    std::string actual = input | filter::strip_number;
    expect(actual == expected);
  };

  "filters can be chained with pipe syntax"_test = []
  {
    std::string input = "Hello, World! (123)";
    std::string expected = "Hello, World!";
    std::string actual = (input | filter::strip_number) | filter::legal_characters;
    expect(actual == expected);

    // With illegal characters before stripping - this won't strip the number as the () get filtered out first
    input = "Hello, [World]! (123)";
    std::string expected2 = "Hello, World! 123";
    std::string actual2 = (input | filter::legal_characters) | filter::strip_number;
    expect(actual2 == expected2);
  };
};
