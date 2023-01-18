/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <iostream>
#include <string>
#include <vector>

#include "cmRange.h"

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << "\n"; \
      return -1;                                                              \
    }                                                                         \
  } while (false)

int testRange(int /*unused*/, char* /*unused*/[])
{
  std::vector<int> const testData = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

  ASSERT_TRUE(!cmMakeRange(testData).empty());
  ASSERT_TRUE(cmMakeRange(testData).size() == 10);

  ASSERT_TRUE(!cmMakeRange(testData).advance(5).empty());
  ASSERT_TRUE(cmMakeRange(testData).advance(5).size() == 5);

  ASSERT_TRUE(cmMakeRange(testData).advance(5).retreat(5).empty());
  ASSERT_TRUE(cmMakeRange(testData).advance(5).retreat(5).size() == 0);

  ASSERT_TRUE(cmMakeRange(testData).any_of([](int n) { return n % 3 == 0; }));
  ASSERT_TRUE(cmMakeRange(testData).all_of([](int n) { return n < 11; }));
  ASSERT_TRUE(cmMakeRange(testData).none_of([](int n) { return n > 11; }));

  std::vector<int> const evenData = { 2, 4, 6, 8, 10 };
  ASSERT_TRUE(cmMakeRange(testData).filter([](int n) { return n % 2 == 0; }) ==
              cmMakeRange(evenData));

  std::vector<std::string> const stringRange = { "1", "2", "3", "4", "5" };
  ASSERT_TRUE(cmMakeRange(testData)
                .transform([](int n) { return std::to_string(n); })
                .retreat(5) == cmMakeRange(stringRange));

  return 0;
}
