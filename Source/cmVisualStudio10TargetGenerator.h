/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cmGeneratorTarget.h"
#include "cmVsProjectType.h"

class cmComputeLinkInformation;
class cmCustomCommand;
class cmCustomCommandGenerator;
class cmGeneratedFileStream;
class cmGlobalVisualStudio10Generator;
class cmLocalVisualStudio10Generator;
class cmMakefile;
class cmSourceFile;
class cmSourceGroup;
class cmVS10GeneratorOptions;

class cmVisualStudio10TargetGenerator
{
public:
  cmVisualStudio10TargetGenerator(cmGeneratorTarget* target,
                                  cmGlobalVisualStudio10Generator* gg);
  ~cmVisualStudio10TargetGenerator();

  cmVisualStudio10TargetGenerator(cmVisualStudio10TargetGenerator const&) =
    delete;
  cmVisualStudio10TargetGenerator& operator=(
    cmVisualStudio10TargetGenerator const&) = delete;

  void Generate();

private:
  struct ToolSource
  {
    cmSourceFile const* SourceFile;
    bool RelativePath;
  };
  struct ToolSources : public std::vector<ToolSource>
  {
  };

  struct TargetsFileAndConfigs
  {
    std::string File;
    std::vector<std::string> Configs;
  };

  struct Elem;
  struct OptionsHelper;

  using ConfigToSettings =
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>;

  std::string ConvertPath(std::string const& path, bool forceRelative);
  std::string CalcCondition(const std::string& config) const;
  void WriteProjectConfigurations(Elem& e0);
  void WriteProjectConfigurationValues(Elem& e0);
  void WriteMSToolConfigurationValues(Elem& e1, std::string const& config);
  void WriteCEDebugProjectConfigurationValues(Elem& e0);
  void WriteMSToolConfigurationValuesManaged(Elem& e1,
                                             std::string const& config);
  void WriteHeaderSource(Elem& e1, cmSourceFile const* sf,
                         ConfigToSettings const& toolSettings);
  void WriteExtraSource(Elem& e1, cmSourceFile const* sf,
                        ConfigToSettings& toolSettings);
  void WriteNsightTegraConfigurationValues(Elem& e1,
                                           std::string const& config);
  void WriteAndroidConfigurationValues(Elem& e1, std::string const& config);
  void WriteSource(Elem& e2, cmSourceFile const* sf);
  void FinishWritingSource(Elem& e2, ConfigToSettings const& toolSettings);
  void WriteExcludeFromBuild(Elem& e2,
                             std::vector<size_t> const& exclude_configs);
  void WriteAllSources(Elem& e0);
  void WritePackageReferences(Elem& e0);
  void WriteDotNetReferences(Elem& e0);
  void WriteDotNetReference(Elem& e1, std::string const& ref,
                            std::string const& hint,
                            std::string const& config);
  void WriteDotNetDocumentationFile(Elem& e0);
  void WriteImports(Elem& e0);
  void WriteDotNetReferenceCustomTags(Elem& e2, std::string const& ref);
  void WriteEmbeddedResourceGroup(Elem& e0);
  void WriteWinRTReferences(Elem& e0);
  void WriteWinRTPackageCertificateKeyFile(Elem& e0);
  void WriteXamlFilesGroup(Elem& e0);
  void WritePathAndIncrementalLinkOptions(Elem& e0);
  void WritePublicProjectContentOptions(Elem& e0);
  void WriteItemDefinitionGroups(Elem& e0);
  void VerifyNecessaryFiles();
  void WriteMissingFiles(Elem& e1);
  void WriteMissingFilesWP80(Elem& e1);
  void WriteMissingFilesWP81(Elem& e1);
  void WriteMissingFilesWS80(Elem& e1);
  void WriteMissingFilesWS81(Elem& e1);
  void WriteMissingFilesWS10_0(Elem& e1);
  void WritePlatformExtensions(Elem& e1);
  void WriteSinglePlatformExtension(Elem& e1, std::string const& extension,
                                    std::string const& version);
  void WriteSDKReferences(Elem& e0);
  void WriteSingleSDKReference(Elem& e1, std::string const& extension,
                               std::string const& version);
  void WriteCommonMissingFiles(Elem& e1, const std::string& manifestFile);
  void WriteTargetSpecificReferences(Elem& e0);
  void WriteTargetsFileReferences(Elem& e1);

