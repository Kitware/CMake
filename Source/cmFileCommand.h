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
#ifndef cmFileCommand_h
#define cmFileCommand_h

#include "cmCommand.h"

struct cmFileInstaller;

/** \class cmFileCommand
 * \brief Command for manipulation of files
 *
 */
class cmFileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmFileCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "file";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "File manipulation command.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  file(WRITE filename \"message to write\"... )\n"
      "  file(APPEND filename \"message to write\"... )\n"
      "  file(READ filename variable [LIMIT numBytes] [OFFSET offset] [HEX])\n"
      "  file(STRINGS filename variable [LIMIT_COUNT num]\n"
      "       [LIMIT_INPUT numBytes] [LIMIT_OUTPUT numBytes]\n"
      "       [LENGTH_MINIMUM numBytes] [LENGTH_MAXIMUM numBytes]\n"
      "       [NEWLINE_CONSUME] [REGEX regex]\n"
      "       [NO_HEX_CONVERSION])\n"
      "  file(GLOB variable [RELATIVE path] [globbing expressions]...)\n"
      "  file(GLOB_RECURSE variable [RELATIVE path] \n"
      "       [globbing expressions]...)\n"
      "  file(REMOVE [file1 ...])\n"
      "  file(REMOVE_RECURSE [file1 ...])\n"
      "  file(MAKE_DIRECTORY [directory1 directory2 ...])\n"
      "  file(RELATIVE_PATH variable directory file)\n"
      "  file(TO_CMAKE_PATH path result)\n"
      "  file(TO_NATIVE_PATH path result)\n"
      "  file(DOWNLOAD url file [TIMEOUT timeout] [STATUS status] [LOG log])\n"
      "WRITE will write a message into a file called 'filename'. It "
      "overwrites the file if it already exists, and creates the file "
      "if it does not exist.\n"
      "APPEND will write a message into a file same as WRITE, except "
      "it will append it to the end of the file\n"
      "READ will read the content of a file and store it into the "
      "variable. It will start at the given offset and read up to numBytes. "
      "If the argument HEX is given, the binary data will be converted to "
      "hexadecimal representation and this will be stored in the variable.\n"
      "STRINGS will parse a list of ASCII strings from a file and "
      "store it in a variable. Binary data in the file are ignored. Carriage "
      "return (CR) characters are ignored. It works also for Intel Hex and "
      "Motorola S-record files, which are automatically converted to binary "
      "format when reading them. Disable this using NO_HEX_CONVERSION.\n"
      "LIMIT_COUNT sets the maximum number of strings to return. "
      "LIMIT_INPUT sets the maximum number of bytes to read from "
      "the input file. "
      "LIMIT_OUTPUT sets the maximum number of bytes to store in the "
      "output variable. "
      "LENGTH_MINIMUM sets the minimum length of a string to return. "
      "Shorter strings are ignored. "
      "LENGTH_MAXIMUM sets the maximum length of a string to return.  Longer "
      "strings are split into strings no longer than the maximum length. "
      "NEWLINE_CONSUME allows newlines to be included in strings instead "
      "of terminating them.\n"
      "REGEX specifies a regular expression that a string must match to be "
      "returned. Typical usage \n"
      "  file(STRINGS myfile.txt myfile)\n"
      "stores a list in the variable \"myfile\" in which each item is "
      "a line from the input file.\n"
      "GLOB will generate a list of all files that match the globbing "
      "expressions and store it into the variable. Globbing expressions "
      "are similar to regular expressions, but much simpler. If RELATIVE "
      "flag is specified for an expression, the results will be returned "
      "as a relative path to the given path.\n"
      "Examples of globbing expressions include:\n"
      "   *.cxx      - match all files with extension cxx\n"
      "   *.vt?      - match all files with extension vta,...,vtz\n"
      "   f[3-5].txt - match files f3.txt, f4.txt, f5.txt\n"
      "GLOB_RECURSE will generate similar list as the regular GLOB, except "
      "it will traverse all the subdirectories of the matched directory and "
      "match the files.\n"
      "Examples of recursive globbing include:\n"
      "   /dir/*.py  - match all python files in /dir and subdirectories\n"
      "MAKE_DIRECTORY will create the given directories, also if their parent "
      "directories don't exist yet\n"
      "REMOVE will remove the given files, also in subdirectories\n"
      "REMOVE_RECURSE will remove the given files and directories, also "
      "non-empty directories\n"
      "RELATIVE_PATH will determine relative path from directory to the given"
      " file.\n"
      "TO_CMAKE_PATH will convert path into a cmake style path with unix /. "
      " The input can be a single path or a system path like \"$ENV{PATH}\". "
      " Note the double quotes around the ENV call TO_CMAKE_PATH only takes "
      " one argument.\n"
      "TO_NATIVE_PATH works just like TO_CMAKE_PATH, but will convert from "
      " a cmake style path into the native path style \\ for windows and / "
      "for UNIX.\n"
      "DOWNLOAD will download the givin URL to the given file. "
      "If LOG var is specified a log of the download will be put in var. "
      "If STATUS var is specified the status of the operation will"
      " be put in var. The status is returned in a list of length 2. "
      "The first element is the numeric return value for the operation, "
      "and the second element is a string value for the error. A 0 "
      "numeric error means no error in the operation. "
      "If TIMEOUT time is specified, the operation will "
      "timeout after time seconds, time can be specified as a float.\n";
    }

  cmTypeMacro(cmFileCommand, cmCommand);

protected:
  bool HandleRemove(std::vector<std::string> const& args, bool recurse);
  bool HandleWriteCommand(std::vector<std::string> const& args, bool append);
  bool HandleReadCommand(std::vector<std::string> const& args);
  bool HandleStringsCommand(std::vector<std::string> const& args);
  bool HandleGlobCommand(std::vector<std::string> const& args, bool recurse);
  bool HandleMakeDirectoryCommand(std::vector<std::string> const& args);

  bool HandleRelativePathCommand(std::vector<std::string> const& args);
  bool HandleCMakePathCommand(std::vector<std::string> const& args,
                              bool nativePath);
  bool HandleRPathChangeCommand(std::vector<std::string> const& args);
  bool HandleRPathCheckCommand(std::vector<std::string> const& args);
  bool HandleRPathRemoveCommand(std::vector<std::string> const& args);

  // file(INSTALL ...) related functions
  bool HandleInstallCommand(std::vector<std::string> const& args);
  bool ParseInstallArgs(std::vector<std::string> const& args,
                        cmFileInstaller& installer,
                        std::map<cmStdString, const char*>& properties,
                        int& itype,
                        std::string& destination,
                        std::string& rename,
                        std::vector<std::string>& files,
                        bool& optional
                       );
  bool DoInstall(cmFileInstaller& installer,
                 const int itype,
                 const std::string& rename,
                 const std::string& destination,
                 const std::vector<std::string>& files,
                 const bool optional
                );
  bool HandleDownloadCommand(std::vector<std::string> const& args);
  void GetTargetTypeFromString(const std::string& stype, int& itype) const;
  bool HandleInstallDestination(cmFileInstaller& installer,
                                std::string& destination);
  void HandleInstallPermissions(cmFileInstaller& installer,
                                mode_t& permissions_file,
                                mode_t& permissions_dir,
                                int itype,
                                bool use_given_permissions_file,
                                bool use_given_permissions_dir,
                                bool use_source_permissions) const;
};


#endif
