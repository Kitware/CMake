/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCacheManager.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"

#include "cmGeneratedFileStream.h"
#include "cmList.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

void cmCacheManager::CleanCMakeFiles(const std::string& path)
{
  std::string glob = cmStrCat(path, "/CMakeFiles/*.cmake");
  cmsys::Glob globIt;
  globIt.FindFiles(glob);
  std::vector<std::string> files = globIt.GetFiles();
  std::for_each(files.begin(), files.end(), cmSystemTools::RemoveFile);
}

bool cmCacheManager::LoadCache(const std::string& path, bool internal,
                               std::set<std::string>& excludes,
                               std::set<std::string>& includes)
{
  std::string cacheFile = cmStrCat(path, "/CMakeCache.txt");
  // clear the old cache, if we are reading in internal values
  if (internal) {
    this->Cache.clear();
  }
  if (!cmSystemTools::FileExists(cacheFile)) {
    this->CleanCMakeFiles(path);
    return false;
  }

  cmsys::ifstream fin(cacheFile.c_str());
  if (!fin) {
    return false;
  }
  const char* realbuffer;
  std::string buffer;
  std::string entryKey;
  unsigned int lineno = 0;
  while (fin) {
    // Format is key:type=value
    std::string helpString;
    CacheEntry e;
    cmSystemTools::GetLineFromStream(fin, buffer);
    lineno++;
    realbuffer = buffer.c_str();
    while (*realbuffer != '0' &&
           (*realbuffer == ' ' || *realbuffer == '\t' || *realbuffer == '\r' ||
            *realbuffer == '\n')) {
      if (*realbuffer == '\n') {
        lineno++;
      }
      realbuffer++;
    }
    // skip blank lines and comment lines
    if (realbuffer[0] == '#' || realbuffer[0] == 0) {
      continue;
    }
    while (realbuffer[0] == '/' && realbuffer[1] == '/') {
      if ((realbuffer[2] == '\\') && (realbuffer[3] == 'n')) {
        helpString += '\n';
        helpString += &realbuffer[4];
      } else {
        helpString += &realbuffer[2];
      }
      cmSystemTools::GetLineFromStream(fin, buffer);
      lineno++;
      realbuffer = buffer.c_str();
      if (!fin) {
        continue;
      }
    }
    e.SetProperty("HELPSTRING", helpString);
    if (cmState::ParseCacheEntry(realbuffer, entryKey, e.Value, e.Type)) {
      if (excludes.find(entryKey) == excludes.end()) {
        // Load internal values if internal is set.
        // If the entry is not internal to the cache being loaded
        // or if it is in the list of internal entries to be
        // imported, load it.
        if (internal || (e.Type != cmStateEnums::INTERNAL) ||
            (includes.find(entryKey) != includes.end())) {
          // If we are loading the cache from another project,
          // make all loaded entries internal so that it is
          // not visible in the gui
          if (!internal) {
            e.Type = cmStateEnums::INTERNAL;
            helpString = cmStrCat("DO NOT EDIT, ", entryKey,
                                  " loaded from external file.  "
                                  "To change this value edit this file: ",
                                  path, "/CMakeCache.txt");
            e.SetProperty("HELPSTRING", helpString);
          }
          if (!this->ReadPropertyEntry(entryKey, e)) {
            e.Initialized = true;
            this->Cache[entryKey] = e;
          }
        }
      }
    } else {
      std::ostringstream error;
      error << "Parse error in cache file " << cacheFile << " on line "
            << lineno << ". Offending entry: " << realbuffer;
      cmSystemTools::Error(error.str());
    }
  }
  this->CacheMajorVersion = 0;
  this->CacheMinorVersion = 0;
  if (cmValue cmajor =
        this->GetInitializedCacheValue("CMAKE_CACHE_MAJOR_VERSION")) {
    unsigned int v = 0;
    if (sscanf(cmajor->c_str(), "%u", &v) == 1) {
      this->CacheMajorVersion = v;
    }
    if (cmValue cminor =
          this->GetInitializedCacheValue("CMAKE_CACHE_MINOR_VERSION")) {
      if (sscanf(cminor->c_str(), "%u", &v) == 1) {
        this->CacheMinorVersion = v;
      }
    }
  } else {
    // CMake version not found in the list file.
    // Set as version 0.0
    this->AddCacheEntry("CMAKE_CACHE_MINOR_VERSION", "0",
                        "Minor version of cmake used to create the "
                        "current loaded cache",
                        cmStateEnums::INTERNAL);
    this->AddCacheEntry("CMAKE_CACHE_MAJOR_VERSION", "0",
                        "Major version of cmake used to create the "
                        "current loaded cache",
                        cmStateEnums::INTERNAL);
  }
  // check to make sure the cache directory has not
  // been moved
  cmValue oldDir = this->GetInitializedCacheValue("CMAKE_CACHEFILE_DIR");
  if (internal && oldDir) {
    std::string currentcwd = path;
    std::string oldcwd = *oldDir;
    cmSystemTools::ConvertToUnixSlashes(currentcwd);
    currentcwd += "/CMakeCache.txt";
    oldcwd += "/CMakeCache.txt";
    if (!cmSystemTools::SameFile(oldcwd, currentcwd)) {
      cmValue dir = this->GetInitializedCacheValue("CMAKE_CACHEFILE_DIR");
      std::ostringstream message;
      message << "The current CMakeCache.txt directory " << currentcwd
              << " is different than the directory " << (dir ? *dir : "")
              << " where CMakeCache.txt was created. This may result "
                 "in binaries being created in the wrong place. If you "
                 "are not sure, reedit the CMakeCache.txt";
      cmSystemTools::Error(message.str());
    }
  }
  this->CacheLoaded = true;
  return true;
}

