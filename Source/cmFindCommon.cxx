/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFindCommon.h"

#include <algorithm>
#include <array>
#include <utility>

#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

cmFindCommon::PathGroup cmFindCommon::PathGroup::All("ALL");
cmFindCommon::PathLabel cmFindCommon::PathLabel::PackageRoot(
  "PackageName_ROOT");
cmFindCommon::PathLabel cmFindCommon::PathLabel::CMake("CMAKE");
cmFindCommon::PathLabel cmFindCommon::PathLabel::CMakeEnvironment(
  "CMAKE_ENVIRONMENT");
cmFindCommon::PathLabel cmFindCommon::PathLabel::Hints("HINTS");
cmFindCommon::PathLabel cmFindCommon::PathLabel::SystemEnvironment(
  "SYSTM_ENVIRONMENT");
cmFindCommon::PathLabel cmFindCommon::PathLabel::CMakeSystem("CMAKE_SYSTEM");
cmFindCommon::PathLabel cmFindCommon::PathLabel::Guess("GUESS");

cmFindCommon::cmFindCommon(cmExecutionStatus& status)
  : Makefile(&status.GetMakefile())
  , Status(status)
{
  this->FindRootPathMode = RootPathModeBoth;
  this->NoDefaultPath = false;
  this->NoPackageRootPath = false;
  this->NoCMakePath = false;
  this->NoCMakeEnvironmentPath = false;
  this->NoSystemEnvironmentPath = false;
  this->NoCMakeSystemPath = false;
  this->NoCMakeInstallPath = false;

// OS X Bundle and Framework search policy.  The default is to
// search frameworks first on apple.
#if defined(__APPLE__)
  this->SearchFrameworkFirst = true;
  this->SearchAppBundleFirst = true;
#else
  this->SearchFrameworkFirst = false;
  this->SearchAppBundleFirst = false;
#endif
  this->SearchFrameworkOnly = false;
  this->SearchFrameworkLast = false;
  this->SearchAppBundleOnly = false;
  this->SearchAppBundleLast = false;

  this->InitializeSearchPathGroups();

  this->DebugMode = false;

  // Windows Registry views
  // When policy CMP0134 is not NEW, rely on previous behavior:
  if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0134) !=
      cmPolicies::NEW) {
    if (this->Makefile->GetDefinition("CMAKE_SIZEOF_VOID_P") == "8") {
      this->RegistryView = cmWindowsRegistry::View::Reg64;
    } else {
      this->RegistryView = cmWindowsRegistry::View::Reg32;
    }
  }
}

void cmFindCommon::SetError(std::string const& e)
{
  this->Status.SetError(e);
}

void cmFindCommon::DebugMessage(std::string const& msg) const
{
  if (this->Makefile) {
    this->Makefile->IssueMessage(MessageType::LOG, msg);
  }
}

bool cmFindCommon::ComputeIfDebugModeWanted()
{
  return this->Makefile->GetDebugFindPkgMode() ||
    this->Makefile->IsOn("CMAKE_FIND_DEBUG_MODE") ||
    this->Makefile->GetCMakeInstance()->GetDebugFindOutput();
}

bool cmFindCommon::ComputeIfDebugModeWanted(std::string const& var)
{
  return this->ComputeIfDebugModeWanted() ||
    this->Makefile->GetCMakeInstance()->GetDebugFindOutput(var);
}

