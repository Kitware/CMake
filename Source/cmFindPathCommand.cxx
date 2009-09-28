/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindPathCommand.h"
#include "cmCacheManager.h"

#include <cmsys/Glob.hxx>

cmFindPathCommand::cmFindPathCommand()
{
  this->EnvironmentPath = "INCLUDE";
  this->IncludeFileInPath = false;
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "FIND_XXX", "find_path");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_PATH", "CMAKE_INCLUDE_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_MAC_PATH",
                               "CMAKE_FRAMEWORK_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_MAC_PATH",
                               "CMAKE_SYSTEM_FRAMEWORK_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SYSTEM", "INCLUDE");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_PATH", 
                               "CMAKE_SYSTEM_INCLUDE_PATH"); 
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX_DESC", 
                               "directory containing the named file");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX", "file in a directory");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SUBDIR", "include");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_FIND_ROOT_PATH_MODE_XXX", 
                               "CMAKE_FIND_ROOT_PATH_MODE_INCLUDE");

  this->ExtraDocAdded = false;
}

const char* cmFindPathCommand::GetFullDocumentation()
{
  if(!this->ExtraDocAdded && !this->IncludeFileInPath)
    {
    this->GenericDocumentation += 
      "\n"
      "When searching for frameworks, if the file is specified as "
      "A/b.h, then the framework search will look for "
      "A.framework/Headers/b.h. "
      "If that is found the path will be set to the path to the framework. "
      "CMake will convert this to the correct -F option to include the "
      "file. ";
    this->ExtraDocAdded = true;
    }
  return this->GenericDocumentation.c_str();
}

// cmFindPathCommand
bool cmFindPathCommand
::InitialPass(std::vector<std::string> const& argsIn, cmExecutionStatus &)
{
  this->VariableDocumentation = "Path to a file.";
  this->CMakePathName = "INCLUDE";
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
      this->Makefile->AddCacheDefinition(
        this->VariableName.c_str(), "",
        this->VariableDocumentation.c_str(),
        (this->IncludeFileInPath ?
         cmCacheManager::FILEPATH :cmCacheManager::PATH)
        );
      }
    return true;
    }

  std::string result = this->FindHeader();
  if(result.size() != 0)
    {
    this->Makefile->AddCacheDefinition
      (this->VariableName.c_str(), result.c_str(),
       this->VariableDocumentation.c_str(),
       (this->IncludeFileInPath) ?
       cmCacheManager::FILEPATH :cmCacheManager::PATH);
    return true;
    }
  this->Makefile->AddCacheDefinition
    (this->VariableName.c_str(),
     (this->VariableName + "-NOTFOUND").c_str(),
     this->VariableDocumentation.c_str(),
     (this->IncludeFileInPath) ? 
     cmCacheManager::FILEPATH :cmCacheManager::PATH);
  return true;
}

//----------------------------------------------------------------------------
std::string cmFindPathCommand::FindHeader()
{
  std::string header;
  if(this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    {
    header = this->FindFrameworkHeader();
    }
  if(header.empty() && !this->SearchFrameworkOnly)
    {
    header = this->FindNormalHeader();
    }
  if(header.empty() && this->SearchFrameworkLast)
    {
    header = this->FindFrameworkHeader();
    }
  return header;
}

std::string
cmFindPathCommand::FindHeaderInFramework(std::string const& file,
                                         std::string const& dir)
{
  cmStdString fileName = file;
  cmStdString frameWorkName;
  cmStdString::size_type pos = fileName.find("/");
  // if there is a / in the name try to find the header as a framework
  // For example bar/foo.h would look for:
  // bar.framework/Headers/foo.h
  if(pos != fileName.npos)
    {
    // remove the name from the slash;
    fileName = fileName.substr(pos+1);
    frameWorkName = file;
    frameWorkName = 
      frameWorkName.substr(0, frameWorkName.size()-fileName.size()-1);
    // if the framework has a path in it then just use the filename
    if(frameWorkName.find("/") != frameWorkName.npos)
      {
      fileName = file;
      frameWorkName = "";
      } 
    if(frameWorkName.size())
      {
      std::string fpath = dir;
      fpath += frameWorkName;
      fpath += ".framework";
      std::string intPath = fpath;
      intPath += "/Headers/";
      intPath += fileName;
      if(cmSystemTools::FileExists(intPath.c_str()))
        { 
        if(this->IncludeFileInPath)
          {
          return intPath;
          }
        return fpath;
        }
      }
    }
  // if it is not found yet or not a framework header, then do a glob search
  // for all frameworks in the directory: dir/*.framework/Headers/<file>
  cmStdString glob = dir;
  glob += "*.framework/Headers/";
  glob += file;
  cmsys::Glob globIt;
  globIt.FindFiles(glob);
  std::vector<std::string> files = globIt.GetFiles();
  if(files.size())
    {
    cmStdString fheader = cmSystemTools::CollapseFullPath(files[0].c_str());
    if(this->IncludeFileInPath)
      {
      return fheader;
      }
    fheader = cmSystemTools::GetFilenamePath(fheader);
    return fheader;
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmFindPathCommand::FindNormalHeader()
{
  std::string tryPath;
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    for(std::vector<std::string>::const_iterator
          p = this->SearchPaths.begin();
        p != this->SearchPaths.end(); ++p)
      {
      tryPath = *p;
      tryPath += *ni;
      if(cmSystemTools::FileExists(tryPath.c_str()))
        {
        if(this->IncludeFileInPath)
          {
          return tryPath;
          }
        else
          {
          return *p;
          }
        }
      }
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmFindPathCommand::FindFrameworkHeader()
{
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    for(std::vector<std::string>::const_iterator
          p = this->SearchPaths.begin();
        p != this->SearchPaths.end(); ++p)
      {
      std::string fwPath = this->FindHeaderInFramework(*ni, *p);
      if(!fwPath.empty())
        {
        return fwPath;
        }
      }
    }
  return "";
}
