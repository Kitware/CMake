include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(CREATION_INFO_EXPECTED [=[
{
  "@id": "_:Build#CreationInfo",
  "comment": "This SBOM was generated from the CMakeLists.txt File",
  "createdBy":
  [
    "https://gitlab.kitware.com/cmake/cmake"
  ],
  "specVersion": "3.0.1",
  "type": "CreationInfo"
}
]=])

set(SPDX_DOCUMENT_EXPECTED [=[
{
  "spdxId" : "urn:interface_targets#SPDXDocument",
  "name": "interface_targets",
  "profileConformance" :
  [
    "core",
    "software"
  ],
  "creationInfo" : "_:Build#CreationInfo",
  "type" : "SpdxDocument"
}
]=])

set(APPLICATION_EXPECTED [=[
{
  "spdxId" : "urn:interface#Package",
  "name" : "interface",
  "software_primaryPurpose" : "library",
  "type" : "software_Package"
}
]=])

set(DEPENDENCY_EXPECTED [=[
{
  "spdxId" : "urn:bar:bar#Package",
  "name" : "bar:bar",
  "originatedBy" :
  [
    {
      "name" : "bar",
      "type" : "Organization"
    }
  ],
  "software_packageVersion" : "1.3.5",
  "type" : "software_Package"
}
]=])

set(BAR_LINKED_LIBRARIES_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "description" : "Linked Libraries",
  "from" : "urn:interface#Package",
  "relationshipType" : "dependsOn",
  "spdxId" : "urn:Static#Relationship",
  "to" :
  [
    "urn:bar:bar#Package"
  ],
  "type" : "Relationship"
}
]=])

set(BAZ_LINKED_LIBRARIES_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "description" : "Required Build-Time Libraries",
  "from" : "urn:foo#Package",
  "relationshipType" : "dependsOn",
  "spdxId" : "urn:Shared#Relationship",
  "to" :
  [
    "urn:baz#Package"
  ],
  "type" : "Relationship"
}
]=])


function(check_config VAR_NAME LIBS)
  set(content "${${VAR_NAME}}")
  if("${content}" STREQUAL "")
    set(RunCMake_TEST_FAILED "Error: Variable '${VAR_NAME}' is empty or not defined." PARENT_SCOPE)
    return()
  endif()

  expect_value("${VAR_NAME}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
  string(JSON CREATION_INFO GET "${content}" "@graph" "0")
  expect_object("${CREATION_INFO}" CREATION_INFO_EXPECTED)

  string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "1")
  expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
  expect_object("${SPDX_DOCUMENT}" APPLICATION_EXPECTED "rootElement")
  expect_object("${SPDX_DOCUMENT}" DEPENDENCY_EXPECTED "element")
  string(JSON LINKED_LIBRARIES GET "${content}" "@graph" "2")
  expect_object("${LINKED_LIBRARIES}" ${LIBS})
endfunction()

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  check_config(BarConfig BAR_LINKED_LIBRARIES_EXPECTED)
  check_config(BazConfig BAZ_LINKED_LIBRARIES_EXPECTED)
else()
  check_config(BarConfig BAR_LINKED_LIBRARIES_EXPECTED)
endif()
