/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmUtilitySourceCommand_h
#define cmUtilitySourceCommand_h

#include "cmCommand.h"

class cmUtilitySourceCommand : public cmCommand
{
public:
  cmTypeMacro(cmUtilitySourceCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE { return new cmUtilitySourceCommand; }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE { return "utility_source"; }
};

#endif
