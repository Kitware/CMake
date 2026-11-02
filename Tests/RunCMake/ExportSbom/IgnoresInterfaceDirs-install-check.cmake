include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Assertions.cmake)
file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/interface_targets/interface_targets-Debug.spdx.json" content)
set(expected [=[
{
  "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
  "@graph": [
    {
      "@id": "_:Build#CreationInfo",
      "type": "CreationInfo",
      "comment": "This SBOM was generated from the CMakeLists.txt File",
      "createdBy": ["https://gitlab.kitware.com/cmake/cmake"],
      "specVersion": "3.0.1"
    },
    {
      "spdxId": "urn:interface_targets#SPDXDocument",
      "type": "SpdxDocument",
      "name": "interface_targets",
      "creationInfo": "_:Build#CreationInfo",
      "profileConformance": ["core", "software"],
      "rootElement": [{
        "spdxId": "urn:interface#Package",
        "type": "software_Package",
        "name": "interface",
        "creationInfo": "_:Build#CreationInfo",
        "software_primaryPurpose": "library"
      }]
    }
  ]
}
]=])
expect_object("${content}" expected)
expect_array("${content}" 2 @graph)
expect_array("${content}" 1 @graph 1 rootElement)
string(JSON unused ERROR_VARIABLE error TYPE "${content}" @graph 1 element)
if(NOT error)
  set(RunCMake_TEST_FAILED "Include directories introduced unexpected SBOM dependencies")
endif()
