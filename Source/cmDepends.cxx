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

//----------------------------------------------------------------------------
cmDepends::cmDepends(const char* dir, const char* targetFile):
  m_Directory(dir),
  m_TargetFile(targetFile),
  m_DependsMakeFile(dir),
  m_DependsMarkFile(dir)
{
  // Construct the path to the make and mark files.  Append
  // appropriate extensions to their names.
  m_DependsMakeFile += "/";
  m_DependsMarkFile += "/";
  m_DependsMakeFile += m_TargetFile;
  m_DependsMarkFile += m_TargetFile;
  m_DependsMakeFile += ".depends.make";
  m_DependsMarkFile += ".depends";
}

//----------------------------------------------------------------------------
cmDepends::~cmDepends()
{
}

//----------------------------------------------------------------------------
void cmDepends::Write()
{
  // Try to generate dependencies for the target file.
  cmGeneratedFileStream fout(m_DependsMakeFile.c_str());
  fout << "# Dependencies for " << m_TargetFile.c_str() << std::endl;
  if(this->WriteDependencies(fout) && fout)
    {
    // Dependencies were generated.  Touch the mark file.
    std::ofstream fmark(m_DependsMarkFile.c_str());
    fmark << "Dependencies updated for " << m_TargetFile.c_str() << std::endl;
    }
}

//----------------------------------------------------------------------------
void cmDepends::Check()
{
  // Check whether dependencies must be regenerated.
  std::ifstream fin(m_DependsMakeFile.c_str());
  if(!(fin && this->CheckDependencies(fin)))
    {
    // Clear all dependencies so they will be regenerated.
    this->Clear();
    }
}

//----------------------------------------------------------------------------
void cmDepends::Clear()
{
  // Remove the dependency mark file to be sure dependencies will be
  // regenerated.
  cmSystemTools::RemoveFile(m_DependsMarkFile.c_str());

  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(m_DependsMakeFile.c_str());
  depFileStream
    << "# Empty dependencies file for " << m_TargetFile.c_str() << ".\n"
    << "# This may be replaced when dependencies are built." << std::endl;
}

//----------------------------------------------------------------------------
const char* cmDepends::GetMakeFileName()
{
  // Skip over the directory part of the name.
  return m_DependsMakeFile.c_str() + m_Directory.length() + 1;
}

//----------------------------------------------------------------------------
const char* cmDepends::GetMarkFileName()
{
  // Skip over the directory part of the name.
  return m_DependsMarkFile.c_str() + m_Directory.length() + 1;
}
