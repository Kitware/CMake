
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

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
  spdxValue.DataLicense = "CC0-1.0";
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

    ASSERT_TRUE(value.isObject());

    Json::Value const& context = value["@context"];
    ASSERT_EQUAL(context.asString(), contextUrl);

    Json::Value const& graph = value["@graph"];
    ASSERT_TRUE(graph.isArray());

    Json::Value const& docValue = graph[0];
    ASSERT_TRUE(docValue.isObject());
    ASSERT_EQUAL(docValue["type"].asString(), "SpdxDocument");
    ASSERT_EQUAL(docValue["spdxId"].asString(), "_:SPDXRef-Document");

    auto const& creationInfo = docValue["creationInfo"];
    ASSERT_EQUAL(creationInfo["@id"].asString(), "_:SPDXRef-CreationInfo");
    ASSERT_EQUAL(creationInfo["type"].asString(), "CreationInfo");

    auto const& package = docValue["rootElement"][0];
    ASSERT_EQUAL(package["type"].asString(), "software_Package");
    ASSERT_EQUAL(package["spdxId"].asString(), "_:SPDXRef-Package");
  }

  return true;
}

} // namespace

int testSpdxSerializer(int /*unused*/, char* /*unused*/[])
{
  return runTests({ testSerializeSpdxJson });
}
