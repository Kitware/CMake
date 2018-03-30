/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenInitializer_h
#define cmQtAutoGenInitializer_h

#include "cmConfigure.h" // IWYU pragma: keep
#include "cmQtAutoGen.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class cmGeneratorTarget;

/// @brief Initializes the QtAutoGen generators
class cmQtAutoGenInitializer : public cmQtAutoGen
{
public:
  static std::string GetQtMajorVersion(cmGeneratorTarget const* target);
  static std::string GetQtMinorVersion(cmGeneratorTarget const* target,
                                       std::string const& qtVersionMajor);

  /// @brief Rcc job information
  class Qrc
  {
  public:
    Qrc()
      : Generated(false)
      , Unique(false)
    {
    }

  public:
    std::string QrcFile;
    std::string QrcName;
    std::string PathChecksum;
    std::string InfoFile;
    std::string SettingsFile;
    std::string RccFile;
    bool Generated;
    bool Unique;
    std::vector<std::string> Options;
    std::vector<std::string> Resources;
  };

public:
  cmQtAutoGenInitializer(cmGeneratorTarget* target, bool mocEnabled,
                         bool uicEnabled, bool rccEnabled,
                         std::string const& qtVersionMajor);

  void InitCustomTargets();
  void SetupCustomTargets();

private:
  void SetupCustomTargetsMoc();
  void SetupCustomTargetsUic();

  void AddGeneratedSource(std::string const& filename, GeneratorT genType);

  bool QtVersionGreaterOrEqual(unsigned long requestMajor,
                               unsigned long requestMinor) const;

  bool RccListInputs(std::string const& fileName,
                     std::vector<std::string>& files,
                     std::string& errorMessage);

private:
  cmGeneratorTarget* Target;
  bool MocEnabled;
  bool UicEnabled;
  bool RccEnabled;
  bool MultiConfig;
  // Qt
  std::string QtVersionMajor;
  std::string QtVersionMinor;
  std::string MocExecutable;
  std::string UicExecutable;
  std::string RccExecutable;
  std::vector<std::string> RccListOptions;
  // Configurations
  std::string ConfigDefault;
  std::vector<std::string> ConfigsList;
  std::string Parallel;
  // Names
  std::string AutogenTargetName;
  std::string AutogenFolder;
  std::string AutogenInfoFile;
  std::string AutogenSettingsFile;
  // Directories
  std::string DirInfo;
  std::string DirBuild;
  std::string DirWork;
  // Sources
  std::vector<std::string> Headers;
  std::vector<std::string> Sources;
  // Moc
  std::string MocPredefsCmd;
  std::set<std::string> MocSkip;
  std::string MocIncludes;
  std::map<std::string, std::string> MocIncludesConfig;
  std::string MocDefines;
  std::map<std::string, std::string> MocDefinesConfig;
  // Uic
  std::set<std::string> UicSkip;
  std::vector<std::string> UicSearchPaths;
  std::string UicOptions;
  std::map<std::string, std::string> UicOptionsConfig;
  std::vector<std::string> UicFileFiles;
  std::vector<std::vector<std::string>> UicFileOptions;
  // Rcc
  std::vector<Qrc> Qrcs;
};

#endif
