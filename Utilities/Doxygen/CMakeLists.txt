#
# Build the documentation
#
INCLUDE (${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

IF (BUILD_DOCUMENTATION)

  #
  # Configure the script and the doxyfile, then add target
  #
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
