file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/test_targets/test_targets-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata-install-check.cmake)

set(DOCUMENT_METADATA_EXPECTED [=[
{
  "description" : "An eloquent description",
  "dataLicense" :
  {
    "creationInfo" : "_:Build#CreationInfo",
    "simplelicensing_licenseExpression" : "BSD-3",
    "spdxId" : "urn:test_targets#DataLicenseExpression",
    "type" : "simplelicensing_LicenseExpression"
  }
}
]=])

set(PACKAGE_METADATA_TEMPLATE [=[
{
  "spdxId" : "urn:@package@#Package",
  "software_packageVersion" : "1.3.4",
  "software_homePage" : "www.example.com",
  "software_downloadLocation" : "https://example.com/test_targets.tar.gz"
}
]=])

expect_object("${SPDX_DOCUMENT}" DOCUMENT_METADATA_EXPECTED)
foreach(package IN ITEMS test test_targets)
  string(CONFIGURE "${PACKAGE_METADATA_TEMPLATE}" PACKAGE_METADATA_EXPECTED @ONLY)
  expect_object("${SPDX_DOCUMENT}" PACKAGE_METADATA_EXPECTED rootElement)
endforeach()
