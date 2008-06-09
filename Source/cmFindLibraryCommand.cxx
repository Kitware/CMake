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
#include "cmFindLibraryCommand.h"
#include "cmCacheManager.h"

cmFindLibraryCommand::cmFindLibraryCommand()
{ 
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "FIND_XXX", "find_library");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_PATH", "CMAKE_LIBRARY_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_MAC_PATH",
                               "CMAKE_FRAMEWORK_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_MAC_PATH",
                               "CMAKE_SYSTEM_FRAMEWORK_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SYSTEM", "LIB");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_PATH", 
                               "CMAKE_SYSTEM_LIBRARY_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX_DESC", "library");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX", "library");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SUBDIR", "lib");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_FIND_ROOT_PATH_MODE_XXX", 
                               "CMAKE_FIND_ROOT_PATH_MODE_LIBRARY");

  this->EnvironmentPath = "LIB";
  this->GenericDocumentation += 
    "\n"
    "If the library found is a framework, then VAR will be set to "
    "the full path to the framework <fullPath>/A.framework. "
    "When a full path to a framework is used as a library, "
    "CMake will use a -framework A, and a -F<fullPath> to "
    "link the framework to the target. ";
}

// cmFindLibraryCommand
bool cmFindLibraryCommand
::InitialPass(std::vector<std::string> const& argsIn, cmExecutionStatus &)
{
  this->VariableDocumentation = "Path to a library.";
  this->CMakePathName = "LIBRARY";
  if(!this->ParseArguments(argsIn))
    {
    return false;
    }
  if(this->AlreadyInCache)
    {
    // If the user specifies the entry on the command line without a
    // type we should add the type and docstring but keep the original
    // value.
    if(this->AlreadyInCacheWithoutMetaInfo)
      {
      this->Makefile->AddCacheDefinition(this->VariableName.c_str(), "",
                                         this->VariableDocumentation.c_str(),
                                         cmCacheManager::FILEPATH);
      }
    return true;
    }

  if(const char* abi_name =
     this->Makefile->GetDefinition("CMAKE_INTERNAL_PLATFORM_ABI"))
    {
    std::string abi = abi_name;
    if(abi.find("ELF N32") != abi.npos)
      {
      // Convert lib to lib32.
      this->AddArchitecturePaths("32");
      }
    }

  if(this->Makefile->GetCMakeInstance()
     ->GetPropertyAsBool("FIND_LIBRARY_USE_LIB64_PATHS"))
    {
    // add special 64 bit paths if this is a 64 bit compile.
    this->AddLib64Paths();
    }

  std::string library = this->FindLibrary();
  if(library != "")
    {
    // Save the value in the cache
    this->Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                       library.c_str(),
                                       this->VariableDocumentation.c_str(),
                                       cmCacheManager::FILEPATH);
    return true;
    }
  std::string notfound = this->VariableName + "-NOTFOUND";
  this->Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                     notfound.c_str(),
                                     this->VariableDocumentation.c_str(),
                                     cmCacheManager::FILEPATH);
  return true;
}

//----------------------------------------------------------------------------
void cmFindLibraryCommand::AddArchitecturePaths(const char* suffix)
{
  std::vector<std::string> newPaths;
  bool found = false;
  std::string subpath = "lib";
  subpath += suffix;
  subpath += "/";
  for(std::vector<std::string>::iterator i = this->SearchPaths.begin();
      i != this->SearchPaths.end(); ++i)
    {
    // Try replacing lib/ with lib<suffix>/
    std::string s = *i;
    cmSystemTools::ReplaceString(s, "lib/", subpath.c_str());
    if((s != *i) && cmSystemTools::FileIsDirectory(s.c_str()))
      {
      found = true;
      newPaths.push_back(s);
      }

    // Now look for lib<suffix>
    s = *i;
    s += suffix;
    if(cmSystemTools::FileIsDirectory(s.c_str()))
      {
      found = true;
      newPaths.push_back(s);
      }
    // now add the original unchanged path
    if(cmSystemTools::FileIsDirectory(i->c_str()))
      {
      newPaths.push_back(*i);
      }
    }

  // If any new paths were found replace the original set.
  if(found)
    {
    this->SearchPaths = newPaths;
    }
}