const char* cmCacheManager::PersistentProperties[] = { "ADVANCED", "MODIFIED",
                                                       "STRINGS" };

bool cmCacheManager::ReadPropertyEntry(const std::string& entryKey,
                                       const CacheEntry& e)
{
  // All property entries are internal.
  if (e.Type != cmStateEnums::INTERNAL) {
    return false;
  }

  const char* end = entryKey.c_str() + entryKey.size();
  for (const char* p : cmCacheManager::PersistentProperties) {
    std::string::size_type plen = strlen(p) + 1;
    if (entryKey.size() > plen && *(end - plen) == '-' &&
        strcmp(end - plen + 1, p) == 0) {
      std::string key = entryKey.substr(0, entryKey.size() - plen);
      if (auto* entry = this->GetCacheEntry(key)) {
        // Store this property on its entry.
        entry->SetProperty(p, e.Value);
      } else {
        // Create an entry and store the property.
        CacheEntry& ne = this->Cache[key];
        ne.SetProperty(p, e.Value);
      }
      return true;
    }
  }
  return false;
}

void cmCacheManager::WritePropertyEntries(std::ostream& os,
                                          const std::string& entryKey,
                                          const CacheEntry& e,
                                          cmMessenger* messenger) const
{
  for (const char* p : cmCacheManager::PersistentProperties) {
    if (cmValue value = e.GetProperty(p)) {
      std::string helpstring =
        cmStrCat(p, " property for variable: ", entryKey);
      cmCacheManager::OutputHelpString(os, helpstring);

      std::string key = cmStrCat(entryKey, '-', p);
      cmCacheManager::OutputKey(os, key);
      os << ":INTERNAL=";
      cmCacheManager::OutputValue(os, *value);
      os << '\n';
      cmCacheManager::OutputNewlineTruncationWarning(os, key, *value,
                                                     messenger);
    }
  }
}

