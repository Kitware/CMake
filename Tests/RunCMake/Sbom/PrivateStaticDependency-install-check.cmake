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
  "name": "foo",
  "profileConformance": [
    "core",
    "software"
  ],
  "spdxId": "urn:foo#SPDXDocument",
  "type": "SpdxDocument"
}
]=])

set(NUMBER_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "name": "foo",
  "software_primaryPurpose": "library",
  "spdxId": "urn:foo#Package",
  "type": "software_Package"
}
]=])

set(DEPENDENCY_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "name": "bar:bar",
  "originatedBy": [
    {
      "creationInfo": "_:Build#CreationInfo",
      "name": "bar",
      "spdxId": "urn:bar#Organization",
      "type": "Organization"
    }
  ],
  "software_packageVersion": "1.3.5",
  "spdxId": "urn:bar:bar#Package",
  "type": "software_Package"
}
]=])

set(SIBLING_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "name": "sibling",
  "software_primaryPurpose": "library",
  "spdxId": "urn:sibling#Package",
  "type": "software_Package"
}
]=])

set(LINKED_LIBRARIES_EXPECTED [=[
{
  "creationInfo": "_:Build#CreationInfo",
  "description": "Required Build-Time Libraries",
  "from": "urn:foo#Package",
  "relationshipType": "dependsOn",
  "spdxId": "urn:foo#StaticRelationship",
  "to": [
    "urn:bar:bar#Package",
    "urn:sibling#Package"
  ],
  "type": "Relationship"
}
]=])

expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
string(JSON CREATION_INFO GET "${content}" @graph 0)
expect_object("${CREATION_INFO}" CREATION_INFO_EXPECTED)

string(JSON SPDX_DOCUMENT GET "${content}" @graph 1)
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_object("${SPDX_DOCUMENT}" NUMBER_EXPECTED rootElement)
expect_object("${SPDX_DOCUMENT}" SIBLING_EXPECTED rootElement)
expect_object("${SPDX_DOCUMENT}" DEPENDENCY_EXPECTED element)
expect_array("${SPDX_DOCUMENT}" 2 rootElement)
expect_array("${SPDX_DOCUMENT}" 1 element)

expect_object("${content}" LINKED_LIBRARIES_EXPECTED @graph)
expect_array("${content}" 3 @graph)
expect_array("${content}" 2 @graph 2 to)
