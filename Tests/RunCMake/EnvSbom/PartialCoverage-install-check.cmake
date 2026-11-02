set(failures "")

foreach(f explicit_root_sbom explicit_subdir_sbom)
  if(NOT EXISTS "${RunCMake_TEST_INSTALL_DIR}/${f}-Debug.spdx.json")
    list(APPEND failures "expected explicit SBOM ${f}-Debug.spdx.json to exist")
  endif()
endforeach()

foreach(f implicit_root implicit_subdir)
  if(NOT EXISTS "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/${f}/${f}-Debug.spdx.json")
    list(APPEND failures "expected autogen SBOM ${f}-Debug.spdx.json to exist")
  endif()
endforeach()

foreach(f explicit_root explicit_subdir)
  if(EXISTS "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/${f}/${f}-Debug.spdx.json")
    list(APPEND failures "autogen wrongly produced ${f}-Debug.spdx.json (should be suppressed by explicit install(SBOM))")
  endif()
endforeach()

if(failures)
  string(REPLACE ";" "\n  " msg "${failures}")
  set(RunCMake_TEST_FAILED "${msg}")
  return()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Assertions.cmake)

set(sbom_names explicit_root_sbom explicit_subdir_sbom implicit_root implicit_subdir)
set(target_names libA libC libB libD)
set(sbom_paths
  "${RunCMake_TEST_INSTALL_DIR}/explicit_root_sbom-Debug.spdx.json"
  "${RunCMake_TEST_INSTALL_DIR}/explicit_subdir_sbom-Debug.spdx.json"
  "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/implicit_root/implicit_root-Debug.spdx.json"
  "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/implicit_subdir/implicit_subdir-Debug.spdx.json"
)
foreach(sbom target path IN ZIP_LISTS sbom_names target_names sbom_paths)
  file(READ "${path}" content)
  string(CONFIGURE [=[
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
        "spdxId": "urn:@sbom@#SPDXDocument",
        "type": "SpdxDocument",
        "name": "@sbom@",
        "creationInfo": "_:Build#CreationInfo",
        "profileConformance": ["core", "software"],
        "rootElement": [{
          "spdxId": "urn:@target@#Package",
          "type": "software_Package",
          "name": "@target@",
          "creationInfo": "_:Build#CreationInfo",
          "software_primaryPurpose": "library"
        }]
      }
    ]
  }
  ]=] expected @ONLY)
  expect_object("${content}" expected)
  expect_array("${content}" 2 @graph)
  expect_array("${content}" 1 @graph 1 rootElement)
  string(JSON unused ERROR_VARIABLE error TYPE "${content}" @graph 1 element)
  if(NOT error)
    expect_array("${content}" 0 @graph 1 element)
  endif()
endforeach()
