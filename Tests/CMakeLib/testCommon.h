/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <functional>
#include <iostream>
#include <vector>

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << "\n"; \
      return false;                                                           \
    }                                                                         \
  } while (false)

inline int runTests(std::vector<std::function<bool()>> const& tests)
{
  for (auto const& test : tests) {
    if (!test()) {
      return 1;
    }
    std::cout << ".";
  }

  std::cout << " Passed" << std::endl;
  return 0;
}

#define BOOL_STRING(b) ((b) ? "TRUE" : "FALSE")
