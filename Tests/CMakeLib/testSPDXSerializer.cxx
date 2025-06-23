/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <iostream>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/type_traits>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmSPDXSerializer.h"

namespace {

std::string const nonOptional(R"================({
  "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
  "@graph": [
    {
      "@id": "_:contentIdentifier_0",
      "contentIdentifierType": "INVALID_CONTENT_IDENTIFIER_TYPE_ID",
      "contentIdentifierValue": "",
      "type": "ContentIdentifier"
    },
    {
      "@id": "_:creationInfo_0",
      "created": "",
      "createdBy": [],
      "type": "CreationInfo"
    },
    {
      "@id": "_:creationInfo_1",
      "created": "",
      "createdBy": [],
      "type": "CreationInfo"
    },
    {
      "@id": "_:dictionaryEntry_0",
      "key": "",
      "type": "DictionaryEntry"
    },
    {
      "@id": "_:externalIdentifier_0",
      "externalIdentifierType": "INVALID_EXTERNAL_IDENTIFIER_TYPE_ID",
      "identifier": "",
      "type": "ExternalIdentifier"
    },
    {
      "@id": "_:externalMap_0",
      "externalSpdxId": "",
      "type": "ExternalMap"
    },
    {
      "@id": "_:externalRef_0",
      "type": "ExternalRef"
    },
    {
      "@id": "_:hash_0",
      "algorithm": "INVALID_HASH_TYPE_ID",
      "hashValue": "",
      "type": "Hash"
    },
    {
      "@id": "_:namespaceMap_0",
      "namespace": "",
      "prefix": "",
      "type": "NamespaceMap"
    },
    {
      "@id": "_:packageVerificationCode_0",
      "algorithm": "INVALID_HASH_TYPE_ID",
      "hashValue": "",
      "type": "PackageVerificationCode"
    },
    {
      "@id": "_:positiveIntegerRange_0",
      "beginIntegerRange": 0,
      "endIntegerRange": 0,
      "type": "PositiveIntegerRange"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd0",
      "type": "Agent"
    },
    {
      "annotationType": "INVALID_ANNOTATION_TYPE_ID",
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd1",
      "subject": "",
      "type": "Annotation"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd10",
      "type": "SpdxDocument"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd11",
      "type": "Tool"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd12",
      "type": "File"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd13",
      "type": "Package"
    },
    {
      "context": "",
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd14",
      "type": "Sbom"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "snippetFromFile": "",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd15",
      "type": "Snippet"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "licenseExpression": "",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd16",
      "type": "LicenseExpression"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "licenseText": "",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd17",
      "type": "SimpleLicensingText"
    },
    {
      "context": "",
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd2",
      "type": "Bom"
    },
    {
      "context": "",
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd3",
      "type": "Bundle"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd4",
      "type": "IndividualElement"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "from": "",
      "relationshipType": "INVALID_RELATIONSHIP_TYPE_ID",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd5",
      "to": [],
      "type": "LifecycleScopedRelationship"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd6",
      "type": "Organization"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd7",
      "type": "Person"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "from": "",
      "relationshipType": "INVALID_RELATIONSHIP_TYPE_ID",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd8",
      "to": [],
      "type": "Relationship"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd9",
      "type": "SoftwareAgent"
    }
  ]
})================");

