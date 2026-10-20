include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(CREATION_INFO_EXPECTED [=[
{
  "@id": "_:Build#CreationInfo",
  "type": "CreationInfo",
  "comment": "This SBOM was generated from the CMakeLists.txt File",
  "createdBy": ["https://gitlab.kitware.com/cmake/cmake"],
  "specVersion": "3.0.1"
}
]=])

set(SPDX_DOCUMENT_EXPECTED [=[
{
  "spdxId": "urn:foo#SPDXDocument",
  "type": "SpdxDocument",
  "name": "foo",
  "creationInfo": "_:Build#CreationInfo",
  "profileConformance": ["core", "software"]
}
]=])

set(FOO_EXPECTED [=[
{
  "spdxId": "urn:foo#Package",
  "type": "software_Package",
  "name": "foo",
  "creationInfo": "_:Build#CreationInfo",
  "software_primaryPurpose": "library"
}
]=])

set(DEPENDENCY_EXPECTED [=[
{
  "spdxId": "urn:m#Package",
  "type": "software_Package",
  "name": "m",
  "creationInfo": "_:Build#CreationInfo"
}
]=])

expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
string(JSON CREATION_INFO GET "${content}" @graph 0)
expect_object("${CREATION_INFO}" CREATION_INFO_EXPECTED)

string(JSON SPDX_DOCUMENT GET "${content}" @graph 1)
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_object("${SPDX_DOCUMENT}" FOO_EXPECTED rootElement)
expect_object("${SPDX_DOCUMENT}" DEPENDENCY_EXPECTED element)
expect_array("${SPDX_DOCUMENT}" 1 rootElement)
expect_array("${SPDX_DOCUMENT}" 1 element)
expect_array("${content}" 2 @graph)
