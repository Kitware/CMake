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
#include "cmFileTimeComparison.h"

#include <assert.h>

//----------------------------------------------------------------------------
cmDepends::cmDepends(): m_Verbose(false), m_FileComparison(0),
  m_MaxPath(cmSystemTools::GetMaximumFilePathLength()),
  m_Dependee(new char[m_MaxPath]),
  m_Depender(new char[m_MaxPath])
{
}

//----------------------------------------------------------------------------
cmDepends::~cmDepends()
{
  delete [] m_Dependee;
  delete [] m_Depender;
}

//----------------------------------------------------------------------------
bool cmDepends::Write(const char *src, const char *obj,
  std::ostream &makeDepends, std::ostream &internalDepends)
{
  return this->WriteDependencies(src, obj, makeDepends, internalDepends);
}

//----------------------------------------------------------------------------
void cmDepends::Check(const char *makeFile, const char *internalFile)
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
  std::ifstream fin(internalFile);
  if(!(fin && this->CheckDependencies(fin)))
    {
    // Clear all dependencies so they will be regenerated.
    this->Clear(makeFile);
    this->Clear(internalFile);
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
  std::cout << "Remove mark file: " << markFile.c_str() << std::endl;
  
  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(file);
  depFileStream
    << "# Empty dependencies file\n"
    << "# This may be replaced when dependencies are built." << std::endl;
}

//----------------------------------------------------------------------------
bool cmDepends::CheckDependencies(std::istream& internalDepends)
{
  // Parse dependencies from the stream.  If any dependee is missing
  // or newer than the depender then dependencies should be
  // regenerated.
  bool okay = true;
  while(internalDepends.getline(m_Dependee, m_MaxPath))
    {
    if ( m_Dependee[0] == 0 || m_Dependee[0] == '#' || m_Dependee[0] == '\r' )
      {
      continue;
      }
    size_t len = internalDepends.gcount()-1;
    if ( m_Dependee[len-1] == '\r' )
      {
      len --;
      m_Dependee[len] = 0;
      }
    if ( m_Dependee[0] != ' ' )
      {
      memcpy(m_Depender, m_Dependee, len+1);
      continue;
      }
    /*
    // Parse the dependency line.
    if(!this->ParseDependency(line.c_str()))
      {
      continue;
      }
      */

    // Dependencies must be regenerated if the dependee does not exist
    // or if the depender exists and is older than the dependee.
    bool regenerate = false;
    const char* dependee = m_Dependee+1;
    const char* depender = m_Depender;
    if(!cmSystemTools::FileExists(dependee))
      {
      // The dependee does not exist.
      regenerate = true;

      // Print verbose output.
      if(m_Verbose)
        {
        cmOStringStream msg;
        msg << "Dependee \"" << dependee
            << "\" does not exist for depender \""
            << depender << "\"." << std::endl;
        cmSystemTools::Stdout(msg.str().c_str());
        }
      }
    else if(cmSystemTools::FileExists(depender))
      {
      // The dependee and depender both exist.  Compare file times.
      int result = 0;
      if((!m_FileComparison->FileTimeCompare(depender, dependee,
                                             &result) || result < 0))
        {
        // The depender is older than the dependee.
        regenerate = true;

        // Print verbose output.
        if(m_Verbose)
          {
          cmOStringStream msg;
          msg << "Dependee \"" << dependee
              << "\" is newer than depender \""
              << depender << "\"." << std::endl;
          cmSystemTools::Stdout(msg.str().c_str());
          }
        }
      }
    if(regenerate)
      {
      // Dependencies must be regenerated.
      okay = false;

      // Remove the depender to be sure it is rebuilt.
      cmSystemTools::RemoveFile(depender);
      }
    }

  return okay;
}


