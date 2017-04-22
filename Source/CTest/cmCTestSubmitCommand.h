/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestSubmitCommand_h
#define cmCTestSubmitCommand_h

#include "cmConfigure.h"

#include "cmCTest.h"
#include "cmCTestHandlerCommand.h"

#include <set>
#include <string>
#include <vector>

class cmCTestGenericHandler;
class cmCommand;
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
  cmCTestSubmitCommand()
  {
    this->PartsMentioned = false;
    this->FilesMentioned = false;
    this->InternalTest = false;
    this->RetryCount = "";
    this->RetryDelay = "";
    this->CDashUpload = false;
  }

  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE
  {
    cmCTestSubmitCommand* ni = new cmCTestSubmitCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
  }

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "ctest_submit"; }

  typedef cmCTestHandlerCommand Superclass;

protected:
  cmCTestGenericHandler* InitializeHandler() CM_OVERRIDE;

  bool CheckArgumentKeyword(std::string const& arg) CM_OVERRIDE;
  bool CheckArgumentValue(std::string const& arg) CM_OVERRIDE;

  enum
  {
    ArgumentDoingParts = Superclass::ArgumentDoingLast1,
    ArgumentDoingFiles,
    ArgumentDoingRetryDelay,
    ArgumentDoingRetryCount,
    ArgumentDoingCDashUpload,
    ArgumentDoingCDashUploadType,
    ArgumentDoingLast2
  };

  bool PartsMentioned;
  std::set<cmCTest::Part> Parts;
  bool FilesMentioned;
  bool InternalTest;
  cmCTest::SetOfStrings Files;
  std::string RetryCount;
  std::string RetryDelay;
  bool CDashUpload;
  std::string CDashUploadFile;
  std::string CDashUploadType;
};

#endif
