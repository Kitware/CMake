/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <functional>       // IWYU pragma: export
#include <initializer_list> // IWYU pragma: export
#include <iostream>         // IWYU pragma: export

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << '\n'; \
      return false;                                                           \
    }                                                                         \
  } while (false)

#define ASSERT_EQUAL(actual, expected)                                        \
  do {                                                                        \
    if (!((actual) == (expected))) {                                          \
      std::cout << "ASSERT_EQUAL(" #actual ", " #expected ") failed on line " \
                << __LINE__ << '\n';                                          \
      std::cout << "  Actual: '" << (actual) << "'\n";                        \
      std::cout << "Expected: '" << (expected) << "'\n";                      \
      return false;                                                           \
    }                                                                         \
  } while (false)

#define BOOL_STRING(b) ((b) ? "TRUE" : "FALSE")

namespace {

inline int runTests(std::initializer_list<std::function<bool()>> const& tests,
                    bool const fail_fast = true)
{
  int result = 0;
  for (auto const& test : tests) {
    if (!test()) {
      result = 1;
      if (fail_fast) {
        break;
      }
    }
  }
  if (result == 0) {
    std::cout << "Passed\n";
  }
  return result;
}

}
