/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

class cmCMakePresetsFile
{
public:
  enum class ReadFileResult
  {
    READ_OK,
    FILE_NOT_FOUND,
    JSON_PARSE_ERROR,
    INVALID_ROOT,
    NO_VERSION,
    INVALID_VERSION,
    UNRECOGNIZED_VERSION,
    INVALID_CMAKE_VERSION,
    UNRECOGNIZED_CMAKE_VERSION,
    INVALID_PRESETS,
    INVALID_PRESET,
    INVALID_VARIABLE,
    DUPLICATE_PRESETS,
    CYCLIC_PRESET_INHERITANCE,
    USER_PRESET_INHERITANCE,
    INVALID_MACRO_EXPANSION,
    BUILD_TEST_PRESETS_UNSUPPORTED,
    INVALID_CONFIGURE_PRESET,
  };

  enum class ArchToolsetStrategy
  {
    Set,
    External,
  };

  class CacheVariable
  {
  public:
    std::string Type;
    std::string Value;
  };

  class Preset
  {
  public:
#if __cplusplus < 201703L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201703L)
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't. Disable the
    // move assignment operator until C++17 is enabled.
    // Explicitly defining a copy assignment operator prevents the compiler
    // from automatically generating a move assignment operator.
    Preset& operator=(const Preset& /*other*/) = default;
#endif

    virtual ~Preset() = default;

    std::string Name;
    std::vector<std::string> Inherits;
    bool Hidden;
    bool User;
    std::string DisplayName;
    std::string Description;

    std::map<std::string, cm::optional<std::string>> Environment;

    virtual ReadFileResult VisitPresetInherit(const Preset& parent) = 0;
    virtual ReadFileResult VisitPresetBeforeInherit()
    {
      return ReadFileResult::READ_OK;
    }

    virtual ReadFileResult VisitPresetAfterInherit()
    {
      return ReadFileResult::READ_OK;
    }
  };

  class ConfigurePreset : public Preset
  {
  public:
#if __cplusplus < 201703L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201703L)
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't. Disable the
    // move assignment operator until C++17 is enabled.
    // Explicitly defining a copy assignment operator prevents the compiler
    // from automatically generating a move assignment operator.
    ConfigurePreset& operator=(const ConfigurePreset& /*other*/) = default;
#endif

    std::string Generator;
    std::string Architecture;
    cm::optional<ArchToolsetStrategy> ArchitectureStrategy;
    std::string Toolset;
    cm::optional<ArchToolsetStrategy> ToolsetStrategy;
    std::string BinaryDir;

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

    ReadFileResult VisitPresetInherit(const Preset& parent) override;
    ReadFileResult VisitPresetBeforeInherit() override;
    ReadFileResult VisitPresetAfterInherit() override;
  };

  class BuildPreset : public Preset
  {
  public:
#if __cplusplus < 201703L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201703L)
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't. Disable the
    // move assignment operator until C++17 is enabled.
    // Explicitly defining a copy assignment operator prevents the compiler
    // from automatically generating a move assignment operator.
    BuildPreset& operator=(const BuildPreset& /*other*/) = default;
#endif

    std::string ConfigurePreset;
    cm::optional<bool> InheritConfigureEnvironment;
    cm::optional<int> Jobs;
    std::vector<std::string> Targets;
    std::string Configuration;
    cm::optional<bool> CleanFirst;
    cm::optional<bool> Verbose;
    std::vector<std::string> NativeToolOptions;

    ReadFileResult VisitPresetInherit(const Preset& parent) override;
    ReadFileResult VisitPresetAfterInherit() override;
  };

  class TestPreset : public Preset
  {
  public:
#if __cplusplus < 201703L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201703L)
    // The move assignment operators for several STL classes did not become
    // noexcept until C++17, which causes some tools to warn about this move
    // assignment operator throwing an exception when it shouldn't. Disable the
    // move assignment operator until C++17 is enabled.
    // Explicitly defining a copy assignment operator prevents the compiler
    // from automatically generating a move assignment operator.
    TestPreset& operator=(const TestPreset& /*other*/) = default;
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
      cm::optional<bool> LabelSummary;
      cm::optional<bool> SubprojectSummary;
      cm::optional<int> MaxPassedTestOutputSize;
      cm::optional<int> MaxFailedTestOutputSize;
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

    ReadFileResult VisitPresetInherit(const Preset& parent) override;
    ReadFileResult VisitPresetAfterInherit() override;
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

  std::vector<std::string> ConfigurePresetOrder;
  std::vector<std::string> BuildPresetOrder;
  std::vector<std::string> TestPresetOrder;

  std::string SourceDir;

  static std::string GetFilename(const std::string& sourceDir);
  static std::string GetUserFilename(const std::string& sourceDir);
  ReadFileResult ReadProjectPresets(const std::string& sourceDir,
                                    bool allowNoFiles = false);
  static const char* ResultToString(ReadFileResult result);

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

  static void PrintPresets(
    const std::vector<const cmCMakePresetsFile::Preset*>& presets);
  void PrintConfigurePresetList() const;
  void PrintConfigurePresetList(
    const std::function<bool(const ConfigurePreset&)>& filter) const;
  void PrintBuildPresetList() const;
  void PrintTestPresetList() const;
  void PrintAllPresets() const;

private:
  ReadFileResult ReadProjectPresetsInternal(bool allowNoFiles);
  ReadFileResult ReadJSONFile(const std::string& filename, bool user);
  void ClearPresets();
};
