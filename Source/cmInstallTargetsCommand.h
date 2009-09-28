/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallTargetsCommand_h
#define cmInstallTargetsCommand_h

#include "cmCommand.h"

/** \class cmInstallTargetsCommand
 * \brief Specifies where to install some targets
 *
 * cmInstallTargetsCommand specifies the relative path where a list of
 * targets should be installed. The targets can be executables or
 * libraries.  
 */
class cmInstallTargetsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmInstallTargetsCommand;
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
  virtual const char* GetName() { return "install_targets";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Deprecated. Use the install(TARGETS )  command instead.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "This command has been superceded by the install command.  It "
      "is provided for compatibility with older CMake code.\n"
      "  install_targets(<dir> [RUNTIME_DIRECTORY dir] target target)\n"
      "Create rules to install the listed targets into the given directory.  "
      "The directory <dir> is relative to the installation prefix, which "
      "is stored in the variable CMAKE_INSTALL_PREFIX. If RUNTIME_DIRECTORY "
      "is specified, then on systems with special runtime files "
      "(Windows DLL), the files will be copied to that directory.";
    }
  
  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged()
    {
    return true;
    }

  cmTypeMacro(cmInstallTargetsCommand, cmCommand);
};


#endif
