/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmArgumentParserTypes.h"
#include "cmCTestHandlerCommand.h"
#include "cmCommand.h"

class cmCTestGenericHandler;

/** \class cmCTestUpload
 * \brief Run a ctest script
 *
 * cmCTestUploadCommand defines the command to upload result files for
 * the project.
 */
class cmCTestUploadCommand : public cmCTestHandlerCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    auto ni = cm::make_unique<cmCTestUploadCommand>();
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return std::unique_ptr<cmCommand>(std::move(ni));
  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const override { return "ctest_upload"; }

protected:
  void BindArguments() override;
  void CheckArguments() override;
  cmCTestGenericHandler* InitializeHandler() override;

  ArgumentParser::MaybeEmpty<std::vector<std::string>> Files;
};
