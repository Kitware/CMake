/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmGlobalGenerator.h"

class cmake;
class cmGeneratorTarget;
class cmLocalGenerator;

/** \class cmGlobalCommonGenerator
 * \brief Common infrastructure for Makefile and Ninja global generators.
 */
class cmGlobalCommonGenerator : public cmGlobalGenerator
{
public:
  cmGlobalCommonGenerator(cmake* cm);
  ~cmGlobalCommonGenerator() override;

  struct DirectoryTarget
  {
    cmLocalGenerator* LG = nullptr;
    struct Target
    {
      cmGeneratorTarget const* GT = nullptr;
      std::vector<std::string> ExcludedFromAllInConfigs;
    };
    std::vector<Target> Targets;
    struct Dir
    {
      std::string Path;
      bool ExcludeFromAll = false;
    };
    std::vector<Dir> Children;
  };
  std::map<std::string, DirectoryTarget> ComputeDirectoryTargets() const;
  bool IsExcludedFromAllInConfig(const DirectoryTarget::Target& t,
                                 const std::string& config);
  void AddClangTidyExportFixesDir(const std::string& dir)
  {
    this->ClangTidyExportFixesDirs.insert(dir);
  }
  void AddClangTidyExportFixesFile(const std::string& file)
  {
    this->ClangTidyExportFixesFiles.insert(file);
  }

protected:
  virtual bool SupportsDirectConsole() const { return true; }
  const char* GetEditCacheTargetName() const override { return "edit_cache"; }
  std::string GetEditCacheCommand() const override;

  std::set<std::string> ClangTidyExportFixesDirs;
  std::set<std::string> ClangTidyExportFixesFiles;
  void RemoveUnknownClangTidyExportFixesFiles() const;
};
