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
#include "cmDependsC.h"

#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmDependsC::cmDependsC(const char* dir, const char* targetFile):
  cmDepends(dir, targetFile),
  m_SourceFile(),
  m_IncludePath(0),
  m_IncludeRegexLine(),
  m_IncludeRegexScan(),
  m_IncludeRegexComplain()
{
}

//----------------------------------------------------------------------------
cmDependsC::cmDependsC(const char* dir, const char* targetFile,
                       const char* sourceFile,
                       std::vector<std::string> const& includes,
                       const char* scanRegex, const char* complainRegex):
  cmDepends(dir, targetFile),
  m_SourceFile(sourceFile),
  m_IncludePath(&includes),
  m_IncludeRegexLine("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]"),
  m_IncludeRegexScan(scanRegex),
  m_IncludeRegexComplain(complainRegex)
{
}

//----------------------------------------------------------------------------
cmDependsC::~cmDependsC()
{
}

//----------------------------------------------------------------------------
bool cmDependsC::WriteDependencies(std::ostream& os)
{
  // Make sure this is a scanning instance.
  if(m_SourceFile == "")
    {
    cmSystemTools::Error("Cannot scan dependencies without an source file.");
    return false;
    }
  if(!m_IncludePath)
    {
    cmSystemTools::Error("Cannot scan dependencies without an include path.");
    return false;
    }

  // Walk the dependency graph starting with the source file.
  bool first = true;
  m_Unscanned.push(m_SourceFile);
  m_Encountered.clear();
  m_Encountered.insert(m_SourceFile);
  std::set<cmStdString> dependencies;
  std::set<cmStdString> scanned;
  while(!m_Unscanned.empty())
    {
    // Get the next file to scan.
    std::string fname = m_Unscanned.front();
    m_Unscanned.pop();

    // If not a full path, find the file in the include path.
    std::string fullName;
    if(first || cmSystemTools::FileIsFullPath(fname.c_str()))
      {
      if(cmSystemTools::FileExists(fname.c_str()))
        {
        fullName = fname;
        }
      }
    else
      {
      for(std::vector<std::string>::const_iterator i = m_IncludePath->begin();
          i != m_IncludePath->end(); ++i)
        {
        std::string temp = *i;
        temp += "/";
        temp += fname;
        if(cmSystemTools::FileExists(temp.c_str()))
          {
          fullName = temp;
          break;
          }
        }
      }

    // Complain if the file cannot be found and matches the complain
    // regex.
    if(fullName.empty() && m_IncludeRegexComplain.find(fname.c_str()))
      {
      cmSystemTools::Error("Cannot find file \"", fname.c_str(), "\".");
      return false;
      }

    // Scan the file if it was found and has not been scanned already.
    if(!fullName.empty() && (scanned.find(fullName) == scanned.end()))
      {
      // Record scanned files.
      scanned.insert(fullName);

      // Try to scan the file.  Just leave it out if we cannot find
      // it.
      std::ifstream fin(fullName.c_str());
      if(fin)
        {
        // Add this file as a dependency.
        dependencies.insert(fullName);

        // Scan this file for new dependencies.
        this->Scan(fin);
        }
      }

    first = false;
    }

  // Write the dependencies to the output stream.
  for(std::set<cmStdString>::iterator i=dependencies.begin();
      i != dependencies.end(); ++i)
    {
    os << m_TargetFile.c_str() << ": "
       << cmSystemTools::ConvertToOutputPath(i->c_str()).c_str()
       << std::endl;
    }
  os << std::endl;

  return true;
}

//----------------------------------------------------------------------------
bool cmDependsC::CheckDependencies(std::istream& is)
{
  // Parse dependencies from the stream.  If any dependee is missing
  // or newer than the depender then dependencies should be
  // regenerated.
  bool okay = true;
  std::string line;
  std::string depender;
  std::string dependee;
  while(cmSystemTools::GetLineFromStream(is, line))
    {
    // Skip empty lines and comments.
    std::string::size_type pos = line.find_first_not_of(" \t\r\n");
    if(pos == std::string::npos || line[pos] == '#')
      {
      continue;
      }

    // Strip leading whitespace.
    if(pos > 0)
      {
      line = line.substr(pos);
      }

    // Skip lines too short to have a dependency.
    if(line.size() < 3)
      {
      continue;
      }

    // Find the colon on the line.  Skip the first two characters to
    // avoid finding the colon in a drive letter on Windows.  Ignore
    // the line if a colon cannot be found.
    if((pos = line.find(':', 2)) == std::string::npos)
      {
      continue;
      }

    // Split the line into depender and dependee.
    depender = line.substr(0, pos);
    dependee = line.substr(pos+1);

    // Strip whitespace from the dependee.
    if((pos = dependee.find_first_not_of(" \t\r\n")) != std::string::npos &&
       pos > 0)
      {
      dependee = dependee.substr(pos);
      }
    if((pos = dependee.find_last_not_of(" \t\r\n")) != std::string::npos)
      {
      dependee = dependee.substr(0, pos+1);
      }

    // Convert dependee to a full path.
    if(!cmSystemTools::FileIsFullPath(dependee.c_str()))
      {
      dependee = cmSystemTools::CollapseFullPath(dependee.c_str(),
                                                 m_Directory.c_str());
      }

    // Strip whitespace from the depender.
    if((pos = depender.find_last_not_of(" \t\r\n")) != std::string::npos)
      {
      depender = depender.substr(0, pos+1);
      }

    // Convert depender to a full path.
    if(!cmSystemTools::FileIsFullPath(depender.c_str()))
      {
      depender = cmSystemTools::CollapseFullPath(depender.c_str(),
                                                 m_Directory.c_str());
      }

    // Dependencies must be regenerated if the dependee does not exist
    // or if the depender exists and is older than the dependee.
    int result = 0;
    if(!cmSystemTools::FileExists(dependee.c_str()) ||
       (cmSystemTools::FileExists(depender.c_str()) &&
         (!cmSystemTools::FileTimeCompare(depender.c_str(), dependee.c_str(),
                                          &result) || result < 0)))
      {
      // Dependencies must be regenerated.
      okay = false;

      // Remove the depender to be sure it is rebuilt.
      cmSystemTools::RemoveFile(depender.c_str());
      }
    }

  return okay;
}

//----------------------------------------------------------------------------
void cmDependsC::Scan(std::istream& is)
{
  // Read one line at a time.
  std::string line;
  while(cmSystemTools::GetLineFromStream(is, line))
    {
    // Match include directives.
    if(m_IncludeRegexLine.find(line.c_str()))
      {
      // Get the file being included.
      std::string includeFile = m_IncludeRegexLine.match(1);

      // Queue the file if it has not yet been encountered and it
      // matches the regular expression for recursive scanning.
      if(m_Encountered.find(includeFile) == m_Encountered.end() &&
         m_IncludeRegexScan.find(includeFile.c_str()))
        {
        m_Encountered.insert(includeFile);
        m_Unscanned.push(includeFile);
        }
      }
    }
}
