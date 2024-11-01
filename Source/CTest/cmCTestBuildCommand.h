/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm/memory>

#include "cmCTestHandlerCommand.h"

class cmCommand;
class cmCTestGenericHandler;
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
  std::unique_ptr<cmCommand> Clone() override;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const override { return "ctest_build"; }

  std::unique_ptr<cmGlobalGenerator> GlobalGenerator;

protected:
  void ProcessAdditionalValues(cmCTestGenericHandler* handler) override;
  void BindArguments() override;
  std::unique_ptr<cmCTestGenericHandler> InitializeHandler() override;

  std::string NumberErrors;
  std::string NumberWarnings;
  std::string Target;
  std::string Configuration;
  std::string Flags;
  std::string ProjectName;
  std::string ParallelLevel;
};
