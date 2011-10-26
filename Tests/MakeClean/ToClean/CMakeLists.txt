cmake_minimum_required(VERSION 2.6)
project(ToClean)

# Build a simple project.
add_executable(toclean toclean.cxx)

# List some build-time-generated files.
get_target_property(TOCLEAN_FILES toclean LOCATION)
set(TOCLEAN_FILES ${TOCLEAN_FILES}
  "${ToClean_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/toclean.dir/toclean.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")

# Create a file that must be registered for cleaning.
file(WRITE "${ToClean_BINARY_DIR}/Registered.txt"
  "File registered for cleaning.\n")
set_directory_properties(PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${ToClean_BINARY_DIR}/Registered.txt")
set(TOCLEAN_FILES ${TOCLEAN_FILES} "${ToClean_BINARY_DIR}/Registered.txt")

# Create a custom command whose output should be cleaned.
add_custom_command(OUTPUT ${ToClean_BINARY_DIR}/generated.txt
  DEPENDS ${ToClean_SOURCE_DIR}/toclean.cxx
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy ${ToClean_SOURCE_DIR}/toclean.cxx
               ${ToClean_BINARY_DIR}/generated.txt
  )
add_custom_target(generate ALL DEPENDS ${ToClean_BINARY_DIR}/generated.txt)
set(TOCLEAN_FILES ${TOCLEAN_FILES} "${ToClean_BINARY_DIR}/generated.txt")

# Configure a file listing these build-time-generated files.
configure_file(${ToClean_SOURCE_DIR}/ToCleanFiles.cmake.in
               ${ToClean_BINARY_DIR}/ToCleanFiles.cmake @ONLY IMMEDIATE)
