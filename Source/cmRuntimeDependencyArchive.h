/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmBinUtilsLinker.h"

class cmExecutionStatus;
class cmMakefile;

class cmRuntimeDependencyArchive
{
public:
  explicit cmRuntimeDependencyArchive(
    cmExecutionStatus& status, std::vector<std::string> searchDirectories,
    std::string bundleExecutable,
    std::vector<std::string> const& preIncludeRegexes,
    std::vector<std::string> const& preExcludeRegexes,
    std::vector<std::string> const& postIncludeRegexes,
    std::vector<std::string> const& postExcludeRegexes,
    std::vector<std::string> postIncludeFiles,
    std::vector<std::string> postExcludeFiles,
    std::vector<std::string> postExcludeFilesStrict);
  bool Prepare();
  bool GetRuntimeDependencies(std::vector<std::string> const& executables,
                              std::vector<std::string> const& libraries,
                              std::vector<std::string> const& modules);

  void SetError(std::string const& e);

  std::string const& GetBundleExecutable() const;
  std::vector<std::string> const& GetSearchDirectories() const;
  std::string const& GetGetRuntimeDependenciesTool() const;
  bool GetGetRuntimeDependenciesCommand(
    std::string const& search, std::vector<std::string>& command) const;
  bool IsPreExcluded(std::string const& name) const;
  bool IsPostExcluded(std::string const& name) const;

  void AddResolvedPath(std::string const& name, std::string const& path,
                       bool& unique, std::vector<std::string> rpaths = {});
  void AddUnresolvedPath(std::string const& name);

  cmMakefile* GetMakefile() const;
  std::map<std::string, std::set<std::string>> const& GetResolvedPaths() const;
  std::set<std::string> const& GetUnresolvedPaths() const;
  std::map<std::string, std::vector<std::string>> const& GetRPaths() const;

  static bool PlatformSupportsRuntimeDependencies(std::string const& platform);

private:
  cmExecutionStatus& Status;
  std::unique_ptr<cmBinUtilsLinker> Linker;

  std::string GetRuntimeDependenciesTool;
  std::vector<std::string> GetRuntimeDependenciesCommand;

  std::vector<std::string> SearchDirectories;
  std::string BundleExecutable;
  std::vector<cmsys::RegularExpression> PreIncludeRegexes;
  std::vector<cmsys::RegularExpression> PreExcludeRegexes;
  std::vector<cmsys::RegularExpression> PostIncludeRegexes;
  std::vector<cmsys::RegularExpression> PostExcludeRegexes;
  std::vector<std::string> PostIncludeFiles;
  std::vector<std::string> PostExcludeFiles;
  std::vector<std::string> PostExcludeFilesStrict;
  std::map<std::string, std::set<std::string>> ResolvedPaths;
  std::set<std::string> UnresolvedPaths;
  std::map<std::string, std::vector<std::string>> RPaths;
};
