/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "cmCacheManager.h"
#include "cmSystemTools.h"
#include "cmCacheManager.h"
#include "cmMakefile.h"
#include "cmRegularExpression.h"
#include "stdio.h"

const char* cmCacheManagerTypes[] = 
{ "BOOL",
  "PATH",
  "FILEPATH",
  "STRING",
  "INTERNAL",
  "STATIC",
  0
};

cmCacheManager::CacheEntryType cmCacheManager::StringToType(const char* s)
{
  int i = 0;
  while(cmCacheManagerTypes[i])
    {
    if(strcmp(s, cmCacheManagerTypes[i]) == 0)
      {
      return static_cast<CacheEntryType>(i);
      }
    ++i;
    }
  return STRING;
}

    
struct CleanUpCacheManager
{
  ~CleanUpCacheManager()
  {
    delete cmCacheManager::GetInstance();
  }
  void Use() {}
};

CleanUpCacheManager cleanup;

cmCacheManager* cmCacheManager::s_Instance = 0;

cmCacheManager* cmCacheManager::GetInstance()
{
  if(!cmCacheManager::s_Instance)
    {
    cleanup.Use();
    cmCacheManager::s_Instance = new cmCacheManager;
    }
  return cmCacheManager::s_Instance;
}



bool cmCacheManager::LoadCache(cmMakefile* mf)
{
  return this->LoadCache(mf->GetHomeOutputDirectory());
}


bool cmCacheManager::LoadCache(const char* path)
{
  return this->LoadCache(path,true);
}

bool cmCacheManager::LoadCache(const char* path,
			       bool internal)
{
  std::set<std::string> emptySet;
  return this->LoadCache(path, internal, emptySet, emptySet);
}

bool cmCacheManager::ParseEntry(const char* entry, 
                                std::string& var,
                                std::string& value,
                                CacheEntryType& type)
{
  // input line is:         key:type=value
  cmRegularExpression reg("^([^:]*):([^=]*)=(.*[^\t ]|[\t ]*)[\t ]*$");
  // input line is:         "key":type=value
  cmRegularExpression regQuoted("^\"([^\"]*)\":([^=]*)=(.*[^\t ]|[\t ]*)[\t ]*$");
  if(regQuoted.find(entry))
    {
    var = regQuoted.match(1);
    type = cmCacheManager::StringToType(regQuoted.match(2).c_str());
    value = regQuoted.match(3);
    return true;
    }
  else if (reg.find(entry))
    {
    var = reg.match(1);
    type = cmCacheManager::StringToType(reg.match(2).c_str());
    value = reg.match(3);
    return true;
    }
  return false;
}

bool cmCacheManager::LoadCache(const char* path,
			       bool internal,
			       std::set<std::string>& excludes,
			       std::set<std::string>& includes)
{
  std::string cacheFile = path;
  cacheFile += "/CMakeCache.txt";
  // clear the old cache, if we are reading in internal values
  if ( internal )
    {
    m_Cache.clear();
    }
  std::ifstream fin(cacheFile.c_str());
  if(!fin)
    {
    return false;
    }
  const int bsize = 4096;
  char buffer[bsize];
  char *realbuffer;
  // input line is:         key:type=value
  cmRegularExpression reg("^([^:]*):([^=]*)=(.*[^\t ]|[\t ]*)[\t ]*$");
  // input line is:         "key":type=value
  cmRegularExpression regQuoted("^\"([^\"]*)\":([^=]*)=(.*[^\t ]|[\t ]*)[\t ]*$");

  std::string entryKey;
  while(fin)
    {
    // Format is key:type=value
    CacheEntry e;
    fin.getline(buffer, bsize);
    realbuffer = buffer;
    while(*realbuffer != '0' &&
          (*realbuffer == ' ' ||
           *realbuffer == '\t' ||
           *realbuffer == '\n'))
      {
      realbuffer++;
      }
    // skip blank lines and comment lines
    if(realbuffer[0] == '#' || realbuffer[0] == 0)
      {
      continue;
      }
    while(realbuffer[0] == '/')
      {
      e.m_HelpString += &realbuffer[2];
      fin.getline(realbuffer, bsize);
      if(!fin)
        {
        continue;
        }
      }
    if(cmCacheManager::ParseEntry(realbuffer, entryKey, e.m_Value, e.m_Type))
      {
      if ( excludes.find(entryKey) == excludes.end() )
	{
	// Load internal values if internal is set.
	// If the entry is not internal to the cache being loaded
	// or if it is in the list of internal entries to be
	// imported, load it.
	if ( internal || (e.m_Type != INTERNAL) || 
	     (includes.find(entryKey) != includes.end()) )
	  {
	  // If we are loading the cache from another project,
	  // make all loaded entries internal so that it is
	  // not visible in the gui
	  if (!internal)
	    {
            e.m_HelpString = "DO NOT EDIT, ";
            e.m_HelpString += entryKey;
            e.m_HelpString += " loaded from external file.  "
              "To change this value edit this file: ";
            e.m_HelpString += path;
            e.m_HelpString += "/CMakeCache.txt"	;
	    }
	  m_Cache[entryKey] = e;
	  }
	}
      }
    else
      {
      cmSystemTools::Error("Parse error in cache file ", cacheFile.c_str(),
			   ". Offending entry: ", realbuffer);
      }
    }
  // if CMAKE version not found in the list file
  // add them as version 0.0
  if(!this->GetCacheValue("CMAKE_CACHE_MINOR_VERSION"))
    {
    this->AddCacheEntry("CMAKE_CACHE_MINOR_VERSION", "0",
                        "Minor version of cmake used to create the "
                        "current loaded cache", cmCacheManager::INTERNAL);
    this->AddCacheEntry("CMAKE_CACHE_MAJOR_VERSION", "0",
                        "Major version of cmake used to create the "
                        "current loaded cache", cmCacheManager::INTERNAL);
    
    }
  // check to make sure the cache directory has not
  // been moved
  if ( internal && this->GetCacheValue("CMAKE_CACHEFILE_DIR") )
    {
    std::string currentcwd = path;
    std::string oldcwd = this->GetCacheValue("CMAKE_CACHEFILE_DIR");
#if defined(_WIN32) || defined(__CYGWIN__)
    currentcwd = cmSystemTools::LowerCase(currentcwd);
    oldcwd = cmSystemTools::LowerCase(oldcwd);
#endif // _WIN32
    cmSystemTools::ConvertToUnixSlashes(currentcwd);
    if(cmSystemTools::CollapseFullPath(oldcwd.c_str()) 
       != cmSystemTools::CollapseFullPath(currentcwd.c_str()))
      { 
      std::string message = 
        std::string("The current CMakeCache.txt directory ") +
        currentcwd + std::string(" is different than the directory ") + 
        std::string(this->GetCacheValue("CMAKE_CACHEFILE_DIR")) +
        std::string(" where CMackeCache.txt was created. This may result "
                    "in binaries being created in the wrong place. If you "
                    "are not sure, reedit the CMakeCache.txt");
      cmSystemTools::Error(message.c_str());   
      }
    }
  

  return true;
}

