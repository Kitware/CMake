/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestCoverageCommand_h
#define cmCTestCoverageCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

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
  void CheckArguments(std::vector<std::string> const& keywords) override;
  cmCTestGenericHandler* InitializeHandler() override;

  bool LabelsMentioned;
  std::vector<std::string> Labels;
};

#endif
