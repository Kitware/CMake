/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackZIPGenerator.h"

#include "cmSystemTools.h"
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
  this->ReadListFile("CPackZIP.cmake");
  if ((!this->IsSet("ZIP_EXECUTABLE")) 
      || (!this->IsSet("CPACK_ZIP_COMMAND")))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find a suitable ZIP program"
      << std::endl);
    return 0;
    }
  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackZIPGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  std::string tempFileName;
  tempFileName = toplevel;
  tempFileName += "/winZip.filelist";
  bool needQuotesInFile = cmSystemTools::IsOn(
    this->GetOption("CPACK_ZIP_NEED_QUOTES"));

  std::string cmd = this->GetOption("CPACK_ZIP_COMMAND");
  cmsys::SystemTools::ReplaceString(cmd, "<ARCHIVE>", outFileName);
  cmsys::SystemTools::ReplaceString(cmd, "<FILELIST>", "winZip.filelist");

  { // the scope is needed for cmGeneratedFileStream
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


  std::string output;
  int retVal = -1;
  int res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);

  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/CompressZip.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << cmd.c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running zip command: "
      << cmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }
  return 1;
}
