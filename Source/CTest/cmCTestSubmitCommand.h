/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestSubmitCommand_h
#define cmCTestSubmitCommand_h

#include "cmCTestHandlerCommand.h"
#include "cmCTest.h"

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
    }

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestSubmitCommand* ni = new cmCTestSubmitCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ctest_submit";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Submit results to a dashboard server.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ctest_submit([PARTS ...] [FILES ...] [RETRY_COUNT count] "
      "               [RETRY_DELAY delay][RETURN_VALUE res])\n"
      "By default all available parts are submitted if no PARTS or FILES "
      "are specified.  "
      "The PARTS option lists a subset of parts to be submitted.  "
      "Valid part names are:\n"
      "  Start      = nothing\n"
      "  Update     = ctest_update results, in Update.xml\n"
      "  Configure  = ctest_configure results, in Configure.xml\n"
      "  Build      = ctest_build results, in Build.xml\n"
      "  Test       = ctest_test results, in Test.xml\n"
      "  Coverage   = ctest_coverage results, in Coverage.xml\n"
      "  MemCheck   = ctest_memcheck results, in DynamicAnalysis.xml\n"
      "  Notes      = Files listed by CTEST_NOTES_FILES, in Notes.xml\n"
      "  ExtraFiles = Files listed by CTEST_EXTRA_SUBMIT_FILES\n"
      "  Submit     = nothing\n"
      "The FILES option explicitly lists specific files to be submitted.  "
      "Each individual file must exist at the time of the call.\n"
      "The RETRY_DELAY option specifies how long in seconds to wait after "
      "a timed-out submission before attempting to re-submit.\n"
      "The RETRY_COUNT option specifies how many times to retry a timed-out "
      "submission.\n";
    }

  cmTypeMacro(cmCTestSubmitCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler();

  virtual bool CheckArgumentKeyword(std::string const& arg);
  virtual bool CheckArgumentValue(std::string const& arg);

  enum
  {
    ArgumentDoingParts = Superclass::ArgumentDoingLast1,
    ArgumentDoingFiles,
    ArgumentDoingRetryDelay,
    ArgumentDoingRetryCount,
    ArgumentDoingLast2
  };

  bool PartsMentioned;
  std::set<cmCTest::Part> Parts;
  bool FilesMentioned;
  bool InternalTest;
  cmCTest::SetOfStrings Files;
  std::string RetryCount;
  std::string RetryDelay;
};


#endif
