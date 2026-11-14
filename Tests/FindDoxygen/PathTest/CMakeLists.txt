cmake_minimum_required(VERSION 3.10)
project(TestFindDoxygen VERSION 1.0 LANGUAGES NONE)

find_package(Doxygen REQUIRED)

# Simple tests to verify that absolute paths are correctly
# resolved by doxygen_add_docs() and forwarded to the underlying
# cmake -E make_directory call.

set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_HTML_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/FindDoxygenPathTest/output")

doxygen_add_docs(
  doxygenAbsPath
  main.cpp
  ALL
)

set(DOXYGEN_GENERATE_LATEX YES)
set(DOXYGEN_LATEX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/FindDoxygenPathTest/output with spaces")

doxygen_add_docs(
  doxygenAbsPathWithSpaces
  main.cpp
  ALL
)
