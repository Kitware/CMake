set(CPACK_PACKAGE_NAME ConfigurePresetName)
include(CPack)

file(WRITE "${CMAKE_BINARY_DIR}/check-preset-names.cmake" [[
if(NOT "$ENV{TEST_PRESET_NAME}" STREQUAL "package")
  message(FATAL_ERROR
    "Expected preset name 'package', got '$ENV{TEST_PRESET_NAME}'")
endif()
if(NOT "$ENV{TEST_CONFIGURE_PRESET_NAME}" STREQUAL "configure")
  message(FATAL_ERROR
    "Expected configure preset name 'configure', got '$ENV{TEST_CONFIGURE_PRESET_NAME}'")
endif()
]])
