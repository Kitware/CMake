/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <iostream>
#include <string>
#include <vector>

#include "cmCTestTestHandler.h"

struct ExpectedParseResult
{
  std::string String;
  bool ExpectedReturnValue;
  std::vector<std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>
    ExpectedValue;
};

static const std::vector<ExpectedParseResult> expectedResults{
  /* clang-format off */
  { "threads:2", true, {
    { { "threads", 2, 1 } },
  } },
  { "3,threads:2", true, {
    { { "threads", 2, 1 } },
    { { "threads", 2, 1 } },
    { { "threads", 2, 1 } },
  } },
  { "3,threads:2,gpus:4", true, {
    { { "threads", 2, 1 }, { "gpus", 4, 1 } },
    { { "threads", 2, 1 }, { "gpus", 4, 1 } },
    { { "threads", 2, 1 }, { "gpus", 4, 1 } },
  } },
  { "2,threads:2;gpus:4", true, {
    { { "threads", 2, 1 } },
    { { "threads", 2, 1 } },
    { { "gpus", 4, 1 } },
  } },
  { "threads:2;2,gpus:4", true, {
    { { "threads", 2, 1 } },
    { { "gpus", 4, 1 } },
    { { "gpus", 4, 1 } },
  } },
  { "threads:2;gpus:4", true, {
    { { "threads", 2, 1 } },
    { { "gpus", 4, 1 } },
  } },
  { "1,threads:2;0,gpus:4", true, {
    { { "threads", 2, 1 } },
  } },
  { "1,_:1", true, {
    { { "_", 1, 1 } },
  } },
  { "1,a:1", true, {
    { { "a", 1, 1 } },
  } },
  { "2", true, {
    {},
    {},
  } },
  { "1;2,threads:1", true, {
    {},
    { { "threads", 1, 1 } },
    { { "threads", 1, 1 } },
  } },
  { "1,,threads:1", true, {
    { { "threads", 1, 1 } },
  } },
  { ";1,threads:1", true, {
    { { "threads", 1, 1 } },
  } },
  { "1,threads:1;", true, {
    { { "threads", 1, 1 } },
  } },
  { "1,threads:1,", true, {
    { { "threads", 1, 1 } },
  } },
  { "threads:1,threads:1", true, {
    { { "threads", 1, 1 }, { "threads", 1, 1 } },
  } },
  { "threads:1;;threads:2", true, {
    { { "threads", 1, 1 } },
    { { "threads", 2, 1 } },
  } },
  { "1,", true, {
    {},
  } },
  { ";", true, {} },
  { "", true, {} },
  { ",", false, {} },
  { "1,0:1", false, {} },
  { "1,A:1", false, {} },
  { "1,a-b:1", false, {} },
  { "invalid", false, {} },
  { ",1,invalid:1", false, {} },
  { "1,1", false, {} },
  { "-1,invalid:1", false, {} },
  { "1,invalid:*", false, {} },
  { "1,invalid:-1", false, {} },
  { "1,invalid:-", false, {} },
  { "1,invalid:ab2", false, {} },
  { "1,invalid :2", false, {} },
  { "1, invalid:2", false, {} },
  { "1,invalid:ab", false, {} },
  /* clang-format on */
};

static bool TestExpectedParseResult(const ExpectedParseResult& expected)
{
  std::vector<std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>
    result;
  bool retval;
  if ((retval = cmCTestTestHandler::ParseResourceGroupsProperty(
         expected.String, result)) != expected.ExpectedReturnValue) {
    std::cout << "ParseResourceGroupsProperty(\"" << expected.String
              << "\") returned " << retval << ", should be "
              << expected.ExpectedReturnValue << std::endl;
    return false;
  }

  if (result != expected.ExpectedValue) {
    std::cout << "ParseResourceGroupsProperty(\"" << expected.String
              << "\") did not yield expected set of resource groups"
              << std::endl;
    return false;
  }

  return true;
}

int testCTestResourceGroups(int /*unused*/, char* /*unused*/ [])
{
  int retval = 0;

  for (auto const& expected : expectedResults) {
    if (!TestExpectedParseResult(expected)) {
      retval = 1;
    }
  }

  return retval;
}
