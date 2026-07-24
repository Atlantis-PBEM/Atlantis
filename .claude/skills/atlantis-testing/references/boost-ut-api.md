# Boost.UT API reference (verified against the vendored 2.0.0 header)

Everything here was compiled and run against `external/boost/ut.hpp` with
`-Wall -Werror -std=c++20`. The upstream README documents 2.3.x and includes APIs this
version does not have — `format_test_parameter` is a confirmed absence. Grep the header
before using anything not listed here.

Contents:
1. Test declaration forms
2. Parameterized tests
3. Exception and abort assertions
4. BDD, spec, and Gherkin styles
5. Matchers and compile-time checks
6. Logging
7. Runner and reporter configuration
8. Quick reference of what exists in 2.0.0

---

## 1. Test declaration forms

```cpp
namespace ut = boost::ut;

// Named suite — the form every existing Atlantis suite uses.
ut::suite<"Suite name"> my_suite = [] {
  using namespace ut;

  "test via literal"_test = [] { expect(1_i == 1); };

  test("test via function") = [] { expect(1_i == 1); };

  should("nested section") = [] { expect(1_i == 1); };
};
```

An unnamed `boost::ut::suite` (non-template) also exists. Prefer the named
`suite<"...">` form so failures identify which suite they came from — the runner prints the
suite name in its summary line.

Tests registered at namespace scope outside any suite land in the implicit `global` suite,
which is why a run reports `Suite 'global': all tests passed (0 asserts in 0 tests)` when
every test lives in a named suite.

## 2. Parameterized tests

**Over values** — the parameter is appended to the generated test name, so a failure tells
you which case broke:

```cpp
"days by level is non-negative"_test = [](int level) {
  expect(that % GetDaysByLevel(level) >= 0);
} | std::vector<int>{1, 2, 3, 4, 5};
```

**Over types:**

```cpp
"integral types"_test = []<class T>() {
  expect(std::is_integral_v<T>);
} | std::tuple<int, long>{};
```

**Over values and types together:**

```cpp
"args and types"_test = []<class TArg>(const TArg& arg) {
  expect(std::is_integral_v<TArg>);
} | std::tuple{ true, 42 };
```

**Loop-generated**, when you want to control the names yourself:

```cpp
for (int i : {1, 2, 3}) {
  test("threshold case " + std::to_string(i)) = [i] {
    expect(that % i > 0);
  };
}
```

Note the loop form builds names as `std::string`, which is usually clearer than the `|`
form's auto-naming when the parameter is not self-describing.

## 3. Exception and abort assertions

```cpp
expect(throws<std::runtime_error>([] { risky(); }));   // specific type
expect(throws([] { risky(); }));                       // any exception
expect(nothrow([] { safe(); }));                       // must not throw
expect(aborts([] { assert(false); }));                 // Unix only
```

The engine barely uses exceptions, so in practice `nothrow` and `aborts` earn their keep more
than `throws` — they are the way to pin down that a defensive path degrades gracefully rather
than taking the process down.

## 4. BDD, spec, and Gherkin

All three exist in the vendored header. No suite in this repo uses them; prefer the plain
style for consistency unless a scenario is genuinely multi-step and reads better this way.

**BDD:**

```cpp
using namespace ut::bdd;

"taxation"_test = [] {
  given("a region with a guarding enemy") = [] {
    // …
    when("the guard is killed") = [=] {
      mut(region).…;                       // mut() gets a mutable ref to a by-value capture
      then("the region becomes taxable") = [=] {
        expect(region->CanTax(unit) == 1_i);
      };
    };
  };
};
```

`feature(...)` and `scenario(...)` are aliases for the same machinery.

**Spec:**

```cpp
using namespace ut::spec;

describe("vector") = [] {
  it("should resize") = [] { … };
};
```

**Gherkin** parsing (`bdd::gherkin::steps` driven by a raw-string feature file) is present
but is almost certainly more machinery than this codebase needs.

## 5. Matchers and compile-time checks

**Custom matcher** — a lambda returning a `that %` expression composes cleanly:

```cpp
constexpr auto is_between = [](auto lo, auto hi) {
  return [=](auto value) { return that % value >= lo and that % value <= hi; };
};

expect(is_between(1, 100)(42));
expect(not is_between(1, 100)(0));
```

**Compile-time assertion** via `constant<>`:

```cpp
constexpr auto expected = 42;
expect(constant<42_i == expected>);
```

**Type identity:**

```cpp
expect(type<TArg> == type<int> or type<TArg> == type<bool>);
expect(std::is_unsigned_v<T>) << reflection::type_name<T>() << "is unsigned";
```

**Compound expressions** use `and` / `or`, and each operand keeps its own reporting:

```cpp
expect(1_i == 1 and that % 2 > 1);
```

## 6. Logging

```cpp
ut::log << "context before the assertion";
expect(1_i == 2) << "message shown on failure";
```

A C++20 format-style form also exists:

```cpp
ut::log("\npre {} == {}", 42, 43);
```

Messages attached with `<<` print only on failure, which is what you usually want — an
explanation of *what the assertion meant*, sitting next to the operand values.

## 7. Runner and reporter configuration

`unittest/main.cpp` installs no configuration, so the defaults apply. That is what makes
`tag(...)` silently skip tests (see the main skill file). If you ever need to change it, the
override point is:

```cpp
template <> auto ut::cfg<ut::override> = ut::runner<cfg::reporter>{};
```

and runtime options are set with designated initialisers:

```cpp
cfg<override> = {
  .filter  = "suite.test.*",   // run a subset by name
  .dry_run = true,             // list test names without executing
};
```

Custom `runner`, `reporter`, and `printer` types are supported by implementing `on(...)`
overloads for the `ut::events::*` types. Introducing any of this affects every suite in the
binary, so treat it as a project-wide decision rather than a local convenience.

## 8. Quick reference — confirmed present in 2.0.0

Verified by grep and by compiling: `expect`, `fatal`, `eq` / `neq` / `gt` / `ge` / `lt` / `le`,
`that %`, all the literal suffixes, `throws` / `throws<T>` / `nothrow` / `aborts`, `skip`,
`tag`, `should`, `test(...)`, `suite<"...">`, `given` / `when` / `then`, `describe`, `mut`,
`constant`, `type`, `reflection::type_name`, `log`, terse operators, gherkin, `cfg` /
`override`, `filter`, `dry_run`.

Confirmed **absent**: `format_test_parameter`.
