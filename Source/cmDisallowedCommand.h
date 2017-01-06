/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmDisallowedCommand_h
#define cmDisallowedCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include "cm_memory.hxx"

#include "cmCommand.h"
#include "cmPolicies.h"

class cmExecutionStatus;

class cmDisallowedCommand : public cmCommand
{
public:
  cmDisallowedCommand(std::unique_ptr<cmCommand> command,
                      cmPolicies::PolicyID policy, const char* message)
    : Command(std::move(command))
    , Policy(policy)
    , Message(message)
  {
  }

  ~cmDisallowedCommand() override = default;

  std::unique_ptr<cmCommand> Clone() override
  {
    return cm::make_unique<cmDisallowedCommand>(this->Command->Clone(),
                                                this->Policy, this->Message);
  }

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

private:
  std::unique_ptr<cmCommand> Command;
  cmPolicies::PolicyID Policy;
  const char* Message;
};

#endif
