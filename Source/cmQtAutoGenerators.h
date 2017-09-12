/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenerators_h
#define cmQtAutoGenerators_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFilePathChecksum.h"
#include "cmQtAutoGen.h"
#include "cmsys/RegularExpression.hxx"

#include <map>
#include <memory> // IWYU pragma: keep
#include <set>
#include <string>
#include <vector>

class cmMakefile;

class cmQtAutoGenerators
{
  CM_DISABLE_COPY(cmQtAutoGenerators)
public:
  cmQtAutoGenerators();
  bool Run(std::string const& targetDirectory, std::string const& config);

private:
  // -- Types

  /// @brief Search key plus regular expression pair
  struct KeyRegExp
  {
    KeyRegExp() = default;

    KeyRegExp(const char* key, const char* regExp)
      : Key(key)
      , RegExp(regExp)
    {
    }

    KeyRegExp(std::string const& key, std::string const& regExp)
      : Key(key)
      , RegExp(regExp)
    {
    }

    std::string Key;
    cmsys::RegularExpression RegExp;
  };

  /// @brief Source file job
  struct SourceJob
  {
    bool Moc = false;
    bool Uic = false;
  };

  /// @brief MOC job
  struct MocJobAuto
  {
    std::string SourceFile;
    std::string BuildFileRel;
    std::set<std::string> Depends;
  };

  /// @brief MOC job
  struct MocJobIncluded : MocJobAuto
  {
    bool DependsValid = false;
    std::string Includer;
    std::string IncludeString;
  };

  /// @brief UIC job
  struct UicJob
  {
    std::string SourceFile;
    std::string BuildFileRel;
    std::string Includer;
    std::string IncludeString;
  };

  /// @brief RCC job
  struct RccJob
  {
    std::string QrcFile;
    std::string RccFile;
    std::vector<std::string> Options;
    std::vector<std::string> Inputs;
  };

  // -- Initialization
  bool InitInfoFile(cmMakefile* makefile, std::string const& targetDirectory,
                    std::string const& config);

  // -- Settings file
  void SettingsFileRead(cmMakefile* makefile);
  bool SettingsFileWrite();
  bool SettingsChanged() const
  {
    return (this->MocSettingsChanged || this->RccSettingsChanged ||
            this->UicSettingsChanged);
  }

  // -- Central processing
  bool Process();

  // -- Source parsing
  bool ParseSourceFile(std::string const& absFilename, const SourceJob& job);
  bool ParseHeaderFile(std::string const& absFilename, const SourceJob& job);
  bool ParsePostprocess();

  // -- Moc
  bool MocEnabled() const { return !this->MocExecutable.empty(); }
  bool MocSkip(std::string const& absFilename) const;
  bool MocRequired(std::string const& contentText,
                   std::string* macroName = nullptr);
  // Moc strings
  std::string MocStringMacros() const;
  std::string MocStringHeaders(std::string const& fileBase) const;
  std::string MocFindIncludedHeader(std::string const& sourcePath,
                                    std::string const& includeBase) const;
  bool MocFindIncludedFile(std::string& absFile, std::string const& sourceFile,
                           std::string const& includeString) const;
  // Moc depends
  bool MocDependFilterPush(std::string const& key, std::string const& regExp);
  void MocFindDepends(std::string const& absFilename,
                      std::string const& contentText,
                      std::set<std::string>& depends);
  // Moc
  bool MocParseSourceContent(std::string const& absFilename,
                             std::string const& contentText);
  void MocParseHeaderContent(std::string const& absFilename,
                             std::string const& contentText);

  bool MocGenerateAll();
  bool MocGenerateFile(const MocJobAuto& mocJob, bool* generated = nullptr);

  // -- Uic
  bool UicEnabled() const { return !this->UicExecutable.empty(); }
  bool UicSkip(std::string const& absFilename) const;
  bool UicParseContent(std::string const& fileName,
                       std::string const& contentText);
  bool UicFindIncludedFile(std::string& absFile, std::string const& sourceFile,
                           std::string const& includeString);
  bool UicGenerateAll();
  bool UicGenerateFile(const UicJob& uicJob);

