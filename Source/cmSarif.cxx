/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSarif.h"

#include <memory>

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

Json::Value GetJson(Location const& location)
{
  Json::Value obj(Json::objectValue);
  obj["physicalLocation"] = cmSarif::GetJson(location.Physical);
  return obj;
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
    resultJson["message"]["text"] = *result.Message;
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

Json::Value GetJson(Run const& run)
{
  Json::Value runJson(Json::objectValue);
  runJson["tool"] = cmSarif::GetJson(run.Tool);
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
