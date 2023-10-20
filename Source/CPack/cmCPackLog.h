/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstring>
#include <memory>
#include <ostream>
#include <string>

#define cmCPack_Log(ctSelf, logType, msg)                                     \
  do {                                                                        \
    std::ostringstream cmCPackLog_msg;                                        \
    cmCPackLog_msg << msg;                                                    \
    (ctSelf)->Log(logType, __FILE__, __LINE__, cmCPackLog_msg.str().c_str()); \
  } while (false)

/** \class cmCPackLog
 * \brief A container for CPack generators
 *
 */
class cmCPackLog
{
public:
  cmCPackLog();
  ~cmCPackLog();

  cmCPackLog(const cmCPackLog&) = delete;
  cmCPackLog& operator=(const cmCPackLog&) = delete;

  enum cm_log_tags
  {
    NOTAG = 0,
    LOG_OUTPUT = 0x1,
    LOG_VERBOSE = 0x2,
    LOG_DEBUG = 0x4,
    LOG_WARNING = 0x8,
    LOG_ERROR = 0x10
  };

  //! Various signatures for logging.
  void Log(const char* file, int line, const char* msg)
  {
    this->Log(LOG_OUTPUT, file, line, msg);
  }
  void Log(const char* file, int line, const char* msg, size_t length)
  {
    this->Log(LOG_OUTPUT, file, line, msg, length);
  }
  void Log(int tag, const char* file, int line, const char* msg)
  {
    this->Log(tag, file, line, msg, strlen(msg));
  }
  void Log(int tag, const char* file, int line, const char* msg,
           size_t length);

  //! Set Verbose
  void VerboseOn() { this->SetVerbose(true); }
  void VerboseOff() { this->SetVerbose(true); }
  void SetVerbose(bool verb) { this->Verbose = verb; }
  bool GetVerbose() { return this->Verbose; }

  //! Set Debug
  void DebugOn() { this->SetDebug(true); }
  void DebugOff() { this->SetDebug(true); }
  void SetDebug(bool verb) { this->Debug = verb; }
  bool GetDebug() { return this->Debug; }

  //! Set Quiet
  void QuietOn() { this->SetQuiet(true); }
  void QuietOff() { this->SetQuiet(true); }
  void SetQuiet(bool verb) { this->Quiet = verb; }
  bool GetQuiet() { return this->Quiet; }

  //! Set the output stream
  void SetOutputStream(std::ostream* os) { this->DefaultOutput = os; }

  //! Set the error stream
  void SetErrorStream(std::ostream* os) { this->DefaultError = os; }

  //! Set the log output stream
  void SetLogOutputStream(std::ostream* os);

  //! Set the log output file. The cmCPackLog will try to create file. If it
  // cannot, it will report an error.
  bool SetLogOutputFile(const char* fname);

  //! Set the various prefixes for the logging. SetPrefix sets the generic
  // prefix that overwrites missing ones.
  void SetPrefix(std::string const& pfx) { this->Prefix = pfx; }
  void SetOutputPrefix(std::string const& pfx) { this->OutputPrefix = pfx; }
  void SetVerbosePrefix(std::string const& pfx) { this->VerbosePrefix = pfx; }
  void SetDebugPrefix(std::string const& pfx) { this->DebugPrefix = pfx; }
  void SetWarningPrefix(std::string const& pfx) { this->WarningPrefix = pfx; }
  void SetErrorPrefix(std::string const& pfx) { this->ErrorPrefix = pfx; }

private:
  bool Verbose = false;
  bool Debug = false;
  bool Quiet = false;

  bool NewLine = true;

  int LastTag = cmCPackLog::NOTAG;

  std::string Prefix;
  std::string OutputPrefix;
  std::string VerbosePrefix;
  std::string DebugPrefix;
  std::string WarningPrefix;
  std::string ErrorPrefix;

  std::ostream* DefaultOutput = nullptr;
  std::ostream* DefaultError = nullptr;

  std::ostream* LogOutput = nullptr;
  std::unique_ptr<std::ostream> LogOutputStream;
};

class cmCPackLogWrite
{
public:
  cmCPackLogWrite(const char* data, size_t length)
    : Data(data)
    , Length(length)
  {
  }

  const char* Data;
  std::streamsize Length;
};

inline std::ostream& operator<<(std::ostream& os, const cmCPackLogWrite& c)
{
  os.write(c.Data, c.Length);
  os.flush();
  return os;
}
