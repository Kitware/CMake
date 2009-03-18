# Verify the current working directory.
if(NOT EXISTS CMakeLists.txt)
  message(FATAL_ERROR "File does not exist:\n  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
endif()
if(NOT EXISTS tutorial.cxx)
  message(FATAL_ERROR "File does not exist:\n  ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.cxx")
endif()

# Check if the patch is already applied.
file(STRINGS CMakeLists.txt prop_line REGEX "^set_property")
if(prop_line)
  message(STATUS "Patch already applied!")
  return()
endif()

# Apply the patch.
file(APPEND CMakeLists.txt "
# Patch by ExternalProject test:
set_property(TARGET Tutorial PROPERTY OUTPUT_NAME EP-Tutorial)
")
message(STATUS "Patched ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
