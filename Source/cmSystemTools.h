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
 * cmWindowsTools
 */
#ifndef cmWindowsTools_h
#define cmWindowsTools_h
#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include <string>

class cmSystemTools
{
public:
  static bool MakeDirectory(const char* path);
  static void ReplaceString(std::string& source,
                            const char* replace,
                            const char* with);
};


#endif
