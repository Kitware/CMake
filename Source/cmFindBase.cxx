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
  this->NoDefaultPath = false;
  this->NoCMakePath = false;
  this->NoCMakeEnvironmentPath = false;
  this->NoSystemEnvironmentPath = false;
  this->NoCMakeSystemPath = false;
  // default is to search frameworks first on apple
#if defined(__APPLE__)
  this->SearchFrameworkFirst = true;
#else
  this->SearchFrameworkFirst = false;
#endif
  this->SearchFrameworkOnly = false;
  this->SearchFrameworkLast = false;
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
    "            )\n"
    ""
    "This command is used to find a SEARCH_XXX_DESC. "
    "A cache entry named by <VAR> is created to store the result "
    "of this command.  If nothing is found, the result will be "
    "<VAR>-NOTFOUND.  The name of the SEARCH_XXX that "
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
    "   CMAKE_FRAMEWORK_PATH\n"
    "   CMAKE_XXX_PATH\n"
    "2. Search cmake variables with the same names as "
    "the cmake specific environment variables.  These "
    "are intended to be used on the command line with a "
    "-DVAR=value.  This can be skipped if NO_CMAKE_PATH "
    "is passed.\n"
    ""
    "   CMAKE_FRAMEWORK_PATH\n"
    "   CMAKE_XXX_PATH\n"
    "3. Search the standard system environment variables. "
    "This can be skipped if NO_SYSTEM_ENVIRONMENT_PATH is an argument.\n"
    "   PATH\n"
    "   XXX_SYSTEM\n"  // replace with "", LIB, or INCLUDE
    "4. Search cmake variables defined in the Platform files "
    "for the current system.  This can be skipped if NO_CMAKE_SYSTEM_PATH "
    "is passed.\n"
    "   CMAKE_SYSTEM_FRAMEWORK_PATH\n"
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
    "   \"NEVER\". - Never try to find frameworks.\n";
}
  
