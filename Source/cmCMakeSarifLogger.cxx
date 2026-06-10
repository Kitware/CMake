/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCMakeSarifLogger.h"

#include <cstddef>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmsys/FStream.hxx"

#include "cmListFileCache.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmSarif.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmVersionConfig.h"
#include "cmake.h"

// CMake-specific SARIF helpers
namespace {

constexpr char const* CMakeSarifOutputFlag = "CMAKE_EXPORT_SARIF";
constexpr char const* DefaultSarifFile = ".cmake/sarif/cmake.sarif";

cm::optional<cmSarif::Location> GetLocationFromBacktrace(
  cmListFileBacktrace const& backtrace, cmake const& cm)
{
  if (backtrace.Empty()) {
    return {};
  }
  cmListFileContext const& lfc = backtrace.Top();
  // Exclude frames with no real location: negative lines are deferred-call
  // placeholders, and LONG_MAX is the synthetic line used by variable_watch
  // callback dispatch. Neither is a meaningful source location.
  if (lfc.Line < 0 || lfc.Line == std::numeric_limits<long>::max()) {
    return {};
  }

  cmSarif::PhysicalLocation location;
  location.Artifact.Uri = lfc.FilePath;

  // SARIF requests that paths are given relative to a logical base. Report
  // paths relative to the source dir / script working directory if possible.
  location.Artifact.UriBaseId = cm.GetHomeDirectory();
  std::string relative = cmSystemTools::RelativePath(
    location.Artifact.UriBaseId, location.Artifact.Uri);
  if (!relative.empty()) {
    location.Artifact.Uri = relative;
  }

  if (lfc.Line != 0) {
    cmSarif::Region region;
    region.StartLine = lfc.Line;
    location.ArtifactRegion = region;
  }
  return cmSarif::Location{ location };
}

cmSarif::Tool CreateCMakeTool()
{
  cmSarif::ToolComponent cmDriver;
  cmDriver.Name = "CMake";
  cmDriver.Version = CMake_VERSION;

  return cmSarif::Tool{ cmDriver };
}

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

cm::string_view MessageRuleId(MessageType type)
{
  switch (type) {
    case MessageType::FATAL_ERROR:
      return "CMake.FatalError";
    case MessageType::INTERNAL_ERROR:
      return "CMake.InternalError";
    case MessageType::WARNING:
      return "CMake.Warning";
    case MessageType::MESSAGE:
      return "CMake.Message";
    case MessageType::LOG:
    default:
      return "CMake.Log";
  }
}

cm::string_view MessageDisplayName(MessageType type)
{
  switch (type) {
    case MessageType::FATAL_ERROR:
      return "CMake Error";
    case MessageType::INTERNAL_ERROR:
      return "CMake Internal Error";
    case MessageType::WARNING:
      return "CMake Warning";
    case MessageType::MESSAGE:
      return "CMake Message";
    case MessageType::LOG:
    default:
      return "CMake Log";
  }
}

cmSarif::ReportingDescriptor RuleForMessageType(MessageType type)
{
  cmSarif::ReportingDescriptor rd;
  rd.Id = MessageRuleId(type);
  rd.Name = MessageDisplayName(type);
  return rd;
}

} // namespace

cmCMakeSarifLogger::cmCMakeSarifLogger(cmake& cm)
  : CM(cm)
{
  if (this->CM.GetState()->GetRole() == cmState::Role::Project) {
    cm.MarkCliAsUsed(CMakeSarifOutputFlag);
  }
}

cmCMakeSarifLogger::~cmCMakeSarifLogger()
{
  this->GenerateForRun();
}

cm::optional<std::string> cmCMakeSarifLogger::FileOutputPath() const
{
  // If a SARIF path was specified via CLI, use it. Otherwise, check whether
  // logging is enabled via the project cache variable and use the default
  // path if so.
  if (cm::optional<std::string> specifiedPath = this->CM.GetSarifFilePath()) {
    return specifiedPath;
  }
  if (this->CM.GetState()->GetRole() == cmState::Role::Project &&
      this->CM.GetCacheDefinition(CMakeSarifOutputFlag).IsOn()) {
    return cmStrCat(this->CM.GetHomeOutputDirectory(), '/', DefaultSarifFile);
  }
  return cm::nullopt;
}

bool cmCMakeSarifLogger::WriteFile(std::string const& path,
                                   bool createParentDirectories) const
{
  if (createParentDirectories) {
    if (!cmSystemTools::MakeDirectory(cmSystemTools::GetFilenamePath(path))
           .IsSuccess()) {
      return false;
    }
  }

  cmsys::ofstream outputFile(path);
  if (!outputFile.good()) {
    return false;
  }

  // Run object to build
  cmSarif::Run run;
  run.Tool = CreateCMakeTool();

  // Helper to add rules to the run as encountered in results and get their
  // index for reporting
  std::unordered_map<cm::string_view, std::size_t> ruleIndices;
  auto use_rule = [&](MessageType t) {
    cm::string_view category_name = MessageRuleId(t);
    auto result = ruleIndices.emplace(category_name, 0);
    if (result.second) {
      result.first->second = run.Tool.Driver.Rules.size();
      run.Tool.Driver.Rules.emplace_back(RuleForMessageType(t));
    }
    return *result.first;
  };

  cmMessenger const& messenger = *this->CM.GetMessenger();
  for (auto const& message : messenger.GetDisplayedMessages()) {
    std::pair<cm::string_view, std::size_t> ruleInfo = use_rule(message.Type);

    cmSarif::Result result;
    result.RuleId = ruleInfo.first;
    result.RuleIndex = ruleInfo.second;
    result.Message = message.Text;
    result.Location = GetLocationFromBacktrace(message.Backtrace, this->CM);
    result.Level = SarifLevelFromMessageType(message.Type);

    run.Results.emplace_back(std::move(result));
  }

  return cmSarif::WriteLog(path, run);
}

void cmCMakeSarifLogger::GenerateForRun() const
{
  cm::optional<std::string> path = this->FileOutputPath();
  if (!path) {
    return;
  }

  // If using the default path within the build dir, ensure parents are created
  bool const createParents = !this->CM.GetSarifFilePath().has_value();
  if (!this->WriteFile(*path, createParents)) {
    cmSystemTools::Error(cmStrCat("Failed to write SARIF log to ", *path));
  }
}
