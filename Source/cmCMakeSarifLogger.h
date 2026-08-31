/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
    file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmMessenger.h"
#include "cmSarif.h"

/// @brief Manages SARIF logging for a CMake run
///
/// Writes diagnostics collected during a CMake run to a SARIF log file if
/// enabled by conditions.
class cmCMakeSarifLogger final
{
public:
  cmCMakeSarifLogger() = default;
  ~cmCMakeSarifLogger();

  /// @brief Enable SARIF file generation at the given path when the logger is
  /// destroyed.
  void SetOutputPath(std::string const& path);

  /// @brief Add a logical base directory used to emit relative paths.
  ///
  /// Artifact locations under a given path will be expressed according to
  /// the named base directory. If multiple base directories are added to one
  /// log, they are tested in order.
  void AddBaseDirectory(cm::string_view name, cm::string_view path);

  /// @brief Save CMake diagnostic messages to the SARIF log.
  void RecordDiagnostics(std::vector<cmMessenger::Message> const& messages);

  /// @brief Save information about the CMake process invocation.
  void RecordInvocation(int ac, char const* const* av, int exitCode,
                        std::chrono::system_clock::time_point startTime,
                        std::chrono::system_clock::time_point endTime);

  void GenerateForRun() const;

private:
  bool WriteFile(std::string const& path) const;

  std::string FilePath;
  cmSarif::Run CMakeRun;
  // Index of a diagnostic category appearing in the current run log.
  std::unordered_map<std::string, std::size_t> RuleIndices;
  // Maps logical name to actual path for base directories
  std::vector<std::pair<std::string, std::string>> UriBaseIds;
};
