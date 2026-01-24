/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

/** \class cmVersion
 * \brief Helper class for providing CMake and CTest version information.
 *
 * Finds all version related information.
 */
class cmVersion
{
public:
  enum class DependencyType
  {
    System,
    Bundled,
  };

  struct DependencyInfo
  {
    /**
     * The name of the dependency.
     * e.g. "curl", "libarchive", "zlib", etc.
     */
    std::string name;
    /**
     * The version of the dependency if available.
     * e.g. "7.66.0", "3.8.0", "1.2.12", etc.
     */
    std::string version;

    /**
     * The type of the dependency.
     */
    DependencyType type;
    /**
     * The source of the dependency.
     * e.g. "curl", "libarchive", etc.
     * Empty if the dependency is directly passed from CMake.
     */
    std::string cameFrom;
  };

  /**
   * Return major and minor version numbers for cmake.
   */
  static unsigned int GetMajorVersion();
  static unsigned int GetMinorVersion();
  static unsigned int GetPatchVersion();
  static unsigned int GetTweakVersion();
  static char const* GetCMakeVersion();

  static std::vector<DependencyInfo> const& CollectDependencyInfo();
};
