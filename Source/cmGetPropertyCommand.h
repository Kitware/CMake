/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;
class cmValue;

namespace GetPropertyCommand {

// Per-entity property lookups used by the get_property command. Each preserves
// the script command's error semantics for its scope (status.SetError on
// missing entity / empty name where applicable). On success, returns true and
// fills `out`; `out` is null iff the property is unset. On hard error returns
// false with status.SetError already set.

bool LookupTargetProperty(cmExecutionStatus& status, std::string const& name,
                          std::string const& propertyName, cmValue& out);

bool LookupDirectoryProperty(cmExecutionStatus& status,
                             std::string const& name,
                             std::string const& propertyName, cmValue& out);

bool LookupSourceProperty(
  cmExecutionStatus& status, std::string const& name,
  std::string const& propertyName, bool sourceFileDirectoryOptionEnabled,
  bool sourceFileTargetOptionEnabled,
  std::vector<std::string>& sourceFileDirectories,
  std::vector<std::string>& sourceFileTargetDirectories, cmValue& out);

// Convenience overload: no DIRECTORY / TARGET_DIRECTORY scoping.
bool LookupSourceProperty(cmExecutionStatus& status, std::string const& name,
                          std::string const& propertyName, cmValue& out);

bool LookupTestProperty(cmExecutionStatus& status, std::string const& name,
                        std::string const& propertyName,
                        bool testDirectoryOptionEnabled,
                        std::string& testDirectory, cmValue& out);

// Convenience overload: no DIRECTORY scoping.
bool LookupTestProperty(cmExecutionStatus& status, std::string const& name,
                        std::string const& propertyName, cmValue& out);

bool LookupCacheProperty(cmExecutionStatus& status, std::string const& name,
                         std::string const& propertyName, cmValue& out);

}

bool cmGetPropertyCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status);
