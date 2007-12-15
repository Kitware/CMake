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

#include "cmFileTimeComparison.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"

#include <ctype.h> // isspace


#define INCLUDE_REGEX_LINE \
  "^[ \t]*#[ \t]*(include|import)[ \t]*[<\"]([^\">]+)([\">])"

#define INCLUDE_REGEX_LINE_MARKER "#IncludeRegexLine: "
#define INCLUDE_REGEX_SCAN_MARKER "#IncludeRegexScan: "
#define INCLUDE_REGEX_COMPLAIN_MARKER "#IncludeRegexComplain: "

//----------------------------------------------------------------------------
cmDependsC::cmDependsC():
  IncludePath(0)
{
}
//----------------------------------------------------------------------------
// yummy look at all those constructor arguments
cmDependsC::cmDependsC(std::vector<std::string> const& includes,
                       const char* scanRegex, const char* complainRegex,
                       const cmStdString& cacheFileName):
  IncludePath(&includes),
  IncludeRegexLine(INCLUDE_REGEX_LINE),
  IncludeRegexScan(scanRegex),
  IncludeRegexComplain(complainRegex),
  IncludeRegexLineString(INCLUDE_REGEX_LINE_MARKER INCLUDE_REGEX_LINE),
  IncludeRegexScanString(std::string(INCLUDE_REGEX_SCAN_MARKER)+scanRegex),
  IncludeRegexComplainString(
    std::string(INCLUDE_REGEX_COMPLAIN_MARKER)+complainRegex),
  CacheFileName(cacheFileName)
{
  this->ReadCacheFile();
}

