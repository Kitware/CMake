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
#ifndef cmSystemTools_h
#define cmSystemTools_h

#include "cmStandardIncludes.h"

#include <cmsys/SystemTools.hxx>

/** \class cmSystemTools
 * \brief A collection of useful functions for CMake.
 *
 * cmSystemTools is a class that provides helper functions
 * for the CMake build system.
 */
class cmSystemTools: public cmsys::SystemTools
{
public:
  typedef cmsys::SystemTools Superclass;
  
  /** Expand out any arguements in the vector that have ; separated
   *  strings into multiple arguements.  A new vector is created 
   *  containing the expanded versions of all arguments in argsIn.
   */
  static void ExpandList(std::vector<std::string> const& argsIn,
                         std::vector<std::string>& argsOut);
  static void ExpandListArgument(const std::string& arg,
                                 std::vector<std::string>& argsOut);

  /**
   * Look for and replace registry values in a string
   */
  static void ExpandRegistryValues(std::string& source);

  /**
   * Platform independent escape spaces, unix uses backslash,
   * windows double quotes the string.
   */
  static std::string EscapeSpaces(const char* str);

  ///! Escape quotes in a string.
  static std::string EscapeQuotes(const char* str);
  
  /**
   * Given a string, replace any escape sequences with the corresponding
   * characters.
   */
  static std::string RemoveEscapes(const char*);
  
  typedef  void (*ErrorCallback)(const char*, const char*, bool&, void*);
  /**
   *  Set the function used by GUI's to display error messages
   *  Function gets passed: message as a const char*, 
   *  title as a const char*, and a reference to bool that when
   *  set to false, will disable furthur messages (cancel).
   */
  static void SetErrorCallback(ErrorCallback f, void* clientData=0);

  /**
   * Display an error message.
   */
  static void Error(const char* m, const char* m2=0,
                    const char* m3=0, const char* m4=0);

  /**
   * Display a message.
   */
  static void Message(const char* m, const char* title=0);

  ///! Return true if there was an error at any point.
  static bool GetErrorOccuredFlag() 
    {
      return cmSystemTools::s_ErrorOccured || cmSystemTools::s_FatalErrorOccured;
    }
  ///! If this is set to true, cmake stops processing commands.
  static void SetFatalErrorOccured()
    {
      cmSystemTools::s_FatalErrorOccured = true;
    }
 ///! Return true if there was an error at any point.
  static bool GetFatalErrorOccured() 
    {
      return cmSystemTools::s_FatalErrorOccured;
    }

  ///! Set the error occured flag and fatal error back to false
  static void ResetErrorOccuredFlag()
    {
      cmSystemTools::s_FatalErrorOccured = false;
      cmSystemTools::s_ErrorOccured = false;
    }
  
  /** 
   * does a string indicate a true or on value ? This is not the same
   * as ifdef. 
   */ 
  static bool IsOn(const char* val);
  
  /** 
   * does a string indicate a false or off value ? Note that this is
   * not the same as !IsOn(...) because there are a number of
   * ambiguous values such as "/usr/local/bin" a path will result in
   * IsON and IsOff both returning false. Note that the special path
   * NOTFOUND, *-NOTFOUND or IGNORE will cause IsOff to return true. 
   */
  static bool IsOff(const char* val);

  ///! Return true if value is NOTFOUND or ends in -NOTFOUND.
  static bool IsNOTFOUND(const char* value);
  
  static bool DoesFileExistWithExtensions(
    const char *name,
    const std::vector<std::string>& sourceExts);

  static void Glob(const char *directory, const char *regexp,
                   std::vector<std::string>& files);
  static void GlobDirs(const char *fullPath, std::vector<std::string>& files);

  /**
   * Try to find a list of files that match the "simple" globbing
   * expression. At this point in time the globbing expressions have
   * to be in form: /directory/partial_file_name*. The * character has
   * to be at the end of the string and it does not support ?
   * []... The optional argument type specifies what kind of files you
   * want to find. 0 means all files, -1 means directories, 1 means
   * files only. This method returns true if search was succesfull.
   */
  static bool SimpleGlob(const cmStdString& glob, std::vector<cmStdString>& files, 
                         int type = 0);
  
