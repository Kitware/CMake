SUBDIRS(Doxygen KWStyle)

MAKE_DIRECTORY(${CMake_BINARY_DIR}/Docs)

# Add a documentation target.
SET(DOC_FILES "")

# Generate documentation for "cmake" executable.
GET_TARGET_PROPERTY(CMD cmake LOCATION)
ADD_CUSTOM_COMMAND(
  OUTPUT ${CMake_BINARY_DIR}/Docs/cmake.txt
  COMMAND ${CMD}
  ARGS --copyright ${CMake_BINARY_DIR}/Docs/Copyright.txt
       --help-full ${CMake_BINARY_DIR}/Docs/cmake.txt
       --help-html ${CMake_BINARY_DIR}/Docs/cmake.html
       --help-man ${CMake_BINARY_DIR}/Docs/cmake.1
  DEPENDS cmake
  MAIN_DEPENDENCY ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
  )
INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES ${CMake_BINARY_DIR}/Docs/cmake.1)
INSTALL_FILES(${CMAKE_DOC_DIR} FILES
  ${CMake_BINARY_DIR}/Docs/Copyright.txt
  ${CMake_BINARY_DIR}/Docs/cmake.txt
  ${CMake_BINARY_DIR}/Docs/cmake.html
  )
SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/cmake.txt)

# Generate documentation for "ctest" executable.
GET_TARGET_PROPERTY(CMD ctest LOCATION)
ADD_CUSTOM_COMMAND(
  OUTPUT ${CMake_BINARY_DIR}/Docs/ctest.txt
  COMMAND ${CMD}
  ARGS --help-full ${CMake_BINARY_DIR}/Docs/ctest.txt
       --help-html ${CMake_BINARY_DIR}/Docs/ctest.html
       --help-man ${CMake_BINARY_DIR}/Docs/ctest.1
  DEPENDS ctest
  MAIN_DEPENDENCY ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
  )
INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES ${CMake_BINARY_DIR}/Docs/ctest.1)
INSTALL_FILES(${CMAKE_DOC_DIR} FILES
  ${CMake_BINARY_DIR}/Docs/ctest.txt
  ${CMake_BINARY_DIR}/Docs/ctest.html
  )
SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/ctest.txt)

# Generate documentation for "cpack" executable.
GET_TARGET_PROPERTY(CMD cpack LOCATION)
ADD_CUSTOM_COMMAND(
  OUTPUT ${CMake_BINARY_DIR}/Docs/cpack.txt
  COMMAND ${CMD}
  ARGS --help-full ${CMake_BINARY_DIR}/Docs/cpack.txt
       --help-html ${CMake_BINARY_DIR}/Docs/cpack.html
       --help-man ${CMake_BINARY_DIR}/Docs/cpack.1
  DEPENDS cpack
  MAIN_DEPENDENCY ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
  )
INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES ${CMake_BINARY_DIR}/Docs/cpack.1)
INSTALL_FILES(${CMAKE_DOC_DIR} FILES
  ${CMake_BINARY_DIR}/Docs/cpack.txt
  ${CMake_BINARY_DIR}/Docs/cpack.html
  )
SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/cpack.txt)

# Generate documentation for "ccmake" executable.
IF(BUILD_CursesDialog)
  GET_TARGET_PROPERTY(CMD ccmake LOCATION)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMake_BINARY_DIR}/Docs/ccmake.txt
    COMMAND ${CMD}
    ARGS --help-full ${CMake_BINARY_DIR}/Docs/ccmake.txt
         --help-html ${CMake_BINARY_DIR}/Docs/ccmake.html
         --help-man ${CMake_BINARY_DIR}/Docs/ccmake.1
    DEPENDS ccmake
    MAIN_DEPENDENCY ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
    )
  INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES
    ${CMake_BINARY_DIR}/Docs/ccmake.1
    )
  INSTALL_FILES(${CMAKE_DOC_DIR} FILES
    ${CMake_BINARY_DIR}/Docs/ccmake.txt
    ${CMake_BINARY_DIR}/Docs/ccmake.html
    )
  SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/ccmake.txt)
ENDIF(BUILD_CursesDialog)

IF(BUILD_MFCDialog)
  GET_TARGET_PROPERTY(CMD CMakeSetup LOCATION)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt
    COMMAND ${CMD}
    ARGS --help-full ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt
    --help-html ${CMake_BINARY_DIR}/Docs/CMakeSetup.html
    --help-man ${CMake_BINARY_DIR}/Docs/CMakeSetup.1
    DEPENDS CMakeSetup
    MAIN_DEPENDENCY ${CMake_SOURCE_DIR}/Utilities/Doxygen/doxyfile.in
    )
  INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES
    ${CMake_BINARY_DIR}/Docs/CMakeSetup.1
    )
  INSTALL_FILES(${CMAKE_DOC_DIR} FILES
    ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt
    ${CMake_BINARY_DIR}/Docs/CMakeSetup.html
    )
  SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/CMakeSetup.txt)
ENDIF(BUILD_MFCDialog)

# Drive documentation generation.
ADD_CUSTOM_TARGET(documentation ALL
  DEPENDS ${DOC_FILES}
  )
