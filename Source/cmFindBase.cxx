/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmFindBase.h"
  
cmFindBase::cmFindBase()
{
  this->AlreadyInCache = false;
  this->AlreadyInCacheWithoutMetaInfo = false;
  this->NoDefaultPath = false;
  this->NoCMakePath = false;
  this->NoCMakeEnvironmentPath = false;
  this->NoSystemEnvironmentPath = false;
  this->NoCMakeSystemPath = false;
  this->FindRootPathMode = RootPathModeBoth;
  // default is to search frameworks first on apple
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
  this->GenericDocumentation = 
    "   FIND_XXX(<VAR> name1 path1 path2 ...)\n"
    "This is the short-hand signature for the command that "
    "is sufficient in many cases.  It is the same "
    "as FIND_XXX(<VAR> name1 PATHS path2 path2 ...)\n"
    "   FIND_XXX(\n"
    "             <VAR> \n"
    "             name | NAMES name1 [name2 ...]\n"
    "             PATHS path1 [path2 ... ENV var]\n"
    "             [PATH_SUFFIXES suffix1 [suffix2 ...]]\n"
    "             [DOC \"cache documentation string\"]\n"
    "             [NO_DEFAULT_PATH]\n"
    "             [NO_CMAKE_ENVIRONMENT_PATH]\n"
    "             [NO_CMAKE_PATH]\n"
    "             [NO_SYSTEM_ENVIRONMENT_PATH]\n"
    "             [NO_CMAKE_SYSTEM_PATH]\n"
    "             [CMAKE_FIND_ROOT_PATH_BOTH | ONLY_CMAKE_FIND_ROOT_PATH | "
                                               "NO_CMAKE_FIND_ROOT_PATH ]\n"
    "            )\n"
    ""
    "This command is used to find a SEARCH_XXX_DESC. "
    "A cache entry named by <VAR> is created to store the result "
    "of this command.  "
    "If the SEARCH_XXX is found the result is stored in the variable "
    "and the search will not be repeated unless the variable is cleared.  "
    "If nothing is found, the result will be "
    "<VAR>-NOTFOUND, and the search will be attempted again the "
    "next time FIND_XXX is invoked with the same variable.  "
    "The name of the SEARCH_XXX that "
    "is searched for is specified by the names listed "
    "after the NAMES argument.   Additional search locations "
    "can be specified after the PATHS argument.  If ENV var is "
    "found in the PATHS section the environment variable var "
    "will be read and converted from a system environment variable to "
    "a cmake style list of paths.  For example ENV PATH would be a way "
    "to list the system path variable. The argument "
    "after DOC will be used for the documentation string in "
    "the cache.  PATH_SUFFIXES can be used to give sub directories "
    "that will be appended to the search paths.\n"
    "If NO_DEFAULT_PATH is specified, then no additional paths are "
    "added to the search. "
    "If NO_DEFAULT_PATH is not specified, the search process is as follows:\n"
    "1. Search cmake specific environment variables.  This "
    "can be skipped if NO_CMAKE_ENVIRONMENT_PATH is passed.\n"
    ""
    "   <prefix>/XXX_SUBDIR for each <prefix> in CMAKE_PREFIX_PATH\n"
    "   CMAKE_FRAMEWORK_PATH\n"
    "   CMAKE_APPBUNDLE_PATH\n"
    "   CMAKE_XXX_PATH\n"
    "2. Search cmake variables with the same names as "
    "the cmake specific environment variables.  These "
    "are intended to be used on the command line with a "
    "-DVAR=value.  This can be skipped if NO_CMAKE_PATH "
    "is passed.\n"
    ""
    "   <prefix>/XXX_SUBDIR for each <prefix> in CMAKE_PREFIX_PATH\n"
    "   CMAKE_FRAMEWORK_PATH\n"
    "   CMAKE_APPBUNDLE_PATH\n"
    "   CMAKE_XXX_PATH\n"
    "3. Search the standard system environment variables. "
    "This can be skipped if NO_SYSTEM_ENVIRONMENT_PATH is an argument.\n"
    "   PATH\n"
    "   XXX_SYSTEM\n"  // replace with "", LIB, or INCLUDE
    "4. Search cmake variables defined in the Platform files "
    "for the current system.  This can be skipped if NO_CMAKE_SYSTEM_PATH "
    "is passed.\n"
    "   <prefix>/XXX_SUBDIR for each <prefix> in CMAKE_SYSTEM_PREFIX_PATH\n"
    "   CMAKE_SYSTEM_FRAMEWORK_PATH\n"
    "   CMAKE_SYSTEM_APPBUNDLE_PATH\n"
    "   CMAKE_SYSTEM_XXX_PATH\n"
    "5. Search the paths specified after PATHS or in the short-hand version "
    "of the command.\n"
    "On Darwin or systems supporting OSX Frameworks, the cmake variable"
    "    CMAKE_FIND_FRAMEWORK can be set to empty or one of the following:\n"
    "   \"FIRST\"  - Try to find frameworks before standard\n"
    "              libraries or headers. This is the default on Darwin.\n"
    "   \"LAST\"   - Try to find frameworks after standard\n"
    "              libraries or headers.\n"
    "   \"ONLY\"   - Only try to find frameworks.\n"
    "   \"NEVER\". - Never try to find frameworks.\n"
    "On Darwin or systems supporting OSX Application Bundles, the cmake "
    "variable CMAKE_FIND_APPBUNDLE can be set to empty or one of the "
    "following:\n"
    "   \"FIRST\"  - Try to find application bundles before standard\n"
    "              programs. This is the default on Darwin.\n"
    "   \"LAST\"   - Try to find application bundles after standard\n"
    "              programs.\n"
    "   \"ONLY\"   - Only try to find application bundles.\n"
    "   \"NEVER\". - Never try to find application bundles.\n"
    "The CMake variable CMAKE_FIND_ROOT_PATH specifies one or more "
    "directories to be prepended to all other search directories. "
    "This effectively \"re-roots\" the entire search under given locations. "
    "By default it is empty. It is especially useful when "
    "cross-compiling to point to the root directory of the "
    "target environment and CMake will search there too. By default at first "
    "the directories listed in CMAKE_FIND_ROOT_PATH and then the non-rooted "
    "directories will be searched. "
    "The default behavior can be adjusted by setting "
    "CMAKE_FIND_ROOT_PATH_MODE_XXX.  This behavior can be manually "
    "overridden on a per-call basis. "
    "By using CMAKE_FIND_ROOT_PATH_BOTH the search order will "
    "be as described above. If NO_CMAKE_FIND_ROOT_PATH is used "
    "then CMAKE_FIND_ROOT_PATH will not be used. If ONLY_CMAKE_FIND_ROOT_PATH "
    "is used then only the re-rooted directories will be searched.\n"
    "The reason the paths listed in the call to the command are searched "
    "last is that most users of CMake would expect things to be found "
    "first in the locations specified by their environment. Projects may "
    "override this behavior by simply calling the command twice:\n"
    "   FIND_XXX(<VAR> NAMES name PATHS paths NO_DEFAULT_PATH)\n"
    "   FIND_XXX(<VAR> NAMES name)\n"
    "Once one of these calls succeeds the result variable will be set "
    "and stored in the cache so that neither call will search again.";
}
  
