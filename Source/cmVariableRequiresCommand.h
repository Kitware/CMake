/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmVariableRequiresCommand_h
#define cmVariableRequiresCommand_h

#include <cmConfigure.h>
#include <string>
#include <vector>

#include "cmCommand.h"
#include "cmTypeMacro.h"

class cmExecutionStatus;

class cmVariableRequiresCommand : public cmCommand
{
public:
  cmTypeMacro(cmVariableRequiresCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE { return new cmVariableRequiresCommand; }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE { return "variable_requires"; }
};

#endif
