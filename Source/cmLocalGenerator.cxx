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
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmake.h"
#include "cmMakefile.h"

cmLocalGenerator::cmLocalGenerator()
{
  m_Makefile = new cmMakefile;
  m_Makefile->SetLocalGenerator(this);
}

cmLocalGenerator::~cmLocalGenerator()
{
  delete m_Makefile;
}

void cmLocalGenerator::Configure()
{
  // set the PROJECT_SOURCE_DIR and PROJECT_BIN_DIR to default values
  // just in case the project does not include a PROJECT command
  m_Makefile->AddDefinition("PROJECT_BINARY_DIR",
                            m_Makefile->GetHomeOutputDirectory());
  m_Makefile->AddDefinition("PROJECT_SOURCE_DIR",
                            m_Makefile->GetHomeDirectory());
  
  // find & read the list file
  std::string currentStart = m_Makefile->GetStartDirectory();
  currentStart += "/CMakeLists.txt";
  m_Makefile->ReadListFile(currentStart.c_str());
}

void cmLocalGenerator::SetGlobalGenerator(cmGlobalGenerator *gg)
{
  m_GlobalGenerator = gg; 

  // setup the home directories
  m_Makefile->SetHomeDirectory(
    gg->GetCMakeInstance()->GetHomeDirectory());
  m_Makefile->SetHomeOutputDirectory(
    gg->GetCMakeInstance()->GetHomeOutputDirectory());
  
}


void cmLocalGenerator::ConfigureFinalPass()
{ 
  m_Makefile->ConfigureFinalPass(); 
}

std::string cmLocalGenerator::ConvertToRelativeOutputPath(const char* p)
{
  // do not use relative paths for network build trees
  // the network paths do not work
  const char* outputDirectory = m_Makefile->GetHomeOutputDirectory();
  if ( outputDirectory && *outputDirectory && *(outputDirectory+1) && 
    outputDirectory[0] == '/' && outputDirectory[1] == '/' )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  
  // The first time this is called, initialize all
  // the path ivars that are used.   This can not 
  // be moved to the constructor because all the paths are not set yet.
  if(m_CurrentOutputDirectory.size() == 0)
    {
    m_CurrentOutputDirectory = m_Makefile->GetCurrentOutputDirectory();
    m_HomeOutputDirectory =  m_Makefile->GetHomeOutputDirectory();
    m_HomeDirectory = m_Makefile->GetHomeDirectory();
    if(m_RelativePathToSourceDir.size() == 0)
      {
      m_RelativePathToSourceDir = cmSystemTools::RelativePath(
        m_CurrentOutputDirectory.c_str(),
        m_HomeDirectory.c_str());
      std::string path = m_CurrentOutputDirectory;
      cmSystemTools::ReplaceString(path, m_HomeOutputDirectory.c_str(), "");
      unsigned i;
      m_RelativePathToBinaryDir = "";
      for(i =0; i < path.size(); ++i)
        {
        if(path[i] == '/')
          {
          m_RelativePathToBinaryDir += "../";
          }
        }
      }
    m_HomeOutputDirectoryNoSlash = m_HomeOutputDirectory;
    m_HomeOutputDirectory += "/";
    m_CurrentOutputDirectory += "/";
    }

  // Do the work of converting to a relative path 
  std::string pathIn = p;
  if(pathIn.find('/') == pathIn.npos)
    {
    return pathIn;
    }
  
  if(pathIn.size() && pathIn[0] == '\"')
    {
    pathIn = pathIn.substr(1, pathIn.size()-2);
    }
  
  std::string ret = pathIn;
  if(m_CurrentOutputDirectory.size() <= ret.size())
    {
    std::string sub = ret.substr(0, m_CurrentOutputDirectory.size());
    if(
#if defined(_WIN32) || defined(__APPLE__)
      cmSystemTools::LowerCase(sub) ==
      cmSystemTools::LowerCase(m_CurrentOutputDirectory)
#else
      sub == m_CurrentOutputDirectory
#endif
      )
      {
      ret = ret.substr(m_CurrentOutputDirectory.size(), ret.npos);
      }
    }
  if(m_HomeDirectory.size() <= ret.size())
    {
    std::string sub = ret.substr(0, m_HomeDirectory.size());
    if(
#if defined(_WIN32) || defined(__APPLE__)
      cmSystemTools::LowerCase(sub) ==
      cmSystemTools::LowerCase(m_HomeDirectory)
#else
      sub == m_HomeDirectory
#endif
      )
      {
      ret = m_RelativePathToSourceDir + ret.substr(m_HomeDirectory.size(), ret.npos);
      }
    }
  if(m_HomeOutputDirectory.size() <= ret.size())
    {
    std::string sub = ret.substr(0, m_HomeOutputDirectory.size());
    if(
#if defined(_WIN32) || defined(__APPLE__)
      cmSystemTools::LowerCase(sub) ==
      cmSystemTools::LowerCase(m_HomeOutputDirectory)
#else
      sub == m_HomeOutputDirectory
#endif
      )
      {
      ret = m_RelativePathToBinaryDir + ret.substr(m_HomeOutputDirectory.size(), ret.npos);
      }
    }
  
  std::string relpath = m_RelativePathToBinaryDir;
  if(relpath.size())
    {
    relpath.erase(relpath.size()-1, 1);
    }
  else
    {
    relpath = ".";
    }
  if(
#if defined(_WIN32) || defined(__APPLE__)
    cmSystemTools::LowerCase(ret) ==
    cmSystemTools::LowerCase(m_HomeOutputDirectoryNoSlash)
#else
    ret == m_HomeOutputDirectoryNoSlash
#endif
    )
    {
    ret = relpath;
    }
  if(ret.size() 
     && ret[0] != '\"' && ret[0] != '/' && ret[0] != '.')
    {
    if(ret.size() > 1 && ret[1] != ':')
      {
      ret = std::string("./") + ret;
      }
    }
  ret = cmSystemTools::ConvertToOutputPath(ret.c_str());
  return ret;
}
