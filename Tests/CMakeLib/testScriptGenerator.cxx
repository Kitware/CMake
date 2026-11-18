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
  // The reader drops one newline after the opening bracket.
  TEST_QUOTE("\n$", "[[\n\n$]]");
  TEST_QUOTE("\r\n$", "[[\n\r\n$]]");
  TEST_QUOTE("\n$]", "[=[\n\n$]]=]");
  TEST_QUOTE("$\n", "[[$\n]]");
  return true;
}

} // namespace

int testScriptGenerator(int /*unused*/, char* /*unused*/[])
{
  return runTests({
    quote,
  });
}
