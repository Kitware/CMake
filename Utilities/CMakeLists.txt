SUBDIRS(Doxygen)

SET(CMD_DIR ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
IF(WIN32)
  SET(CMD_EXT ".exe")
ELSE(WIN32)
  SET(CMD_EXT "")
ENDIF(WIN32)

MAKE_DIRECTORY(${CMake_BINARY_DIR}/Docs)

# Add a documentation target.
ADD_CUSTOM_TARGET(documentation ALL)
SET(DOC_FILES "")

# Generate documentation for "cmake" executable.
SET(CMD ${CMD_DIR}/cmake${CMD_EXT})
ADD_CUSTOM_COMMAND(
  TARGET documentation
  COMMAND ${CMD}
  ARGS --copyright ${CMake_BINARY_DIR}/Docs/Copyright.txt
       --help-full ${CMake_BINARY_DIR}/Docs/cmake.txt
       --help-html ${CMake_BINARY_DIR}/Docs/cmake.html
       --help-man ${CMake_BINARY_DIR}/Docs/cmake.1
  DEPENDS ${CMD}
  SOURCE ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
  OUTPUTS ${CMake_BINARY_DIR}/Docs/cmake.txt
  )
ADD_DEPENDENCIES(documentation cmake)
INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES ${CMake_BINARY_DIR}/Docs/cmake.1)
INSTALL_FILES(${CMAKE_DOC_DIR} FILES
  ${CMake_BINARY_DIR}/Docs/Copyright.txt
  ${CMake_BINARY_DIR}/Docs/cmake.txt
  ${CMake_BINARY_DIR}/Docs/cmake.html
  )
SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/cmake.txt)

# Generate documentation for "ctest" executable.
SET(CMD ${CMD_DIR}/ctest${CMD_EXT})
ADD_CUSTOM_COMMAND(
  TARGET documentation
  COMMAND ${CMD}
  ARGS --help-full ${CMake_BINARY_DIR}/Docs/ctest.txt
       --help-html ${CMake_BINARY_DIR}/Docs/ctest.html
       --help-man ${CMake_BINARY_DIR}/Docs/ctest.1
  DEPENDS ${CMD}
  SOURCE ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
  OUTPUTS ${CMake_BINARY_DIR}/Docs/ctest.txt
  )
ADD_DEPENDENCIES(documentation ctest)
INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES ${CMake_BINARY_DIR}/Docs/ctest.1)
INSTALL_FILES(${CMAKE_DOC_DIR} FILES
  ${CMake_BINARY_DIR}/Docs/ctest.txt
  ${CMake_BINARY_DIR}/Docs/ctest.html
  )
SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/ctest.txt)

# Generate documentation for "ccmake" executable.
IF(UNIX)
  IF(CURSES_LIBRARY)
    SET(CMD ${CMD_DIR}/ccmake${CMD_EXT})
    ADD_CUSTOM_COMMAND(
      TARGET documentation
      COMMAND ${CMD}
      ARGS --help-full ${CMake_BINARY_DIR}/Docs/ccmake.txt
           --help-html ${CMake_BINARY_DIR}/Docs/ccmake.html
           --help-man ${CMake_BINARY_DIR}/Docs/ccmake.1
      DEPENDS ${CMD}
      SOURCE ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
      OUTPUTS ${CMake_BINARY_DIR}/Docs/ccmake.txt
      )
    ADD_DEPENDENCIES(documentation ccmake)
    INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES
      ${CMake_BINARY_DIR}/Docs/ccmake.1
      )
    INSTALL_FILES(${CMAKE_DOC_DIR} FILES
      ${CMake_BINARY_DIR}/Docs/ccmake.txt
      ${CMake_BINARY_DIR}/Docs/ccmake.html
      )
    SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/ccmake.txt)
  ENDIF(CURSES_LIBRARY)
ENDIF(UNIX)

IF(WIN32)
  IF(NOT UNIX)
    IF(NOT BORLAND)
      SET(CMD ${CMD_DIR}/CMakeSetup${CMD_EXT})
      ADD_CUSTOM_COMMAND(
        TARGET documentation
        COMMAND ${CMD}
        ARGS --help-full ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt
             --help-html ${CMake_BINARY_DIR}/Docs/CMakeSetup.html
             --help-man ${CMake_BINARY_DIR}/Docs/CMakeSetup.1
        DEPENDS ${CMD}
        SOURCE ${CMake_SOURCE_DIR}/Utilities/Doxygen/doxyfile.in
        OUTPUTS ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt
        )
      ADD_DEPENDENCIES(documentation CMakeSetup)
      INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES
        ${CMake_BINARY_DIR}/Docs/CMakeSetup.1
        )
      INSTALL_FILES(${CMAKE_DOC_DIR} FILES
        ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt
        ${CMake_BINARY_DIR}/Docs/CMakeSetup.html
        )
      SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt)
    ENDIF(NOT BORLAND)
  ENDIF(NOT UNIX)
ENDIF(WIN32)

# Drive documentation generation.
ADD_CUSTOM_COMMAND(
  TARGET documentation
  SOURCE documentation
  DEPENDS ${DOC_FILES}
  )
