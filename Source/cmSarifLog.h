/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
    file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/filesystem>
#include <cm/optional>

#include <cm3p/json/value.h>

class cmake;
class cmListFileBacktrace;
enum class MessageType;

/// @brief CMake support for SARIF logging
namespace cmSarif {

constexpr char const* PROJECT_SARIF_FILE_VARIABLE = "CMAKE_EXPORT_SARIF";

constexpr char const* PROJECT_DEFAULT_SARIF_FILE = ".cmake/sarif/cmake.sarif";

/// @brief The severity level of a result in SARIF
///
/// The SARIF specification section 3.27.10 defines four levels of severity
/// for results.
enum class ResultSeverityLevel
{
  SARIF_WARNING,
  SARIF_ERROR,
  SARIF_NOTE,
  SARIF_NONE,
};

/// @brief A location in a source file logged with a SARIF result
struct SourceFileLocation
{
  std::string Uri;
  long Line = 0;

  /// @brief Construct a SourceFileLocation at the top of the call stack
  SourceFileLocation(cmListFileBacktrace const& backtrace);

  /// @brief Get the SourceFileLocation from the top of a call stack, if any
  /// @return The location or nullopt if the call stack is empty or is missing
  /// location information
  static cm::optional<SourceFileLocation> FromBacktrace(
    cmListFileBacktrace const& backtrace);
};

/// @brief A result defined by SARIF reported by a CMake run
///
/// This is the data model for results in a SARIF log. Typically, a result only
/// requires either a message or a rule index. The most common properties are
/// named in this struct, but arbitrary metadata can be added to the result
/// using the additionalProperties field.
struct Result
{
  /// @brief The message text of the result (required if no rule index)
  cm::optional<std::string> Message;

  /// @brief The location of the result (optional)
  cm::optional<cmSarif::SourceFileLocation> Location;

  /// @brief The severity level of the result (optional)
  cm::optional<cmSarif::ResultSeverityLevel> Level;

  /// @brief The rule ID of the result (optional)
  cm::optional<std::string> RuleId;

  /// @brief The index of the rule in the log's rule array (optional)
  cm::optional<std::size_t> RuleIndex;

  /// @brief Additional JSON properties for the result (optional)
  ///
  /// The additional properties should be merged into the result object when it
  /// is written to the SARIF log.
  Json::Value AdditionalProperties;
};

/// @brief A SARIF reporting rule
///
/// A rule in SARIF is described by a reportingDescriptor object (SARIF
/// specification section 3.49). The only property required for a rule is the
/// ID property. The ID is normally an opaque string that identifies a rule
/// applicable to a class of results. The other included properties are
/// optional but recommended for rules reported by CMake.
struct Rule
{
  /// @brief The ID of the rule. Required by SARIF
  std::string Id;

  /// @brief The end-user name of the rule (optional)
  cm::optional<std::string> Name;

  /// @brief The extended description of the rule (optional)
  cm::optional<std::string> FullDescription;

  /// @brief The default message for the rule (optional)
  cm::optional<std::string> DefaultMessage;

  /// @brief Get the JSON representation of this rule
  Json::Value GetJson() const;
};

/// @brief A builder for SARIF rules
///
/// `Rule` is a data model for SARIF rules. Known rules are usually initialized
/// manually by field. Using a builder makes initialization more readable and
/// prevents issues with reordering and optional fields.
class RuleBuilder
{
public:
  /// @brief Construct a new rule builder for a rule with the given ID
  RuleBuilder(char const* id) { this->NewRule.Id = id; }

  /// @brief Set the name of the rule
  RuleBuilder& Name(std::string name)
  {
    this->NewRule.Name = std::move(name);
    return *this;
  }

  /// @brief Set the full description of the rule
  RuleBuilder& FullDescription(std::string fullDescription)
  {
    this->NewRule.FullDescription = std::move(fullDescription);
    return *this;
  }

  /// @brief Set the default message for the rule
  RuleBuilder& DefaultMessage(std::string defaultMessage)
  {
    this->NewRule.DefaultMessage = std::move(defaultMessage);
    return *this;
  }

