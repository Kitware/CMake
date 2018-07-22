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
    std::string LockFile;
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

  bool InitCustomTargets();
  bool SetupCustomTargets();

private:
  bool InitCustomTargetsMoc();
  bool InitCustomTargetsUic();
  bool InitCustomTargetsRcc();

  bool SetupWriteAutogenInfo();
  bool SetupWriteRccInfo();

  void AddGeneratedSource(std::string const& filename, GeneratorT genType);

  bool QtVersionGreaterOrEqual(unsigned long requestMajor,
                               unsigned long requestMinor) const;

  bool GetMocExecutable();
  bool GetUicExecutable();
  bool GetRccExecutable();

  bool RccListInputs(std::string const& fileName,
                     std::vector<std::string>& files,
                     std::string& errorMessage);

private:
  cmGeneratorTarget* Target;
  bool MultiConfig = false;
  // Qt
  std::string QtVersionMajor;
  std::string QtVersionMinor;
  // Configurations
  std::string ConfigDefault;
  std::vector<std::string> ConfigsList;
  std::string Parallel;
  std::string Verbosity;
  // Names
  std::string AutogenTargetName;
  std::string AutogenFolder;
  std::string AutogenInfoFile;
  std::string AutogenSettingsFile;
  // Directories
  std::string DirInfo;
  std::string DirBuild;
  std::string DirWork;
  std::string DirInclude;
  std::map<std::string, std::string> DirConfigInclude;
  // Sources
  std::vector<std::string> Headers;
  std::vector<std::string> Sources;
  // Moc
  struct
  {
    bool Enabled = false;
    std::string Executable;
    std::string PredefsCmd;
    std::set<std::string> Skip;
    std::string Includes;
    std::map<std::string, std::string> ConfigIncludes;
    std::string Defines;
    std::map<std::string, std::string> ConfigDefines;
    std::string MocsCompilation;
  } Moc;
  // Uic
  struct
  {
    bool Enabled = false;
    std::string Executable;
    std::set<std::string> Skip;
    std::vector<std::string> SearchPaths;
    std::string Options;
    std::map<std::string, std::string> ConfigOptions;
    std::vector<std::string> FileFiles;
    std::vector<std::vector<std::string>> FileOptions;
  } Uic;
  // Rcc
  struct
  {
    bool Enabled = false;
    std::string Executable;
    std::vector<std::string> ListOptions;
    std::vector<Qrc> Qrcs;
  } Rcc;
};

#endif
