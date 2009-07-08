cmake_minimum_required (VERSION 2.6)
PROJECT(MakeClean)

# Build the to-clean project.
TRY_COMPILE(TOCLEAN_BUILT
  ${MakeClean_BINARY_DIR}/ToClean
  ${MakeClean_SOURCE_DIR}/ToClean
  ToClean
  OUTPUT_VARIABLE OUTPUT
  )
IF(TOCLEAN_BUILT)
  MESSAGE(
    "Building ToClean succeeded with the following output:\n"
    "[${OUTPUT}]"
    )
ELSE(TOCLEAN_BUILT)
  MESSAGE(FATAL_ERROR
    "Building ToClean failed with the following output:\n"
    "[${OUTPUT}]"
    )
ENDIF(TOCLEAN_BUILT)

# Get the set of files to check from the ToClean project.
INCLUDE(${MakeClean_BINARY_DIR}/ToClean/ToCleanFiles.cmake)

# Check for the existence of the files.
FOREACH(f ${TOCLEAN_FILES})
  IF(EXISTS "${f}")
  ELSE(EXISTS "${f}")
    MESSAGE(FATAL_ERROR "File \"${f}\" does not exist!")
  ENDIF(EXISTS "${f}")
ENDFOREACH(f)

# Configure an executable to check that all the files are missing.
SET(CHECK_FILES)
FOREACH(f ${TOCLEAN_FILES})
  SET(CHECK_FILES "${CHECK_FILES}      \"${f}\",\n")
ENDFOREACH(f)
CONFIGURE_FILE(${MakeClean_SOURCE_DIR}/check_clean.c.in
               ${MakeClean_BINARY_DIR}/check_clean.c @ONLY IMMEDIATE)
ADD_EXECUTABLE(check_clean ${MakeClean_BINARY_DIR}/check_clean.c)

# After the executable builds, clean the files.
ADD_CUSTOM_COMMAND(
  TARGET check_clean
  POST_BUILD
  COMMAND ${CMAKE_CTEST_COMMAND}
  ARGS --build-and-test
       ${MakeClean_SOURCE_DIR}/ToClean
       ${MakeClean_BINARY_DIR}/ToClean
       --build-generator ${CMAKE_GENERATOR}
       --build-project ToClean
       --build-makeprogram ${CMAKE_MAKE_PROGRAM}
       --build-noclean
       --build-target clean
  COMMENT "Clean the ToClean Project"
  )
