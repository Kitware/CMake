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

string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "1")
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
# liba should only appear once.
expect_array("${SPDX_DOCUMENT}" 1 "rootElement")
expect_object("${SPDX_DOCUMENT}" LIBA_EXPECTED "rootElement")
