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

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::cmCPackPackageMakerGenerator()
{
}

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::~cmCPackPackageMakerGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::ProcessGenerator()
{
  return this->Superclass::ProcessGenerator();
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::CompressFiles(const char* outFileName, const char* toplevel,
  const std::vector<std::string>& files)
{
  // Create directory structure
  std::string resDir = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  resDir += "/Resources";
  std::string preflightDirName = resDir + "/PreFlight";
  std::string postflightDirName = resDir + "/PostFlight";
  
  if ( !cmsys::SystemTools::MakeDirectory(preflightDirName.c_str())
    || !cmsys::SystemTools::MakeDirectory(postflightDirName.c_str()) )
    {
    std::cerr << "Problem creating installer directories: " << preflightDirName.c_str() << " and " << postflightDirName.c_str() << std::endl;
    return 0;
    }

  if ( !this->CopyCreateResourceFile("License")
    || !this->CopyCreateResourceFile("ReadMe")
    || !this->CopyCreateResourceFile("Welcome") 
    || !this->CopyResourcePlistFile("Info.plist")
    || !this->CopyResourcePlistFile("Description.plist") )
    {
    std::cerr << "Problem copying the resource files" << std::endl;
    return 0;
    }

  std::string packageDirFileName = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  packageDirFileName += ".pkg";

  std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/PackageMakerOutput.log";
  cmOStringStream pkgCmd;
  /*
  pkgCmd << "sh -c '\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
  << "\" -build -p \"" << packageDirFileName << "\" -f \"" << this->GetOption("CPACK_TEMPORARY_DIRECTORY")
  << "\" -r \"" << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/Resources\" -i \""
  << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/Info.plist\" -d \""
  << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/Description.plist\"'";
  */
  pkgCmd << "/Users/kitware/Andy/CMake-CPack/foo.sh";
  std::cout << "Execute: " << pkgCmd.str().c_str() << std::endl;
  std::string output;
  int retVal = 1;
  //bool res = cmSystemTools::RunSingleCommand(pkgCmd.str().c_str(), &output, &retVal, 0, m_GeneratorVerbose, 0);
  bool res = true;
  retVal = system(pkgCmd.str().c_str());
  std::cout << "Done running package maker" << std::endl;
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << pkgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    std::cerr << "Problem running PackageMaker command: " << pkgCmd.str().c_str() << std::endl;
    std::cerr << "Please check " << tmpFile.c_str() << " for errors" << std::endl;
    return 0;
    }

  tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/hdiutilOutput.log";
  cmOStringStream dmgCmd;
  dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM_DISK_IMAGE") << "\" create -ov -format UDZO -srcfolder \"" << packageDirFileName
    << "\" \"" << outFileName << "\"";
  res = cmSystemTools::RunSingleCommand(dmgCmd.str().c_str(), &output, &retVal, 0, m_GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    std::cerr << "Problem running hdiutil command: " << dmgCmd.str().c_str() << std::endl;
    std::cerr << "Please check " << tmpFile.c_str() << " for errors" << std::endl;
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::Initialize(const char* name)
{
  std::cout << "cmCPackPackageMakerGenerator::Initialize()" << std::endl;
  int res = this->Superclass::Initialize(name);
  std::vector<std::string> path;
  std::string pkgPath = "/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS";
  path.push_back(pkgPath);
  pkgPath = cmSystemTools::FindProgram("PackageMaker", path, false);
  if ( pkgPath.empty() )
    {
    std::cerr << "Cannot find PackageMaker compiler" << std::endl;
    return 0;
    }
  this->SetOption("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
  pkgPath = cmSystemTools::FindProgram("hdiutil", path, false);
  if ( pkgPath.empty() )
    {
    std::cerr << "Cannot find hdiutil compiler" << std::endl;
    return 0;
    }
  this->SetOption("CPACK_INSTALLER_PROGRAM_DISK_IMAGE", pkgPath.c_str());

  return res;
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::CopyCreateResourceFile(const char* name)
{
  std::string uname = cmSystemTools::UpperCase(name);
  std::string cpackVar = "CPACK_RESOURCE_FILE_" + uname;
  const char* inFileName = this->GetOption(cpackVar.c_str());
  if ( !inFileName )
    {
    std::cerr << "CPack option: " << cpackVar.c_str() << " not specified. It should point to " << name << ".rtf, " << name << ".html, or " << name << ".txt file" << std::endl;
    return false;
    }
  if ( !cmSystemTools::FileExists(inFileName) )
    {
    std::cerr << "Cannot find " << name << " resource file: " << inFileName << std::endl;
    return false;
    }
  std::string ext = cmSystemTools::GetFilenameLastExtension(inFileName);
  if ( ext != ".rtfd" && ext != ".rtf" && ext != ".html" && ext != ".txt" )
    {
    std::cerr << "Bad file extension specified: " << ext << ". Currently only .rtfd, .rtf, .html, and .txt files allowed." << std::endl;
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/Resources/";
  destFileName += name + ext;


  std::cout << "Configure file: " << inFileName << " to " << destFileName.c_str() << std::endl;
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
    std::cerr << "Cannot find input file: " << inFName << std::endl;
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/";
  destFileName += name;

  std::cout << "Configure file: " << inFileName.c_str() << " to " << destFileName.c_str() << std::endl;
  this->ConfigureFile(inFileName.c_str(), destFileName.c_str());
  return true;
}

