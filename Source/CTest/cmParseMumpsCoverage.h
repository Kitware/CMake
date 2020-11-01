/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>

class cmCTest;
class cmCTestCoverageHandlerContainer;

/** \class cmParseMumpsCoverage
 * \brief Parse Mumps coverage information
 *
 * This class is used as the base class for Mumps coverage
 * parsing.
 */
class cmParseMumpsCoverage
{
public:
  cmParseMumpsCoverage(cmCTestCoverageHandlerContainer& cont, cmCTest* ctest);
  virtual ~cmParseMumpsCoverage();
  // This is the toplevel coverage file locating the coverage files
  // and the mumps source code package tree.
  bool ReadCoverageFile(const char* file);

protected:
  // sub classes will use this to
  // load all coverage files found in the given directory
  virtual bool LoadCoverageData(std::string const& d) = 0;
  // search the package directory for mumps files and fill
  // in the RoutineToDirectory map
  bool LoadPackages(std::string const& dir);
  // initialize the coverage information for a single mumps file
  void InitializeMumpsFile(std::string& file);
  // Find mumps file for routine
  bool FindMumpsFile(std::string const& routine, std::string& filepath);

protected:
  std::map<std::string, std::string> RoutineToDirectory;
  cmCTestCoverageHandlerContainer& Coverage;
  cmCTest* CTest;
};
