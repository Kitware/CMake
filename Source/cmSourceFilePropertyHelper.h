/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include "cmValue.h"

class cmExecutionStatus;

enum class cmGetSourceFilePropertyResult
{
  Success,
  ScopeError,     // directory scope validation failed
  SourceNotFound, // source file does not exist in this directory scope
  Error, // unexpected internal failure (e.g. source could not be created)
};

cmGetSourceFilePropertyResult cmGetSourceFileProperty(
  std::string const& sourceName, std::string const& propertyName,
  cmExecutionStatus& status, bool sourceFileDirectoryOptionEnabled,
  bool sourceFileTargetOptionEnabled,
  std::vector<std::string>& sourceFileDirectories,
  std::vector<std::string>& sourceFileTargetDirectories,
  cmValue& propertyValue, bool alwaysCreateSource = false);