  std::vector<std::string> GetIncludes(std::string const& config,
                                       std::string const& lang) const;
  std::string GetTargetOutputName() const;

  bool ComputeClOptions();
  bool ComputeClOptions(std::string const& configName);
  void WriteClOptions(Elem& e1, std::string const& config);
  bool ComputeRcOptions();
  bool ComputeRcOptions(std::string const& config);
  void WriteRCOptions(Elem& e1, std::string const& config);
  bool ComputeCudaOptions();
  bool ComputeCudaOptions(std::string const& config);
  void WriteCudaOptions(Elem& e1, std::string const& config);

  bool ComputeCudaLinkOptions();
  bool ComputeCudaLinkOptions(std::string const& config);
  void WriteCudaLinkOptions(Elem& e1, std::string const& config);

  bool ComputeMarmasmOptions();
  bool ComputeMarmasmOptions(std::string const& config);
  void WriteMarmasmOptions(Elem& e1, std::string const& config);
  bool ComputeMasmOptions();
  bool ComputeMasmOptions(std::string const& config);
  void WriteMasmOptions(Elem& e1, std::string const& config);
  bool ComputeNasmOptions();
  bool ComputeNasmOptions(std::string const& config);
  void WriteNasmOptions(Elem& e1, std::string const& config);

  bool ComputeLinkOptions();
  bool ComputeLinkOptions(std::string const& config);
  bool ComputeLibOptions();
  bool ComputeLibOptions(std::string const& config);
  void WriteLinkOptions(Elem& e1, std::string const& config);
  void WriteMidlOptions(Elem& e1, std::string const& config);
  void WriteAntBuildOptions(Elem& e1, std::string const& config);
  void OutputLinkIncremental(Elem& e1, std::string const& configName);
  void WriteCustomRule(Elem& e0, cmSourceFile const* source,
                       cmCustomCommand const& command);
  enum class BuildInParallel
  {
    No,
    Yes,
  };
  void WriteCustomRuleCpp(Elem& e2, std::string const& config,
                          std::string const& script,
                          std::string const& additional_inputs,
                          std::string const& outputs,
                          std::string const& comment,
                          cmCustomCommandGenerator const& ccg, bool symbolic,
                          BuildInParallel buildInParallel);
  void WriteCustomRuleCSharp(Elem& e0, std::string const& config,
                             std::string const& commandName,
                             std::string const& script,
                             std::string const& inputs,
                             std::string const& outputs,
                             std::string const& comment,
                             cmCustomCommandGenerator const& ccg);
  void WriteCustomCommands(Elem& e0);
  void WriteCustomCommand(Elem& e0, cmSourceFile const* sf);
  void WriteGroups();
  void WriteProjectReferences(Elem& e0);
  void WriteApplicationTypeSettings(Elem& e1);
  void OutputSourceSpecificFlags(Elem& e2, cmSourceFile const* source);
  void AddLibraries(const cmComputeLinkInformation& cli,
                    std::vector<std::string>& libVec,
                    std::vector<std::string>& vsTargetVec,
                    const std::string& config);
  void AddTargetsFileAndConfigPair(std::string const& targetsFile,
                                   std::string const& config);
  void WriteLibOptions(Elem& e1, std::string const& config);
  void WriteManifestOptions(Elem& e1, std::string const& config);
  void WriteEvents(Elem& e1, std::string const& configName);
  void WriteEvent(Elem& e1, std::string const& name,
                  std::vector<cmCustomCommand> const& commands,
                  std::string const& configName);
  void WriteGroupSources(Elem& e0, std::string const& name,
                         ToolSources const& sources,
                         std::vector<cmSourceGroup>&);
  void AddMissingSourceGroups(std::set<cmSourceGroup const*>& groupsUsed,
                              const std::vector<cmSourceGroup>& allGroups);
  bool IsResxHeader(const std::string& headerFile);
  bool IsXamlHeader(const std::string& headerFile);
  bool IsXamlSource(const std::string& headerFile);

