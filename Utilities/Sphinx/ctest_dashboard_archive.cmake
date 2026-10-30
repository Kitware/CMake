if(NOT version)
  message(FATAL_ERROR "Pass -Dversion=")
endif()

# Name of the archive and its top-level directory.
set(archive_name "cmake-${version}-ctest-dashboard-client-source")

# Base directory for CMake Documentation.
set(help_dir "${CMAKE_CURRENT_LIST_DIR}/../../Help")
cmake_path(ABSOLUTE_PATH help_dir NORMALIZE)

# Collect the non-documentation part of the guide directory.
file(COPY "${help_dir}/guide/ctest-dashboard-client/"
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}"
  NO_SOURCE_PERMISSIONS
  PATTERN *.rst EXCLUDE
  REGEX "/guide/ctest-dashboard-client/include$" EXCLUDE
  )
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}/README.txt" [[
This directory contains the examples for the CTest Dashboard Client Guide.
Each step has its own subdirectory containing a complete dashboard script.
The Project subdirectory contains the example project that the scripts
build, test, and submit to CDash.
]])

# Create an archive containing the guide's examples.
file(MAKE_DIRECTORY "${help_dir}/_generated")
file(ARCHIVE_CREATE
  OUTPUT "${help_dir}/_generated/${archive_name}.zip"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}"
  FORMAT zip
  )

# Write a reStructuredText snippet included from the guide index.
file(WRITE "${help_dir}/guide/ctest-dashboard-client/include/source.rst" [[
.. |ctest_dashboard_source| replace::
  The scripts and example projects are available in
  :download:`this archive </_generated/]] "${archive_name}" [[.zip>`.
]])

# Remove temporary directory.
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}")
