SUBDIRS(Doxygen)

IF(WIN32)
  SET(CMD ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/cmake.exe)
ELSE(WIN32)
  SET(CMD ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/cmake)
ENDIF(WIN32)

MAKE_DIRECTORY(${CMake_BINARY_DIR}/Docs)
ADD_CUSTOM_TARGET(documentation ALL)
ADD_CUSTOM_COMMAND(
  TARGET documentation
  SOURCE documentation
  DEPENDS ${CMake_BINARY_DIR}/Docs/Copyright.txt
  )
ADD_CUSTOM_COMMAND(
  TARGET documentation
  COMMAND ${CMD}
  ARGS --copyright ${CMake_BINARY_DIR}/Docs/Copyright.txt
       --help-full ${CMake_BINARY_DIR}/Docs/cmake.txt
       --help-html ${CMake_BINARY_DIR}/Docs/cmake.html
       --help-man ${CMake_BINARY_DIR}/Docs/cmake.1
  DEPENDS ${CMD}
  SOURCE ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
  OUTPUTS ${CMake_BINARY_DIR}/Docs/Copyright.txt
  )
ADD_DEPENDENCIES(documentation cmake)

INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES ${CMake_BINARY_DIR}/Docs/cmake.1)
INSTALL_FILES(${CMAKE_DOC_DIR} FILES
  ${CMake_BINARY_DIR}/Docs/Copyright.txt
  ${CMake_BINARY_DIR}/Docs/cmake.txt
  ${CMake_BINARY_DIR}/Docs/cmake.html
)
