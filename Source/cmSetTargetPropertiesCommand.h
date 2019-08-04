/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmSetTargetsPropertiesCommand_h
#define cmSetTargetsPropertiesCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/memory>

#include "cmCommand.h"

class cmExecutionStatus;
class cmMakefile;

class cmSetTargetPropertiesCommand : public cmCommand
{
public:
  std::unique_ptr<cmCommand> Clone() override
  {
    return cm::make_unique<cmSetTargetPropertiesCommand>();
  }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

  /**
   *  Used by this command and cmSetPropertiesCommand
   */
  static bool SetOneTarget(const std::string& tname,
                           std::vector<std::string>& propertyPairs,
                           cmMakefile* mf);
};

#endif
