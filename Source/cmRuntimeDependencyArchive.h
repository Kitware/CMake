/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmRuntimeDependencyArchive_h
#define cmRuntimeDependencyArchive_h

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
    const std::vector<std::string>& postExcludeRegexes);
  bool Prepare();
  bool GetRuntimeDependencies(const std::vector<std::string>& executables,
                              const std::vector<std::string>& libraries,
                              const std::vector<std::string>& modules);

  void SetError(const std::string& e);

  std::string GetBundleExecutable();
  const std::vector<std::string>& GetSearchDirectories();
  std::string GetGetRuntimeDependenciesTool();
  bool GetGetRuntimeDependenciesCommand(const std::string& search,
                                        std::vector<std::string>& command);
  bool IsPreExcluded(const std::string& name);
  bool IsPostExcluded(const std::string& name);

  void AddResolvedPath(const std::string& name, const std::string& path,
                       bool& unique);
  void AddUnresolvedPath(const std::string& name);

  cmMakefile* GetMakefile();
  const std::map<std::string, std::set<std::string>>& GetResolvedPaths();
  const std::set<std::string>& GetUnresolvedPaths();

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
  std::map<std::string, std::set<std::string>> ResolvedPaths;
  std::set<std::string> UnresolvedPaths;
};

#endif // cmRuntimeDependencyArchive_h
