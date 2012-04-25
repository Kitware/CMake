/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParseGTMCoverage_h
#define cmParseGTMCoverage_h

#include "cmStandardIncludes.h"
#include "cmCTestCoverageHandler.h"

/** \class cmParseGTMCoverage
 * \brief Parse GTM coverage information
 *
 * This class is used to parse GTM coverage information for
 * mumps.
 */
class cmParseGTMCoverage
{
public:
  cmParseGTMCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest);
  bool ReadGTMCoverage(const char* file);
private:
  bool ParseFile(std::string& filepath,
                 std::string& function,
                 int& lineoffset);
  bool ParseLine(std::string const& line,
                 std::string& routine,
                 std::string& function,
                 int& linenumber,
                 int& count);
  bool LoadPackages(const char* dir);
  bool LoadCoverageData(const char* dir);
  bool ReadMCovFile(const char* f);
  void InitializeFile(std::string& file);
  std::map<cmStdString, cmStdString> RoutineToDirectory;
  cmCTestCoverageHandlerContainer& Coverage;
  cmCTest* CTest;
};


#endif
