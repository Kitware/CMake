/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "cmCTest.h"
#include "cmSystemTools.h"
#include "cmValue.h"

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
  using t_StringToMultiString =
    std::map<std::string, std::vector<std::string>>;

  /**
   * Options collect a single value from flags; passing the
   * flag multiple times on the command-line *overwrites* values,
   * and only the last one specified counts. Set an option to
   * nullptr to "unset" it.
   *
   * The value is stored as a string. The values set for single
   * and multi-options (see below) live in different spaces,
   * so calling a single-getter for a key that has only been set
   * as a multi-value will return nullptr.
   */
  void SetPersistentOption(const std::string& op, const std::string& value);
  void SetPersistentOption(const std::string& op, cmValue value);
  void SetOption(const std::string& op, const std::string& value);
  void SetOption(const std::string& op, cmValue value);
  cmValue GetOption(const std::string& op);

  /**
   * Multi-Options collect one or more values from flags; passing
   * the flag multiple times on the command-line *adds* values,
   * rather than overwriting the previous values.
   *
   * Adding an empty value does nothing.
   *
   * The value is stored as a vector of strings. The values set for single
   * (see above) and multi-options live in different spaces,
   * so calling a multi-getter for a key that has only been set
   * as a single-value will return an empty vector.
   */
  void AddPersistentMultiOption(const std::string& optionName,
                                const std::string& value);
  void AddMultiOption(const std::string& optionName, const std::string& value);
  std::vector<std::string> GetMultiOption(const std::string& op) const;

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
  t_StringToMultiString MultiOptions;
  t_StringToMultiString PersistentMultiOptions;
  t_StringToString LogFileNames;

  int SubmitIndex;
};