  bool ForceOld(const std::string& source) const;

  void GetCSharpSourceProperties(cmSourceFile const* sf,
                                 std::map<std::string, std::string>& tags);
  void WriteCSharpSourceProperties(
    Elem& e2, const std::map<std::string, std::string>& tags);
  std::string GetCSharpSourceLink(cmSourceFile const* source);

  void WriteStdOutEncodingUtf8(Elem& e1);
  void UpdateCache();

  friend class cmVS10GeneratorOptions;
  using Options = cmVS10GeneratorOptions;
  using OptionsMap = std::map<std::string, std::unique_ptr<Options>>;
  OptionsMap ClOptions;
  OptionsMap RcOptions;
  OptionsMap CudaOptions;
  OptionsMap CudaLinkOptions;
  OptionsMap MarmasmOptions;
  OptionsMap MasmOptions;
  OptionsMap NasmOptions;
  OptionsMap LinkOptions;
  std::string LangForClCompile;

  VsProjectType ProjectType;
  bool InSourceBuild;
  std::vector<std::string> Configurations;
  std::vector<TargetsFileAndConfigs> TargetsFileAndConfigsVec;
  cmGeneratorTarget* const GeneratorTarget;
  cmMakefile* const Makefile;
  std::string const Platform;
  std::string const Name;
  std::string const GUID;
  bool MSTools;
  bool Managed;
  bool NsightTegra;
  bool Android;
  bool HaveCustomCommandDepfile = false;
  unsigned int NsightTegraVersion[4];
  bool TargetCompileAsWinRT;
  std::set<std::string> IPOEnabledConfigurations;
  std::set<std::string> ASanEnabledConfigurations;
  std::set<std::string> FuzzerEnabledConfigurations;
  std::map<std::string, std::string> SpectreMitigation;
  cmGlobalVisualStudio10Generator* const GlobalGenerator;
  cmLocalVisualStudio10Generator* const LocalGenerator;
  std::set<std::string> CSharpCustomCommandNames;
  bool IsMissingFiles;
  std::vector<std::string> AddedFiles;
  std::string DefaultArtifactDir;
  bool AddedDefaultCertificate = false;
  // managed C++/C# relevant members
  using DotNetHintReference = std::pair<std::string, std::string>;
  using DotNetHintReferenceList = std::vector<DotNetHintReference>;
  using DotNetHintReferenceMap =
    std::map<std::string, DotNetHintReferenceList>;
  DotNetHintReferenceMap DotNetHintReferences;
  using UsingDirectories = std::set<std::string>;
  using UsingDirectoriesMap = std::map<std::string, UsingDirectories>;
  UsingDirectoriesMap AdditionalUsingDirectories;

  using ToolSourceMap = std::map<std::string, ToolSources>;
  ToolSourceMap Tools;

  std::set<std::string> ExpectedResxHeaders;
  std::set<std::string> ExpectedXamlHeaders;
  std::set<std::string> ExpectedXamlSources;
  std::vector<cmSourceFile const*> ResxObjs;
  std::vector<cmSourceFile const*> XamlObjs;
  void ClassifyAllConfigSources();
  void ClassifyAllConfigSource(cmGeneratorTarget::AllConfigSource const& acs);

  // .Net SDK-stype project variable and helper functions
  void WriteClassicMsBuildProjectFile(cmGeneratedFileStream& BuildFileStream);
  void WriteSdkStyleProjectFile(cmGeneratedFileStream& BuildFileStream);

  void WriteCommonPropertyGroupGlobals(
    cmVisualStudio10TargetGenerator::Elem& e1);

  bool HasCustomCommands() const;

  std::unordered_map<std::string, ConfigToSettings> ParsedToolTargetSettings;
  bool PropertyIsSameInAllConfigs(const ConfigToSettings& toolSettings,
                                  const std::string& propName);
  void ParseSettingsProperty(const std::string& settingsPropertyValue,
                             ConfigToSettings& toolSettings);
  std::string GetCMakeFilePath(const char* name) const;
};
