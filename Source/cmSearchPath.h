/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <set>
#include <string>
#include <vector>

class cmFindCommon;

/** \class cmSearchPath
 * \brief Container for encapsulating a set of search paths
 *
 * cmSearchPath is a container that encapsulates search path construction and
 * management
 */
class cmSearchPath
{
public:
  // cmSearchPath must be initialized from a valid pointer.  The only reason
  // for the default is to allow it to be easily used in stl containers.
  // Attempting to initialize with a NULL value will fail an assertion
  cmSearchPath(cmFindCommon* findCmd = nullptr);
  ~cmSearchPath();

  cmSearchPath(const cmSearchPath&) = default;
  cmSearchPath& operator=(const cmSearchPath&) = default;

  struct PathWithPrefix
  {
    std::string Path;
    std::string Prefix;

    bool operator<(const PathWithPrefix& other) const
    {
      return this->Path < other.Path ||
        (this->Path == other.Path && this->Prefix < other.Prefix);
    }
  };
  const std::vector<PathWithPrefix>& GetPaths() const { return this->Paths; }
  std::size_t size() const { return this->Paths.size(); }

  void ExtractWithout(const std::set<std::string>& ignorePaths,
                      const std::set<std::string>& ignorePrefixes,
                      std::vector<std::string>& outPaths,
                      bool clear = false) const;

  void AddPath(const std::string& path);
  void AddUserPath(const std::string& path);
  void AddCMakePath(const std::string& variable);
  void AddEnvPath(const std::string& variable);
  void AddCMakePrefixPath(const std::string& variable);
  void AddEnvPrefixPath(const std::string& variable, bool stripBin = false);
  void AddSuffixes(const std::vector<std::string>& suffixes);
  void AddPrefixPaths(const std::vector<std::string>& paths,
                      const char* base = nullptr);

protected:
  void AddPathInternal(const std::string& path, const std::string& prefix,
                       const char* base = nullptr);

  cmFindCommon* FC;
  std::vector<PathWithPrefix> Paths;
};
