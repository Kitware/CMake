#
# Build the documentation
#
INCLUDE (${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

IF (BUILD_DOCUMENTATION)

  INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)

  #
  # Configure the script and the doxyfile, then add target
  #
  IF(NOT DOT_PATH)
    GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH)
  ENDIF(NOT DOT_PATH)

  CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/Utilities/Doxygen/doxyfile.in
    ${CMAKE_BINARY_DIR}/Utilities/Doxygen/doxyfile)

  CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/Utilities/Doxygen/doc_makeall.sh.in
    ${CMAKE_BINARY_DIR}/Utilities/Doxygen/doc_makeall.sh)

  ADD_CUSTOM_TARGET(DoxygenDoc
    ${BASH}
    ${CMAKE_BINARY_DIR}/Utilities/Doxygen/doc_makeall.sh)

ENDIF (BUILD_DOCUMENTATION)
