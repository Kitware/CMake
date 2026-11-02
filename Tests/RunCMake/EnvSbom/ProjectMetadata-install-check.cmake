file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test_targets/test_targets-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata-install-check.cmake)

expect_value("${SPDX_DOCUMENT}" "Metadata Test Project" description)
expect_value("${SPDX_DOCUMENT}" BSD-3 dataLicense simplelicensing_licenseExpression)
expect_value("${SPDX_DOCUMENT}" "urn:test_targets#DataLicenseExpression" dataLicense spdxId)
set(PROJECT_METADATA_TEMPLATE [=[
{
  "spdxId": "urn:@package@#Package",
  "software_packageVersion": "1.2.0",
  "software_homePage": "www.example.com"
}
]=])
foreach(package IN ITEMS test test_targets)
  string(CONFIGURE "${PROJECT_METADATA_TEMPLATE}" PROJECT_METADATA_EXPECTED @ONLY)
  expect_object("${SPDX_DOCUMENT}" PROJECT_METADATA_EXPECTED rootElement)
endforeach()