bool cmFindBase::ParseArguments(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string ff = this->Makefile->GetSafeDefinition("CMAKE_FIND_FRAMEWORK");
  if(ff == "NEVER")
    {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = false;
    }
  else if (ff == "ONLY")
    {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = true;
    }
  else if (ff == "FIRST")
    {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = true;
    this->SearchFrameworkOnly = false;
    }
  else if (ff == "LAST")
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
  else if (fab == "ONLY")
    {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = true;
    }
  else if (fab == "FIRST")
    {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = true;
    this->SearchAppBundleOnly = false;
    }
  else if (fab == "LAST")
    {
    this->SearchAppBundleLast = true;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = false;
    }

  // CMake versions below 2.3 did not search all these extra
  // locations.  Preserve compatibility unless a modern argument is
  // passed.
  bool compatibility = false;
  const char* versionValue =
    this->Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  int major = 0;
  int minor = 0;
  if(versionValue && sscanf(versionValue, "%d.%d", &major, &minor) != 2)
    {
    versionValue = 0;
    }
  if(versionValue && (major < 2 || major == 2 && minor < 3))
    {
    compatibility = true;
    }

  // copy argsIn into args so it can be modified,
  // in the process extract the DOC "documentation" 
  size_t size = argsIn.size();
  std::vector<std::string> args;
  bool foundDoc = false;
  for(unsigned int j = 0; j < size; ++j)
    {
    if(foundDoc  || argsIn[j] != "DOC" )
      {
      if(argsIn[j] == "ENV")
        {
        if(j+1 < size)
          {
          j++;
          cmSystemTools::GetPath(args, argsIn[j].c_str());
          }
        }
      else
        {
        args.push_back(argsIn[j]);
        }
      }
    else
      {
      if(j+1 < size)
        {
        foundDoc = true;
        this->VariableDocumentation = argsIn[j+1];
        j++;
        if(j >= size)
          {
          break;
          }
        }
      }
    }
  this->VariableName = args[0];
  if(this->CheckForVariableInCache())
    {
    this->AlreadyInCache = true;
    return true;
    }
  this->AlreadyInCache = false; 
  
  std::string findRootPathVar = "CMAKE_FIND_ROOT_PATH_MODE_";
  findRootPathVar += this->CMakePathName;
  std::string rootPathMode = 
              this->Makefile->GetSafeDefinition(findRootPathVar.c_str());
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

  std::vector<std::string> userPaths;
  std::string doc;
  bool doingNames = true; // assume it starts with a name
  bool doingPaths = false;
  bool doingPathSuf = false;
  bool newStyle = false;
  
  for (unsigned int j = 1; j < args.size(); ++j)
    {
    if(args[j] == "NAMES")
      {
      doingNames = true;
      newStyle = true;
      doingPathSuf = false;
      doingPaths = false;
      }
    else if (args[j] == "PATHS")
      {
      doingPaths = true;
      newStyle = true;
      doingNames = false;
      doingPathSuf = false;
      }
    else if (args[j] == "PATH_SUFFIXES")
      {
      compatibility = false;
      doingPathSuf = true;
      newStyle = true;
      doingNames = false;
      doingPaths = false;
      }
    else if (args[j] == "NO_SYSTEM_PATH")
      {
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoDefaultPath = true;
      }
    else if (args[j] == "NO_DEFAULT_PATH")
      {
      compatibility = false;
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoDefaultPath = true;
      }
    else if (args[j] == "NO_CMAKE_ENVIRONMENT_PATH")
      {
      compatibility = false;
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoCMakeEnvironmentPath = true;
      }
    else if (args[j] == "NO_CMAKE_PATH")
      {
      compatibility = false;
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoCMakePath = true;
      }
    else if (args[j] == "NO_SYSTEM_ENVIRONMENT_PATH")
      {
      compatibility = false;
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoSystemEnvironmentPath = true;
      }
    else if (args[j] == "NO_CMAKE_SYSTEM_PATH")
      {
      compatibility = false;
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoCMakeSystemPath = true;
      }
    else if (args[j] == "NO_CMAKE_FIND_ROOT_PATH")
      {
      compatibility = false;
      this->FindRootPathMode = RootPathModeNoRootPath;
      }
    else if (args[j] == "ONLY_CMAKE_FIND_ROOT_PATH")
      {
      compatibility = false;
      this->FindRootPathMode = RootPathModeOnlyRootPath;
      }
    else if (args[j] == "CMAKE_FIND_ROOT_PATH_BOTH")
      {
      compatibility = false;
      this->FindRootPathMode = RootPathModeBoth;
      }
    else
      {
      if(doingNames)
        {
        this->Names.push_back(args[j]);
        }
      else if(doingPaths)
        { 
        userPaths.push_back(args[j]);
        }
      else if(doingPathSuf)
        { 
        this->SearchPathSuffixes.push_back(args[j]);
        }
      }
    }

  // Now that arguments have been parsed check the compatibility
  // setting.  If we need to be compatible with CMake 2.2 and earlier
  // do not add the CMake system paths.  It is safe to add the CMake
  // environment paths and system environment paths because that
  // existed in 2.2.  It is safe to add the CMake user variable paths
  // because the user or project has explicitly set them.
  if(compatibility)
    {
    this->NoCMakeSystemPath = true;
    }

  if(this->VariableDocumentation.size() == 0)
    {
    this->VariableDocumentation = "Where can ";
    if(this->Names.size() == 0)
      {
      this->VariableDocumentation += "the (unknown) library be found";
      }
    else if(this->Names.size() == 1)
      {
      this->VariableDocumentation += "the " 
        + this->Names[0] + " library be found";
      }
    else
      { 
      this->VariableDocumentation += "one of the " + this->Names[0];
      for (unsigned int j = 1; j < this->Names.size() - 1; ++j)
        {
        this->VariableDocumentation += ", " + this->Names[j];
        }
      this->VariableDocumentation += " or " 
        + this->Names[this->Names.size() - 1] + " libraries be found";
      }
    }

  // look for old style
  // FIND_*(VAR name path1 path2 ...)
  if(!newStyle)
    {
    this->Names.clear(); // clear out any values in Names
    this->Names.push_back(args[1]);
    for(unsigned int j = 2; j < args.size(); ++j)
      {
      userPaths.push_back(args[j]);
      }
    }
  this->ExpandPaths(userPaths);
  
  this->HandleCMakeFindRootPath();
  return true;
}

