# Prepare environment and variables
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
if(WIN32)
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}\\pc-bletch")
else()
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/pc-bletch")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(BLETCH QUIET bletch)

if (NOT BLETCH_FOUND)
  message(FATAL_ERROR "Failed to find embedded package bletch via CMAKE_PREFIX_PATH")
endif ()

set(expected_value "item1;item2;item3;item with spaces")
pkg_get_variable(bletchvar1 bletch multiple_values1)
pkg_get_variable(bletchvar2 bletch multiple_values2)

string(FIND "${bletchvar1}" ";" IS_VARIABLE_A_LIST1)
string(FIND "${bletchvar2}" ";" IS_VARIABLE_A_LIST2)

if (IS_VARIABLE_A_LIST1 EQUAL -1 OR IS_VARIABLE_A_LIST2 EQUAL -1)
  message(FATAL_ERROR "Failed to fetch variable multiple_values from embedded package bletch as a list")
endif()

if (NOT (bletchvar1 STREQUAL expected_value AND bletchvar2 STREQUAL expected_value))
  message(NOTICE "multiple_values1=${bletchvar1} and expected_value=${expected_value}")
  message(NOTICE "multiple_values2=${bletchvar2} and expected_value=${expected_value}")
  message(FATAL_ERROR "Failed to fetch variable multiple_values from embedded package bletch with escaped spaces")
endif()
