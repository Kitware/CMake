#
# Check if the files can be included
#
# CHECK_INCLUDE_FILES - macro which checks the include file exists.
# INCLUDE - list of files to include
# VARIABLE - variable to return result
#

MACRO(CHECK_INCLUDE_FILES INCLUDE VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(CHECK_INCLUDE_FILES_CONTENT "/* */\n")
    STRING(ASCII 35 POUND)
    STRING(ASCII 40 41 PARENTS)
    FOREACH(FILE ${INCLUDE})
      SET(CHECK_INCLUDE_FILES_CONTENT
            "${CHECK_INCLUDE_FILES_CONTENT}${POUND}include <${FILE}>\n")
    ENDFOREACH(FILE)
    SET(CHECK_INCLUDE_FILES_CONTENT
        "${CHECK_INCLUDE_FILES_CONTENT}\n\nvoid main${PARENTS}{}\n")
    WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFiles.c 
               "${CHECK_INCLUDE_FILES_CONTENT}")
    
    MESSAGE(STATUS "Looking for set of ${INCLUDE}")
    TRY_COMPILE(${VARIABLE}
               ${CMAKE_BINARY_DIR}
               ${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFiles.c
               OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for set of ${INCLUDE} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${INCLUDE}")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for set of ${INCLUDE} - not found.")
      SET(${VARIABLE} "" CACHE INTERNAL "Have include ${INCLUDE}")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeError.log 
        "Determining if files ${INCLUDE} "
        "exist failed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_INCLUDE_FILES)