void cmFindBase::ExpandPaths(std::vector<std::string> userPaths)
{
  // if NO Default paths was not specified add the
  // standard search paths.
  if(!this->NoDefaultPath)
    {
    if(this->SearchFrameworkFirst)
      {
      this->AddFrameWorkPaths();
      }
    if(this->SearchAppBundleFirst)
      {
      this->AddAppBundlePaths();
      }
    if(!this->NoCMakeEnvironmentPath && 
       !(this->SearchFrameworkOnly || this->SearchAppBundleOnly))
      {
      // Add CMAKE_*_PATH environment variables
      this->AddEnvironmentVariables();
      }
    if(!this->NoCMakePath && 
       !(this->SearchFrameworkOnly || this->SearchAppBundleOnly))
      {
      // Add CMake varibles of the same name as the previous environment
      // varibles CMAKE_*_PATH to be used most of the time with -D
      // command line options
      this->AddCMakeVariables();
      }
    if(!this->NoSystemEnvironmentPath && 
       !(this->SearchFrameworkOnly || this->SearchAppBundleOnly))
      {
      // add System environment PATH and (LIB or INCLUDE)
      this->AddSystemEnvironmentVariables();
      }
    if(!this->NoCMakeSystemPath && 
       !(this->SearchFrameworkOnly || this->SearchAppBundleOnly))
      {
      // Add CMAKE_SYSTEM_*_PATH variables which are defined in platform files
      this->AddCMakeSystemVariables();
      }
    if(this->SearchAppBundleLast)
      {
      this->AddAppBundlePaths();
      }
    if(this->SearchFrameworkLast)
      {
      this->AddFrameWorkPaths();
      }
    }
  std::vector<std::string> paths;
  // add the paths specified in the FIND_* call 
  for(unsigned int i =0; i < userPaths.size(); ++i)
    {
    paths.push_back(userPaths[i]);
    }
  this->AddPaths(paths);
}

