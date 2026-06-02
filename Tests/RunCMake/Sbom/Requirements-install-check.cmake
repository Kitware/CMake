include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(CREATION_INFO [=[
{
  "@id" : "_:Build#CreationInfo",
  "comment" : "This SBOM was generated from the CMakeLists.txt File",
  "created" : "2026-02-17T10:34:30Z",
  "createdBy" :
  [
    "https://gitlab.kitware.com/cmake/cmake"
  ],
  "specVersion" : "3.0.1",
  "type" : "CreationInfo"
}
]=])


set(BAR_SPDX_DOCUMENT_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "bar_sbom",
  "profileConformance" :
  [
    "core",
    "software"
  ],
  "spdxId" : "urn:bar_sbom#SPDXDocument",
  "type" : "SpdxDocument"
}
]=])

set(FOO_SPDX_DOCUMENT_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "foo_sbom",
  "profileConformance" :
  [
    "core",
    "software"
  ],
  "spdxId" : "urn:foo_sbom#SPDXDocument",
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

set(BAR_LIBB [=[
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

set(BAR_LIBD [=[
{
  "spdxId" : "urn:libd#Package",
  "name" : "libd",
  "software_primaryPurpose" : "library",
  "type" : "software_Package"
}
]=])

set(BAR_DEPENDENCY_TEST [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "name" : "test:liba",
  "originatedBy" :
  [
    {
      "creationInfo" : "_:Build#CreationInfo",
      "name" : "test",
      "spdxId" : "urn:test#Organization",
      "type" : "Organization"
    }
  ],
  "spdxId" : "urn:test:liba#Package",
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

expect_value("${FOO_CONTENT}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON FOO_CREATION_INFO GET "${FOO_CONTENT}" "@graph" "0")
string(JSON FOO_SPDX_DOCUMENT GET "${FOO_CONTENT}" "@graph" "1")
expect_object("${FOO_SPDX_DOCUMENT}" FOO_SPDX_DOCUMENT_EXPECTED)
expect_object("${FOO_SPDX_DOCUMENT}" FOO_LIBB "rootElement")

expect_value("${BAR_CONTENT}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON BAR_CREATION_INFO GET "${BAR_CONTENT}" "@graph" "0")
string(JSON BAR_SPDX_DOCUMENT GET "${BAR_CONTENT}" "@graph" "1")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_SPDX_DOCUMENT_EXPECTED)
expect_object("${BAR_SPDX_DOCUMENT}" BAR_LIBC "rootElement")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_LIBD "rootElement")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_DEPENDENCY_TEST "element")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_DEPENDENCY_FOO "element")
