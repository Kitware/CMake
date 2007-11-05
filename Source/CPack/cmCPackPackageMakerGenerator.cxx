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
#include "cmCPackPackageMakerGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::cmCPackPackageMakerGenerator()
{
  this->PackageMakerVersion = 0.0;
}

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::~cmCPackPackageMakerGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::CompressFiles(const char* outFileName,
  const char* toplevel,
  const std::vector<std::string>& files)
{
  (void) files; // TODO: Fix api to not need files.
  (void) toplevel; // TODO: Use toplevel
  // Create directory structure
  std::string resDir = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  resDir += "/Resources";
  std::string preflightDirName = resDir + "/PreFlight";
  std::string postflightDirName = resDir + "/PostFlight";

  if ( !cmsys::SystemTools::MakeDirectory(preflightDirName.c_str())
    || !cmsys::SystemTools::MakeDirectory(postflightDirName.c_str()) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem creating installer directories: "
      << preflightDirName.c_str() << " and "
      << postflightDirName.c_str() << std::endl);
    return 0;
    }

  if ( !this->CopyCreateResourceFile("License")
    || !this->CopyCreateResourceFile("ReadMe")
    || !this->CopyCreateResourceFile("Welcome")
    || !this->CopyResourcePlistFile("Info.plist")
    || !this->CopyResourcePlistFile("Description.plist") )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying the resource files"
      << std::endl);
    return 0;
    }

  std::string packageDirFileName
    = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  packageDirFileName += ".pkg";

  std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/PackageMakerOutput.log";
  cmOStringStream pkgCmd;
  pkgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
  << "\" -build -p \"" << packageDirFileName << "\" -f \""
  << this->GetOption("CPACK_TEMPORARY_DIRECTORY")
  << "\" -r \"" << this->GetOption("CPACK_TOPLEVEL_DIRECTORY")
  << "/Resources\" -i \""
  << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/Info.plist\" -d \""
  << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/Description.plist\"";
  if ( this->PackageMakerVersion > 2.0 )
    {
    pkgCmd << " -v";
    }
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << pkgCmd.str().c_str()
    << std::endl);
  std::string output;
  int retVal = 1;
  //bool res = cmSystemTools::RunSingleCommand(pkgCmd.str().c_str(), &output,
  //&retVal, 0, this->GeneratorVerbose, 0);
  bool res = true;
  retVal = system(pkgCmd.str().c_str());
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Done running package maker"
    << std::endl);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << pkgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem running PackageMaker command: " << pkgCmd.str().c_str()
      << std::endl << "Please check " << tmpFile.c_str() << " for errors"
      << std::endl);
    return 0;
    }

  tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/hdiutilOutput.log";
  cmOStringStream dmgCmd;
  dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM_DISK_IMAGE")
    << "\" create -ov -format UDZO -srcfolder \"" << packageDirFileName
    << "\" \"" << outFileName << "\"";
  res = cmSystemTools::RunSingleCommand(dmgCmd.str().c_str(), &output,
    &retVal, 0, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running hdiutil command: "
      << dmgCmd.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::InitializeInternal()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "cmCPackPackageMakerGenerator::Initialize()" << std::endl);
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");
  std::vector<std::string> path;
  std::string pkgPath
    = "/Developer/Applications/Utilities/PackageMaker.app/Contents";
  std::string versionFile = pkgPath + "/version.plist";
  if ( !cmSystemTools::FileExists(versionFile.c_str()) )
    {
    pkgPath = "/Developer/Applications/PackageMaker.app/Contents";
    std::string newVersionFile = pkgPath + "/version.plist";
    if ( !cmSystemTools::FileExists(newVersionFile.c_str()) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Cannot find PackageMaker compiler version file: "
        << versionFile.c_str() << " or " << newVersionFile.c_str()
        << std::endl);
      return 0;
      }
    versionFile = newVersionFile;
    }
  std::ifstream ifs(versionFile.c_str());
  if ( !ifs )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot open PackageMaker compiler version file" << std::endl);
    return 0;
    }
  // Check the PackageMaker version
  cmsys::RegularExpression rexKey("<key>CFBundleShortVersionString</key>");
  cmsys::RegularExpression rexVersion("<string>([0-9]+.[0-9.]+)</string>");
  std::string line;
  bool foundKey = false;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    if ( rexKey.find(line) )
      {
      foundKey = true;
      break;
      }
    }
  if ( !foundKey )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot find CFBundleShortVersionString in the PackageMaker compiler "
      "version file" << std::endl);
    return 0;
    }
  if ( !cmSystemTools::GetLineFromStream(ifs, line) ||
    !rexVersion.find(line) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem reading the PackageMaker compiler version file: "
      << versionFile.c_str() << std::endl);
    return 0;
    }
  this->PackageMakerVersion = atof(rexVersion.match(1).c_str());
  if ( this->PackageMakerVersion < 1.0 )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Require PackageMaker 1.0 or higher"
      << std::endl);
    return 0;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "PackageMaker version is: "
    << this->PackageMakerVersion << std::endl);

  pkgPath += "/MacOS";
  path.push_back(pkgPath);
  pkgPath = cmSystemTools::FindProgram("PackageMaker", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find PackageMaker compiler"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
  pkgPath = cmSystemTools::FindProgram("hdiutil", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find hdiutil compiler"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM_DISK_IMAGE", 
                          pkgPath.c_str());

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::CopyCreateResourceFile(const char* name)
{
  std::string uname = cmSystemTools::UpperCase(name);
  std::string cpackVar = "CPACK_RESOURCE_FILE_" + uname;
  const char* inFileName = this->GetOption(cpackVar.c_str());
  if ( !inFileName )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "CPack option: " << cpackVar.c_str()
                  << " not specified. It should point to " 
                  << (name ? name : "(NULL)")
                  << ".rtf, " << name
                  << ".html, or " << name << ".txt file" << std::endl);
    return false;
    }
  if ( !cmSystemTools::FileExists(inFileName) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find " 
                  << (name ? name : "(NULL)")
                  << " resource file: " << inFileName << std::endl);
    return false;
    }
  std::string ext = cmSystemTools::GetFilenameLastExtension(inFileName);
  if ( ext != ".rtfd" && ext != ".rtf" && ext != ".html" && ext != ".txt" )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Bad file extension specified: "
      << ext << ". Currently only .rtfd, .rtf, .html, and .txt files allowed."
      << std::endl);
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/Resources/";
  destFileName += name + ext;


  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: " 
                << (inFileName ? inFileName : "(NULL)")
                << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName, destFileName.c_str());
  return true;
}

bool cmCPackPackageMakerGenerator::CopyResourcePlistFile(const char* name)
{
  std::string inFName = "CPack.";
  inFName += name;
  inFName += ".in";
  std::string inFileName = this->FindTemplate(inFName.c_str());
  if ( inFileName.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find input file: "
      << inFName << std::endl);
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/";
  destFileName += name;

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: "
    << inFileName.c_str() << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName.c_str(), destFileName.c_str());
  return true;
}
