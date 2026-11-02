include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(FOO_SPDX_DOCUMENT_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "foo",
  "profileConformance" :
  [
    "core",
    "software"
  ],
  "spdxId" : "urn:foo#SPDXDocument",
  "type" : "SpdxDocument"
}
]=])

set(BAR_SPDX_DOCUMENT_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "bar",
  "profileConformance" :
  [
    "core",
    "software"
  ],
  "spdxId" : "urn:bar#SPDXDocument",
  "type" : "SpdxDocument"
}
]=])

set(FOO_LIBB [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "libb",
  "software_primaryPurpose" : "library",
  "spdxId" : "urn:libb#Package",
  "type" : "software_Package"
}
]=])

set(BAR_LIBC [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "libc",
  "software_primaryPurpose" : "library",
  "spdxId" : "urn:libc#Package",
  "type" : "software_Package"
}
]=])

set(BAR_DEPENDENCY_FOO [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "foo:libb",
  "originatedBy" :
  [
    {
      "creationInfo" : "_:Build#CreationInfo",
      "name" : "foo",
      "spdxId" : "urn:foo#Organization",
      "type" : "Organization"
    }
  ],
  "spdxId" : "urn:foo:libb#Package",
  "type" : "software_Package"
}
]=])

expect_value("${FOO_CONTENT}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
expect_array("${FOO_CONTENT}" 2 @graph)
string(JSON FOO_SPDX_DOCUMENT GET "${FOO_CONTENT}" @graph 1)
expect_object("${FOO_SPDX_DOCUMENT}" FOO_SPDX_DOCUMENT_EXPECTED)
expect_object("${FOO_SPDX_DOCUMENT}" FOO_LIBB rootElement)
expect_array("${FOO_SPDX_DOCUMENT}" 1 rootElement)
string(JSON unused ERROR_VARIABLE error TYPE "${FOO_SPDX_DOCUMENT}" element)
if(NOT error)
  expect_array("${FOO_SPDX_DOCUMENT}" 0 element)
endif()

expect_value("${BAR_CONTENT}" https://spdx.org/rdf/3.0.1/spdx-context.jsonld @context)
expect_array("${BAR_CONTENT}" 3 @graph)
string(JSON BAR_SPDX_DOCUMENT GET "${BAR_CONTENT}" @graph 1)
expect_object("${BAR_SPDX_DOCUMENT}" BAR_SPDX_DOCUMENT_EXPECTED)
expect_object("${BAR_SPDX_DOCUMENT}" BAR_LIBC rootElement)
expect_object("${BAR_SPDX_DOCUMENT}" BAR_DEPENDENCY_FOO element)
expect_array("${BAR_SPDX_DOCUMENT}" 1 rootElement)
expect_array("${BAR_SPDX_DOCUMENT}" 1 element)

set(BAR_LINKED_LIBRARIES_EXPECTED [=[
{
  "spdxId": "urn:libc#StaticRelationship",
  "type": "Relationship",
  "creationInfo": "_:Build#CreationInfo",
  "description": "Required Build-Time Libraries",
  "from": "urn:libc#Package",
  "relationshipType": "dependsOn",
  "to": ["urn:foo:libb#Package"]
}
]=])
expect_object("${BAR_CONTENT}" BAR_LINKED_LIBRARIES_EXPECTED @graph)
expect_array("${BAR_CONTENT}" 1 @graph 2 to)
