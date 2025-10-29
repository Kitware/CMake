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


set(BAR_SPDX_DOCUMENT [=[
{
  "@id" : "bar#SPDXDocument",
  "name": "bar",
  "profileConformance": ["core", "software"],
  "type": "SpdxDocument"
}
]=])

set(FOO_SPDX_DOCUMENT [=[
{
  "@id" : "foo#SPDXDocument",
  "name": "foo",
  "profileConformance": ["core", "software"],
  "type": "SpdxDocument"
}
]=])


set(FOO_LIBB [=[
{
  "@id" : "libb#Package",
  "externalRef" :
  [
    {
      "comment" : "Build System used for this target",
      "externalRefType" : "buildSystem",
      "locator" : "CMake#Agent",
      "type" : "ExternalRef"
    }
  ],
  "name" : "libb",
  "primaryPurpose" : "LIBRARY",
  "type" : "software_Package"
}
]=])

set(BAR_LIBC [=[
{
  "@id" : "libc#Package",
  "externalRef" :
  [
    {
      "comment" : "Build System used for this target",
      "externalRefType" : "buildSystem",
      "locator" : "CMake#Agent",
      "type" : "ExternalRef"
    }
  ],
  "name" : "libc",
  "primaryPurpose" : "LIBRARY",
  "type" : "software_Package"
}
]=])

set(BAR_LIBD [=[
{
  "@id" : "libd#Package",
  "externalRef" :
  [
    {
      "comment" : "Build System used for this target",
      "externalRefType" : "buildSystem",
      "locator" : "CMake#Agent",
      "type" : "ExternalRef"
    }
  ],
  "name" : "libd",
  "primaryPurpose" : "LIBRARY",
  "type" : "software_Package"
}
]=])

set(BAR_DEPENDENCY_TEST [=[
{
  "@id" : "test:liba#Package",
  "name" : "test:liba",
  "originatedBy" :
  [
    {
      "name" : "test",
      "type" : "Organization"
    }
  ],
  "type" : "software_Package"
}
]=])


set(BAR_DEPENDENCY_FOO [=[
{
  "@id" : "foo:libb#Package",
  "name" : "foo:libb",
  "originatedBy" :
  [
    {
      "name" : "foo",
      "type" : "Organization"
    }
  ],
  "type" : "software_Package"
}
]=])


expect_value("${FOO_CONTENT}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON FOO_SPDX_DOCUMENT GET "${FOO_CONTENT}" "@graph" "0")
expect_object("${FOO_SPDX_DOCUMENT}" FOO_SPDX_DOCUMENT)
expect_object("${FOO_SPDX_DOCUMENT}" CREATION_INFO "creationInfo")
expect_object("${FOO_SPDX_DOCUMENT}" FOO_LIBB "rootElement" "0")

expect_value("${BAR_CONTENT}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON BAR_SPDX_DOCUMENT GET "${BAR_CONTENT}" "@graph" "0")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_SPDX_DOCUMENT)
expect_object("${BAR_SPDX_DOCUMENT}" CREATION_INFO "creationInfo")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_LIBC "rootElement" "0")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_LIBD "rootElement" "1")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_DEPENDENCY_TEST "element" "0")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_DEPENDENCY_FOO "element" "1")
