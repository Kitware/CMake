#=============================================================================
# KWSys - Kitware System Library
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
SET(KWSYS_PLATFORM_TEST_FILE_C kwsysPlatformTestsC.c)
SET(KWSYS_PLATFORM_TEST_FILE_CXX kwsysPlatformTestsCXX.cxx)

MACRO(KWSYS_PLATFORM_TEST lang var description invert)
  IF("${var}_COMPILED" MATCHES "^${var}_COMPILED$")
    MESSAGE(STATUS "${description}")
    TRY_COMPILE(${var}_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/${KWSYS_PLATFORM_TEST_FILE_${lang}}
      COMPILE_DEFINITIONS -DTEST_${var} ${KWSYS_PLATFORM_TEST_DEFINES} ${KWSYS_PLATFORM_TEST_EXTRA_FLAGS}
      OUTPUT_VARIABLE OUTPUT)
    IF(${var}_COMPILED)
      FILE(APPEND
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "${description} compiled with the following output:\n${OUTPUT}\n\n")
    ELSE(${var}_COMPILED)
      FILE(APPEND
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
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
ENDMACRO(KWSYS_PLATFORM_TEST)

MACRO(KWSYS_PLATFORM_TEST_RUN lang var description invert)
  IF("${var}" MATCHES "^${var}$")
    MESSAGE(STATUS "${description}")
    TRY_RUN(${var} ${var}_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/${KWSYS_PLATFORM_TEST_FILE_${lang}}
      COMPILE_DEFINITIONS -DTEST_${var} ${KWSYS_PLATFORM_TEST_DEFINES} ${KWSYS_PLATFORM_TEST_EXTRA_FLAGS}
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
      SET(${var} -1 CACHE INTERNAL "${description} failed to compile.")
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
ENDMACRO(KWSYS_PLATFORM_TEST_RUN)

MACRO(KWSYS_PLATFORM_C_TEST var description invert)
  SET(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_C_TEST_DEFINES})
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_C_TEST_EXTRA_FLAGS})
  KWSYS_PLATFORM_TEST(C "${var}" "${description}" "${invert}")
  SET(KWSYS_PLATFORM_TEST_DEFINES)
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
ENDMACRO(KWSYS_PLATFORM_C_TEST)

MACRO(KWSYS_PLATFORM_C_TEST_RUN var description invert)
  SET(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_C_TEST_DEFINES})
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_C_TEST_EXTRA_FLAGS})
  KWSYS_PLATFORM_TEST_RUN(C "${var}" "${description}" "${invert}")
  SET(KWSYS_PLATFORM_TEST_DEFINES)
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
ENDMACRO(KWSYS_PLATFORM_C_TEST_RUN)

MACRO(KWSYS_PLATFORM_CXX_TEST var description invert)
  SET(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_CXX_TEST_DEFINES})
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_CXX_TEST_EXTRA_FLAGS})
  KWSYS_PLATFORM_TEST(CXX "${var}" "${description}" "${invert}")
  SET(KWSYS_PLATFORM_TEST_DEFINES)
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
ENDMACRO(KWSYS_PLATFORM_CXX_TEST)

MACRO(KWSYS_PLATFORM_CXX_TEST_RUN var description invert)
  SET(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_CXX_TEST_DEFINES})
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_CXX_TEST_EXTRA_FLAGS})
  KWSYS_PLATFORM_TEST_RUN(CXX "${var}" "${description}" "${invert}")
  SET(KWSYS_PLATFORM_TEST_DEFINES)
  SET(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
ENDMACRO(KWSYS_PLATFORM_CXX_TEST_RUN)

#-----------------------------------------------------------------------------
# KWSYS_PLATFORM_INFO_TEST(lang var description)
#
# Compile test named by ${var} and store INFO strings extracted from binary.
MACRO(KWSYS_PLATFORM_INFO_TEST lang var description)
  # We can implement this macro on CMake 2.6 and above.
  IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.6)
    SET(${var} "")
  ELSE()
    # Choose a location for the result binary.
    SET(KWSYS_PLATFORM_INFO_FILE
      ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${var}.bin)

    # Compile the test binary.
    IF(NOT EXISTS ${KWSYS_PLATFORM_INFO_FILE})
      MESSAGE(STATUS "${description}")
      TRY_COMPILE(${var}_COMPILED
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${KWSYS_PLATFORM_TEST_FILE_${lang}}
        COMPILE_DEFINITIONS -DTEST_${var}
          ${KWSYS_PLATFORM_${lang}_TEST_DEFINES}
          ${KWSYS_PLATFORM_${lang}_TEST_EXTRA_FLAGS}
        OUTPUT_VARIABLE OUTPUT
        COPY_FILE ${KWSYS_PLATFORM_INFO_FILE}
        )
      IF(${var}_COMPILED)
        FILE(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
          "${description} compiled with the following output:\n${OUTPUT}\n\n")
      ELSE()
        FILE(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "${description} failed to compile with the following output:\n${OUTPUT}\n\n")
      ENDIF()
      IF(${var}_COMPILED)
        MESSAGE(STATUS "${description} - compiled")
      ELSE()
        MESSAGE(STATUS "${description} - failed")
      ENDIF()
    ENDIF()

    # Parse info strings out of the compiled binary.
    IF(${var}_COMPILED)
      FILE(STRINGS ${KWSYS_PLATFORM_INFO_FILE} ${var} REGEX "INFO:[A-Za-z0-9]+\\[[^]]*\\]")
    ELSE()
      SET(${var} "")
    ENDIF()

    SET(KWSYS_PLATFORM_INFO_FILE)
  ENDIF()
ENDMACRO()
