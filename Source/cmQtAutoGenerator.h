/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenerator_h
#define cmQtAutoGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <istream>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include <cm/string_view>

#include "cm_jsoncpp_value.h"

#include "cmFileTime.h"
#include "cmQtAutoGen.h"

/** \class cmQtAutoGenerator
 * \brief Base class for QtAutoGen generators
 */
class cmQtAutoGenerator : public cmQtAutoGen
{
public:
  // -- Types

  /** Thread safe logger.  */
  class Logger
  {
  public:
    // -- Construction
    Logger();
    ~Logger();
    // -- Verbosity
    unsigned int Verbosity() const { return this->Verbosity_; }
    void SetVerbosity(unsigned int value) { this->Verbosity_ = value; }
    void RaiseVerbosity(unsigned int value);
    bool Verbose() const { return (this->Verbosity_ != 0); }
    void SetVerbose(bool value) { this->Verbosity_ = value ? 1 : 0; }
    // -- Color output
    bool ColorOutput() const { return this->ColorOutput_; }
    void SetColorOutput(bool value);
    // -- Log info
    void Info(GenT genType, cm::string_view message) const;
    // -- Log warning
    void Warning(GenT genType, cm::string_view message) const;
    // -- Log error
    void Error(GenT genType, cm::string_view message) const;
    void ErrorCommand(GenT genType, cm::string_view message,
                      std::vector<std::string> const& command,
                      std::string const& output) const;

  private:
    static std::string HeadLine(cm::string_view title);

  private:
    mutable std::mutex Mutex_;
    unsigned int Verbosity_ = 0;
    bool ColorOutput_ = false;
  };

  /** Project directories.  */
  struct ProjectDirsT
  {
    std::string Source;
    std::string Binary;
    std::string CurrentSource;
    std::string CurrentBinary;
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
  cmQtAutoGenerator(GenT genType);
  virtual ~cmQtAutoGenerator();

  cmQtAutoGenerator(cmQtAutoGenerator const&) = delete;
  cmQtAutoGenerator& operator=(cmQtAutoGenerator const&) = delete;

  // -- Info options
  std::string const& InfoFile() const { return InfoFile_; }
  std::string const& InfoDir() const { return InfoDir_; }
  cmFileTime const& InfoFileTime() const { return InfoFileTime_; }
  std::string const& InfoConfig() const { return InfoConfig_; }

  // -- Info file parsing
  /** Info file reader class. */
  class InfoT
  {
  public:
    InfoT(cmQtAutoGenerator& gen)
      : Gen_(gen)
    {
    }

    /** Read json data from a stream.  */
    bool Read(std::istream& istr);

    /** Returns false if the JSON value isn't a string.  */
    bool GetString(std::string const& key, std::string& value,
                   bool required) const;
    bool GetStringConfig(std::string const& key, std::string& value,
                         bool required) const;
    bool GetBool(std::string const& key, bool& value, bool required) const;
    bool GetUInt(std::string const& key, unsigned int& value,
                 bool required) const;
    /** Returns false if the JSON value isn't an array.  */
    bool GetArray(std::string const& key, std::vector<std::string>& list,
                  bool required) const;
    bool GetArray(std::string const& key,
                  std::unordered_set<std::string>& list, bool required) const;
    bool GetArrayConfig(std::string const& key, std::vector<std::string>& list,
                        bool required) const;

    Json::Value const& GetValue(std::string const& key) const
    {
      return Json_[key];
    }

    /** Returns true if strings were appended to the list.  */
    static bool GetJsonArray(std::vector<std::string>& list,
                             Json::Value const& jval);
    /** Returns true if strings were found in the JSON array.  */
    static bool GetJsonArray(std::unordered_set<std::string>& list,
                             Json::Value const& jval);

    bool LogError(GenT genType, cm::string_view message) const;
    bool LogError(cm::string_view message) const;

  private:
    std::string ConfigKey(cm::string_view key) const;

  private:
    Json::Value Json_;
    cmQtAutoGenerator& Gen_;
  };

  // -- Settings file
  static std::string SettingsFind(cm::string_view content,
                                  cm::string_view key);

  // -- Directories
  ProjectDirsT const& ProjectDirs() const { return ProjectDirs_; }
  std::string MessagePath(cm::string_view path) const;

  // -- Run
  bool Run(cm::string_view infoFile, cm::string_view config);

protected:
  // -- Abstract processing interface
  virtual bool InitFromInfo(InfoT const& info) = 0;
  virtual bool Process() = 0;
  // - Utility classes
  Logger const& Log() const { return Logger_; }

private:
  // -- Generator type
  GenT GenType_;
  // -- Logging
  Logger Logger_;
  // -- Info file
  std::string InfoFile_;
  std::string InfoDir_;
  cmFileTime InfoFileTime_;
  std::string InfoConfig_;
  // -- Directories
  ProjectDirsT ProjectDirs_;
};

#endif
