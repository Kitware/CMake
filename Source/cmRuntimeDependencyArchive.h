/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

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
    const std::vector<std::string>& preIncludeRegexes,
    const std::vector<std::string>& preExcludeRegexes,
    const std::vector<std::string>& postIncludeRegexes,
    const std::vector<std::string>& postExcludeRegexes,
    std::vector<std::string> postIncludeFiles,
    std::vector<std::string> postExcludeFiles,
    std::vector<std::string> postExcludeFilesStrict);
  bool Prepare();
  bool GetRuntimeDependencies(const std::vector<std::string>& executables,
                              const std::vector<std::string>& libraries,
                              const std::vector<std::string>& modules);

  void SetError(const std::string& e);

  const std::string& GetBundleExecutable() const;
  const std::vector<std::string>& GetSearchDirectories() const;
  const std::string& GetGetRuntimeDependenciesTool() const;
  bool GetGetRuntimeDependenciesCommand(
    const std::string& search, std::vector<std::string>& command) const;
  bool IsPreExcluded(const std::string& name) const;
  bool IsPostExcluded(const std::string& name) const;

  void AddResolvedPath(const std::string& name, const std::string& path,
                       bool& unique, std::vector<std::string> rpaths = {});
  void AddUnresolvedPath(const std::string& name);

  cmMakefile* GetMakefile() const;
  const std::map<std::string, std::set<std::string>>& GetResolvedPaths() const;
  const std::set<std::string>& GetUnresolvedPaths() const;
  const std::map<std::string, std::vector<std::string>>& GetRPaths() const;

  static bool PlatformSupportsRuntimeDependencies(const std::string& platform);

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
