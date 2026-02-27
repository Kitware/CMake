/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <string>

#include "cmScriptGenerator.h"

#include "testCommon.h"

#define TEST_QUOTE(input, expected_output)                                    \
  ASSERT_EQUAL(cmScriptGenerator::Quote(input).str(), (expected_output))

namespace {

bool quote()
{
  TEST_QUOTE("", "\"\"");
  TEST_QUOTE("\"", "[[\"]]");
  TEST_QUOTE("\\", "[[\\]]");
  TEST_QUOTE("$", "[[$]]");
  TEST_QUOTE("$]", "[=[$]]=]");
  TEST_QUOTE("$]=", "[[$]=]]");
  TEST_QUOTE("$]=]", "[==[$]=]]==]");
  TEST_QUOTE("$]==]", "[=[$]==]]=]");
  return true;
}

} // namespace

int testScriptGenerator(int /*unused*/, char* /*unused*/[])
{
  return runTests({
    quote,
  });
}
