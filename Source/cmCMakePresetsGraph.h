/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmJSONState.h"
#include "cmStateTypes.h" // IWYU pragma: keep

#include "CTest/cmCTestTypes.h" // IWYU pragma: keep

enum class PackageResolveMode;

class cmCMakePresetsGraph
{
public:
  std::string errors;
  cmJSONState parseState;

  enum class ArchToolsetStrategy
  {
    Set,
    External,
  };

  enum class TraceEnableMode
  {
    Disable,
    Default,
    Expand,
  };

  class CacheVariable
  {
  public:
    std::string Type;
    std::string Value;
  };

  class Condition;

  class File
  {
  public:
    std::string Filename;
    int Version;

    std::unordered_set<File*> ReachableFiles;
  };

  class Preset
  {
  public:
    Preset() = default;
    Preset(Preset&& /*other*/) = default;
    Preset(const Preset& /*other*/) = default;
    Preset& operator=(const Preset& /*other*/) = default;
    virtual ~Preset() = default;
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    Preset& operator=(Preset&& /*other*/) = default;
#else
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't.
    Preset& operator=(Preset&& /*other*/) = delete;
#endif

    std::string Name;
    std::vector<std::string> Inherits;
    bool Hidden = false;
    File* OriginFile;
    std::string DisplayName;
    std::string Description;

    std::shared_ptr<Condition> ConditionEvaluator;
    bool ConditionResult = true;

    std::map<std::string, cm::optional<std::string>> Environment;

    virtual bool VisitPresetInherit(const Preset& parent) = 0;
    virtual bool VisitPresetBeforeInherit() { return true; }

    virtual bool VisitPresetAfterInherit(int /* version */,
                                         cmJSONState* /*state*/)
    {
      return true;
    }
  };

  class ConfigurePreset : public Preset
  {
  public:
    ConfigurePreset() = default;
    ConfigurePreset(ConfigurePreset&& /*other*/) = default;
    ConfigurePreset(const ConfigurePreset& /*other*/) = default;
    ConfigurePreset& operator=(const ConfigurePreset& /*other*/) = default;
    ~ConfigurePreset() override = default;
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    ConfigurePreset& operator=(ConfigurePreset&& /*other*/) = default;
#else
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't.
    ConfigurePreset& operator=(ConfigurePreset&& /*other*/) = delete;
#endif

    std::string Generator;
    std::string Architecture;
    cm::optional<ArchToolsetStrategy> ArchitectureStrategy;
    std::string Toolset;
    cm::optional<ArchToolsetStrategy> ToolsetStrategy;
    std::string ToolchainFile;
    std::string BinaryDir;
    std::string InstallDir;

    std::map<std::string, cm::optional<CacheVariable>> CacheVariables;

    cm::optional<bool> WarnDev;
    cm::optional<bool> ErrorDev;
    cm::optional<bool> WarnDeprecated;
    cm::optional<bool> ErrorDeprecated;
    cm::optional<bool> WarnUninitialized;
    cm::optional<bool> WarnUnusedCli;
    cm::optional<bool> WarnSystemVars;

    cm::optional<bool> DebugOutput;
    cm::optional<bool> DebugTryCompile;
    cm::optional<bool> DebugFind;

    cm::optional<TraceEnableMode> TraceMode;
    cm::optional<cmTraceEnums::TraceOutputFormat> TraceFormat;
    std::vector<std::string> TraceSource;
    std::string TraceRedirect;

    bool VisitPresetInherit(const Preset& parent) override;
    bool VisitPresetBeforeInherit() override;
    bool VisitPresetAfterInherit(int version, cmJSONState* state) override;
  };

  class BuildPreset : public Preset
  {
  public:
    BuildPreset() = default;
    BuildPreset(BuildPreset&& /*other*/) = default;
    BuildPreset(const BuildPreset& /*other*/) = default;
    BuildPreset& operator=(const BuildPreset& /*other*/) = default;
    ~BuildPreset() override = default;
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    BuildPreset& operator=(BuildPreset&& /*other*/) = default;
#else
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't.
    BuildPreset& operator=(BuildPreset&& /*other*/) = delete;
#endif

    std::string ConfigurePreset;
    cm::optional<bool> InheritConfigureEnvironment;
    cm::optional<int> Jobs;
    std::vector<std::string> Targets;
    std::string Configuration;
    cm::optional<bool> CleanFirst;
    cm::optional<bool> Verbose;
    std::vector<std::string> NativeToolOptions;
    cm::optional<PackageResolveMode> ResolvePackageReferences;

    bool VisitPresetInherit(const Preset& parent) override;
    bool VisitPresetAfterInherit(int /* version */,
                                 cmJSONState* /*state*/) override;
  };

