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
  "software_primaryPurpose" : "application",
  "type" : "software_Package"
}
]=])

set(DEPENDENCY_EXPECTED [=[
{
  "spdxId" : "urn:bar:bar#Package",
  "name" : "bar:bar",
  "originatedBy" :
  [
    {
      "name" : "bar",
      "type" : "Organization"
    }
  ],
  "software_packageVersion" : "1.3.5",
  "type" : "software_Package"
}
]=])

set(BUILD_LINKED_LIBRARIES_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "description" : "Linked Libraries",
  "from" : "urn:test#Package",
  "relationshipType" : "dependsOn",
  "spdxId" : "urn:Static#Relationship",
  "to" :
  [
    "urn:bar:bar#Package"
  ],
  "type" : "Relationship"
}
]=])


expect_value("${content}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON CREATION_INFO GET "${content}" "@graph" "0")
expect_object("${CREATION_INFO}" CREATION_INFO_EXPECTED)

string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "1")
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_object("${SPDX_DOCUMENT}" APPLICATION_EXPECTED "rootElement" "0")
expect_object("${SPDX_DOCUMENT}" DEPENDENCY_EXPECTED "element" "0")
string(JSON LINKED_LIBRARIES GET "${content}" "@graph" "2")
expect_object("${LINKED_LIBRARIES}" BUILD_LINKED_LIBRARIES_EXPECTED)
