MACRO(KWSYS_PLATFORM_CXX_TEST var description invert)
  IF("${var}_COMPILED" MATCHES "^${var}_COMPILED$")
    MESSAGE(STATUS "${description}")
    TRY_COMPILE(${var}_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/kwsysPlatformCxxTests.cxx
      COMPILE_DEFINITIONS -DTEST_${var} ${KWSYS_PLATFORM_CXX_TEST_DEFINES}
      OUTPUT_VARIABLE OUTPUT)
    IF(${var}_COMPILED)
      FILE(APPEND 
        ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "${description} compiled with the following output:\n${OUTPUT}\n\n")
    ELSE(${var}_COMPILED)
      FILE(APPEND 
        ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "${description} failed to compile with the following output:\n${OUTPUT}\n\n")
    ENDIF(${var}_COMPILED)
    IF(${invert} MATCHES INVERT)
      IF(${var}_COMPILED)
        MESSAGE(STATUS "${description} - no")
      ELSE(${var}_COMPILED)
        MESSAGE(STATUS "${description} - yes")
      ENDIF(${var}_COMPILED)
    ELSE(${invert} MATCHES INVERT)
      IF(${var}_COMPILED)
        MESSAGE(STATUS "${description} - yes")
      ELSE(${var}_COMPILED)
        MESSAGE(STATUS "${description} - no")
      ENDIF(${var}_COMPILED)
    ENDIF(${invert} MATCHES INVERT)
  ENDIF("${var}_COMPILED" MATCHES "^${var}_COMPILED$")
  IF(${invert} MATCHES INVERT)
    IF(${var}_COMPILED)
      SET(${var} 0)
    ELSE(${var}_COMPILED)
      SET(${var} 1)
    ENDIF(${var}_COMPILED)
  ELSE(${invert} MATCHES INVERT)
    IF(${var}_COMPILED)
      SET(${var} 1)
    ELSE(${var}_COMPILED)
      SET(${var} 0)
    ENDIF(${var}_COMPILED)
  ENDIF(${invert} MATCHES INVERT)
ENDMACRO(KWSYS_PLATFORM_CXX_TEST)

MACRO(KWSYS_PLATFORM_CXX_TEST_RUN var description invert)
  IF("${var}" MATCHES "^${var}$")
    MESSAGE(STATUS "${description}")
    TRY_RUN(${var} ${var}_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/kwsysPlatformCxxTests.cxx
      COMPILE_DEFINITIONS -DTEST_${var} ${KWSYS_PLATFORM_CXX_TEST_DEFINES}
      OUTPUT_VARIABLE OUTPUT)

    # Note that ${var} will be a 0 return value on success.
    IF(${var}_COMPILED)
      IF(${var})
        FILE(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "${description} compiled but failed to run with the following output:\n${OUTPUT}\n\n")
      ELSE(${var})
        FILE(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
          "${description} compiled and ran with the following output:\n${OUTPUT}\n\n")
      ENDIF(${var})
    ELSE(${var}_COMPILED)
      FILE(APPEND
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "${description} failed to compile with the following output:\n${OUTPUT}\n\n")
    ENDIF(${var}_COMPILED)

    IF(${invert} MATCHES INVERT)
      IF(${var}_COMPILED)
        IF(${var})
          MESSAGE(STATUS "${description} - yes")
        ELSE(${var})
          MESSAGE(STATUS "${description} - no")
        ENDIF(${var})
      ELSE(${var}_COMPILED)
        MESSAGE(STATUS "${description} - failed to compile")
      ENDIF(${var}_COMPILED)
    ELSE(${invert} MATCHES INVERT)
      IF(${var}_COMPILED)
        IF(${var})
          MESSAGE(STATUS "${description} - no")
        ELSE(${var})
          MESSAGE(STATUS "${description} - yes")
        ENDIF(${var})
      ELSE(${var}_COMPILED)
        MESSAGE(STATUS "${description} - failed to compile")
      ENDIF(${var}_COMPILED)
    ENDIF(${invert} MATCHES INVERT)
  ENDIF("${var}" MATCHES "^${var}$")

  IF(${invert} MATCHES INVERT)
    IF(${var}_COMPILED)
      IF(${var})
        SET(${var} 1)
      ELSE(${var})
        SET(${var} 0)
      ENDIF(${var})
    ELSE(${var}_COMPILED)
      SET(${var} 1)
    ENDIF(${var}_COMPILED)
  ELSE(${invert} MATCHES INVERT)
    IF(${var}_COMPILED)
      IF(${var})
        SET(${var} 0)
      ELSE(${var})
        SET(${var} 1)
      ENDIF(${var})
    ELSE(${var}_COMPILED)
      SET(${var} 0)
    ENDIF(${var}_COMPILED)
  ENDIF(${invert} MATCHES INVERT)
ENDMACRO(KWSYS_PLATFORM_CXX_TEST_RUN)
