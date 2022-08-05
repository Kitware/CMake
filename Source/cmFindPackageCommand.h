/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include <cm3p/kwiml/int.h>

#include "cmFindCommon.h"
#include "cmPolicies.h"

// IWYU insists we should forward-declare instead of including <functional>,
// but we cannot forward-declare reliably because some C++ standard libraries
// put the template in an inline namespace.
#ifdef CMAKE_IWYU_FORWARD_STD_HASH
/* clang-format off */
namespace std {
  template <class T> struct hash;
}
/* clang-format on */
#endif

class cmExecutionStatus;
class cmSearchPath;

/** \class cmFindPackageCommand
 * \brief Load settings from an external project.
 *
 * cmFindPackageCommand
 */
class cmFindPackageCommand : public cmFindCommon
{
public:
  /*! A sorting order strategy to be applied to recovered package folders (see
   * FIND_PACKAGE_SORT_ORDER)*/
  enum /*class*/ SortOrderType
  {
    None,
    Name_order,
    Natural
  };
  /*! A sorting direction to be applied to recovered package folders (see
   * FIND_PACKAGE_SORT_DIRECTION)*/
  enum /*class*/ SortDirectionType
  {
    Asc,
    Dec
  };

  /*! sorts a given list of string based on the input sort parameters */
  static void Sort(std::vector<std::string>::iterator begin,
                   std::vector<std::string>::iterator end, SortOrderType order,
                   SortDirectionType dir);

  cmFindPackageCommand(cmExecutionStatus& status);

  bool InitialPass(std::vector<std::string> const& args);

private:
  class PathLabel : public cmFindCommon::PathLabel
  {
  protected:
    PathLabel();

  public:
    PathLabel(const std::string& label)
      : cmFindCommon::PathLabel(label)
    {
    }
    static PathLabel PackageRedirect;
    static PathLabel UserRegistry;
    static PathLabel Builds;
    static PathLabel SystemRegistry;
  };

  bool FindPackageUsingModuleMode();
  bool FindPackageUsingConfigMode();

  // Add additional search path labels and groups not present in the
  // parent class
  void AppendSearchPathGroups();

  void AppendSuccessInformation();
  void AppendToFoundProperty(bool found);
  void SetVersionVariables(
    const std::function<void(const std::string&, cm::string_view)>&
      addDefinition,
    const std::string& prefix, const std::string& version, unsigned int count,
    unsigned int major, unsigned int minor, unsigned int patch,
    unsigned int tweak);
  void SetModuleVariables(
    const std::string& components,
    const std::vector<std::pair<std::string, const char*>>& componentVarDefs);
  bool FindModule(bool& found);
  void AddFindDefinition(const std::string& var, cm::string_view value);
  void RestoreFindDefinitions();

  enum /*class*/ HandlePackageModeType
  {
    Module,
    Config
  };
  bool HandlePackageMode(HandlePackageModeType type);

  bool FindConfig();
  bool FindPrefixedConfig();
  bool FindFrameworkConfig();
  bool FindAppBundleConfig();
  enum PolicyScopeRule
  {
    NoPolicyScope,
    DoPolicyScope
  };
  bool ReadListFile(const std::string& f, PolicyScopeRule psr);
  void StoreVersionFound();
  void SetConfigDirCacheVariable(const std::string& value);

  void ComputePrefixes();
  void FillPrefixesPackageRedirect();
  void FillPrefixesPackageRoot();
  void FillPrefixesCMakeEnvironment();
  void FillPrefixesCMakeVariable();
  void FillPrefixesSystemEnvironment();
  void FillPrefixesUserRegistry();
  void FillPrefixesSystemRegistry();
  void FillPrefixesCMakeSystemVariable();
  void FillPrefixesUserGuess();
  void FillPrefixesUserHints();
  void LoadPackageRegistryDir(std::string const& dir, cmSearchPath& outPaths);
  void LoadPackageRegistryWinUser();
  void LoadPackageRegistryWinSystem();
  void LoadPackageRegistryWin(bool user, unsigned int view,
                              cmSearchPath& outPaths);
  bool CheckPackageRegistryEntry(const std::string& fname,
                                 cmSearchPath& outPaths);
  bool SearchDirectory(std::string const& dir);
  bool CheckDirectory(std::string const& dir);
  bool FindConfigFile(std::string const& dir, std::string& file);
  bool CheckVersion(std::string const& config_file);
  bool CheckVersionFile(std::string const& version_file,
                        std::string& result_version);
  bool SearchPrefix(std::string const& prefix);
  bool SearchFrameworkPrefix(std::string const& prefix_in);
  bool SearchAppBundlePrefix(std::string const& prefix_in);

