include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(bar_expected [=[{"spdxId":"urn:bar#Package"}]=])
set(child_expected [=[{"spdxId":"urn:child#Package"}]=])

set(license_expected [=[
{
  "from": "urn:bar#Package",
  "relationshipType": "hasDeclaredLicense",
  "to": [{"simplelicensing_licenseExpression": "MIT"}]
}
]=])

set(child_license_expected [=[
{
  "from": "urn:child#Package",
  "relationshipType": "hasDeclaredLicense",
  "to": [{"simplelicensing_licenseExpression": "Apache-2.0"}]
}
]=])

string(JSON document GET "${content}" @graph 1)
if(sbom STREQUAL "bundle")
  expect_value("${document}" "Description from project variables" description)
  expect_value("${document}" MIT dataLicense simplelicensing_licenseExpression)
elseif(sbom STREQUAL "selected")
  expect_value("${document}" "Selected project" description)
  expect_value("${document}" ISC dataLicense simplelicensing_licenseExpression)
else()
  foreach(field IN ITEMS description dataLicense)
    string(JSON unused ERROR_VARIABLE error TYPE "${document}" "${field}")
    if(NOT error)
      set(RunCMake_TEST_FAILED "An unmatched SBOM inherited document ${field}")
    endif()
  endforeach()
endif()

expect_array("${document}" 2 rootElement)
expect_object("${document}" bar_expected rootElement)
expect_object("${document}" child_expected rootElement)
expect_object("${content}" license_expected @graph)
expect_object("${content}" child_license_expected @graph)
expect_array("${content}" 4 @graph)
