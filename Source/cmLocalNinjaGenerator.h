/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmLocalCommonGenerator.h"
#include "cmNinjaTypes.h"
#include "cmOutputConverter.h"

class cmCustomCommand;
class cmCustomCommandGenerator;
class cmGeneratedFileStream;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmGlobalNinjaGenerator;
class cmListFileBacktrace;
class cmMakefile;
class cmRulePlaceholderExpander;
class cmake;

/**
 * \class cmLocalNinjaGenerator
 * \brief Write a local build.ninja file.
 *
 * cmLocalNinjaGenerator produces a local build.ninja file from its
 * member Makefile.
 */
class cmLocalNinjaGenerator : public cmLocalCommonGenerator
{
public:
  cmLocalNinjaGenerator(cmGlobalGenerator* gg, cmMakefile* mf);

  ~cmLocalNinjaGenerator() override;

  void Generate() override;

  cmRulePlaceholderExpander* CreateRulePlaceholderExpander() const override;

  std::string GetTargetDirectory(
    cmGeneratorTarget const* target) const override;

  const cmGlobalNinjaGenerator* GetGlobalNinjaGenerator() const;
  cmGlobalNinjaGenerator* GetGlobalNinjaGenerator();

  const cmake* GetCMakeInstance() const;
  cmake* GetCMakeInstance();

  /// @returns the relative path between the HomeOutputDirectory and this
  /// local generators StartOutputDirectory.
  std::string GetHomeRelativeOutputPath() const
  {
    return this->HomeRelativeOutputPath;
  }

  std::string BuildCommandLine(
    std::vector<std::string> const& cmdLines, std::string const& outputConfig,
    std::string const& commandConfig,
    std::string const& customStep = std::string(),
    cmGeneratorTarget const* target = nullptr) const;

  void AppendTargetOutputs(cmGeneratorTarget* target, cmNinjaDeps& outputs,
                           const std::string& config);
  void AppendTargetDepends(cmGeneratorTarget* target, cmNinjaDeps& outputs,
                           const std::string& config,
                           const std::string& fileConfig,
                           cmNinjaTargetDepends depends);

  std::string CreateUtilityOutput(std::string const& targetName,
                                  std::vector<std::string> const& byproducts,
                                  cmListFileBacktrace const& bt) override;

  std::vector<cmCustomCommandGenerator> MakeCustomCommandGenerators(
    cmCustomCommand const& cc, std::string const& config) override;

  void AddCustomCommandTarget(cmCustomCommand const* cc,
                              cmGeneratorTarget* target);
  void AppendCustomCommandLines(cmCustomCommandGenerator const& ccg,
                                std::vector<std::string>& cmdLines);
  void AppendCustomCommandDeps(cmCustomCommandGenerator const& ccg,
                               cmNinjaDeps& ninjaDeps,
                               const std::string& config);

  bool HasUniqueByproducts(std::vector<std::string> const& byproducts,
                           cmListFileBacktrace const& bt);

protected:
  std::string ConvertToIncludeReference(
    std::string const& path, cmOutputConverter::OutputFormat format) override;

private:
  cmGeneratedFileStream& GetImplFileStream(const std::string& config) const;
  cmGeneratedFileStream& GetCommonFileStream() const;
  cmGeneratedFileStream& GetRulesFileStream() const;

  void WriteBuildFileTop();
  void WriteProjectHeader(std::ostream& os);
  void WriteNinjaRequiredVersion(std::ostream& os);
  void WriteNinjaConfigurationVariable(std::ostream& os,
                                       const std::string& config);
  void WriteNinjaFilesInclusionConfig(std::ostream& os);
  void WriteNinjaFilesInclusionCommon(std::ostream& os);
  void WriteNinjaWorkDir(std::ostream& os);
  void WriteProcessedMakefile(std::ostream& os);
  void WritePools(std::ostream& os);

  void WriteCustomCommandBuildStatement(
    cmCustomCommand const* cc, const std::set<cmGeneratorTarget*>& targets,
    const std::string& config);

  void WriteCustomCommandBuildStatements(const std::string& config);

  std::string MakeCustomLauncher(cmCustomCommandGenerator const& ccg);

  std::string WriteCommandScript(std::vector<std::string> const& cmdLines,
                                 std::string const& outputConfig,
                                 std::string const& commandConfig,
                                 std::string const& customStep,
                                 cmGeneratorTarget const* target) const;

  void AdditionalCleanFiles(const std::string& config);

  std::string HomeRelativeOutputPath;

  using CustomCommandTargetMap =
    std::map<cmCustomCommand const*, std::set<cmGeneratorTarget*>>;
  CustomCommandTargetMap CustomCommandTargets;
  std::vector<cmCustomCommand const*> CustomCommands;
};
