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

set(LIBB_EXPECTED [=[
{
  "spdxId" : "urn:libb#Package",
  "name" : "libb",
  "software_primaryPurpose" : "library",
  "type" : "software_Package"
}
]=])

set(LIBB_LINKED_LIBRARIES_EXPECTED [=[
{
  "description" : "Linked Libraries",
  "from" : "urn:libb#Package",
  "relationshipType" : "dependsOn",
  "to" :
  [
    "urn:liba#Package"
  ],
  "type" : "Relationship"
}
]=])

expect_value("${content}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "1")
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_array("${SPDX_DOCUMENT}" 2 "rootElement")
expect_object("${SPDX_DOCUMENT}" LIBA_EXPECTED "rootElement")
expect_object("${SPDX_DOCUMENT}" LIBB_EXPECTED "rootElement")
string(JSON LINKED_LIBRARIES GET "${content}" "@graph" "2")
expect_object("${LINKED_LIBRARIES}" LIBB_LINKED_LIBRARIES_EXPECTED)
