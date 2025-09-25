/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSarifLog.h"

#include <memory>
#include <stdexcept>

#include <cm/filesystem>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/FStream.hxx"

#include "cmListFileCache.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmVersionConfig.h"
#include "cmake.h"

cmSarif::ResultsLog::ResultsLog()
{
  // Add the known CMake rules
  this->KnownRules.emplace(RuleBuilder("CMake.AuthorWarning")
                             .Name("CMake Warning (dev)")
                             .DefaultMessage("CMake Warning (dev): {0}")
                             .Build());
  this->KnownRules.emplace(RuleBuilder("CMake.Warning")
                             .Name("CMake Warning")
                             .DefaultMessage("CMake Warning: {0}")
                             .Build());
  this->KnownRules.emplace(RuleBuilder("CMake.DeprecationWarning")
                             .Name("CMake Deprecation Warning")
                             .DefaultMessage("CMake Deprecation Warning: {0}")
                             .Build());
  this->KnownRules.emplace(RuleBuilder("CMake.AuthorError")
                             .Name("CMake Error (dev)")
                             .DefaultMessage("CMake Error (dev): {0}")
                             .Build());
  this->KnownRules.emplace(RuleBuilder("CMake.FatalError")
                             .Name("CMake Error")
                             .DefaultMessage("CMake Error: {0}")
                             .Build());
  this->KnownRules.emplace(
    RuleBuilder("CMake.InternalError")
      .Name("CMake Internal Error")
      .DefaultMessage("CMake Internal Error (please report a bug): {0}")
      .Build());
  this->KnownRules.emplace(RuleBuilder("CMake.DeprecationError")
                             .Name("CMake Deprecation Error")
                             .DefaultMessage("CMake Deprecation Error: {0}")
                             .Build());
  this->KnownRules.emplace(RuleBuilder("CMake.Message")
                             .Name("CMake Message")
                             .DefaultMessage("CMake Message: {0}")
                             .Build());
  this->KnownRules.emplace(RuleBuilder("CMake.Log")
                             .Name("CMake Log")
                             .DefaultMessage("CMake Log: {0}")
                             .Build());
}

void cmSarif::ResultsLog::Log(cmSarif::Result&& result) const
{
  // The rule ID is optional, but if it is present, enable metadata output for
  // the rule by marking it as used
  if (result.RuleId) {
    std::size_t index = this->UseRule(*result.RuleId);
    result.RuleIndex = index;
  }

  // Add the result to the log
  this->Results.emplace_back(result);
}

void cmSarif::ResultsLog::LogMessage(
  MessageType t, std::string const& text,
  cmListFileBacktrace const& backtrace) const
{
  // Add metadata to the result object
  // The CMake SARIF rules for messages all expect 1 string argument with the
  // message text
  Json::Value additionalProperties(Json::objectValue);
  Json::Value args(Json::arrayValue);
  args.append(text);
  additionalProperties["message"]["id"] = "default";
  additionalProperties["message"]["arguments"] = args;

  // Create and log a result object
  // Rule indices are assigned when writing the final JSON output. Right now,
  // leave it as nullopt. The other optional fields are filled if available
  this->Log(cmSarif::Result{
    text, cmSarif::SourceFileLocation::FromBacktrace(backtrace),
    cmSarif::MessageSeverityLevel(t), cmSarif::MessageRuleId(t), cm::nullopt,
    additionalProperties });
}

std::size_t cmSarif::ResultsLog::UseRule(std::string const& id) const
{
  // Check if the rule is already in the index
  auto it = this->RuleToIndex.find(id);
  if (it != this->RuleToIndex.end()) {
    // The rule is already in use. Return the known index
    return it->second;
  }

  // This rule is not yet in the index, so check if it is recognized
  auto itKnown = this->KnownRules.find(id);
  if (itKnown == this->KnownRules.end()) {
    // The rule is not known. Add an empty rule to the known rules so that it
    // is included in the output
    this->KnownRules.emplace(RuleBuilder(id.c_str()).Build());
  }

  // Since this is the first time the rule is used, enable it and add it to the
  // index
  std::size_t idx = this->EnabledRules.size();
  this->RuleToIndex[id] = idx;
  this->EnabledRules.emplace_back(id);
  return idx;
}

