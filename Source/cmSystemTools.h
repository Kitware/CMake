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
#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include <string>
#include <vector>
#include <fstream>

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
   *  Remove extra spaces and the trailing \ from a string.
   */
  static std::string CleanUpName(const char* name);
  /**
   * Replace windows slashes with unix style slashes
   */
  static void ConvertToUnixSlashes(std::string& path);
 
  /**
   * Return true if a file exists
   */
  static bool FileExists(const char* filename);
  /**
   * Read a list from a file into the array of strings.
   * This function assumes that the first line of the
   * list has been read.  For example: NAME = \ was already
   * read in.   The reading stops when there are no more
   * continuation characters.
   */
  static void ReadList(std::vector<std::string>& stringList, 
                       std::ifstream& fin);
};


#endif
