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

#include "cmCacheManager.h"
#include "cmSystemTools.h"
#include "cmCacheManager.h"
#include "cmMakefile.h"
#include "stdio.h"

#include <cmsys/RegularExpression.hxx>

#if defined(_WIN32) || defined(__CYGWIN__)
# include <windows.h>
#endif // _WIN32

const char* cmCacheManagerTypes[] = 
{ "BOOL",
  "PATH",
  "FILEPATH",
  "STRING",
  "INTERNAL",
  "STATIC",
  "UNINITIALIZED",
  0
};

const char* cmCacheManager::TypeToString(cmCacheManager::CacheEntryType type)
{
  if ( type > 6 || type < 0 )
    {
    return cmCacheManagerTypes[6];
    }
  return cmCacheManagerTypes[type];
}

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
                                std::string& value)
{
  // input line is:         key:type=value
  cmsys::RegularExpression reg("^([^:]*)=(.*[^\t ]|[\t ]*)[\t ]*$");
  // input line is:         "key":type=value
  cmsys::RegularExpression regQuoted("^\"([^\"]*)\"=(.*[^\t ]|[\t ]*)[\t ]*$");
  bool flag = false;
  if(regQuoted.find(entry))
    {
    var = regQuoted.match(1);
    value = regQuoted.match(2);
    flag = true;
    }
  else if (reg.find(entry))
    {
    var = reg.match(1);
    value = reg.match(2);
    flag = true;
    }

  // if value is enclosed in single quotes ('foo') then remove them
  // it is used to enclose trailing space or tab
  if (flag && 
      value.size() >= 2 &&
      value[0] == '\'' && 
      value[value.size() - 1] == '\'') 
    {
    value = value.substr(1, 
                         value.size() - 2);
    }

  return flag;
}

