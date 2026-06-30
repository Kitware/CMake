/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
    file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include <cm/optional>

class cmake;

/// @brief Manages SARIF logging for a CMake run
///
/// Writes diagnostics collected during a CMake run to a SARIF log file if
/// enabled by conditions.
class cmCMakeSarifLogger final
{
public:
  cmCMakeSarifLogger(cmake& cm);
  ~cmCMakeSarifLogger();

  void GenerateForRun() const;

private:
  bool WriteFile(std::string const& path,
                 bool createParentDirectories = false) const;
  cm::optional<std::string> FileOutputPath() const;

  cmake const& CM;
};
