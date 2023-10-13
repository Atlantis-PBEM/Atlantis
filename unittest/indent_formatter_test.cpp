#include <sstream>
#include "../external/boost/ut.hpp"

#include "../game.h"
#include "../gamedata.h"
#include "../indenter.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Indent Formatter"> indent_formatter_suite = []
{
  using namespace ut;

  "increment indents"_test = []
  {
    stringstream ss;
    ss << indent::incr << "test\n";
    ss.flush();
    expect(eq(ss.str(), string("  test\n")));
  };

  "incr and decr control indent level"_test = []
  {
    stringstream ss;
    ss << indent::incr << "test\n";
    ss << indent::incr << "test\n";
    ss << indent::decr << "test\n";
    ss.flush();
    expect(eq(ss.str(), string("  test\n    test\n  test\n")));
  };

  "clear removes formatter"_test = []
  {
    stringstream ss;
    ss << indent::incr << "test\n";
    ss << indent::incr << "test\n";
    ss << indent::clear << "test\n";
    expect(eq(ss.str(), string("  test\n    test\ntest\n")));
  };

  "default wraps at first space before 70 characters then indents wrapped lines"_test = []
  {
    stringstream ss;
    ss << indent::incr << "123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 \n";
    expect(eq(
      ss.str(),
      string("  123456789 123456789 123456789 123456789 123456789 123456789\n    123456789 123456789 \n")
    ));
  };

  "wrap with no indent still wraps correctly"_test = []
  {
    stringstream ss;
    ss << indent::wrap << "--123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 \n";
    expect(eq(
      ss.str(),
      string("--123456789 123456789 123456789 123456789 123456789 123456789\n  123456789 123456789 \n")
    ));
  };


  "wrap at shorter values works"_test = []
  {
    stringstream ss;
    ss << indent::wrap(5) << "test bar\n";
    expect(eq(ss.str(), string("test\n  bar\n")));
  };

  "wrap with different lookback works"_test = []
  {
    stringstream ss;
    // since the lookback is only 2, this cannot break at the first space, so should just break at the wrap point.
    // and then indent, then break the second wrapped line also at the wrap point and indent until done.
    ss << indent::wrap(5, 2) << "t esttestbar\n";
    expect(eq(ss.str(), string("t est\n  tes\n  tba\n  r\n")));
  };

  "comment prefixes line with semicolon"_test = []
  {
    stringstream ss;
    ss << indent::comment << "test\n";
    expect(eq(ss.str(), string(";test\n")));
  };

  "comment is not counted in wrapping and persists on wrapped lines"_test = []
  {
    stringstream ss;
    ss << indent::wrap(5, 2) << indent::comment << "t esttestbar\n";
    expect(eq(ss.str(), string(";t est\n;  tes\n;  tba\n;  r\n")));
  };

  "flushes correctly"_test = []
  {
    stringstream ss;
    ss << indent::incr << "test";
    ss.flush();
    expect(eq(ss.str(), string("  test")));
  };
};
