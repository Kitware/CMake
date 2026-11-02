include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

string(JSON document GET "${content}" @graph 1)
expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
expect_array("${document}" 2 rootElement)
expect_array("${content}" 3 @graph)
foreach(field IN ITEMS description dataLicense)
  string(JSON unused ERROR_VARIABLE error TYPE "${document}" "${field}")
  if(NOT error)
    set(RunCMake_TEST_FAILED "NO_PROJECT_METADATA inherited document ${field}")
  endif()
endforeach()

set(packages explicit_license no_license)
foreach(package IN LISTS packages)
  string(CONFIGURE [=[
  {
    "spdxId": "urn:@package@#Package",
    "type": "software_Package",
    "name": "@package@",
    "creationInfo": "_:Build#CreationInfo",
    "software_primaryPurpose": "library"
  }
  ]=] package_expected @ONLY)
  expect_object("${document}" package_expected rootElement)
endforeach()
foreach(index RANGE 0 1)
  foreach(field IN ITEMS software_packageVersion software_homePage)
    string(JSON unused ERROR_VARIABLE error TYPE "${document}" rootElement ${index} "${field}")
    if(NOT error)
      set(RunCMake_TEST_FAILED "NO_PROJECT_METADATA inherited target ${field}")
    endif()
  endforeach()
endforeach()
string(JSON unused ERROR_VARIABLE error TYPE "${document}" element)
if(NOT error)
  expect_array("${document}" 0 element)
endif()

set(explicit_expected [=[
{
  "spdxId": "urn:explicit_license#DeclaredLicenseRelationship",
  "type": "Relationship",
  "creationInfo": "_:Build#CreationInfo",
  "from": "urn:explicit_license#Package",
  "relationshipType": "hasDeclaredLicense",
  "to": [{
    "spdxId": "urn:explicit_license#LicenseExpression",
    "type": "simplelicensing_LicenseExpression",
    "creationInfo": "_:Build#CreationInfo",
    "simplelicensing_licenseExpression": "Apache-2.0"
  }]
}
]=])
expect_object("${content}" explicit_expected @graph)
expect_array("${content}" 1 @graph 2 to)
