/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/json/value.h>

#include "cmStateTypes.h"

class cmExecutionStatus;
class cmMakefile;
class cmTarget;

struct cmPackageRequirement
{
  std::string Name;
  std::string Version;
  std::vector<std::string> Components;
  std::vector<std::string> Hints;
};

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
  static std::unique_ptr<cmPackageInfoReader> Read(
    std::string const& path, cmPackageInfoReader const* parent = nullptr);

  std::string GetName() const;
  cm::optional<std::string> GetVersion() const;
  cm::optional<std::string> GetCompatVersion() const;

  // NOTE: The eventual intent is for CPS to support multiple version schemas,
  // and in particular, we expect to want to support "simple", "custom", "rpm",
  // "dpkg" and "pep440". Additionally, we desire to be able to parse each of
  // these to the maximum extent possible; in particular, we want to be able
  // to decompose "simple" and "pep440" versions into components represented
  // as numeric types rather than strings, which is not possible with the "rpm"
  // and "dpkg" schemas. Therefore, we require different data structures to
  // represent different version schemas.

  struct Pep440Version
  {
    // NOTE: This structure is currently incomplete as we only support the
    // "simple" schema at this time.
    bool Simple; // "simple" can be represented as a subset of "pep440"
    std::vector<unsigned> ReleaseComponents;
    cm::optional<std::string> LocalLabel;
  };

  // FIXME: Return a sum type (e.g. {cm,std}::variant) of possible versions
  // when we support more than just the "simple" (and possibly "pep440")
  // schema(s).
  /// If the package uses the 'simple' version scheme, parse the provided
  /// version string as a numeric tuple and optional trailing string.  Returns
  /// a disengaged optional for other schemes or if no version is specified.
  cm::optional<Pep440Version> ParseVersion(
    cm::optional<std::string> const& version) const;

  std::vector<cmPackageRequirement> GetRequirements() const;
  std::vector<std::string> GetComponentNames() const;

  /// Create targets for components specified in the CPS file.
  bool ImportTargets(cmMakefile* makefile, cmExecutionStatus& status);

  /// Add configuration-specific properties for targets.
  bool ImportTargetConfigurations(cmMakefile* makefile,
                                  cmExecutionStatus& status) const;

private:
  cmPackageInfoReader() = default;

  cmTarget* AddLibraryComponent(cmMakefile* makefile,
                                cmStateEnums::TargetType type,
                                std::string const& name,
                                Json::Value const& data,
                                std::string const& package) const;

  void SetTargetProperties(cmMakefile* makefile, cmTarget* target,
                           Json::Value const& data, std::string const& package,
                           cm::string_view configuration) const;
  void SetOptionalProperty(cmTarget* target, cm::string_view property,
                           cm::string_view configuration,
                           Json::Value const& value) const;

  std::string ResolvePath(std::string path) const;

  std::string Path;
  Json::Value Data;
  std::string Prefix;

  std::map<std::string, cmTarget*> ComponentTargets;
  std::vector<std::string> DefaultConfigurations;
};
