if(NOT version)
  message(FATAL_ERROR "Pass -Dversion=")
endif()

# Name of the archive and its top-level directory.
set(archive_name "cmake-${version}-tutorial-source")

# Base directory for CMake Documentation.
set(help_dir "${CMAKE_CURRENT_LIST_DIR}/../../Help")
cmake_path(ABSOLUTE_PATH help_dir NORMALIZE)

# Collect the non-documentation part of the tutorial directory.
file(COPY "${help_dir}/guide/tutorial/"
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}"
  NO_SOURCE_PERMISSIONS
  PATTERN *.rst EXCLUDE
  PATTERN source.txt EXCLUDE
  )
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}/README.txt" [[
This directory contains source code examples for the CMake Tutorial.
Each step has its own subdirectory containing code that may be used as a
starting point. The tutorial examples are progressive so that each step
provides the complete solution for the previous step.
]])

# Create an archive containing the tutorial source examples.
file(MAKE_DIRECTORY "${help_dir}/_generated")
file(ARCHIVE_CREATE
  OUTPUT "${help_dir}/_generated/${archive_name}.zip"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}"
  FORMAT zip
  )

# Write a reStructuredText snippet included from the tutorial index.
file(WRITE "${help_dir}/guide/tutorial/source.txt" "
.. |tutorial_source| replace::
  The tutorial source code examples are available in
  :download:`this archive </_generated/${archive_name}.zip>`.
")

# Remove temporary directory.
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/${archive_name}")
