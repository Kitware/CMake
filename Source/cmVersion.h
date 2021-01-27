/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <cm3p/kwiml/int.h>

/** \class cmVersion
 * \brief Helper class for providing CMake and CTest version information.
 *
 * Finds all version related information.
 */
class cmVersion
{
public:
  /**
   * Return major and minor version numbers for cmake.
   */
  static unsigned int GetMajorVersion();
  static unsigned int GetMinorVersion();
  static unsigned int GetPatchVersion();
  static unsigned int GetTweakVersion();
  static const char* GetCMakeVersion();
};

/* Encode with room for up to 1000 minor releases between major releases
   and to encode dates until the year 10000 in the patch level.  */
#define CMake_VERSION_ENCODE_BASE KWIML_INT_UINT64_C(100000000)
#define CMake_VERSION_ENCODE(major, minor, patch)                             \
  ((((major)*1000u) * CMake_VERSION_ENCODE_BASE) +                            \
   (((minor) % 1000u) * CMake_VERSION_ENCODE_BASE) +                          \
   (((patch) % CMake_VERSION_ENCODE_BASE)))
