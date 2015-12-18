/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cm_kwiml_h
#define cm_kwiml_h

/* Use the KWIML library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_KWIML
# include <kwiml/abi.h>
# include <kwiml/int.h>
#else
# include "KWIML/include/kwiml/abi.h"
# include "KWIML/include/kwiml/int.h"
#endif

#endif
