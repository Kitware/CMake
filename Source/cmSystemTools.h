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

/** \class cmSystemTools
 * \brief A collection of useful functions for CMake.
 *
 * cmSystemTools is a class that provides helper functions
 * for the CMake build system.
 */
class cmSystemTools
{
public:
  /**
   * Replace symbols in str that are not valid in C identifiers as
   * defined by the 1999 standard, ie. anything except [A-Za-z0-9_].
   * They are replaced with `_' and if the first character is a digit
   * then an underscore is prepended.  Note that this can produce
   * identifiers that the standard reserves (_[A-Z].* and __.*).
   */
  static std::string MakeCindentifier(const char* s);
  
  /**
   * Make a new directory if it is not there.  This function
   * can make a full path even if none of the directories existed
   * prior to calling this function.  
   */
  static bool MakeDirectory(const char* path);

  /**
   * Get current time as a double. On certain platforms this will
   * return higher resolution than seconds:
   * (1) gettimeofday() -- resolution in microseconds
   * (2) ftime() -- resolution in milliseconds
   * (3) time() -- resolution in seconds
   */
  static double GetTime();

  /**
   * Replace replace all occurances of the string in
   * the source string.
   */
  static void ReplaceString(std::string& source,
                            const char* replace,
                            const char* with);

  /** Expand out any arguements in the vector that have ; separated
   *  strings into multiple arguements.  A new vector is created 
   *  containing the expanded versions of all arguments in argsIn.
   */
  static void ExpandList(std::vector<std::string> const& argsIn,
                         std::vector<std::string>& argsOut);
  static void ExpandListArgument(const std::string& arg,
                                 std::vector<std::string>& argsOut);

  /**
   * Read a registry value
   */
  static bool ReadRegistryValue(const char *key, std::string &value);

  /**
   * Write a registry value
   */
  static bool WriteRegistryValue(const char *key, const char *value);

  /**
   * Delete a registry value
   */
  static bool DeleteRegistryValue(const char *key);

  /**
   * Look for and replace registry values in a string
   */
  static void ExpandRegistryValues(std::string& source);

  /**
   * Return a capitalized string (i.e the first letter is uppercased, all other
   * are lowercased).
   */
  static std::string Capitalized(const std::string&);
  
  /**
   * Return a lower case string
   */
  static std::string LowerCase(const std::string&);
  
  /**
   * Return a lower case string
   */
  static std::string UpperCase(const std::string&);
  
  /**
   * Replace Windows file system slashes with Unix-style slashes.
   */
  static void ConvertToUnixSlashes(std::string& path);
  
  /**
   * Platform independent escape spaces, unix uses backslash,
   * windows double quotes the string.
   */
  static std::string EscapeSpaces(const char* str);

  ///! Escape quotes in a string.
  static std::string EscapeQuotes(const char* str);

  /**
   * For windows this calles ConvertToWindowsOutputPath and for unix
   * it calls ConvertToUnixOutputPath
   */
  static std::string ConvertToOutputPath(const char*);
  
  ///! Return true if a file exists in the current directory.
  static bool FileExists(const char* filename);
  
  /**
   * Given a string, replace any escape sequences with the corresponding
   * characters.
   */
  static std::string RemoveEscapes(const char*);
  

  /**
   *  Add the paths from the environment variable PATH to the 
   *  string vector passed in.
   */
  static void GetPath(std::vector<std::string>& path);

  /**
   *  Get the file extension (including ".") needed for an executable
   *  on the current platform ("" for unix, ".exe" for Windows).
   */
  static const char* GetExecutableExtension();

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
   * Copy the source file to the destination file only
   * if the two files differ.  
   */
  static bool CopyFileIfDifferent(const char* source,
                                  const char* destination);
  
  ///! Compare the contents of two files.  Return true if different.
  static bool FilesDiffer(const char* source,
                          const char* destination);
  ///! return true if the two files are the same file
  static bool SameFile(const char* file1, const char* file2);

  ///! Copy a file.
  static void cmCopyFile(const char* source,
                       const char* destination);
  
  ///! Remove a file.
  static bool RemoveFile(const char* source);
  
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
  
  ///! Find a file in the system PATH, with optional extra paths.
  static std::string FindFile(const char* name,
                                 const std::vector<std::string>& path= std::vector<std::string>());

  ///! Find an executable in the system PATH, with optional extra paths.
  static std::string FindProgram(const char* name,
                                 const std::vector<std::string>& path = std::vector<std::string>(),
                                 bool no_system_path = false);

  ///! Find a library in the system PATH, with optional extra paths.
  static std::string FindLibrary(const char* name,
                                 const std::vector<std::string>& path);

  ///! return true if the file is a directory.
  static bool FileIsDirectory(const char* name);
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
  static bool SimpleGlob(const std::string& glob, std::vector<std::string>& files, 
                         int type = 0);
  
  static std::string GetCurrentWorkingDirectory();
  static std::string GetProgramPath(const char*);
  static void SplitProgramPath(const char* in_name, 
                               std::string& dir, 
                               std::string& file);
  static std::string CollapseFullPath(const char* in_relative);
  static std::string CollapseFullPath(const char* in_relative,
                                      const char* in_base);
  
  ///! return path of a full filename (no trailing slashes).
  static std::string GetFilenamePath(const std::string&);

  
  ///! return file name of a full filename (i.e. file name without path).
  static std::string GetFilenameName(const std::string&);
  
  ///! Split a program from its arguments and handle spaces in the paths.
  static void SplitProgramFromArgs(const char* path, 
                                   std::string& program, std::string& args);
  
  ///! return file extension of a full filename (dot included).
  static std::string GetFilenameExtension(const std::string&);
  
  ///! return file name without extension of a full filename.
  static std::string GetFilenameWithoutExtension(const std::string&);
  
  ///! return file name without its last (shortest) extension.
  static std::string GetFilenameWithoutLastExtension(const std::string&);
  
  /** Return whether the path represents a full path (not relative).  */
  static bool FileIsFullPath(const char*);
  
  static long int ModifiedTime(const char* filename);

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
  
  ///! for windows return the short path for the given path, unix just a pass through
  static bool GetShortPath(const char* path, std::string& result);
  
  ///! change directory the the directory specified
  static int ChangeDirectory(const char* dir);
    
  static void EnableMessages() { s_DisableMessages = false; }
  static void DisableMessages() { s_DisableMessages = true; }
  static void DisableRunCommandOutput() {s_DisableRunCommandOutput = true; }
  static void EnableRunCommandOutput() {s_DisableRunCommandOutput = false; }
  static bool GetRunCommandOutput() { return s_DisableRunCommandOutput; }

  /** Split a string on its newlines into multiple lines.  Returns
      false only if the last line stored had no newline.  */
  static bool Split(const char* s, std::vector<cmStdString>& l);
  
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
  
  /** When building DEBUG with MSVC, this enables a hook that prevents
   * error dialogs from popping up if the program is being run from
   * DART.
   */
  static void EnableMSVCDebugHook();
protected:
  // these two functions can be called from ConvertToOutputPath
  /**
   * Convert the path to a string that can be used in a unix makefile.
   * double slashes are removed, and spaces are escaped.
   */
  static std::string ConvertToUnixOutputPath(const char*);
  
  /**
   * Convert the path to string that can be used in a windows project or
   * makefile.   Double slashes are removed if they are not at the start of
   * the string, the slashes are converted to windows style backslashes, and
   * if there are spaces in the string it is double quoted.
   */
  static std::string ConvertToWindowsOutputPath(const char*);

private:
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
