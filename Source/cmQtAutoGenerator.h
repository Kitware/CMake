/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenerator_h
#define cmQtAutoGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFilePathChecksum.h"
#include "cmQtAutoGen.h"

#include <mutex>
#include <string>
#include <vector>

class cmMakefile;

/// @brief Base class for QtAutoGen gernerators
class cmQtAutoGenerator : public cmQtAutoGen
{
public:
  // -- Types

  /// @brief Thread safe logging
  class Logger
  {
  public:
    // -- Construction
    Logger();
    ~Logger();
    // -- Verbosity
    unsigned int Verbosity() const { return this->Verbosity_; }
    void SetVerbosity(unsigned int value) { this->Verbosity_ = value; }
    void RaiseVerbosity(std::string const& value);
    bool Verbose() const { return (this->Verbosity_ != 0); }
    void SetVerbose(bool value) { this->Verbosity_ = value ? 1 : 0; }
    // -- Color output
    bool ColorOutput() const { return this->ColorOutput_; }
    void SetColorOutput(bool value);
    // -- Log info
    void Info(GenT genType, std::string const& message);
    // -- Log warning
    void Warning(GenT genType, std::string const& message);
    void WarningFile(GenT genType, std::string const& filename,
                     std::string const& message);
    // -- Log error
    void Error(GenT genType, std::string const& message);
    void ErrorFile(GenT genType, std::string const& filename,
                   std::string const& message);
    void ErrorCommand(GenT genType, std::string const& message,
                      std::vector<std::string> const& command,
                      std::string const& output);

  private:
    static std::string HeadLine(std::string const& title);

  private:
    std::mutex Mutex_;
    unsigned int Verbosity_ = 0;
    bool ColorOutput_ = false;
  };

  // -- File system methods
  static bool MakeParentDirectory(std::string const& filename);
  static bool FileRead(std::string& content, std::string const& filename,
                       std::string* error = nullptr);
  static bool FileWrite(std::string const& filename,
                        std::string const& content,
                        std::string* error = nullptr);

  /// @brief Thread safe file system interface
  class FileSystem
  {
  public:
    FileSystem();
    ~FileSystem();

    // -- Paths
    /// @brief Wrapper for cmSystemTools::GetRealPath
    std::string GetRealPath(std::string const& filename);
    /// @brief Wrapper for cmSystemTools::CollapseFullPath
    std::string CollapseFullPath(std::string const& file,
                                 std::string const& dir);
    /// @brief Wrapper for cmSystemTools::SplitPath
    void SplitPath(const std::string& p, std::vector<std::string>& components,
                   bool expand_home_dir = true);
    /// @brief Wrapper for cmSystemTools::JoinPath
    std::string JoinPath(const std::vector<std::string>& components);
    /// @brief Wrapper for cmSystemTools::JoinPath
    std::string JoinPath(std::vector<std::string>::const_iterator first,
                         std::vector<std::string>::const_iterator last);
    /// @brief Wrapper for cmSystemTools::GetFilenameWithoutLastExtension
    std::string GetFilenameWithoutLastExtension(const std::string& filename);
    /// @brief Wrapper for cmQtAutoGen::SubDirPrefix
    std::string SubDirPrefix(std::string const& filename);
    /// @brief Wrapper for cmFilePathChecksum::setupParentDirs
    void setupFilePathChecksum(std::string const& currentSrcDir,
                               std::string const& currentBinDir,
                               std::string const& projectSrcDir,
                               std::string const& projectBinDir);
    /// @brief Wrapper for cmFilePathChecksum::getPart
    std::string GetFilePathChecksum(std::string const& filename);

    // -- File access
    /// @brief Wrapper for cmSystemTools::FileExists
    bool FileExists(std::string const& filename);
    /// @brief Wrapper for cmSystemTools::FileExists
    bool FileExists(std::string const& filename, bool isFile);
    /// @brief Wrapper for cmSystemTools::FileLength
    unsigned long FileLength(std::string const& filename);
    bool FileIsOlderThan(std::string const& buildFile,
                         std::string const& sourceFile,
                         std::string* error = nullptr);

    bool FileRead(std::string& content, std::string const& filename,
                  std::string* error = nullptr);

    bool FileWrite(std::string const& filename, std::string const& content,
                   std::string* error = nullptr);

    bool FileDiffers(std::string const& filename, std::string const& content);

    bool FileRemove(std::string const& filename);
    bool Touch(std::string const& filename, bool create = false);

    // -- Directory access
    bool MakeDirectory(std::string const& dirname);
    bool MakeParentDirectory(std::string const& filename);

  private:
    std::mutex Mutex_;
    cmFilePathChecksum FilePathChecksum_;
  };

public:
  // -- Constructors
  cmQtAutoGenerator();
  virtual ~cmQtAutoGenerator();

  cmQtAutoGenerator(cmQtAutoGenerator const&) = delete;
  cmQtAutoGenerator& operator=(cmQtAutoGenerator const&) = delete;

  // -- Run
  bool Run(std::string const& infoFile, std::string const& config);

  // InfoFile
  std::string const& InfoFile() const { return InfoFile_; }
  std::string const& InfoDir() const { return InfoDir_; }
  std::string const& InfoConfig() const { return InfoConfig_; }

  // -- Utility
  static std::string SettingsFind(std::string const& content, const char* key);

protected:
  // -- Abstract processing interface
  virtual bool Init(cmMakefile* makefile) = 0;
  virtual bool Process() = 0;

private:
  // -- Info settings
  std::string InfoFile_;
  std::string InfoDir_;
  std::string InfoConfig_;
};

#endif
