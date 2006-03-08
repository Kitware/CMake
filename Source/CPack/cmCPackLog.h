/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackLog_h
#define cmCPackLog_h

#include "cmObject.h"

#define cmCPack_Log(ctSelf, logType, msg) \
  do { \
  cmOStringStream cmCPackLog_msg; \
  cmCPackLog_msg << msg; \
  (ctSelf)->Log(logType, __FILE__, __LINE__, cmCPackLog_msg.str().c_str());\
  } while ( 0 )

#ifdef cerr
#  undef cerr
#endif
#define cerr no_cerr_use_cmCPack_Log

#ifdef cout
#  undef cout
#endif
#define cout no_cout_use_cmCPack_Log


/** \class cmCPackLog
 * \brief A container for CPack generators
 *
 */
class cmCPackLog : public cmObject
{
public:
  cmTypeMacro(cmCPackLog, cmObject);

  cmCPackLog();
  ~cmCPackLog();

  enum __log_tags {
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
  void SetVerbose(bool verb) { m_Verbose = verb; }
  bool GetVerbose() { return m_Verbose; }

  //! Set Debug
  void DebugOn() { this->SetDebug(true); }
  void DebugOff() { this->SetDebug(true); }
  void SetDebug(bool verb) { m_Debug = verb; }
  bool GetDebug() { return m_Debug; }

  //! Set Quiet
  void QuietOn() { this->SetQuiet(true); }
  void QuietOff() { this->SetQuiet(true); }
  void SetQuiet(bool verb) { m_Quiet = verb; }
  bool GetQuiet() { return m_Quiet; }

  //! Set the output stream
  void SetOutputStream(std::ostream* os) { m_DefaultOutput = os; }

  //! Set the error stream
  void SetErrorStream(std::ostream* os) { m_DefaultError = os; }

  //! Set the log output stream
  void SetLogOutputStream(std::ostream* os);

  //! Set the log output file. The cmCPackLog will try to create file. If it
  // cannot, it will report an error.
  bool SetLogOutputFile(const char* fname);

  //! Set the various prefixes for the logging. SetPrefix sets the generic
  // prefix that overwrittes missing ones.
  void SetPrefix(std::string pfx) { m_Prefix = pfx; }
  void SetOutputPrefix(std::string pfx) { m_OutputPrefix = pfx; }
  void SetVerbosePrefix(std::string pfx) { m_VerbosePrefix = pfx; }
  void SetDebugPrefix(std::string pfx) { m_DebugPrefix = pfx; }
  void SetWarningPrefix(std::string pfx) { m_WarningPrefix = pfx; }
  void SetErrorPrefix(std::string pfx) { m_ErrorPrefix = pfx; }

private:
  bool m_Verbose;
  bool m_Debug;
  bool m_Quiet;

  bool m_NewLine;

  int m_LastTag;

  std::string m_Prefix;
  std::string m_OutputPrefix;
  std::string m_VerbosePrefix;
  std::string m_DebugPrefix;
  std::string m_WarningPrefix;
  std::string m_ErrorPrefix;

  std::ostream *m_DefaultOutput;
  std::ostream *m_DefaultError;

  std::string m_LogOutputFileName;
  std::ostream *m_LogOutput;
  // Do we need to cleanup log output stream
  bool m_LogOutputCleanup;
};

class cmCPackLogWrite
{
public:
  cmCPackLogWrite(const char* data, size_t length)
    : Data(data), Length(length) {}

  const char* Data;
  size_t Length;
};

inline std::ostream& operator<< (std::ostream& os, const cmCPackLogWrite& c)
{
  os.write(c.Data, c.Length);
  os.flush();
  return os;
}

#endif
