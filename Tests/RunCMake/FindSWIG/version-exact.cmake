cmake_minimum_required (VERSION 3.18...3.19)

find_package (SWIG)
if (NOT SWIG_FOUND)
  message (FATAL_ERROR "Failed to find SWIG")
endif()

# clean-up SWIG variables
unset (SWIG_EXECUTABLE CACHE)
unset (SWIG_DIR CACHE)

set (version ${SWIG_VERSION})

find_package (SWIG ${SWIG_VERSION} EXACT)
if (NOT SWIG_FOUND)
  message (FATAL_ERROR "Failed to find SWIG with version ${version} EXACT")
endif()
