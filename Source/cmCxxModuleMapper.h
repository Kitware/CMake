/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

enum class LookupMethod;
struct cmScanDepInfo;

enum class CxxModuleMapFormat
{
  Clang,
  Gcc,
  Msvc,
};

struct CxxBmiLocation
{
  static CxxBmiLocation Unknown();
  static CxxBmiLocation Private();
  static CxxBmiLocation Known(std::string path);

  bool IsKnown() const;
  bool IsPrivate() const;
  std::string const& Location() const;

private:
  CxxBmiLocation();
  CxxBmiLocation(std::string path);

  cm::optional<std::string> BmiLocation;
};

struct CxxModuleLocations
{
  // The path from which all relative paths should be computed. If
  // this is relative, it is relative to the compiler's working
  // directory.
  std::string RootDirectory;

  // A function to convert a full path to a path for the generator.
  std::function<std::string(std::string)> PathForGenerator;

  // Lookup the BMI location of a logical module name.
  std::function<CxxBmiLocation(std::string const&)> BmiLocationForModule;

  // Returns the generator path (if known) for the BMI given a
  // logical module name.
  CxxBmiLocation BmiGeneratorPathForModule(
    std::string const& logical_name) const;
};

struct CxxModuleReference
{
  // The path to the module file used.
  std::string Path;
  // How the module was looked up.
  LookupMethod Method;
};

struct CxxModuleUsage
{
  // The usage requirements for this object.
  std::map<std::string, std::set<std::string>> Usage;

  // The references for this object.
  std::map<std::string, CxxModuleReference> Reference;

  // Add a reference to a module.
  //
  // Returns `true` if it matches how it was found previously, `false` if it
  // conflicts.
  bool AddReference(std::string const& logical, std::string const& loc,
                    LookupMethod method);
};

// Return the extension to use for a given modulemap format.
cm::static_string_view CxxModuleMapExtension(
  cm::optional<CxxModuleMapFormat> format);

// Fill in module usage information for internal usages.
//
// Returns the set of unresolved module usage requirements (these form an
// import cycle).
std::set<std::string> CxxModuleUsageSeed(
  CxxModuleLocations const& loc, std::vector<cmScanDepInfo> const& objects,
  CxxModuleUsage& usages);

// Return the contents of the module map in the given format for the
// object file.
std::string CxxModuleMapContent(CxxModuleMapFormat format,
                                CxxModuleLocations const& loc,
                                cmScanDepInfo const& obj,
                                CxxModuleUsage const& usages);
