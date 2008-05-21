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

#include <cmsys/DateStamp.h>

std::string cmVersion::GetReleaseVersion()
{
#if CMake_VERSION_MINOR & 1
  return cmsys_DATE_STAMP_STRING_FULL;
#else
# ifdef CMake_VERSION_RC
  return "patch " CMAKE_TO_STRING(CMake_VERSION_PATCH) " RC-" 
    CMAKE_TO_STRING(CMake_VERSION_RC);
# else
  return "patch " CMAKE_TO_STRING(CMake_VERSION_PATCH);
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