void cmCacheManager::DefineCache(cmMakefile *mf)
{
  if (!mf)
    {
    return;
    }
  
  // add definition to the makefile
  for( std::map<cmStdString, CacheEntry>::const_iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    const CacheEntry& ce = (*i).second;
    mf->AddDefinition((*i).first.c_str(), ce.m_Value.c_str());
    }
}

bool cmCacheManager::SaveCache(cmMakefile* mf) 
{
  return this->SaveCache(mf->GetHomeOutputDirectory());
}


bool cmCacheManager::SaveCache(const char* path) 
{
  std::string cacheFile = path;
  cacheFile += "/CMakeCache.txt";
  std::string tempFile = cacheFile;
  tempFile += ".tmp";
  std::ofstream fout(tempFile.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Unable to open cache file for save. ", 
                         cacheFile.c_str());
    return false;
    }
  // before writting the cache, update the version numbers
  // to the 
  char temp[1024];
  sprintf(temp, "%d", cmMakefile::GetMinorVersion());
  this->AddCacheEntry("CMAKE_CACHE_MINOR_VERSION", temp,
                      "Minor version of cmake used to create the "
                      "current loaded cache", cmCacheManager::INTERNAL);
  sprintf(temp, "%d", cmMakefile::GetMajorVersion());
  this->AddCacheEntry("CMAKE_CACHE_MAJOR_VERSION", temp,
                      "Major version of cmake used to create the "
                      "current loaded cache", cmCacheManager::INTERNAL);

  // Let us store the current working directory so that if somebody
  // Copies it, he will not be surprised
  std::string currentcwd = path;
  if ( currentcwd[0] >= 'A' && currentcwd[0] <= 'Z' &&
       currentcwd[1] == ':' )
    {
    currentcwd[0] = currentcwd[0] - 'A' + 'a';
    }
  cmSystemTools::ConvertToUnixSlashes(currentcwd);
  this->AddCacheEntry("CMAKE_CACHEFILE_DIR", currentcwd.c_str(),
                      "This is the directory where this CMakeCahe.txt"
                      " was created", cmCacheManager::INTERNAL);

  fout << "# This is the CMakeCache file.\n"
       << "# For build in directory: " << path << "\n"
       << "# You can edit this file to change values found and used by cmake.\n"
       << "# If you do not want to change any of the values, simply exit the editor.\n"
       << "# If you do want to change a value, simply edit, save, and exit the editor.\n"
       << "# The syntax for the file is as follows:\n"
       << "# KEY:TYPE=VALUE\n"
       << "# KEY is the name of a varible in the cache.\n"
       << "# TYPE is a hint to GUI's for the type of VALUE, DO NOT EDIT TYPE!.\n"
       << "# VALUE is the current value for the KEY.\n\n";

  fout << "########################\n";
  fout << "# EXTERNAL cache entries\n";
  fout << "########################\n";
  fout << "\n";

  for( std::map<cmStdString, CacheEntry>::const_iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    const CacheEntry& ce = (*i).second; 
    CacheEntryType t = ce.m_Type;
    if(t != INTERNAL)
      {
      // Format is key:type=value
      cmCacheManager::OutputHelpString(fout, ce.m_HelpString);
      std::string key;
      // support : in key name by double quoting 
      if((*i).first.find(':') != std::string::npos)
        {
        key = "\"";
        key += i->first;
        key += "\"";
        }
      else
        {
        key = i->first;
        }
      fout << key.c_str() << ":"
           << cmCacheManagerTypes[t] << "="
           << ce.m_Value << "\n\n";
      }
    }

  fout << "\n";
  fout << "########################\n";
  fout << "# INTERNAL cache entries\n";
  fout << "########################\n";
  fout << "\n";

  for( std::map<cmStdString, CacheEntry>::const_iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    const CacheEntry& ce = (*i).second;
    CacheEntryType t = ce.m_Type;
    if(t == INTERNAL)
      {
      // Format is key:type=value
      cmCacheManager::OutputHelpString(fout, ce.m_HelpString);
      std::string key;
      // support : in key name by double quoting 
      if((*i).first.find(':') != std::string::npos)
        {
        key = "\"";
        key += i->first;
        key += "\"";
        }
      else
        {
        key = i->first;
        }
      fout << key.c_str() << ":"
           << cmCacheManagerTypes[t] << "="
           << ce.m_Value << "\n";
      }
    }
  fout << "\n";
  fout.close();
  cmSystemTools::CopyFileIfDifferent(tempFile.c_str(),
                                     cacheFile.c_str());
  cmSystemTools::RemoveFile(tempFile.c_str());
  return true;
}

