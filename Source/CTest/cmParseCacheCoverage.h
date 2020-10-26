/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmParseMumpsCoverage.h"

class cmCTest;
class cmCTestCoverageHandlerContainer;

/** \class cmParseCacheCoverage
 * \brief Parse Cache coverage information
 *
 * This class is used to parse Cache coverage information for
 * mumps.
 */
class cmParseCacheCoverage : public cmParseMumpsCoverage
{
public:
  cmParseCacheCoverage(cmCTestCoverageHandlerContainer& cont, cmCTest* ctest);

protected:
  // implement virtual from parent
  bool LoadCoverageData(std::string const& dir) override;
  // remove files with no coverage
  void RemoveUnCoveredFiles();
  // Read a single mcov file
  bool ReadCMCovFile(const char* f);
};
