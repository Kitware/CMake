/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmInstallProgramsCommand_h
#define cmInstallProgramsCommand_h

#include "cmStandardIncludes.h"
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
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "INSTALL_PROGRAMS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create UNIX install rules for programs.";
    }
  
  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  INSTALL_PROGRAMS(<dir> file file ...)\n"
      "Create rules to install the listed programs into the given directory.\n"
      "  INSTALL_PROGRAMS(<dir> regexp)\n"
      "In the second form any program in the current source directory that "
      "matches the regular expression will be installed.\n"
      "This command is intended to install programs that are not built "
      "by cmake, such as shell scripts.  See INSTALL_TARGETS to "
      "create installation rules for targets built by cmake.\n"
      "The directory <dir> is relative to the installation prefix, which "
      "is stored in the variable CMAKE_INSTALL_PREFIX.";
    }
  
  cmTypeMacro(cmInstallProgramsCommand, cmCommand);

protected:
  std::string FindInstallSource(const char* name) const;
private:
  std::string m_TargetName;
  std::vector<std::string> m_FinalArgs;
};


#endif