void cmFindBase::HandleCMakeFindRootPath()
{
  if (this->FindRootPathMode == RootPathModeNoRootPath)
    {
    return;
    }

  const char* rootPath = this->Makefile->GetDefinition("CMAKE_FIND_ROOT_PATH");
  if ((rootPath == 0) || (strlen(rootPath) == 0))
    {
    return;
    }

  std::vector<std::string> roots;
  cmSystemTools::ExpandListArgument(rootPath, roots);

  std::vector<std::string> unrootedPaths=this->SearchPaths;
  this->SearchPaths.clear();

  for (std::vector<std::string>::const_iterator rootIt = roots.begin();
       rootIt != roots.end();
       ++rootIt )
    {
    for (std::vector<std::string>::const_iterator it = unrootedPaths.begin();
       it != unrootedPaths.end();
       ++it )
      {
      std::string rootedDir=*rootIt;
      rootedDir+=*it;
      this->SearchPaths.push_back(rootedDir);
      }
    }

  if (this->FindRootPathMode == RootPathModeBoth)
    {
    this->AddPaths(unrootedPaths);
    }
}

void cmFindBase::AddEnvironmentVariables()
{ 
  std::vector<std::string> paths;

  std::vector<std::string> prefixPaths;
  cmSystemTools::GetPath(prefixPaths, "CMAKE_PREFIX_PATH");
  this->AddFindPrefix(paths, prefixPaths);

  std::string var = "CMAKE_";
  var += this->CMakePathName;
  var += "_PATH";
  cmSystemTools::GetPath(paths, var.c_str());
  if(this->SearchAppBundleLast)
    {
    cmSystemTools::GetPath(paths, "CMAKE_APPBUNDLE_PATH");
    }
  if(this->SearchFrameworkLast)
    {
    cmSystemTools::GetPath(paths, "CMAKE_FRAMEWORK_PATH");
    }
  this->AddPaths(paths);
}

