/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGeneratorMocUic_h
#define cmQtAutoGeneratorMocUic_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFilePathChecksum.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenerator.h"
#include "cmsys/RegularExpression.hxx"

#include <map>
#include <memory> // IWYU pragma: keep
#include <set>
#include <string>
#include <vector>

class cmMakefile;

class cmQtAutoGeneratorMocUic : public cmQtAutoGenerator
{
  CM_DISABLE_COPY(cmQtAutoGeneratorMocUic)
public:
  cmQtAutoGeneratorMocUic();

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

  // -- Initialization
  bool InitInfoFile(cmMakefile* makefile);

  // -- Settings file
  void SettingsFileRead(cmMakefile* makefile);
  bool SettingsFileWrite();
  bool SettingsChanged() const
  {
    return (this->MocSettingsChanged || this->UicSettingsChanged);
  }

  // -- Central processing
  bool Process(cmMakefile* makefile) override;

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

  // -- Utility
  bool FindHeader(std::string& header, std::string const& testBasePath) const;

  // -- Meta
  std::string ConfigSuffix;
  cmQtAutoGen::MultiConfig MultiConfig;
  // -- Settings
  bool IncludeProjectDirsBefore;
  std::string SettingsFile;
  std::string SettingsStringMoc;
  std::string SettingsStringUic;
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
};

#endif
