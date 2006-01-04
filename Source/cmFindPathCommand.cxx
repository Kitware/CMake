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

// cmFindPathCommand
bool cmFindPathCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  std::string helpString = "What is the path where the file ";
  helpString += argsIn[1] + " can be found";
  std::vector<std::string> args;
  size_t size = argsIn.size();
  for(unsigned int j = 0; j < size; ++j)
    {
    if(argsIn[j] != "DOC")
      {
      args.push_back(argsIn[j]);
      }
    else
      {
      if(j+1 < size)
        {
        helpString = argsIn[j+1];
        }
      break;
      }
    }

  const char* cacheValue
    = m_Makefile->GetDefinition(args[0].c_str());
  if(cacheValue && !cmSystemTools::IsNOTFOUND(cacheValue))
    { 
    return true;
    }
  if(cacheValue)
    {
    cmCacheManager::CacheIterator it = 
      m_Makefile->GetCacheManager()->GetCacheIterator(args[0].c_str());
    if(!it.IsAtEnd())
      {
      const char* hs = it.GetProperty("HELPSTRING");
      helpString = hs?hs:"(none)";
      }
    }
  std::vector<std::string> path;
  // add any user specified paths
  for (unsigned int j = 2; j < args.size(); j++)
    {
    // expand variables
    std::string exp = args[j];
    cmSystemTools::ExpandRegistryValues(exp);      

    // Glob the entry in case of wildcards.
    cmSystemTools::GlobDirs(exp.c_str(), path);
    }
  cmSystemTools::GetPath(path, "CMAKE_INCLUDE_PATH");
  // add the standard path
  cmSystemTools::GetPath(path);
  unsigned int k;
  for(k=0; k < path.size(); k++)
    {
    std::string tryPath = path[k];
    tryPath += "/";
    tryPath += args[1];
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      path[k] = cmSystemTools::CollapseFullPath(path[k].c_str()); 
      if(path[k].size() && path[k][path[k].size()-1] == '/')
        {
        path[k] = path[k].substr(0, path[k].size()-1);
        }
      m_Makefile->AddCacheDefinition(args[0].c_str(),
                                     path[k].c_str(),
                                     helpString.c_str(),
                                     cmCacheManager::PATH);
      return true;
      }
    }
#if defined (__APPLE__)
  cmStdString fpath = this->FindHeaderInFrameworks(path, args[0].c_str(), args[1].c_str());
  if(fpath.size())
    {
    m_Makefile->AddCacheDefinition(args[0].c_str(),
                                   fpath.c_str(),
                                   helpString.c_str(),
                                   cmCacheManager::FILEPATH);
    return true;
    }
#endif  

  m_Makefile->AddCacheDefinition(args[0].c_str(),
                                 (args[0] + "-NOTFOUND").c_str(),
                                 helpString.c_str(),
                                 cmCacheManager::PATH);
  return true;
}

cmStdString cmFindPathCommand::FindHeaderInFrameworks(
  std::vector<std::string> path,
  const char* defineVar,
  const char* file)
{
  (void)defineVar;

#ifndef __APPLE__
  (void)path;
  (void)file;
  return cmStdString("");
#else
  cmStdString fileName = file;
  cmStdString frameWorkName;
  cmStdString::size_type pos = fileName.find("/");
  if(pos != fileName.npos)
    {
    // remove the name from the slash;
    fileName = fileName.substr(pos+1);
    frameWorkName = file;
    frameWorkName = frameWorkName.substr(0, frameWorkName.size()-fileName.size()-1);
    // if the framework has a path in it then just use the filename
    if(frameWorkName.find("/") != frameWorkName.npos)
      {
      fileName = file;
      frameWorkName = "";
      }
    }
  path.push_back("~/Library/Frameworks");
  path.push_back("/Library/Frameworks");
  path.push_back("/Network/Library/Frameworks");
  path.push_back("/System/Library/Frameworks");
  for(  std::vector<std::string>::iterator i = path.begin();
        i != path.end(); ++i)
    {
    if(frameWorkName.size())
      {
      std::string fpath = *i;
      fpath += "/";
      fpath += frameWorkName;
      fpath += ".framework";
      std::string intPath = fpath;
      intPath += "/Headers/";
      intPath += fileName;
      if(cmSystemTools::FileExists(intPath.c_str()))
        {
        return fpath;
        }
      }
    cmStdString glob = *i;
    glob += "/*/Headers/";
    glob += file;
    cmGlob globIt;
    globIt.FindFiles(glob);
    std::vector<std::string> files = globIt.GetFiles();
    if(files.size())
      {
      cmStdString fheader = cmSystemTools::CollapseFullPath(files[0].c_str());
      fheader = cmSystemTools::GetFilenamePath(fheader);
      return fheader;
      }
    }
  return cmStdString("");
#endif

}
