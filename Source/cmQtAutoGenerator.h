/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenerator_h
#define cmQtAutoGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFileTime.h"
#include "cmQtAutoGen.h"

#include <mutex>
#include <string>
#include <vector>

class cmMakefile;

/** \class cmQtAutoGenerator
 * \brief Base class for QtAutoGen generators
 */
class cmQtAutoGenerator : public cmQtAutoGen
{
public:
  // -- Types

  /**
   * Thread safe logger
   */
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
    void Info(GenT genType, std::string const& message) const;
    // -- Log warning
    void Warning(GenT genType, std::string const& message) const;
    void WarningFile(GenT genType, std::string const& filename,
                     std::string const& message) const;
    // -- Log error
    void Error(GenT genType, std::string const& message) const;
    void ErrorFile(GenT genType, std::string const& filename,
                   std::string const& message) const;
    void ErrorCommand(GenT genType, std::string const& message,
                      std::vector<std::string> const& command,
                      std::string const& output) const;

  private:
    static std::string HeadLine(std::string const& title);

  private:
    mutable std::mutex Mutex_;
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
  static bool FileDiffers(std::string const& filename,
                          std::string const& content);

public:
  // -- Constructors
  cmQtAutoGenerator();
  virtual ~cmQtAutoGenerator();

  cmQtAutoGenerator(cmQtAutoGenerator const&) = delete;
  cmQtAutoGenerator& operator=(cmQtAutoGenerator const&) = delete;

  // -- Run
  bool Run(std::string const& infoFile, std::string const& config);

  // -- InfoFile
  std::string const& InfoFile() const { return InfoFile_; }
  cmFileTime const& InfoFileTime() const { return InfoFileTime_; }
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
  cmFileTime InfoFileTime_;
  std::string InfoDir_;
  std::string InfoConfig_;
};

#endif
