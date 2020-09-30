cmake_minimum_required (VERSION 3.18...3.19)

find_package (SWIG)
if (NOT SWIG_FOUND)
  message (FATAL_ERROR "Failed to find SWIG")
endif()

# clean-up SWIG variables
unset (SWIG_EXECUTABLE CACHE)
unset (SWIG_DIR CACHE)

## Specify a range including current SWIG version
string (REGEX MATCH "^([0-9]+)" upper_version "${SWIG_VERSION}")
math (EXPR upper_version "${upper_version} + 1")

find_package (SWIG 1.0...${upper_version}.0)
if (NOT SWIG_FOUND)
  message (FATAL_ERROR "Failed to find SWIG with version range 1.0...${upper_version}.0")
endif()

# clean-up SWIG variables
unset (SWIG_EXECUTABLE CACHE)
unset (SWIG_DIR CACHE)

## Specify a range excluding current SWIG version
set (range 1.0...<${SWIG_VERSION})
find_package (SWIG ${range})
if (SWIG_FOUND)
  message (FATAL_ERROR "Unexpectedly find SWIG with version range ${range}")
endif()
