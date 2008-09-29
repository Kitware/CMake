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
#ifndef __cm_curl_h
#define __cm_curl_h

/* Use the curl library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_CURL
# include <curl/curl.h>
#else
# ifdef CMAKE_USE_NEW_CURL
#   include <cmcurl-7.19.0/include/curl/curl.h>
# else CMAKE_USE_NEW_CURL
#   include <cmcurl/curl/curl.h>
# endif
#endif

#endif
