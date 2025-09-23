/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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

  cmSearchPath(cmSearchPath const&) = default;
  cmSearchPath& operator=(cmSearchPath const&) = default;

  struct PathWithPrefix
  {
    std::string Path;
    std::string Prefix;

    bool operator<(PathWithPrefix const& other) const
    {
      return this->Path < other.Path ||
        (this->Path == other.Path && this->Prefix < other.Prefix);
    }
  };
  std::vector<PathWithPrefix> const& GetPaths() const { return this->Paths; }
  std::size_t size() const { return this->Paths.size(); }

  void ExtractWithout(std::set<std::string> const& ignorePaths,
                      std::set<std::string> const& ignorePrefixes,
                      std::vector<std::string>& outPaths) const;

  void AddPath(std::string const& path);
  void AddUserPath(std::string const& path);
  void AddCMakePath(std::string const& variable);
  void AddEnvPath(std::string const& variable);
  void AddCMakePrefixPath(std::string const& variable);
  void AddEnvPrefixPath(std::string const& variable, bool stripBin = false);
  void AddSuffixes(std::vector<std::string> const& suffixes);
  void AddPrefixPaths(std::vector<std::string> const& paths);

protected:
  void AddPathInternal(std::string const& path, std::string const& prefix);

  cmFindCommon* FC;
  std::vector<PathWithPrefix> Paths;
};
