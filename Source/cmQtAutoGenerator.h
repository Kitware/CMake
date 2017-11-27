/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenerator_h
#define cmQtAutoGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmQtAutoGen.h"

#include <string>
#include <vector>

class cmMakefile;

class cmQtAutoGenerator
{
  CM_DISABLE_COPY(cmQtAutoGenerator)
public:
  cmQtAutoGenerator();
  virtual ~cmQtAutoGenerator() = default;
  bool Run(std::string const& infoFile, std::string const& config);

  std::string const& GetInfoFile() const { return InfoFile; }
  std::string const& GetInfoDir() const { return InfoDir; }
  std::string const& GetInfoConfig() const { return InfoConfig; }
  bool GetVerbose() const { return Verbose; }

protected:
  // -- Central processing
  virtual bool Process(cmMakefile* makefile) = 0;

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
  bool FileIsOlderThan(std::string const& buildFile,
                       std::string const& sourceFile,
                       std::string* error = nullptr);
  bool FileRead(std::string& content, std::string const& filename,
                std::string* error = nullptr);
  bool FileWrite(cmQtAutoGen::Generator genType, std::string const& filename,
                 std::string const& content);
  bool FileDiffers(std::string const& filename, std::string const& content);
  bool RunCommand(std::vector<std::string> const& command,
                  std::string& output) const;

private:
  // -- Info settings
  std::string InfoFile;
  std::string InfoDir;
  std::string InfoConfig;
  // -- Settings
  bool Verbose;
  bool ColorOutput;
};

#endif
