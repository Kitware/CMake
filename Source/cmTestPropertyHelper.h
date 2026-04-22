/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmValue.h"

class cmExecutionStatus;

enum class cmGetTestPropertyResult
{
  Success,
  ScopeError,   // test directory scope validation failed
  TestNotFound, // test does not exist
};

cmGetTestPropertyResult cmGetTestProperty(std::string const& testName,
                                          std::string const& propertyName,
                                          cmExecutionStatus& status,
                                          bool testDirectoryOptionEnabled,
                                          std::string& testDirectory,
                                          cmValue& propertyValue);
