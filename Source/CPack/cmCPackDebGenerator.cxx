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

  // debian-binary file
  std::string dbfilename;
  dbfilename = toplevel;
  dbfilename += "/debian-binary";
    { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(dbfilename.c_str());
    out << "2.0";
    out << std::endl; // required for valid debian package
    }

  // control file
  std::string ctlfilename;
  ctlfilename = toplevel;
  ctlfilename += "/control";

  // debian policy enforce lower case for package name
  std::string debian_pkg_name = cmsys::SystemTools::LowerCase( this->GetOption("DEBIAN_PACKAGE_NAME") );
  const char* debian_pkg_version = this->GetOption("DEBIAN_PACKAGE_VERSION");
  const char* debian_pkg_arch = this->GetOption("DEBIAN_PACKAGE_ARCHITECTURE");
  const char* debian_pkg_dep  = this->GetOption("DEBIAN_PACKAGE_DEPENDS");
  const char* maintainer = this->GetOption("DEBIAN_PACKAGE_MAINTAINER");
  const char* desc = this->GetOption("DEBIAN_PACKAGE_DESCRIPTION");

    { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(ctlfilename.c_str());
    out << "Package: " << debian_pkg_name << "\n";
    out << "Version: " << debian_pkg_version << "\n";
    out << "Section: devel\n";
    out << "Priority: optional\n";
    out << "Architecture: " << debian_pkg_arch << "\n";
    out << "Depends: " << debian_pkg_dep << " \n";
    out << "Maintainer: " << maintainer << "\n";
    out << "Description: " << desc << "\n";
    out << " " << debian_pkg_name << " was packaged by CMake.\n";
    out << std::endl;
    }

  std::string cmd = cmakeExecutable;
  cmd += " -E tar cfz data.tar.gz ./usr";
  std::string output;
  int retVal = -1;
  int res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);

  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/Deb.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << cmd.c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running tar command: "
      << cmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  std::string md5filename;
  md5filename = toplevel;
  md5filename += "/md5sums";

    { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(md5filename.c_str());
    std::vector<std::string>::const_iterator fileIt;
    for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
      {
      cmd = cmakeExecutable;
      cmd += " -E md5sum ";
      cmd += *fileIt;
      //std::string output;
      //int retVal = -1;
      res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
        &retVal, toplevel, this->GeneratorVerbose, 0);
      out << output;
      }
    out << std::endl;
    }


  cmd = cmakeExecutable;
  cmd += " -E tar cfz control.tar.gz ./control ./md5sums";
  res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);

  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/Deb.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << cmd.c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running tar command: "
      << cmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  // ar -r your-package-name.deb debian-binary control.tar.gz data.tar.gz
  cmd = arExecutable;
  cmd += " -r \"";
  cmd += outFileName;
  cmd += "\" debian-binary control.tar.gz data.tar.gz";
  res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);

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