bool cmCacheManager::ParseEntry(const char* entry, 
                                std::string& var,
                                std::string& value,
                                CacheEntryType& type)
{
  // input line is:         key:type=value
  cmsys::RegularExpression reg("^([^:]*):([^=]*)=(.*[^\t ]|[\t ]*)[\t ]*$");
  // input line is:         "key":type=value
  cmsys::RegularExpression regQuoted("^\"([^\"]*)\":([^=]*)=(.*[^\t ]|[\t ]*)[\t ]*$");
  bool flag = false;
  if(regQuoted.find(entry))
    {
    var = regQuoted.match(1);
    type = cmCacheManager::StringToType(regQuoted.match(2).c_str());
    value = regQuoted.match(3);
    flag = true;
    }
  else if (reg.find(entry))
    {
    var = reg.match(1);
    type = cmCacheManager::StringToType(reg.match(2).c_str());
    value = reg.match(3);
    flag = true;
    }

  // if value is enclosed in single quotes ('foo') then remove them
  // it is used to enclose trailing space or tab
  if (flag && 
      value.size() >= 2 &&
      value[0] == '\'' && 
      value[value.size() - 1] == '\'') 
    {
    value = value.substr(1, 
                         value.size() - 2);
    }

  return flag;
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
  const char *realbuffer;
  std::string buffer;
  std::string entryKey;
  while(fin)
    {
    // Format is key:type=value
    CacheEntry e;
    cmSystemTools::GetLineFromStream(fin, buffer);
    realbuffer = buffer.c_str();
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
    while(realbuffer[0] == '/' && realbuffer[1] == '/')
      {
      e.m_Properties["HELPSTRING"] += &realbuffer[2];
      cmSystemTools::GetLineFromStream(fin, buffer);
      realbuffer = buffer.c_str();
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
            e.m_Type = INTERNAL;
            e.m_Properties["HELPSTRING"] = "DO NOT EDIT, ";
            e.m_Properties["HELPSTRING"] += entryKey;
            e.m_Properties["HELPSTRING"] += " loaded from external file.  "
              "To change this value edit this file: ";
            e.m_Properties["HELPSTRING"] += path;
            e.m_Properties["HELPSTRING"] += "/CMakeCache.txt"   ;
            }
          if ( e.m_Type == cmCacheManager::INTERNAL &&
               (entryKey.size() > strlen("-ADVANCED")) &&
               strcmp(entryKey.c_str() + (entryKey.size() - strlen("-ADVANCED")),
                      "-ADVANCED") == 0 )
            {
            std::string value = e.m_Value;
            std::string akey = entryKey.substr(0, (entryKey.size() - strlen("-ADVANCED")));
            cmCacheManager::CacheIterator it = this->GetCacheIterator(akey.c_str());
            if ( it.IsAtEnd() )
              {
              e.m_Type = cmCacheManager::UNINITIALIZED;
              m_Cache[akey] = e;
              }
            if (!it.Find(akey.c_str()))
              {
              cmSystemTools::Error("Internal CMake error when reading cache");
              }
            it.SetProperty("ADVANCED", value.c_str());
            }
          else
            {
            e.m_Initialized = true;
            m_Cache[entryKey] = e;
            }
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
    cmSystemTools::ConvertToUnixSlashes(currentcwd);
    currentcwd += "/CMakeCache.txt";
    oldcwd += "/CMakeCache.txt";
    if(!cmSystemTools::SameFile(oldcwd.c_str(), currentcwd.c_str()))
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

  this->AddCacheEntry("CMAKE_CACHE_RELEASE_VERSION", cmMakefile::GetReleaseVersion(),
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
       << "# For build in directory: " << currentcwd << "\n"
       << "# You can edit this file to change values found and used by cmake.\n"
       << "# If you do not want to change any of the values, simply exit the editor.\n"
       << "# If you do want to change a value, simply edit, save, and exit the editor.\n"
       << "# The syntax for the file is as follows:\n"
       << "# KEY:TYPE=VALUE\n"
       << "# KEY is the name of a variable in the cache.\n"
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
    if(t == cmCacheManager::UNINITIALIZED || !ce.m_Initialized)
      {
      /*
        // This should be added in, but is not for now.
      cmSystemTools::Error("Cache entry \"", (*i).first.c_str(), 
                           "\" is uninitialized");
      */
      }
    else if(t != INTERNAL)
      {
      // Format is key:type=value
      std::map<cmStdString,cmStdString>::const_iterator it = 
        ce.m_Properties.find("HELPSTRING");
      if ( it == ce.m_Properties.end() )
        {
        cmCacheManager::OutputHelpString(fout, "Missing description");
        }
      else
        {
        cmCacheManager::OutputHelpString(fout, it->second);
        }
      std::string key;
      // support : in key name by double quoting 
      if((*i).first.find(':') != std::string::npos ||
        (*i).first.find("//") == 0)
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
           << cmCacheManagerTypes[t] << "=";
      // if value has trailing space or tab, enclose it in single quotes
      if (ce.m_Value.size() &&
          (ce.m_Value[ce.m_Value.size() - 1] == ' ' || 
           ce.m_Value[ce.m_Value.size() - 1] == '\t'))
        {
        fout << '\'' << ce.m_Value << '\'';
        }
      else
        {
        fout << ce.m_Value;
        }
      fout << "\n\n";
      }
    }

  fout << "\n";
  fout << "########################\n";
  fout << "# INTERNAL cache entries\n";
  fout << "########################\n";
  fout << "\n";

  for( cmCacheManager::CacheIterator i = this->NewIterator();
       !i.IsAtEnd(); i.Next())
    {
    if ( !i.Initialized() )
      {
      continue;
      }

    CacheEntryType t = i.GetType();
    bool advanced = i.PropertyExists("ADVANCED");
    if ( advanced )
      {
      // Format is key:type=value
      std::string key;
      std::string rkey = i.GetName();
      std::string helpstring;
      // If this is advanced variable, we have to do some magic for
      // backward compatibility
      helpstring = "Advanced flag for variable: ";
      helpstring += i.GetName();
      rkey += "-ADVANCED";
      cmCacheManager::OutputHelpString(fout, helpstring.c_str());
      // support : in key name by double quoting 
      if(rkey.find(':') != std::string::npos ||
         rkey.find("//") == 0)
        {
        key = "\"";
        key += rkey;
        key += "\"";
        }
      else
        {
        key = rkey;
        }
      fout << key.c_str() << ":INTERNAL="
           << (i.GetPropertyAsBool("ADVANCED") ? "1" : "0") << "\n";
      }
    if(t == cmCacheManager::INTERNAL)
      {
      // Format is key:type=value
      std::string key;
      std::string rkey = i.GetName();
      std::string helpstring;
      const char* hs = i.GetProperty("HELPSTRING");
      if ( hs )
        {
        helpstring = i.GetProperty("HELPSTRING");
        }
      else
        {
        helpstring = "";
        }
      cmCacheManager::OutputHelpString(fout, helpstring.c_str());
      // support : in key name by double quoting 
      if(rkey.find(':') != std::string::npos ||
         rkey.find("//") == 0)
        {
        key = "\"";
        key += rkey;
        key += "\"";
        }
      else
        {
        key = rkey;
        }
      fout << key.c_str() << ":"
           << cmCacheManagerTypes[t] << "=";
      // if value has trailing space or tab, enclose it in single quotes
      std::string value = i.GetValue();
      if (value.size() &&
          (value[value.size() - 1] == ' ' || 
           value[value.size() - 1] == '\t'))
        {
        fout << '\'' << value << '\'';
          }
      else
        {
        fout << value;
        }
      fout << "\n";      
      }
    }
  fout << "\n";
  fout.close();
  cmSystemTools::CopyFileIfDifferent(tempFile.c_str(),
                                     cacheFile.c_str());
  cmSystemTools::RemoveFile(tempFile.c_str());
  std::string checkCacheFile = path;
  checkCacheFile += "/cmake.check_cache";
  std::ofstream checkCache(checkCacheFile.c_str());
  if(!checkCache)
    {
    cmSystemTools::Error("Unable to open check cache file for write. ", 
                         checkCacheFile.c_str());
    return false;
    }
  checkCache << "# This file is generated by cmake for dependency checking of the CMakeCache.txt file\n";
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
  CacheEntryMap::iterator i = m_Cache.find(key);
  if(i != m_Cache.end())
    {
    m_Cache.erase(i);
    }
  else
    {
    std::cerr << "Failed to remove entry:" << key << std::endl;
    }
}


cmCacheManager::CacheEntry *cmCacheManager::GetCacheEntry(const char* key)
{
  CacheEntryMap::iterator i = m_Cache.find(key);
  if(i != m_Cache.end())
    {
    return &i->second;
    }
  return 0;
}

cmCacheManager::CacheIterator cmCacheManager::GetCacheIterator(const char *key)
{
  return CacheIterator(*this, key);
}

const char* cmCacheManager::GetCacheValue(const char* key) const
{
  CacheEntryMap::const_iterator i = m_Cache.find(key);
  if(i != m_Cache.end() && i->second.m_Type != cmCacheManager::UNINITIALIZED &&
    i->second.m_Initialized)
    {
    return i->second.m_Value.c_str();
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
  CacheEntry& e = m_Cache[key];
  if ( value )
    {
    e.m_Value = value;
    e.m_Initialized = true;
    }
  else 
    {
    e.m_Value = "";
    }
  e.m_Type = type;
  // make sure we only use unix style paths
  if(type == FILEPATH || type == PATH)
    {
    cmSystemTools::ConvertToUnixSlashes(e.m_Value);
    }
  if ( helpString )
    {
    e.m_Properties["HELPSTRING"] = helpString;
    }
  else 
    {
    e.m_Properties["HELPSTRING"] = "(This variable does not exists and should not be used)";
    }
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

bool cmCacheManager::CacheIterator::IsAtEnd() const
{
  return m_Position == m_Container.m_Cache.end();
}

void cmCacheManager::CacheIterator::Begin() 
{
  m_Position = m_Container.m_Cache.begin(); 
}

bool cmCacheManager::CacheIterator::Find(const char* key)
{
  m_Position = m_Container.m_Cache.find(key);
  return !this->IsAtEnd();
}

void cmCacheManager::CacheIterator::Next() 
{
  if (!this->IsAtEnd())
    {
    ++m_Position; 
    }
}

void cmCacheManager::CacheIterator::SetValue(const char* value)
{
  if (this->IsAtEnd())
    {
    return;
    }
  CacheEntry* entry = &this->GetEntry();
  if ( value )
    {
    entry->m_Value = value;
    entry->m_Initialized = true;
    }
  else
    {
    entry->m_Value = "";
    }
}

const char* cmCacheManager::CacheIterator::GetProperty(const char* property) const
{
  // make sure it is not at the end
  if (this->IsAtEnd())
    {
    return 0;
    }

  if ( !strcmp(property, "TYPE") || !strcmp(property, "VALUE") )
    {
    cmSystemTools::Error("Property \"", property, 
                         "\" cannot be accessed through the GetProperty()");
    return 0;
    }
  const CacheEntry* ent = &this->GetEntry();
  std::map<cmStdString,cmStdString>::const_iterator it = 
    ent->m_Properties.find(property);
  if ( it == ent->m_Properties.end() )
    {
    return 0;
    }
  return it->second.c_str();
}

void cmCacheManager::CacheIterator::SetProperty(const char* p, const char* v) 
{
  // make sure it is not at the end
  if (this->IsAtEnd())
    {
    return;
    }

  if ( !strcmp(p, "TYPE") || !strcmp(p, "VALUE") )
    {
    cmSystemTools::Error("Property \"", p, 
                         "\" cannot be accessed through the SetProperty()");
    return;
    }
  CacheEntry* ent = &this->GetEntry();
  ent->m_Properties[p] = v;
}

bool cmCacheManager::CacheIterator::GetValueAsBool() const 
{ 
  return cmSystemTools::IsOn(this->GetEntry().m_Value.c_str()); 
}

bool cmCacheManager::CacheIterator::GetPropertyAsBool(const char* property) const
{
  // make sure it is not at the end
  if (this->IsAtEnd())
    {
    return false;
    }
  
  if ( !strcmp(property, "TYPE") || !strcmp(property, "VALUE") )
    {
    cmSystemTools::Error("Property \"", property, 
                         "\" cannot be accessed through the GetPropertyAsBool()");
    return false;
    }
  const CacheEntry* ent = &this->GetEntry();
  std::map<cmStdString,cmStdString>::const_iterator it = 
    ent->m_Properties.find(property);
  if ( it == ent->m_Properties.end() )
    {
    return false;
    }
  return cmSystemTools::IsOn(it->second.c_str());
}


void cmCacheManager::CacheIterator::SetProperty(const char* p, bool v) 
{
  // make sure it is not at the end
  if (this->IsAtEnd())
    {
    return;
    }

  if ( !strcmp(p, "TYPE") || !strcmp(p, "VALUE") )
    {
    cmSystemTools::Error("Property \"", p, 
                         "\" cannot be accessed through the SetProperty()");
    return;
    }
  CacheEntry* ent = &this->GetEntry();
  ent->m_Properties[p] = v ? "ON" : "OFF";
}

bool cmCacheManager::CacheIterator::PropertyExists(const char* property) const
{
  // make sure it is not at the end
  if (this->IsAtEnd())
    {
    return false;
    }

  if ( !strcmp(property, "TYPE") || !strcmp(property, "VALUE") )
    {
    cmSystemTools::Error("Property \"", property, 
                         "\" cannot be accessed through the PropertyExists()");
    return false;
    }
  const CacheEntry* ent = &this->GetEntry();
  std::map<cmStdString,cmStdString>::const_iterator it = 
    ent->m_Properties.find(property);
  if ( it == ent->m_Properties.end() )
    {
    return false;
    }
  return true;
}
