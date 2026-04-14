install(FILES CMakeLists.txt DESTINATION foo COMPONENT test)

set(_pfx multilene_defines_test)
set(CPACK_RPM_SPEC_MORE_DEFINE "%define ${_pfx}_satu 1")
list(APPEND CPACK_RPM_SPEC_MORE_DEFINE "%define ${_pfx}_dua 2")
list(APPEND CPACK_RPM_SPEC_MORE_DEFINE "%define ${_pfx}_tiga 3")

if(PACKAGING_TYPE STREQUAL "MONOLITHIC")
  set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_LIST_DIR}/custom.spec.in")
elseif(PACKAGING_TYPE STREQUAL "COMPONENT")
  install(FILES CMakeLists.txt DESTINATION bar COMPONENT test2)
  set(CPACK_RPM_TEST_USER_BINARY_SPECFILE
    "${CMAKE_CURRENT_LIST_DIR}/custom.spec.in")
endif()
