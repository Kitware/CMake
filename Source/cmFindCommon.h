/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cmPathLabel.h"
#include "cmSearchPath.h"
#include "cmWindowsRegistry.h"

class cmConfigureLog;
class cmFindCommonDebugState;
class cmExecutionStatus;
class cmMakefile;

/** \class cmFindCommon
 * \brief Base class for FIND_XXX implementations.
 *
 * cmFindCommon is a parent class for cmFindBase,
 * cmFindProgramCommand, cmFindPathCommand, cmFindLibraryCommand,
 * cmFindFileCommand, and cmFindPackageCommand.
 */
class cmFindCommon
{
public:
  cmFindCommon(cmExecutionStatus& status);
  virtual ~cmFindCommon();

  void SetError(std::string const& e);

  bool DebugModeEnabled() const;

protected:
  friend class cmSearchPath;
  friend class cmFindBaseDebugState;
  friend class cmFindCommonDebugState;

  /** Used to define groups of path labels */
  class PathGroup : public cmPathLabel
  {
  protected:
    PathGroup();

  public:
    PathGroup(std::string const& label)
      : cmPathLabel(label)
    {
    }
    static PathGroup All;
  };

  /* Individual path types */
  class PathLabel : public cmPathLabel
  {
  protected:
    PathLabel();

  public:
    PathLabel(std::string const& label)
      : cmPathLabel(label)
    {
    }
    static PathLabel PackageRoot;
    static PathLabel CMake;
    static PathLabel CMakeEnvironment;
    static PathLabel Hints;
    static PathLabel SystemEnvironment;
    static PathLabel CMakeSystem;
    static PathLabel Guess;
  };

  enum RootPathMode
  {
    RootPathModeNever,
    RootPathModeOnly,
    RootPathModeBoth
  };

  virtual bool IsFound() const = 0;
  virtual bool IsDefined() const = 0;

  /** Construct the various path groups and labels */
  void InitializeSearchPathGroups();

  /** Place a set of search paths under the search roots.  */
  void RerootPaths(std::vector<std::string>& paths,
                   std::string* debugBuffer = nullptr);

  /** Get ignored paths from CMAKE_[SYSTEM_]IGNORE_PATH variables.  */
  void GetIgnoredPaths(std::vector<std::string>& ignore);
  void GetIgnoredPaths(std::set<std::string>& ignore);

  /** Get ignored paths from CMAKE_[SYSTEM_]IGNORE_PREFIX_PATH variables.  */
  void GetIgnoredPrefixPaths(std::vector<std::string>& ignore);
  void GetIgnoredPrefixPaths(std::set<std::string>& ignore);

  /** Compute final search path list (reroot + trailing slash).  */
  enum class IgnorePaths
  {
    No,
    Yes,
  };
  void ComputeFinalPaths(IgnorePaths ignorePaths,
                         std::string* debugBuffer = nullptr);

  /** Compute the current default root path mode.  */
  void SelectDefaultRootPathMode();

  /** Compute the current default bundle/framework search policy.  */
  void SelectDefaultMacMode();

  /** Compute the current default search modes based on global variables.  */
  void SelectDefaultSearchModes();

  /** The `InitialPass` functions of the child classes should set
      this->DebugMode to the result of these.  */
  bool ComputeIfDebugModeWanted();
  bool ComputeIfDebugModeWanted(std::string const& var);

  // Path arguments prior to path manipulation routines
  std::vector<std::string> UserHintsArgs;
  std::vector<std::string> UserGuessArgs;

  std::string CMakePathName;
  RootPathMode FindRootPathMode;

  bool CheckCommonArgument(std::string const& arg);
  void AddPathSuffix(std::string const& arg);

  void DebugMessage(std::string const& msg) const;
  std::unique_ptr<cmFindCommonDebugState> DebugState;
  bool NoDefaultPath;
  bool NoPackageRootPath;
  bool NoCMakePath;
  bool NoCMakeEnvironmentPath;
  bool NoSystemEnvironmentPath;
  bool NoCMakeSystemPath;
  bool NoCMakeInstallPath;
  cmWindowsRegistry::View RegistryView = cmWindowsRegistry::View::Target;

  std::vector<std::string> SearchPathSuffixes;

  std::map<PathGroup, std::vector<PathLabel>> PathGroupLabelMap;
  std::vector<PathGroup> PathGroupOrder;
  std::map<std::string, PathLabel> PathLabelStringMap;
  std::map<PathLabel, cmSearchPath> LabeledPaths;

  std::vector<std::string> SearchPaths;
  std::set<cmSearchPath::PathWithPrefix> SearchPathsEmitted;

  bool SearchFrameworkFirst;
  bool SearchFrameworkOnly;
  bool SearchFrameworkLast;

  bool SearchAppBundleFirst;
  bool SearchAppBundleOnly;
  bool SearchAppBundleLast;

  cmMakefile* Makefile;
  cmExecutionStatus& Status;
};

class cmFindCommonDebugState
{
public:
  cmFindCommonDebugState(std::string name, cmFindCommon const* findCommand);
  virtual ~cmFindCommonDebugState() = default;

  void FoundAt(std::string const& path, std::string regexName = std::string());
  void FailedAt(std::string const& path,
                std::string regexName = std::string());

  void Write();

protected:
  virtual void FoundAtImpl(std::string const& path, std::string regexName) = 0;
  virtual void FailedAtImpl(std::string const& path,
                            std::string regexName) = 0;

  virtual void WriteDebug() const = 0;
#ifndef CMAKE_BOOTSTRAP
  virtual void WriteEvent(cmConfigureLog& log, cmMakefile const& mf) const = 0;
#endif

  cmFindCommon const* const FindCommand;
  std::string const CommandName;
  std::string const Mode;
  bool HasBeenFound() const { return this->IsFound; }

private:
  bool TrackSearchProgress() const;

  bool IsFound{ false };
};