  class TestPreset : public Preset
  {
  public:
    TestPreset() = default;
    TestPreset(TestPreset&& /*other*/) = default;
    TestPreset(const TestPreset& /*other*/) = default;
    TestPreset& operator=(const TestPreset& /*other*/) = default;
    ~TestPreset() override = default;
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    TestPreset& operator=(TestPreset&& /*other*/) = default;
#else
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't.
    TestPreset& operator=(TestPreset&& /*other*/) = delete;
#endif

    struct OutputOptions
    {
      enum class VerbosityEnum
      {
        Default,
        Verbose,
        Extra
      };

      cm::optional<bool> ShortProgress;
      cm::optional<VerbosityEnum> Verbosity;
      cm::optional<bool> Debug;
      cm::optional<bool> OutputOnFailure;
      cm::optional<bool> Quiet;
      std::string OutputLogFile;
      std::string OutputJUnitFile;
      cm::optional<bool> LabelSummary;
      cm::optional<bool> SubprojectSummary;
      cm::optional<int> MaxPassedTestOutputSize;
      cm::optional<int> MaxFailedTestOutputSize;
      cm::optional<cmCTestTypes::TruncationMode> TestOutputTruncation;
      cm::optional<int> MaxTestNameWidth;
    };

    struct IncludeOptions
    {
      struct IndexOptions
      {
        cm::optional<int> Start;
        cm::optional<int> End;
        cm::optional<int> Stride;
        std::vector<int> SpecificTests;

        std::string IndexFile;
      };

      std::string Name;
      std::string Label;
      cm::optional<IndexOptions> Index;
      cm::optional<bool> UseUnion;
    };

    struct ExcludeOptions
    {
      struct FixturesOptions
      {
        std::string Any;
        std::string Setup;
        std::string Cleanup;
      };

      std::string Name;
      std::string Label;
      cm::optional<FixturesOptions> Fixtures;
    };

    struct FilterOptions
    {
      cm::optional<IncludeOptions> Include;
      cm::optional<ExcludeOptions> Exclude;
    };

    struct ExecutionOptions
    {
      enum class ShowOnlyEnum
      {
        Human,
        JsonV1
      };

      struct RepeatOptions
      {
        enum class ModeEnum
        {
          UntilFail,
          UntilPass,
          AfterTimeout
        };

        ModeEnum Mode;
        int Count;
      };

      enum class NoTestsActionEnum
      {
        Default,
        Error,
        Ignore
      };

      cm::optional<bool> StopOnFailure;
      cm::optional<bool> EnableFailover;
      cm::optional<int> Jobs;
      std::string ResourceSpecFile;
      cm::optional<int> TestLoad;
      cm::optional<ShowOnlyEnum> ShowOnly;

      cm::optional<RepeatOptions> Repeat;
      cm::optional<bool> InteractiveDebugging;
      cm::optional<bool> ScheduleRandom;
      cm::optional<int> Timeout;
      cm::optional<NoTestsActionEnum> NoTestsAction;
    };

    std::string ConfigurePreset;
    cm::optional<bool> InheritConfigureEnvironment;
    std::string Configuration;
    std::vector<std::string> OverwriteConfigurationFile;
    cm::optional<OutputOptions> Output;
    cm::optional<FilterOptions> Filter;
    cm::optional<ExecutionOptions> Execution;

    bool VisitPresetInherit(const Preset& parent) override;
    bool VisitPresetAfterInherit(int /* version */,
                                 cmJSONState* /*state*/) override;
  };

  class PackagePreset : public Preset
  {
  public:
    PackagePreset() = default;
    PackagePreset(PackagePreset&& /*other*/) = default;
    PackagePreset(const PackagePreset& /*other*/) = default;
    PackagePreset& operator=(const PackagePreset& /*other*/) = default;
    ~PackagePreset() override = default;
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    PackagePreset& operator=(PackagePreset&& /*other*/) = default;
#else
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't.
    PackagePreset& operator=(PackagePreset&& /*other*/) = delete;
#endif

    std::string ConfigurePreset;
    cm::optional<bool> InheritConfigureEnvironment;
    std::vector<std::string> Generators;
    std::vector<std::string> Configurations;
    std::map<std::string, std::string> Variables;
    std::string ConfigFile;

    cm::optional<bool> DebugOutput;
    cm::optional<bool> VerboseOutput;

    std::string PackageName;
    std::string PackageVersion;
    std::string PackageDirectory;
    std::string VendorName;

    bool VisitPresetInherit(const Preset& parent) override;
    bool VisitPresetAfterInherit(int /* version */,
                                 cmJSONState* /*state*/) override;
  };

