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
#ifndef cmInstallCommand_h
#define cmInstallCommand_h

#include "cmCommand.h"

/** \class cmInstallCommand
 * \brief Specifies where to install some files
 *
 * cmInstallCommand is a general-purpose interface command for
 * specifying install rules.
 */
class cmInstallCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmInstallCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "INSTALL";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Specify rules to run at install time.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "This command generates installation rules for a project.  "
      "Rules specified by calls to this command within a source directory "
      "are executed in order during installation.  "
      "The order across directories is not defined."
      "\n"
      "There are multiple signatures for this command.  Some of them define "
      "installation properties for files and targets.  Properties common to "
      "multiple signatures are covered here but they are valid only for "
      "signatures that specify them.  "
      "DESTINATION arguments specify "
      "the directory on disk to which a file will be installed.  "
      "If a full path (with a leading slash or drive letter) is given it "
      "is used directly.  If a relative path is given it is interpreted "
      "relative to the value of CMAKE_INSTALL_PREFIX.  "
      "PERMISSIONS arguments specify permissions for installed files.  "
      "Valid permissions are "
      "OWNER_READ, OWNER_WRITE, OWNER_EXECUTE, "
      "GROUP_READ, GROUP_WRITE, GROUP_EXECUTE, "
      "WORLD_READ, WORLD_WRITE, WORLD_EXECUTE, "
      "SETUID, and SETGID.  "
      "Permissions that do not make sense on certain platforms are ignored "
      "on those platforms.  "
      "The COMPONENT argument specifies an installation component name "
      "with which the install rule is associated, such as \"runtime\" or "
      "\"development\".  During component-specific installation only "
      "install rules associated with the given component name will be "
      "executed.  During a full installation all components are installed.  "
      "The RENAME argument specifies a name for an installed file that "
      "may be different from the original file.  Renaming is allowed only "
      "when a single file is installed by the command.  "
      "\n"
      "The TARGETS signature:\n"
      "  INSTALL(TARGETS targets... [[ARCHIVE|LIBRARY|RUNTIME]\n"
      "                              [DESTINATION <dir>]\n"
      "                              [PERMISSIONS permissions...]\n"
      "                              [COMPONENT <component>]\n"
      "                             ] [...])\n"
      "The TARGETS form specifies rules for installing targets from a "
      "project.  There are three kinds of target files that may be "
      "installed: archive, library, and runtime.  "

      "Executables are always treated as runtime targets. "
      "Static libraries are always treated as archive targets. "
      "Module libraries are always treated as library targets. "
      "For non-DLL platforms shared libraries are treated as library targets. "
      "For DLL platforms the DLL part of a shared library is treated as "
      "a runtime target and the corresponding import library is treated as "
      "an archive target. "
      "All Windows-based systems including Cygwin are DLL platforms. "
      "The ARCHIVE, LIBRARY, and RUNTIME "
      "arguments change the type of target to which the subsequent properties "
      "apply.  If none is given the installation properties apply to "
      "all target types.  If only one is given then only targets of that "
      "type will be installed (which can be used to install just a DLL or "
      "just an import library)."
      "\n"
      "One or more groups of properties may be specified in a single call "
      "to the TARGETS form of this command.  A target may be installed more "
      "than once to different locations.  Consider hypothetical "
      "targets \"myExe\", \"mySharedLib\", and \"myStaticLib\".  The code\n"
      "    INSTALL(TARGETS myExe mySharedLib myStaticLib\n"
      "            RUNTIME DESTINATION bin\n"
      "            LIBRARY DESTINATION lib\n"
      "            ARCHIVE DESTINATION lib/static)\n"
      "    INSTALL(TARGETS mySharedLib DESTINATION /some/full/path)\n"
      "will install myExe to <prefix>/bin and myStaticLib to "
      "<prefix>/lib/static.  "
      "On non-DLL platforms mySharedLib will be installed to <prefix>/lib and "
      "/some/full/path.  On DLL platforms the mySharedLib DLL will be "
      "installed to <prefix>/bin and /some/full/path and its import library "
      "will be installed to <prefix>/lib/static and /some/full/path. "
      "On non-DLL platforms mySharedLib will be installed to <prefix>/lib and "
      "/some/full/path."
      "\n"
      "The FILES signature:\n"
      "  INSTALL(FILES files... DESTINATION <dir>\n"
      "          [PERMISSIONS permissions...]\n"
      "          [COMPONENT <component>]\n"
      "          [RENAME <name>])\n"
      "The FILES form specifies rules for installing files for a "
      "project.  File names given as relative paths are interpreted with "
      "respect to the current source directory.  Files installed by this "
      "form are by default given permissions OWNER_WRITE, OWNER_READ, "
      "GROUP_READ, and WORLD_READ if no PERMISSIONS argument is given."
      "\n"
      "The PROGRAMS signature:\n"
      "  INSTALL(PROGRAMS files... DESTINATION <dir>\n"
      "          [PERMISSIONS permissions...]\n"
      "          [COMPONENT <component>]\n"
      "          [RENAME <name>])\n"
      "The PROGRAMS form is identical to the FILES form except that the "
      "default permissions for the installed file also include "
      "OWNER_EXECUTE, GROUP_EXECUTE, and WORLD_EXECUTE.  "
      "This form is intended to install programs that are not targets, "
      "such as shell scripts.  Use the TARGETS form to install targets "
      "built within the project."
      "\n"
      "The SCRIPT and CODE signature:\n"
      "  INSTALL([[SCRIPT <file>] [CODE <code>]] [...])\n"
      "The SCRIPT form will invoke the given CMake script files during "
      "installation.  If the script file name is a relative path "
      "it will be interpreted with respect to the current source directory.  "
      "The CODE form will invoke the given CMake code during installation.  "
      "Code is specified as a single argument inside a double-quoted string.  "
      "For example, the code\n"
      "  INSTALL(CODE \"MESSAGE(\\\"Sample install message.\\\")\")\n"
      "will print a message during installation.\n"
      "NOTE: This command supercedes the INSTALL_TARGETS command and the "
      "target properties PRE_INSTALL_SCRIPT and POST_INSTALL_SCRIPT.  "
      "It also replaces the FILES forms of the INSTALL_FILES and "
      "INSTALL_PROGRAMS commands.  "
      "The processing order of these install rules relative to those "
      "generated by INSTALL_TARGETS, INSTALL_FILES, and INSTALL_PROGRAMS "
      "commands is not defined.\n"
      ;
    }

  cmTypeMacro(cmInstallCommand, cmCommand);

private:
  bool HandleScriptMode(std::vector<std::string> const& args);
  bool HandleTargetsMode(std::vector<std::string> const& args);
  bool HandleFilesMode(std::vector<std::string> const& args);
  void ComputeDestination(const char* destination, std::string& dest);
  bool CheckPermissions(std::string const& arg, std::string& permissions);
};


#endif
