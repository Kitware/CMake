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
  // mandatory entries:
  std::string debian_pkg_name = 
       cmsys::SystemTools::LowerCase( this->GetOption("DEBIAN_PACKAGE_NAME") );
  const char* debian_pkg_version = this->GetOption("DEBIAN_PACKAGE_VERSION");
  const char* debian_pkg_section = this->GetOption("DEBIAN_PACKAGE_SECTION");
  const char* debian_pkg_priority = this->GetOption("DEBIAN_PACKAGE_PRIORITY");
  const char* debian_pkg_arch = this->GetOption("DEBIAN_PACKAGE_ARCHITECTURE");
  const char* maintainer = this->GetOption("DEBIAN_PACKAGE_MAINTAINER");
  const char* desc = this->GetOption("DEBIAN_PACKAGE_DESCRIPTION");

  // optional entries
  const char* debian_pkg_dep = this->GetOption("DEBIAN_PACKAGE_DEPENDS");
  const char* debian_pkg_rec = this->GetOption("DEBIAN_PACKAGE_RECOMMENDS");
  const char* debian_pkg_sug = this->GetOption("DEBIAN_PACKAGE_SUGGESTS");

    { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(ctlfilename.c_str());
    out << "Package: " << debian_pkg_name << "\n";
    out << "Version: " << debian_pkg_version << "\n";
    out << "Section: " << debian_pkg_section << "\n";
    out << "Priority: " << debian_pkg_priority << "\n";
    out << "Architecture: " << debian_pkg_arch << "\n";
    if(debian_pkg_dep)
      {
      out << "Depends: " << debian_pkg_dep << "\n";
      }
    if(debian_pkg_rec)
      {
      out << "Recommends: " << debian_pkg_rec << "\n";
      }
    if(debian_pkg_sug)
      {
      out << "Suggests: " << debian_pkg_sug << "\n";
      }
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
      << "# Working directory: " << toplevel << std::endl
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
    std::string topLevelWithTrailingSlash = toplevel;
    topLevelWithTrailingSlash += '/';
    for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
      {
      cmd = cmakeExecutable;
      cmd += " -E md5sum ";
      cmd += *fileIt;
      //std::string output;
      //int retVal = -1;
      res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
        &retVal, toplevel, this->GeneratorVerbose, 0);
      // debian md5sums entries are like this:
      // 014f3604694729f3bf19263bac599765  usr/bin/ccmake
      // thus strip the full path (with the trailing slash)
      cmSystemTools::ReplaceString(output, 
                                   topLevelWithTrailingSlash.c_str(), "");
      out << output;
      }
    // each line contains a eol. 
    // Do not end the md5sum file with yet another (invalid)
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
      << "# Working directory: " << toplevel << std::endl
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

  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/Deb.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << cmd.c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running ar command: "
      << cmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

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

