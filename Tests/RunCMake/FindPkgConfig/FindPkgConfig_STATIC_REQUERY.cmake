if(WIN32)
  set(PKG_CONFIG_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config-static-requery.bat")
else()
  set(PKG_CONFIG_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config-static-requery.sh")
endif()
find_package(PkgConfig REQUIRED)

pkg_check_modules(P REQUIRED good)
if(NOT P_STATIC_LIBRARIES STREQUAL "good;dep")
  message(FATAL_ERROR
    "Expected static libraries from first query, got: ${P_STATIC_LIBRARIES}")
endif()

pkg_check_modules(P REQUIRED bad)
if(NOT P_LIBRARIES STREQUAL "bad")
  message(FATAL_ERROR
    "Expected non-static libraries from second query, got: ${P_LIBRARIES}")
endif()
if(P_STATIC_LIBRARIES)
  message(FATAL_ERROR
    "Expected failed static query to clear stale libraries, got: ${P_STATIC_LIBRARIES}")
endif()
