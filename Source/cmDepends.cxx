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
#include "cmDepends.h"

#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"

#include <assert.h>

//----------------------------------------------------------------------------
cmDepends::cmDepends()
{
  m_Verbose = false;
}

//----------------------------------------------------------------------------
cmDepends::~cmDepends()
{
}

//----------------------------------------------------------------------------
bool cmDepends::Write(const char *src, const char *obj, std::ostream &fout)
{
  return this->WriteDependencies(src, obj, fout);
}

//----------------------------------------------------------------------------
void cmDepends::Check(const char *file)
{
  // Dependency checks must be done in proper working directory.
  std::string oldcwd = ".";
  if(m_CompileDirectory != ".")
    {
    // Get the CWD but do not call CollapseFullPath because
    // we only need it to cd back, and the form does not matter
    oldcwd = cmSystemTools::GetCurrentWorkingDirectory(false);
    cmSystemTools::ChangeDirectory(m_CompileDirectory.c_str());
    }

  // Check whether dependencies must be regenerated.
  std::ifstream fin(file);
  if(!(fin && this->CheckDependencies(fin)))
    {
    // Clear all dependencies so they will be regenerated.
    this->Clear(file);
    }

  // Restore working directory.
  if(oldcwd != ".")
    {
    cmSystemTools::ChangeDirectory(oldcwd.c_str());
    }
}

//----------------------------------------------------------------------------
void cmDepends::Clear(const char *file)
{
  // Print verbose output.
  if(m_Verbose)
    {
    cmOStringStream msg;
    msg << "Clearing dependencies in \"" << file << "\"." << std::endl;
    cmSystemTools::Stdout(msg.str().c_str());
    }

  // Remove the dependency mark file to be sure dependencies will be
  // regenerated.
  std::string markFile = file;
  markFile += ".mark";
  cmSystemTools::RemoveFile(markFile.c_str());
  
  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(file);
  depFileStream
    << "# Empty dependencies file\n"
    << "# This may be replaced when dependencies are built." << std::endl;
}