  class WorkflowPreset : public Preset
  {
  public:
    WorkflowPreset() = default;
    WorkflowPreset(WorkflowPreset&& /*other*/) = default;
    WorkflowPreset(const WorkflowPreset& /*other*/) = default;
    WorkflowPreset& operator=(const WorkflowPreset& /*other*/) = default;
    ~WorkflowPreset() override = default;
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    WorkflowPreset& operator=(WorkflowPreset&& /*other*/) = default;
#else
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't.
    WorkflowPreset& operator=(WorkflowPreset&& /*other*/) = delete;
#endif

    class WorkflowStep
    {
    public:
      enum class Type
      {
        Configure,
        Build,
        Test,
        Package,
      };
      Type PresetType;
      std::string PresetName;
    };

    std::vector<WorkflowStep> Steps;

    bool VisitPresetInherit(const Preset& parent) override;
    bool VisitPresetAfterInherit(int /* version */,
                                 cmJSONState* /* state */) override;
  };

  template <class T>
  class PresetPair
  {
  public:
    T Unexpanded;
    cm::optional<T> Expanded;
  };

  std::map<std::string, PresetPair<ConfigurePreset>> ConfigurePresets;
  std::map<std::string, PresetPair<BuildPreset>> BuildPresets;
  std::map<std::string, PresetPair<TestPreset>> TestPresets;
  std::map<std::string, PresetPair<PackagePreset>> PackagePresets;
  std::map<std::string, PresetPair<WorkflowPreset>> WorkflowPresets;

  std::vector<std::string> ConfigurePresetOrder;
  std::vector<std::string> BuildPresetOrder;
  std::vector<std::string> TestPresetOrder;
  std::vector<std::string> PackagePresetOrder;
  std::vector<std::string> WorkflowPresetOrder;

  std::string SourceDir;
  std::vector<std::unique_ptr<File>> Files;

  int GetVersion(const Preset& preset) const
  {
    return preset.OriginFile->Version;
  }

  static std::string GetFilename(const std::string& sourceDir);
  static std::string GetUserFilename(const std::string& sourceDir);
  bool ReadProjectPresets(const std::string& sourceDir,
                          bool allowNoFiles = false);

  std::string GetGeneratorForPreset(const std::string& presetName) const
  {
    auto configurePresetName = presetName;

    auto buildPresetIterator = this->BuildPresets.find(presetName);
    if (buildPresetIterator != this->BuildPresets.end()) {
      configurePresetName =
        buildPresetIterator->second.Unexpanded.ConfigurePreset;
    } else {
      auto testPresetIterator = this->TestPresets.find(presetName);
      if (testPresetIterator != this->TestPresets.end()) {
        configurePresetName =
          testPresetIterator->second.Unexpanded.ConfigurePreset;
      }
    }

    auto configurePresetIterator =
      this->ConfigurePresets.find(configurePresetName);
    if (configurePresetIterator != this->ConfigurePresets.end()) {
      return configurePresetIterator->second.Unexpanded.Generator;
    }

    // This should only happen if the preset is hidden
    // or (for build or test presets) if ConfigurePreset is invalid.
    return "";
  }

  enum class PrintPrecedingNewline
  {
    False,
    True,
  };
  static void printPrecedingNewline(PrintPrecedingNewline* p);

  static void PrintPresets(
    const std::vector<const cmCMakePresetsGraph::Preset*>& presets);
  void PrintConfigurePresetList(
    PrintPrecedingNewline* newline = nullptr) const;
  void PrintConfigurePresetList(
    const std::function<bool(const ConfigurePreset&)>& filter,
    PrintPrecedingNewline* newline = nullptr) const;
  void PrintBuildPresetList(PrintPrecedingNewline* newline = nullptr) const;
  void PrintTestPresetList(PrintPrecedingNewline* newline = nullptr) const;
  void PrintPackagePresetList(PrintPrecedingNewline* newline = nullptr) const;
  void PrintPackagePresetList(
    const std::function<bool(const PackagePreset&)>& filter,
    PrintPrecedingNewline* newline = nullptr) const;
  void PrintWorkflowPresetList(PrintPrecedingNewline* newline = nullptr) const;
  void PrintAllPresets() const;

private:
  enum class RootType
  {
    Project,
    User,
  };

  enum class ReadReason
  {
    Root,
    Included,
  };

  bool ReadProjectPresetsInternal(bool allowNoFiles);
  bool ReadJSONFile(const std::string& filename, RootType rootType,
                    ReadReason readReason, std::vector<File*>& inProgressFiles,
                    File*& file, std::string& errMsg);
  void ClearPresets();
};
