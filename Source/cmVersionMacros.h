/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cm3p/kwiml/int.h>

#include "cmVersionConfig.h"

#define CMake_VERSION_PATCH_IS_RELEASE(patch) ((patch) < 20000000)
#if CMake_VERSION_PATCH_IS_RELEASE(CMake_VERSION_PATCH)
#  define CMake_VERSION_IS_RELEASE 1
#endif

/* Encode with room for up to 1000 minor releases between major releases
   and to encode dates until the year 10000 in the patch level.  */
#define CMake_VERSION_ENCODE_BASE KWIML_INT_UINT64_C(100000000)
#define CMake_VERSION_ENCODE(major, minor, patch)                             \
  ((((major) * 1000u) * CMake_VERSION_ENCODE_BASE) +                          \
   (((minor) % 1000u) * CMake_VERSION_ENCODE_BASE) +                          \
   (((patch) % CMake_VERSION_ENCODE_BASE)))

#define CMV_STRINGIFY(X) CMV_STRINGIFY_DELAY(X)
#define CMV_STRINGIFY_DELAY(X) #X

#define CMake_VERSION_DEVEL(major, minor)                                     \
  (CMake_VERSION_ENCODE(major, minor, 0) >                                    \
       CMake_VERSION_ENCODE(CMake_VERSION_MAJOR, CMake_VERSION_MINOR, 0)      \
     ? CMV_STRINGIFY(CMake_VERSION_MAJOR) "." CMV_STRINGIFY(                  \
         CMake_VERSION_MINOR) "." CMV_STRINGIFY(CMake_VERSION_PATCH)          \
     : #major "." #minor ".0")
