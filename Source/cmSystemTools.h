/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

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
   * Make a new directory if it is not there.  This function
   * can make a full path even if none of the directories existed
   * prior to calling this function.  
   */
  static bool MakeDirectory(const char* path);

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
  static void ExpandListArguments(std::vector<std::string> const& argsIn,
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
   * Read a CMake command (or function) from an input file.  This
   * returns the name of the function and a list of its 
   * arguments.  The last argument is the name of the file that 
   * the ifstream points to, and is used for debug info only.
   */
  static bool ParseFunction(std::ifstream&, 
                            std::string& name,
                            std::vector<std::string>& arguments,
                            const char* filename, bool& parseError);

  /**
   *  Extract white-space separated arguments from a string.
   *  Double quoted strings are accepted with spaces.
   *  This is called by ParseFunction.
   */
  static void GetArguments(std::string& line,
                           std::vector<std::string>& arguments);
  
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

  typedef  void (*ErrorCallback)(const char*, const char*, bool&);
  /**
   *  Set the function used by GUI's to display error messages
   *  Function gets passed: message as a const char*, 
   *  title as a const char*, and a reference to bool that when
   *  set to false, will disable furthur messages (cancel).
   */
  static void SetErrorCallback(ErrorCallback f);

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
      return cmSystemTools::s_ErrorOccured;
    }

  ///! Set the error occured flag back to false
  static void ResetErrorOccuredFlag()
    {
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
   * NOTFOUND or IGNORE will cause IsOff to return true. 
   */
  static bool IsOff(const char* val);

  ///! Find a file in the system PATH, with optional extra paths.
  static std::string FindFile(const char* name,
				 const std::vector<std::string>& path= std::vector<std::string>());

  ///! Find an executable in the system PATH, with optional extra paths.
  static std::string FindProgram(const char* name,
				 const std::vector<std::string>& path= std::vector<std::string>());

  ///! Find a library in the system PATH, with optional extra paths.
  static std::string FindLibrary(const char* name,
				 const std::vector<std::string>& path);

  ///! return true if the file is a directory.
  static bool FileIsDirectory(const char* name);
  static void Glob(const char *directory, const char *regexp,
                   std::vector<std::string>& files);
  static void GlobDirs(const char *fullPath, std::vector<std::string>& files);
  
  static std::string GetCurrentWorkingDirectory();
  static std::string GetProgramPath(const char*);
  static void SplitProgramPath(const char* in_name, 
                               std::string& dir, 
                               std::string& file);
  static std::string CollapseFullPath(const char*);

  ///! return path of a full filename (no trailing slashes).
  static std::string GetFilenamePath(const std::string&);
  
  ///! return file name of a full filename (i.e. file name without path).
  static std::string GetFilenameName(const std::string&);
  
  ///! return file extension of a full filename (dot included).
  static std::string GetFilenameExtension(const std::string&);
  static std::string GetFilenameShortestExtension(const std::string&);
  
  ///! return file name without extension of a full filename.
  static std::string GetFilenameNameWithoutExtension(const std::string&);
  
  static long int ModifiedTime(const char* filename);

  /**
   * Run an executable command and put the stdout in output.
   * A temporary file is created in the binaryDir for storing the
   * output because windows does not have popen.
   *
   * If verbose is false, no user-viewable output from the program
   * being run will be generated.
   */
  static bool RunCommand(const char* command, std::string& output, const char* directory = 0,
                         bool verbose = true);
  static bool RunCommand(const char* command, std::string& output,
                         int &retVal, const char* directory = 0, bool verbose = true);
  
  ///! for windows return the short path for the given path, unix just a pass through
  static bool GetShortPath(const char* path, std::string& result);
  
  ///! change directory the the directory specified
  static int ChangeDirectory(const char* dir);
    
  static void EnableMessages() { s_DisableMessages = false; }
  static void DisableMessages() { s_DisableMessages = true; }
  static void DisableRunCommandOutput() {s_DisableRunCommandOutput = true; }
  static void EnableRunCommandOutput() {s_DisableRunCommandOutput = false; }
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
  static bool s_ErrorOccured;
  static bool s_DisableMessages;
  static bool s_DisableRunCommandOutput;
  static ErrorCallback s_ErrorCallback;
};


#endif
