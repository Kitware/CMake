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
#include "cmFileTimeComparison.h"

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
                       std::set<cmStdString> const& generatedFiles, 
                       const cmStdString& cacheFileName):
  m_IncludePath(&includes),
  m_IncludeRegexLine("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)([\">])"),
  m_IncludeRegexScan(scanRegex),
  m_IncludeRegexComplain(complainRegex),
  m_GeneratedFiles(&generatedFiles),
  m_cacheFileName(cacheFileName)
{
  this->ReadCacheFile();
}

//----------------------------------------------------------------------------
cmDependsC::~cmDependsC()
{
  this->WriteCacheFile();

  for (std::map<cmStdString, cmIncludeLines*>::iterator it=m_fileCache.begin(); 
       it!=m_fileCache.end(); ++it)
    {
    delete it->second;
    }
}

//----------------------------------------------------------------------------
bool cmDependsC::WriteDependencies(const char *src, const char *obj,
  std::ostream& makeDepends, std::ostream& internalDepends)
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

      // Check whether this file is already in the cache
      std::map<cmStdString, cmIncludeLines*>::iterator fileIt=m_fileCache.find(fullName);
      if (fileIt!=m_fileCache.end())
        {
        fileIt->second->m_Used=true;
        dependencies.insert(fullName);
        for (std::vector<UnscannedEntry>::const_iterator incIt=
               fileIt->second->m_UnscannedEntries.begin(); 
             incIt!=fileIt->second->m_UnscannedEntries.end(); ++incIt)
          {
          if (m_Encountered.find(incIt->FileName) == m_Encountered.end())
            {
            m_Encountered.insert(incIt->FileName);
            m_Unscanned.push(*incIt);
            }
          }
        }
      else
        {
        
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
          this->Scan(fin, dir.c_str(), fullName);
          }
        }
      }
    
    first = false;
    }
  
  // Write the dependencies to the output stream.
  internalDepends << obj << std::endl;
  for(std::set<cmStdString>::iterator i=dependencies.begin();
      i != dependencies.end(); ++i)
    {
    makeDepends << obj << ": "
                << cmSystemTools::ConvertToOutputPath(i->c_str()).c_str()
                << std::endl;
    internalDepends << " " << i->c_str() << std::endl;
    }
  makeDepends << std::endl;
  
  return true;
}

//----------------------------------------------------------------------------
void cmDependsC::ReadCacheFile()
{
  if(m_cacheFileName.size() == 0)
    {
    return;
    }
  std::ifstream fin(m_cacheFileName.c_str());
  if(!fin)
    {
    return;
    }
  
  std::string line;
  cmIncludeLines* cacheEntry=0;
  bool haveFileName=false;
  
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if (line.empty())
      {
      cacheEntry=0;
      haveFileName=false;
      continue;
      }
    //the first line after an empty line is the name of the parsed file
    if (haveFileName==false)
      {
      haveFileName=true;
      int newer=0;
      cmFileTimeComparison comp;
      bool res=comp.FileTimeCompare(m_cacheFileName.c_str(), line.c_str(), &newer);
      
      if ((res==true) && (newer==1)) //cache is newer than the parsed file
        {
        cacheEntry=new cmIncludeLines;
        m_fileCache[line]=cacheEntry; 
        }
      }
    else if (cacheEntry!=0)
      {
      UnscannedEntry entry;
      entry.FileName = line;
      if (cmSystemTools::GetLineFromStream(fin, line))
        {
        if (line!="-")
          {
          entry.QuotedLocation=line;
          }
        cacheEntry->m_UnscannedEntries.push_back(entry);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmDependsC::WriteCacheFile() const
{
  if(m_cacheFileName.size() == 0)
    {
    return;
    }
  std::ofstream cacheOut(m_cacheFileName.c_str());
  if(!cacheOut)
    {
    return;
    }
  
  for (std::map<cmStdString, cmIncludeLines*>::const_iterator fileIt=m_fileCache.begin(); 
       fileIt!=m_fileCache.end(); ++fileIt)
    {
    if (fileIt->second->m_Used)
      {
      cacheOut<<fileIt->first.c_str()<<std::endl;
      
      for (std::vector<UnscannedEntry>::const_iterator
             incIt=fileIt->second->m_UnscannedEntries.begin(); 
           incIt!=fileIt->second->m_UnscannedEntries.end(); ++incIt)
        {
        cacheOut<<incIt->FileName.c_str()<<std::endl;
        if (incIt->QuotedLocation.empty())
          {
          cacheOut<<"-"<<std::endl;
          }
        else
          {
          cacheOut<<incIt->QuotedLocation.c_str()<<std::endl;
          }
        }
      cacheOut<<std::endl;
      }
   }
}

//----------------------------------------------------------------------------
void cmDependsC::Scan(std::istream& is, const char* directory, const cmStdString& fullName)
{
  cmIncludeLines* newCacheEntry=new cmIncludeLines;
  newCacheEntry->m_Used=true;
  m_fileCache[fullName]=newCacheEntry;
  
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
      if (m_IncludeRegexScan.find(entry.FileName.c_str()))
        {
        newCacheEntry->m_UnscannedEntries.push_back(entry);
        if(m_Encountered.find(entry.FileName) == m_Encountered.end())
          {
          m_Encountered.insert(entry.FileName);
          m_Unscanned.push(entry);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
bool cmDependsC::FileExistsOrIsGenerated(const std::string& fname,
                                         std::set<cmStdString>& scanned,
                                         std::set<cmStdString>& dependencies)
{
  // Check first for a generated file.
  if(m_GeneratedFiles &&
     std::set<cmStdString>::const_iterator(m_GeneratedFiles->find(fname)) !=
     m_GeneratedFiles->end())
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
