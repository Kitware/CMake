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

const char* cmCacheManagerTypes[] = 
{ "BOOL",
  "PATH",
  "FILEPATH",
  "STRING",
  "INTERNAL",
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

    
cmCacheManager* cmCacheManager::s_Instance = 0;

cmCacheManager* cmCacheManager::GetInstance()
{
  if(!cmCacheManager::s_Instance)
    {
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
  // input line is:         key:type=value
  cmRegularExpression reg("(.*):(.*)=(.*)");
  while(fin)
    {
    // Format is key:type=value
    CacheEntry e;
    fin.getline(buffer, bsize);
    // skip blank lines and comment lines
    if(buffer[0] == '#' || buffer[0] == 0)
      {
      continue;
      }
    while(buffer[0] == '/')
      {
      e.m_HelpString += &buffer[2];
      fin.getline(buffer, bsize);
      if(!fin)
        {
        continue;
        }
      }
    if(reg.find(buffer))
      {
      e.m_Type = cmCacheManager::StringToType(reg.match(2).c_str());
      // only load internal values if internal is set
      if (internal || e.m_Type != INTERNAL)
	{
	  e.m_Value = reg.match(3);
	  m_Cache[reg.match(1)] = e;
	}
      }
    else
      {
      cmSystemTools::Error("Parse error in cache file ", cacheFile.c_str());
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
  for( std::map<std::string, CacheEntry>::const_iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    const CacheEntry& ce = (*i).second;
    mf->AddDefinition((*i).first.c_str(), ce.m_Value.c_str());
    }
}

bool cmCacheManager::SaveCache(cmMakefile* mf) const
{
  return this->SaveCache(mf->GetHomeOutputDirectory());
}


bool cmCacheManager::SaveCache(const char* path) const
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

  for( std::map<std::string, CacheEntry>::const_iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    const CacheEntry& ce = (*i).second; 
    CacheEntryType t = ce.m_Type;
    if(t != INTERNAL)
      {
      // Format is key:type=value
      cmCacheManager::OutputHelpString(fout, ce.m_HelpString);
      fout << (*i).first.c_str() << ":"
           << cmCacheManagerTypes[t] << "="
           << ce.m_Value << "\n\n";
      }
    }

  fout << "\n";
  fout << "########################\n";
  fout << "# INTERNAL cache entries\n";
  fout << "########################\n";
  fout << "\n";

  for( std::map<std::string, CacheEntry>::const_iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    const CacheEntry& ce = (*i).second;
    CacheEntryType t = ce.m_Type;
    if(t == INTERNAL)
      {
      // Format is key:type=value
      cmCacheManager::OutputHelpString(fout, ce.m_HelpString);
      fout << (*i).first.c_str() << ":"
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
  m_Cache.erase(key);
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


bool cmCacheManager::IsOn(const char* key) const
{ 
  if(!m_Cache.count(key))
    {
    return false;
    }
  const std::string &v = m_Cache.find(key)->second.m_Value;
  return cmSystemTools::IsOn(v.c_str());
}



void cmCacheManager::PrintCache(std::ostream& out) const
{
  out << "=================================================" << std::endl;
  out << "CMakeCache Contents:" << std::endl;
  for(std::map<std::string, CacheEntry>::const_iterator i = m_Cache.begin();
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

