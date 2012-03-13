/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallProgramsCommand_h
#define cmInstallProgramsCommand_h

#include "cmCommand.h"

/** \class cmInstallProgramsCommand
 * \brief Specifies where to install some programs
 *
 * cmInstallProgramsCommand specifies the relative path where a list of
 * programs should be installed.  
 */
class cmInstallProgramsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmInstallProgramsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "install_programs";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Deprecated. Use the install(PROGRAMS ) command instead.";
    }
  
  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  virtual bool HasFinalPass() const { return true; }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "This command has been superceded by the install command.  It "
      "is provided for compatibility with older CMake code.  "
      "The FILES form is directly replaced by the PROGRAMS form of the "
      "INSTALL command.  The regexp form can be expressed more clearly "
      "using the GLOB form of the FILE command.\n"
      "  install_programs(<dir> file1 file2 [file3 ...])\n"
      "  install_programs(<dir> FILES file1 [file2 ...])\n"
      "Create rules to install the listed programs into the given directory. "
      "Use the FILES argument to guarantee that the file list version of "
      "the command will be used even when there is only one argument.\n"
      "  install_programs(<dir> regexp)\n"
      "In the second form any program in the current source directory that "
      "matches the regular expression will be installed.\n"
      "This command is intended to install programs that are not built "
      "by cmake, such as shell scripts.  See the TARGETS form of "
      "the INSTALL command to "
      "create installation rules for targets built by cmake.\n"
      "The directory <dir> is relative to the installation prefix, which "
      "is stored in the variable CMAKE_INSTALL_PREFIX.";
    }
  
  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }

  cmTypeMacro(cmInstallProgramsCommand, cmCommand);

protected:
  std::string FindInstallSource(const char* name) const;
private:
  std::vector<std::string> FinalArgs;
  std::string Destination;
  std::vector<std::string> Files;
};


#endif
