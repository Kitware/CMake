#
# Check if the include file exists.
#
# CHECK_INCLUDE_FILE - macro which checks the include file exists.
# INCLUDE - name of include file
# VARIABLE - variable to return result
#

MACRO(CHECK_INCLUDE_FILE INCLUDE VARIABLE)
  SET(CHECK_INCLUDE_FILE_VAR ${INCLUDE})
  CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CheckIncludeFile.c.in
                 ${PROJECT_BINARY_DIR}/CheckIncludeFile.c IMMEDIATE)
  TRY_COMPILE(COMPILE_OK
             ${PROJECT_BINARY_DIR}
             ${PROJECT_BINARY_DIR}/CheckIncludeFile.c
             OUTPUT_VARIABLE OUTPUT)
  IF(COMPILE_OK)
    SET(${VARIABLE} ${COMPILE_OK})
  ELSE(COMPILE_OK)
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining if the include file ${INCLUDE} "
      "exists failed with the following output:\n"
      "${OUTPUT}\n")
  ENDIF(COMPILE_OK)
ENDMACRO(CHECK_INCLUDE_FILE)
