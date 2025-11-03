# Keep formatting here consistent with bootstrap script expectations.
if(BEOS)
  set(CMAKE_BIN_DIR_DEFAULT "bin") # HAIKU
  set(CMAKE_DATA_DIR_DEFAULT "share/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # HAIKU
  set(CMAKE_DOC_DIR_DEFAULT "documentation/doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # HAIKU
  set(CMAKE_INFO_DIR_DEFAULT "documentation/info") # HAIKU
  set(CMAKE_MAN_DIR_DEFAULT "documentation/man") # HAIKU
  set(CMAKE_XDGDATA_DIR_DEFAULT "share") # HAIKU
elseif(CYGWIN)
  set(CMAKE_BIN_DIR_DEFAULT "bin") # CYGWIN
  set(CMAKE_DATA_DIR_DEFAULT "share/cmake-${CMake_VERSION}") # CYGWIN
  set(CMAKE_DOC_DIR_DEFAULT "share/doc/cmake-${CMake_VERSION}") # CYGWIN
  set(CMAKE_INFO_DIR_DEFAULT "share/info") # CYGWIN
  set(CMAKE_MAN_DIR_DEFAULT "share/man") # CYGWIN
  set(CMAKE_XDGDATA_DIR_DEFAULT "share") # CYGWIN
else()
  set(CMAKE_BIN_DIR_DEFAULT "bin") # OTHER
  set(CMAKE_DATA_DIR_DEFAULT "share/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # OTHER
  set(CMAKE_DOC_DIR_DEFAULT "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}") # OTHER
  set(CMAKE_INFO_DIR_DEFAULT "info") # OTHER
  set(CMAKE_MAN_DIR_DEFAULT "man") # OTHER
  set(CMAKE_XDGDATA_DIR_DEFAULT "share") # OTHER
endif()

set(CMAKE_BIN_DIR_DESC "bin")
set(CMAKE_DATA_DIR_DESC "data")
set(CMAKE_DOC_DIR_DESC "docs")
set(CMAKE_INFO_DIR_DESC "Info manual")
set(CMAKE_MAN_DIR_DESC "man pages")
set(CMAKE_XDGDATA_DIR_DESC "XDG specific files")

set(CMake_INSTALL_INFIX "" CACHE STRING "")
set_property(CACHE CMake_INSTALL_INFIX PROPERTY HELPSTRING
  "Intermediate installation path (empty by default)"
  )
mark_as_advanced(CMake_INSTALL_INFIX)

if(APPLE AND BUILD_QtDialog)
  set(CMake_INSTALL_APP_DIR "CMake.app/Contents")
  set(CMake_INSTALL_APP_DIR_SLASH "${CMake_INSTALL_APP_DIR}/")
else()
  set(CMake_INSTALL_APP_DIR ".")
  set(CMake_INSTALL_APP_DIR_SLASH "")
endif()

foreach(v
    BIN
    DATA
    DOC
    INFO
    MAN
    XDGDATA
    )
  # Populate the cache with empty values so we know when the user sets them.
  set(CMAKE_${v}_DIR "" CACHE STRING "")
  set_property(CACHE CMAKE_${v}_DIR PROPERTY HELPSTRING
    "Location under install prefix for ${CMAKE_${v}_DIR_DESC} (default \"${CMAKE_${v}_DIR_DEFAULT}\")"
    )
  set_property(CACHE CMAKE_${v}_DIR PROPERTY ADVANCED 1)

  # Use the default when the user did not set this variable.
  if(NOT CMAKE_${v}_DIR)
    set(CMAKE_${v}_DIR "${CMake_INSTALL_INFIX}${CMAKE_${v}_DIR_DEFAULT}")
  endif()
  # Remove leading slash to treat as relative to install prefix.
  string(REGEX REPLACE "^/" "" CMAKE_${v}_DIR "${CMAKE_${v}_DIR}")

  # Install under a base path within the prefix.
  set(CMake_INSTALL_${v}_DIR "${CMake_INSTALL_APP_DIR_SLASH}${CMAKE_${v}_DIR}")
endforeach()
