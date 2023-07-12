/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <cm3p/cppdap/optional.h>
#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/types.h>

#include "cmDebuggerVariables.h"
#include "cmDebuggerVariablesManager.h"

#include "testCommon.h"
#include "testDebugger.h"

static dap::VariablesRequest CreateVariablesRequest(int64_t reference)
{
  dap::VariablesRequest variableRequest;
  variableRequest.variablesReference = reference;
  return variableRequest;
}

static bool testUniqueIds()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  std::unordered_set<int64_t> variableIds;
  bool noDuplicateIds = true;
  for (int i = 0; i < 10000 && noDuplicateIds; ++i) {
    auto variable =
      cmDebugger::cmDebuggerVariables(variablesManager, "Locals", true, []() {
        return std::vector<cmDebugger::cmDebuggerVariableEntry>();
      });

    if (variableIds.find(variable.GetId()) != variableIds.end()) {
      noDuplicateIds = false;
    }
    variableIds.insert(variable.GetId());
  }

  ASSERT_TRUE(noDuplicateIds);

  return true;
}

static bool testConstructors()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto parent = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Parent", true, [=]() {
      return std::vector<cmDebugger::cmDebuggerVariableEntry>{
        { "ParentKey", "ParentValue", "ParentType" }
      };
    });

  auto children1 = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Children1", true, [=]() {
      return std::vector<cmDebugger::cmDebuggerVariableEntry>{
        { "ChildKey1", "ChildValue1", "ChildType1" },
        { "ChildKey2", "ChildValue2", "ChildType2" }
      };
    });

  parent->AddSubVariables(children1);

  auto children2 = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Children2", true);

  auto grandChildren21 = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "GrandChildren21", true);
  grandChildren21->SetValue("GrandChildren21 Value");
  children2->AddSubVariables(grandChildren21);
  parent->AddSubVariables(children2);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(parent->GetId()));
  ASSERT_TRUE(variables.size() == 3);
  ASSERT_VARIABLE_REFERENCE(variables[0], "Children1", "", "collection",
                            children1->GetId());
  ASSERT_VARIABLE_REFERENCE(variables[1], "Children2", "", "collection",
                            children2->GetId());
  ASSERT_VARIABLE(variables[2], "ParentKey", "ParentValue", "ParentType");

  variables = variablesManager->HandleVariablesRequest(
    CreateVariablesRequest(children1->GetId()));
  ASSERT_TRUE(variables.size() == 2);
  ASSERT_VARIABLE(variables[0], "ChildKey1", "ChildValue1", "ChildType1");
  ASSERT_VARIABLE(variables[1], "ChildKey2", "ChildValue2", "ChildType2");

  variables = variablesManager->HandleVariablesRequest(
    CreateVariablesRequest(children2->GetId()));
  ASSERT_TRUE(variables.size() == 1);
  ASSERT_VARIABLE_REFERENCE(variables[0], "GrandChildren21",
                            "GrandChildren21 Value", "collection",
                            grandChildren21->GetId());

  return true;
}

static bool testIgnoreEmptyStringEntries()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto vars = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Variables", true, []() {
      return std::vector<cmDebugger::cmDebuggerVariableEntry>{
        { "IntValue1", 5 },           { "StringValue1", "" },
        { "StringValue2", "foo" },    { "StringValue3", "" },
        { "StringValue4", "bar" },    { "StringValue5", "" },
        { "IntValue2", int64_t(99) }, { "BooleanTrue", true },
        { "BooleanFalse", false },
      };
    });

  vars->SetIgnoreEmptyStringEntries(true);
  vars->SetEnableSorting(false);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));
  ASSERT_TRUE(variables.size() == 6);
  ASSERT_VARIABLE(variables[0], "IntValue1", "5", "int");
  ASSERT_VARIABLE(variables[1], "StringValue2", "foo", "string");
  ASSERT_VARIABLE(variables[2], "StringValue4", "bar", "string");
  ASSERT_VARIABLE(variables[3], "IntValue2", "99", "int");
  ASSERT_VARIABLE(variables[4], "BooleanTrue", "TRUE", "bool");
  ASSERT_VARIABLE(variables[5], "BooleanFalse", "FALSE", "bool");

  return true;
}

static bool testSortTheResult()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto vars = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Variables", true, []() {
      return std::vector<cmDebugger::cmDebuggerVariableEntry>{
        { "4", "4" }, { "2", "2" }, { "1", "1" }, { "3", "3" }, { "5", "5" },
      };
    });

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));
  ASSERT_TRUE(variables.size() == 5);
  ASSERT_VARIABLE(variables[0], "1", "1", "string");
  ASSERT_VARIABLE(variables[1], "2", "2", "string");
  ASSERT_VARIABLE(variables[2], "3", "3", "string");
  ASSERT_VARIABLE(variables[3], "4", "4", "string");
  ASSERT_VARIABLE(variables[4], "5", "5", "string");

  vars->SetEnableSorting(false);

  variables = variablesManager->HandleVariablesRequest(
    CreateVariablesRequest(vars->GetId()));
  ASSERT_TRUE(variables.size() == 5);
  ASSERT_VARIABLE(variables[0], "4", "4", "string");
  ASSERT_VARIABLE(variables[1], "2", "2", "string");
  ASSERT_VARIABLE(variables[2], "1", "1", "string");
  ASSERT_VARIABLE(variables[3], "3", "3", "string");
  ASSERT_VARIABLE(variables[4], "5", "5", "string");

  return true;
}

static bool testNoSupportsVariableType()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto vars = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Variables", false, []() {
      return std::vector<cmDebugger::cmDebuggerVariableEntry>{ { "test",
                                                                 "value" } };
    });

  auto subvars = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Children", false);

  vars->AddSubVariables(subvars);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(variables.size() == 2);
  ASSERT_VARIABLE(variables[0], "Children", "", nullptr);
  ASSERT_VARIABLE(variables[1], "test", "value", nullptr);

  return true;
}

int testDebuggerVariables(int, char*[])
{
  return runTests(std::vector<std::function<bool()>>{
    testUniqueIds,
    testConstructors,
    testIgnoreEmptyStringEntries,
    testSortTheResult,
    testNoSupportsVariableType,
  });
}
