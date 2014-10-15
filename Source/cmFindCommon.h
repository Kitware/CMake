/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFindCommon_h
#define cmFindCommon_h

#include "cmCommand.h"
#include "cmSearchPath.h"

/** \class cmFindCommon
 * \brief Base class for FIND_XXX implementations.
 *
 * cmFindCommon is a parent class for cmFindBase,
 * cmFindProgramCommand, cmFindPathCommand, cmFindLibraryCommand,
 * cmFindFileCommand, and cmFindPackageCommand.
 */
class cmFindCommon : public cmCommand
{
public:
  cmFindCommon();
  ~cmFindCommon();
  cmTypeMacro(cmFindCommon, cmCommand);

protected:
  friend class cmSearchPath;

  enum RootPathMode { RootPathModeNever,
                      RootPathModeOnly,
                      RootPathModeBoth };

  /** Place a set of search paths under the search roots.  */
  void RerootPaths(std::vector<std::string>& paths);

  /** Get ignored paths from CMAKE_[SYSTEM_]IGNORE_path variables.  */
  void GetIgnoredPaths(std::vector<std::string>& ignore);
  void GetIgnoredPaths(std::set<std::string>& ignore);

  /** Remove paths in the ignore set from the supplied vector.  */
  void FilterPaths(const std::vector<std::string>& inPaths,
                   const std::set<std::string>& ignore,
                   std::vector<std::string>& outPaths);

  /** Compute final search path list (reroot + trailing slash).  */
  void ComputeFinalPaths();

  /** Compute the current default root path mode.  */
  void SelectDefaultRootPathMode();

  /** Compute the current default bundle/framework search policy.  */
  void SelectDefaultMacMode();

  // Path arguments prior to path manipulation routines
  std::vector<std::string> UserHintsArgs;
  std::vector<std::string> UserGuessArgs;

  std::string CMakePathName;
  RootPathMode FindRootPathMode;

  bool CheckCommonArgument(std::string const& arg);
  void AddPathSuffix(std::string const& arg);
  void SetMakefile(cmMakefile* makefile);

  bool NoDefaultPath;
  bool NoCMakePath;
  bool NoCMakeEnvironmentPath;
  bool NoSystemEnvironmentPath;
  bool NoCMakeSystemPath;

  std::vector<std::string> SearchPathSuffixes;
  cmSearchPath CMakeVariablePaths;
  cmSearchPath CMakeEnvironmentPaths;
  cmSearchPath UserHintsPaths;
  cmSearchPath SystemEnvironmentPaths;
  cmSearchPath UserRegistryPaths;
  cmSearchPath BuildPaths;
  cmSearchPath CMakeSystemVariablePaths;
  cmSearchPath SystemRegistryPaths;
  cmSearchPath UserGuessPaths;

  std::vector<std::string> SearchPaths;
  std::set<std::string> SearchPathsEmitted;

  bool SearchFrameworkFirst;
  bool SearchFrameworkOnly;
  bool SearchFrameworkLast;

  bool SearchAppBundleFirst;
  bool SearchAppBundleOnly;
  bool SearchAppBundleLast;
};

#endif