void cmFindCommon::InitializeSearchPathGroups()
{
  std::vector<PathLabel>* labels;

  // Define the various different groups of path types

  // All search paths
  labels = &this->PathGroupLabelMap[PathGroup::All];
  labels->push_back(PathLabel::PackageRoot);
  labels->push_back(PathLabel::CMake);
  labels->push_back(PathLabel::CMakeEnvironment);
  labels->push_back(PathLabel::Hints);
  labels->push_back(PathLabel::SystemEnvironment);
  labels->push_back(PathLabel::CMakeSystem);
  labels->push_back(PathLabel::Guess);

  // Define the search group order
  this->PathGroupOrder.push_back(PathGroup::All);

  // Create the individual labeled search paths
  this->LabeledPaths.insert(
    std::make_pair(PathLabel::PackageRoot, cmSearchPath(this)));
  this->LabeledPaths.insert(
    std::make_pair(PathLabel::CMake, cmSearchPath(this)));
  this->LabeledPaths.insert(
    std::make_pair(PathLabel::CMakeEnvironment, cmSearchPath(this)));
  this->LabeledPaths.insert(
    std::make_pair(PathLabel::Hints, cmSearchPath(this)));
  this->LabeledPaths.insert(
    std::make_pair(PathLabel::SystemEnvironment, cmSearchPath(this)));
  this->LabeledPaths.insert(
    std::make_pair(PathLabel::CMakeSystem, cmSearchPath(this)));
  this->LabeledPaths.insert(
    std::make_pair(PathLabel::Guess, cmSearchPath(this)));
}

void cmFindCommon::SelectDefaultRootPathMode()
{
  // Check the policy variable for this find command type.
  std::string findRootPathVar =
    cmStrCat("CMAKE_FIND_ROOT_PATH_MODE_", this->CMakePathName);
  std::string rootPathMode =
    this->Makefile->GetSafeDefinition(findRootPathVar);
  if (rootPathMode == "NEVER") {
    this->FindRootPathMode = RootPathModeNever;
  } else if (rootPathMode == "ONLY") {
    this->FindRootPathMode = RootPathModeOnly;
  } else if (rootPathMode == "BOTH") {
    this->FindRootPathMode = RootPathModeBoth;
  }
}

void cmFindCommon::SelectDefaultMacMode()
{
  std::string ff = this->Makefile->GetSafeDefinition("CMAKE_FIND_FRAMEWORK");
  if (ff == "NEVER") {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = false;
  } else if (ff == "ONLY") {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = true;
  } else if (ff == "FIRST") {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = true;
    this->SearchFrameworkOnly = false;
  } else if (ff == "LAST") {
    this->SearchFrameworkLast = true;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = false;
  }

  std::string fab = this->Makefile->GetSafeDefinition("CMAKE_FIND_APPBUNDLE");
  if (fab == "NEVER") {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = false;
  } else if (fab == "ONLY") {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = true;
  } else if (fab == "FIRST") {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = true;
    this->SearchAppBundleOnly = false;
  } else if (fab == "LAST") {
    this->SearchAppBundleLast = true;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = false;
  }
}

void cmFindCommon::SelectDefaultSearchModes()
{
  const std::array<std::pair<bool&, std::string>, 6> search_paths = {
    { { this->NoPackageRootPath, "CMAKE_FIND_USE_PACKAGE_ROOT_PATH" },
      { this->NoCMakePath, "CMAKE_FIND_USE_CMAKE_PATH" },
      { this->NoCMakeEnvironmentPath,
        "CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH" },
      { this->NoSystemEnvironmentPath,
        "CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH" },
      { this->NoCMakeSystemPath, "CMAKE_FIND_USE_CMAKE_SYSTEM_PATH" },
      { this->NoCMakeInstallPath, "CMAKE_FIND_USE_INSTALL_PREFIX" } }
  };

  for (auto const& path : search_paths) {
    cmValue def = this->Makefile->GetDefinition(path.second);
    if (def) {
      path.first = !cmIsOn(*def);
    }
  }
}

