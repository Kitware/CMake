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

#include "cmCPackCygwinSourceGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>

// Includes needed for implementation of RenameFile.  This is not in
// system tools because it is not implemented robustly enough to move
// files across directories.
#ifdef _WIN32
# include <windows.h>
# include <sys/stat.h>
#endif

//----------------------------------------------------------------------
cmCPackCygwinSourceGenerator::cmCPackCygwinSourceGenerator()
{
  this->Compress = false;
}

//----------------------------------------------------------------------
cmCPackCygwinSourceGenerator::~cmCPackCygwinSourceGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackCygwinSourceGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "0");
  std::vector<std::string> path;
  std::string pkgPath = cmSystemTools::FindProgram("bzip2", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find BZip2" << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Found Compress program: "
    << pkgPath.c_str()
    << std::endl);

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackCygwinSourceGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  std::string packageDirFileName
    = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  packageDirFileName += ".tar";
  std::string output;
  int retVal = -1;
  if ( !this->Superclass::CompressFiles(packageDirFileName.c_str(),
      toplevel, files) )
    {
    return 0;
    }
  cmOStringStream dmgCmd1;
  dmgCmd1 << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
    << "\" \"" << packageDirFileName
    << "\"";
  retVal = -1;
  int res = cmSystemTools::RunSingleCommand(dmgCmd1.str().c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/CompressBZip2.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd1.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running BZip2 command: "
      << dmgCmd1.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }
  std::string compressOutFile = packageDirFileName + ".bz2";
  // at this point compressOutFile is the full path to 
  // _CPack_Package/.../package-2.5.0.tar.bz2
  // we want to create a tar _CPack_Package/.../package-2.5.0-1-src.tar.bz2
  // with these 
  //   _CPack_Package/.../package-2.5.0-1.patch 
  //   _CPack_Package/.../package-2.5.0-1.sh
  //   _CPack_Package/.../package-2.5.0.tar.bz2
  // the -1 is CPACK_CYGWIN_PATCH_NUMBER
  if(!cmSystemTools::CopyFileIfDifferent(
       this->GetOption("CPACK_CYGWIN_PATCH_FILE"),
       this->GetOption("CPACK_TOPLEVEL_DIRECTORY")))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "problem copying: ["
                  << this->GetOption("CPACK_CYGWIN_PATCH_FILE") << "]\nto\n["
                  << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "]\n");
    return 0;
    }
  if(!cmSystemTools::CopyFileIfDifferent(
       this->GetOption("CPACK_CYGWIN_BUILD_SCRIPT"),
       this->GetOption("CPACK_TOPLEVEL_DIRECTORY")))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "problem copying: "
                  << this->GetOption("CPACK_CYGWIN_BUILD_SCRIPT") << "\nto\n"
                  << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "]\n");
    return 0;
    }
                                     
  std::string outerTarFile
    = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  outerTarFile += "-";
  outerTarFile += this->GetOption("CPACK_CYGWIN_PATCH_NUMBER");
  outerTarFile += "-src.tar";
  std::string buildScript = cmSystemTools::GetFilenameName(
    this->GetOption("CPACK_CYGWIN_BUILD_SCRIPT"));
  std::string patchFile = cmSystemTools::GetFilenameName(
    this->GetOption("CPACK_CYGWIN_PATCH_FILE"));
  std::vector<cmStdString> outerFiles;
  std::string file = cmSystemTools::GetFilenameName(compressOutFile);
  std::string path = cmSystemTools::GetFilenamePath(compressOutFile);
  // a source release in cygwin should have the build script used
  // to build the package, the patch file that is different from the
  // regular upstream version of the sources, and a bziped tar file
  // of the original sources
  outerFiles.push_back(buildScript);
  outerFiles.push_back(patchFile);
  outerFiles.push_back(file);
  std::string saveDir= cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(path.c_str());
  cmSystemTools::CreateTar(outerTarFile.c_str(),
                           outerFiles, false, false);
  cmSystemTools::ChangeDirectory(saveDir.c_str());
  if(!this->BZip2File(outerTarFile.c_str()))
    {
    return 0;
    }
  compressOutFile = outerTarFile;
  compressOutFile += ".bz2";
  // now rename the file to its final name
  if ( !cmSystemTools::SameFile(compressOutFile.c_str(), outFileName ) )
    {
    if ( !this->RenameFile(compressOutFile.c_str(), outFileName) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem renaming: \""
        << compressOutFile.c_str() << "\" to \""
        << (outFileName ? outFileName : "(NULL)") << std::endl);
      return 0;
      }
    }

  return 1;
}

const char* cmCPackCygwinSourceGenerator::GetInstallPrefix()
{
  this->InstallPrefix = "/";
  this->InstallPrefix += this->GetOption("CPACK_PACKAGE_FILE_NAME");
  return this->InstallPrefix.c_str();
}

const char* cmCPackCygwinSourceGenerator::GetOutputExtension()
{
  this->OutputExtension = "-";
  this->OutputExtension += this->GetOption("CPACK_CYGWIN_PATCH_NUMBER");
  this->OutputExtension += "-src.tar.bz2";
  return this->OutputExtension.c_str();
}
  
