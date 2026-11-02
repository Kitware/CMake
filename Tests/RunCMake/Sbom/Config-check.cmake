include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(creation_expected [=[
{
  "@id": "_:Build#CreationInfo",
  "type": "CreationInfo",
  "comment": "This SBOM was generated from the CMakeLists.txt File",
  "createdBy": ["https://gitlab.kitware.com/cmake/cmake"],
  "specVersion": "3.0.1"
}
]=])
set(document_expected [=[
{
  "spdxId": "urn:foo#SPDXDocument",
  "type": "SpdxDocument",
  "name": "foo",
  "creationInfo": "_:Build#CreationInfo",
  "profileConformance": ["core", "software"]
}
]=])

set(configs Debug)
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  list(APPEND configs Release)
endif()
foreach(config IN LISTS configs)
  set(content "${${config}Sbom}")
  if(config STREQUAL "Debug")
    set(dependency debug_dependency)
  else()
    set(dependency release_dependency)
  endif()

  expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
  expect_array("${content}" 7 @graph)
  expect_object("${content}" creation_expected @graph 0)
  expect_object("${content}" document_expected @graph 1)
  string(JSON document GET "${content}" @graph 1)
  expect_array("${document}" 1 rootElement)
  expect_array("${document}" 2 element)

  set(root_expected [=[
  {
    "spdxId": "urn:foo#Package",
    "type": "software_Package",
    "name": "foo",
    "creationInfo": "_:Build#CreationInfo",
    "software_primaryPurpose": "library"
  }
  ]=])
  expect_object("${document}" root_expected rootElement)
  string(CONFIGURE [=[
  {
    "spdxId": "urn:@dependency@#Package",
    "type": "software_Package",
    "name": "@dependency@",
    "creationInfo": "_:Build#CreationInfo",
    "software_primaryPurpose": "library"
  }
  ]=] dependency_expected @ONLY)
  expect_object("${document}" dependency_expected element)
  set(common_expected [=[
  {
    "spdxId": "urn:Common:Common#Package",
    "type": "software_Package",
    "name": "Common:Common",
    "creationInfo": "_:Build#CreationInfo",
    "originatedBy": [{
      "spdxId": "urn:Common#Organization",
      "type": "Organization",
      "name": "Common",
      "creationInfo": "_:Build#CreationInfo"
    }],
    "externalRef": [{
      "type": "ExternalRef",
      "externalRefType": "buildSystem",
      "locator": "cmake:find_package(Common)"
    }]
  }
  ]=])
  expect_object("${document}" common_expected element)

  string(CONFIGURE [=[
  {
    "spdxId": "urn:foo#StaticRelationship",
    "type": "Relationship",
    "creationInfo": "_:Build#CreationInfo",
    "description": "Required Build-Time Libraries",
    "from": "urn:foo#Package",
    "relationshipType": "dependsOn",
    "to": ["urn:Common:Common#Package", "urn:@dependency@#Package"]
  }
  ]=] root_links_expected @ONLY)

  expect_object("${content}" root_links_expected @graph)
  string(CONFIGURE [=[
  {
    "spdxId": "urn:@dependency@#StaticRelationship",
    "type": "Relationship",
    "creationInfo": "_:Build#CreationInfo",
    "description": "Required Build-Time Libraries",
    "from": "urn:@dependency@#Package",
    "relationshipType": "dependsOn",
    "to": ["urn:Common:Common#Package"]
  }
  ]=] dependency_links_expected @ONLY)
  expect_object("${content}" dependency_links_expected @graph)

  foreach(package IN ITEMS foo "${dependency}" Common:Common)
    string(CONFIGURE [=[
    {
      "spdxId": "urn:@package@#DeclaredLicenseRelationship",
      "type": "Relationship",
      "creationInfo": "_:Build#CreationInfo",
      "from": "urn:@package@#Package",
      "relationshipType": "hasDeclaredLicense",
      "to": [{
        "spdxId": "urn:@package@#LicenseExpression",
        "type": "simplelicensing_LicenseExpression",
        "creationInfo": "_:Build#CreationInfo",
        "simplelicensing_licenseExpression": "MIT"
      }]
    }
    ]=] license_expected @ONLY)
    expect_object("${content}" license_expected @graph)
  endforeach()

  foreach(index RANGE 2 6)
    string(JSON id GET "${content}" @graph ${index} spdxId)
    if(id STREQUAL "urn:foo#StaticRelationship")
      expect_array("${content}" 2 @graph ${index} to)
    else()
      expect_array("${content}" 1 @graph ${index} to)
    endif()
  endforeach()
endforeach()
