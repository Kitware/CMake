/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestBuildCommand_h
#define cmCTestBuildCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmCTestHandlerCommand.h"
#include "cmCommand.h"

class cmCTestBuildHandler;
class cmCTestGenericHandler;
class cmExecutionStatus;
class cmGlobalGenerator;

/** \class cmCTestBuild
 * \brief Run a ctest script
 *
 * cmCTestBuildCommand defineds the command to build the project.
 */
class cmCTestBuildCommand : public cmCTestHandlerCommand
{
public:
  ~cmCTestBuildCommand() override;

  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    auto ni = cm::make_unique<cmCTestBuildCommand>();
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return std::unique_ptr<cmCommand>(std::move(ni));
  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const override { return "ctest_build"; }

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

  std::unique_ptr<cmGlobalGenerator> GlobalGenerator;

protected:
  cmCTestBuildHandler* Handler;
  void BindArguments() override;
  cmCTestGenericHandler* InitializeHandler() override;

  std::string NumberErrors;
  std::string NumberWarnings;
  std::string Target;
  std::string Configuration;
  std::string Flags;
  std::string ProjectName;
};

#endif
