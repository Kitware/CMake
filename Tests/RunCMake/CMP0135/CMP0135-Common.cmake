#==============================================================================
# ExternalProject
#==============================================================================
set(stamp_dir "${CMAKE_CURRENT_BINARY_DIR}/stamps-ep")
include(ExternalProject)
ExternalProject_Add(fake_ext_proj
  # We don't actually do a build, so we never try to download from this URL
  URL https://example.com/something.zip
  STAMP_DIR ${stamp_dir}
)

# Report whether the --touch option was added to the extraction script
set(extraction_script "${stamp_dir}/extract-fake_ext_proj.cmake")
file(STRINGS "${extraction_script}" results REGEX "--touch")
if("${results}" STREQUAL "")
  message(STATUS "ExternalProject: Using timestamps from archive")
else()
  message(STATUS "ExternalProject: Using extraction time for the timestamps")
endif()

#==============================================================================
# FetchContent
#==============================================================================
set(stamp_dir "${CMAKE_CURRENT_BINARY_DIR}/stamps-fc")
set(archive_file ${CMAKE_CURRENT_BINARY_DIR}/test_archive.7z)
file(ARCHIVE_CREATE
  OUTPUT ${archive_file}
  PATHS ${CMAKE_CURRENT_LIST_DIR}
  FORMAT 7zip
)
include(FetchContent)
FetchContent_Declare(fake_fc_proj
  URL file://${archive_file}
  STAMP_DIR ${stamp_dir}
)
FetchContent_MakeAvailable(fake_fc_proj)

# Report whether the --touch option was added to the extraction script
set(extraction_script "${stamp_dir}/extract-fake_fc_proj-populate.cmake")
file(STRINGS "${extraction_script}" results REGEX "--touch")
if("${results}" STREQUAL "")
  message(STATUS "FetchContent: Using timestamps from archive")
else()
  message(STATUS "FetchContent: Using extraction time for the timestamps")
endif()