cmSarif::ResultSeverityLevel cmSarif::MessageSeverityLevel(MessageType t)
{
  switch (t) {
    case MessageType::AUTHOR_WARNING:
    case MessageType::WARNING:
    case MessageType::DEPRECATION_WARNING:
      return ResultSeverityLevel::SARIF_WARNING;
    case MessageType::AUTHOR_ERROR:
    case MessageType::FATAL_ERROR:
    case MessageType::INTERNAL_ERROR:
    case MessageType::DEPRECATION_ERROR:
      return ResultSeverityLevel::SARIF_ERROR;
    case MessageType::MESSAGE:
    case MessageType::LOG:
      return ResultSeverityLevel::SARIF_NOTE;
    default:
      return ResultSeverityLevel::SARIF_NONE;
  }
}

cm::optional<std::string> cmSarif::MessageRuleId(MessageType t)
{
  switch (t) {
    case MessageType::AUTHOR_WARNING:
      return "CMake.AuthorWarning";
    case MessageType::WARNING:
      return "CMake.Warning";
    case MessageType::DEPRECATION_WARNING:
      return "CMake.DeprecationWarning";
    case MessageType::AUTHOR_ERROR:
      return "CMake.AuthorError";
    case MessageType::FATAL_ERROR:
      return "CMake.FatalError";
    case MessageType::INTERNAL_ERROR:
      return "CMake.InternalError";
    case MessageType::DEPRECATION_ERROR:
      return "CMake.DeprecationError";
    case MessageType::MESSAGE:
      return "CMake.Message";
    case MessageType::LOG:
      return "CMake.Log";
    default:
      return cm::nullopt;
  }
}

Json::Value cmSarif::Rule::GetJson() const
{
  Json::Value rule(Json::objectValue);
  rule["id"] = this->Id;

  if (this->Name) {
    rule["name"] = *this->Name;
  }
  if (this->FullDescription) {
    rule["fullDescription"]["text"] = *this->FullDescription;
  }
  if (this->DefaultMessage) {
    rule["messageStrings"]["default"]["text"] = *this->DefaultMessage;
  }

  return rule;
}

cmSarif::SourceFileLocation::SourceFileLocation(
  cmListFileBacktrace const& backtrace)
{
  if (backtrace.Empty()) {
    throw std::runtime_error("Empty source file location");
  }

  cmListFileContext const& lfc = backtrace.Top();
  this->Uri = lfc.FilePath;
  this->Line = lfc.Line;
}

cm::optional<cmSarif::SourceFileLocation>
cmSarif::SourceFileLocation::FromBacktrace(
  cmListFileBacktrace const& backtrace)
{
  if (backtrace.Empty()) {
    return cm::nullopt;
  }
  cmListFileContext const& lfc = backtrace.Top();
  if (lfc.Line <= 0 || lfc.FilePath.empty()) {
    return cm::nullopt;
  }

  return cm::make_optional<cmSarif::SourceFileLocation>(backtrace);
}

void cmSarif::ResultsLog::WriteJson(Json::Value& root) const
{
  // Add SARIF metadata
  root["version"] = "2.1.0";
  root["$schema"] = "https://schemastore.azurewebsites.net/schemas/json/"
                    "sarif-2.1.0-rtm.4.json";

  // JSON object for the SARIF runs array
  Json::Value runs(Json::arrayValue);

  // JSON object for the current (only) run
  Json::Value currentRun(Json::objectValue);

  // Accumulate info about the reported rules
  Json::Value jsonRules(Json::arrayValue);
  for (auto const& ruleId : this->EnabledRules) {
    jsonRules.append(KnownRules.at(ruleId).GetJson());
  }

  // Add info the driver for the current run (CMake)
  Json::Value driverTool(Json::objectValue);
  driverTool["name"] = "CMake";
  driverTool["version"] = CMake_VERSION;
  driverTool["rules"] = jsonRules;
  currentRun["tool"]["driver"] = driverTool;

  runs.append(currentRun);

  // Add all results
  Json::Value jsonResults(Json::arrayValue);
  for (auto const& res : this->Results) {
    Json::Value jsonResult(Json::objectValue);

    if (res.Message) {
      jsonResult["message"]["text"] = *(res.Message);
    }

    // If the result has a level, add it to the result
    if (res.Level) {
      switch (*res.Level) {
        case ResultSeverityLevel::SARIF_WARNING:
          jsonResult["level"] = "warning";
          break;
        case ResultSeverityLevel::SARIF_ERROR:
          jsonResult["level"] = "error";
          break;
        case ResultSeverityLevel::SARIF_NOTE:
          jsonResult["level"] = "note";
          break;
        case ResultSeverityLevel::SARIF_NONE:
          jsonResult["level"] = "none";
          break;
      }
    }

    // If the result has a rule ID or index, add it to the result
    if (res.RuleId) {
      jsonResult["ruleId"] = *res.RuleId;
    }
    if (res.RuleIndex) {
      jsonResult["ruleIndex"] = Json::UInt64(*res.RuleIndex);
    }

    if (res.Location) {
      jsonResult["locations"][0]["physicalLocation"]["artifactLocation"]
                ["uri"] = (res.Location)->Uri;
      jsonResult["locations"][0]["physicalLocation"]["region"]["startLine"] =
        Json::Int64((res.Location)->Line);
    }

    jsonResults.append(jsonResult);
  }

  currentRun["results"] = jsonResults;
  runs[0] = currentRun;
  root["runs"] = runs;
}

