/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCTest.h"
#include "cmSystemTools.h"

class cmGeneratedFileStream;
class cmMakefile;
class cmake;

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
   * Get the CTest instance
   */
  cmCTest* GetCTestInstance() { return this->CTest; }

  /**
   * Construct handler
   */
  cmCTestGenericHandler(cmCTest* ctest);
  virtual ~cmCTestGenericHandler();

  void SetSubmitIndex(int idx) { this->SubmitIndex = idx; }
  int GetSubmitIndex() { return this->SubmitIndex; }

  void SetAppendXML(bool b) { this->AppendXML = b; }
  void SetQuiet(bool b) { this->Quiet = b; }
  bool GetQuiet() { return this->Quiet; }
  void SetTestLoad(unsigned long load) { this->TestLoad = load; }
  unsigned long GetTestLoad() const { return this->TestLoad; }

  void SetCMakeInstance(cmake* cm) { this->CMake = cm; }

protected:
  bool StartResultingXML(cmCTest::Part part, char const* name,
                         cmGeneratedFileStream& xofs);
  bool StartLogFile(char const* name, cmGeneratedFileStream& xofs);

  bool AppendXML = false;
  bool Quiet = false;
  unsigned long TestLoad = 0;
  cmSystemTools::OutputOption HandlerVerbose = cmSystemTools::OUTPUT_NONE;
  cmCTest* CTest;
  cmake* CMake = nullptr;

  int SubmitIndex = 0;
};
