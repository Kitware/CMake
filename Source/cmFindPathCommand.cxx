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
  std::string ff = this->Makefile->GetSafeDefinition("CMAKE_FIND_FRAMEWORK");
  bool supportFrameworks = true;
  if( ff.size() == 0 || ff == "NEVER" )
    {
    supportFrameworks = false;
    }
  std::string framework;
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
  // Use the search path to find the file.
  unsigned int k;
  std::string result;
  for(k=0; k < this->SearchPaths.size(); k++)
    {
    for(unsigned int j =0; j < this->Names.size(); ++j)
      {
      // if frameworks are supported try to find the header in a framework
      std::string tryPath;
      if(supportFrameworks)
        {
        tryPath = this->FindHeaderInFramework(this->Names[j],
                                              this->SearchPaths[k]);
        if(tryPath.size())
          {
          result = tryPath;
          }
        }
      if(result.size() == 0)
        {
        tryPath = this->SearchPaths[k];
        tryPath += this->Names[j];
        if(cmSystemTools::FileExists(tryPath.c_str()))
          {
          if(this->IncludeFileInPath)
            {
            result = tryPath;
            }
          else
            {
            result = this->SearchPaths[k];
            }
          }
        }
      if(result.size() != 0)
        {
        this->Makefile->AddCacheDefinition
          (this->VariableName.c_str(), result.c_str(),
           this->VariableDocumentation.c_str(),
           (this->IncludeFileInPath) ? 
           cmCacheManager::FILEPATH :cmCacheManager::PATH);
        return true;
        }
      }
    }
  this->Makefile->AddCacheDefinition
    (this->VariableName.c_str(),
     (this->VariableName + "-NOTFOUND").c_str(),
     this->VariableDocumentation.c_str(),
     (this->IncludeFileInPath) ? 
     cmCacheManager::FILEPATH :cmCacheManager::PATH);
  return true;
}

std::string cmFindPathCommand::FindHeaderInFramework(std::string& file,
                                                     std::string& dir)
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
  // for all files in dir/*/Headers/
  cmStdString glob = dir;
  glob += "*/Headers/";
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

