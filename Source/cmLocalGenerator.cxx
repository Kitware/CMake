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
  // The first time this is called, initialize all
  // the path ivars that are used.   This can not 
  // be moved to the constructor because all the paths are not set yet.
  if(m_CurrentOutputDirectory.size() == 0)
    {
    m_CurrentOutputDirectory = m_Makefile->GetCurrentOutputDirectory();
    m_HomeOutputDirectory =  m_Makefile->GetHomeOutputDirectory();
    m_HomeDirectory = m_Makefile->GetHomeDirectory();
#if defined(_WIN32) || defined(__APPLE__)
    m_CurrentOutputDirectory = cmSystemTools::LowerCase(m_CurrentOutputDirectory);
    m_HomeOutputDirectory = cmSystemTools::LowerCase(m_HomeOutputDirectory);
    m_HomeDirectory = cmSystemTools::LowerCase(m_HomeDirectory);
#endif
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
#if defined(_WIN32) || defined(__APPLE__)
  pathIn = cmSystemTools::LowerCase(pathIn);
#endif

  std::string ret = pathIn;
  cmSystemTools::ReplaceString(ret, m_CurrentOutputDirectory.c_str(), "");
  cmSystemTools::ReplaceString(ret, m_HomeDirectory.c_str(),
                               m_RelativePathToSourceDir.c_str());
  cmSystemTools::ReplaceString(ret, m_HomeOutputDirectory.c_str(),
                               m_RelativePathToBinaryDir.c_str());
  std::string relpath = m_RelativePathToBinaryDir;
  if(relpath.size())
    {
    relpath.erase(relpath.size()-1, 1);
    }
  else
    {
    relpath = ".";
    }
  cmSystemTools::ReplaceString(ret, m_HomeOutputDirectoryNoSlash.c_str(),
                               relpath.c_str());
  ret = cmSystemTools::ConvertToOutputPath(ret.c_str());
  return ret;
}
