configure_file(test_manifest1.in test_manifest1.manifest)
configure_file(test_manifest2.in test_manifest2.manifest)
configure_file(test_manifest3.in test_manifest3.manifest)
add_executable(MSMultipleManifest main.c
 ${CMAKE_CURRENT_BINARY_DIR}/test_manifest1.manifest
 ${CMAKE_CURRENT_BINARY_DIR}/test_manifest2.manifest
 ${CMAKE_CURRENT_BINARY_DIR}/test_manifest3.manifest)

if((MSVC AND NOT MSVC_VERSION LESS 1400) OR (WIN32 AND CMAKE_C_COMPILER_ID MATCHES "Clang") )
  add_test(NAME MSManifest.Multiple COMMAND
    ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:MSMultipleManifest>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake)
  if(MSVC AND NOT MSVC_VERSION LESS 1400)
    add_executable(MSMultipleManifestNonIncremental main.c
      ${CMAKE_CURRENT_BINARY_DIR}/test_manifest1.manifest
      ${CMAKE_CURRENT_BINARY_DIR}/test_manifest2.manifest
      ${CMAKE_CURRENT_BINARY_DIR}/test_manifest3.manifest)
    set_property(TARGET MSMultipleManifestNonIncremental PROPERTY LINK_FLAGS "/INCREMENTAL:NO")
    add_test(NAME MSManifest.Multiple.NonIncremental COMMAND
      ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:MSMultipleManifestNonIncremental>
      -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake)
  endif()
endif()