bool cmCacheManager::SaveCache(const std::string& path, cmMessenger* messenger)
{
  std::string cacheFile = cmStrCat(path, "/CMakeCache.txt");
  cmGeneratedFileStream fout(cacheFile);
  fout.SetCopyIfDifferent(true);
  if (!fout) {
    cmSystemTools::Error("Unable to open cache file for save. " + cacheFile);
    cmSystemTools::ReportLastSystemError("");
    return false;
  }
  // before writing the cache, update the version numbers
  // to the
  this->AddCacheEntry("CMAKE_CACHE_MAJOR_VERSION",
                      std::to_string(cmVersion::GetMajorVersion()),
                      "Major version of cmake used to create the "
                      "current loaded cache",
                      cmStateEnums::INTERNAL);
  this->AddCacheEntry("CMAKE_CACHE_MINOR_VERSION",
                      std::to_string(cmVersion::GetMinorVersion()),
                      "Minor version of cmake used to create the "
                      "current loaded cache",
                      cmStateEnums::INTERNAL);
  this->AddCacheEntry("CMAKE_CACHE_PATCH_VERSION",
                      std::to_string(cmVersion::GetPatchVersion()),
                      "Patch version of cmake used to create the "
                      "current loaded cache",
                      cmStateEnums::INTERNAL);

  // Let us store the current working directory so that if somebody
  // Copies it, he will not be surprised
  std::string currentcwd = path;
  if (currentcwd[0] >= 'A' && currentcwd[0] <= 'Z' && currentcwd[1] == ':') {
    // Cast added to avoid compiler warning. Cast is ok because
    // value is guaranteed to fit in char by the above if...
    currentcwd[0] = static_cast<char>(currentcwd[0] - 'A' + 'a');
  }
  cmSystemTools::ConvertToUnixSlashes(currentcwd);
  this->AddCacheEntry("CMAKE_CACHEFILE_DIR", currentcwd,
                      "This is the directory where this CMakeCache.txt"
                      " was created",
                      cmStateEnums::INTERNAL);

  /* clang-format off */
  fout << "# This is the CMakeCache file.\n"
          "# For build in directory: " << currentcwd << "\n"
          "# It was generated by CMake: "
       << cmSystemTools::GetCMakeCommand()
       << "\n"
          "# You can edit this file to change values found and used by cmake."
          "\n"
          "# If you do not want to change any of the values, simply exit the "
          "editor.\n"
          "# If you do want to change a value, simply edit, save, and exit "
          "the editor.\n"
          "# The syntax for the file is as follows:\n"
          "# KEY:TYPE=VALUE\n"
          "# KEY is the name of a variable in the cache.\n"
          "# TYPE is a hint to GUIs for the type of VALUE, DO NOT EDIT TYPE!."
          "\n"
          "# VALUE is the current value for the KEY.\n"
          "\n"
          "########################\n"
          "# EXTERNAL cache entries\n"
          "########################\n"
          "\n";
  /* clang-format on */

  for (auto const& i : this->Cache) {
    CacheEntry const& ce = i.second;
    cmStateEnums::CacheEntryType t = ce.Type;
    if (!ce.Initialized) {
      /*
        // This should be added in, but is not for now.
      cmSystemTools::Error("Cache entry \"" + i.first + "\" is uninitialized");
      */
    } else if (t != cmStateEnums::INTERNAL) {
      // Format is key:type=value
      if (cmValue help = ce.GetProperty("HELPSTRING")) {
        cmCacheManager::OutputHelpString(fout, *help);
      } else {
        cmCacheManager::OutputHelpString(fout, "Missing description");
      }
      cmCacheManager::OutputKey(fout, i.first);
      fout << ':' << cmState::CacheEntryTypeToString(t) << '=';
      cmCacheManager::OutputValue(fout, ce.Value);
      fout << '\n';
      cmCacheManager::OutputNewlineTruncationWarning(fout, i.first, ce.Value,
                                                     messenger);
      fout << '\n';
    }
  }

  fout << "\n"
          "########################\n"
          "# INTERNAL cache entries\n"
          "########################\n"
          "\n";

  for (auto const& i : this->Cache) {
    if (!i.second.Initialized) {
      continue;
    }

    cmStateEnums::CacheEntryType t = i.second.GetType();
    this->WritePropertyEntries(fout, i.first, i.second, messenger);
    if (t == cmStateEnums::INTERNAL) {
      // Format is key:type=value
      if (cmValue help = i.second.GetProperty("HELPSTRING")) {
        cmCacheManager::OutputHelpString(fout, *help);
      }
      cmCacheManager::OutputKey(fout, i.first);
      fout << ':' << cmState::CacheEntryTypeToString(t) << '=';
      cmCacheManager::OutputValue(fout, i.second.GetValue());
      fout << '\n';
      cmCacheManager::OutputNewlineTruncationWarning(
        fout, i.first, i.second.GetValue(), messenger);
    }
  }
  fout << '\n';
  fout.Close();
  std::string checkCacheFile = cmStrCat(path, "/CMakeFiles");
  cmSystemTools::MakeDirectory(checkCacheFile);
  checkCacheFile += "/cmake.check_cache";
  cmsys::ofstream checkCache(checkCacheFile.c_str());
  if (!checkCache) {
    cmSystemTools::Error("Unable to open check cache file for write. " +
                         checkCacheFile);
    return false;
  }
  checkCache << "# This file is generated by cmake for dependency checking "
                "of the CMakeCache.txt file\n";
  return true;
}

