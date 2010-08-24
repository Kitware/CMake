/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackArchiveGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"
#include <errno.h>

#include <cmsys/SystemTools.hxx>
#include <cm_libarchive.h>

//----------------------------------------------------------------------
cmCPackArchiveGenerator::cmCPackArchiveGenerator(cmArchiveWrite::Compress t,
  cmArchiveWrite::Type at)
{
  this->Compress = t;
  this->Archive = at;
}

//----------------------------------------------------------------------
cmCPackArchiveGenerator::~cmCPackArchiveGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  return this->Superclass::InitializeInternal();
}

int cmCPackArchiveGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                << toplevel << std::endl);

  // Open binary stream
  cmGeneratedFileStream gf;
  gf.Open(packageFileNames[0].c_str(), false, true);
  // Add an eventual header to the archive
  if (!GenerateHeader(&gf))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem to generate Header for archive < "
        << packageFileNames[0]
        << ">." << std::endl);
    }
  // create a new archive
  cmArchiveWrite archive(gf,this->Compress, this->Archive);
  if (!archive)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem to create archive < "
                          << packageFileNames[0]
                          << ">. ERROR ="
                          << archive.GetError()
                          << std::endl);
    return 0;
    }
  std::vector<std::string>::const_iterator fileIt;
  std::string dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(toplevel.c_str());
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    // Get the relative path to the file
    std::string rp = cmSystemTools::RelativePath(toplevel.c_str(), fileIt->c_str());
    archive.Add(rp);
    if(!archive)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem while adding file< "
          << *fileIt
          << "> to archive <"
          << packageFileNames[0] << "> .ERROR ="
          << archive.GetError()
          << std::endl);
      return 0;
      }
    }
  cmSystemTools::ChangeDirectory(dir.c_str());
  // The destructor of cmArchiveWrite will close and finish the write
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::GenerateHeader(std::ostream*)
{
  return 1;
}
