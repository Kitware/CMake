include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

string(JSON document GET "${content}" @graph 1)
expect_array("${document}" 3 rootElement)
expect_array("${content}" 4 @graph)

set(explicit_expected [=[
{
  "from": "urn:explicit_license#Package",
  "relationshipType": "hasDeclaredLicense",
  "to": [{"simplelicensing_licenseExpression": "Apache-2.0"}]
}
]=])
expect_object("${content}" explicit_expected @graph)

set(default_expected [=[
{
  "from": "urn:default_license#Package",
  "relationshipType": "hasDeclaredLicense",
  "to": [{"simplelicensing_licenseExpression": "MIT"}]
}
]=])
expect_object("${content}" default_expected @graph)