bool cmFindBase::ParseArguments(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
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
      doingPathSuf = true;
      newStyle = true;
      doingNames = false;
      doingPaths = false;
      }
    else if (args[j] == "NO_DEFAULT_PATH" || args[j] == "NO_SYSTEM_PATH")
      {
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoDefaultPath = true;
      }
    else if (args[j] == "NO_CMAKE_ENVIRONMENT_PATH")
      {
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoCMakeEnvironmentPath = true;
      }
    else if (args[j] == "NO_CMAKE_PATH")
      {
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoCMakePath = true;
      }
    else if (args[j] == "NO_SYSTEM_ENVIRONMENT_PATH")
      {
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoSystemEnvironmentPath = true;
      }
    else if (args[j] == "NO_CMAKE_SYSTEM_PATH")
      {
      doingPaths = false;
      doingPathSuf = false;
      doingNames = false;
      this->NoCMakeSystemPath = true;
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
  if(this->VariableDocumentation.size() == 0)
    {
    this->VariableDocumentation = "Whare can ";
    if(this->Names.size() == 0)
      {
      this->VariableDocumentation += "the (unknown) library be found";
      }
    else if(this->Names.size() == 1)
      {
      this->VariableDocumentation += "the " + this->Names[0] + " library be found";
      }
    else
      { 
      this->VariableDocumentation += "one of the " + this->Names[0];
      for (unsigned int j = 1; j < this->Names.size() - 1; ++j)
        {
        this->VariableDocumentation += ", " + this->Names[j];
        }
      this->VariableDocumentation += " or " + this->Names[this->Names.size() - 1] + " libraries be found";
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
  return true;
}

void cmFindBase::ExpandPaths(std::vector<std::string> userPaths)
{
  // if NO Default paths was not specified add the
  // standard search paths.
  if(!this->NoDefaultPath)
    {
    if(!this->NoCMakeEnvironmentPath)
      {
      // Add CMAKE_*_PATH environment variables
      this->AddEnvironmentVairables();
      }
    if(!this->NoCMakePath)
      {
      // Add CMake varibles of the same name as the previous environment
      // varibles CMAKE_*_PATH to be used most of the time with -D
      // command line options
      this->AddCMakeVairables();
      }
    if(!this->NoSystemEnvironmentPath)
      {
      // add System environment PATH and (LIB or INCLUDE)
      this->AddSystemEnvironmentVairables();
      }
    if(!this->NoCMakeSystemPath)
      {
      // Add CMAKE_SYSTEM_*_PATH variables which are defined in platform files
      this->AddCMakeSystemVariables();
      }
    }
  // add the paths specified in the FIND_* call 
  for(unsigned int i =0; i < userPaths.size(); ++i)
    {
    this->SearchPaths.push_back(userPaths[i]);
    }
  // clean things up
  this->ExpandRegistryAndCleanPath();
}

void cmFindBase::AddEnvironmentVairables()
{ 
  if(this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    {
    cmSystemTools::GetPath(this->SearchPaths, "CMAKE_FRAMEWORK_PATH");
    }
  std::string var = "CMAKE_";
  var += this->CMakePathName;
  var += "_PATH";
  cmSystemTools::GetPath(this->SearchPaths, var.c_str());
  if(this->SearchFrameworkLast)
    {
    cmSystemTools::GetPath(this->SearchPaths, "CMAKE_FRAMEWORK_PATH");
    }

}

void cmFindBase::AddCMakeVairables()
{ 
  if(this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    { 
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_FRAMEWORK_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, this->SearchPaths);
      }
    }
  std::string var = "CMAKE_";
  var += this->CMakePathName;
  var += "_PATH";
  if(const char* path = this->Makefile->GetDefinition(var.c_str()))
    {
    cmSystemTools::ExpandListArgument(path, this->SearchPaths);
    } 
  if(this->SearchFrameworkLast)
    {
    if(const char* path = 
       this->Makefile->GetDefinition("CMAKE_FRAMEWORK_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, this->SearchPaths);
      }
    }
}

void cmFindBase::AddSystemEnvironmentVairables()
{
  // Add LIB or INCLUDE
  if(this->EnvironmentPath.size())
    {
    cmSystemTools::GetPath(this->SearchPaths, this->EnvironmentPath.c_str());
    }
  // Add PATH 
  cmSystemTools::GetPath(this->SearchPaths);
}

void cmFindBase::AddCMakeSystemVariables()
{  
  if(this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    { 
    if(const char* path = this->Makefile->GetDefinition("CMAKE_SYSTEM_FRAMEWORK_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, this->SearchPaths);
      }
    }
  std::string var = "CMAKE_SYSTEM_";
  var += this->CMakePathName;
  var += "_PATH";
  if(const char* path = this->Makefile->GetDefinition(var.c_str()))
    {
    cmSystemTools::ExpandListArgument(path, this->SearchPaths);
    }  
  if(this->SearchFrameworkLast)
    {
    if(const char* path = this->Makefile->GetDefinition("CMAKE_SYSTEM_FRAMEWORK_PATH"))
      {
      cmSystemTools::ExpandListArgument(path, this->SearchPaths);
      }
    }
}

void cmFindBase::ExpandRegistryAndCleanPath()
{
  std::vector<std::string> finalPath;
  std::vector<std::string>::iterator i;
  for(i = this->SearchPaths.begin();
      i != this->SearchPaths.end(); ++i)
    {
    cmSystemTools::ExpandRegistryValues(*i);
    cmSystemTools::GlobDirs(i->c_str(), finalPath);
    }
  this->SearchPaths.clear();
  // convert all paths to unix slashes
  for(i = finalPath.begin();
      i != finalPath.end(); ++i)
    {
    cmSystemTools::ConvertToUnixSlashes(*i);
    // copy each finalPath combined with SearchPathSuffixes
    // to the SearchPaths ivar
    for(std::vector<std::string>::iterator j = this->SearchPathSuffixes.begin();
        j != this->SearchPathSuffixes.end(); ++j)
      {
      std::string p = *i + std::string("/") + *j;
      if(cmSystemTools::FileIsDirectory(p.c_str()))
        {
        this->SearchPaths.push_back(p);
        }
      }
    }
  // now put the path without the path suffixes in the SearchPaths
  for(i = finalPath.begin();
      i != finalPath.end(); ++i)
    {
    if(cmSystemTools::FileIsDirectory(i->c_str()))
      {
      this->SearchPaths.push_back(*i);
      }
    }
}

void cmFindBase::PrintFindStuff()
{
  std::cerr << "VariableName " << this->VariableName << "\n";
  std::cerr << "VariableDocumentation " << this->VariableDocumentation << "\n";
  std::cerr << "NoDefaultPath " << this->NoDefaultPath << "\n";
  std::cerr << "NoCMakeEnvironmentPath " << this->NoCMakeEnvironmentPath << "\n";
  std::cerr << "NoCMakePath " << this->NoCMakePath << "\n";
  std::cerr << "NoSystemEnvironmentPath " << this->NoSystemEnvironmentPath << "\n";
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
  const char* cacheValue
    = this->Makefile->GetDefinition(this->VariableName.c_str());
  if(cacheValue && !cmSystemTools::IsNOTFOUND(cacheValue))
    {
    return true;
    }
  if(cacheValue)
    {
    cmCacheManager::CacheIterator it = 
      this->Makefile->GetCacheManager()->GetCacheIterator(this->VariableName.c_str());
    if(!it.IsAtEnd())
      {
      const char* hs = it.GetProperty("HELPSTRING");
      this->VariableDocumentation = hs?hs:"(none)";
      }
    }
  return false;
}
