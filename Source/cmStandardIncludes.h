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
 * Include header files as a function of the build process, compiler,
 * and operating system.
 */
#ifndef cmStandardIncludes_h
#define cmStandardIncludes_h

// include configure generated  header to define
// CMAKE_NO_ANSI_STREAM_HEADERS and CMAKE_NO_STD_NAMESPACE
#ifdef CMAKE_HAS_AUTOCONF
#include "cmConfigure.h"
#endif

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
// for loop scoping hack
#define for if(false) {} else for
#endif

#ifdef __ICL
#pragma warning ( disable : 985 )
#endif

#ifndef CMAKE_NO_ANSI_STREAM_HEADERS
#include <fstream>
#include <iostream>
#else
#include <fstream.h>
#include <iostream.h>
#endif

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <map>
#include <list>

#ifdef CMAKE_NO_STD_NAMESPACE
#define std 
# define for if (false) { } else for
#endif


#endif
