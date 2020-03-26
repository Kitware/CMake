/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLoadCacheCommand.h"

#include <set>

#include "cmsys/FStream.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmake.h"

static bool ReadWithPrefix(std::vector<std::string> const& args,
                           cmExecutionStatus& status);

static void CheckLine(cmMakefile& mf, std::string const& prefix,
                      std::set<std::string> const& variablesToRead,
                      const char* line);

bool cmLoadCacheCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with wrong number of arguments.");
    return false;
  }

  if (args.size() >= 2 && args[1] == "READ_WITH_PREFIX") {
    return ReadWithPrefix(args, status);
  }

  if (status.GetMakefile().GetCMakeInstance()->GetWorkingMode() ==
      cmake::SCRIPT_MODE) {
    status.SetError(
      "Only load_cache(READ_WITH_PREFIX) may be used in script mode");
    return false;
  }

  // Cache entries to be excluded from the import list.
  // If this set is empty, all cache entries are brought in
  // and they can not be overridden.
  bool excludeFiles = false;
  std::set<std::string> excludes;

  for (std::string const& arg : args) {
    if (excludeFiles) {
      excludes.insert(arg);
    }
    if (arg == "EXCLUDE") {
      excludeFiles = true;
    }
    if (excludeFiles && (arg == "INCLUDE_INTERNALS")) {
      break;
    }
  }

  // Internal cache entries to be imported.
  // If this set is empty, no internal cache entries are
  // brought in.
  bool includeFiles = false;
  std::set<std::string> includes;

  for (std::string const& arg : args) {
    if (includeFiles) {
      includes.insert(arg);
    }
    if (arg == "INCLUDE_INTERNALS") {
      includeFiles = true;
    }
    if (includeFiles && (arg == "EXCLUDE")) {
      break;
    }
  }

  cmMakefile& mf = status.GetMakefile();

  // Loop over each build directory listed in the arguments.  Each
  // directory has a cache file.
  for (std::string const& arg : args) {
    if ((arg == "EXCLUDE") || (arg == "INCLUDE_INTERNALS")) {
      break;
    }
    mf.GetCMakeInstance()->LoadCache(arg, false, excludes, includes);
  }

  return true;
}

static bool ReadWithPrefix(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  // Make sure we have a prefix.
  if (args.size() < 3) {
    status.SetError("READ_WITH_PREFIX form must specify a prefix.");
    return false;
  }

  // Make sure the cache file exists.
  std::string cacheFile = args[0] + "/CMakeCache.txt";
  if (!cmSystemTools::FileExists(cacheFile)) {
    std::string e = "Cannot load cache file from " + cacheFile;
    status.SetError(e);
    return false;
  }

  // Prepare the table of variables to read.
  std::string const prefix = args[2];
  std::set<std::string> const variablesToRead(args.begin() + 3, args.end());

  // Read the cache file.
  cmsys::ifstream fin(cacheFile.c_str());

  cmMakefile& mf = status.GetMakefile();

  // This is a big hack read loop to overcome a buggy ifstream
  // implementation on HP-UX.  This should work on all platforms even
  // for small buffer sizes.
  const int bufferSize = 4096;
  char buffer[bufferSize];
  std::string line;
  while (fin) {
    // Read a block of the file.
    fin.read(buffer, bufferSize);
    if (fin.gcount()) {
      // Parse for newlines directly.
      const char* i = buffer;
      const char* end = buffer + fin.gcount();
      while (i != end) {
        const char* begin = i;
        while (i != end && *i != '\n') {
          ++i;
        }
        if (i == begin || *(i - 1) != '\r') {
          // Include this portion of the line.
          line += std::string(begin, i - begin);
        } else {
          // Include this portion of the line.
          // Don't include the \r in a \r\n pair.
          line += std::string(begin, i - 1 - begin);
        }
        if (i != end) {
          // Completed a line.
          CheckLine(mf, prefix, variablesToRead, line.c_str());
          line.clear();

          // Skip the newline character.
          ++i;
        }
      }
    }
  }
  if (!line.empty()) {
    // Partial last line.
    CheckLine(mf, prefix, variablesToRead, line.c_str());
  }

  return true;
}

static void CheckLine(cmMakefile& mf, std::string const& prefix,
                      std::set<std::string> const& variablesToRead,
                      const char* line)
{
  // Check one line of the cache file.
  std::string var;
  std::string value;
  cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;
  if (cmake::ParseCacheEntry(line, var, value, type)) {
    // Found a real entry.  See if this one was requested.
    if (variablesToRead.find(var) != variablesToRead.end()) {
      // This was requested.  Set this variable locally with the given
      // prefix.
      var = prefix + var;
      if (!value.empty()) {
        mf.AddDefinition(var, value);
      } else {
        mf.RemoveDefinition(var);
      }
    }
  }
}
