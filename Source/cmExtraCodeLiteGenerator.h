/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmExternalMakefileProjectGenerator.h"

class cmLocalGenerator;
class cmMakefile;
class cmGeneratorTarget;
class cmXMLWriter;
class cmSourceFile;

class cmExtraCodeLiteGenerator : public cmExternalMakefileProjectGenerator
{
protected:
  std::string ConfigName;
  std::string WorkspacePath;
  unsigned int CpuCount = 2;

  std::string GetCodeLiteCompilerName(cmMakefile const* mf) const;
  std::string GetConfigurationName(cmMakefile const* mf) const;
  std::string GetBuildCommand(cmMakefile const* mf,
                              std::string const& targetName) const;
  std::string GetCleanCommand(cmMakefile const* mf,
                              std::string const& targetName) const;
  std::string GetRebuildCommand(cmMakefile const* mf,
                                std::string const& targetName) const;
  std::string GetSingleFileBuildCommand(cmMakefile const* mf) const;
  std::vector<std::string> CreateProjectsByTarget(cmXMLWriter* xml);
  std::vector<std::string> CreateProjectsByProjectMaps(cmXMLWriter* xml);
  std::string CollectSourceFiles(cmMakefile const* makefile,
                                 cmGeneratorTarget const* gt,
                                 std::map<std::string, cmSourceFile*>& cFiles,
                                 std::set<std::string>& otherFiles);
  void FindMatchingHeaderfiles(std::map<std::string, cmSourceFile*>& cFiles,
                               std::set<std::string>& otherFiles);
  void CreateProjectSourceEntries(std::map<std::string, cmSourceFile*>& cFiles,
                                  std::set<std::string>& otherFiles,
                                  cmXMLWriter* xml,
                                  std::string const& projectPath,
                                  cmMakefile const* mf,
                                  std::string const& projectType,
                                  std::string const& targetName);
  void CreateFoldersAndFiles(std::set<std::string>& cFiles, cmXMLWriter& xml,
                             std::string const& projectPath);
  void CreateFoldersAndFiles(std::map<std::string, cmSourceFile*>& cFiles,
                             cmXMLWriter& xml, std::string const& projectPath);

public:
  cmExtraCodeLiteGenerator();

  static cmExternalMakefileProjectGeneratorFactory* GetFactory();

  void Generate() override;
  void CreateProjectFile(std::vector<cmLocalGenerator*> const& lgs);

  void CreateNewProjectFile(std::vector<cmLocalGenerator*> const& lgs,
                            std::string const& filename);
  void CreateNewProjectFile(cmGeneratorTarget const* lg,
                            std::string const& filename);
};
