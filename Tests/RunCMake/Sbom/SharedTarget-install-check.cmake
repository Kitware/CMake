include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(CREATION_INFO [=[
{
  "comment" : "This SBOM was generated from the CMakeLists.txt File",
  "createdUsing" :
  [
    {
      "@id" : "CMake#Agent",
      "name" : "CMake",
      "type" : "Tool"
    }
  ],
  "type" : "CreationInfo"
}
]=])

set(ELEMENTS [=[
[
  {
    "@id" : "foo:foo#Package",
    "name" : "foo:foo",
    "type" : "software_Package"
  }
]
]=])

set(SPDX_DOCUMENT [=[
{
  "@id" : "shared_targets#SPDXDocument",
  "name": "shared_targets",
  "profileConformance": ["core", "software"],
  "type": "SpdxDocument"
}
]=])

set(SHARED [=[
{
  "@id" : "shared#Package",
  "externalRef" :
  [
    {
      "comment" : "Build System used for this target",
      "externalRefType" : "buildSystem",
      "locator" : "CMake#Agent",
      "type" : "ExternalRef"
    }
  ],
  "name" : "shared",
  "primaryPurpose" : "LIBRARY",
  "type" : "software_Package"
}
]=])

set(DEPENDENCY [=[
{
  "@id" : "foo:foo#Package",
  "name" : "foo:foo",
  "originatedBy" :
  [
    {
      "name" : "foo",
      "type" : "Organization"
    }
  ],
  "packageVersion" : "1.2.3",
  "type" : "software_Package"
}
]=])

set(BUILD_LINKED_LIBRARIES [=[
{
  "description" : "Linked Libraries",
  "from" : "shared#Package",
  "relationshipType" : "DEPENDS_ON",
  "to" :
  [
    "foo:foo#Package"
  ],
  "type" : "Relationship"
}
]=])


expect_value("${content}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "0")
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT)
expect_object("${SPDX_DOCUMENT}" CREATION_INFO "creationInfo")
expect_object("${SPDX_DOCUMENT}" SHARED "rootElement" "0")
expect_object("${SPDX_DOCUMENT}" DEPENDENCY "element" "0")

string(JSON LINKED_LIBRARIES GET "${content}" "@graph" "1")
expect_object("${LINKED_LIBRARIES}" BUILD_LINKED_LIBRARIES)
