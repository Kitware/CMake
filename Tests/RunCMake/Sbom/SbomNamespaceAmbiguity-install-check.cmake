include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

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
  "name" : "foo_sbom:libb",
  "originatedBy" :
  [
    {
      "creationInfo" : "_:Build#CreationInfo",
      "name" : "foo_sbom",
      "spdxId" : "urn:foo_sbom#Organization",
      "type" : "Organization"
    }
  ],
  "spdxId" : "urn:foo_sbom:libb#Package",
  "type" : "software_Package"
}
]=])

expect_value("${BAR_CONTENT}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON BAR_SPDX_DOCUMENT GET "${BAR_CONTENT}" "@graph" "1")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_SPDX_DOCUMENT_EXPECTED)
expect_object("${BAR_SPDX_DOCUMENT}" BAR_LIBC "rootElement")
expect_object("${BAR_SPDX_DOCUMENT}" BAR_DEPENDENCY_FOO "element")
