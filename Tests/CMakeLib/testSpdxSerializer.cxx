
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmSbomObject.h"
#include "cmSpdx.h"
#include "cmSpdxSerializer.h"

#include "testCommon.h"

namespace {

bool testSerializeSpdxJson()
{

  auto constexpr contextUrl = "https://spdx.org/rdf/3.0.1/spdx-context.jsonld";

  cmSbomDocument doc;
  doc.Context = contextUrl;

  cmSpdxDocument spdxValue;
  spdxValue.SpdxId = "_:SPDXRef-Document";
  auto spdx = insert_back(doc.Graph, std::move(spdxValue));

  {
    cmSpdxCreationInfo ci;
    ci.SpdxId = "_:SPDXRef-CreationInfo";
    ci.SpecVersion = "3.0.1";
    spdx->CreationInfo = std::move(ci);
  }

  {
    cmSpdxPackage pkg;
    pkg.Name = "sample-package";
    pkg.SpdxId = "_:SPDXRef-Package";

    spdx->RootElements.emplace_back(std::move(pkg));
  }

  {
    cmSpdxSerializer serializer;
    doc.Serialize(serializer);
    auto value = serializer.GetJson();

    Json::Value expected;
    expected["@context"] = contextUrl;
    auto& document = expected["@graph"][0];
    document["type"] = "SpdxDocument";
    document["spdxId"] = "_:SPDXRef-Document";
    auto& creation = document["creationInfo"];
    creation["@id"] = "_:SPDXRef-CreationInfo";
    creation["type"] = "CreationInfo";
    creation["specVersion"] = "3.0.1";
    auto& package = document["rootElement"][0];
    package["type"] = "software_Package";
    package["spdxId"] = "_:SPDXRef-Package";
    package["name"] = "sample-package";
    ASSERT_EQUAL(value, expected);
  }

  return true;
}

bool testSerializeAfterReallocation()
{
  cmSbomDocument doc;
  cmSpdxCreationInfo creationInfo;
  creationInfo.SpdxId = "_:creation-info";
  doc.Graph.emplace_back(creationInfo);

  cmSpdxDocument document;
  document.SpdxId = "urn:document";
  document.CreationInfo = &creationInfo;
  auto* spdx = insert_back(doc.Graph, std::move(document));

  auto const count = 128u;
  for (auto i = 0u; i < count; ++i) {
    std::string const suffix = std::to_string(i);
    cmSpdxPackage rootValue;
    rootValue.SpdxId = "urn:root-" + suffix;
    rootValue.CreationInfo = &creationInfo;
    auto const* root = insert_back(spdx->RootElements, std::move(rootValue));

    cmSpdxPackage dependencyValue;
    dependencyValue.SpdxId = "urn:dependency-" + suffix;
    dependencyValue.CreationInfo = &creationInfo;
    auto const* dependency =
      insert_back(spdx->Elements, std::move(dependencyValue));

    cmSpdxRelationship relationship;
    relationship.SpdxId = "urn:relationship-" + suffix;
    relationship.CreationInfo = &creationInfo;
    relationship.RelationshipType = cmSpdxRelationship::DEPENDS_ON;
    relationship.From = root;
    relationship.To.emplace_back(dependency);
    doc.Graph.emplace_back(std::move(relationship));
  }

  cmSbomObject moved(std::move(doc));
  cmSbomObject copied(moved);
  cmSpdxSerializer serializer;
  std::ostringstream output;
  ASSERT_TRUE(serializer.WriteSbom(output, copied));
  auto const value = serializer.GetJson();

  Json::Value expected;
  expected["@context"] = "https://spdx.org/rdf/3.0.1/spdx-context.jsonld";
  auto& graph = expected["@graph"];
  graph[0]["@id"] = "_:creation-info";
  graph[0]["type"] = "CreationInfo";
  graph[1]["spdxId"] = "urn:document";
  graph[1]["type"] = "SpdxDocument";
  graph[1]["creationInfo"] = "_:creation-info";
  for (auto i = 0u; i < count; ++i) {
    std::string const rootId = "urn:root-" + std::to_string(i);
    std::string const dependencyId = "urn:dependency-" + std::to_string(i);
    auto& root = graph[1]["rootElement"][i];
    root["spdxId"] = rootId;
    root["creationInfo"] = "_:creation-info";
    root["type"] = "software_Package";
    auto& dependency = graph[1]["element"][i];
    dependency["spdxId"] = dependencyId;
    dependency["creationInfo"] = "_:creation-info";
    dependency["type"] = "software_Package";
    auto& relationship = graph[i + 2];
    relationship["spdxId"] = "urn:relationship-" + std::to_string(i);
    relationship["creationInfo"] = "_:creation-info";
    relationship["from"] = rootId;
    relationship["to"][0] = dependencyId;
    relationship["type"] = "Relationship";
    relationship["relationshipType"] = "dependsOn";
  }
  ASSERT_EQUAL(value, expected);
  Json::Value written;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::istringstream input(output.str());
  ASSERT_TRUE(Json::parseFromStream(reader, input, &written, &errors));
  ASSERT_EQUAL(written, expected);
  return true;
}

}

int testSpdxSerializer(int, char*[])
{
  return runTests({ testSerializeSpdxJson, testSerializeAfterReallocation });
}
