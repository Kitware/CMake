/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

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

  /**
   * Return a string equivalent to the input string, but with all " " replaced
   * with "\ " to escape the spaces.
   */
  static std::string EscapeSpaces(const char*);
  
  /**
   * Replace Windows file system slashes with Unix-style slashes.
   */
  static void ConvertToUnixSlashes(std::string& path);
 
  ///! Return true if a file exists in the current directory.
  static bool FileExists(const char* filename);

  /**
   * Return the number of times the given expression occurs in the file
   * specified by the concatenation of dir/file.
   */
  static int Grep(const char* dir, const char* file, const char* expression);
  
  /**
   * Convert a path containing a cygwin drive specifier to its natural
   * equivalent.
   */
  static void ConvertCygwinPath(std::string& pathname);

  /**
   * Read a CMake command (or function) from an input file.  This
   * returns the name of the function and a list of its 
   * arguments.
   */
  static bool ParseFunction(std::ifstream&, 
                            std::string& name,
                            std::vector<std::string>& arguments);

  /**
   *  Extract white-space separated arguments from a string.
   *  Double quoted strings are accepted with spaces.
   *  This is called by ParseFunction.
   */
  static void GetArguments(std::string& line,
                           std::vector<std::string>& arguments);

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
  
  /**
   * Display an error message.
   */
  static void Error(const char* m, const char* m2=0,
                    const char* m3=0, const char* m4=0);

  ///! Return true if there was an error at any point.
  static bool GetErrorOccuredFlag() 
    {
      return cmSystemTools::s_ErrorOccured;
    }
  
  /**
   * Copy the source file to the destination file only
   * if the two files differ.  
   */
  static void CopyFileIfDifferent(const char* source,
                                  const char* destination);
  
  ///! Compare the contents of two files.  Return true if different.
  static bool FilesDiffer(const char* source,
                          const char* destination);
  ///! Copy a file.
  static void cmCopyFile(const char* source,
                       const char* destination);
  
  ///! Remove a file.
  static void RemoveFile(const char* source);
  
  ///! does a string indicate a true or on value ?
  static bool IsOn(const char* val);
  
  static long int ModifiedTime(const char* filename);

private:
  static bool s_ErrorOccured;
};


#endif
