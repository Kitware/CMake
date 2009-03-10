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
#include "cmake.h"
#include "cmVersion.h"

#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>

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

cmCacheManager::cmCacheManager()
{
  this->CacheMajorVersion = 0;
  this->CacheMinorVersion = 0;
}

const char* cmCacheManager::TypeToString(cmCacheManager::CacheEntryType type)
{
  if ( type > 6 )
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
  std::set<cmStdString> emptySet;
  return this->LoadCache(path, internal, emptySet, emptySet);
}

bool cmCacheManager::ParseEntry(const char* entry,
                                std::string& var,
                                std::string& value)
{
  // input line is:         key:type=value
  static cmsys::RegularExpression reg(
    "^([^:]*)=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
  // input line is:         "key":type=value
  static cmsys::RegularExpression regQuoted(
    "^\"([^\"]*)\"=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
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
  static cmsys::RegularExpression reg(
    "^([^:]*):([^=]*)=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
  // input line is:         "key":type=value
  static cmsys::RegularExpression regQuoted(
    "^\"([^\"]*)\":([^=]*)=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
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

void cmCacheManager::CleanCMakeFiles(const char* path)
{
  std::string glob = path;
  glob += cmake::GetCMakeFilesDirectory();
  glob += "/*.cmake";
  cmsys::Glob globIt;
  globIt.FindFiles(glob);
  std::vector<std::string> files = globIt.GetFiles();
  for(std::vector<std::string>::iterator i = files.begin();
      i != files.end(); ++i)
    {
    cmSystemTools::RemoveFile(i->c_str());
    }
}

bool cmCacheManager::LoadCache(const char* path,
                               bool internal,
                               std::set<cmStdString>& excludes,
                               std::set<cmStdString>& includes)
{
  std::string cacheFile = path;
  cacheFile += "/CMakeCache.txt";
  // clear the old cache, if we are reading in internal values
  if ( internal )
    {
    this->Cache.clear();
    }
  if(!cmSystemTools::FileExists(cacheFile.c_str()))
    {
    this->CleanCMakeFiles(path);
    return false;
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
    std::string helpString;
    CacheEntry e;
    cmSystemTools::GetLineFromStream(fin, buffer);
    realbuffer = buffer.c_str();
    while(*realbuffer != '0' &&
          (*realbuffer == ' ' ||
           *realbuffer == '\t' ||
           *realbuffer == '\r' ||
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
      if ((realbuffer[2] == '\\') && (realbuffer[3]=='n'))
        {
        helpString += "\n";
        helpString += &realbuffer[4];
        }
      else
        {
        helpString += &realbuffer[2];
        }
      cmSystemTools::GetLineFromStream(fin, buffer);
      realbuffer = buffer.c_str();
      if(!fin)
        {
        continue;
        }
      }
    e.SetProperty("HELPSTRING", helpString.c_str());
    if(cmCacheManager::ParseEntry(realbuffer, entryKey, e.Value, e.Type))
      {
      if ( excludes.find(entryKey) == excludes.end() )
        {
        // Load internal values if internal is set.
        // If the entry is not internal to the cache being loaded
        // or if it is in the list of internal entries to be
        // imported, load it.
        if ( internal || (e.Type != INTERNAL) || 
             (includes.find(entryKey) != includes.end()) )
          {
          // If we are loading the cache from another project,
          // make all loaded entries internal so that it is
          // not visible in the gui
          if (!internal)
            {
            e.Type = INTERNAL;
            helpString = "DO NOT EDIT, ";
            helpString += entryKey;
            helpString += " loaded from external file.  "
              "To change this value edit this file: ";
            helpString += path;
            helpString += "/CMakeCache.txt"   ;
            e.SetProperty("HELPSTRING", helpString.c_str());
            }
          if ( e.Type == cmCacheManager::INTERNAL &&
               (entryKey.size() > strlen("-ADVANCED")) &&
               strcmp(entryKey.c_str() + (entryKey.size() -
                   strlen("-ADVANCED")), "-ADVANCED") == 0 )
            {
            std::string value = e.Value;
            std::string akey = 
              entryKey.substr(0, (entryKey.size() - strlen("-ADVANCED")));
            cmCacheManager::CacheIterator it = 
              this->GetCacheIterator(akey.c_str());
            if ( it.IsAtEnd() )
              {
              e.Type = cmCacheManager::UNINITIALIZED;
              this->Cache[akey] = e;
              }
            if (!it.Find(akey.c_str()))
              {
              cmSystemTools::Error("Internal CMake error when reading cache");
              }
            it.SetProperty("ADVANCED", value.c_str());
            }
          else if ( e.Type == cmCacheManager::INTERNAL &&
                    (entryKey.size() > strlen("-MODIFIED")) &&
                    strcmp(entryKey.c_str() + (entryKey.size() -
                        strlen("-MODIFIED")), "-MODIFIED") == 0 )
            {
            std::string value = e.Value;
            std::string akey = 
              entryKey.substr(0, (entryKey.size() - strlen("-MODIFIED")));
            cmCacheManager::CacheIterator it = 
              this->GetCacheIterator(akey.c_str());
            if ( it.IsAtEnd() )
              {
              e.Type = cmCacheManager::UNINITIALIZED;
              this->Cache[akey] = e;
              }
            if (!it.Find(akey.c_str()))
              {
              cmSystemTools::Error("Internal CMake error when reading cache");
              }
            it.SetProperty("MODIFIED", value.c_str());
            }
          else
            {
            e.Initialized = true;
            this->Cache[entryKey] = e;
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
  this->CacheMajorVersion = 0;
  this->CacheMinorVersion = 0;
  if(const char* cmajor = this->GetCacheValue("CMAKE_CACHE_MAJOR_VERSION"))
    {
    unsigned int v=0;
    if(sscanf(cmajor, "%u", &v) == 1)
      {
      this->CacheMajorVersion = v;
      }
    if(const char* cminor = this->GetCacheValue("CMAKE_CACHE_MINOR_VERSION"))
      {
      if(sscanf(cminor, "%u", &v) == 1)
        {
        this->CacheMinorVersion = v;
        }
      }
    }
  else
    {
    // CMake version not found in the list file.
    // Set as version 0.0
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
    cmSystemTools::ReportLastSystemError("");
    return false;
    }
  // before writing the cache, update the version numbers
  // to the
  char temp[1024];
  sprintf(temp, "%d", cmVersion::GetMinorVersion());
  this->AddCacheEntry("CMAKE_CACHE_MINOR_VERSION", temp,
                      "Minor version of cmake used to create the "
                      "current loaded cache", cmCacheManager::INTERNAL);
  sprintf(temp, "%d", cmVersion::GetMajorVersion());
  this->AddCacheEntry("CMAKE_CACHE_MAJOR_VERSION", temp,
                      "Major version of cmake used to create the "
                      "current loaded cache", cmCacheManager::INTERNAL);
  sprintf(temp, "%d", cmVersion::GetPatchVersion());
  this->AddCacheEntry("CMAKE_CACHE_PATCH_VERSION", temp,
                      "Patch version of cmake used to create the "
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
       << "# For build in directory: " << currentcwd << "\n";
  cmCacheManager::CacheEntry* cmakeCacheEntry
    = this->GetCacheEntry("CMAKE_COMMAND");
  if ( cmakeCacheEntry )
    {
    fout << "# It was generated by CMake: " << 
      cmakeCacheEntry->Value << std::endl;
    }

  fout << "# You can edit this file to change values found and used by cmake."
       << std::endl
       << "# If you do not want to change any of the values, simply exit the "
       "editor." << std::endl
       << "# If you do want to change a value, simply edit, save, and exit "
       "the editor." << std::endl
       << "# The syntax for the file is as follows:\n"
       << "# KEY:TYPE=VALUE\n"
       << "# KEY is the name of a variable in the cache.\n"
       << "# TYPE is a hint to GUI's for the type of VALUE, DO NOT EDIT "
       "TYPE!." << std::endl
       << "# VALUE is the current value for the KEY.\n\n";

  fout << "########################\n";
  fout << "# EXTERNAL cache entries\n";
  fout << "########################\n";
  fout << "\n";

  for( std::map<cmStdString, CacheEntry>::const_iterator i = 
         this->Cache.begin(); i != this->Cache.end(); ++i)
    {
    const CacheEntry& ce = (*i).second; 
    CacheEntryType t = ce.Type;
    if(!ce.Initialized)
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
      if(const char* help = ce.GetProperty("HELPSTRING"))
        {
        cmCacheManager::OutputHelpString(fout, help);
        }
      else
        {
        cmCacheManager::OutputHelpString(fout, "Missing description");
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
      if (ce.Value.size() &&
          (ce.Value[ce.Value.size() - 1] == ' ' || 
           ce.Value[ce.Value.size() - 1] == '\t'))
        {
        fout << '\'' << ce.Value << '\'';
        }
      else
        {
        fout << ce.Value;
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
    bool modified = i.PropertyExists("MODIFIED");
    if ( modified )
      {
      // Format is key:type=value
      std::string key;
      std::string rkey = i.GetName();
      std::string helpstring;
      // If this is advanced variable, we have to do some magic for
      // backward compatibility
      helpstring = "Modified flag for variable: ";
      helpstring += i.GetName();
      rkey += "-MODIFIED";
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
           << (i.GetPropertyAsBool("MODIFIED") ? "1" : "0") << "\n";
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
  checkCacheFile += cmake::GetCMakeFilesDirectory();
  cmSystemTools::MakeDirectory(checkCacheFile.c_str());
  checkCacheFile += "/cmake.check_cache";
  std::ofstream checkCache(checkCacheFile.c_str());
  if(!checkCache)
    {
    cmSystemTools::Error("Unable to open check cache file for write. ",
                         checkCacheFile.c_str());
    return false;
    }
  checkCache << "# This file is generated by cmake for dependency checking "
    "of the CMakeCache.txt file\n";
  return true;
}

bool cmCacheManager::DeleteCache(const char* path)
{
  std::string cacheFile = path;
  cmSystemTools::ConvertToUnixSlashes(cacheFile);
  std::string cmakeFiles = cacheFile;
  cacheFile += "/CMakeCache.txt";
  cmSystemTools::RemoveFile(cacheFile.c_str());
  // now remove the files in the CMakeFiles directory
  // this cleans up language cache files
  cmsys::Directory dir;
  cmakeFiles += cmake::GetCMakeFilesDirectory();
  dir.Load(cmakeFiles.c_str());
  for (unsigned long fileNum = 0;
    fileNum <  dir.GetNumberOfFiles();
    ++fileNum)
    {
    if(!cmSystemTools::
       FileIsDirectory(dir.GetFile(fileNum)))
      {
      std::string fullPath = cmakeFiles;
      fullPath += "/";
      fullPath += dir.GetFile(fileNum);
      cmSystemTools::RemoveFile(fullPath.c_str());
      }
    }
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
  for (std::string::size_type i=0; i<=end; i++)
    {
    if ((i==end) 
        || (helpString[i]=='\n')
        || ((i-pos >= 60) && (helpString[i]==' ')))
      {
      fout << "//";
      if (helpString[pos] == '\n')
        {
        pos++;
        fout << "\\n";
        }
      oneLine = helpString.substr(pos, i - pos);
      fout << oneLine.c_str() << "\n";
      pos = i;
      }
    }
}

void cmCacheManager::RemoveCacheEntry(const char* key)
{
  CacheEntryMap::iterator i = this->Cache.find(key);
  if(i != this->Cache.end())
    {
    this->Cache.erase(i);
    }
}


cmCacheManager::CacheEntry *cmCacheManager::GetCacheEntry(const char* key)
{
  CacheEntryMap::iterator i = this->Cache.find(key);
  if(i != this->Cache.end())
    {
    return &i->second;
    }
  return 0;
}

cmCacheManager::CacheIterator cmCacheManager::GetCacheIterator(
  const char *key)
{
  return CacheIterator(*this, key);
}

const char* cmCacheManager::GetCacheValue(const char* key) const
{
  CacheEntryMap::const_iterator i = this->Cache.find(key);
  if(i != this->Cache.end() &&
    i->second.Initialized)
    {
    return i->second.Value.c_str();
    }
  return 0;
}


void cmCacheManager::PrintCache(std::ostream& out) const
{
  out << "=================================================" << std::endl;
  out << "CMakeCache Contents:" << std::endl;
  for(std::map<cmStdString, CacheEntry>::const_iterator i = 
        this->Cache.begin(); i != this->Cache.end(); ++i)
    {
    if((*i).second.Type != INTERNAL)
      {
      out << (*i).first.c_str() << " = " << (*i).second.Value.c_str() 
          << std::endl;
      }
    }
  out << "\n\n";
  out << "To change values in the CMakeCache, "
    << std::endl << "edit CMakeCache.txt in your output directory.\n";
  out << "=================================================" << std::endl;
}


void cmCacheManager::AddCacheEntry(const char* key,
                                   const char* value,
                                   const char* helpString,
                                   CacheEntryType type)
{
  CacheEntry& e = this->Cache[key];
  if ( value )
    {
    e.Value = value;
    e.Initialized = true;
    }
  else
    {
    e.Value = "";
    }
  e.Type = type;
  // make sure we only use unix style paths
  if(type == FILEPATH || type == PATH)
    {
    if(e.Value.find(';') != e.Value.npos)
      {
      std::vector<std::string> paths;
      cmSystemTools::ExpandListArgument(e.Value, paths);
      const char* sep = "";
      e.Value = "";
      for(std::vector<std::string>::iterator i = paths.begin();
          i != paths.end(); ++i)
        {
        cmSystemTools::ConvertToUnixSlashes(*i);
        e.Value += sep;
        e.Value += *i;
        sep = ";";
        }
      }
    else
      {
      cmSystemTools::ConvertToUnixSlashes(e.Value);
      }
    }
  e.SetProperty("HELPSTRING", helpString? helpString :
                "(This variable does not exist and should not be used)");
  this->Cache[key] = e;
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
  return this->Position == this->Container.Cache.end();
}

void cmCacheManager::CacheIterator::Begin()
{
  this->Position = this->Container.Cache.begin(); 
}

bool cmCacheManager::CacheIterator::Find(const char* key)
{
  this->Position = this->Container.Cache.find(key);
  return !this->IsAtEnd();
}

void cmCacheManager::CacheIterator::Next()
{
  if (!this->IsAtEnd())
    {
    ++this->Position; 
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
    entry->Value = value;
    entry->Initialized = true;
    }
  else
    {
    entry->Value = "";
    }
}

//----------------------------------------------------------------------------
bool cmCacheManager::CacheIterator::GetValueAsBool() const
{
  return cmSystemTools::IsOn(this->GetEntry().Value.c_str());
}

//----------------------------------------------------------------------------
const char*
cmCacheManager::CacheEntry::GetProperty(const char* prop) const
{
  if(strcmp(prop, "TYPE") == 0)
    {
    return cmCacheManagerTypes[this->Type];
    }
  else if(strcmp(prop, "VALUE") == 0)
    {
    return this->Value.c_str();
    }
  bool c = false;
  return
    this->Properties.GetPropertyValue(prop, cmProperty::CACHE, c);
}

//----------------------------------------------------------------------------
void cmCacheManager::CacheEntry::SetProperty(const char* prop,
                                             const char* value)
{
  if(strcmp(prop, "TYPE") == 0)
    {
    this->Type = cmCacheManager::StringToType(value? value : "STRING");
    }
  else if(strcmp(prop, "VALUE") == 0)
    {
    this->Value = value? value : "";
    }
  else
    {
    this->Properties.SetProperty(prop, value, cmProperty::CACHE);
    }
}

//----------------------------------------------------------------------------
void cmCacheManager::CacheEntry::AppendProperty(const char* prop,
                                                const char* value)
{
  if(strcmp(prop, "TYPE") == 0)
    {
    this->Type = cmCacheManager::StringToType(value? value : "STRING");
    }
  else if(strcmp(prop, "VALUE") == 0)
    {
    if(value)
      {
      if(!this->Value.empty() && *value)
        {
        this->Value += ";";
        }
      this->Value += value;
      }
    }
  else
    {
    this->Properties.AppendProperty(prop, value, cmProperty::CACHE);
    }
}

//----------------------------------------------------------------------------
const char* cmCacheManager::CacheIterator::GetProperty(const char* prop) const
{
  if(!this->IsAtEnd())
    {
    return this->GetEntry().GetProperty(prop);
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmCacheManager::CacheIterator::SetProperty(const char* p, const char* v)
{
  if(!this->IsAtEnd())
    {
    return this->GetEntry().SetProperty(p, v);
    }
}

//----------------------------------------------------------------------------
void cmCacheManager::CacheIterator::AppendProperty(const char* p,
                                                   const char* v)
{
  if(!this->IsAtEnd())
    {
    this->GetEntry().AppendProperty(p, v);
    }
}

//----------------------------------------------------------------------------
bool cmCacheManager::CacheIterator::GetPropertyAsBool(const char* prop) const
{
  if(const char* value = this->GetProperty(prop))
    {
    return cmSystemTools::IsOn(value);
    }
  return false;
}

//----------------------------------------------------------------------------
void cmCacheManager::CacheIterator::SetProperty(const char* p, bool v)
{
  this->SetProperty(p, v ? "ON" : "OFF");
}

//----------------------------------------------------------------------------
bool cmCacheManager::CacheIterator::PropertyExists(const char* prop) const
{
  return this->GetProperty(prop)? true:false;
}

//----------------------------------------------------------------------------
bool cmCacheManager::NeedCacheCompatibility(int major, int minor)
{
  // Compatibility is not needed if the cache version is zero because
  // the cache was created or modified by the user.
  if(this->CacheMajorVersion == 0)
    {
    return false;
    }

  // Compatibility is needed if the cache version is equal to or lower
  // than the given version.
  unsigned int actual_compat =
    CMake_VERSION_ENCODE(this->CacheMajorVersion, this->CacheMinorVersion, 0);
  return (actual_compat &&
          actual_compat <= CMake_VERSION_ENCODE(major, minor, 0));
}
