include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(CREATION_INFO_EXPECTED [=[
{
  "@id": "_:Build#CreationInfo",
  "comment": "This SBOM was generated from the CMakeLists.txt File",
  "createdBy":
  [
    "https://gitlab.kitware.com/cmake/cmake"
  ],
  "specVersion": "3.0.1",
  "type": "CreationInfo"
}
]=])

set(SPDX_DOCUMENT_EXPECTED [=[
{
  "name" : "test_targets",
  "profileConformance" :
  [
    "core",
    "software"
  ],
  "creationInfo" : "_:Build#CreationInfo",
  "spdxId" : "urn:test_targets#SPDXDocument",
  "type" : "SpdxDocument"
}
]=])

set(APPLICATION_EXPECTED [=[
{
  "spdxId" : "urn:test#Package",
  "name" : "test",
  "software_primaryPurpose" : "library",
  "type" : "software_Package"
}
]=])

set(SECOND_TARGET_EXPECTED [=[
{
  "spdxId": "urn:test_targets#Package",
  "name": "test_targets",
  "software_primaryPurpose": "library",
  "type": "software_Package"
}
]=])

set(LICENSE_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "from" : "urn:test#Package",
  "relationshipType" : "hasDeclaredLicense",
  "spdxId" : "urn:test#DeclaredLicenseRelationship",
  "to" :
  [
    {
      "creationInfo" : "_:Build#CreationInfo",
      "simplelicensing_licenseExpression" : "license-for-test-target",
      "spdxId" : "urn:test#LicenseExpression",
      "type" : "simplelicensing_LicenseExpression"
    }
  ],
  "type" : "Relationship"
}
]=])

set(SECOND_LICENSE_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "from": "urn:test_targets#Package",
  "relationshipType": "hasDeclaredLicense",
  "spdxId": "urn:test_targets#DeclaredLicenseRelationship",
  "to": [{
    "creationInfo": "_:Build#CreationInfo",
    "simplelicensing_licenseExpression": "Apache-2.0",
    "spdxId": "urn:test_targets#LicenseExpression",
    "type": "simplelicensing_LicenseExpression"
  }],
  "type": "Relationship"
}
]=])


expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
string(JSON CREATION_INFO GET "${content}" @graph 0)
expect_object("${CREATION_INFO}" CREATION_INFO_EXPECTED)

string(JSON SPDX_DOCUMENT GET "${content}" @graph 1)
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_object("${SPDX_DOCUMENT}" APPLICATION_EXPECTED rootElement)
expect_object("${SPDX_DOCUMENT}" SECOND_TARGET_EXPECTED rootElement)
expect_array("${SPDX_DOCUMENT}" 2 rootElement)

expect_object("${content}" LICENSE_EXPECTED @graph)
expect_object("${content}" SECOND_LICENSE_EXPECTED @graph)
expect_array("${content}" 4 @graph)
expect_array("${content}" 1 @graph 2 to)
expect_array("${content}" 1 @graph 3 to)
