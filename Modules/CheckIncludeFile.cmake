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
                 ${PROJECT_BINARY_DIR}/CMakeTmp/CheckIncludeFile.c IMMEDIATE)
  TRY_COMPILE(${VARIABLE}
             ${PROJECT_BINARY_DIR}
             ${PROJECT_BINARY_DIR}/CMakeTmp/CheckIncludeFile.c
             OUTPUT_VARIABLE OUTPUT)
  IF(${VARIABLE})
    SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${INCLUDE}")
  ELSE(${VARIABLE})
    SET(${VARIABLE} "" CACHE INTERNAL "Have include ${INCLUDE}")
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining if the include file ${INCLUDE} "
      "exists failed with the following output:\n"
      "${OUTPUT}\n" APPEND)
  ENDIF(${VARIABLE})
ENDMACRO(CHECK_INCLUDE_FILE)
