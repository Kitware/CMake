# Disable formats not wanted in the package's documentation.
set(SPHINX_INFO OFF CACHE BOOL "")
set(SPHINX_SINGLEHTML OFF CACHE BOOL "")
set(SPHINX_TEXT OFF CACHE BOOL "")

# Set the destination directory for docs that packages expect.
set(CMAKE_DOC_DIR "doc/cmake" CACHE STRING "")

# Use a custom prefix to avoid conflicting with other builds.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install-doc" CACHE PATH "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_sphinx.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
