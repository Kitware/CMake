/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmDeprecatedCommand_h
#define cmDeprecatedCommand_h

#include <cmConfigure.h>
#include <string>
#include <vector>

#include "cmCommand.h"
#include "cmPolicies.h"

class cmExecutionStatus;

class cmDeprecatedCommand : public cmCommand
{
public:
  cmDeprecatedCommand(cmCommand* command, cmPolicies::PolicyID policy,
                      const char* message)
    : Command(command)
    , Policy(policy)
    , Message(message)
  {
  }

  ~cmDeprecatedCommand() { delete this->Command; }

  cmCommand* Clone() CM_OVERRIDE
  {
    return new cmDeprecatedCommand(this->Command->Clone(), this->Policy,
                                   this->Message);
  }

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  bool IsScriptable() const CM_OVERRIDE
  {
    return this->Command->IsScriptable();
  }

  std::string GetName() const CM_OVERRIDE { return this->Command->GetName(); }

private:
  cmCommand* Command;
  cmPolicies::PolicyID Policy;
  const char* Message;
};

#endif
