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
#include "cmCPackDebGenerator.h"

#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>

//----------------------------------------------------------------------
cmCPackDebGenerator::cmCPackDebGenerator()
{
}

//----------------------------------------------------------------------
cmCPackDebGenerator::~cmCPackDebGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackDebGenerator::CompressFiles(const char* outFileName,
  const char* toplevel,
  const std::vector<std::string>& files)
{
  const char* arExecutable = this->GetOption("AR_EXECUTABLE");
  const char* cmakeExecutable = this->GetOption("CMAKE_COMMAND");
  return 1;
}

//----------------------------------------------------------------------
int cmCPackDebGenerator::InitializeInternal()
{
  this->ReadListFile("CPackDeb.cmake");
  if (!this->IsSet("AR_EXECUTABLE")) 
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find ar" << std::endl);
    return 0;
    }
  return this->Superclass::InitializeInternal();
}