void cmFindBase::AddFindPrefix(std::vector<std::string>& dest, 
                               const std::vector<std::string>& src)
{
  // default for programs
  std::string subdir = "/bin";

  if (this->CMakePathName == "INCLUDE")
    {
    subdir = "/include";
    }
  else if (this->CMakePathName == "LIBRARY")
    {
    subdir = "/lib";
    }
  else if (this->CMakePathName == "FRAMEWORK")
    {
    subdir = "";  // ? what to do for frameworks ?
    }

  for (std::vector<std::string>::const_iterator it = src.begin();
       it != src.end();
       ++it)
    {
    std::string dirWithSubdir = it->c_str();
    dirWithSubdir += subdir;
    dest.push_back(dirWithSubdir);
    if (subdir == "/bin")
      {
      dirWithSubdir = it->c_str();
      dirWithSubdir += "/sbin";
      dest.push_back(dirWithSubdir);
      }
    if(!subdir.empty())
      {
      dest.push_back(*it);
      }
    }
}

void cmFindBase::AddFrameWorkPaths()
{
  if(this->NoDefaultPath)
    {
    return;
    }
  std::vector<std::string> paths;
  // first environment variables
  if(!this->NoCMakeEnvironmentPath)
    {
    cmSystemTools::GetPath(paths, "CMAKE_FRAMEWORK_PATH");
    }
  // add cmake variables
  if(!this->NoCMakePath)
    {
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_FRAMEWORK_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, paths);
      }
    }
  // AddCMakeSystemVariables
   if(!this->NoCMakeSystemPath)
     {
     if(const char* path = 
        this->Makefile->GetDefinition("CMAKE_SYSTEM_FRAMEWORK_PATH"))
       {
       cmSystemTools::ExpandListArgument(path, paths);
       }
     }
   this->AddPaths(paths);
}

void cmFindBase::AddPaths(std::vector<std::string> & paths)
{
  // add suffixes and clean up paths
  this->ExpandRegistryAndCleanPath(paths);
  // add the paths to the search paths
  this->SearchPaths.insert(this->SearchPaths.end(),
                           paths.begin(),
                           paths.end());
}