void cmFindLibraryCommand::AddLib64Paths()
{  
  if(!this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
     GetLanguageEnabled("C"))
    {
    return;
    }
  std::string voidsize =
    this->Makefile->GetSafeDefinition("CMAKE_SIZEOF_VOID_P");
  int size = atoi(voidsize.c_str());
  if(size != 8)
    {
    return;
    }
  std::vector<std::string> path64;
  bool found64 = false;
  for(std::vector<std::string>::iterator i = this->SearchPaths.begin(); 
      i != this->SearchPaths.end(); ++i)
    {
    std::string s = *i;
    std::string s2 = *i;
    cmSystemTools::ReplaceString(s, "lib/", "lib64/");
    // try to replace lib with lib64 and see if it is there,
    // then prepend it to the path
    if((s != *i) && cmSystemTools::FileIsDirectory(s.c_str()))
      {
      path64.push_back(s);
      found64 = true;
      }  
    // now just add a 64 to the path name and if it is there,
    // add it to the path
    s2 += "64";
    if(cmSystemTools::FileIsDirectory(s2.c_str()))
      {
      found64 = true;
      path64.push_back(s2);
      } 
    // now add the original unchanged path
    if(cmSystemTools::FileIsDirectory(i->c_str()))
      {
      path64.push_back(*i);
      }
    }
  // now replace the SearchPaths with the 64 bit converted path
  // if any 64 bit paths were discovered
  if(found64)
    {
    this->SearchPaths = path64;
    }
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindLibrary()
{
  std::string library;
  if(this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    {
    library = this->FindFrameworkLibrary();
    }
  if(library.empty() && !this->SearchFrameworkOnly)
    {
    library = this->FindNormalLibrary();
    }
  if(library.empty() && this->SearchFrameworkLast)
    {
    library = this->FindFrameworkLibrary();
    }
  return library;
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindNormalLibrary()
{
  // Collect the list of library name prefixes/suffixes to try.
  const char* prefixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_PREFIXES");
  const char* suffixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_SUFFIXES");
  std::vector<std::string> prefixes;
  std::vector<std::string> suffixes;
  cmSystemTools::ExpandListArgument(prefixes_list, prefixes, true);
  cmSystemTools::ExpandListArgument(suffixes_list, suffixes, true);

  // Search the entire path for each name.
  std::string tryPath;
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    // If the original library name provided by the user matches one of
    // the suffixes, try it first.
    bool tryOrig = false;
    std::string const& name = *ni;
    for(std::vector<std::string>::const_iterator si = suffixes.begin();
        !tryOrig && si != suffixes.end(); ++si)
      {
      std::string const& suffix = *si;
      if(name.length() > suffix.length() &&
         name.substr(name.size()-suffix.length()) == suffix)
        {
        tryOrig = true;
        }
      }

    for(std::vector<std::string>::const_iterator
          p = this->SearchPaths.begin();
        p != this->SearchPaths.end(); ++p)
      {
      // Try the original library name as specified by the user.
      if(tryOrig)
        {
        tryPath = *p;
        tryPath += name;
        if(cmSystemTools::FileExists(tryPath.c_str(), true))
          {
          tryPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
          cmSystemTools::ConvertToUnixSlashes(tryPath);
          return tryPath;
          }
        }

      // Try various library naming conventions.
      for(std::vector<std::string>::iterator prefix = prefixes.begin();
          prefix != prefixes.end(); ++prefix)
        {
        for(std::vector<std::string>::iterator suffix = suffixes.begin();
            suffix != suffixes.end(); ++suffix)
          {
          tryPath = *p;
          tryPath += *prefix;
          tryPath += name;
          tryPath += *suffix;
          if(cmSystemTools::FileExists(tryPath.c_str())
             && !cmSystemTools::FileIsDirectory(tryPath.c_str()))
            {
            tryPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
            cmSystemTools::ConvertToUnixSlashes(tryPath);
            return tryPath;
            }
          }
        }
      }
    }
  // Couldn't find the library.
  return "";
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindFrameworkLibrary()
{
  // Search for a framework of each name in the entire search path.
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    // Search the paths for a framework with this name.
    std::string fwName = *ni;
    fwName += ".framework";
    std::string fwPath = cmSystemTools::FindDirectory(fwName.c_str(),
                                                      this->SearchPaths,
                                                      true);
    if(!fwPath.empty())
      {
      return fwPath;
      }
    }

  // No framework found.
  return "";
}