std::string const Optional(R"================({
  "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
  "@graph": [
    {
      "@id": "_:contentIdentifier_0",
      "contentIdentifierType": "gitoid",
      "contentIdentifierValue": "ContentIdentifierValue",
      "type": "ContentIdentifier"
    },
    {
      "@id": "_:creationInfo_0",
      "created": "",
      "createdBy": [],
      "type": "CreationInfo"
    },
    {
      "@id": "_:creationInfo_1",
      "Comment": "Comment",
      "created": "Created",
      "createdBy": [
        "testRef"
      ],
      "createdUsing": [
        "testRef"
      ],
      "type": "CreationInfo"
    },
    {
      "@id": "_:dictionaryEntry_0",
      "key": "Key",
      "type": "DictionaryEntry",
      "value": "Value"
    },
    {
      "@id": "_:externalIdentifier_0",
      "comment": "Comment",
      "externalIdentifierType": "other",
      "identifier": "Identifier",
      "identifierLocator": [
        "IdentifierLocator"
      ],
      "issuingAuthority": "IssuingAuthority",
      "type": "ExternalIdentifier"
    },
    {
      "@id": "_:externalMap_0",
      "definingArtifact": "testRef",
      "externalSpdxId": "ExternalSpdxId",
      "integrityMethod": [
        "testRef"
      ],
      "locationHint": "LocationHint",
      "type": "ExternalMap"
    },
    {
      "@id": "_:externalRef_0",
      "comment": "Comment",
      "contentType": "ContentType",
      "externalRefType": "other",
      "locator": [
        "Locator"
      ],
      "type": "ExternalRef"
    },
    {
      "@id": "_:hash_0",
      "algorithm": "other",
      "hashValue": "HashValue",
      "type": "Hash"
    },
    {
      "@id": "_:namespaceMap_0",
      "namespace": "Namespace",
      "prefix": "Namespace",
      "type": "NamespaceMap"
    },
    {
      "@id": "_:packageVerificationCode_0",
      "algorithm": "other",
      "hashValue": "HashValue",
      "type": "PackageVerificationCode"
    },
    {
      "@id": "_:positiveIntegerRange_0",
      "beginIntegerRange": 1,
      "endIntegerRange": 2,
      "type": "PositiveIntegerRange"
    },
    {
      "comment": "Comment",
      "creationInfo": "_:creationInfo_0",
      "description": "Description",
      "extension": [
        "testRef"
      ],
      "externalIdentifier": [
        "testRef"
      ],
      "externalRef": [
        "testRef"
      ],
      "name": "Name",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd0",
      "summary": "Summary",
      "type": "Agent",
      "verifiedUsing": [
        "testRef"
      ]
    },
    {
      "annotationType": "other",
      "contentType": "ContentType",
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd1",
      "statement": "Statement",
      "subject": "testRef",
      "type": "Annotation"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "dataLicense": "testRef",
      "externalMap": "testRef",
      "namespaceMap": "testRef",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd10",
      "type": "SpdxDocument"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd11",
      "type": "Tool"
    },
    {
      "additionalPurpose": [
        "other"
      ],
      "attributionText": "AttributionText",
      "contentIdentifier": "testRef",
      "contentType": "ContentType",
      "copyrightText": "CopyrightText",
      "creationInfo": "_:creationInfo_0",
      "fileKind": "file",
      "primaryPurpose": "file",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd12",
      "type": "File"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "downloadLocation": "DownloadLocation",
      "homePage": "HomePage",
      "packageUrl": "PackageUrl",
      "packageVersion": "PackageVersion",
      "sourceInfo": "SourceInfo",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd13",
      "type": "Package"
    },
    {
      "context": "",
      "creationInfo": "_:creationInfo_0",
      "sbomType": [
        "build"
      ],
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd14",
      "type": "Sbom"
    },
    {
      "byteRange": "testRef",
      "creationInfo": "_:creationInfo_0",
      "lineRange": "testRef",
      "snippetFromFile": "testRef",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd15",
      "type": "Snippet"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "customIdToUri": [
        "testRef"
      ],
      "licenseExpression": "LicenseExpression",
      "licenseListVersion": "LicenseListVersion",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd16",
      "type": "LicenseExpression"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "licenseText": "LicenseText",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd17",
      "type": "SimpleLicensingText"
    },
    {
      "context": "Context",
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd2",
      "type": "Bom"
    },
    {
      "context": "",
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd3",
      "type": "Bundle"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd4",
      "type": "IndividualElement"
    },
    {
      "completeness": "noAssertion",
      "creationInfo": "_:creationInfo_0",
      "endTime": "EndTime",
      "from": "testRef",
      "relationshipType": "other",
      "scope": "other",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd5",
      "startTime": "StartTime",
      "to": [
        "testRef"
      ],
      "type": "LifecycleScopedRelationship"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd6",
      "type": "Organization"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd7",
      "type": "Person"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "from": "",
      "relationshipType": "INVALID_RELATIONSHIP_TYPE_ID",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd8",
      "to": [],
      "type": "Relationship"
    },
    {
      "creationInfo": "_:creationInfo_0",
      "spdxId": "https://cmake.org/testSPDXSerialization-gnrtd9",
      "type": "SoftwareAgent"
    }
  ]
})================");
}

