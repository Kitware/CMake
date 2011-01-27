/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCacheManager.h"
#include "cmSystemTools.h"
#include "cmCacheManager.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmVersion.h"

#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>

#include <cmsys/RegularExpression.hxx>

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

cmCacheManager::cmCacheManager(cmake* cm)
{
  this->CacheMajorVersion = 0;
  this->CacheMinorVersion = 0;
  this->CMakeInstance = cm;
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

bool cmCacheManager::IsType(const char* s)
{
  for(int i=0; cmCacheManagerTypes[i]; ++i)
    {
    if(strcmp(s, cmCacheManagerTypes[i]) == 0)
      {
      return true;
      }
    }
  return false;
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

static bool ParseEntryWithoutType(const char* entry,
                                  std::string& var,
                                  std::string& value)
{
  // input line is:         key=value
  static cmsys::RegularExpression reg(
    "^([^=]*)=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
  // input line is:         "key"=value
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

  if (!flag)
    {
    return ParseEntryWithoutType(entry, var, value);
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
    e.Properties.SetCMakeInstance(this->CMakeInstance);
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
          if(!this->ReadPropertyEntry(entryKey, e))
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
        std::string(" where CMakeCache.txt was created. This may result "
                    "in binaries being created in the wrong place. If you "
                    "are not sure, reedit the CMakeCache.txt");
      cmSystemTools::Error(message.c_str());
      }
    }
  return true;
}

//----------------------------------------------------------------------------
const char* cmCacheManager::PersistentProperties[] =
{
  "ADVANCED",
  "MODIFIED",
  "STRINGS",
  0
};

//----------------------------------------------------------------------------
bool cmCacheManager::ReadPropertyEntry(std::string const& entryKey,
                                       CacheEntry& e)
{
  // All property entries are internal.
  if(e.Type != cmCacheManager::INTERNAL)
    {
    return false;
    }

  const char* end = entryKey.c_str() + entryKey.size();
  for(const char** p = this->PersistentProperties; *p; ++p)
    {
    std::string::size_type plen = strlen(*p) + 1;
    if(entryKey.size() > plen && *(end-plen) == '-' &&
       strcmp(end-plen+1, *p) == 0)
      {
      std::string key = entryKey.substr(0, entryKey.size() - plen);
      cmCacheManager::CacheIterator it = this->GetCacheIterator(key.c_str());
      if(it.IsAtEnd())
        {
        // Create an entry and store the property.
        CacheEntry& ne = this->Cache[key];
        ne.Properties.SetCMakeInstance(this->CMakeInstance);
        ne.Type = cmCacheManager::UNINITIALIZED;
        ne.SetProperty(*p, e.Value.c_str());
        }
      else
        {
        // Store this property on its entry.
        it.SetProperty(*p, e.Value.c_str());
        }
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void cmCacheManager::WritePropertyEntries(std::ostream& os,
                                          CacheIterator const& i)
{
  for(const char** p = this->PersistentProperties; *p; ++p)
    {
    if(const char* value = i.GetProperty(*p))
      {
      std::string helpstring = *p;
      helpstring += " property for variable: ";
      helpstring += i.GetName();
      cmCacheManager::OutputHelpString(os, helpstring);

      std::string key = i.GetName();
      key += "-";
      key += *p;
      this->OutputKey(os, key);
      os << ":INTERNAL=";
      this->OutputValue(os, value);
      os << "\n";
      }
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
    // Cast added to avoid compiler warning. Cast is ok because
    // value is guaranteed to fit in char by the above if...
    currentcwd[0] = static_cast<char>(currentcwd[0] - 'A' + 'a');
    }
  cmSystemTools::ConvertToUnixSlashes(currentcwd);
  this->AddCacheEntry("CMAKE_CACHEFILE_DIR", currentcwd.c_str(),
                      "This is the directory where this CMakeCache.txt"
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
      this->OutputKey(fout, i->first);
      fout << ":" << cmCacheManagerTypes[t] << "=";
      this->OutputValue(fout, ce.Value);
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
    this->WritePropertyEntries(fout, i);
    if(t == cmCacheManager::INTERNAL)
      {
      // Format is key:type=value
      if(const char* help = i.GetProperty("HELPSTRING"))
        {
        this->OutputHelpString(fout, help);
        }
      this->OutputKey(fout, i.GetName());
      fout << ":" << cmCacheManagerTypes[t] << "=";
      this->OutputValue(fout, i.GetValue());
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

void cmCacheManager::OutputKey(std::ostream& fout, std::string const& key)
{
  // support : in key name by double quoting
  const char* q = (key.find(':') != key.npos ||
                   key.find("//") == 0)? "\"" : "";
  fout << q << key << q;
}

void cmCacheManager::OutputValue(std::ostream& fout, std::string const& value)
{
  // if value has trailing space or tab, enclose it in single quotes
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
}

void cmCacheManager::OutputHelpString(std::ostream& fout,
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
  e.Properties.SetCMakeInstance(this->CMakeInstance);
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
    this->GetEntry().SetProperty(p, v);
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

//----------------------------------------------------------------------------
void cmCacheManager::DefineProperties(cmake *cm)
{
  cm->DefineProperty
    ("ADVANCED", cmProperty::CACHE,
     "True if entry should be hidden by default in GUIs.",
     "This is a boolean value indicating whether the entry is considered "
     "interesting only for advanced configuration.  "
     "The mark_as_advanced() command modifies this property."
      );

  cm->DefineProperty
    ("HELPSTRING", cmProperty::CACHE,
     "Help associated with entry in GUIs.",
     "This string summarizes the purpose of an entry to help users set it "
     "through a CMake GUI."
      );

  cm->DefineProperty
    ("TYPE", cmProperty::CACHE,
     "Widget type for entry in GUIs.",
     "Cache entry values are always strings, but CMake GUIs present widgets "
     "to help users set values.  "
     "The GUIs use this property as a hint to determine the widget type.  "
     "Valid TYPE values are:\n"
     "  BOOL          = Boolean ON/OFF value.\n"
     "  PATH          = Path to a directory.\n"
     "  FILEPATH      = Path to a file.\n"
     "  STRING        = Generic string value.\n"
     "  INTERNAL      = Do not present in GUI at all.\n"
     "  STATIC        = Value managed by CMake, do not change.\n"
     "  UNINITIALIZED = Type not yet specified.\n"
     "Generally the TYPE of a cache entry should be set by the command "
     "which creates it (set, option, find_library, etc.)."
      );

  cm->DefineProperty
    ("MODIFIED", cmProperty::CACHE,
     "Internal management property.  Do not set or get.",
     "This is an internal cache entry property managed by CMake to "
     "track interactive user modification of entries.  Ignore it."
      );

  cm->DefineProperty
    ("STRINGS", cmProperty::CACHE,
     "Enumerate possible STRING entry values for GUI selection.",
     "For cache entries with type STRING, this enumerates a set of values.  "
     "CMake GUIs may use this to provide a selection widget instead of a "
     "generic string entry field.  "
     "This is for convenience only.  "
     "CMake does not enforce that the value matches one of those listed."
      );

  cm->DefineProperty
    ("VALUE", cmProperty::CACHE,
     "Value of a cache entry.",
     "This property maps to the actual value of a cache entry.  "
     "Setting this property always sets the value without checking, so "
     "use with care."
      );
}