void cmFindCommon::RerootPaths(std::vector<std::string>& paths)
{
#if 0
  for(std::string const& p : paths)
    {
    fprintf(stderr, "[%s]\n", p.c_str());
    }
#endif
  // Short-circuit if there is nothing to do.
  if (this->FindRootPathMode == RootPathModeNever) {
    return;
  }

  cmValue sysroot = this->Makefile->GetDefinition("CMAKE_SYSROOT");
  cmValue sysrootCompile =
    this->Makefile->GetDefinition("CMAKE_SYSROOT_COMPILE");
  cmValue sysrootLink = this->Makefile->GetDefinition("CMAKE_SYSROOT_LINK");
  cmValue rootPath = this->Makefile->GetDefinition("CMAKE_FIND_ROOT_PATH");
  const bool noSysroot = !cmNonempty(sysroot);
  const bool noCompileSysroot = !cmNonempty(sysrootCompile);
  const bool noLinkSysroot = !cmNonempty(sysrootLink);
  const bool noRootPath = !cmNonempty(rootPath);
  if (noSysroot && noCompileSysroot && noLinkSysroot && noRootPath) {
    return;
  }

  // Construct the list of path roots with no trailing slashes.
  std::vector<std::string> roots;
  if (rootPath) {
    cmExpandList(*rootPath, roots);
  }
  if (sysrootCompile) {
    roots.emplace_back(*sysrootCompile);
  }
  if (sysrootLink) {
    roots.emplace_back(*sysrootLink);
  }
  if (sysroot) {
    roots.emplace_back(*sysroot);
  }
  for (std::string& r : roots) {
    cmSystemTools::ConvertToUnixSlashes(r);
  }

  cmValue stagePrefix = this->Makefile->GetDefinition("CMAKE_STAGING_PREFIX");

  // Copy the original set of unrooted paths.
  std::vector<std::string> unrootedPaths = paths;
  paths.clear();

  auto isSameDirectoryOrSubDirectory = [](std::string const& l,
                                          std::string const& r) {
    return (cmSystemTools::GetRealPath(l) == cmSystemTools::GetRealPath(r)) ||
      cmSystemTools::IsSubDirectory(l, r);
  };

  for (std::string const& r : roots) {
    for (std::string const& up : unrootedPaths) {
      // Place the unrooted path under the current root if it is not
      // already inside.  Skip the unrooted path if it is relative to
      // a user home directory or is empty.
      std::string rootedDir;
      if (isSameDirectoryOrSubDirectory(up, r) ||
          (stagePrefix && isSameDirectoryOrSubDirectory(up, *stagePrefix))) {
        rootedDir = up;
      } else if (!up.empty() && up[0] != '~') {
        auto const* split = cmSystemTools::SplitPathRootComponent(up);
        if (split && *split) {
          // Start with the new root.
          rootedDir = cmStrCat(r, '/', split);
        } else {
          rootedDir = r;
        }
      }

      // Store the new path.
      paths.push_back(rootedDir);
    }
  }

  // If searching both rooted and unrooted paths add the original
  // paths again.
  if (this->FindRootPathMode == RootPathModeBoth) {
    cm::append(paths, unrootedPaths);
  }
}

void cmFindCommon::GetIgnoredPaths(std::vector<std::string>& ignore)
{
  static constexpr const char* paths[] = {
    "CMAKE_SYSTEM_IGNORE_PATH",
    "CMAKE_IGNORE_PATH",
  };

  // Construct the list of path roots with no trailing slashes.
  for (const char* pathName : paths) {
    // Get the list of paths to ignore from the variable.
    this->Makefile->GetDefExpandList(pathName, ignore);
  }

  for (std::string& i : ignore) {
    cmSystemTools::ConvertToUnixSlashes(i);
  }
}

void cmFindCommon::GetIgnoredPaths(std::set<std::string>& ignore)
{
  std::vector<std::string> ignoreVec;
  this->GetIgnoredPaths(ignoreVec);
  ignore.insert(ignoreVec.begin(), ignoreVec.end());
}

