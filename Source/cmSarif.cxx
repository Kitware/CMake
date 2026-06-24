/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSarif.h"

#include <memory>
#include <utility>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/FStream.hxx"

namespace cmSarif {

constexpr char const* SpecVersion = "2.1.0";
constexpr char const* SpecSchema =
  "https://docs.oasis-open.org/sarif/sarif/v2.1.0/errata01/os/schemas/"
  "sarif-schema-2.1.0.json";

Json::Value GetJson(ResultSeverityLevel level)
{
  switch (level) {
    case ResultSeverityLevel::Warning:
      return "warning";
    case ResultSeverityLevel::Error:
      return "error";
    case ResultSeverityLevel::Note:
      return "note";
    case ResultSeverityLevel::None:
    default:
      return "none";
  }
}

Json::Value GetJson(Message const& message)
{
  Json::Value obj(Json::objectValue);
  obj["text"] = message.Text;
  return obj;
}

Json::Value GetJson(ArtifactLocation const& artifactLocation)
{
  Json::Value obj(Json::objectValue);
  obj["uri"] = artifactLocation.Uri;
  if (!artifactLocation.UriBaseId.empty()) {
    obj["uriBaseId"] = artifactLocation.UriBaseId;
  }
  return obj;
}

Json::Value GetJson(Region region)
{
  Json::Value obj(Json::objectValue);
  obj["startLine"] = Json::Int64(region.StartLine);
  return obj;
}

Json::Value GetJson(PhysicalLocation const& physicalLocation)
{
  Json::Value obj(Json::objectValue);
  obj["artifactLocation"] = cmSarif::GetJson(physicalLocation.Artifact);
  if (physicalLocation.ArtifactRegion) {
    obj["region"] = cmSarif::GetJson(*physicalLocation.ArtifactRegion);
  }
  return obj;
}

Json::Value GetJson(LogicalLocation const& logicalLocation)
{
  Json::Value obj(Json::objectValue);
  obj["name"] = logicalLocation.Name;
  if (!logicalLocation.Kind.empty()) {
    obj["kind"] = std::string(logicalLocation.Kind);
  }
  return obj;
}

Json::Value GetJson(Location const& location)
{
  Json::Value obj(Json::objectValue);
  obj["physicalLocation"] = cmSarif::GetJson(location.Physical);
  if (!location.Logical.empty()) {
    Json::Value logical(Json::arrayValue);
    for (auto const& loc : location.Logical) {
      logical.append(cmSarif::GetJson(loc));
    }
    obj["logicalLocations"] = logical;
  }
  if (location.Message) {
    obj["message"] = cmSarif::GetJson(*location.Message);
  }

  return obj;
}

Json::Value GetJson(StackFrame const& frame)
{
  Json::Value frameJson(Json::objectValue);
  if (frame.Location) {
    frameJson["location"] = cmSarif::GetJson(*frame.Location);
  }
  return frameJson;
}

Json::Value GetJson(Stack const& stack)
{
  Json::Value stackJson(Json::objectValue);
  Json::Value frames(Json::arrayValue);
  for (auto const& frame : stack.Frames) {
    frames.append(cmSarif::GetJson(frame));
  }
  stackJson["frames"] = frames;
  return stackJson;
}

Json::Value GetJson(ReportingDescriptor const& reportingDescriptor)
{
  Json::Value rd(Json::objectValue);
  rd["id"] = reportingDescriptor.Id;
  if (reportingDescriptor.Name) {
    rd["name"] = *reportingDescriptor.Name;
  }
  return rd;
}

Json::Value GetJson(Result const& result)
{
  Json::Value resultJson(Json::objectValue);

  if (result.Message) {
    resultJson["message"] = cmSarif::GetJson(*result.Message);
  }

  if (result.Level) {
    resultJson["level"] = cmSarif::GetJson(*result.Level);
  }

  if (result.RuleId) {
    resultJson["ruleId"] = *result.RuleId;
  }
  if (result.RuleIndex) {
    resultJson["ruleIndex"] = Json::UInt64(*result.RuleIndex);
  }

  if (result.Location) {
    resultJson["locations"][0] = cmSarif::GetJson(*result.Location);
  }

  if (!result.Stacks.empty()) {
    Json::Value stacks(Json::arrayValue);
    for (auto const& stack : result.Stacks) {
      stacks.append(cmSarif::GetJson(stack));
    }
    resultJson["stacks"] = stacks;
  }

  return resultJson;
}

Json::Value GetJson(ToolComponent const& toolComponent)
{
  Json::Value component(Json::objectValue);
  component["name"] = toolComponent.Name;
  component["version"] = toolComponent.Version;
  Json::Value rules(Json::arrayValue);
  for (auto const& rule : toolComponent.Rules) {
    rules.append(cmSarif::GetJson(rule));
  }
  component["rules"] = rules;
  return component;
}

Json::Value GetJson(Tool const& tool)
{
  Json::Value toolJson(Json::objectValue);
  toolJson["driver"] = cmSarif::GetJson(tool.Driver);
  return toolJson;
}

Json::Value GetJson(Invocation const& invocation)
{
  Json::Value obj(Json::objectValue);
  Json::Value arguments(Json::arrayValue);
  for (auto const& argument : invocation.Arguments) {
    arguments.append(argument);
  }
  obj["arguments"] = arguments;
  obj["executableLocation"] = cmSarif::GetJson(invocation.ExecutableLocation);
  obj["startTimeUtc"] = invocation.StartTimeUtc;
  obj["endTimeUtc"] = invocation.EndTimeUtc;
  if (invocation.ExitCode) {
    obj["exitCode"] = *invocation.ExitCode;
  }
  if (invocation.ExitSignalNumber) {
    obj["exitSignalNumber"] = *invocation.ExitSignalNumber;
  }
  obj["executionSuccessful"] = invocation.ExecutionSuccessful;
  return obj;
}

Json::Value GetJson(Run const& run)
{
  Json::Value runJson(Json::objectValue);
  runJson["tool"] = cmSarif::GetJson(run.Tool);

  if (!run.OriginalUriBaseIds.empty()) {
    Json::Value uriBaseIds(Json::objectValue);
    for (auto const& base : run.OriginalUriBaseIds) {
      uriBaseIds[base.first] = cmSarif::GetJson(base.second);
    }
    runJson["originalUriBaseIds"] = uriBaseIds;
  }

  if (!run.Invocations.empty()) {
    Json::Value invocations(Json::arrayValue);
    for (auto const& invocation : run.Invocations) {
      invocations.append(cmSarif::GetJson(invocation));
    }
    runJson["invocations"] = invocations;
  }

  Json::Value results(Json::arrayValue);
  for (auto const& result : run.Results) {
    results.append(cmSarif::GetJson(result));
  }
  runJson["results"] = results;

  return runJson;
}

bool WriteLog(std::string const& path, cmSarif::Run const& run)
{
  cmsys::ofstream outputFile(path.c_str());
  if (!outputFile.good()) {
    return false;
  }

  Json::Value root(Json::objectValue);
  root["version"] = SpecVersion;
  root["$schema"] = SpecSchema;
  Json::Value runs(Json::arrayValue);
  runs.append(cmSarif::GetJson(run));
  root["runs"] = runs;

  Json::StreamWriterBuilder builder;
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(root, &outputFile);
  outputFile.close();

  return true;
}

} // namespace cmSarif
