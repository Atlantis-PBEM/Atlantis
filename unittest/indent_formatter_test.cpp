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
    std::stringstream ss;
    ss << indent::incr << "test\n";
    ss.flush();
    expect(eq(ss.str(), std::string("  test\n")));
  };

  "incr and decr control indent level"_test = []
  {
    std::stringstream ss;
    ss << indent::incr << "test\n";
    ss << indent::incr << "test\n";
    ss << indent::decr << "test\n";
    ss.flush();
    expect(eq(ss.str(), std::string("  test\n    test\n  test\n")));
  };

  "decreasing indent when it's already 0 leaves it at 0"_test = []
  {
    std::stringstream ss;
    ss << indent::wrap(20, 5, 0) << indent::decr << "test\n";
    expect(eq(ss.str(), std::string("test\n")));
  };

  "clear removes formatter"_test = []
  {
    std::stringstream ss;
    ss << indent::incr << "test\n";
    ss << indent::incr << "test\n";
    ss << indent::clear << "test\n";
    expect(eq(ss.str(), std::string("  test\n    test\ntest\n")));
  };

  "default wraps at first space before 70 characters then indents wrapped lines"_test = []
  {
    std::stringstream ss;
    ss << indent::incr << "123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 \n";
    expect(eq(
      ss.str(),
      std::string("  123456789 123456789 123456789 123456789 123456789 123456789\n    123456789 123456789 \n")
    ));
  };

  "wrap with no indent still wraps correctly"_test = []
  {
    std::stringstream ss;
    ss << indent::wrap << "--123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 \n";
    expect(eq(
      ss.str(),
      std::string("--123456789 123456789 123456789 123456789 123456789 123456789\n  123456789 123456789 \n")
    ));
  };


  "wrap at shorter values works"_test = []
  {
    std::stringstream ss;
    ss << indent::wrap(5) << "test bar\n";
    expect(eq(ss.str(), std::string("test\n  bar\n")));
  };

  "wrap with different lookback works"_test = []
  {
    std::stringstream ss;
    // since the lookback is only 2, this cannot break at the first space, so should just break at the wrap point.
    // and then indent, then break the second wrapped line also at the wrap point and indent until done.
    ss << indent::wrap(5, 2) << "t esttestbar\n";
    expect(eq(ss.str(), std::string("t est\n  tes\n  tba\n  r\n")));
  };

  "wrap with different wrapped line indent works"_test = []
  {
    std::stringstream ss;
    ss << indent::wrap(5, 2, 1) << "t esttestbar\n";
    expect(eq(ss.str(), std::string("t est\n test\n bar\n")));
  };

  "comment prefixes line with semicolon"_test = []
  {
    std::stringstream ss;
    ss << indent::comment << "test\n";
    expect(eq(ss.str(), std::string(";test\n")));
  };

  "comment is not counted in wrapping and persists on wrapped lines"_test = []
  {
    std::stringstream ss;
    ss << indent::wrap(5, 2) << indent::comment << "t esttestbar\n";
    expect(eq(ss.str(), std::string(";t est\n;  tes\n;  tba\n;  r\n")));
  };

  "flushes correctly"_test = []
  {
    std::stringstream ss;
    ss << indent::incr << "test";
    ss.flush();
    expect(eq(ss.str(), std::string("  test")));
  };

  "push and pop indent works"_test = []
  {
    std::stringstream ss;
    ss << indent::incr << "test\n";
    ss << indent::push_indent(0) << "test\n";
    ss << indent::pop_indent() << "test\n";
    expect(eq(ss.str(), std::string("  test\ntest\n  test\n")));
  };
};
