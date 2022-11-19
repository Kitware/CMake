/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

#include "cmArgumentParserTypes.h"
#include "cmCTestHandlerCommand.h"

class cmCommand;
class cmCTestGenericHandler;
class cmExecutionStatus;

/** \class cmCTestSubmit
 * \brief Run a ctest script
 *
 * cmCTestSubmitCommand defineds the command to submit the test results for
 * the project.
 */
class cmCTestSubmitCommand : public cmCTestHandlerCommand
{
public:
  std::unique_ptr<cmCommand> Clone() override;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const override { return "ctest_submit"; }

protected:
  void BindArguments() override;
  void CheckArguments() override;
  cmCTestGenericHandler* InitializeHandler() override;

  bool CDashUpload = false;
  bool InternalTest = false;

  std::string BuildID;
  std::string CDashUploadFile;
  std::string CDashUploadType;
  std::string RetryCount;
  std::string RetryDelay;
  std::string SubmitURL;

  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Files;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> HttpHeaders;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Parts;
};
