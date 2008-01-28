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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "install";}

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
      "signatures that specify them.\n"
      "DESTINATION arguments specify "
      "the directory on disk to which a file will be installed.  "
      "If a full path (with a leading slash or drive letter) is given it "
      "is used directly.  If a relative path is given it is interpreted "
      "relative to the value of CMAKE_INSTALL_PREFIX.\n"
      "PERMISSIONS arguments specify permissions for installed files.  "
      "Valid permissions are "
      "OWNER_READ, OWNER_WRITE, OWNER_EXECUTE, "
      "GROUP_READ, GROUP_WRITE, GROUP_EXECUTE, "
      "WORLD_READ, WORLD_WRITE, WORLD_EXECUTE, "
      "SETUID, and SETGID.  "
      "Permissions that do not make sense on certain platforms are ignored "
      "on those platforms.\n"
      "The CONFIGURATIONS argument specifies a list of build configurations "
      "for which the install rule applies (Debug, Release, etc.).\n"
      "The COMPONENT argument specifies an installation component name "
      "with which the install rule is associated, such as \"runtime\" or "
      "\"development\".  During component-specific installation only "
      "install rules associated with the given component name will be "
      "executed.  During a full installation all components are installed.\n"
      "The RENAME argument specifies a name for an installed file that "
      "may be different from the original file.  Renaming is allowed only "
      "when a single file is installed by the command.\n"
      "The OPTIONAL argument specifies that it is not an error if the "
      "file to be installed does not exist.  "
      "\n"
      "The TARGETS signature:\n"
      "  install(TARGETS targets... [EXPORT <export-name>]\n"
      "          [[ARCHIVE|LIBRARY|RUNTIME]\n"
      "           [DESTINATION <dir>]\n"
      "           [PERMISSIONS permissions...]\n"
      "           [CONFIGURATIONS [Debug|Release|...]]\n"
      "           [COMPONENT <component>]\n"
      "           [OPTIONAL]\n"
      "          ] [...])\n"
      "The TARGETS form specifies rules for installing targets from a "
      "project.  There are three kinds of target files that may be "
      "installed: archive, library, and runtime.  "

      "Executables are always treated as runtime targets. "
      "Static libraries are always treated as archive targets. "
      "Module libraries are always treated as library targets. "
      "For non-DLL platforms shared libraries are treated as library "
      "targets. "
      "For DLL platforms the DLL part of a shared library is treated as "
      "a runtime target and the corresponding import library is treated as "
      "an archive target. "
      "All Windows-based systems including Cygwin are DLL platforms. "
      "The ARCHIVE, LIBRARY, and RUNTIME "
      "arguments change the type of target to which the subsequent "
      "properties "
      "apply.  If none is given the installation properties apply to "
      "all target types.  If only one is given then only targets of that "
      "type will be installed (which can be used to install just a DLL or "
      "just an import library)."
      "\n"
      "One or more groups of properties may be specified in a single call "
      "to the TARGETS form of this command.  A target may be installed more "
      "than once to different locations.  Consider hypothetical "
      "targets \"myExe\", \"mySharedLib\", and \"myStaticLib\".  The code\n"
      "    install(TARGETS myExe mySharedLib myStaticLib\n"
      "            RUNTIME DESTINATION bin\n"
      "            LIBRARY DESTINATION lib\n"
      "            ARCHIVE DESTINATION lib/static)\n"
      "    install(TARGETS mySharedLib DESTINATION /some/full/path)\n"
      "will install myExe to <prefix>/bin and myStaticLib to "
      "<prefix>/lib/static.  "
      "On non-DLL platforms mySharedLib will be installed to <prefix>/lib "
      "and /some/full/path.  On DLL platforms the mySharedLib DLL will be "
      "installed to <prefix>/bin and /some/full/path and its import library "
      "will be installed to <prefix>/lib/static and /some/full/path. "
      "On non-DLL platforms mySharedLib will be installed to <prefix>/lib "
      "and /some/full/path."
      "\n"
      "The EXPORT option associates the installed target files with an "
      "export called <export-name>.  "
      "It must appear before any RUNTIME, LIBRARY, or ARCHIVE options.  "
      "See documentation of the install(EXPORT ...) signature below for "
      "details."
      "\n"
      "Installing a target with EXCLUDE_FROM_ALL set to true has "
      "undefined behavior."
      "\n"
      "The FILES signature:\n"
      "  install(FILES files... DESTINATION <dir>\n"
      "          [PERMISSIONS permissions...]\n"
      "          [CONFIGURATIONS [Debug|Release|...]]\n"
      "          [COMPONENT <component>]\n"
      "          [RENAME <name>] [OPTIONAL])\n"
      "The FILES form specifies rules for installing files for a "
      "project.  File names given as relative paths are interpreted with "
      "respect to the current source directory.  Files installed by this "
      "form are by default given permissions OWNER_WRITE, OWNER_READ, "
      "GROUP_READ, and WORLD_READ if no PERMISSIONS argument is given."
      "\n"
      "The PROGRAMS signature:\n"
      "  install(PROGRAMS files... DESTINATION <dir>\n"
      "          [PERMISSIONS permissions...]\n"
      "          [CONFIGURATIONS [Debug|Release|...]]\n"
      "          [COMPONENT <component>]\n"
      "          [RENAME <name>] [OPTIONAL])\n"
      "The PROGRAMS form is identical to the FILES form except that the "
      "default permissions for the installed file also include "
      "OWNER_EXECUTE, GROUP_EXECUTE, and WORLD_EXECUTE.  "
      "This form is intended to install programs that are not targets, "
      "such as shell scripts.  Use the TARGETS form to install targets "
      "built within the project."
      "\n"
      "The DIRECTORY signature:\n"
      "  install(DIRECTORY dirs... DESTINATION <dir>\n"
      "          [FILE_PERMISSIONS permissions...]\n"
      "          [DIRECTORY_PERMISSIONS permissions...]\n"
      "          [USE_SOURCE_PERMISSIONS]\n"
      "          [CONFIGURATIONS [Debug|Release|...]]\n"
      "          [COMPONENT <component>] [FILES_MATCHING]\n"
      "          [[PATTERN <pattern> | REGEX <regex>]\n"
      "           [EXCLUDE] [PERMISSIONS permissions...]] [...])\n"
      "The DIRECTORY form installs contents of one or more directories "
      "to a given destination.  "
      "The directory structure is copied verbatim to the destination.  "
      "The last component of each directory name is appended to the "
      "destination directory but a trailing slash may be used to "
      "avoid this because it leaves the last component empty.  "
      "Directory names given as relative paths are interpreted with "
      "respect to the current source directory.  "
      "If no input directory names are given the destination directory "
      "will be created but nothing will be installed into it.  "
      "The FILE_PERMISSIONS and DIRECTORY_PERMISSIONS options specify "
      "permissions given to files and directories in the destination.  "
      "If USE_SOURCE_PERMISSIONS is specified and FILE_PERMISSIONS is not, "
      "file permissions will be copied from the source directory structure.  "
      "If no permissions are specified files will be given the default "
      "permissions specified in the FILES form of the command, and the "
      "directories will be given the default permissions specified in the "
      "PROGRAMS form of the command.\n"

      "Installation of directories may be controlled with fine granularity "
      "using the PATTERN or REGEX options.  These \"match\" options specify a "
      "globbing pattern or regular expression to match directories or files "
      "encountered within input directories.  They may be used to apply "
      "certain options (see below) to a subset of the files and directories "
      "encountered.  "
      "The full path to each input file or directory "
      "(with forward slashes) is matched against the expression.  "
      "A PATTERN will match only complete file names: the portion of the "
      "full path matching the pattern must occur at the end of the file name "
      "and be preceded by a slash.  "
      "A REGEX will match any portion of the full path but it may use "
      "'/' and '$' to simulate the PATTERN behavior.  "
      "By default all files and directories are installed whether "
      "or not they are matched.  "
      "The FILES_MATCHING option may be given before the first match option "
      "to disable installation of files (but not directories) not matched by "
      "any expression.  For example, the code\n"
      "  install(DIRECTORY src/ DESTINATION include/myproj\n"
      "          FILES_MATCHING PATTERN \"*.h\")\n"
      "will extract and install header files from a source tree.\n"
      "Some options may follow a PATTERN or REGEX expression and are "
      "applied only to files or directories matching them.  "
      "The EXCLUDE option will skip the matched file or directory.  "
      "The PERMISSIONS option overrides the permissions setting for the "
      "matched file or directory.  "
      "For example the code\n"
      "  install(DIRECTORY icons scripts/ DESTINATION share/myproj\n"
      "          PATTERN \"CVS\" EXCLUDE\n"
      "          PATTERN \"scripts/*\"\n"
      "          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ\n"
      "                      GROUP_EXECUTE GROUP_READ)\n"
      "will install the icons directory to share/myproj/icons and the "
      "scripts directory to share/myproj.  The icons will get default file "
      "permissions, the scripts will be given specific permissions, and "
      "any CVS directories will be excluded."
      "\n"
      "The SCRIPT and CODE signature:\n"
      "  install([[SCRIPT <file>] [CODE <code>]] [...])\n"
      "The SCRIPT form will invoke the given CMake script files during "
      "installation.  If the script file name is a relative path "
      "it will be interpreted with respect to the current source directory.  "
      "The CODE form will invoke the given CMake code during installation.  "
      "Code is specified as a single argument inside a double-quoted string. "
      "For example, the code\n"
      "  install(CODE \"MESSAGE(\\\"Sample install message.\\\")\")\n"
      "will print a message during installation.\n"
      ""
      "The EXPORT signature:\n"
      "  install(EXPORT <export-name> DESTINATION <dir>\n"
      "          [NAMESPACE <namespace>] [FILE <name>.cmake]\n"
      "          [PERMISSIONS permissions...]\n"
      "          [CONFIGURATIONS [Debug|Release|...]]\n"
      "          [COMPONENT <component>])\n"
      "The EXPORT form generates and installs a CMake file containing code "
      "to import targets from the installation tree into another project.  "
      "Target installations are associated with the export <export-name> "
      "using the EXPORT option of the install(TARGETS ...) signature "
      "documented above.  The NAMESPACE option will prepend <namespace> to "
      "the target names as they are written to the import file.  "
      "By default the generated file will be called <export-name>.cmake but "
      "the FILE option may be used to specify a different name.  The value "
      "given to the FILE option must be a file name with the \".cmake\" "
      "extension.  "
      "If a CONFIGURATIONS option is given then the file will only be "
      "installed when one of the named configurations is installed.  "
      "Additionally, the generated import file will reference only the "
      "matching target configurations.  "
      "If a COMPONENT option is specified that does not match that given "
      "to the targets associated with <export-name> the behavior is "
      "undefined.  "
      "If a library target is included in the export but "
      "a target to which it links is not included the behavior is "
      "unspecified."
      "\n"
      "The EXPORT form is useful to help outside projects use targets built "
      "and installed by the current project.  For example, the code\n"
      "  install(TARGETS myexe EXPORT myproj DESTINATION bin)\n"
      "  install(EXPORT myproj NAMESPACE mp_ DESTINATION lib/myproj)\n"
      "will install the executable myexe to <prefix>/bin and code to import "
      "it in the file \"<prefix>/lib/myproj/myproj.cmake\".  "
      "An outside project may load this file with the include command "
      "and reference the myexe executable from the installation tree using "
      "the imported target name mp_myexe as if the target were built "
      "in its own tree."
      "\n"
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
  bool HandleDirectoryMode(std::vector<std::string> const& args);
  bool HandleExportMode(std::vector<std::string> const& args);
  bool MakeFilesFullPath(const char* modeName, 
                         const std::vector<std::string>& relFiles,
                         std::vector<std::string>& absFiles);
};


#endif
