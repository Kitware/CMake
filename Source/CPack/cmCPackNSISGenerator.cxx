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

#include "cmCPackNSISGenerator.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/Directory.hxx>
#include <cmsys/RegularExpression.hxx>

/* NSIS uses different command line syntax on Windows and others */
#ifdef _WIN32
# define NSIS_OPT "/"
#else
# define NSIS_OPT "-"
#endif

//----------------------------------------------------------------------
cmCPackNSISGenerator::cmCPackNSISGenerator()
{
}

//----------------------------------------------------------------------
cmCPackNSISGenerator::~cmCPackNSISGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackNSISGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  (void)outFileName; // TODO: Fix nsis to force out file name
  (void)toplevel;
  std::string nsisInFileName = this->FindTemplate("NSIS.template.in");
  if ( nsisInFileName.size() == 0 )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "CPack error: Could not find NSIS installer template file."
      << std::endl);
    return false;
    }
  std::string nsisInInstallOptions
    = this->FindTemplate("NSIS.InstallOptions.ini.in");
  if ( nsisInInstallOptions.size() == 0 )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "CPack error: Could not find NSIS installer options file."
      << std::endl);
    return false;
    }
  std::string nsisFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  std::string tmpFile = nsisFileName;
  tmpFile += "/NSISOutput.log";
  std::string nsisInstallOptions = nsisFileName + "/NSIS.InstallOptions.ini";
  nsisFileName += "/project.nsi";
  cmOStringStream str;
  std::vector<std::string>::const_iterator it;
  for ( it = files.begin(); it != files.end(); ++ it )
    {
    std::string fileN = cmSystemTools::RelativePath(toplevel,
      it->c_str());
    cmSystemTools::ReplaceString(fileN, "/", "\\");
    str << "  Delete \"$INSTDIR\\" << fileN.c_str() << "\"" << std::endl;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Uninstall Files: "
    << str.str().c_str() << std::endl);
  this->SetOptionIfNotSet("CPACK_NSIS_DELETE_FILES", str.str().c_str());
  std::vector<std::string> dirs;
  this->GetListOfSubdirectories(toplevel, dirs);
  std::vector<std::string>::const_iterator sit;
  cmOStringStream dstr;
  for ( sit = dirs.begin(); sit != dirs.end(); ++ sit )
    {
    std::string fileN = cmSystemTools::RelativePath(toplevel,
      sit->c_str());
    if ( fileN.empty() )
      {
      continue;
      }
    cmSystemTools::ReplaceString(fileN, "/", "\\");
    dstr << "  RMDir \"$INSTDIR\\" << fileN.c_str() << "\"" << std::endl;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Uninstall Dirs: "
    << dstr.str().c_str() << std::endl);
  this->SetOptionIfNotSet("CPACK_NSIS_DELETE_DIRECTORIES", 
                          dstr.str().c_str());

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: " << nsisInFileName
    << " to " << nsisFileName << std::endl);
  if(this->IsSet("CPACK_NSIS_MUI_ICON") 
     && this->IsSet("CPACK_NSIS_MUI_UNIICON"))
    {
    std::string installerIconCode="!define MUI_ICON \"";
    installerIconCode += this->GetOption("CPACK_NSIS_MUI_ICON");
    installerIconCode += "\"\n";
    installerIconCode += "!define MUI_UNICON \"";
    installerIconCode += this->GetOption("CPACK_NSIS_MUI_ICON");
    installerIconCode += "\"\n";
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_MUI_ICON_CODE",
                            installerIconCode.c_str());
    }
  if(this->IsSet("CPACK_PACKAGE_ICON"))
    {
    std::string installerIconCode = "!define MUI_HEADERIMAGE_BITMAP \"";
    installerIconCode += this->GetOption("CPACK_PACKAGE_ICON");
    installerIconCode += "\"\n";
    this->SetOptionIfNotSet("CPACK_NSIS_INSTALLER_ICON_CODE",
                            installerIconCode.c_str());
    }
  this->ConfigureFile(nsisInInstallOptions.c_str(), 
                      nsisInstallOptions.c_str());
  this->ConfigureFile(nsisInFileName.c_str(), nsisFileName.c_str());
  std::string nsisCmd = "\"";
  nsisCmd += this->GetOption("CPACK_INSTALLER_PROGRAM");
  nsisCmd += "\" \"" + nsisFileName + "\"";
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << nsisCmd.c_str()
    << std::endl);
  std::string output;
  int retVal = 1;
  bool res = cmSystemTools::RunSingleCommand(nsisCmd.c_str(), &output,
    &retVal, 0, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << nsisCmd.c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running NSIS command: "
      << nsisCmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackNSISGenerator::InitializeInternal()
{
  if ( cmSystemTools::IsOn(this->GetOption(
        "CPACK_INCLUDE_TOPLEVEL_DIRECTORY")) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "NSIS Generator cannot work with CPACK_INCLUDE_TOPLEVEL_DIRECTORY. "
      "This option will be ignored."
      << std::endl);
    this->SetOption("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", 0);
    }

  cmCPackLogger(cmCPackLog::LOG_DEBUG, "cmCPackNSISGenerator::Initialize()"
    << std::endl);
  std::vector<std::string> path;
  std::string nsisPath;

#ifdef _WIN32
  if ( !cmsys::SystemTools::ReadRegistryValue(
      "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS", nsisPath) )
    {
    cmCPackLogger
      (cmCPackLog::LOG_ERROR, 
       "Cannot find NSIS registry value. This is usually caused by NSIS "
       "not being installed. Please install NSIS from "
       "http://nsis.sourceforge.org"
       << std::endl);
    return 0;
    }
  path.push_back(nsisPath);
#endif
  nsisPath = cmSystemTools::FindProgram("makensis", path, false);
  if ( nsisPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find NSIS compiler"
      << std::endl);
    return 0;
    }
  std::string nsisCmd = "\"" + nsisPath + "\" " NSIS_OPT "VERSION";
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Test NSIS version: "
    << nsisCmd.c_str() << std::endl);
  std::string output;
  int retVal = 1;
  bool resS = cmSystemTools::RunSingleCommand(nsisCmd.c_str(),
    &output, &retVal, 0, this->GeneratorVerbose, 0);

  cmsys::RegularExpression versionRex("v([0-9]+.[0-9]+)");
  if ( !resS || retVal || !versionRex.find(output))
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/NSISOutput.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << nsisCmd.c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem checking NSIS version with command: "
      << nsisCmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }
  double nsisVersion = atof(versionRex.match(1).c_str());
  double minNSISVersion = 2.09;
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "NSIS Version: "
    << nsisVersion << std::endl);
  if ( nsisVersion < minNSISVersion )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "CPack requires NSIS Version 2.09 or greater. NSIS found on the system "
      "was: "
      << nsisVersion << std::endl);
    return 0;
    }

  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", nsisPath.c_str());
  const char* cpackPackageExecutables
    = this->GetOption("CPACK_PACKAGE_EXECUTABLES");
  if ( cpackPackageExecutables )
    {
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "The cpackPackageExecutables: "
      << cpackPackageExecutables << "." << std::endl);
    cmOStringStream str;
    cmOStringStream deleteStr;
    std::vector<std::string> cpackPackageExecutablesVector;
    cmSystemTools::ExpandListArgument(cpackPackageExecutables,
      cpackPackageExecutablesVector);
    if ( cpackPackageExecutablesVector.size() % 2 != 0 )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "CPACK_PACKAGE_EXECUTABLES should contain pairs of <executable> and "
        "<icon name>." << std::endl);
      return 0;
      }
    std::vector<std::string>::iterator it;
    for ( it = cpackPackageExecutablesVector.begin();
      it != cpackPackageExecutablesVector.end();
      ++it )
      {
      std::string execName = *it;
      ++ it;
      std::string linkName = *it;
      str << "  CreateShortCut \"$SMPROGRAMS\\$STARTMENU_FOLDER\\"
        << linkName << ".lnk\" \"$INSTDIR\\bin\\" << execName << ".exe\""
        << std::endl;
      deleteStr << "  Delete \"$SMPROGRAMS\\$MUI_TEMP\\" << linkName
        << ".lnk\"" << std::endl;
      // see if CPACK_CREATE_DESKTOP_LINK_ExeName is on
      // if so add a desktop link
      std::string desktop = "CPACK_CREATE_DESKTOP_LINK_";
      desktop += execName;
      if(this->IsSet(desktop.c_str()))
        {
        str << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
        str << "    CreateShortCut \"$DESKTOP\\"
            << linkName << ".lnk\" \"$INSTDIR\\bin\\" << execName << ".exe\""
            << std::endl;
        deleteStr << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
        deleteStr << "    Delete \"$DESKTOP\\" << linkName
                  << ".lnk\"" << std::endl;
        }
      }
    this->CreateMenuLinks(str, deleteStr);
    this->SetOptionIfNotSet("CPACK_NSIS_CREATE_ICONS", str.str().c_str());
    this->SetOptionIfNotSet("CPACK_NSIS_DELETE_ICONS", 
                            deleteStr.str().c_str());
    }
  this->SetOptionIfNotSet("CPACK_NSIS_COMPRESSOR", "lzma");

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
void cmCPackNSISGenerator::CreateMenuLinks( cmOStringStream& str,
                                            cmOStringStream& deleteStr)
{
  const char* cpackMenuLinks
    = this->GetOption("CPACK_NSIS_MENU_LINKS");
  if(!cpackMenuLinks)
    {
    return;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "The cpackMenuLinks: "
                << cpackMenuLinks << "." << std::endl);
  std::vector<std::string> cpackMenuLinksVector;
  cmSystemTools::ExpandListArgument(cpackMenuLinks,
                                    cpackMenuLinksVector);
  if ( cpackMenuLinksVector.size() % 2 != 0 )
    {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "CPACK_PACKAGE_EXECUTABLES should contain pairs of <executable> and "
      "<icon name>." << std::endl);
    return;
    }
  std::vector<std::string>::iterator it;
  for ( it = cpackMenuLinksVector.begin();
        it != cpackMenuLinksVector.end();
        ++it )
    {
    std::string sourceName = *it;
    bool url = false;
    if(sourceName.find("http:") == 0)
      {
      url = true;
      }
    /* convert / to \\ */
    if(!url)
      {
      cmSystemTools::ReplaceString(sourceName, "/", "\\");
      }
    ++ it;
    std::string linkName = *it;
    if(!url)
      {
      str << "  CreateShortCut \"$SMPROGRAMS\\$STARTMENU_FOLDER\\"
          << linkName << ".lnk\" \"$INSTDIR\\" << sourceName << "\""
          << std::endl;
      deleteStr << "  Delete \"$SMPROGRAMS\\$MUI_TEMP\\" << linkName
                << ".lnk\"" << std::endl;
      }
    else
      {
      str << "  WriteINIStr \"$SMPROGRAMS\\$STARTMENU_FOLDER\\"
          << linkName << ".url\" \"InternetShortcut\" \"URL\" \"" 
          << sourceName << "\""
          << std::endl;
      deleteStr << "  Delete \"$SMPROGRAMS\\$MUI_TEMP\\" << linkName
                << ".url\"" << std::endl;
      }
    // see if CPACK_CREATE_DESKTOP_LINK_ExeName is on
    // if so add a desktop link
    std::string desktop = "CPACK_CREATE_DESKTOP_LINK_";
    desktop += linkName;
    if(this->IsSet(desktop.c_str()))
      {
      str << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
      str << "    CreateShortCut \"$DESKTOP\\"
          << linkName << ".lnk\" \"$INSTDIR\\" << sourceName << "\""
          << std::endl;
      deleteStr << "  StrCmp \"$INSTALL_DESKTOP\" \"1\" 0 +2\n";
      deleteStr << "    Delete \"$DESKTOP\\" << linkName
                << ".lnk\"" << std::endl;
      }
    }
}

//----------------------------------------------------------------------
bool cmCPackNSISGenerator::GetListOfSubdirectories(const char* topdir,
  std::vector<std::string>& dirs)
{
  cmsys::Directory dir;
  dir.Load(topdir);
  size_t fileNum;
  for (fileNum = 0; fileNum <  dir.GetNumberOfFiles(); ++fileNum)
    {
    if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
        strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
      {
      cmsys_stl::string fullPath = topdir;
      fullPath += "/";
      fullPath += dir.GetFile(static_cast<unsigned long>(fileNum));
      if(cmsys::SystemTools::FileIsDirectory(fullPath.c_str()) &&
        !cmsys::SystemTools::FileIsSymlink(fullPath.c_str()))
        {
        if (!this->GetListOfSubdirectories(fullPath.c_str(), dirs))
          {
          return false;
          }
        }
      }
    }
  dirs.push_back(topdir);
  return true;
}
