include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(SPDX_DOCUMENT_EXPECTED [=[
{
  "spdxId" : "urn:mySbom#SPDXDocument",
  "name" : "mySbom",
  "type" : "SpdxDocument"
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

set(LIBA_BARE_EXPECTED [=[
{
  "spdxId" : "urn:liba#Package",
  "name" : "liba",
  "type" : "software_Package"
}
]=])

set(LIBB_BUILD_REQUIRES_EXPECTED [=[
{
  "description" : "Required Build-Time Libraries",
  "from" : "urn:libb#Package",
  "relationshipType" : "dependsOn",
  "spdxId" : "urn:Shared#Relationship",
  "to" :
  [
    "urn:liba#Package"
  ],
  "type" : "Relationship"
}
]=])

string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "1")
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_array("${SPDX_DOCUMENT}" 1 "rootElement")
expect_object("${SPDX_DOCUMENT}" LIBB_EXPECTED "rootElement")
expect_object("${SPDX_DOCUMENT}" LIBA_BARE_EXPECTED "element")
string(JSON BUILD_REQUIRES GET "${content}" "@graph" "2")
expect_object("${BUILD_REQUIRES}" LIBB_BUILD_REQUIRES_EXPECTED)
