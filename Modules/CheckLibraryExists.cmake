#
# Check if the function exists.
#
# CHECK_LIBRARY_EXISTS - macro which checks if the function exists
# FUNCTION - the name of the function
# VARIABLE - variable to store the result
#

MACRO(CHECK_LIBRARY_EXISTS LIBRARY FUNCTION LOCATION VARIABLE)
  SET(CHECK_LIBRARY_EXISTS_LIBRARY ${LIBRARY})
  SET(CHECK_LIBRARY_EXISTS_FUNCTION ${FUNCTION})
  SET(CHECK_LIBRARY_EXISTS_LOCATION ${LOCATION})
  SET(CHECK_LIBRARY_EXISTS_VARIABLE ${VARIABLE})
  SET(CHECK_LIBRARY_EXISTS_SOURCE ${CMAKE_ROOT}/Modules/CheckFunctionExists.c)
  CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CheckLibraryExists.lists.in
                 ${PROJECT_BINARY_DIR}/CMakeTmp/CheckLibraryExists/CMakeLists.txt
                 IMMEDIATE)
  TRY_COMPILE(${VARIABLE}
             ${PROJECT_BINARY_DIR}/CMakeTmp/CheckLibraryExists
             ${PROJECT_BINARY_DIR}/CMakeTmp/CheckLibraryExists
             CHECK_LIBRARY_EXISTS OUTPUT_VARIABLE OUTPUT)
  IF(NOT ${VARIABLE})
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining if the function ${FUNCTION} exists failed with the following output:\n"
      "${OUTPUT}\n")
  ENDIF(NOT ${VARIABLE})
ENDMACRO(CHECK_LIBRARY_EXISTS)
