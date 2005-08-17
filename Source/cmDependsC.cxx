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

#include <ctype.h> // isspace

//----------------------------------------------------------------------------
cmDependsC::cmDependsC():
  m_IncludePath(0), m_GeneratedFiles(0)
{
}

//----------------------------------------------------------------------------
// yummy look at all those constructor arguments
cmDependsC::cmDependsC(std::vector<std::string> const& includes,
                       const char* scanRegex, const char* complainRegex,
                       std::set<cmStdString> const& generatedFiles):
  m_IncludePath(&includes),
  m_IncludeRegexLine("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)([\">])"),
  m_IncludeRegexScan(scanRegex),
  m_IncludeRegexComplain(complainRegex),
  m_GeneratedFiles(&generatedFiles)
{
}

//----------------------------------------------------------------------------
cmDependsC::~cmDependsC()
{
}

//----------------------------------------------------------------------------
bool cmDependsC::WriteDependencies(const char *src, 
                                   const char *obj, std::ostream& os)
{
  // Make sure this is a scanning instance.
  if(!src || src[0] == '\0')
    {
    cmSystemTools::Error("Cannot scan dependencies without a source file.");
    return false;
    }
  if(!obj || obj[0] == '\0')
    {
    cmSystemTools::Error("Cannot scan dependencies without an object file.");
    return false;
    }
  if(!m_IncludePath)
    {
    cmSystemTools::Error("Cannot scan dependencies without an include path.");
    return false;
    }

  // Walk the dependency graph starting with the source file.
  bool first = true;
  UnscannedEntry root;
  root.FileName = src;
  m_Unscanned.push(root);
  m_Encountered.clear();
  m_Encountered.insert(src);
  std::set<cmStdString> dependencies;
  std::set<cmStdString> scanned;
  while(!m_Unscanned.empty())
    {
    // Get the next file to scan.
    UnscannedEntry current = m_Unscanned.front();
    m_Unscanned.pop();

    // If not a full path, find the file in the include path.
    std::string fullName;
    if(first || cmSystemTools::FileIsFullPath(current.FileName.c_str()))
      {
      if(this->FileExistsOrIsGenerated(current.FileName, scanned,
                                       dependencies))
        {
        fullName = current.FileName;
        }
      }
    else if(!current.QuotedLocation.empty() &&
            this->FileExistsOrIsGenerated(current.QuotedLocation, scanned,
                                          dependencies))
      {
      // The include statement producing this entry was a double-quote
      // include and the included file is present in the directory of
      // the source containing the include statement.
      fullName = current.QuotedLocation;
      }
    else
      {
      for(std::vector<std::string>::const_iterator i = m_IncludePath->begin();
          i != m_IncludePath->end(); ++i)
        {
        // Construct the name of the file as if it were in the current
        // include directory.  Avoid using a leading "./".
        std::string temp = *i;
        if(temp == ".")
          {
          temp = "";
          }
        else
          {
          temp += "/";
          }
        temp += current.FileName;

        // Look for the file in this location.
        if(this->FileExistsOrIsGenerated(temp, scanned, dependencies))
          {
          fullName = temp;
          break;
          }
        }
      }

    // Complain if the file cannot be found and matches the complain
    // regex.
    if(fullName.empty() &&
       m_IncludeRegexComplain.find(current.FileName.c_str()))
      {
      cmSystemTools::Error("Cannot find file \"",
                           current.FileName.c_str(), "\".");
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

        // Scan this file for new dependencies.  Pass the directory
        // containing the file to handle double-quote includes.
        std::string dir = cmSystemTools::GetFilenamePath(fullName);
        this->Scan(fin, dir.c_str());
        }
      }

    first = false;
    }

  // Write the dependencies to the output stream.
  for(std::set<cmStdString>::iterator i=dependencies.begin();
      i != dependencies.end(); ++i)
    {
    os << obj << ": "
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
    // Parse the dependency line.
    if(!this->ParseDependency(line.c_str(), depender, dependee))
      {
      continue;
      }

    // Dependencies must be regenerated if the dependee does not exist
    // or if the depender exists and is older than the dependee.
    bool regenerate = false;
    if(!cmSystemTools::FileExists(dependee.c_str()))
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
    else if(cmSystemTools::FileExists(depender.c_str()))
      {
      // The dependee and depender both exist.  Compare file times.
      int result = 0;
      if((!cmSystemTools::FileTimeCompare(depender.c_str(), dependee.c_str(),
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
      cmSystemTools::RemoveFile(depender.c_str());
      }
    }

  return okay;
}

//----------------------------------------------------------------------------
void cmDependsC::Scan(std::istream& is, const char* directory)
{
  // Read one line at a time.
  std::string line;
  while(cmSystemTools::GetLineFromStream(is, line))
    {
    // Match include directives.
    if(m_IncludeRegexLine.find(line.c_str()))
      {
      // Get the file being included.
      UnscannedEntry entry;
      entry.FileName = m_IncludeRegexLine.match(1);
      if(m_IncludeRegexLine.match(2) == "\"")
        {
        // This was a double-quoted include.  We must check for the
        // file in the directory containing the file we are scanning.
        entry.QuotedLocation = directory;
        entry.QuotedLocation += "/";
        entry.QuotedLocation += entry.FileName;
        }

      // Queue the file if it has not yet been encountered and it
      // matches the regular expression for recursive scanning.  Note
      // that this check does not account for the possibility of two
      // headers with the same name in different directories when one
      // is included by double-quotes and the other by angle brackets.
      // This kind of problem will be fixed when a more
      // preprocessor-like implementation of this scanner is created.
      if(m_Encountered.find(entry.FileName) == m_Encountered.end() &&
         m_IncludeRegexScan.find(entry.FileName.c_str()))
        {
        m_Encountered.insert(entry.FileName);
        m_Unscanned.push(entry);
        }
      }
    }
}

//----------------------------------------------------------------------------
bool cmDependsC::ParseDependency(const char* line, std::string& depender,
                                 std::string& dependee)
{
  // Start with empty names.
  depender = "";
  dependee = "";

  // Get the left-hand-side of the dependency.
  const char* c = this->ParseFileName(line, depender);

  // Skip the ':' separator.
  for(;c && *c && isspace(*c);++c);
  if(!c || !*c || *c != ':')
    {
    return false;
    }
  ++c;

  // Get the right-hand-side of the dependency.
  return this->ParseFileName(c, dependee)?true:false;
}

//----------------------------------------------------------------------------
const char* cmDependsC::ParseFileName(const char* in, std::string& name)
{
  // Skip leading whitespace.
  const char* c = in;
  for(;c && *c && isspace(*c);++c);

  // If this is an empty line or a comment line return failure.
  if(!c || !*c || *c == '#')
    {
    return 0;
    }

  // Parse the possibly quoted file name.
  bool quoted = false;
  char* buf = new char[strlen(in)+1];
  char* pos = buf;
  
  // for every character while we haven't hit the end of the string AND we
  // are in a quoted string OR the current character isn't a : or the second
  // character AND it isn't a space
  for(;*c && (quoted ||
              ((*c != ':' || pos == buf+1) && !isspace(*c))); ++c)
    {
    if(*c == '"')
      {
      quoted = !quoted;
      }
    // handle unquoted escaped spaces
    else if(!quoted && *c == '\\' && isspace(*(c+1)))
      {
      *pos =  *(++c);
      pos++;
      }
    else
      {
      *pos = *c;
      pos++;
      }
    }
  *pos =0;
  name += buf;
  delete [] buf;
  // Return the ending position.
  return c;
}

//----------------------------------------------------------------------------
bool cmDependsC::FileExistsOrIsGenerated(const std::string& fname,
                                         std::set<cmStdString>& scanned,
                                         std::set<cmStdString>& dependencies)
{
  // Check first for a generated file.
  if(m_GeneratedFiles &&
     m_GeneratedFiles->find(fname) != m_GeneratedFiles->end())
    {
    // If the file does not really exist yet pretend it has already
    // been scanned.  When it exists later then dependencies will be
    // rescanned.
    if(!cmSystemTools::FileExists(fname.c_str()))
      {
      scanned.insert(fname);
      dependencies.insert(fname);
      }
    return true;
    }
  else
    {
    return cmSystemTools::FileExists(fname.c_str());
    }
}