bool cmCacheManager::DeleteCache(const std::string& path)
{
  std::string cacheFile = path;
  cmSystemTools::ConvertToUnixSlashes(cacheFile);
  std::string cmakeFiles = cacheFile;
  cacheFile += "/CMakeCache.txt";
  if (cmSystemTools::FileExists(cacheFile)) {
    cmSystemTools::RemoveFile(cacheFile);
    // now remove the files in the CMakeFiles directory
    // this cleans up language cache files
    cmakeFiles += "/CMakeFiles";
    if (cmSystemTools::FileIsDirectory(cmakeFiles)) {
      cmSystemTools::RemoveADirectory(cmakeFiles);
    }
  }
  return true;
}

void cmCacheManager::OutputKey(std::ostream& fout, std::string const& key)
{
  // support : in key name by double quoting
  const char* q =
    (key.find(':') != std::string::npos || cmHasLiteralPrefix(key, "//"))
    ? "\""
    : "";
  fout << q << key << q;
}

void cmCacheManager::OutputValue(std::ostream& fout, std::string const& value)
{
  // look for and truncate newlines
  std::string::size_type newline = value.find('\n');
  if (newline != std::string::npos) {
    std::string truncated = value.substr(0, newline);
    OutputValueNoNewlines(fout, truncated);
  } else {
    OutputValueNoNewlines(fout, value);
  }
}

void cmCacheManager::OutputValueNoNewlines(std::ostream& fout,
                                           std::string const& value)
{
  // if value has trailing space or tab, enclose it in single quotes
  if (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
    fout << '\'' << value << '\'';
  } else {
    fout << value;
  }
}

void cmCacheManager::OutputHelpString(std::ostream& fout,
                                      const std::string& helpString)
{
  std::string::size_type end = helpString.size();
  if (end == 0) {
    return;
  }
  std::string oneLine;
  std::string::size_type pos = 0;
  for (std::string::size_type i = 0; i <= end; i++) {
    if ((i == end) || (helpString[i] == '\n') ||
        ((i - pos >= 60) && (helpString[i] == ' '))) {
      fout << "//";
      if (helpString[pos] == '\n') {
        pos++;
        fout << "\\n";
      }
      oneLine = helpString.substr(pos, i - pos);
      fout << oneLine << '\n';
      pos = i;
    }
  }
}

void cmCacheManager::OutputWarningComment(std::ostream& fout,
                                          std::string const& message,
                                          bool wrapSpaces)
{
  std::string::size_type end = message.size();
  std::string oneLine;
  std::string::size_type pos = 0;
  for (std::string::size_type i = 0; i <= end; i++) {
    if ((i == end) || (message[i] == '\n') ||
        ((i - pos >= 60) && (message[i] == ' ') && wrapSpaces)) {
      fout << "# ";
      if (message[pos] == '\n') {
        pos++;
        fout << "\\n";
      }
      oneLine = message.substr(pos, i - pos);
      fout << oneLine << '\n';
      pos = i;
    }
  }
}

void cmCacheManager::OutputNewlineTruncationWarning(std::ostream& fout,
                                                    std::string const& key,
                                                    std::string const& value,
                                                    cmMessenger* messenger)
{
  if (value.find('\n') != std::string::npos) {
    if (messenger) {
      std::string message =
        cmStrCat("Value of ", key, " contained a newline; truncating");
      messenger->IssueMessage(MessageType::WARNING, message);
    }

    std::string comment =
      cmStrCat("WARNING: Value of ", key,
               " contained a newline and was truncated. Original value:");

    OutputWarningComment(fout, comment, true);
    OutputWarningComment(fout, value, false);
  }
}

void cmCacheManager::RemoveCacheEntry(const std::string& key)
{
  this->Cache.erase(key);
}

cmCacheManager::CacheEntry* cmCacheManager::GetCacheEntry(
  const std::string& key)
{
  auto i = this->Cache.find(key);
  if (i != this->Cache.end()) {
    return &i->second;
  }
  return nullptr;
}

