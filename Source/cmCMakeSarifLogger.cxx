/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCMakeSarifLogger.h"

#include <cstddef>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmsys/String.h"

#include "cmDiagnostics.h"
#include "cmListFileCache.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmSarif.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmVersionConfig.h"

// CMake-specific SARIF helpers
namespace {

/// @brief Express the location of a `cmListFileContext` in SARIF
/// @param[in] uriBaseIds A list of logical base directory names and their path
///
/// Build a SARIF location object detailing the location data available from a
/// context. More specific information like the region (line number) and
/// function call name will be included if available.
///
/// SARIF requests that paths are given relative to a logical base for
/// relocatability. Context locations will be made relative to a logical base
/// iff they fall under one of the directories listed in the `uriBaseIds`
/// map. Bases are tried in order.
cmSarif::Location LocationFromContext(
  cmListFileContext const& lfc,
  std::vector<std::pair<std::string, std::string>> const& uriBaseIds = {})
{
  cmSarif::Location location;
  location.Physical.Artifact.Uri = lfc.FilePath;

  // SARIF requests that paths are given relative to a logical base for
  // relocatability. Check if these files are under any of the bases, if
  // provided.
  for (auto const& baseUri : uriBaseIds) {
    std::string relative = cmSystemTools::RelativeIfUnder(
      baseUri.second, location.Physical.Artifact.Uri);
    if (relative != location.Physical.Artifact.Uri) {
      location.Physical.Artifact.Uri = relative;
      location.Physical.Artifact.UriBaseId = baseUri.first;
    }
  }

  if (!lfc.Name.empty()) {
    location.Logical.emplace_back(
      cmSarif::LogicalLocation{ lfc.Name, cmSarif::LocationKind::Function });
  }

  // Add info about the region within the file depending on how specific the
  // context is. Watch for deferred call and variable watch placeholders or
  // a zero, which indicates the start of processing a list file.
  if (lfc.Line == cmListFileContext::DeferPlaceholderLine) {
    location.Message = cmSarif::Message{ "DEFERRED" };
  } else if (lfc.Line > 0 && lfc.Line != std::numeric_limits<long>::max()) {
    cmSarif::Region region;
    region.StartLine = lfc.Line;
    location.Physical.ArtifactRegion = region;
  }

  return location;
}

cm::optional<cmSarif::Location> LastLocation(
  cmListFileBacktrace backtrace,
  std::vector<std::pair<std::string, std::string>> const& uriBaseIds = {})
{
  if (backtrace.Empty()) {
    return {};
  }
  return LocationFromContext(backtrace.Top(), uriBaseIds);
}

cm::optional<cmSarif::Stack> StackFromBacktrace(
  cmListFileBacktrace bt,
  std::vector<std::pair<std::string, std::string>> const& uriBaseIds = {})
{
  if (bt.Empty()) {
    return {};
  }

  cmSarif::Stack stack;
  for (; !bt.Empty(); bt = bt.Pop()) {
    cmSarif::Location topLocation = LocationFromContext(bt.Top(), uriBaseIds);

    // If the location doesn't have a specific region, this entry is a
    // placeholder and should not appear in the call stack.
    if (!topLocation.Message && !topLocation.Physical.ArtifactRegion) {
      continue;
    }

    cmSarif::StackFrame frame;
    frame.Location = std::move(topLocation);
    stack.Frames.emplace_back(std::move(frame));
  }
  return stack;
}

cmSarif::Tool CreateCMakeTool()
{
  cmSarif::ToolComponent cmDriver;
  cmDriver.Name = "CMake";
  cmDriver.Version = CMake_VERSION;

  return cmSarif::Tool{ cmDriver };
}

std::string RuleIdForMessageType(MessageType type,
                                 cmDiagnosticCategory category)
{
  cm::string_view name = cmDiagnostics::GetCategoryString(category);
  if (!name.empty()) {
    // Strip the "CMD_" prefix from the category name and convert to PascalCase
    std::string sarifIdName;
    bool nextWord = true;
    for (char c : name.substr(4)) {
      if (c == '_') {
        nextWord = true;
        continue;
      }
      if (nextWord) {
        sarifIdName += c;
        nextWord = false;
      } else {
        sarifIdName += cmsysString_tolower(c);
      }
    }
    return cmStrCat("CMake.", sarifIdName);
  }

  // Fall back to message type if not a diagnostic
  switch (type) {
    case MessageType::FATAL_ERROR:
      return "CMake.FatalError";
    case MessageType::INTERNAL_ERROR:
      return "CMake.InternalError";
    case MessageType::WARNING:
      return "CMake.Warning";
    default:
      return "";
  }
}

cm::string_view NameForMessageType(MessageType type,
                                   cmDiagnosticCategory category)
{
  if (category != cmDiagnostics::CMD_NONE) {
    return cmDiagnostics::GetCategoryString(category);
  }
  switch (type) {
    case MessageType::FATAL_ERROR:
      return "CMake Error";
    case MessageType::INTERNAL_ERROR:
      return "CMake Internal Error";
    case MessageType::WARNING:
      return "CMake Warning";
    default:
      return "";
  }
}

cmSarif::ReportingDescriptor ReportingDescriptorForMessageType(
  MessageType type, cmDiagnosticCategory category)
{
  cmSarif::ReportingDescriptor rd;
  rd.Id = RuleIdForMessageType(type, category);
  rd.Name = NameForMessageType(type, category);
  return rd;
};

cmSarif::ResultSeverityLevel SarifLevelFromMessageType(MessageType type)
{
  switch (type) {
    case MessageType::FATAL_ERROR:
    case MessageType::INTERNAL_ERROR:
      return cmSarif::ResultSeverityLevel::Error;
    case MessageType::WARNING:
      return cmSarif::ResultSeverityLevel::Warning;
    default:
      return cmSarif::ResultSeverityLevel::Note;
  }
}

} // namespace

