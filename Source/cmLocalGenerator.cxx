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
  if ( libOutPath.size() == 0 )
    {
    // LIBRARY_OUTPUT_PATH not defined
    libOutPath = currdir + "/";
    }
  if ( exeOutPath.size() == 0 )
    {
    // EXECUTABLE_OUTPUT_PATH not defined
    exeOutPath = currdir + "/";
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
          std::string properties;
          const char* lib_version = l->second.GetProperty("VERSION");
          const char* lib_soversion = l->second.GetProperty("SOVERSION");
          if(!m_Makefile->GetDefinition("CMAKE_SHARED_LIBRARY_SONAME_C_FLAG"))
            {
            // Versioning is supported only for shared libraries and modules,
            // and then only when the platform supports an soname flag.
            lib_version = 0;
            lib_soversion = 0;
            }
          if ( lib_version )
            {
            properties += " VERSION ";
            properties += lib_version;
            }
          if ( lib_soversion )
            {
            properties += " SOVERSION ";
            properties += lib_soversion;
            }
          this->AddInstallRule(fout, dest, type, files, false, properties.c_str());
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
      fout << "INCLUDE(\"" <<  odir.c_str() 
           << "/cmake_install.cmake\")" << std::endl;
      }
    fout << std::endl;;
    }
  if ( toplevel_install )
    {
    fout << "FILE(WRITE \"" << homedir.c_str() << "/install_manifest.txt\" "
         << "\"\")" << std::endl;
    fout << "FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})" << std::endl
      << "  FILE(APPEND \"" << homedir.c_str() << "/install_manifest.txt\" "
      << "\"${file}\\n\")" << std::endl
      << "ENDFOREACH(file)" << std::endl;
    }
}

void cmLocalGenerator::AddInstallRule(std::ostream& fout, const char* dest, 
  int type, const char* files, bool optional /* = false */, const char* properties /* = 0 */)
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
    << "\" TYPE " << stype.c_str() << (optional?" OPTIONAL":"") ;
  if ( properties && *properties )
    {
    fout << " PROPERTIES" << properties;
    }
  fout
    << " FILES \"" << sfiles.c_str() << "\")\n";
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
    targetPrefix = m_Makefile->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = m_Makefile->GetSafeDefinition(suffixVar);
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
  // copy to a string class
  std::string pathIn = p;
  // check to see if the path is already relative, it is
  // considered relative if one of the following is true
  // - has no / in it at all
  // - does not start with / or drive leter :
  // - starts with a ".."
  if(pathIn.find('/') == pathIn.npos ||        
     (pathIn[0] != '/' && pathIn[1] != ':') || 
     pathIn.find("..") == 0)
    {
    return pathIn;
    } 

  // do not use relative paths for network build trees
  // the network paths do not work
  const char* outputDirectory = m_Makefile->GetHomeOutputDirectory();
  if ( outputDirectory && *outputDirectory && *(outputDirectory+1) &&
    outputDirectory[0] == '/' && outputDirectory[1] == '/' )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  // if the path is double quoted remove the double quotes
  if(pathIn.size() && pathIn[0] == '\"')
    {
    pathIn = pathIn.substr(1, pathIn.size()-2);
    } 
  // The first time this is called
  // initialize m_CurrentOutputDirectory to contain
  // the full path to the current output directory
  // This has to be done here and not in the constructor
  // because the output directory is not yet set in the constructor.
  if(m_CurrentOutputDirectory.size() == 0)
    {
    m_CurrentOutputDirectory = cmSystemTools::CollapseFullPath(m_Makefile->GetCurrentOutputDirectory());
    }
  // Given that we are in m_CurrentOutputDirectory how to we
  // get to pathIn with a relative path, store in ret
  std::string ret = cmSystemTools::RelativePath(m_CurrentOutputDirectory.c_str(), pathIn.c_str());
  // If the path is 0 sized make it a .
  // this happens when pathIn is the same as m_CurrentOutputDirectory
  if(ret.size() == 0)
    {
    ret = ".";
    }
  // if there was a trailing / there still is one, and
  // if there was not one, there still is not one
  if(ret[ret.size()-1] == '/' && 
     pathIn[pathIn.size()-1] != '/')
    {
    ret.erase(ret.size()-1, 1);
    }
  if(ret[ret.size()-1] != '/' && 
     pathIn[pathIn.size()-1] == '/')
    {
    ret += "/";
    }
  // Now convert the relative path to an output path
  ret = cmSystemTools::ConvertToOutputPath(ret.c_str());
  // finally return the path 
  // at this point it should be relative and in the correct format
  // for the native build system.  (i.e. \ for windows and / for unix,
  // and correct escaping/quoting of spaces in the path
  return ret;
}


  
