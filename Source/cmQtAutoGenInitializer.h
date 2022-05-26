/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmFilePathChecksum.h"
#include "cmQtAutoGen.h"

class cmGeneratorTarget;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmQtAutoGenGlobalInitializer;
class cmSourceFile;
class cmTarget;

/** \class cmQtAutoGenerator
 * \brief Initializes the QtAutoGen generators
 */
class cmQtAutoGenInitializer : public cmQtAutoGen
{
public:
  /** String value with per configuration variants.  */
  class ConfigString
  {
  public:
    std::string Default;
    std::unordered_map<std::string, std::string> Config;
  };

  /** String values with per configuration variants.  */
  template <typename C>
  class ConfigStrings
  {
  public:
    C Default;
    std::unordered_map<std::string, C> Config;
  };

  /** rcc job.  */
  class Qrc
  {
  public:
    std::string LockFile;
    std::string QrcFile;
    std::string QrcName;
    std::string QrcPathChecksum;
    std::string InfoFile;
    ConfigString SettingsFile;
    std::string OutputFile;
    bool Generated = false;
    bool Unique = false;
    std::vector<std::string> Options;
    std::vector<std::string> Resources;
  };

  /** moc and/or uic file.  */
  struct MUFile
  {
    std::string FullPath;
    cmSourceFile* SF = nullptr;
    std::vector<size_t> Configs;
    bool Generated = false;
    bool SkipMoc = false;
    bool SkipUic = false;
    bool MocIt = false;
    bool UicIt = false;
  };
  using MUFileHandle = std::unique_ptr<MUFile>;

  /** Abstract moc/uic/rcc generator variables base class.  */
  struct GenVarsT
  {
    bool Enabled = false;
    // Generator type/name
    GenT Gen;
    cm::string_view GenNameUpper;
    // Executable
    std::string ExecutableTargetName;
    cmGeneratorTarget* ExecutableTarget = nullptr;
    std::string Executable;
    CompilerFeaturesHandle ExecutableFeatures;

    GenVarsT(GenT gen)
      : Gen(gen)
      , GenNameUpper(cmQtAutoGen::GeneratorNameUpper(gen))
    {
    }
  };

  /** @param mocExecutable The file path to the moc executable. Will be used as
     fallback to query the version
      @return The detected Qt version and the required Qt major version. */
  static std::pair<IntegerVersion, unsigned int> GetQtVersion(
    cmGeneratorTarget const* genTarget, std::string mocExecutable);

  cmQtAutoGenInitializer(cmQtAutoGenGlobalInitializer* globalInitializer,
                         cmGeneratorTarget* genTarget,
                         IntegerVersion const& qtVersion, bool mocEnabled,
                         bool uicEnabled, bool rccEnabled,
                         bool globalAutogenTarget, bool globalAutoRccTarget);

  bool InitCustomTargets();
  bool SetupCustomTargets();

private:
  /** If moc or uic is enabled, the autogen target will be generated.  */
  bool MocOrUicEnabled() const
  {
    return (this->Moc.Enabled || this->Uic.Enabled);
  }

  bool InitMoc();
  bool InitUic();
  bool InitRcc();

  bool InitScanFiles();
  bool InitAutogenTarget();
  bool InitRccTargets();

  bool SetupWriteAutogenInfo();
  bool SetupWriteRccInfo();

  cmSourceFile* RegisterGeneratedSource(std::string const& filename);
  cmSourceFile* AddGeneratedSource(std::string const& filename,
                                   GenVarsT const& genVars,
                                   bool prepend = false);
  void AddGeneratedSource(ConfigString const& filename,
                          GenVarsT const& genVars, bool prepend = false);
  void AddToSourceGroup(std::string const& fileName,
                        cm::string_view genNameUpper);
  void AddCleanFile(std::string const& fileName);

