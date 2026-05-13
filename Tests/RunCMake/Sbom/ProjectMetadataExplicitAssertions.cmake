include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(DOCUMENT_METADATA_EXPECTED [=[
{
  "description" : "An eloquent description",
  "dataLicense" :
  {
    "creationInfo" : "_:Build#CreationInfo",
    "simplelicensing_licenseExpression" : "BSD-3",
    "spdxId" : "urn:test_targets#LicenseExpression",
    "type" : "simplelicensing_LicenseExpression"
  }
}
]=])

set(PACKAGE_METADATA_EXPECTED [=[
{
  "spdxId" : "urn:test#Package",
  "software_packageVersion" : "1.3.4",
  "software_homePage" : "www.example.com",
  "software_downloadLocation" : "https://example.com/test_targets.tar.gz"
}
]=])

string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "1")
expect_object("${SPDX_DOCUMENT}" DOCUMENT_METADATA_EXPECTED)
expect_object("${SPDX_DOCUMENT}" PACKAGE_METADATA_EXPECTED "rootElement")