  /// @brief Build the rule
  std::pair<std::string, Rule> Build() const
  {
    return std::make_pair(this->NewRule.Id, this->NewRule);
  }

private:
  Rule NewRule;
};

/// @brief Get the SARIF severity level of a CMake message type
ResultSeverityLevel MessageSeverityLevel(MessageType t);

/// @brief Get the SARIF rule ID of a CMake message type
/// @return The rule ID or nullopt if the message type is unrecognized
///
/// The rule ID is a string assigned to SARIF results to identify the category
/// of the result. CMake maps messages to rules based on the message type.
/// CMake's rules are of the form "CMake.<MessageType>".
cm::optional<std::string> MessageRuleId(MessageType t);

/// @brief A log for reporting results in the SARIF format
class ResultsLog
{
public:
  ResultsLog();

  /// @brief Log a result of this run to the SARIF output
  void Log(cmSarif::Result&& result) const;

  /// @brief Log a result from a CMake message with a source file location
  /// @param t The type of the message, which corresponds to the level and rule
  /// of the result
  /// @param text The contents of the message
  /// @param backtrace The call stack where the message originated (may be
  /// empty)
  void LogMessage(MessageType t, std::string const& text,
                  cmListFileBacktrace const& backtrace) const;

  /// @brief Write this SARIF log to an empty JSON object
  /// @param[out] root The JSON object to write to
  void WriteJson(Json::Value& root) const;

private:
  // Private methods

  // Log that a rule was used and should be included in the output. Returns the
  // index of the rule in the log
  std::size_t UseRule(std::string const& id) const;

  // Private data
  // All data is mutable since log results are often added in const methods

  // All results added chronologically
  mutable std::vector<cmSarif::Result> Results;

  // Mapping of rule IDs to rule indices in the log.
  // In SARIF, rule metadata is typically only included if the rule is
  // referenced. The indices are unique to one log output and and vary
  // depending on when the rule was first encountered.
  mutable std::unordered_map<std::string, std::size_t> RuleToIndex;

  // Rules that will be added to the log in order of appearance
  mutable std::vector<std::string> EnabledRules;

  // All known rules that could be included in a log
  mutable std::unordered_map<std::string, Rule> KnownRules;
};

/// @brief Writes contents of a `cmSarif::ResultsLog` to a file
///
/// The log file writer is a helper class that writes the contents of a
/// `cmSarif::ResultsLog` upon destruction if a condition (e.g. project
/// variable is enabled) is met.
class LogFileWriter
{
public:
  /// @brief Create a new, disabled log file writer
  ///
  /// The returned writer will not write anything until the path generator
  /// and write condition are set. If the log has not been written when the
  /// object is being destroyed, the destructor will write the log if the
  /// condition is met and a valid path is available.
  LogFileWriter(ResultsLog const& log)
    : Log(log)
  {
  }

  /// @brief Configure a log file writer for a CMake run
  ///
  /// CMake should write a SARIF log if the project variable
  /// `CMAKE_EXPORT_SARIF` is `ON` or if the `--sarif-output=<path>` command
  /// line option is set. The writer will be configured to respond to these
  /// conditions.
  ///
  /// This does not configure a default path, so one must be set once it is
  /// known that we're in normal mode if none was explicitly provided.
  bool ConfigureForCMakeRun(cmake& cm);

  ~LogFileWriter();

  /// @brief Check if a valid path is set by opening the output file
  /// @return True if the file can be opened for writing
  bool EnsureFileValid();

  /// @brief The possible outcomes of trying to write the log file
  enum class WriteResult
  {
    SUCCESS, ///< File written with no issues
    FAILURE, ///< Error encountered while writing the file
    SKIPPED, ///< Writing was skipped due to false write condition
  };

  /// @brief Try to write the log file and return `true` if it was written
  ///
  /// Check the write condition and path generator to determine if the log
  /// file should be written.
  WriteResult TryWrite();

  /// @brief Set a lambda to check if the log file should be written
  void SetWriteCondition(std::function<bool()> const& checkConditionCallback)
  {
    this->WriteCondition = checkConditionCallback;
  }

  /// @brief Set the output file path, optionally creating parent directories
  ///
  /// The settings will apply when the log file is written. If the output
  /// file should be checked earlier, use `CheckFileValidity`.
  void SetPath(cm::filesystem::path const& path,
               bool createParentDirectories = false)
  {
    this->FilePath = path;
    this->CreateDirectories = createParentDirectories;
  }

private:
  ResultsLog const& Log;
  std::function<bool()> WriteCondition;
  cm::filesystem::path FilePath;
  bool CreateDirectories = false;
  bool FileWritten = false;
};

} // namespace cmSarif
