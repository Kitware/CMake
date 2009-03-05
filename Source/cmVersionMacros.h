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
#ifndef cmVersionMacros_h
#define cmVersionMacros_h

#include "cmVersionConfig.h"

#define CMAKE_TO_STRING(x) CMAKE_TO_STRING0(x)
#define CMAKE_TO_STRING0(x) #x

#define CMake_VERSION                      \
  CMAKE_TO_STRING(CMake_VERSION_MAJOR) "." \
  CMAKE_TO_STRING(CMake_VERSION_MINOR)

#define CMake_VERSION_FULL \
  CMAKE_TO_STRING(CMake_VERSION_MAJOR) "." \
  CMAKE_TO_STRING(CMake_VERSION_MINOR) "." \
  CMAKE_TO_STRING(CMake_VERSION_PATCH)

#if !(CMake_VERSION_MINOR & 1) && defined(CMake_VERSION_RC)
# define CMake_VERSION_RC_SUFFIX "-rc" CMAKE_TO_STRING(CMake_VERSION_RC)
#else
# define CMake_VERSION_RC_SUFFIX ""
#endif

#endif