//----------------------------------------------------------------------------
cmDependsC::~cmDependsC()
{
  this->WriteCacheFile();

  for (std::map<cmStdString, cmIncludeLines*>::iterator it=
         this->FileCache.begin(); it!=this->FileCache.end(); ++it)
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
  if(!this->IncludePath)
    {
    cmSystemTools::Error("Cannot scan dependencies without an include path.");
    return false;
    }

  // Walk the dependency graph starting with the source file.
  bool first = true;
  UnscannedEntry root;
  root.FileName = src;
  this->Unscanned.push(root);
  this->Encountered.clear();
  this->Encountered.insert(src);
  std::set<cmStdString> dependencies;
  std::set<cmStdString> scanned;

  // Use reserve to allocate enough memory for both strings,
  // so that during the loops no memory is allocated or freed
  std::string cacheKey;
  cacheKey.reserve(4*1024);
  std::string tempPathStr;
  tempPathStr.reserve(4*1024);

  while(!this->Unscanned.empty())
    {
    // Get the next file to scan.
    UnscannedEntry current = this->Unscanned.front();
    this->Unscanned.pop();

    // If not a full path, find the file in the include path.
    std::string fullName;
    if(first || cmSystemTools::FileIsFullPath(current.FileName.c_str()))
      {
      if(cmSystemTools::FileExists(current.FileName.c_str(), true))
        {
        fullName = current.FileName;
        }
      }
    else if(!current.QuotedLocation.empty() &&
            cmSystemTools::FileExists(current.QuotedLocation.c_str(), true))
      {
      // The include statement producing this entry was a double-quote
      // include and the included file is present in the directory of
      // the source containing the include statement.
      fullName = current.QuotedLocation;
      }
    else
      {
      // With GCC distribution of STL, assigning to a string directly
      // throws away the internal buffer of the left-hand-side.  We
      // want to keep the pre-allocated buffer so we use C-style
      // string assignment and then operator+=.  We could call
      // .clear() instead of assigning to an empty string but the
      // method does not exist on some older compilers.
      cacheKey = "";
      cacheKey += current.FileName;

      for(std::vector<std::string>::const_iterator i = 
            this->IncludePath->begin(); i != this->IncludePath->end(); ++i)
        {
        cacheKey+=*i;
        }
      std::map<cmStdString, cmStdString>::iterator
        headerLocationIt=this->HeaderLocationCache.find(cacheKey);
      if (headerLocationIt!=this->HeaderLocationCache.end())
        {
        fullName=headerLocationIt->second;
        }
      else for(std::vector<std::string>::const_iterator i =
            this->IncludePath->begin(); i != this->IncludePath->end(); ++i)
        {
        // Construct the name of the file as if it were in the current
        // include directory.  Avoid using a leading "./".

        tempPathStr = "";
        if((*i) == ".")
          {
          tempPathStr += current.FileName;
          }
        else
          {
            tempPathStr += *i;
            tempPathStr+="/";
            tempPathStr+=current.FileName;
          }

        // Look for the file in this location.
        if(cmSystemTools::FileExists(tempPathStr.c_str(), true))
          {
            fullName = tempPathStr;
            HeaderLocationCache[cacheKey]=fullName;
          break;
          }
        }
      }

    // Complain if the file cannot be found and matches the complain
    // regex.
    if(fullName.empty() &&
       this->IncludeRegexComplain.find(current.FileName.c_str()))
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
      std::map<cmStdString, cmIncludeLines*>::iterator fileIt=
        this->FileCache.find(fullName);
      if (fileIt!=this->FileCache.end())
        {
        fileIt->second->Used=true;
        dependencies.insert(fullName);
        for (std::vector<UnscannedEntry>::const_iterator incIt=
               fileIt->second->UnscannedEntries.begin(); 
             incIt!=fileIt->second->UnscannedEntries.end(); ++incIt)
          {
          if (this->Encountered.find(incIt->FileName) == 
              this->Encountered.end())
            {
            this->Encountered.insert(incIt->FileName);
            this->Unscanned.push(*incIt);
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

  // Write the dependencies to the output stream.  Makefile rules
  // written by the original local generator for this directory
  // convert the dependencies to paths relative to the home output
  // directory.  We must do the same here.
  internalDepends << obj << std::endl;
  for(std::set<cmStdString>::iterator i=dependencies.begin();
      i != dependencies.end(); ++i)
    {
    makeDepends << obj << ": " << 
      this->LocalGenerator->Convert(i->c_str(),
                                    cmLocalGenerator::HOME_OUTPUT,
                                    cmLocalGenerator::MAKEFILE)
                << std::endl;
    internalDepends << " " << i->c_str() << std::endl;
    }
  makeDepends << std::endl;

  return true;
}

//----------------------------------------------------------------------------
void cmDependsC::ReadCacheFile()
{
  if(this->CacheFileName.size() == 0)
    {
    return;
    }
  std::ifstream fin(this->CacheFileName.c_str());
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
      bool res=comp.FileTimeCompare(this->CacheFileName.c_str(), 
                                    line.c_str(), &newer);
      
      if ((res==true) && (newer==1)) //cache is newer than the parsed file
        {
        cacheEntry=new cmIncludeLines;
        this->FileCache[line]=cacheEntry;
        }
      // file doesn't exist, check that the regular expressions
      // haven't changed
      else if (res==false)
        {
        if (line.find(INCLUDE_REGEX_LINE_MARKER) == 0)
          {
          if (line != this->IncludeRegexLineString)
            {
            return;
            }
          }
        else if (line.find(INCLUDE_REGEX_SCAN_MARKER) == 0)
          {
          if (line != this->IncludeRegexScanString)
            {
            return;
            }
          }
        else if (line.find(INCLUDE_REGEX_COMPLAIN_MARKER) == 0)
          {
          if (line != this->IncludeRegexComplainString)
            {
            return;
            }
          }
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
        cacheEntry->UnscannedEntries.push_back(entry);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmDependsC::WriteCacheFile() const
{
  if(this->CacheFileName.size() == 0)
    {
    return;
    }
  std::ofstream cacheOut(this->CacheFileName.c_str());
  if(!cacheOut)
    {
    return;
    }
  
  cacheOut << this->IncludeRegexLineString << "\n\n";
  cacheOut << this->IncludeRegexScanString << "\n\n";
  cacheOut << this->IncludeRegexComplainString << "\n\n";

  for (std::map<cmStdString, cmIncludeLines*>::const_iterator fileIt=
         this->FileCache.begin();
       fileIt!=this->FileCache.end(); ++fileIt)
    {
    if (fileIt->second->Used)
      {
      cacheOut<<fileIt->first.c_str()<<std::endl;

      for (std::vector<UnscannedEntry>::const_iterator
             incIt=fileIt->second->UnscannedEntries.begin(); 
           incIt!=fileIt->second->UnscannedEntries.end(); ++incIt)
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
void cmDependsC::Scan(std::istream& is, const char* directory,
  const cmStdString& fullName)
{
  cmIncludeLines* newCacheEntry=new cmIncludeLines;
  newCacheEntry->Used=true;
  this->FileCache[fullName]=newCacheEntry;
  
  // Read one line at a time.
  std::string line;
  while(cmSystemTools::GetLineFromStream(is, line))
    {
    // Match include directives.
    if(this->IncludeRegexLine.find(line.c_str()))
      {
      // Get the file being included.
      UnscannedEntry entry;
      entry.FileName = this->IncludeRegexLine.match(2);
      if(this->IncludeRegexLine.match(3) == "\"" &&
         !cmSystemTools::FileIsFullPath(entry.FileName.c_str()))
        {
        // This was a double-quoted include with a relative path.  We
        // must check for the file in the directory containing the
        // file we are scanning.
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
      if (this->IncludeRegexScan.find(entry.FileName.c_str()))
        {
        newCacheEntry->UnscannedEntries.push_back(entry);
        if(this->Encountered.find(entry.FileName) == this->Encountered.end())
          {
          this->Encountered.insert(entry.FileName);
          this->Unscanned.push(entry);
          }
        }
      }
    }
}
