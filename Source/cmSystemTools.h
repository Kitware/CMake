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
/**
 * cmSystemTools - a collection of useful functions for CMake.
 */
#ifndef cmSystemTools_h
#define cmSystemTools_h

#include "cmStandardIncludes.h"

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
   * Replace replace all occurances of the string in in
   * souce string.
   */
  static void ReplaceString(std::string& source,
                            const char* replace,
                            const char* with);
  /**
   * Replace windows slashes with unix style slashes
   */
  static void ConvertToUnixSlashes(std::string& path);
 
  /**
   * Return true if a file exists
   */
  static bool FileExists(const char* filename);
  /**
   * Return the number of times expression occurs in file in dir
   */
  static int Grep(const char* dir, const char* file, const char* expression);
  
  /**
   * remove /cygdrive/d and replace with d:/
   */
  static void ConvertCygwinPath(std::string& pathname);

  /**
   * Read a cmake function from an input file.  This
   * returns the name of the function and a list of its 
   * arguments.
   */
  static bool ParseFunction(std::ifstream&, 
                            std::string& name,
                            std::vector<std::string>& arguments);
  /**
   *  Extract space separated arguments from a string.
   *  Double quoted strings are accepted with spaces.
   *  This is called by ParseFunction.
   */
  static void GetArguments(std::string& line,
                           std::vector<std::string>& arguments);
  /**
   * Display an error message.
   */
  static void Error(const char* m, const char* m2=0 );
  
};


#endif
