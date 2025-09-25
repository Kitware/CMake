/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

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
  static char const* GetCMakeVersion();
};
