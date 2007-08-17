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
#include "cmCPackRPMGenerator.h"

#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>

//----------------------------------------------------------------------
cmCPackRPMGenerator::cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
cmCPackRPMGenerator::~cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::CompressFiles(const char* /*outFileName*/,
  const char* /*toplevel*/,
  const std::vector<std::string>& /*files*/)
{
  this->ReadListFile("CPackRPM.cmake");
//   const char* cmakeExecutable = this->GetOption("CMAKE_COMMAND");

  if (!this->IsSet("RPMBUILD_EXECUTABLE")) 
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find rpmbuild" << std::endl);
    return 0;
    }

  /* FIXME we should be able to stick with RPM naming scheme
   * and not following CMake naming scheme?
  const char* rpmFileName = this->GetOption("RPM_FILE_NAME");
  const char* rpmDirectory = this->GetOption("RPM_DIRECTORY");
  this->SetOption("CPACK_OUTPUT_FILE_NAME",rpmFileName);
  std::string rpmFilePath;
  rpmFilePath += rpmDirectory;
  rpmFilePath += "/";
  rpmFilePath += rpmFileName;
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME",rpmFilePath.c_str());
  */
  //  this->SetOption("CPACK_OUTPUT_FILE_PATH",rpmFilePath);
  
  //FIXME I think we should split CPackRPM.cmake into (at least)
  //      2 differents files

  return 1;
}


//----------------------------------------------------------------------
int cmCPackRPMGenerator::InitializeInternal()
{
  return this->Superclass::InitializeInternal();
}

