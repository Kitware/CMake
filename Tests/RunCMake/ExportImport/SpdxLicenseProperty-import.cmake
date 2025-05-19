find_package(foo REQUIRED CONFIG NO_DEFAULT_PATH)
get_property(SPDX_LICENSE TARGET foo PROPERTY SPDX_LICENSE)
if(NOT SPDX_LICENSE STREQUAL "BSD-3-Clause")
  message(FATAL_ERROR
    "Expected SPDX_LICENSE property to be 'BSD-3-Clause' but got '${SPDX_LICENSE}'")
endif()
