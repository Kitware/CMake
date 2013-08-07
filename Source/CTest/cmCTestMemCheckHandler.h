/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCTestMemCheckHandler_h
#define cmCTestMemCheckHandler_h


#include "cmCTestTestHandler.h"
#include "cmListFileCache.h"

class cmMakefile;

/** \class cmCTestMemCheckHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestMemCheckHandler : public cmCTestTestHandler
{
  friend class cmCTestRunTest;
public:
  cmTypeMacro(cmCTestMemCheckHandler, cmCTestTestHandler);

  void PopulateCustomVectors(cmMakefile *mf);

  cmCTestMemCheckHandler();

  void Initialize();
protected:
  virtual int PreProcessHandler();
  virtual int PostProcessHandler();
  virtual void GenerateTestCommand(std::vector<std::string>& args, int test);

private:

  enum { // Memory checkers
    UNKNOWN = 0,
    VALGRIND,
    PURIFY,
    BOUNDS_CHECKER
  };
public:
  enum { // Memory faults
    ABR = 0,
    ABW,
    ABWL,
    COR,
    EXU,
    FFM,
    FIM,
    FMM,
    FMR,
    FMW,
    FUM,
    IPR,
    IPW,
    MAF,
    MLK,
    MPK,
    NPR,
    ODS,
    PAR,
    PLK,
    UMC,
    UMR,
    NO_MEMORY_FAULT
  };
private:
  enum { // Program statuses
    NOT_RUN = 0,
    TIMEOUT,
    SEGFAULT,
    ILLEGAL,
    INTERRUPT,
    NUMERICAL,
    OTHER_FAULT,
    FAILED,
    BAD_COMMAND,
    COMPLETED
  };
  std::string              BoundsCheckerDPBDFile;
  std::string              BoundsCheckerXMLFile;
  std::string              MemoryTester;
  std::vector<cmStdString> MemoryTesterDynamicOptions;
  std::vector<cmStdString> MemoryTesterOptions;
  int                      MemoryTesterStyle;
  std::string              MemoryTesterOutputFile;
  int                      MemoryTesterGlobalResults[NO_MEMORY_FAULT];

  ///! Initialize memory checking subsystem.
  bool InitializeMemoryChecking();

  /**
   * Generate the Dart compatible output
   */
  void GenerateDartOutput(std::ostream& os);

  std::vector<cmStdString> CustomPreMemCheck;
  std::vector<cmStdString> CustomPostMemCheck;

  //! Parse Valgrind/Purify/Bounds Checker result out of the output
  //string. After running, log holds the output and results hold the
  //different memmory errors.
  bool ProcessMemCheckOutput(const std::string& str,
                             std::string& log, int* results);
  bool ProcessMemCheckValgrindOutput(const std::string& str,
                                     std::string& log, int* results);
  bool ProcessMemCheckPurifyOutput(const std::string& str,
                                   std::string& log, int* results);
  bool ProcessMemCheckBoundsCheckerOutput(const std::string& str,
                                          std::string& log, int* results);

  void PostProcessPurifyTest(cmCTestTestResult& res, int test);
  void PostProcessBoundsCheckerTest(cmCTestTestResult& res, int test);
  void PostProcessValgrindTest(cmCTestTestResult& res, int test);

  ///! append MemoryTesterOutputFile to the test log
  void appendMemTesterOutput(cmCTestTestHandler::cmCTestTestResult& res,
                             int test);

  ///! generate the output filename for the given test index
  cmStdString testOutputFileName(int test);
};

#endif

