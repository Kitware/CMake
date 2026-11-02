include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(CREATION_INFO_EXPECTED [=[
{
  "@id": "_:Build#CreationInfo",
  "comment": "This SBOM was generated from the CMakeLists.txt File",
  "createdBy": [
    "https://gitlab.kitware.com/cmake/cmake"
  ],
  "specVersion": "3.0.1",
  "type": "CreationInfo"
}
]=])

set(SPDX_DOCUMENT_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "name": "baz",
  "profileConformance": [
    "core",
    "software"
  ],
  "spdxId": "urn:baz#SPDXDocument",
  "type": "SpdxDocument"
}
]=])

set(APPLICATION_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "name": "baz",
  "software_primaryPurpose": "application",
  "spdxId": "urn:baz#Package",
  "type": "software_Package"
}
]=])

set(FOO_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "name": "foo",
  "software_primaryPurpose": "library",
  "spdxId": "urn:foo#Package",
  "type": "software_Package"
}
]=])

set(LINKED_LIBRARIES_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "description": "Required Build-Time Libraries",
  "from": "urn:baz#Package",
  "relationshipType": "dependsOn",
  "spdxId": "urn:baz#StaticRelationship",
  "to": [
    "urn:foo#Package"
  ],
  "type": "Relationship"
}
]=])

expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
string(JSON CREATION_INFO GET "${content}" @graph 0)
expect_object("${CREATION_INFO}" CREATION_INFO_EXPECTED)

string(JSON SPDX_DOCUMENT GET "${content}" @graph 1)
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_object("${SPDX_DOCUMENT}" APPLICATION_EXPECTED rootElement)
expect_object("${SPDX_DOCUMENT}" FOO_EXPECTED element)
expect_array("${SPDX_DOCUMENT}" 1 rootElement)
expect_array("${SPDX_DOCUMENT}" 1 element)

expect_object("${content}" LINKED_LIBRARIES_EXPECTED @graph)
expect_array("${content}" 3 @graph)
expect_array("${content}" 1 @graph 2 to)