const cmCacheManager::CacheEntry* cmCacheManager::GetCacheEntry(
  const std::string& key) const
{
  auto i = this->Cache.find(key);
  if (i != this->Cache.end()) {
    return &i->second;
  }
  return nullptr;
}

cmValue cmCacheManager::GetInitializedCacheValue(const std::string& key) const
{
  if (const auto* entry = this->GetCacheEntry(key)) {
    if (entry->Initialized) {
      return cmValue(entry->GetValue());
    }
  }
  return nullptr;
}

void cmCacheManager::PrintCache(std::ostream& out) const
{
  out << "=================================================\n"
         "CMakeCache Contents:\n";
  for (auto const& i : this->Cache) {
    if (i.second.Type != cmStateEnums::INTERNAL) {
      out << i.first << " = " << i.second.Value << '\n';
    }
  }
  out << "\n\n"
         "To change values in the CMakeCache, \n"
         "edit CMakeCache.txt in your output directory.\n"
         "=================================================\n";
}

void cmCacheManager::AddCacheEntry(const std::string& key, cmValue value,
                                   cmValue helpString,
                                   cmStateEnums::CacheEntryType type)
{
  CacheEntry& e = this->Cache[key];
  e.SetValue(value);
  e.Type = type;
  // make sure we only use unix style paths
  if (type == cmStateEnums::FILEPATH || type == cmStateEnums::PATH) {
    if (e.Value.find(';') != std::string::npos) {
      cmList paths{ e.Value };
      for (std::string& i : paths) {
        cmSystemTools::ConvertToUnixSlashes(i);
      }
      e.Value = paths.to_string();
    } else {
      cmSystemTools::ConvertToUnixSlashes(e.Value);
    }
  }
  e.SetProperty(
    "HELPSTRING",
    helpString ? *helpString
               : std::string{
                   "(This variable does not exist and should not be used)" });
}

void cmCacheManager::CacheEntry::SetValue(cmValue value)
{
  if (value) {
    this->Value = *value;
    this->Initialized = true;
  } else {
    this->Value.clear();
  }
}

std::vector<std::string> cmCacheManager::CacheEntry::GetPropertyList() const
{
  return this->Properties.GetKeys();
}

cmValue cmCacheManager::CacheEntry::GetProperty(const std::string& prop) const
{
  if (prop == "TYPE") {
    return cmValue(cmState::CacheEntryTypeToString(this->Type));
  }
  if (prop == "VALUE") {
    return cmValue(this->Value);
  }
  return this->Properties.GetPropertyValue(prop);
}

bool cmCacheManager::CacheEntry::GetPropertyAsBool(
  const std::string& prop) const
{
  return cmIsOn(this->GetProperty(prop));
}

void cmCacheManager::CacheEntry::SetProperty(const std::string& prop,
                                             const std::string& value)
{
  if (prop == "TYPE") {
    this->Type = cmState::StringToCacheEntryType(value);
  } else if (prop == "VALUE") {
    this->Value = value;
  } else {
    this->Properties.SetProperty(prop, value);
  }
}

void cmCacheManager::CacheEntry::SetProperty(const std::string& p, bool v)
{
  this->SetProperty(p, v ? std::string{ "ON" } : std::string{ "OFF" });
}

void cmCacheManager::CacheEntry::SetProperty(const std::string& prop,
                                             std::nullptr_t)
{
  if (prop == "TYPE") {
    this->Type = cmState::StringToCacheEntryType("STRING");
  } else if (prop == "VALUE") {
    this->Value = "";
  } else {
    this->Properties.SetProperty(prop, cmValue{ nullptr });
  }
}

void cmCacheManager::CacheEntry::AppendProperty(const std::string& prop,
                                                const std::string& value,
                                                bool asString)
{
  if (prop == "TYPE") {
    this->Type =
      cmState::StringToCacheEntryType(!value.empty() ? value : "STRING");
  } else if (prop == "VALUE") {
    if (!value.empty()) {
      if (!this->Value.empty() && !asString) {
        this->Value += ";";
      }
      this->Value += value;
    }
  } else {
    this->Properties.AppendProperty(prop, value, asString);
  }
}