void cmCacheManager::OutputHelpString(std::ofstream& fout, 
                                      const std::string& helpString)
{
  std::string::size_type end = helpString.size();
  if(end == 0)
    {
    return;
    }
  std::string oneLine;
  std::string::size_type pos = 0;
  std::string::size_type nextBreak = 60;
  bool done = false;

  while(!done)
    {
    if(nextBreak >= end)
      {
      nextBreak = end;
      done = true;
      }
    else
      {
      while(nextBreak < end && helpString[nextBreak] != ' ')
        {
        nextBreak++;
        }
      }
    oneLine = helpString.substr(pos, nextBreak - pos);
    fout << "//" << oneLine.c_str() << "\n";
    pos = nextBreak;
    nextBreak += 60;
    }
}

void cmCacheManager::RemoveCacheEntry(const char* key)
{
  if(m_Cache.count(key))
  {  
    m_Cache.erase(key);
  }
  else
  {
    std::cerr << "Failed to remove entry" << std::endl;
  }
}


cmCacheManager::CacheEntry *cmCacheManager::GetCacheEntry(const char* key)
{
  if(m_Cache.count(key))
    {
    return &(m_Cache.find(key)->second);
    }
  return 0;
}

const char* cmCacheManager::GetCacheValue(const char* key) const
{
  if(m_Cache.count(key))
    {
    return m_Cache.find(key)->second.m_Value.c_str();
    }
  return 0;
}


void cmCacheManager::PrintCache(std::ostream& out) const
{
  out << "=================================================" << std::endl;
  out << "CMakeCache Contents:" << std::endl;
  for(std::map<cmStdString, CacheEntry>::const_iterator i = m_Cache.begin();
      i != m_Cache.end(); ++i)
    {
    if((*i).second.m_Type != INTERNAL)
      {
      out << (*i).first.c_str() << " = " << (*i).second.m_Value.c_str() << std::endl;
      }
    }
  out << "\n\n";
  out << "To change values in the CMakeCache, \nedit CMakeCache.txt in your output directory.\n";
  out << "=================================================" << std::endl;
}


void cmCacheManager::AddCacheEntry(const char* key, 
                                   const char* value, 
                                   const char* helpString,
				   CacheEntryType type)
{
  CacheEntry e;
  e.m_Value = value;
  e.m_Type = type;
  // make sure we only use unix style paths
  if(type == FILEPATH || type == PATH)
    {
    cmSystemTools::ConvertToUnixSlashes(e.m_Value);
    }  
  e.m_HelpString = helpString;
  m_Cache[key] = e;
}

void cmCacheManager::AddCacheEntry(const char* key, bool v, 
                                   const char* helpString)
{
  if(v)
    {
    this->AddCacheEntry(key, "ON", helpString, cmCacheManager::BOOL);
    }
  else
    {
    this->AddCacheEntry(key, "OFF", helpString, cmCacheManager::BOOL);
    }
}

