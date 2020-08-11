file(STRINGS "${exe}" manifest_content1 REGEX "name=\"Kitware.CMake.MSMultipleManifest\"")
if(manifest_content1)
  message(STATUS "Expected manifest content found:\n ${manifest_content1}")
else()
  message(FATAL_ERROR "Expected manifest content not found in\n ${exe}")
endif()

# Verify Second Manifest Content is inside Executable.
file(STRINGS "${exe}" manifest_content2 REGEX "8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a")
if(manifest_content2)
  message(STATUS "Expected manifest content found:\n ${manifest_content2}")
else()
  message(FATAL_ERROR "Expected manifest content not found in\n ${exe}")
endif()

# Verify Third Manifest Content is inside Executable.
file(STRINGS "${exe}" manifest_content3 REGEX "<dpiAware>true</dpiAware>")
if(manifest_content3)
  message(STATUS "Expected manifest content found:\n ${manifest_content3}")
else()
  message(FATAL_ERROR "Expected manifest content not found in\n ${exe}")
endif()
