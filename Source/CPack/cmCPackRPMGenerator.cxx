/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCPackRPMGenerator.h"
#include "cmCPackLog.h"

//----------------------------------------------------------------------
cmCPackRPMGenerator::cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
cmCPackRPMGenerator::~cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::CompressFiles(const char* /*outFileName*/,
  const char* /*toplevel*/,
  const std::vector<std::string>& /*files*/)
{
  this->ReadListFile("CPackRPM.cmake");
  if (!this->IsSet("RPMBUILD_EXECUTABLE")) 
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find rpmbuild" << std::endl);
    return 0;
    }
  return 1;
}


