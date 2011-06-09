/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindBase.h"
  
cmFindBase::cmFindBase()
{
  this->AlreadyInCache = false;
  this->AlreadyInCacheWithoutMetaInfo = false;
}

//----------------------------------------------------------------------------
void cmFindBase::GenerateDocumentation()
{
  this->cmFindCommon::GenerateDocumentation();
  cmSystemTools::ReplaceString(this->GenericDocumentationPathsOrder,
                               "FIND_ARGS_XXX", "<VAR> NAMES name");
  this->GenericDocumentation =
    "   FIND_XXX(<VAR> name1 [path1 path2 ...])\n"
    "This is the short-hand signature for the command that "
    "is sufficient in many cases.  It is the same "
    "as FIND_XXX(<VAR> name1 [PATHS path1 path2 ...])\n"
    "   FIND_XXX(\n"
    "             <VAR>\n"
    "             name | NAMES name1 [name2 ...]\n"
    "             [HINTS path1 [path2 ... ENV var]]\n"
    "             [PATHS path1 [path2 ... ENV var]]\n"
    "             [PATH_SUFFIXES suffix1 [suffix2 ...]]\n"
    "             [DOC \"cache documentation string\"]\n"
    "             [NO_DEFAULT_PATH]\n"
    "             [NO_CMAKE_ENVIRONMENT_PATH]\n"
    "             [NO_CMAKE_PATH]\n"
    "             [NO_SYSTEM_ENVIRONMENT_PATH]\n"
    "             [NO_CMAKE_SYSTEM_PATH]\n"
    "             [CMAKE_FIND_ROOT_PATH_BOTH |\n"
    "              ONLY_CMAKE_FIND_ROOT_PATH |\n"
    "              NO_CMAKE_FIND_ROOT_PATH]\n"
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
    "found in the HINTS or PATHS section the environment variable var "
    "will be read and converted from a system environment variable to "
    "a cmake style list of paths.  For example ENV PATH would be a way "
    "to list the system path variable. The argument "
    "after DOC will be used for the documentation string in "
    "the cache.  "
    "PATH_SUFFIXES specifies additional subdirectories to check below "
    "each search path."
    "\n"
    "If NO_DEFAULT_PATH is specified, then no additional paths are "
    "added to the search. "
    "If NO_DEFAULT_PATH is not specified, the search process is as follows:\n"
    "1. Search paths specified in cmake-specific cache variables.  "
    "These are intended to be used on the command line with a -DVAR=value.  "
    "This can be skipped if NO_CMAKE_PATH is passed.\n"
    "XXX_EXTRA_PREFIX_ENTRY"
    "   <prefix>/XXX_SUBDIR for each <prefix> in CMAKE_PREFIX_PATH\n"
    "   CMAKE_XXX_PATH\n"
    "   CMAKE_XXX_MAC_PATH\n"
    "2. Search paths specified in cmake-specific environment variables.  "
    "These are intended to be set in the user's shell configuration.  "
    "This can be skipped if NO_CMAKE_ENVIRONMENT_PATH is passed.\n"
    "XXX_EXTRA_PREFIX_ENTRY"
    "   <prefix>/XXX_SUBDIR for each <prefix> in CMAKE_PREFIX_PATH\n"
    "   CMAKE_XXX_PATH\n"
    "   CMAKE_XXX_MAC_PATH\n"
    "3. Search the paths specified by the HINTS option.  "
    "These should be paths computed by system introspection, such as a "
    "hint provided by the location of another item already found.  "
    "Hard-coded guesses should be specified with the PATHS option.\n"
    "4. Search the standard system environment variables. "
    "This can be skipped if NO_SYSTEM_ENVIRONMENT_PATH is an argument.\n"
    "   PATH\n"
    "   XXX_SYSTEM\n"  // replace with "", LIB, or INCLUDE
    "5. Search cmake variables defined in the Platform files "
    "for the current system.  This can be skipped if NO_CMAKE_SYSTEM_PATH "
    "is passed.\n"
    "XXX_EXTRA_PREFIX_ENTRY"
    "   <prefix>/XXX_SUBDIR for each <prefix> in CMAKE_SYSTEM_PREFIX_PATH\n"
    "   CMAKE_SYSTEM_XXX_PATH\n"
    "   CMAKE_SYSTEM_XXX_MAC_PATH\n"
    "6. Search the paths specified by the PATHS option "
    "or in the short-hand version of the command.  "
    "These are typically hard-coded guesses.\n"
    ;
  this->GenericDocumentation += this->GenericDocumentationMacPolicy;
  this->GenericDocumentation += this->GenericDocumentationRootPath;
  this->GenericDocumentation += this->GenericDocumentationPathsOrder;
}

