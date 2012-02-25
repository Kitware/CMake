/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestStartCommand_h
#define cmCTestStartCommand_h

#include "cmCTestCommand.h"

/** \class cmCTestStart
 * \brief Run a ctest script
 *
 * cmCTestStartCommand defineds the command to start the nightly testing.
 */
class cmCTestStartCommand : public cmCTestCommand
{
public:

  cmCTestStartCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestStartCommand* ni = new cmCTestStartCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    ni->CreateNewTag = this->CreateNewTag;
    return ni;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * Will this invocation of ctest_start create a new TAG file?
   */
  bool ShouldCreateNewTag()
    {
    return this->CreateNewTag;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "ctest_start";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Starts the testing for a given model";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  ctest_start(Model [TRACK <track>] [APPEND] [source [binary]])\n"
      "Starts the testing for a given model. The command should be called "
      "after the binary directory is initialized. If the 'source' and "
      "'binary' directory are not specified, it reads the "
      "CTEST_SOURCE_DIRECTORY and CTEST_BINARY_DIRECTORY. If the track is "
      "specified, the submissions will go to the specified track. "
      "If APPEND is used, the existing TAG is used rather than "
      "creating a new one based on the current time stamp.";
    }

  cmTypeMacro(cmCTestStartCommand, cmCTestCommand);

private:
  bool InitialCheckout(std::ostream& ofs, std::string const& sourceDir);
  bool CreateNewTag;
};


#endif
