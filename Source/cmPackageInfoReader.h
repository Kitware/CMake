/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

#include <cm3p/json/value.h>

// class cmExecutionStatus;

/** \class cmPackageInfoReader
 * \brief Read and parse CPS files.
 *
 * This class encapsulates the functionality to read package configuration
 * files which use the Common Package Specification, and provides utilities to
 * translate the declarations therein into imported targets.
 */
class cmPackageInfoReader
{
public:
  static std::unique_ptr<cmPackageInfoReader> Read(std::string const& path);

  std::string GetName() const;
  cm::optional<std::string> GetVersion() const;

  /// If the package uses the 'simple' version scheme, obtain the version as
  /// a numeric tuple.  Returns an empty vector for other schemes or if no
  /// version is specified.
  std::vector<unsigned> ParseVersion() const;

private:
  cmPackageInfoReader() = default;

  std::string Path;
  Json::Value Data;
};
