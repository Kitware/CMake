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
public:
  void PopulateCustomVectors(cmMakefile *mf);
  
  cmCTestMemCheckHandler();

protected:
  virtual int PreProcessHandler();
  virtual int PostProcessHandler();
  virtual void GenerateTestCommand(std::vector<const char*>& args);

private:

  enum { // Memory checkers
    UNKNOWN = 0,
    VALGRIND,
    PURIFY,
    BOUNDS_CHECKER
  };

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

  std::string              m_MemoryTester;
  std::vector<cmStdString> m_MemoryTesterOptionsParsed;
  std::string              m_MemoryTesterOptions;
  int                      m_MemoryTesterStyle;
  std::string              m_MemoryTesterOutputFile;
  int                      m_MemoryTesterGlobalResults[NO_MEMORY_FAULT];

  ///! Initialize memory checking subsystem.
  bool InitializeMemoryChecking();

  /**
   * Generate the Dart compatible output
   */
  void GenerateDartOutput(std::ostream& os);

  std::vector<cmStdString> m_CustomPreMemCheck;
  std::vector<cmStdString> m_CustomPostMemCheck;

  //! Parse Valgrind/Purify/Bounds Checker result out of the output
  //string. After running, log holds the output and results hold the
  //different memmory errors.
  bool ProcessMemCheckOutput(const std::string& str, 
                             std::string& log, int* results);
  bool ProcessMemCheckValgrindOutput(const std::string& str, 
                                     std::string& log, int* results);
  bool ProcessMemCheckPurifyOutput(const std::string& str, 
                                   std::string& log, int* results);

};

#endif

