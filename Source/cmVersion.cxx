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
#include "cmVersion.h"

std::string cmVersion::GetReleaseVersion()
{
#if CMake_VERSION_MINOR & 1
  std::string cver = "Date: 2007-11-02 02:50:39 +0000";
  std::string res = "";
  std::string::size_type cc, len = cver.size();
  bool aftercol = false;
  int cnt = 0;
  for ( cc = 0; cc < len; cc ++ )
    {
    if ( aftercol )
      {
      char ch = cver[cc];
      switch ( ch )
        {
      case ' ': 
      case ':':
      case '/':
      case '-':
      case '$':
        break;
      default:
        res += ch;
        cnt ++;
        }
      if ( cnt >= 8 )
        {
        return res;
        }
      }
    else
      {
      if ( cver[cc] == ':' )
        {
        aftercol = true;
        }
      }
    }
  return res;
#else
# if CMake_VERSION_PATCH == 1
  return "1-beta";
# else
#   ifdef CMake_VERSION_RC
  return "patch " CMAKE_TO_STRING(CMake_VERSION_PATCH) " RC-" 
    CMAKE_TO_STRING(CMake_VERSION_RC);
#   else
  return "patch " CMAKE_TO_STRING(CMake_VERSION_PATCH);
#   endif
# endif  
#endif
}

std::string cmVersion::GetCMakeVersion()
{
  cmOStringStream str;
  str << CMake_VERSION_MAJOR << "." << CMake_VERSION_MINOR
    << "-"
    << cmVersion::GetReleaseVersion();
  return str.str();
}
