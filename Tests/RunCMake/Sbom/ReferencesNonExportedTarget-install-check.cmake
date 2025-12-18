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

set(SPDX_DOCUMENT [=[
{
  "@id": "_:dog#SPDXDocument",
  "name": "dog",
  "profileConformance": ["core", "software"],
  "type": "SpdxDocument"
}
]=])

set(CANINE [=[
{
  "@id" : "canine#Package",
  "externalRef" :
  [
    {
      "comment" : "Build System used for this target",
      "externalRefType" : "buildSystem",
      "locator" : "CMake#Agent",
      "type" : "ExternalRef"
    }
  ],
  "name" : "canine",
  "primaryPurpose" : "LIBRARY",
  "type" : "software_Package"
}
]=])

set(DEPENDENCY [=[
{
  "@id" : "mammal#Package",
  "name" : "mammal",
  "type" : "software_Package"
}
]=])

set(BUILD_LINK_LIBRARIES [=[
{
  "description" : "Required Build-Time Libraries",
  "from" : "canine#Package",
  "relationshipType" : "DEPENDS_ON",
  "to" :
  [
    "mammal#Package"
  ],
  "type" : "Relationship"
}
]=])

expect_value("${content}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "0")
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT)
expect_object("${SPDX_DOCUMENT}" CREATION_INFO "creationInfo")
expect_object("${SPDX_DOCUMENT}" CANINE "rootElement" "0")
expect_object("${SPDX_DOCUMENT}" DEPENDENCY "element" "0")

string(JSON LINKED_LIBRARIES GET "${content}" "@graph" "1")
expect_object("${LINKED_LIBRARIES}" BUILD_LINK_LIBRARIES)
