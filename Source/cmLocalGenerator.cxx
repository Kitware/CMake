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
#include "cmGeneratedFileStream.h"

cmLocalGenerator::cmLocalGenerator()
{
  m_Makefile = new cmMakefile;
  m_Makefile->SetLocalGenerator(this);
  m_ExcludeFromAll = false;
  m_Parent = 0;
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

void cmLocalGenerator::GenerateInstallRules()
{
  const cmTargets &tgts = m_Makefile->GetTargets();
  const char* prefix
    = m_Makefile->GetDefinition("CMAKE_INSTALL_PREFIX");
  if (!prefix)
    {
    prefix = "/usr/local";
    }

  std::string file = m_Makefile->GetStartOutputDirectory();
  std::string homedir = m_Makefile->GetHomeOutputDirectory();
  std::string currdir = m_Makefile->GetCurrentOutputDirectory();
  cmSystemTools::ConvertToUnixSlashes(file);
  cmSystemTools::ConvertToUnixSlashes(homedir);
  cmSystemTools::ConvertToUnixSlashes(currdir);
  int toplevel_install = 0;
  if ( currdir == homedir )
    {
    toplevel_install = 1;
    }
  file += "/cmake_install.cmake";
  cmGeneratedFileStream tempFile(file.c_str());
  std::ostream&  fout = tempFile.GetStream();

  fout << "# Install script for directory: " << m_Makefile->GetCurrentDirectory() 
    << std::endl << std::endl;

  const char* cmakeDebugPosfix = m_Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX");
  if ( cmakeDebugPosfix )
    {
    fout << "SET(CMAKE_DEBUG_POSTFIX \"" << cmakeDebugPosfix << "\")"
      << std::endl << std::endl;
    }

  std::string libOutPath = "";
  if (m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    libOutPath = m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    if(libOutPath.size())
      {
      if(libOutPath[libOutPath.size() -1] != '/')
        {
        libOutPath += "/";
        }
      }
    }

  std::string exeOutPath = "";
  if (m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    exeOutPath =
      m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    if(exeOutPath.size())
      {
      if(exeOutPath[exeOutPath.size() -1] != '/')
        {
        exeOutPath += "/";
        }
      }
    }

  std::string destination;
  for(cmTargets::const_iterator l = tgts.begin(); 
    l != tgts.end(); l++)
    {
    const char* preinstall = l->second.GetProperty("PRE_INSTALL_SCRIPT");
    const char* postinstall = l->second.GetProperty("POST_INSTALL_SCRIPT");
    if ( preinstall )
      {
      fout << "INCLUDE(\"" << preinstall << "\")" << std::endl;
      }
    if (l->second.GetInstallPath() != "")
      {
      destination = prefix + l->second.GetInstallPath();
      cmSystemTools::ConvertToUnixSlashes(destination);
      const char* dest = destination.c_str();
      int type = l->second.GetType();


      std::string fname;
      const char* files;
      // now install the target
      switch (type)
        {
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        fname = libOutPath;
        fname += this->GetFullTargetName(l->first.c_str(), l->second);
        files = fname.c_str();
        this->AddInstallRule(fout, dest, type, files);
        break;
      case cmTarget::SHARED_LIBRARY:
        {
        // Special code to handle DLL
        fname = libOutPath;
        fname += this->GetFullTargetName(l->first.c_str(), l->second);
        std::string ext = cmSystemTools::GetFilenameExtension(fname);
        ext = cmSystemTools::LowerCase(ext);
        if ( ext == ".dll" )
          {
          std::string libname = libOutPath;
          libname += cmSystemTools::GetFilenameWithoutExtension(fname);
          libname += ".lib";
          files = libname.c_str();
          this->AddInstallRule(fout, dest, cmTarget::STATIC_LIBRARY, files, true);
          std::string dlldest = prefix + l->second.GetRuntimeInstallPath();
          files = fname.c_str();
          this->AddInstallRule(fout, dlldest.c_str(), type, files);
          }
        else
          {
          files = fname.c_str();
          this->AddInstallRule(fout, dest, type, files);
          }
        }
        break;
      case cmTarget::EXECUTABLE:
        fname = exeOutPath;
        fname += this->GetFullTargetName(l->first.c_str(), l->second);
        files = fname.c_str();
        this->AddInstallRule(fout, dest, type, files);
        break;
      case cmTarget::INSTALL_FILES:
          {
          std::string sourcePath = m_Makefile->GetCurrentDirectory();
          std::string binaryPath = m_Makefile->GetCurrentOutputDirectory();
          sourcePath += "/";
          binaryPath += "/";
          const std::vector<std::string> &sf = l->second.GetSourceLists();
          std::vector<std::string>::const_iterator i;
          for (i = sf.begin(); i != sf.end(); ++i)
            {
            std::string f = *i;
            if(f.substr(0, sourcePath.length()) == sourcePath)
              {
              f = f.substr(sourcePath.length());
              }
            else if(f.substr(0, binaryPath.length()) == binaryPath)
              {
              f = f.substr(binaryPath.length());
              }

            files = i->c_str();
            this->AddInstallRule(fout, dest, type, files);
            }
          }
        break;
      case cmTarget::INSTALL_PROGRAMS:
          {
          std::string sourcePath = m_Makefile->GetCurrentDirectory();
          std::string binaryPath = m_Makefile->GetCurrentOutputDirectory();
          sourcePath += "/";
          binaryPath += "/";
          const std::vector<std::string> &sf = l->second.GetSourceLists();
          std::vector<std::string>::const_iterator i;
          for (i = sf.begin(); i != sf.end(); ++i)
            {
            std::string f = *i;
            if(f.substr(0, sourcePath.length()) == sourcePath)
              {
              f = f.substr(sourcePath.length());
              }
            else if(f.substr(0, binaryPath.length()) == binaryPath)
              {
              f = f.substr(binaryPath.length());
              }
            files = i->c_str();
            this->AddInstallRule(fout, dest, type, files);
            }
          }
        break;
      case cmTarget::UTILITY:
      default:
        break;
        }
      }
    if ( postinstall )
      {
      fout << "INCLUDE(\"" << postinstall << "\")" << std::endl;
      }
    }
  cmMakefile* mf = this->GetMakefile();
  if ( !mf->GetSubDirectories().empty() )
    {
    const std::vector<std::pair<cmStdString, bool> >& subdirs = mf->GetSubDirectories();
    std::vector<std::pair<cmStdString, bool> >::const_iterator i = subdirs.begin();
    for(; i != subdirs.end(); ++i)
      {
      std::string odir = mf->GetCurrentOutputDirectory();
      odir += "/" + (*i).first;
      cmSystemTools::ConvertToUnixSlashes(odir);
      fout << "INCLUDE(" <<  odir.c_str() 
           << "/cmake_install.cmake)" << std::endl;
      }
    fout << std::endl;;
    }
  if ( toplevel_install )
    {
    fout << "FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})" << std::endl
      << "  FILE(APPEND \"" << homedir.c_str() << "/install_manifest.txt\" "
      << "\"${file}\\n\")" << std::endl
      << "ENDFOREACH(file)" << std::endl;
    }
}

void cmLocalGenerator::AddInstallRule(std::ostream& fout, const char* dest, 
  int type, const char* files, bool optional)
{
  std::string sfiles = files;
  std::string destination = dest;
  std::string stype;
  switch ( type )
    {
  case cmTarget::INSTALL_PROGRAMS: stype = "PROGRAM"; break;
  case cmTarget::EXECUTABLE: stype = "EXECUTABLE"; break;
  case cmTarget::STATIC_LIBRARY:   stype = "STATIC_LIBRARY"; break;
  case cmTarget::SHARED_LIBRARY:   stype = "SHARED_LIBRARY"; break;
  case cmTarget::MODULE_LIBRARY:   stype = "MODULE"; break;
  case cmTarget::INSTALL_FILES:
  default:                         stype = "FILE"; break;
    }
  std::string fname = cmSystemTools::GetFilenameName(sfiles.c_str());
  fout 
    << "MESSAGE(STATUS \"Installing " << destination.c_str() 
    << "/" << fname.c_str() << "\")\n" 
    << "FILE(INSTALL DESTINATION \"" << destination.c_str() 
    << "\" TYPE " << stype.c_str() << (optional?" OPTIONAL":"") 
    << " FILES \"" << sfiles.c_str() << "\")\n";
}

const char* cmLocalGenerator::GetSafeDefinition(const char* def)
{
  const char* ret = m_Makefile->GetDefinition(def);
  if(!ret)
    {
    return "";
    }
  return ret;
}

std::string cmLocalGenerator::GetFullTargetName(const char* n,
  const cmTarget& t)
{
  const char* targetPrefix = t.GetProperty("PREFIX");
  const char* targetSuffix = t.GetProperty("SUFFIX");
  const char* prefixVar = 0;
  const char* suffixVar = 0;
  switch(t.GetType())
    {
  case cmTarget::STATIC_LIBRARY:
    prefixVar = "CMAKE_STATIC_LIBRARY_PREFIX";
    suffixVar = "CMAKE_STATIC_LIBRARY_SUFFIX";
    break;
  case cmTarget::SHARED_LIBRARY:
    prefixVar = "CMAKE_SHARED_LIBRARY_PREFIX";
    suffixVar = "CMAKE_SHARED_LIBRARY_SUFFIX";
    break;
  case cmTarget::MODULE_LIBRARY:
    prefixVar = "CMAKE_SHARED_MODULE_PREFIX";
    suffixVar = "CMAKE_SHARED_MODULE_SUFFIX";
    break;
  case cmTarget::EXECUTABLE:
    targetSuffix = cmSystemTools::GetExecutableExtension();
  case cmTarget::UTILITY:
  case cmTarget::INSTALL_FILES:
  case cmTarget::INSTALL_PROGRAMS:
    break;
    }
  // if there is no prefix on the target use the cmake definition
  if(!targetPrefix && prefixVar)
    {
    targetPrefix = this->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = this->GetSafeDefinition(suffixVar);
    }
  std::string name = targetPrefix?targetPrefix:"";
  name += n;
  name += targetSuffix?targetSuffix:"";
  return name;
}


std::string cmLocalGenerator::ConvertToRelativeOutputPath(const char* p)
{
  if ( !m_Makefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
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

  // Relative paths should always start in a '.', so add a './' if
  // necessary.
  if(ret.size()
     && ret[0] != '\"' && ret[0] != '/' && ret[0] != '.' && ret[0] != '$')
    {
    if(ret.size() > 1 && ret[1] != ':')
      {
      ret = std::string("./") + ret;
      }
    }
  ret = cmSystemTools::ConvertToOutputPath(ret.c_str());
  return ret;
}