void cmFindBase::AddAppBundlePaths()
{
  if(this->NoDefaultPath)
    {
    return;
    }
  std::vector<std::string> paths;
  // first environment variables
  if(!this->NoCMakeEnvironmentPath)
    {
    cmSystemTools::GetPath(paths, "CMAKE_APPBUNDLE_PATH");
    }
  // add cmake variables
  if(!this->NoCMakePath)
    {
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_APPBUNDLE_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, paths);
      }
    }
  // AddCMakeSystemVariables
   if(!this->NoCMakeSystemPath)
     {
     if(const char* path = 
        this->Makefile->GetDefinition("CMAKE_SYSTEM_APPBUNDLE_PATH"))
       {
       cmSystemTools::ExpandListArgument(path, paths);
       }
     }
   this->AddPaths(paths);
}

void cmFindBase::AddCMakeVariables()
{ 
  std::string var = "CMAKE_";
  var += this->CMakePathName;
  var += "_PATH";
  std::vector<std::string> paths;

  if(const char* prefixPath = 
      this->Makefile->GetDefinition("CMAKE_PREFIX_PATH"))
    {
    std::vector<std::string> prefixPaths;
    cmSystemTools::ExpandListArgument(prefixPath, prefixPaths);
    this->AddFindPrefix(paths, prefixPaths);
    }

  if(const char* path = this->Makefile->GetDefinition(var.c_str()))
    {
    cmSystemTools::ExpandListArgument(path, paths);
    } 
  if(this->SearchAppBundleLast)
    {
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_APPBUNDLE_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, paths);
      }
    }
  if(this->SearchFrameworkLast)
    {
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_FRAMEWORK_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, paths);
      }
    }
  this->AddPaths(paths);
}

void cmFindBase::AddSystemEnvironmentVariables()
{
  // Add LIB or INCLUDE
  std::vector<std::string> paths;
  if(this->EnvironmentPath.size())
    {
    cmSystemTools::GetPath(paths, this->EnvironmentPath.c_str());
    }
  // Add PATH 
  cmSystemTools::GetPath(paths);
  this->AddPaths(paths);
}

void cmFindBase::AddCMakeSystemVariables()
{  
  std::string var = "CMAKE_SYSTEM_";
  var += this->CMakePathName;
  var += "_PATH";
  std::vector<std::string> paths;
  if(const char* prefixPath =
      this->Makefile->GetDefinition("CMAKE_SYSTEM_PREFIX_PATH"))
    {
    std::vector<std::string> prefixPaths;
    cmSystemTools::ExpandListArgument(prefixPath, prefixPaths);
    this->AddFindPrefix(paths, prefixPaths);
    }
  if(const char* path = this->Makefile->GetDefinition(var.c_str()))
    {
    cmSystemTools::ExpandListArgument(path, paths);
    }  
  if(this->SearchAppBundleLast)
    {
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_SYSTEM_APPBUNDLE_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, paths);
      }
    }
  if(this->SearchFrameworkLast)
    {
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_SYSTEM_FRAMEWORK_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, paths);
      }
    }
  this->AddPaths(paths);
}