void cmFindCommon::GetIgnoredPrefixPaths(std::vector<std::string>& ignore)
{
  static constexpr const char* paths[] = {
    "CMAKE_SYSTEM_IGNORE_PREFIX_PATH",
    "CMAKE_IGNORE_PREFIX_PATH",
  };

  // Construct the list of path roots with no trailing slashes.
  for (const char* pathName : paths) {
    // Get the list of paths to ignore from the variable.
    this->Makefile->GetDefExpandList(pathName, ignore);
  }

  for (std::string& i : ignore) {
    cmSystemTools::ConvertToUnixSlashes(i);
  }
}

void cmFindCommon::GetIgnoredPrefixPaths(std::set<std::string>& ignore)
{
  std::vector<std::string> ignoreVec;
  this->GetIgnoredPrefixPaths(ignoreVec);
  ignore.insert(ignoreVec.begin(), ignoreVec.end());
}

bool cmFindCommon::CheckCommonArgument(std::string const& arg)
{
  if (arg == "NO_DEFAULT_PATH") {
    this->NoDefaultPath = true;
  } else if (arg == "NO_PACKAGE_ROOT_PATH") {
    this->NoPackageRootPath = true;
  } else if (arg == "NO_CMAKE_PATH") {
    this->NoCMakePath = true;
  } else if (arg == "NO_CMAKE_ENVIRONMENT_PATH") {
    this->NoCMakeEnvironmentPath = true;
  } else if (arg == "NO_SYSTEM_ENVIRONMENT_PATH") {
    this->NoSystemEnvironmentPath = true;
  } else if (arg == "NO_CMAKE_SYSTEM_PATH") {
    this->NoCMakeSystemPath = true;
  } else if (arg == "NO_CMAKE_INSTALL_PREFIX") {
    this->NoCMakeInstallPath = true;
  } else if (arg == "NO_CMAKE_FIND_ROOT_PATH") {
    this->FindRootPathMode = RootPathModeNever;
  } else if (arg == "ONLY_CMAKE_FIND_ROOT_PATH") {
    this->FindRootPathMode = RootPathModeOnly;
  } else if (arg == "CMAKE_FIND_ROOT_PATH_BOTH") {
    this->FindRootPathMode = RootPathModeBoth;
  } else {
    // The argument is not one of the above.
    return false;
  }

  // The argument is one of the above.
  return true;
}

void cmFindCommon::AddPathSuffix(std::string const& arg)
{
  std::string suffix = arg;

  // Strip leading and trailing slashes.
  if (suffix.empty()) {
    return;
  }
  if (suffix.front() == '/') {
    suffix = suffix.substr(1);
  }
  if (suffix.empty()) {
    return;
  }
  if (suffix.back() == '/') {
    suffix = suffix.substr(0, suffix.size() - 1);
  }
  if (suffix.empty()) {
    return;
  }

  // Store the suffix.
  this->SearchPathSuffixes.push_back(std::move(suffix));
}

static void AddTrailingSlash(std::string& s)
{
  if (!s.empty() && s.back() != '/') {
    s += '/';
  }
}
void cmFindCommon::ComputeFinalPaths(IgnorePaths ignorePaths)
{
  // Filter out ignored paths from the prefix list
  std::set<std::string> ignoredPaths;
  std::set<std::string> ignoredPrefixes;
  if (ignorePaths == IgnorePaths::Yes) {
    this->GetIgnoredPaths(ignoredPaths);
    this->GetIgnoredPrefixPaths(ignoredPrefixes);
  }

  // Combine the separate path types, filtering out ignores
  this->SearchPaths.clear();
  std::vector<PathLabel>& allLabels = this->PathGroupLabelMap[PathGroup::All];
  for (PathLabel const& l : allLabels) {
    this->LabeledPaths[l].ExtractWithout(ignoredPaths, ignoredPrefixes,
                                         this->SearchPaths);
  }

  // Expand list of paths inside all search roots.
  this->RerootPaths(this->SearchPaths);

  // Add a trailing slash to all paths to aid the search process.
  std::for_each(this->SearchPaths.begin(), this->SearchPaths.end(),
                &AddTrailingSlash);
}
