/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  virtual const char* GetName() const { return "install";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Specify rules to run at install time.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
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
      "relative to the value of CMAKE_INSTALL_PREFIX. The prefix can "
      "be relocated at install time using DESTDIR mechanism explained in the "
      "CMAKE_INSTALL_PREFIX variable documentation.\n"
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
      "executed.  During a full installation all components are installed."
      " If COMPONENT is not provided a default component \"Unspecified\" is"
      " created. The default component name may be controlled with the "
      "CMAKE_INSTALL_DEFAULT_COMPONENT_NAME variable.\n"
      "The RENAME argument specifies a name for an installed file that "
      "may be different from the original file.  Renaming is allowed only "
      "when a single file is installed by the command.\n"
      "The OPTIONAL argument specifies that it is not an error if the "
      "file to be installed does not exist.  "
      "\n"
      "The TARGETS signature:\n"
      "  install(TARGETS targets... [EXPORT <export-name>]\n"
      "          [[ARCHIVE|LIBRARY|RUNTIME|FRAMEWORK|BUNDLE|\n"
      "            PRIVATE_HEADER|PUBLIC_HEADER|RESOURCE]\n"
      "           [DESTINATION <dir>]\n"
      "           [PERMISSIONS permissions...]\n"
      "           [CONFIGURATIONS [Debug|Release|...]]\n"
      "           [COMPONENT <component>]\n"
      "           [OPTIONAL] [NAMELINK_ONLY|NAMELINK_SKIP]\n"
      "          ] [...])\n"
      "The TARGETS form specifies rules for installing targets from a "
      "project.  There are five kinds of target files that may be "
      "installed: ARCHIVE, LIBRARY, RUNTIME, FRAMEWORK, and BUNDLE.  "

      "Executables are treated as RUNTIME targets, except that those "
      "marked with the MACOSX_BUNDLE property are treated as BUNDLE "
      "targets on OS X. "
      "Static libraries are always treated as ARCHIVE targets. "
      "Module libraries are always treated as LIBRARY targets. "
      "For non-DLL platforms shared libraries are treated as LIBRARY "
      "targets, except that those marked with the FRAMEWORK property "
      "are treated as FRAMEWORK targets on OS X.  "
      "For DLL platforms the DLL part of a shared library is treated as "
      "a RUNTIME target and the corresponding import library is treated as "
      "an ARCHIVE target. "
      "All Windows-based systems including Cygwin are DLL platforms. "
      "The ARCHIVE, LIBRARY, RUNTIME, and FRAMEWORK "
      "arguments change the type of target to which the subsequent "
      "properties "
      "apply.  If none is given the installation properties apply to "
      "all target types.  If only one is given then only targets of that "
      "type will be installed (which can be used to install just a DLL or "
      "just an import library)."
      "\n"
      "The PRIVATE_HEADER, PUBLIC_HEADER, and RESOURCE arguments cause "
      "subsequent properties to be applied to installing a FRAMEWORK "
      "shared library target's associated files on non-Apple platforms.  "
      "Rules defined by these arguments are ignored on Apple platforms "
      "because the associated files are installed into the appropriate "
      "locations inside the framework folder.  "
      "See documentation of the PRIVATE_HEADER, PUBLIC_HEADER, and RESOURCE "
      "target properties for details."
      "\n"
      "Either NAMELINK_ONLY or NAMELINK_SKIP may be specified as a LIBRARY "
      "option.  "
      "On some platforms a versioned shared library has a symbolic link "
      "such as\n"
      "  lib<name>.so -> lib<name>.so.1\n"
      "where \"lib<name>.so.1\" is the soname of the library and "
      "\"lib<name>.so\" is a \"namelink\" allowing linkers to find the "
      "library when given \"-l<name>\".  "
      "The NAMELINK_ONLY option causes installation of only the namelink "
      "when a library target is installed.  "
      "The NAMELINK_SKIP option causes installation of library files other "
      "than the namelink when a library target is installed.  "
      "When neither option is given both portions are installed.  "
      "On platforms where versioned shared libraries do not have namelinks "
      "or when a library is not versioned the NAMELINK_SKIP option installs "
      "the library and the NAMELINK_ONLY option installs nothing.  "
      "See the VERSION and SOVERSION target properties for details on "
      "creating versioned shared libraries."
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
      "will be installed to <prefix>/lib/static and /some/full/path."
      "\n"
      "The EXPORT option associates the installed target files with an "
      "export called <export-name>.  "
      "It must appear before any RUNTIME, LIBRARY, or ARCHIVE options.  "
      "To actually install the export file itself, call install(EXPORT).  "
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
      "          [USE_SOURCE_PERMISSIONS] [OPTIONAL]\n"
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
  bool CheckCMP0006(bool& failure);

  std::string DefaultComponentName;
};


#endif
