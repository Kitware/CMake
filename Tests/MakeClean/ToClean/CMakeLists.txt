PROJECT(ToClean)

# Build a simple project.
ADD_EXECUTABLE(toclean toclean.cxx)

# List some build-time-generated files.
GET_TARGET_PROPERTY(TOCLEAN_FILES toclean LOCATION)
SET(TOCLEAN_FILES ${TOCLEAN_FILES}
  "${ToClean_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/toclean.dir/toclean${CMAKE_CXX_OUTPUT_EXTENSION}")

# Create a file that must be registered for cleaning.
FILE(WRITE "${ToClean_BINARY_DIR}/Registered.txt"
  "File registered for cleaning.\n")
SET_DIRECTORY_PROPERTIES(PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${ToClean_BINARY_DIR}/Registered.txt")
SET(TOCLEAN_FILES ${TOCLEAN_FILES} "${ToClean_BINARY_DIR}/Registered.txt")

# Create a custom command whose output should be cleaned.
ADD_CUSTOM_COMMAND(OUTPUT ${ToClean_BINARY_DIR}/generated.txt
  DEPENDS ${ToClean_SOURCE_DIR}/toclean.cxx
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy ${ToClean_SOURCE_DIR}/toclean.cxx
               ${ToClean_BINARY_DIR}/generated.txt
  )
ADD_CUSTOM_TARGET(generate ALL DEPENDS ${ToClean_BINARY_DIR}/generated.txt)
SET(TOCLEAN_FILES ${TOCLEAN_FILES} "${ToClean_BINARY_DIR}/generated.txt")

# Configure a file listing these build-time-generated files.
CONFIGURE_FILE(${ToClean_SOURCE_DIR}/ToCleanFiles.cmake.in
               ${ToClean_BINARY_DIR}/ToCleanFiles.cmake @ONLY IMMEDIATE)
