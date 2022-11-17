set(CPACK_PACKAGE_NAME Good)
set(CPACK_GENERATOR "TGZ;TXZ")

include(CPack)

install(CODE [[
function(print_env name)
  if(DEFINED ENV{${name}})
    file(APPEND $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/env.txt "${name}=$ENV{${name}}\n")
  else()
    file(APPEND $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/env.txt "${name} not defined\n")
  endif()
endfunction()

file(REMOVE $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/env.txt)
print_env(TEST_ENV)
print_env(TEST_ENV_REF)
print_env(TEST_ENV_OVERRIDE)
print_env(TEST_ENV_OVERRIDE_REF)

file(APPEND $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/configs.txt "$<CONFIG>\n")
]])

file(WRITE "${CMAKE_BINARY_DIR}/CPackConfigAlt.cmake" [[include(${CMAKE_CURRENT_LIST_DIR}/CPackConfig.cmake)
set(CPACK_PACKAGE_FILE_NAME "config-file-alt")
]])

file(WRITE "${CMAKE_BINARY_DIR}/external_package.cmake" [[if(NOT CPACK_PACKAGE_VENDOR STREQUAL "some-vendor")
  message(FATAL_ERROR "Expected vendor to be \"some-vendor\" but it was \"${CPACK_PACKAGE_VENDOR}\"")
endif()
]])