int testNonOptional()
{
  cmSPDXSimpleGraph graph("https://cmake.org/testSPDXSerialization-gnrtd");

  // Core
  graph.insert<cmSPDXAgent>();
  graph.insert<cmSPDXAnnotation>();
  graph.insert<cmSPDXBom>();
  graph.insert<cmSPDXBundle>();
  graph.insert<cmSPDXCreationInfo>();
  graph.insert<cmSPDXDictionaryEntry>();
  graph.insert<cmSPDXExternalIdentifier>();
  graph.insert<cmSPDXExternalMap>();
  graph.insert<cmSPDXExternalRef>();
  graph.insert<cmSPDXHash>();
  graph.insert<cmSPDXIndividualElement>();
  graph.insert<cmSPDXLifecycleScopedRelationship>();
  graph.insert<cmSPDXNamespaceMap>();
  graph.insert<cmSPDXOrganization>();
  graph.insert<cmSPDXPackageVerificationCode>();
  graph.insert<cmSPDXPerson>();
  graph.insert<cmSPDXPositiveIntegerRange>();
  graph.insert<cmSPDXRelationship>();
  graph.insert<cmSPDXSoftwareAgent>();
  graph.insert<cmSPDXSpdxDocument>();
  graph.insert<cmSPDXTool>();

  // Software
  graph.insert<cmSPDXContentIdentifier>();
  graph.insert<cmSPDXFile>();
  graph.insert<cmSPDXPackage>();
  graph.insert<cmSPDXSbom>();
  graph.insert<cmSPDXSnippet>();

  // SimpleLicensing
  graph.insert<cmSPDXLicenseExpression>();
  graph.insert<cmSPDXSimpleLicensingText>();

  Json::Value root;
  Json::Reader().parse(nonOptional.c_str(), root);

  std::cout << "NonOptional SPDX:";
  std::cout << "\nConstructed Graph: " << graph.toJsonLD().toStyledString();
  std::cout << "\nComparison Graph:" << root.toStyledString() << "\n";

  // Convert to string to disregard differences in number signedness
  return root.toStyledString() == graph.toJsonLD().toStyledString();
};

