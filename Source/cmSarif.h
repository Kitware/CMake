#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <cm/optional>

#include <cm3p/json/value.h>

/// @brief Objects for building serializable SARIF logs
namespace cmSarif {

/// @brief The severity level of a result in SARIF
///
/// The SARIF specification section 3.27.10 defines four levels of severity
/// for results. It is a string property of a result rather than its own type.
enum class ResultSeverityLevel
{
  Warning,
  Error,
  Note,
  None,
};

Json::Value GetJson(ResultSeverityLevel level);

/// @brief SARIF artifactLocation object (§3.4)
struct ArtifactLocation
{
  std::string Uri;
  std::string UriBaseId;
};

Json::Value GetJson(ArtifactLocation const& artifactLocation);

/// @brief SARIF region object (§3.30)
struct Region
{
  long StartLine;
};

Json::Value GetJson(Region region);

/// @brief SARIF physicalLocation object (§3.29)
struct PhysicalLocation
{
  ArtifactLocation Artifact;
  cm::optional<Region> ArtifactRegion;
};

Json::Value GetJson(PhysicalLocation const& physicalLocation);

/// @brief SARIF location object (§3.28)
struct Location
{
  PhysicalLocation Physical;
};

Json::Value GetJson(Location const& location);

/// @brief A result reported by a run of a static analysis tool
///
/// This is the data model for results in a SARIF log. Typically, a result only
/// requires either a message or a rule index.
struct Result
{
  /// @brief The message text of the result (required if no rule index)
  cm::optional<std::string> Message;

  /// @brief The location of the result (optional)
  cm::optional<cmSarif::Location> Location;

  /// @brief The severity level of the result (optional)
  cm::optional<cmSarif::ResultSeverityLevel> Level;

  /// @brief The rule ID of the result (optional)
  cm::optional<std::string> RuleId;

  /// @brief The index of the rule in the log's rule array (optional)
  cm::optional<std::size_t> RuleIndex;
};

Json::Value GetJson(Result const& result);

/// @brief A reporting descriptor provides information about an analysis result
///
/// Reporting descriptors (SARIF specification section 3.49) provide
/// information about categories of reporting items and is used to define
/// rules and taxa.
struct ReportingDescriptor
{
  std::string Id;
  cm::optional<std::string> Name;
};

Json::Value GetJson(ReportingDescriptor const& reportingDescriptor);

struct ToolComponent
{
  std::string Name;
  std::string Version;
  std::vector<ReportingDescriptor> Rules;
};

Json::Value GetJson(ToolComponent const& toolComponent);

struct Tool
{
  ToolComponent Driver;
};

Json::Value GetJson(Tool const& tool);

struct Run
{
  cmSarif::Tool Tool;
  std::vector<Result> Results;
};

Json::Value GetJson(Run const& run);

bool WriteLog(std::string const& path, cmSarif::Run const& run);

} // namespace cmSarif
