/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmTestPropertyHelper.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSetPropertyCommand.h"
#include "cmTest.h"
#include "cmValue.h"

cmGetTestPropertyResult cmGetTestProperty(std::string const& testName,
                                          std::string const& propertyName,
                                          cmExecutionStatus& status,
                                          bool testDirectoryOptionEnabled,
                                          std::string& testDirectory,
                                          cmValue& propertyValue)
{
  cmMakefile* testDirectoryMakefile = &status.GetMakefile();
  if (!SetPropertyCommand::HandleAndValidateTestDirectoryScopes(
        status, testDirectoryOptionEnabled, testDirectory,
        testDirectoryMakefile)) {
    return cmGetTestPropertyResult::ScopeError;
  }

  cmTest* test = testDirectoryMakefile->GetTest(testName);
  if (!test) {
    return cmGetTestPropertyResult::TestNotFound;
  }

  propertyValue = test->GetProperty(propertyName);
  return cmGetTestPropertyResult::Success;
}
