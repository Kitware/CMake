/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <string>

#include "testCommon.h"

namespace {

class WrapFailureInBlockFixture
{
public:
  WrapFailureInBlockFixture()
  {
    std::cout << "---[ BEGIN Expected Failure Output]---\n";
  }
  ~WrapFailureInBlockFixture()
  {
    std::cout << "---[ END Expected Failure Output]---\n";
  }
};

bool testASSERT_EQUAL()
{
  ASSERT_EQUAL(7 == 7, 42 == 42);
  {
    std::string actual = "Hello Africa!";
    ASSERT_EQUAL(actual, "Hello Africa!");
  }
  return true;
}

bool testASSERT_EQUALFail()
{
  WrapFailureInBlockFixture fx;
  static_cast<void>(fx);

  auto fail_int = [](const int unexpected) -> bool {
    ASSERT_EQUAL(unexpected, 42);
    return true;
  };

  auto fail_string = [](const std::string& unexpected) -> bool {
    ASSERT_EQUAL(unexpected, "Hello Africa!");
    return true;
  };

  return !(fail_int(7) || fail_string("Habari Afrika!"));
}

} // anonymous namespace

int testAssert(int /*unused*/, char* /*unused*/[])
{
  return runTests({ testASSERT_EQUAL, testASSERT_EQUALFail });
}
