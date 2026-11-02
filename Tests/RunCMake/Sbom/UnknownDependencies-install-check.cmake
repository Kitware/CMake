include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
string(JSON document GET "${content}" @graph 1)
expect_array("${document}" 3 rootElement)
expect_array("${document}" 4 element)
expect_array("${content}" 7 @graph)

set(document_expected [=[
{
  "spdxId": "urn:unknown_dependencies#SPDXDocument",
  "type": "SpdxDocument",
  "name": "unknown_dependencies",
  "creationInfo": "_:Build#CreationInfo",
  "profileConformance": ["core", "software"]
}
]=])
expect_object("${document}" document_expected)
foreach(package IN ITEMS root scoped_First scoped_Second)
  string(CONFIGURE [=[
  {
    "spdxId": "urn:@package@#Package",
    "type": "software_Package",
    "name": "@package@",
    "creationInfo": "_:Build#CreationInfo",
    "software_primaryPurpose": "library"
  }
  ]=] root_expected @ONLY)
  expect_object("${document}" root_expected rootElement)
endforeach()

set(imported_expected [=[
{
  "spdxId": "urn:Unknown:dep#Package",
  "type": "software_Package",
  "name": "Unknown:dep",
  "creationInfo": "_:Build#CreationInfo",
  "originatedBy": [{"name": "Unknown", "type": "Organization"}],
  "externalRef": [{
    "type": "ExternalRef",
    "externalRefType": "buildSystem",
    "locator": "cmake:find_package(Unknown)"
  }]
}
]=])
expect_object("${document}" imported_expected element)
set(bare_expected [=[
{
  "spdxId": "urn:bare_library#Package",
  "type": "software_Package",
  "name": "bare_library",
  "creationInfo": "_:Build#CreationInfo"
}
]=])
expect_object("${document}" bare_expected element)

set(links_expected [=[
{
  "spdxId": "urn:root#StaticRelationship",
  "type": "Relationship",
  "creationInfo": "_:Build#CreationInfo",
  "from": "urn:root#Package",
  "relationshipType": "dependsOn",
  "description": "Required Build-Time Libraries",
  "to": ["urn:First:Scoped::dep#Package", "urn:Second:Scoped::dep#Package"]
}
]=])
expect_object("${content}" links_expected @graph)

set(packages First Second)
set(licenses MIT Apache-2.0)
foreach(package license IN ZIP_LISTS packages licenses)
  string(CONFIGURE [=[
  {
    "spdxId": "urn:@package@:Scoped::dep#Package",
    "type": "software_Package",
    "name": "@package@:Scoped::dep",
    "creationInfo": "_:Build#CreationInfo",
    "originatedBy": [{
      "spdxId": "urn:@package@#Organization",
      "type": "Organization",
      "name": "@package@",
      "creationInfo": "_:Build#CreationInfo"
    }],
    "externalRef": [{
      "type": "ExternalRef",
      "externalRefType": "buildSystem",
      "locator": "cmake:find_package(@package@)"
    }]
  }
  ]=] scoped_expected @ONLY)
  expect_object("${document}" scoped_expected element)

  string(CONFIGURE [=[
  {
    "spdxId": "urn:scoped_@package@#StaticRelationship",
    "type": "Relationship",
    "creationInfo": "_:Build#CreationInfo",
    "description": "Required Build-Time Libraries",
    "from": "urn:scoped_@package@#Package",
    "relationshipType": "dependsOn",
    "to": ["urn:@package@:Scoped::dep#Package"]
  }
  ]=] scoped_links_expected @ONLY)
  expect_object("${content}" scoped_links_expected @graph)

  string(CONFIGURE [=[
  {
    "spdxId": "urn:@package@:Scoped::dep#DeclaredLicenseRelationship",
    "type": "Relationship",
    "creationInfo": "_:Build#CreationInfo",
    "from": "urn:@package@:Scoped::dep#Package",
    "relationshipType": "hasDeclaredLicense",
    "to": [{
      "spdxId": "urn:@package@:Scoped::dep#LicenseExpression",
      "type": "simplelicensing_LicenseExpression",
      "creationInfo": "_:Build#CreationInfo",
      "simplelicensing_licenseExpression": "@license@"
    }]
  }
  ]=] license_expected @ONLY)
  expect_object("${content}" license_expected @graph)
endforeach()

foreach(index RANGE 2 6)
  string(JSON id GET "${content}" @graph ${index} spdxId)
  if(id STREQUAL "urn:root#StaticRelationship")
    expect_array("${content}" 2 @graph ${index} to)
  else()
    expect_array("${content}" 1 @graph ${index} to)
  endif()
endforeach()