  ///! Copy a file.
  static bool cmCopyFile(const char* source, const char* destination);

  /**
   * Run an executable command and put the stdout in output.
   * A temporary file is created in the binaryDir for storing the
   * output because windows does not have popen.
   *
   * If verbose is false, no user-viewable output from the program
   * being run will be generated.
   *
   * If timeout is specified, the command will be terminated after
   * timeout expires.
   */
  static bool RunCommand(const char* command, std::string& output, 
                         const char* directory = 0,
                         bool verbose = true, int timeout = 0);
  static bool RunCommand(const char* command, std::string& output,
                         int &retVal, const char* directory = 0, 
                         bool verbose = true, int timeout = 0);  
  /**
   * Run a single executable command and put the stdout and stderr 
   * in output.
   *
   * If verbose is false, no user-viewable output from the program
   * being run will be generated.
   *
   * If timeout is specified, the command will be terminated after
   * timeout expires. Timeout is specified in seconds.
   *
   * Argument retVal should be a pointer to the location where the
   * exit code will be stored. If the retVal is not specified and 
   * the program exits with a code other than 0, then the this 
   * function will return false.
   */
  static bool RunSingleCommand(const char* command, std::string* output = 0,
    int* retVal = 0, const char* dir = 0, bool verbose = true, int timeout = 0);

  /**
   * Parse arguments out of a single string command
   */
  static std::vector<cmStdString> ParseArguments(const char* command);
    
  static void EnableMessages() { s_DisableMessages = false; }
  static void DisableMessages() { s_DisableMessages = true; }
  static void DisableRunCommandOutput() {s_DisableRunCommandOutput = true; }
  static void EnableRunCommandOutput() {s_DisableRunCommandOutput = false; }
  static bool GetRunCommandOutput() { return s_DisableRunCommandOutput; }

  /**
   * Come constants for different file formats.
   */
  enum FileFormat {
    NO_FILE_FORMAT = 0,
    C_FILE_FORMAT,
    CXX_FILE_FORMAT,
    JAVA_FILE_FORMAT,
    HEADER_FILE_FORMAT,
    RESOURCE_FILE_FORMAT,
    DEFINITION_FILE_FORMAT,
    STATIC_LIBRARY_FILE_FORMAT,
    SHARED_LIBRARY_FILE_FORMAT,
    MODULE_FILE_FORMAT,
    OBJECT_FILE_FORMAT,
    UNKNOWN_FILE_FORMAT
  };

  /**
   * Determine the file type based on the extension
   */
  static FileFormat GetFileFormat(const char* ext);

  /**
   * On Windows 9x we need a comspec (command.com) substitute to run
   * programs correctly. This string has to be constant available
   * through the running of program. This method does not create a copy.
   */
  static void SetWindows9xComspecSubstitute(const char*);
  static const char* GetWindows9xComspecSubstitute();

  /** Windows if this is true, the CreateProcess in RunCommand will
   *  not show new consol windows when running programs.   
   */
  static void SetRunCommandHideConsole(bool v){s_RunCommandHideConsole = v;}
  static bool GetRunCommandHideConsole(){ return s_RunCommandHideConsole;}
  /** Call cmSystemTools::Error with the message m, plus the
   * result of strerror(errno)
   */
  static void ReportLastSystemError(const char* m);
  
  /** Split a string on its newlines into multiple lines.  Returns
      false only if the last line stored had no newline.  */
  static bool Split(const char* s, std::vector<cmStdString>& l);  
  static void SetForceUnixPaths(bool v)
    {
      s_ForceUnixPaths = v;
    }
  static std::string ConvertToOutputPath(const char* path);

  //! Check if the first string ends with the second one.
  static bool StringEndsWith(const char* str1, const char* str2);

  static bool CreateSymlink(const char* origName, const char* newName);
private:
  static bool s_ForceUnixPaths;
  static bool s_RunCommandHideConsole;
  static bool s_ErrorOccured;
  static bool s_FatalErrorOccured;
  static bool s_DisableMessages;
  static bool s_DisableRunCommandOutput;
  static ErrorCallback s_ErrorCallback;
  static void* s_ErrorCallbackClientData;

  static std::string s_Windows9xComspecSubstitute;
};

#endif
