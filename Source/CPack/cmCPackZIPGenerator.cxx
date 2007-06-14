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

#include "cmCPackZIPGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>

//----------------------------------------------------------------------
cmCPackZIPGenerator::cmCPackZIPGenerator()
{
}

//----------------------------------------------------------------------
cmCPackZIPGenerator::~cmCPackZIPGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackZIPGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  std::vector<std::string> path;
  std::string pkgPath = "c:/Program Files/WinZip";
  path.push_back(pkgPath);
  pkgPath = cmSystemTools::FindProgram("wzzip", path, false);
  this->ZipStyle = cmCPackZIPGenerator::StyleUnkown;
  bool found = false;
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Cannot find WinZip" << std::endl);
    }
  else
    {
    this->ZipStyle = cmCPackZIPGenerator::StyleWinZip;
    found = true;
    }

  if ( !found )
    {
    pkgPath = "c:/Program Files/7-Zip";
    path.push_back(pkgPath);
    pkgPath = cmSystemTools::FindProgram("7z", path, false);

    if ( pkgPath.empty() )
      {
      cmCPackLogger(cmCPackLog::LOG_DEBUG, "Cannot find 7ZIP"
        << std::endl);
      }
    else
      {
      this->ZipStyle = cmCPackZIPGenerator::Style7Zip;
      found = true;
      }
    }

  if ( !found )
    {
    path.erase(path.begin(), path.end());
    pkgPath = "c:/cygwin/bin";
    path.push_back(pkgPath);
    pkgPath = cmSystemTools::FindProgram("zip", path, false);
    if ( pkgPath.empty() )
      {
      cmCPackLogger(cmCPackLog::LOG_DEBUG, "Cannot find unix ZIP"
        << std::endl);
      }
    else
      {
      this->ZipStyle = cmCPackZIPGenerator::StyleUnixZip;
      found = true;
      }
    }
  if ( !found )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find a sutable ZIP program"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Found ZIP program: "
    << pkgPath.c_str()
    << std::endl);
  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackZIPGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  std::string tempFileName;
  tempFileName = toplevel;
  tempFileName += "/winZip.filelist";
  bool needQuotesInFile = false;
  cmOStringStream dmgCmd;
  switch ( this->ZipStyle )
    {
  case cmCPackZIPGenerator::StyleWinZip:
    dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
           << "\" -P \"" << outFileName
           << "\" @winZip.filelist";
    needQuotesInFile = true;
    break;
  case cmCPackZIPGenerator::Style7Zip:
    // this is the zip generator, so tell 7zip to generate zip files
    dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
           << "\" a -tzip \"" << outFileName
           << "\" @winZip.filelist";
    needQuotesInFile = true;
    break;
  case cmCPackZIPGenerator::StyleUnixZip:
    dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
      << "\" -r \"" << outFileName
      << "\" . -i@winZip.filelist";
    break;
  default:
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Unknown ZIP style"
      << std::endl);
    return 0;
    }
  if(tempFileName.size())
    {
    cmGeneratedFileStream out(tempFileName.c_str());
    std::vector<std::string>::const_iterator fileIt;
    for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
      {
      if ( needQuotesInFile )
        {
        out << "\"";
        }
      out << cmSystemTools::RelativePath(toplevel, fileIt->c_str());
      if ( needQuotesInFile )
        {
        out << "\"";
        }
      out << std::endl;
      }
    }
  else
    {
    std::vector<std::string>::const_iterator fileIt;
    for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
      {
      dmgCmd << " \""
             << cmSystemTools::RelativePath(toplevel, fileIt->c_str())
             << "\"";
      }
    }
  std::string output;
  int retVal = -1;
  int res = cmSystemTools::RunSingleCommand(dmgCmd.str().c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/CompressZip.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running zip command: "
      << dmgCmd.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }
  return 1;
}
