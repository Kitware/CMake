/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>

#include "cmArgumentParserTypes.h" // IWYU pragma: keep
#include "cmCTestHandlerCommand.h"
#include "cmCommand.h"

class cmCTestGenericHandler;

/** \class cmCTestCoverage
 * \brief Run a ctest script
 *
 * cmCTestCoverageCommand defineds the command to test the project.
 */
class cmCTestCoverageCommand : public cmCTestHandlerCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    auto ni = cm::make_unique<cmCTestCoverageCommand>();
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return std::unique_ptr<cmCommand>(std::move(ni));
  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const override { return "ctest_coverage"; }

protected:
  void BindArguments() override;
  cmCTestGenericHandler* InitializeHandler() override;

  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Labels;
};