cmCMakeSarifLogger::~cmCMakeSarifLogger()
{
  this->GenerateForRun();
}

void cmCMakeSarifLogger::SetOutputPath(std::string const& path)
{
  this->FilePath = path;
}

void cmCMakeSarifLogger::AddBaseDirectory(cm::string_view name,
                                          cm::string_view path)
{
  this->UriBaseIds.emplace_back(std::string(name), std::string(path));
  this->CMakeRun.OriginalUriBaseIds.emplace(
    std::string(name),
    cmSarif::ArtifactLocation{ cmStrCat("file://", path, "/"), "" });
}

void cmCMakeSarifLogger::RecordDiagnostics(
  std::vector<cmMessenger::Message> const& messages)
{
  this->CMakeRun.Tool = CreateCMakeTool();

  // Helper to add rules to the run as encountered in results and get their
  // index for reporting
  auto use_rule = [&](MessageType type, cmDiagnosticCategory category) {
    auto category_name = RuleIdForMessageType(type, category);
    auto ruleIt = this->RuleIndices.find(category_name);
    if (ruleIt != this->RuleIndices.end()) {
      return std::make_pair(category_name, ruleIt->second);
    }

    this->CMakeRun.Tool.Driver.Rules.emplace_back(
      ReportingDescriptorForMessageType(type, category));
    this->RuleIndices.emplace(category_name,
                              this->CMakeRun.Tool.Driver.Rules.size() - 1);
    return std::make_pair(category_name,
                          this->CMakeRun.Tool.Driver.Rules.size() - 1);
  };

  for (auto const& message : messages) {
    // SARIF should only emit diagnostic messages, not general messages/logs
    switch (message.Type) {
      case MessageType::MESSAGE:
      case MessageType::LOG:
      case MessageType::UNDEFINED:
        continue;
      default:
        break;
    }

    std::pair<std::string, std::size_t> ruleInfo =
      use_rule(message.Type, message.Category);

    cmSarif::Result result;
    result.RuleId = ruleInfo.first;
    result.RuleIndex = ruleInfo.second;
    result.Message = cmSarif::Message{ message.Text };
    result.Location = LastLocation(message.Backtrace, this->UriBaseIds);
    if (cm::optional<cmSarif::Stack> stack =
          StackFromBacktrace(message.Backtrace, this->UriBaseIds)) {
      result.Stacks.emplace_back(std::move(*stack));
    }
    result.Level = SarifLevelFromMessageType(message.Type);

    this->CMakeRun.Results.emplace_back(std::move(result));
  }
}

void cmCMakeSarifLogger::RecordInvocation(
  int ac, char const* const* av, int exitCode,
  std::chrono::system_clock::time_point startTime,
  std::chrono::system_clock::time_point endTime)
{
  cmTimestamp timestamp;

  cmSarif::Invocation invocation;
  invocation.Arguments.assign(av, av + ac);
  invocation.ExecutableLocation.Uri = cmSystemTools::GetCMakeCommand();
  invocation.StartTimeUtc = timestamp.CreateTimestampFromTimeT(
    std::chrono::system_clock::to_time_t(startTime), "", true);
  invocation.EndTimeUtc = timestamp.CreateTimestampFromTimeT(
    std::chrono::system_clock::to_time_t(endTime), "", true);
  invocation.ExitCode = exitCode;
  invocation.ExecutionSuccessful = (exitCode == 0);

  this->CMakeRun.Invocations.emplace_back(std::move(invocation));
}

bool cmCMakeSarifLogger::WriteFile(std::string const& path) const
{
  std::string const dir = cmSystemTools::GetFilenamePath(path);
  if (!cmSystemTools::FileIsDirectory(dir)) {
    return false;
  }

  return cmSarif::WriteLog(path, this->CMakeRun);
}

void cmCMakeSarifLogger::GenerateForRun() const
{
  if (this->FilePath.empty()) {
    return;
  }

  if (!this->WriteFile(this->FilePath)) {
    cmSystemTools::Error(
      cmStrCat("Failed to write SARIF log to ", this->FilePath));
  }
}
