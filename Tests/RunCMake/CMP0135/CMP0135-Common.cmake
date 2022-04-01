include(ExternalProject)

set(stamp_dir "${CMAKE_CURRENT_BINARY_DIR}/stamps")

ExternalProject_Add(fake_ext_proj
  # We don't actually do a build, so we never try to download from this URL
  URL https://example.com/something.zip
  STAMP_DIR ${stamp_dir}
)

# Report whether the --touch option was added to the extraction script
set(extraction_script "${stamp_dir}/extract-fake_ext_proj.cmake")
file(STRINGS "${extraction_script}" results REGEX "--touch")
if("${results}" STREQUAL "")
  message(STATUS "Using timestamps from archive")
else()
  message(STATUS "Using extraction time for the timestamps")
endif()
