/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <string>

#include <cm/optional>
#include <cmext/string_view>

struct cmScanDepInfo;

enum class CxxModuleMapFormat
{
  Gcc,
};

struct CxxModuleLocations
{
  // The path from which all relative paths should be computed. If
  // this is relative, it is relative to the compiler's working
  // directory.
  std::string RootDirectory;

  // A function to convert a full path to a path for the generator.
  std::function<std::string(std::string const&)> PathForGenerator;

  // Lookup the BMI location of a logical module name.
  std::function<cm::optional<std::string>(std::string const&)>
    BmiLocationForModule;

  // Returns the generator path (if known) for the BMI given a
  // logical module name.
  cm::optional<std::string> BmiGeneratorPathForModule(
    std::string const& logical_name) const;
};

// Return the extension to use for a given modulemap format.
cm::static_string_view CxxModuleMapExtension(
  cm::optional<CxxModuleMapFormat> format);

// Return the contents of the module map in the given format for the
// object file.
std::string CxxModuleMapContent(CxxModuleMapFormat format,
                                CxxModuleLocations const& loc,
                                cmScanDepInfo const& obj);
