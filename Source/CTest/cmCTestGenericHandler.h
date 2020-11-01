/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include <stddef.h>

#include "cmCTest.h"
#include "cmSystemTools.h"

class cmCTestCommand;
class cmGeneratedFileStream;
class cmMakefile;

/** \class cmCTestGenericHandler
 * \brief A superclass of all CTest Handlers
 *
 */
class cmCTestGenericHandler
{
public:
  /**
   * If verbose then more information is printed out
   */
  void SetVerbose(bool val)
  {
    this->HandlerVerbose =
      val ? cmSystemTools::OUTPUT_MERGE : cmSystemTools::OUTPUT_NONE;
  }

  /**
   * Populate internals from CTest custom scripts
   */
  virtual void PopulateCustomVectors(cmMakefile*) {}

  /**
   * Do the actual processing. Subclass has to override it.
   * Return < 0 if error.
   */
  virtual int ProcessHandler() = 0;

  /**
   * Process command line arguments that are applicable for the handler
   */
  virtual int ProcessCommandLineArguments(
    const std::string& /*currentArg*/, size_t& /*idx*/,
    const std::vector<std::string>& /*allArgs*/)
  {
    return 1;
  }

  /**
   * Initialize handler
   */
  virtual void Initialize();

  /**
   * Set the CTest instance
   */
  void SetCTestInstance(cmCTest* ctest) { this->CTest = ctest; }
  cmCTest* GetCTestInstance() { return this->CTest; }

  /**
   * Construct handler
   */
  cmCTestGenericHandler();
  virtual ~cmCTestGenericHandler();

  using t_StringToString = std::map<std::string, std::string>;

  void SetPersistentOption(const std::string& op, const char* value);
  void SetOption(const std::string& op, const char* value);
  const char* GetOption(const std::string& op);

  void SetCommand(cmCTestCommand* command) { this->Command = command; }

  void SetSubmitIndex(int idx) { this->SubmitIndex = idx; }
  int GetSubmitIndex() { return this->SubmitIndex; }

  void SetAppendXML(bool b) { this->AppendXML = b; }
  void SetQuiet(bool b) { this->Quiet = b; }
  bool GetQuiet() { return this->Quiet; }
  void SetTestLoad(unsigned long load) { this->TestLoad = load; }
  unsigned long GetTestLoad() const { return this->TestLoad; }

protected:
  bool StartResultingXML(cmCTest::Part part, const char* name,
                         cmGeneratedFileStream& xofs);
  bool StartLogFile(const char* name, cmGeneratedFileStream& xofs);

  bool AppendXML;
  bool Quiet;
  unsigned long TestLoad;
  cmSystemTools::OutputOption HandlerVerbose;
  cmCTest* CTest;
  t_StringToString Options;
  t_StringToString PersistentOptions;
  t_StringToString LogFileNames;

  cmCTestCommand* Command;
  int SubmitIndex;
};
