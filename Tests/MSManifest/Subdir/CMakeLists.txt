configure_file(test.manifest.in test.manifest)
add_executable(MSManifest main.c ${CMAKE_CURRENT_BINARY_DIR}/test.manifest)

if(MSVC AND NOT MSVC_VERSION LESS 1400)
  add_test(NAME MSManifest.Single COMMAND
    ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:MSManifest>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake)
  add_executable(MSManifestNone main.c)
  set_property(TARGET MSManifestNone PROPERTY LINK_FLAGS "/MANIFEST:NO")
elseif(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_test(NAME MSManifest.Single COMMAND
    ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:MSManifest>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake)
endif()
