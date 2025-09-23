/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "cmListFileCache.h"

class cmMessenger;
struct cmGlobCacheEntry;

/** \class cmGlobVerificationManager
 * \brief Class for expressing build-time dependencies on glob expressions.
 *
 * Generates a CMake script which verifies glob outputs during prebuild.
 *
 */
class cmGlobVerificationManager
{
protected:
  //! Save verification script for given makefile.
  //! Saves to output <path>/<CMakeFilesDirectory>/VerifyGlobs.cmake
  bool SaveVerificationScript(std::string const& path, cmMessenger* messenger);

  //! Add an entry into the glob cache
  void AddCacheEntry(cmGlobCacheEntry const& entry,
                     std::string const& variable,
                     cmListFileBacktrace const& bt, cmMessenger* messenger);

  //! Get all cache entries
  std::vector<cmGlobCacheEntry> GetCacheEntries() const;

  //! Clear the glob cache for state reset.
  void Reset();

  //! Check targets should be written in generated build system.
  bool DoWriteVerifyTarget() const;

  //! Get the paths to the generated script and stamp files
  std::string const& GetVerifyScript() const { return this->VerifyScript; }
  std::string const& GetVerifyStamp() const { return this->VerifyStamp; }

private:
  struct CacheEntryKey
  {
    bool const Recurse;
    bool const ListDirectories;
    bool const FollowSymlinks;
    std::string const Relative;
    std::string const Expression;
    CacheEntryKey(bool const rec, bool const l, bool const s, std::string rel,
                  std::string e)
      : Recurse(rec)
      , ListDirectories(l)
      , FollowSymlinks(s)
      , Relative(std::move(rel))
      , Expression(std::move(e))
    {
    }
    bool operator<(CacheEntryKey const& r) const;
    void PrintGlobCommand(std::ostream& out, std::string const& cmdVar);
  };

  struct CacheEntryValue
  {
    bool Initialized = false;
    std::vector<std::string> Files;
    std::vector<std::pair<std::string, cmListFileBacktrace>> Backtraces;
  };

  using CacheEntryMap = std::map<CacheEntryKey, CacheEntryValue>;
  CacheEntryMap Cache;
  std::string VerifyScript;
  std::string VerifyStamp;

  // Only cmState should be able to add cache values.
  // cmGlobVerificationManager should never be used directly.
  friend class cmState; // allow access to add cache values
};
