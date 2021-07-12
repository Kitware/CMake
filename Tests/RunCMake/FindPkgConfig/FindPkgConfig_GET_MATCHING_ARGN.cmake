if(WIN32)
    set(ENV{PKG_CONFIG} "\"${CMAKE_CURRENT_SOURCE_DIR}\\dummy-pkg-config.bat\" --static --print-errors")
else()
    set(ENV{PKG_CONFIG} "\"${CMAKE_CURRENT_SOURCE_DIR}/dummy-pkg-config.sh\" --static --print-errors")
endif()

find_package(PkgConfig REQUIRED)

if(NOT PKG_CONFIG_ARGN STREQUAL "--static;--print-errors")
  message(SEND_ERROR "PKG_CONFIG_ARGN has wrong value '${PKG_CONFIG_ARGN}'")
endif()

_pkgconfig_invoke("none" "prefix" "output" "")

if(NOT prefix_output STREQUAL "Received;--static;Received;--print-errors")
  message(SEND_ERROR "prefix_output has wrong value '${prefix_output}'")
endif()
