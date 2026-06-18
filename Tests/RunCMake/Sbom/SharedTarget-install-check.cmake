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
  "name" : "shared_targets",
  "profileConformance" :
  [
    "core",
    "software"
  ],
  "creationInfo" : "_:Build#CreationInfo",
  "spdxId" : "urn:shared_targets#SPDXDocument",
  "type" : "SpdxDocument"
}
]=])

set(APPLICATION_EXPECTED [=[
{
  "spdxId" : "urn:shared#Package",
  "name" : "shared",
  "software_primaryPurpose" : "library",
  "type" : "software_Package"
}
]=])

set(DEPENDENCY_EXPECTED [=[
{
  "spdxId" : "urn:foo:foo#Package",
  "name" : "foo:foo",
  "originatedBy" :
  [
    {
      "name" : "foo",
      "type" : "Organization"
    }
  ],
  "software_packageVersion" : "1.2.3",
  "type" : "software_Package"
}
]=])

set(BUILD_LINKED_LIBRARIES_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "description" : "Linked Libraries",
  "from" : "urn:shared#Package",
  "relationshipType" : "dependsOn",
  "spdxId" : "urn:Static#Relationship",
  "to" :
  [
    "urn:foo:foo#Package"
  ],
  "type" : "Relationship"
}
]=])

set(LICENSE_EXPECTED [=[
{
  "creationInfo" : "_:Build#CreationInfo",
  "from" : "urn:foo:foo#Package",
  "relationshipType" : "hasDeclaredLicense",
  "spdxId" : "urn:foo::foo#DeclaredLicenseRelationship",
  "to" :
  [
    {
      "creationInfo" : "_:Build#CreationInfo",
      "simplelicensing_licenseExpression" : "BSD-3-Clause",
      "spdxId" : "urn:foo::foo#LicenseExpression",
      "type" : "simplelicensing_LicenseExpression"
    }
  ],
  "type" : "Relationship"
}
]=])


expect_value("${content}" "https://spdx.org/rdf/3.0.1/spdx-context.jsonld" "@context")
string(JSON CREATION_INFO GET "${content}" "@graph" "0")
expect_object("${CREATION_INFO}" CREATION_INFO_EXPECTED)

string(JSON SPDX_DOCUMENT GET "${content}" "@graph" "1")
expect_object("${SPDX_DOCUMENT}" SPDX_DOCUMENT_EXPECTED)
expect_object("${SPDX_DOCUMENT}" APPLICATION_EXPECTED "rootElement")
expect_object("${SPDX_DOCUMENT}" DEPENDENCY_EXPECTED "element")
string(JSON LINKED_LIBRARIES GET "${content}" "@graph" "3")
expect_object("${LINKED_LIBRARIES}" BUILD_LINKED_LIBRARIES_EXPECTED)

# Check that default_license from the imported CPS package is reported
string(JSON LICENSE_RELATIONSHIP GET "${content}" "@graph" "2")
expect_object("${LICENSE_RELATIONSHIP}" LICENSE_EXPECTED)