  friend class cmFindPackageFileList;

  struct OriginalDef
  {
    bool exists;
    std::string value;
  };
  std::map<std::string, OriginalDef> OriginalDefs;

  std::map<std::string, cmPolicies::PolicyID> DeprecatedFindModules;

  static const cm::string_view VERSION_ENDPOINT_INCLUDED;
  static const cm::string_view VERSION_ENDPOINT_EXCLUDED;

  std::string Name;
  std::string Variable;
  std::string VersionComplete;
  std::string VersionRange;
  cm::string_view VersionRangeMin;
  cm::string_view VersionRangeMax;
  std::string Version;
  unsigned int VersionMajor = 0;
  unsigned int VersionMinor = 0;
  unsigned int VersionPatch = 0;
  unsigned int VersionTweak = 0;
  unsigned int VersionCount = 0;
  std::string VersionMax;
  unsigned int VersionMaxMajor = 0;
  unsigned int VersionMaxMinor = 0;
  unsigned int VersionMaxPatch = 0;
  unsigned int VersionMaxTweak = 0;
  unsigned int VersionMaxCount = 0;
  bool VersionExact = false;
  std::string FileFound;
  std::string VersionFound;
  unsigned int VersionFoundMajor = 0;
  unsigned int VersionFoundMinor = 0;
  unsigned int VersionFoundPatch = 0;
  unsigned int VersionFoundTweak = 0;
  unsigned int VersionFoundCount = 0;
  KWIML_INT_uint64_t RequiredCMakeVersion = 0;
  bool Quiet = false;
  bool Required = false;
  bool UseConfigFiles = true;
  bool UseFindModules = true;
  bool NoUserRegistry = false;
  bool NoSystemRegistry = false;
  bool UseLib32Paths = false;
  bool UseLib64Paths = false;
  bool UseLibx32Paths = false;
  bool UseRealPath = false;
  bool PolicyScope = true;
  bool GlobalScope = false;
  bool RegistryViewDefined = false;
  std::string LibraryArchitecture;
  std::vector<std::string> Names;
  std::vector<std::string> Configs;
  std::set<std::string> IgnoredPaths;
  std::set<std::string> IgnoredPrefixPaths;
  std::string DebugBuffer;

  /*! the selected sortOrder (None by default)*/
  SortOrderType SortOrder = None;
  /*! the selected sortDirection (Asc by default)*/
  SortDirectionType SortDirection = Asc;

  struct ConfigFileInfo
  {
    std::string filename;
    std::string version;

    bool operator<(ConfigFileInfo const& rhs) const
    {
      return this->filename < rhs.filename;
    }

    bool operator==(ConfigFileInfo const& rhs) const
    {
      return this->filename == rhs.filename;
    }

    bool operator!=(ConfigFileInfo const& rhs) const
    {
      return !(*this == rhs);
    }
  };
  std::vector<ConfigFileInfo> ConsideredConfigs;

  friend struct std::hash<ConfigFileInfo>;
};

namespace std {

template <>
struct hash<cmFindPackageCommand::ConfigFileInfo>
{
  using argument_type = cmFindPackageCommand::ConfigFileInfo;
  using result_type = size_t;

  result_type operator()(argument_type const& s) const noexcept
  {
    result_type const h(std::hash<std::string>{}(s.filename));
    return h;
  }
};
}

bool cmFindPackage(std::vector<std::string> const& args,
                   cmExecutionStatus& status);
