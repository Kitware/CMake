#
# Check if the symbol exists in include files
#
# CHECK_SYMBOL_EXISTS - macro which checks the symbol exists in include files.
# SYMBOL - symbol
# FILES  - include files to check
# VARIABLE - variable to return result
#

MACRO(CHECK_SYMBOL_EXISTS SYMBOL FILES VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(CHECK_SYMBOL_EXISTS_CONTENT "/* */\n")
    SET(MACRO_CHECK_SYMBOL_EXISTS_FLAGS ${CMAKE_REQUIRED_FLAGS})
    STRING(ASCII 35 POUND)
    STRING(ASCII 40 OPEN_PARENT)
    STRING(ASCII 41 CLOSE_PARENT)
    SET(PARENTS "${OPEN_PARENT}${CLOSE_PARENT}")
    FOREACH(FILE ${FILES})
      SET(CHECK_SYMBOL_EXISTS_CONTENT
            "${CHECK_SYMBOL_EXISTS_CONTENT}${POUND}include <${FILE}>\n")
    ENDFOREACH(FILE)
    SET(CHECK_SYMBOL_EXISTS_CONTENT
        "${CHECK_SYMBOL_EXISTS_CONTENT}\n\nint main${PARENTS}{"
        "\n${POUND}ifndef ${SYMBOL}\nchar ${OPEN_PARENT}*f${CLOSE_PARENT} ${PARENTS} = ${SYMBOL};\n${POUND}endif\nreturn 0;\n}\n")
    WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeTmp/CheckSymbolExists.c 
               "${CHECK_SYMBOL_EXISTS_CONTENT}")
    
    MESSAGE(STATUS "Looking for ${SYMBOL} in ${FILES}")
    TRY_COMPILE(${VARIABLE}
               ${CMAKE_BINARY_DIR}
               ${CMAKE_BINARY_DIR}/CMakeTmp/CheckSymbolExists.c
               CMAKE_FLAGS 
                -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_SYMBOL_EXISTS_FLAGS}
               OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for ${SYMBOL} in ${FILES} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have symbol ${SYMBOL}")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${SYMBOL} in ${FILES} - not found.")
      SET(${VARIABLE} "" CACHE INTERNAL "Have symbol ${SYMBOL}")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeError.log 
        "Determining if the ${SYMBOL} "
        "exist failed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_SYMBOL_EXISTS)
