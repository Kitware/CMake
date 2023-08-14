/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <functional>
#include <memory>
#include <vector>

#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/types.h>
#include <stdint.h>

#include "cmDebuggerVariables.h"
#include "cmDebuggerVariablesManager.h"

#include "testCommon.h"

static bool testVariablesRegistration()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  int64_t line = 5;
  auto local = std::make_shared<cmDebugger::cmDebuggerVariables>(
    variablesManager, "Local", true, [=]() {
      return std::vector<cmDebugger::cmDebuggerVariableEntry>{ { "CurrentLine",
                                                                 line } };
    });

  dap::VariablesRequest variableRequest;
  variableRequest.variablesReference = local->GetId();

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(variableRequest);

  ASSERT_TRUE(variables.size() == 1);

  local.reset();

  variables = variablesManager->HandleVariablesRequest(variableRequest);
  ASSERT_TRUE(variables.size() == 0);

  return true;
}

int testDebuggerVariablesManager(int, char*[])
{
  return runTests(std::vector<std::function<bool()>>{
    testVariablesRegistration,
  });
}
