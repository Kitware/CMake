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
#include "cmFindFileCommand.h"
#include "cmCacheManager.h"
#include "cmGlob.h"
#include <stdlib.h>
  

// cmFindFileCommand
bool cmFindFileCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string helpString = "Where can the ";
  helpString += argsIn[1] + " file be found";
  size_t size = argsIn.size();
  std::vector<std::string> args;
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

  std::vector<std::string>::const_iterator i = args.begin();
  // Use the first argument as the name of something to be defined
  const char* define = (*i).c_str();
  i++; // move iterator to next arg
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = m_Makefile->GetDefinition(define);
  if(cacheValue && !cmSystemTools::IsNOTFOUND(cacheValue))
    {
    return true;
    }
  // if it is not in the cache, then search the system path
  std::vector<std::string> path;

  // add any user specified paths
  for (unsigned int j = 2; j < args.size(); j++)
    {
    // Glob the entry in case of wildcards.
    cmSystemTools::GlobDirs(args[j].c_str(), path);
    }

  cmSystemTools::GetPath(path, "CMAKE_LIBRARY_PATH");

  // add the standard path
  cmSystemTools::GetPath(path);
  for(unsigned int k=0; k < path.size(); k++)
    {
    std::string tryPath = path[k];
    tryPath += "/";
    tryPath += *i;
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      // Save the value in the cache
      m_Makefile->AddCacheDefinition(define,
                                     tryPath.c_str(),
                                     helpString.c_str(),
                                     cmCacheManager::FILEPATH);
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
  std::string s = args[0] + "-NOTFOUND";
  m_Makefile->AddCacheDefinition(args[0].c_str(),
                                 s.c_str(),
                                 helpString.c_str(),
                                 cmCacheManager::FILEPATH);
  return true;
}

cmStdString cmFindFileCommand::FindHeaderInFrameworks(
  std::vector<std::string> path,
  const char* defineVar,
  const char* file)
{
#ifndef __APPLE__
  (void)path;
  (void)defineVar;
  (void)file;
  return cmStdString("");
#else
  cmStdString fileName = file;
  cmStdString frameWorkName;
  cmStdString::size_type pos = fileName.find("/");
  std::cerr << "ff " << fileName << " " << pos << "\n";
  if(pos != fileName.npos)
    {
    // remove the name from the slash;
    fileName = fileName.substr(pos+1);
    frameWorkName = file;
    frameWorkName = frameWorkName.substr(0, frameWorkName.size()-fileName.size()-1);
    // if the framework has a path in it then just use the filename
    std::cerr << fileName << " " << frameWorkName << "\n";
    if(frameWorkName.find("/") != frameWorkName.npos)
      {
      fileName = file;
      frameWorkName = "";
      }
    }
  path.push_back("~/Library/Frameworks");
  path.push_back("/Library/Frameworks");
  path.push_back("/System/Library/Frameworks");
  path.push_back("/Network/Library/Frameworks");
  for(  std::vector<std::string>::iterator i = path.begin();
        i != path.end(); ++i)
    {
    if(frameWorkName.size())
      {
      std::string fpath = *i;
      fpath += "/";
      fpath += frameWorkName;
      fpath += ".framework";
      std::string path = fpath;
      path += "/Headers/";
      path += fileName;
      std::cerr << "try " << path << "\n";
      if(cmSystemTools::FileExists(path.c_str()))
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
