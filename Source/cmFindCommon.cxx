/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindCommon.h"
#include <functional>
#include <algorithm>

//----------------------------------------------------------------------------
cmFindCommon::cmFindCommon()
{
  this->FindRootPathMode = RootPathModeBoth;
  this->NoDefaultPath = false;
  this->NoCMakePath = false;
  this->NoCMakeEnvironmentPath = false;
  this->NoSystemEnvironmentPath = false;
  this->NoCMakeSystemPath = false;

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
}

//----------------------------------------------------------------------------
cmFindCommon::~cmFindCommon()
{
}

//----------------------------------------------------------------------------
std::string cmFindCommon::MakeFullPath(const std::string& path,
                                       PathType pathType)
{
  // Select the base path with which to interpret relative paths.
  if(pathType == CMakePath)
    {
    return cmSystemTools::CollapseFullPath(
      path, this->Makefile->GetCurrentDirectory());
    }
  else
    {
    return cmSystemTools::CollapseFullPath(path, 0);
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::SelectDefaultRootPathMode()
{
  // Use both by default.
  this->FindRootPathMode = RootPathModeBoth;

  // Check the policy variable for this find command type.
  std::string findRootPathVar = "CMAKE_FIND_ROOT_PATH_MODE_";
  findRootPathVar += this->CMakePathName;
  std::string rootPathMode =
    this->Makefile->GetSafeDefinition(findRootPathVar);
  if (rootPathMode=="NEVER")
    {
    this->FindRootPathMode = RootPathModeNoRootPath;
    }
  else if (rootPathMode=="ONLY")
    {
    this->FindRootPathMode = RootPathModeOnlyRootPath;
    }
  else if (rootPathMode=="BOTH")
    {
    this->FindRootPathMode = RootPathModeBoth;
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::SelectDefaultMacMode()
{
  std::string ff = this->Makefile->GetSafeDefinition("CMAKE_FIND_FRAMEWORK");
  if(ff == "NEVER")
    {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = false;
    }
  else if(ff == "ONLY")
    {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = true;
    }
  else if(ff == "FIRST")
    {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = true;
    this->SearchFrameworkOnly = false;
    }
  else if(ff == "LAST")
    {
    this->SearchFrameworkLast = true;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = false;
    }

  std::string fab = this->Makefile->GetSafeDefinition("CMAKE_FIND_APPBUNDLE");
  if(fab == "NEVER")
    {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = false;
    }
  else if(fab == "ONLY")
    {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = true;
    }
  else if(fab == "FIRST")
    {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = true;
    this->SearchAppBundleOnly = false;
    }
  else if(fab == "LAST")
    {
    this->SearchAppBundleLast = true;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = false;
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::RerootPaths(std::vector<std::string>& paths)
{
#if 0
  for(std::vector<std::string>::const_iterator i = paths.begin();
      i != paths.end(); ++i)
    {
    fprintf(stderr, "[%s]\n", i->c_str());
    }
#endif

  // Short-circuit if there is nothing to do.
  if(this->FindRootPathMode == RootPathModeNoRootPath)
    {
    return;
    }
  const char* sysroot =
    this->Makefile->GetDefinition("CMAKE_SYSROOT");
  const char* rootPath =
    this->Makefile->GetDefinition("CMAKE_FIND_ROOT_PATH");
  const bool noSysroot = !sysroot || !*sysroot;
  const bool noRootPath = !rootPath || !*rootPath;
  if(noSysroot && noRootPath)
    {
    return;
    }

  // Construct the list of path roots with no trailing slashes.
  std::vector<std::string> roots;
  if (rootPath)
    {
    cmSystemTools::ExpandListArgument(rootPath, roots);
    }
  if (sysroot)
    {
    roots.push_back(sysroot);
    }
  for(std::vector<std::string>::iterator ri = roots.begin();
      ri != roots.end(); ++ri)
    {
    cmSystemTools::ConvertToUnixSlashes(*ri);
    }

  const char* stagePrefix =
      this->Makefile->GetDefinition("CMAKE_STAGING_PREFIX");

  // Copy the original set of unrooted paths.
  std::vector<std::string> unrootedPaths = paths;
  paths.clear();

  for(std::vector<std::string>::const_iterator ri = roots.begin();
      ri != roots.end(); ++ri)
    {
    for(std::vector<std::string>::const_iterator ui = unrootedPaths.begin();
        ui != unrootedPaths.end(); ++ui)
      {
      // Place the unrooted path under the current root if it is not
      // already inside.  Skip the unrooted path if it is relative to
      // a user home directory or is empty.
      std::string rootedDir;
      if(cmSystemTools::IsSubDirectory(*ui, *ri)
          || (stagePrefix
            && cmSystemTools::IsSubDirectory(*ui, stagePrefix)))
        {
        rootedDir = *ui;
        }
      else if(!ui->empty() && (*ui)[0] != '~')
        {
        // Start with the new root.
        rootedDir = *ri;
        rootedDir += "/";

        // Append the original path with its old root removed.
        rootedDir += cmSystemTools::SplitPathRootComponent(*ui);
        }

      // Store the new path.
      paths.push_back(rootedDir);
      }
    }

  // If searching both rooted and unrooted paths add the original
  // paths again.
  if(this->FindRootPathMode == RootPathModeBoth)
    {
    paths.insert(paths.end(), unrootedPaths.begin(), unrootedPaths.end());
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::FilterPaths(const std::vector<std::string>& inPaths,
                               const std::set<std::string>& ignore,
                               std::vector<std::string>& outPaths)
{
  for(std::vector<std::string>::const_iterator i = inPaths.begin();
      i != inPaths.end(); ++i)
    {
    if(ignore.count(*i) == 0)
      {
      outPaths.push_back(*i);
      }
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::GetIgnoredPaths(std::vector<std::string>& ignore)
{
  // null-terminated list of paths.
  static const char *paths[] =
    { "CMAKE_SYSTEM_IGNORE_PATH", "CMAKE_IGNORE_PATH", 0 };

  // Construct the list of path roots with no trailing slashes.
  for(const char **pathName = paths; *pathName; ++pathName)
    {
    // Get the list of paths to ignore from the variable.
    const char* ignorePath = this->Makefile->GetDefinition(*pathName);
    if((ignorePath == 0) || (strlen(ignorePath) == 0))
      {
      continue;
      }

    cmSystemTools::ExpandListArgument(ignorePath, ignore);
    }

  for(std::vector<std::string>::iterator i = ignore.begin();
      i != ignore.end(); ++i)
    {
    cmSystemTools::ConvertToUnixSlashes(*i);
    }
}


//----------------------------------------------------------------------------
void cmFindCommon::GetIgnoredPaths(std::set<std::string>& ignore)
{
  std::vector<std::string> ignoreVec;
  GetIgnoredPaths(ignoreVec);
  ignore.insert(ignoreVec.begin(), ignoreVec.end());
}



//----------------------------------------------------------------------------
bool cmFindCommon::CheckCommonArgument(std::string const& arg)
{
  if(arg == "NO_DEFAULT_PATH")
    {
    this->NoDefaultPath = true;
    }
  else if(arg == "NO_CMAKE_ENVIRONMENT_PATH")
    {
    this->NoCMakeEnvironmentPath = true;
    }
  else if(arg == "NO_CMAKE_PATH")
    {
    this->NoCMakePath = true;
    }
  else if(arg == "NO_SYSTEM_ENVIRONMENT_PATH")
    {
    this->NoSystemEnvironmentPath = true;
    }
  else if(arg == "NO_CMAKE_SYSTEM_PATH")
    {
    this->NoCMakeSystemPath = true;
    }
  else if(arg == "NO_CMAKE_FIND_ROOT_PATH")
    {
    this->FindRootPathMode = RootPathModeNoRootPath;
    }
  else if(arg == "ONLY_CMAKE_FIND_ROOT_PATH")
    {
    this->FindRootPathMode = RootPathModeOnlyRootPath;
    }
  else if(arg == "CMAKE_FIND_ROOT_PATH_BOTH")
    {
    this->FindRootPathMode = RootPathModeBoth;
    }
  else
    {
    // The argument is not one of the above.
    return false;
    }

  // The argument is one of the above.
  return true;
}

//----------------------------------------------------------------------------
void cmFindCommon::AddPathSuffix(std::string const& arg)
{
  std::string suffix = arg;

  // Strip leading and trailing slashes.
  if(suffix.empty())
    {
    return;
    }
  if(suffix[0] == '/')
    {
    suffix = suffix.substr(1, suffix.npos);
    }
  if(suffix.empty())
    {
    return;
    }
  if(suffix[suffix.size()-1] == '/')
    {
    suffix = suffix.substr(0, suffix.size()-1);
    }
  if(suffix.empty())
    {
    return;
    }

  // Store the suffix.
  this->SearchPathSuffixes.push_back(suffix);
}

//----------------------------------------------------------------------------
void cmFindCommon::AddUserPath(std::string const& inPath,
                               std::vector<std::string>& outPaths)
{
  // We should view the registry as the target application would view
  // it.
  cmSystemTools::KeyWOW64 view = cmSystemTools::KeyWOW64_32;
  cmSystemTools::KeyWOW64 other_view = cmSystemTools::KeyWOW64_64;
  if(this->Makefile->PlatformIs64Bit())
    {
    view = cmSystemTools::KeyWOW64_64;
    other_view = cmSystemTools::KeyWOW64_32;
    }

  // Expand using the view of the target application.
  std::string expanded = inPath;
  cmSystemTools::ExpandRegistryValues(expanded, view);
  cmSystemTools::GlobDirs(expanded, outPaths);

  // Executables can be either 32-bit or 64-bit, so expand using the
  // alternative view.
  if(expanded != inPath && this->CMakePathName == "PROGRAM")
    {
    expanded = inPath;
    cmSystemTools::ExpandRegistryValues(expanded, other_view);
    cmSystemTools::GlobDirs(expanded, outPaths);
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::AddCMakePath(const std::string& variable,
                                std::vector<std::string>& outPaths)
{
  // Get a path from a CMake variable.
  if(const char* varPath = this->Makefile->GetDefinition(variable))
    {
    std::vector<std::string> tmp;
    cmSystemTools::ExpandListArgument(varPath, tmp);

    // Relative paths are interpreted with respect to the current
    // source directory.
    this->AddPathsInternal(tmp, CMakePath, outPaths);
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::AddEnvPath(const char* variable,
                              std::vector<std::string>& outPaths)
{
  // Get a path from the environment.
  std::vector<std::string> tmp;
  cmSystemTools::GetPath(tmp, variable);
  // Relative paths are interpreted with respect to the current
  // working directory.
  this->AddPathsInternal(tmp, EnvPath, outPaths);
}

//----------------------------------------------------------------------------
void cmFindCommon::AddPathsInternal(std::vector<std::string> const& inPaths,
                                    PathType pathType,
                                    std::vector<std::string>& outPaths)
{
  for(std::vector<std::string>::const_iterator i = inPaths.begin();
      i != inPaths.end(); ++i)
    {
    this->AddPathInternal(this->MakeFullPath(*i, pathType), outPaths);
    }
}

//----------------------------------------------------------------------------
void cmFindCommon::AddPathInternal(std::string const& inPath,
                                   std::vector<std::string>& outPaths)
{
  if(inPath.empty())
    {
    return;
    }

  // Insert the path if has not already been emitted.
  if(this->SearchPathsEmitted.insert(inPath).second)
    {
    outPaths.push_back(inPath);
    }
}

//----------------------------------------------------------------------------
void AddTrailingSlash(std::string& s)
{
  if(!s.empty() && *s.rbegin() != '/')
    {
    s += '/';
    }
}
void cmFindCommon::ComputeFinalPaths()
{
  // Filter out ignored paths from the prefix list
  std::set<std::string> ignored;
  this->GetIgnoredPaths(ignored);

  // Combine the seperate path types, filtering out ignores
  this->SearchPaths.clear();
  this->FilterPaths(this->CMakeVariablePaths, ignored, this->SearchPaths);
  this->FilterPaths(this->CMakeEnvironmentPaths, ignored, this->SearchPaths);
  this->FilterPaths(this->UserHintsPaths, ignored, this->SearchPaths);
  this->FilterPaths(this->SystemEnvironmentPaths, ignored, this->SearchPaths);
  this->FilterPaths(this->UserRegistryPaths, ignored, this->SearchPaths);
  this->FilterPaths(this->BuildPaths, ignored, this->SearchPaths);
  this->FilterPaths(this->CMakeSystemVariablePaths, ignored,
                    this->SearchPaths);
  this->FilterPaths(this->SystemRegistryPaths, ignored, this->SearchPaths);
  this->FilterPaths(this->UserGuessPaths, ignored, this->SearchPaths);

  // Expand list of paths inside all search roots.
  this->RerootPaths(this->SearchPaths);

  // Add a trailing slash to all paths to aid the search process.
  std::for_each(this->SearchPaths.begin(), this->SearchPaths.end(),
                &AddTrailingSlash);
}

//----------------------------------------------------------------------------
void cmFindCommon::SetMakefile(cmMakefile* makefile)
{
  cmCommand::SetMakefile(makefile);

  // If we are building for Apple (OSX or also iphone), make sure
  // that frameworks and bundles are searched first.
  if(this->Makefile->IsOn("APPLE"))
    {
    this->SearchFrameworkFirst = true;
    this->SearchAppBundleFirst = true;
    }
}