  // -- Rcc
  bool RccEnabled() const { return !this->RccExecutable.empty(); }
  bool RccGenerateAll();
  bool RccGenerateFile(const RccJob& rccJob);

  // -- Log info
  void LogBold(std::string const& message) const;
  void LogInfo(cmQtAutoGen::Generator genType,
               std::string const& message) const;
  // -- Log warning
  void LogWarning(cmQtAutoGen::Generator genType,
                  std::string const& message) const;
  void LogFileWarning(cmQtAutoGen::Generator genType,
                      std::string const& filename,
                      std::string const& message) const;
  // -- Log error
  void LogError(cmQtAutoGen::Generator genType,
                std::string const& message) const;
  void LogFileError(cmQtAutoGen::Generator genType,
                    std::string const& filename,
                    std::string const& message) const;
  void LogCommandError(cmQtAutoGen::Generator genType,
                       std::string const& message,
                       std::vector<std::string> const& command,
                       std::string const& output) const;

  // -- Utility
  bool MakeParentDirectory(cmQtAutoGen::Generator genType,
                           std::string const& filename) const;
  bool FileDiffers(std::string const& filename, std::string const& content);
  bool FileWrite(cmQtAutoGen::Generator genType, std::string const& filename,
                 std::string const& content);
  bool FindHeader(std::string& header, std::string const& testBasePath) const;
  bool RunCommand(std::vector<std::string> const& command,
                  std::string& output) const;

  // -- Meta
  std::string InfoFile;
  std::string ConfigSuffix;
  cmQtAutoGen::MultiConfig MultiConfig;
  // -- Settings
  bool IncludeProjectDirsBefore;
  bool Verbose;
  bool ColorOutput;
  std::string SettingsFile;
  std::string SettingsStringMoc;
  std::string SettingsStringUic;
  std::string SettingsStringRcc;
  // -- Directories
  std::string ProjectSourceDir;
  std::string ProjectBinaryDir;
  std::string CurrentSourceDir;
  std::string CurrentBinaryDir;
  std::string AutogenBuildDir;
  std::string AutogenIncludeDir;
  // -- Qt environment
  std::string QtMajorVersion;
  std::string QtMinorVersion;
  std::string MocExecutable;
  std::string UicExecutable;
  std::string RccExecutable;
  // -- File lists
  std::map<std::string, SourceJob> HeaderJobs;
  std::map<std::string, SourceJob> SourceJobs;
  std::vector<std::string> HeaderExtensions;
  cmFilePathChecksum FilePathChecksum;
  // -- Moc
  bool MocSettingsChanged;
  bool MocPredefsChanged;
  bool MocRelaxedMode;
  std::string MocCompFileRel;
  std::string MocCompFileAbs;
  std::string MocPredefsFileRel;
  std::string MocPredefsFileAbs;
  std::vector<std::string> MocSkipList;
  std::vector<std::string> MocIncludePaths;
  std::vector<std::string> MocIncludes;
  std::vector<std::string> MocDefinitions;
  std::vector<std::string> MocOptions;
  std::vector<std::string> MocAllOptions;
  std::vector<std::string> MocPredefsCmd;
  std::vector<KeyRegExp> MocDependFilters;
  std::vector<KeyRegExp> MocMacroFilters;
  cmsys::RegularExpression MocRegExpInclude;
  std::vector<std::unique_ptr<MocJobIncluded>> MocJobsIncluded;
  std::vector<std::unique_ptr<MocJobAuto>> MocJobsAuto;
  // -- Uic
  bool UicSettingsChanged;
  std::vector<std::string> UicSkipList;
  std::vector<std::string> UicTargetOptions;
  std::map<std::string, std::vector<std::string>> UicOptions;
  std::vector<std::string> UicSearchPaths;
  cmsys::RegularExpression UicRegExpInclude;
  std::vector<std::unique_ptr<UicJob>> UicJobs;
  // -- Rcc
  bool RccSettingsChanged;
  std::vector<RccJob> RccJobs;
};

#endif