  void ConfigFileNames(ConfigString& configString, cm::string_view prefix,
                       cm::string_view suffix);
  void ConfigFileNamesAndGenex(ConfigString& configString, std::string& genex,
                               cm::string_view prefix, cm::string_view suffix);
  void ConfigFileClean(ConfigString& configString);

  std::string GetMocBuildPath(MUFile const& muf);

  bool GetQtExecutable(GenVarsT& genVars, const std::string& executable,
                       bool ignoreMissingTarget) const;

  cmQtAutoGenGlobalInitializer* GlobalInitializer = nullptr;
  cmGeneratorTarget* GenTarget = nullptr;
  cmGlobalGenerator* GlobalGen = nullptr;
  cmLocalGenerator* LocalGen = nullptr;
  cmMakefile* Makefile = nullptr;
  cmFilePathChecksum const PathCheckSum;

  // -- Configuration
  IntegerVersion QtVersion;
  unsigned int Verbosity = 0;
  bool MultiConfig = false;
  bool CMP0071Accept = false;
  bool CMP0071Warn = false;
  bool CMP0100Accept = false;
  bool CMP0100Warn = false;
  std::string ConfigDefault;
  std::vector<std::string> ConfigsList;
  std::string TargetsFolder;

  /** Common directories.  */
  struct
  {
    std::string Info;
    std::string Build;
    std::string RelativeBuild;
    std::string Work;
    ConfigString Include;
    std::string IncludeGenExp;
  } Dir;

  /** Autogen target variables.  */
  struct
  {
    std::string Name;
    bool GlobalTarget = false;
    // Settings
    unsigned int Parallel = 1;
    // Configuration files
    std::string InfoFile;
    ConfigString SettingsFile;
    ConfigString ParseCacheFile;
    // Dependencies
    bool DependOrigin = false;
    std::set<std::string> DependFiles;
    std::set<cmTarget*> DependTargets;
    std::string DepFile;
    std::string DepFileRuleName;
    // Sources to process
    std::unordered_map<cmSourceFile*, MUFileHandle> Headers;
    std::unordered_map<cmSourceFile*, MUFileHandle> Sources;
    std::vector<MUFile*> FilesGenerated;
    std::vector<cmSourceFile*> CMP0100HeadersWarn;
  } AutogenTarget;

  /** moc variables.  */
  struct MocT : public GenVarsT
  {
    MocT()
      : GenVarsT(GenT::MOC)
    {
    }

    bool RelaxedMode = false;
    bool PathPrefix = false;
    ConfigString CompilationFile;
    std::string CompilationFileGenex;
    // Compiler implicit pre defines
    std::vector<std::string> PredefsCmd;
    ConfigString PredefsFile;
    // Defines
    ConfigStrings<std::set<std::string>> Defines;
    // Includes
    ConfigStrings<std::vector<std::string>> Includes;
    // Options
    std::vector<std::string> Options;
    // Filters
    std::vector<std::string> MacroNames;
    std::vector<std::pair<std::string, std::string>> DependFilters;
    // Utility
    std::unordered_set<std::string> EmittedBuildPaths;
  } Moc;

  /** uic variables.  */
  struct UicT : public GenVarsT
  {
    using UiFileT = std::pair<std::string, std::vector<std::string>>;

    UicT()
      : GenVarsT(GenT::UIC)
    {
    }

    std::set<std::string> SkipUi;
    std::vector<std::string> UiFilesNoOptions;
    std::vector<UiFileT> UiFilesWithOptions;
    ConfigStrings<std::vector<std::string>> Options;
    std::vector<std::string> SearchPaths;
    std::vector<std::pair<ConfigString /*ui header*/, std::string /*genex*/>>
      UiHeaders;
  } Uic;

  /** rcc variables.  */
  struct RccT : public GenVarsT
  {
    RccT()
      : GenVarsT(GenT::RCC)
    {
    }

    bool GlobalTarget = false;
    std::vector<Qrc> Qrcs;
  } Rcc;
};