void cmFindBase::ExpandRegistryAndCleanPath(std::vector<std::string>& paths)
{
  std::vector<std::string> finalPath;
  std::vector<std::string>::iterator i;
  // glob and expand registry stuff from paths and put
  // into finalPath
  for(i = paths.begin();
      i != paths.end(); ++i)
    {
    cmSystemTools::ExpandRegistryValues(*i);
    cmSystemTools::GlobDirs(i->c_str(), finalPath);
    }
  // clear the path
  paths.clear();
  // convert all paths to unix slashes and add search path suffixes
  // if there are any
  for(i = finalPath.begin();
      i != finalPath.end(); ++i)
    {
    cmSystemTools::ConvertToUnixSlashes(*i);
    // copy each finalPath combined with SearchPathSuffixes
    // to the SearchPaths ivar
    for(std::vector<std::string>::iterator j = 
          this->SearchPathSuffixes.begin();
        j != this->SearchPathSuffixes.end(); ++j)
      {
      std::string p = *i + std::string("/") + *j;
      // add to all paths because the search path may be modified 
      // later with lib being replaced for lib64 which may exist
      paths.push_back(p);
      }
    }
  // now put the path without the path suffixes in the SearchPaths
  for(i = finalPath.begin();
      i != finalPath.end(); ++i)
    {
    // put all search paths in because it may later be replaced
    // by lib64 stuff fixes bug 4009
    paths.push_back(*i);
    }
}

void cmFindBase::PrintFindStuff()
{
  std::cerr << "SearchFrameworkLast: " << this->SearchFrameworkLast << "\n";
  std::cerr << "SearchFrameworkOnly: " << this->SearchFrameworkOnly << "\n";
  std::cerr << "SearchFrameworkFirst: " << this->SearchFrameworkFirst << "\n";
  std::cerr << "SearchAppBundleLast: " << this->SearchAppBundleLast << "\n";
  std::cerr << "SearchAppBundleOnly: " << this->SearchAppBundleOnly << "\n";
  std::cerr << "SearchAppBundleFirst: " << this->SearchAppBundleFirst << "\n";
  std::cerr << "VariableName " << this->VariableName << "\n";
  std::cerr << "VariableDocumentation " 
            << this->VariableDocumentation << "\n";
  std::cerr << "NoDefaultPath " << this->NoDefaultPath << "\n";
  std::cerr << "NoCMakeEnvironmentPath " 
            << this->NoCMakeEnvironmentPath << "\n";
  std::cerr << "NoCMakePath " << this->NoCMakePath << "\n";
  std::cerr << "NoSystemEnvironmentPath " 
            << this->NoSystemEnvironmentPath << "\n";
  std::cerr << "NoCMakeSystemPath " << this->NoCMakeSystemPath << "\n";
  std::cerr << "EnvironmentPath " << this->EnvironmentPath << "\n";
  std::cerr << "CMakePathName " << this->CMakePathName << "\n";
  std::cerr << "Names  ";
  for(unsigned int i =0; i < this->Names.size(); ++i)
    {
    std::cerr << this->Names[i] << " ";
    }
  std::cerr << "\n";
  std::cerr << "\n";
  std::cerr << "SearchPathSuffixes  ";
  for(unsigned int i =0; i < this->SearchPathSuffixes.size(); ++i)
    {
    std::cerr << this->SearchPathSuffixes[i] << "\n";
    }
  std::cerr << "\n";
  std::cerr << "SearchPaths\n";
  for(unsigned int i =0; i < this->SearchPaths.size(); ++i)
    {
    std::cerr << "[" << this->SearchPaths[i] << "]\n";
    }
}

bool cmFindBase::CheckForVariableInCache()
{
  if(const char* cacheValue =
     this->Makefile->GetDefinition(this->VariableName.c_str()))
    {
    cmCacheManager::CacheIterator it =
      this->Makefile->GetCacheManager()->
      GetCacheIterator(this->VariableName.c_str());
    bool found = !cmSystemTools::IsNOTFOUND(cacheValue);
    bool cached = !it.IsAtEnd();
    if(found)
      {
      // If the user specifies the entry on the command line without a
      // type we should add the type and docstring but keep the
      // original value.  Tell the subclass implementations to do
      // this.
      if(cached && it.GetType() == cmCacheManager::UNINITIALIZED)
        {
        this->AlreadyInCacheWithoutMetaInfo = true;
        }
      return true;
      }
    else if(cached)
      {
      const char* hs = it.GetProperty("HELPSTRING");
      this->VariableDocumentation = hs?hs:"(none)";
      }
    }
  return false;
}
