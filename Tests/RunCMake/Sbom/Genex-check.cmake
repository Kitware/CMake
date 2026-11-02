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

set(configs BarConfig)
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  list(APPEND configs BazConfig)
endif()
foreach(config IN LISTS configs)
  set(content "${${config}}")
  if(config STREQUAL "BarConfig")
    set(external_dependency bar:bar)
    set(origin bar)
    set(version 1.3.5)
  else()
    set(external_dependency baz:baz)
    set(origin baz)
    set(version 1.8.5)
  endif()

  expect_value("${content}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
  expect_array("${content}" 18 @graph)
  expect_object("${content}" creation_expected @graph 0)
  expect_object("${content}" document_expected @graph 1)
  string(JSON document GET "${content}" @graph 1)
  expect_array("${document}" 10 rootElement)
  expect_array("${document}" 6 element)

  foreach(package IN ITEMS foo dep1 dep2 runtime_dep
      plain_STATIC whole_STATIC group_STATIC
      plain_SHARED whole_SHARED group_SHARED)
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
  foreach(package IN ITEMS resolved_dependency compile_dependency compile_leaf
      "${interface_dependency}")
    string(CONFIGURE [=[
    {
      "spdxId": "urn:@package@#Package",
      "type": "software_Package",
      "name": "@package@",
      "creationInfo": "_:Build#CreationInfo",
      "software_primaryPurpose": "library"
    }
    ]=] package_expected @ONLY)
    expect_object("${document}" package_expected element)
  endforeach()
  string(CONFIGURE [=[
  {
    "spdxId": "urn:@external_dependency@#Package",
    "type": "software_Package",
    "name": "@external_dependency@",
    "creationInfo": "_:Build#CreationInfo",
    "software_packageVersion": "@version@",
    "originatedBy": [{
      "spdxId": "urn:@origin@#Organization",
      "type": "Organization",
      "name": "@origin@",
      "creationInfo": "_:Build#CreationInfo"
    }],
    "externalRef": [{
      "type": "ExternalRef",
      "externalRefType": "buildSystem",
      "locator": "cmake:find_package(@origin@)"
    }]
  }
  ]=] external_expected @ONLY)
  expect_object("${document}" external_expected element)
  set(bare_expected [=[
  {
    "spdxId": "urn:bare_library#Package",
    "type": "software_Package",
    "name": "bare_library",
    "creationInfo": "_:Build#CreationInfo"
  }
  ]=])
  expect_object("${document}" bare_expected element)

  string(CONFIGURE [=[
  {
    "spdxId": "urn:foo#StaticRelationship",
    "type": "Relationship",
    "creationInfo": "_:Build#CreationInfo",
    "description": "Required Build-Time Libraries",
    "from": "urn:foo#Package",
    "relationshipType": "dependsOn",
    "to": [
      "urn:@external_dependency@#Package",
      "urn:resolved_dependency#Package",
      "urn:@interface_dependency@#Package",
      "urn:compile_dependency#Package"
    ]
  }
  ]=] root_links_expected @ONLY)
  expect_object("${content}" root_links_expected @graph)
  foreach(type IN ITEMS STATIC SHARED)
    foreach(feature IN ITEMS plain whole group)
      set(package "${feature}_${type}")
      string(CONFIGURE [=[
      {
        "spdxId": "urn:@package@#StaticRelationship",
        "type": "Relationship",
        "creationInfo": "_:Build#CreationInfo",
        "description": "Required Build-Time Libraries",
        "from": "urn:@package@#Package",
        "relationshipType": "dependsOn",
        "to": ["urn:dep1#Package", "urn:dep2#Package"]
      }
      ]=] static_links_expected @ONLY)
      expect_object("${content}" static_links_expected @graph)
      string(CONFIGURE [=[
      {
        "spdxId": "urn:@package@#SharedRelationship",
        "type": "Relationship",
        "creationInfo": "_:Build#CreationInfo",
        "description": "Required Run-Time Libraries",
        "from": "urn:@package@#Package",
        "relationshipType": "dependsOn",
        "to": ["urn:runtime_dep#Package"]
      }
      ]=] shared_links_expected @ONLY)
      expect_object("${content}" shared_links_expected @graph)
    endforeach()
  endforeach()

  set(compile_links_expected [=[
  {
    "spdxId": "urn:compile_dependency#StaticRelationship",
    "type": "Relationship",
    "creationInfo": "_:Build#CreationInfo",
    "description": "Required Build-Time Libraries",
    "from": "urn:compile_dependency#Package",
    "relationshipType": "dependsOn",
    "to": ["urn:compile_leaf#Package"]
  }
  ]=])
  expect_object("${content}" compile_links_expected @graph)
  set(licensed_packages compile_dependency compile_leaf)
  set(licenses MIT Zlib)
  foreach(package license IN ZIP_LISTS licensed_packages licenses)
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
        "simplelicensing_licenseExpression": "@license@"
      }]
    }
    ]=] license_expected @ONLY)
    expect_object("${content}" license_expected @graph)
  endforeach()

  foreach(index RANGE 2 17)
    string(JSON id GET "${content}" @graph ${index} spdxId)
    if(id STREQUAL "urn:foo#StaticRelationship")
      expect_array("${content}" 4 @graph ${index} to)
    elseif(id MATCHES "^urn:(plain|whole|group)_(STATIC|SHARED)#StaticRelationship$")
      expect_array("${content}" 2 @graph ${index} to)
    else()
      expect_array("${content}" 1 @graph ${index} to)
    endif()
  endforeach()
endforeach()
