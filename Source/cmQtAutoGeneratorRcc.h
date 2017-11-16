/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGeneratorRcc_h
#define cmQtAutoGeneratorRcc_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFilePathChecksum.h"
#include "cmQtAutoGen.h"

#include <string>
#include <vector>

class cmMakefile;

class cmQtAutoGeneratorRcc
{
  CM_DISABLE_COPY(cmQtAutoGeneratorRcc)
public:
  cmQtAutoGeneratorRcc();
  bool Run(std::string const& infoFile, std::string const& config);

private:
  // -- Initialization & settings
  bool InfoFileRead(cmMakefile* makefile);
  void SettingsFileRead(cmMakefile* makefile);
  bool SettingsFileWrite();
  // -- Central processing
  bool Process(cmMakefile* makefile);
  bool RccGenerate();
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
  bool RunCommand(std::vector<std::string> const& command,
                  std::string& output) const;

  // -- Info settings
  std::string InfoFile;
  std::string InfoDir;
  std::string InfoConfig;
  // -- Config settings
  std::string ConfigSuffix;
  cmQtAutoGen::MultiConfig MultiConfig;
  // -- Settings
  bool Verbose;
  bool ColorOutput;
  bool SettingsChanged;
  std::string SettingsFile;
  std::string SettingsString;
  // -- Directories
  std::string ProjectSourceDir;
  std::string ProjectBinaryDir;
  std::string CurrentSourceDir;
  std::string CurrentBinaryDir;
  std::string AutogenBuildDir;
  cmFilePathChecksum FilePathChecksum;
  // -- Qt environment
  std::string QtMajorVersion;
  std::string RccExecutable;
  // -- Job
  std::string QrcFile;
  std::string RccFile;
  std::vector<std::string> Options;
  std::vector<std::string> Inputs;
};

#endif
