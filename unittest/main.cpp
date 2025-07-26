#include "external/boost/ut.hpp"

namespace ut = boost::ut;

// This is the driver for the unit test suite.   The tests
// are defined in the individual files (suites) in the tests
// directory, and the framework registers and executes them
// automatically.
int main() {
    return ut::cfg<>.run();
 }