int testOptional()
{
  cmSPDXSimpleGraph graph("https://cmake.org/testSPDXSerialization-gnrtd");

  cmSPDXIdentifierReference ident("testRef");

  // Core
  auto& agent = graph.insert<cmSPDXAgent>();
  agent.Comment = "Comment";
  agent.Description = "Description";
  agent.Extension.emplace().push_back(ident);
  agent.ExternalIdentifier.emplace().push_back(ident);
  agent.ExternalRef.emplace().push_back(ident);
  agent.Name = "Name";
  agent.Summary = "Summary";
  agent.VerifiedUsing.emplace().push_back(ident);

  auto& annotation = graph.insert<cmSPDXAnnotation>();
  annotation.AnnotationType = cmSPDXAnnotationType::OTHER;
  annotation.ContentType = "ContentType";
  annotation.Statement = "Statement";
  annotation.Subject = ident;

  auto& bom = graph.insert<cmSPDXBom>();
  bom.Context = "Context";

  graph.insert<cmSPDXBundle>();

  auto& creationInfo = graph.insert<cmSPDXCreationInfo>();
  creationInfo.Comment = "Comment";
  creationInfo.Created = "Created";
  creationInfo.CreatedBy.push_back(ident);
  creationInfo.CreatedUsing.emplace().push_back(ident);

  auto& dictionaryEntry = graph.insert<cmSPDXDictionaryEntry>();
  dictionaryEntry.Key = "Key";
  dictionaryEntry.Value = "Value";

  auto& externalIdentifier = graph.insert<cmSPDXExternalIdentifier>();
  externalIdentifier.Comment = "Comment";
  externalIdentifier.ExternalIdentifierType =
    cmSPDXExternalIdentifierType::OTHER;
  externalIdentifier.Identifier = "Identifier";
  externalIdentifier.IdentifierLocator.emplace().push_back(
    "IdentifierLocator");
  externalIdentifier.IssuingAuthority = "IssuingAuthority";

  auto& externalMap = graph.insert<cmSPDXExternalMap>();
  externalMap.DefiningArtifact = ident;
  externalMap.ExternalSpdxId = "ExternalSpdxId";
  externalMap.LocationHint = "LocationHint";
  externalMap.IntegrityMethod.emplace().push_back(ident);

  auto& externalRef = graph.insert<cmSPDXExternalRef>();
  externalRef.Comment = "Comment";
  externalRef.ContentType = "ContentType";
  externalRef.ExternalRefType = cmSPDXExternalRefType::OTHER;
  externalRef.Locator.emplace().push_back("Locator");

  auto& hash = graph.insert<cmSPDXHash>();
  hash.Algorithm = cmSPDXHashAlgorithm::OTHER;
  hash.HashValue = "HashValue";

  graph.insert<cmSPDXIndividualElement>();

  auto& lifecycleScopedRelationship =
    graph.insert<cmSPDXLifecycleScopedRelationship>();
  lifecycleScopedRelationship.Completeness =
    cmSPDXRelationshipCompletenessType::NO_ASSERTION;
  lifecycleScopedRelationship.EndTime = "EndTime";
  lifecycleScopedRelationship.From = ident;
  lifecycleScopedRelationship.RelationshipType = cmSPDXRelationshipType::OTHER;
  lifecycleScopedRelationship.StartTime = "StartTime";
  lifecycleScopedRelationship.To.push_back(ident);
  lifecycleScopedRelationship.Scope = cmSPDXLifecycleScopeType::OTHER;

  auto& namespaceMap = graph.insert<cmSPDXNamespaceMap>();
  namespaceMap.Namespace = "Namespace";
  namespaceMap.Prefix = "Prefix";

  graph.insert<cmSPDXOrganization>();

  auto& packageVerificationCode =
    graph.insert<cmSPDXPackageVerificationCode>();
  packageVerificationCode.Algorithm = cmSPDXHashAlgorithm::OTHER;
  packageVerificationCode.HashValue = "HashValue";
  packageVerificationCode.PackageVerificationCodeExcludedFile.emplace()
    .push_back("PacakgeVerificationCodeExcludeFile");

  graph.insert<cmSPDXPerson>();

  auto& positiveIntegerRange = graph.insert<cmSPDXPositiveIntegerRange>();
  positiveIntegerRange.BeginIntegerRange = 1;
  positiveIntegerRange.EndIntegerRange = 2;

  graph.insert<cmSPDXRelationship>();

  graph.insert<cmSPDXSoftwareAgent>();

  auto& spdxDocument = graph.insert<cmSPDXSpdxDocument>();
  spdxDocument.DataLicense = ident;
  spdxDocument.ExternalMap = ident;
  spdxDocument.NamespaceMap = ident;

  graph.insert<cmSPDXTool>();

  // Software
  auto& contentIdentifier = graph.insert<cmSPDXContentIdentifier>();
  contentIdentifier.ContentIdentifierType =
    cmSPDXContentIdentifierType::GITOID;
  contentIdentifier.ContentIdentifierValue = "ContentIdentifierValue";

  auto& file = graph.insert<cmSPDXFile>();
  file.AdditionalPurpose.emplace().push_back(cmSPDXSoftwarePurpose::OTHER);
  file.AttributionText = "AttributionText";
  file.ContentIdentifier = ident;
  file.CopyrightText = "CopyrightText";
  file.PrimaryPurpose = cmSPDXSoftwarePurpose::FILE;
  file.ContentType = "ContentType";
  file.FileKind = cmSPDXFileKindType::FILE;

  auto& package = graph.insert<cmSPDXPackage>();
  package.DownloadLocation = "DownloadLocation";
  package.HomePage = "HomePage";
  package.PackageUrl = "PackageUrl";
  package.PackageVersion = "PackageVersion";
  package.SourceInfo = "SourceInfo";

  auto& sbom = graph.insert<cmSPDXSbom>();
  sbom.SbomType.emplace().push_back(cmSPDXSbomType::BUILD);

  auto& snippet = graph.insert<cmSPDXSnippet>();
  snippet.ByteRange = ident;
  snippet.LineRange = ident;
  snippet.SnippetFromFile = ident;

  // SimpleLicensing
  auto& licenseExpression = graph.insert<cmSPDXLicenseExpression>();
  licenseExpression.CustomIdToUri.emplace().push_back(ident);
  licenseExpression.LicenseExpression = "LicenseExpression";
  licenseExpression.LicenseListVersion = "LicenseListVersion";

  auto& simpleLicensingText = graph.insert<cmSPDXSimpleLicensingText>();
  simpleLicensingText.LicenseText = "LicenseText";

  Json::Value root;
  Json::Reader().parse(Optional.c_str(), root);

  std::cout << "Optional SPDX:";
  std::cout << "\nConstructed Graph: " << graph.toJsonLD().toStyledString();
  std::cout << "\nComparison Graph:" << root.toStyledString() << "\n";

  // Convert to string to disregard differences in number signedness
  return root.toStyledString() == graph.toJsonLD().toStyledString();
};

int testSPDXSerializer(int /* argc */, char* /* argv */[])
{
  if (!testNonOptional())
    return -1;

  if (!testOptional())
    return -1;

  return 0;
}
