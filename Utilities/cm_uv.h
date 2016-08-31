/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2016 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cm_uv_h
#define cm_uv_h

/* Use the libuv library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_LIBUV
#include <uv.h>
#else
#include <cmlibuv/include/uv.h>
#endif

#endif
