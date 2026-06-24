#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

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

/// @brief SARIF message object (§3.11)
struct Message
{
  std::string Text;
};

Json::Value GetJson(Message const& message);

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

/// @brief Suggested values for the logical location `kind` property (§3.33.7)
///
/// The SARIF-recommended terminology for identifying the type of construct at
/// the associated location. This namespace is for defining the SARIF-specified
/// vocabulary only (although the actual `kind` property can be any string).
namespace LocationKind {
cm::string_view const Function = "function"_s;
}

/// @brief SARIF logicalLocation object (§3.33)
struct LogicalLocation
{
  std::string Name;

  /// @brief The type of construct identified by the logical location
  ///
  /// The value should be from the suggestions in §3.33.7 but can be any string
  /// if none of the specified suggestions apply. See `cmSarif::LocationKind`
  /// for the suggested values.
  cm::string_view Kind;
};

Json::Value GetJson(LogicalLocation const& logicalLocation);

/// @brief SARIF location object (§3.28)
struct Location
{
  PhysicalLocation Physical;
  std::vector<LogicalLocation> Logical;
  cm::optional<cmSarif::Message> Message;
};

Json::Value GetJson(Location const& location);

struct StackFrame
{
  cm::optional<cmSarif::Location> Location;
  std::vector<std::string> Parameters;
};

Json::Value GetJson(StackFrame const& stackFrame);

struct Stack
{
  std::vector<StackFrame> Frames;
};

Json::Value GetJson(Stack const& stack);

/// @brief A result reported by a run of a static analysis tool
///
/// This is the data model for results in a SARIF log. Typically, a result only
/// requires either a message or a rule index.
struct Result
{
  /// @brief The message text of the result (required if no rule index)
  cm::optional<cmSarif::Message> Message;

  /// @brief The location of the result (optional)
  cm::optional<cmSarif::Location> Location;

  /// @brief Call stacks related to the result (optional)
  std::vector<cmSarif::Stack> Stacks;

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
/// information about categories of reporting items and are used to define
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

struct Invocation
{
  std::vector<std::string> Arguments;
  ArtifactLocation ExecutableLocation;
  std::string StartTimeUtc;
  std::string EndTimeUtc;
  cm::optional<int> ExitCode;
  cm::optional<int> ExitSignalNumber;
  bool ExecutionSuccessful;
};

Json::Value GetJson(Invocation const& invocation);

struct Run
{
  cmSarif::Tool Tool;
  std::vector<Invocation> Invocations;
  std::vector<Result> Results;
  std::unordered_map<std::string, ArtifactLocation> OriginalUriBaseIds;
};

Json::Value GetJson(Run const& run);

bool WriteLog(std::string const& path, cmSarif::Run const& run);

} // namespace cmSarif
