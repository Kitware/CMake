/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobVerificationManager_h
#define cmGlobVerificationManager_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "cmListFileCache.h"

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
  bool SaveVerificationScript(const std::string& path);

  //! Add an entry into the glob cache
  void AddCacheEntry(bool recurse, bool listDirectories, bool followSymlinks,
                     const std::string& relative,
                     const std::string& expression,
                     const std::vector<std::string>& files,
                     const std::string& variable,
                     const cmListFileBacktrace& bt);

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
    const bool Recurse;
    const bool ListDirectories;
    const bool FollowSymlinks;
    const std::string Relative;
    const std::string Expression;
    CacheEntryKey(const bool rec, const bool l, const bool s, std::string rel,
                  std::string e)
      : Recurse(rec)
      , ListDirectories(l)
      , FollowSymlinks(s)
      , Relative(std::move(rel))
      , Expression(std::move(e))
    {
    }
    bool operator<(const CacheEntryKey& r) const;
    void PrintGlobCommand(std::ostream& out, const std::string& cmdVar);
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

#endif
