include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(SPDX_DOCUMENT_EXPECTED [=[
{
  "spdxId" : "urn:mySbom#SPDXDocument",
  "name" : "mySbom",
  "type" : "SpdxDocument"
}
]=])

set(LIBA_EXPECTED [=[
{
  "spdxId" : "urn:liba#Package",
  "name" : "liba",
  "software_primaryPurpose" : "library",
  "type" : "software_Package"
}
]=])

string(JSON SPDX_DOCUMENT GET "${content}" @graph 1)
expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
expect_array("${content}" 2 @graph)
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_array("${SPDX_DOCUMENT}" 1 rootElement)
expect_object("${SPDX_DOCUMENT}" LIBA_EXPECTED rootElement)
string(JSON unused ERROR_VARIABLE error TYPE "${SPDX_DOCUMENT}" element)
if(NOT error)
  expect_array("${SPDX_DOCUMENT}" 0 element)
endif()