//----------------------------------------------------------------------------
const char* cmFindBase::GetFullDocumentation()
{
  if(this->GenericDocumentation.empty())
    {
    this->GenerateDocumentation();
    }
  return this->GenericDocumentation.c_str();
}

//----------------------------------------------------------------------------
bool cmFindBase::ParseArguments(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // CMake versions below 2.3 did not search all these extra
  // locations.  Preserve compatibility unless a modern argument is
  // passed.
  bool compatibility = this->Makefile->NeedBackwardsCompatibility(2,3);

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
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  this->VariableName = args[0];
  if(this->CheckForVariableInCache())
    {
    this->AlreadyInCache = true;
    return true;
    }
  this->AlreadyInCache = false; 

  // Find the current root path mode.
  this->SelectDefaultRootPathMode();

  // Find the current bundle/framework search policy.
  this->SelectDefaultMacMode();

  bool newStyle = false;
  enum Doing { DoingNone, DoingNames, DoingPaths, DoingPathSuffixes,
               DoingHints };
  Doing doing = DoingNames; // assume it starts with a name
  for (unsigned int j = 1; j < args.size(); ++j)
    {
    if(args[j] == "NAMES")
      {
      doing = DoingNames;
      newStyle = true;
      }
    else if (args[j] == "PATHS")
      {
      doing = DoingPaths;
      newStyle = true;
      }
    else if (args[j] == "HINTS")
      {
      doing = DoingHints;
      newStyle = true;
      }
    else if (args[j] == "PATH_SUFFIXES")
      {
      doing = DoingPathSuffixes;
      compatibility = false;
      newStyle = true;
      }
    else if (args[j] == "NO_SYSTEM_PATH")
      {
      doing = DoingNone;
      this->NoDefaultPath = true;
      }
    else if (this->CheckCommonArgument(args[j]))
      {
      doing = DoingNone;
      compatibility = false;
      // Some common arguments were accidentally supported by CMake
      // 2.4 and 2.6.0 in the short-hand form of the command, so we
      // must support it even though it is not documented.
      }
    else if(doing == DoingNames)
      {
      this->Names.push_back(args[j]);
      }
    else if(doing == DoingPaths)
      {
      this->AddUserPath(args[j], this->UserPaths);
      }
    else if(doing == DoingHints)
      {
      this->AddUserPath(args[j], this->UserHints);
      }
    else if(doing == DoingPathSuffixes)
      {
      this->AddPathSuffix(args[j]);
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
    // All the short-hand arguments have been recorded as names.
    std::vector<std::string> shortArgs = this->Names;
    this->Names.clear(); // clear out any values in Names
    this->Names.push_back(shortArgs[0]);
    for(unsigned int j = 1; j < shortArgs.size(); ++j)
      {
      this->AddUserPath(shortArgs[j], this->UserPaths);
      }
    }
  this->ExpandPaths();

  // Filter out ignored paths from the prefix list
  std::set<std::string> ignored;
  this->GetIgnoredPaths(ignored);
  this->FilterPaths(this->SearchPaths, ignored);

  // Handle search root stuff.
  this->RerootPaths(this->SearchPaths);

  // Add a trailing slash to all prefixes to aid the search process.
  this->AddTrailingSlashes(this->SearchPaths);

  return true;
}

void cmFindBase::ExpandPaths()
{
  this->AddCMakeVariablePath();
  this->AddCMakeEnvironmentPath();
  this->AddUserHintsPath();
  this->AddSystemEnvironmentPath();
  this->AddCMakeSystemVariablePath();
  this->AddUserGuessPath();

  // Add suffixes and clean up paths.
  this->AddPathSuffixes();
}

//----------------------------------------------------------------------------
void cmFindBase::AddPrefixPaths(std::vector<std::string> const& in_paths,
                                PathType pathType)
{
  // default for programs
  std::string subdir = "bin";

  if (this->CMakePathName == "INCLUDE")
    {
    subdir = "include";
    }
  else if (this->CMakePathName == "LIBRARY")
    {
    subdir = "lib";
    }
  else if (this->CMakePathName == "FRAMEWORK")
    {
    subdir = "";  // ? what to do for frameworks ?
    }

  for(std::vector<std::string>::const_iterator it = in_paths.begin();
      it != in_paths.end(); ++it)
    {
    std::string dir = it->c_str();
    if(!subdir.empty() && !dir.empty() && dir[dir.size()-1] != '/')
      {
      dir += "/";
      }
    if(subdir == "lib")
      {
      const char* arch =
        this->Makefile->GetDefinition("CMAKE_LIBRARY_ARCHITECTURE");
      if(arch && *arch)
        {
        this->AddPathInternal(dir+"lib/"+arch, pathType);
        }
      }
    std::string add = dir + subdir;
    if(add != "/")
      {
      this->AddPathInternal(add, pathType);
      }
    if (subdir == "bin")
      {
      this->AddPathInternal(dir+"sbin", pathType);
      }
    if(!subdir.empty() && *it != "/")
      {
      this->AddPathInternal(*it, pathType);
      }
    }
}

//----------------------------------------------------------------------------
void cmFindBase::AddCMakePrefixPath(const char* variable)
{
  // Get a path from a CMake variable.
  if(const char* varPath = this->Makefile->GetDefinition(variable))
    {
    std::vector<std::string> tmp;
    cmSystemTools::ExpandListArgument(varPath, tmp);
    this->AddPrefixPaths(tmp, CMakePath);
    }
}

//----------------------------------------------------------------------------
void cmFindBase::AddEnvPrefixPath(const char* variable)
{
  // Get a path from the environment.
  std::vector<std::string> tmp;
  cmSystemTools::GetPath(tmp, variable);
  this->AddPrefixPaths(tmp, EnvPath);
}

//----------------------------------------------------------------------------
void cmFindBase::AddCMakeEnvironmentPath()
{
  if(!this->NoCMakeEnvironmentPath && !this->NoDefaultPath)
    {
    // Add CMAKE_*_PATH environment variables
    std::string var = "CMAKE_";
    var += this->CMakePathName;
    var += "_PATH";
    this->AddEnvPrefixPath("CMAKE_PREFIX_PATH");
    this->AddEnvPath(var.c_str());

    if(this->CMakePathName == "PROGRAM")
      {
      this->AddEnvPath("CMAKE_APPBUNDLE_PATH");
      }
    else
      {
      this->AddEnvPath("CMAKE_FRAMEWORK_PATH");
      }
    }
}

//----------------------------------------------------------------------------
void cmFindBase::AddCMakeVariablePath()
{
  if(!this->NoCMakePath && !this->NoDefaultPath)
    {
    // Add CMake varibles of the same name as the previous environment
    // varibles CMAKE_*_PATH to be used most of the time with -D
    // command line options
    std::string var = "CMAKE_";
    var += this->CMakePathName;
    var += "_PATH";
    this->AddCMakePrefixPath("CMAKE_PREFIX_PATH");
    this->AddCMakePath(var.c_str());

    if(this->CMakePathName == "PROGRAM")
      {
      this->AddCMakePath("CMAKE_APPBUNDLE_PATH");
      }
    else
      {
      this->AddCMakePath("CMAKE_FRAMEWORK_PATH");
      }
    }
}

//----------------------------------------------------------------------------
void cmFindBase::AddSystemEnvironmentPath()
{
  if(!this->NoSystemEnvironmentPath && !this->NoDefaultPath)
    {
    // Add LIB or INCLUDE
    if(!this->EnvironmentPath.empty())
      {
      this->AddEnvPath(this->EnvironmentPath.c_str());
      }
    // Add PATH
    this->AddEnvPath(0);
    }
}

//----------------------------------------------------------------------------
void cmFindBase::AddCMakeSystemVariablePath()
{
  if(!this->NoCMakeSystemPath && !this->NoDefaultPath)
    {
    std::string var = "CMAKE_SYSTEM_";
    var += this->CMakePathName;
    var += "_PATH";
    this->AddCMakePrefixPath("CMAKE_SYSTEM_PREFIX_PATH");
    this->AddCMakePath(var.c_str());

    if(this->CMakePathName == "PROGRAM")
      {
      this->AddCMakePath("CMAKE_SYSTEM_APPBUNDLE_PATH");
      }
    else
      {
      this->AddCMakePath("CMAKE_SYSTEM_FRAMEWORK_PATH");
      }
    }
}

//----------------------------------------------------------------------------
void cmFindBase::AddUserHintsPath()
{
  this->AddPathsInternal(this->UserHints, CMakePath);
}

//----------------------------------------------------------------------------
void cmFindBase::AddUserGuessPath()
{
  this->AddPathsInternal(this->UserPaths, CMakePath);
}

//----------------------------------------------------------------------------
void cmFindBase::AddPathSuffixes()
{
  std::vector<std::string>& paths = this->SearchPaths;
  std::vector<std::string> finalPath = paths;
  std::vector<std::string>::iterator i;
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
      // if *i is only / then do not add a //
      // this will get incorrectly considered a network
      // path on windows and cause huge delays.
      std::string p = *i;
      if(p.size() && p[p.size()-1] != '/')
        {
        p += std::string("/");
        }
      p +=  *j;
      // add to all paths because the search path may be modified 
      // later with lib being replaced for lib64 which may exist
      paths.push_back(p);
      }
    // now put the path without the path suffixes in the SearchPaths
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