cmSarif::LogFileWriter::~LogFileWriter()
{
  // If the file has not been written yet, try to finalize it
  if (!this->FileWritten) {
    // Try to write and check the result
    if (this->TryWrite() == WriteResult::FAILURE) {
      // If the result is `FAILURE`, it means the write condition is true but
      // the file still wasn't written. This is an error.
      cmSystemTools::Error("Failed to write SARIF log to " +
                           this->FilePath.generic_string());
    }
  }
}

bool cmSarif::LogFileWriter::EnsureFileValid()
{
  // First, ensure directory exists
  cm::filesystem::path dir = this->FilePath.parent_path();
  if (!cmSystemTools::FileIsDirectory(dir.generic_string())) {
    if (!this->CreateDirectories ||
        !cmSystemTools::MakeDirectory(dir.generic_string()).IsSuccess()) {
      return false;
    }
  }

  // Open the file for writing
  cmsys::ofstream outputFile(this->FilePath.generic_string().c_str());
  if (!outputFile.good()) {
    return false;
  }
  return true;
}

cmSarif::LogFileWriter::WriteResult cmSarif::LogFileWriter::TryWrite()
{
  // Check that SARIF logging is enabled
  if (!this->WriteCondition || !this->WriteCondition()) {
    return WriteResult::SKIPPED;
  }

  // Open the file
  if (!this->EnsureFileValid()) {
    return WriteResult::FAILURE;
  }
  cmsys::ofstream outputFile(this->FilePath.generic_string().c_str());

  // The file is available, so proceed to write the log

  // Assemble the SARIF JSON from the results in the log
  Json::Value root(Json::objectValue);
  this->Log.WriteJson(root);

  // Serialize the JSON to the file
  Json::StreamWriterBuilder builder;
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

  writer->write(root, &outputFile);
  outputFile.close();

  this->FileWritten = true;
  return WriteResult::SUCCESS;
}

bool cmSarif::LogFileWriter::ConfigureForCMakeRun(cmake& cm)
{
  // If an explicit SARIF output path has been provided, set and check it
  cm::optional<std::string> sarifFilePath = cm.GetSarifFilePath();
  if (sarifFilePath) {
    this->SetPath(cm::filesystem::path(*sarifFilePath));
    if (!this->EnsureFileValid()) {
      cmSystemTools::Error(
        cmStrCat("Invalid SARIF output file path: ", *sarifFilePath));
      return false;
    }
  }

  // The write condition is checked immediately before writing the file, which
  // allows projects to enable SARIF diagnostics by setting a cache variable
  // and have it take effect for the current run.
  this->SetWriteCondition([&cm]() {
    // The command-line option can be used to set an explicit path, but in
    // normal mode, the project variable `CMAKE_EXPORT_SARIF` can also enable
    // SARIF logging.
    return cm.GetSarifFilePath().has_value() ||
      (cm.GetWorkingMode() == cmake::NORMAL_MODE &&
       cm.GetCacheDefinition(cmSarif::PROJECT_SARIF_FILE_VARIABLE).IsOn());
  });

  return true;
}
