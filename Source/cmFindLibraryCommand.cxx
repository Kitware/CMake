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
  this->GenericDocumentation +=
    "\n"
    "Some platforms define implicit library directories such as "
    "/lib and /usr/lib that are automatically searched by the linker.  "
    "If this command finds a library in one of these directories "
    "it will report only the name of the library file and not the path.  "
    "When the name is used to link the library CMake will generate a "
    "link line that asks the linker to search for it.  This allows "
    "the system linker to automatically adjust the implicit directory "
    "set based on the current architecture."
    ;
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

    // If the existing value was loaded from a cache written by CMake
    // 2.4 or below then force the implicit link directory fix on the
    // value.
    if(this->Makefile->NeedCacheCompatibility(2, 4))
      {
      if(const char* v =
         this->Makefile->GetDefinition(this->VariableName.c_str()))
        {
        std::string nv = this->FixForImplicitLocations(v);
        if(nv != v)
          {
          this->Makefile
            ->AddCacheDefinition(this->VariableName.c_str(),
                                 nv.c_str(),
                                 this->VariableDocumentation.c_str(),
                                 cmCacheManager::FILEPATH);
          }
        }
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

  std::string library;
  for(std::vector<std::string>::iterator i = this->Names.begin();
      i != this->Names.end() ; ++i)
    {
    library = this->FindLibrary(i->c_str());
    if(library != "")
      {
      library = this->FixForImplicitLocations(library);
      this->Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                         library.c_str(),
                                         this->VariableDocumentation.c_str(),
                                         cmCacheManager::FILEPATH);
      return true;
      } 
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
    this->Makefile->GetRequiredDefinition("CMAKE_SIZEOF_VOID_P");
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

std::string cmFindLibraryCommand::FindLibrary(const char* name)
{
  bool supportFrameworks = false;
  bool onlyFrameworks = false;
  std::string ff = this->Makefile->GetSafeDefinition("CMAKE_FIND_FRAMEWORK");
  if(ff == "FIRST" || ff == "LAST")
    {
    supportFrameworks = true;
    }
  if(ff == "ONLY")
    {
    onlyFrameworks = true;
    supportFrameworks = true;
    }
  
  const char* prefixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_PREFIXES");
  const char* suffixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_SUFFIXES");
  std::vector<std::string> prefixes;
  std::vector<std::string> suffixes;
  cmSystemTools::ExpandListArgument(prefixes_list, prefixes, true);
  cmSystemTools::ExpandListArgument(suffixes_list, suffixes, true);
  // Add a trailing slash to all paths to aid the search process.
  for(std::vector<std::string>::iterator i = this->SearchPaths.begin();
      i != this->SearchPaths.end(); ++i)
    {
    std::string& p = *i;
    if(p.empty() || p[p.size()-1] != '/')
      {
      p += "/";
      }
    }
  std::string tryPath;
  for(std::vector<std::string>::const_iterator p = this->SearchPaths.begin();
      p != this->SearchPaths.end(); ++p)
    {
    if(supportFrameworks)
      {
      tryPath = *p;
      tryPath += name;
      tryPath += ".framework";
      if(cmSystemTools::FileExists(tryPath.c_str())
         && cmSystemTools::FileIsDirectory(tryPath.c_str()))
        {
        tryPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
        cmSystemTools::ConvertToUnixSlashes(tryPath);
        return tryPath;
        }
      }
    if(!onlyFrameworks)
      {
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
std::string
cmFindLibraryCommand::FixForImplicitLocations(std::string const& lib)
{
  // Get implicit link directories for the platform.
  const char* implicitLinks =
     (this->Makefile->GetDefinition
      ("CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES"));
  if(!implicitLinks)
    {
    // There are no implicit link directories.  No fix is needed.
    return lib;
    }
  std::vector<std::string> implicitLinkVec;
  cmSystemTools::ExpandListArgument(implicitLinks, implicitLinkVec);

  // Get the path containing the library.
  std::string libDir = cmSystemTools::GetFilenamePath(lib);

  // Many system linkers support multiple architectures by
  // automatically selecting the implicit linker search path for the
  // current architecture.  If the library appears in an implicit link
  // directory then just report the file name without the directory
  // portion.  This will allow the system linker to locate the proper
  // library for the architecture at link time.
  for(std::vector<std::string>::const_iterator i = implicitLinkVec.begin();
      i != implicitLinkVec.end(); ++i)
    {
    if(*i == libDir)
      {
      // The library appears in an implicit link directory.  Report
      // only the file name.
      return cmSystemTools::GetFilenameName(lib);
      }
    }

  // No implicit link directory matched.  No fix is needed.
  return lib;
}
