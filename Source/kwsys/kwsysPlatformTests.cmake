# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing#kwsys for details.

set(KWSYS_PLATFORM_TEST_FILE_C kwsysPlatformTestsC.c)
set(KWSYS_PLATFORM_TEST_FILE_CXX kwsysPlatformTestsCXX.cxx)

macro(KWSYS_PLATFORM_TEST lang var description invert)
  if(NOT DEFINED ${var}_COMPILED)
    message(STATUS "${description}")
    set(maybe_cxx_standard "")
    if(CMAKE_VERSION VERSION_LESS 3.8 AND CMAKE_CXX_STANDARD)
      set(maybe_cxx_standard "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")
    endif()
    try_compile(${var}_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/${KWSYS_PLATFORM_TEST_FILE_${lang}}
      COMPILE_DEFINITIONS -DTEST_${var} ${KWSYS_PLATFORM_TEST_DEFINES} ${KWSYS_PLATFORM_TEST_EXTRA_FLAGS}
      CMAKE_FLAGS "-DLINK_LIBRARIES:STRING=${KWSYS_PLATFORM_TEST_LINK_LIBRARIES}"
                  ${maybe_cxx_standard}
      OUTPUT_VARIABLE OUTPUT)
    if(${var}_COMPILED)
      file(APPEND
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "${description} compiled with the following output:\n${OUTPUT}\n\n")
    else()
      file(APPEND
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "${description} failed to compile with the following output:\n${OUTPUT}\n\n")
    endif()
    if(${invert} MATCHES INVERT)
      if(${var}_COMPILED)
        message(STATUS "${description} - no")
      else()
        message(STATUS "${description} - yes")
      endif()
    else()
      if(${var}_COMPILED)
        message(STATUS "${description} - yes")
      else()
        message(STATUS "${description} - no")
      endif()
    endif()
  endif()
  if(${invert} MATCHES INVERT)
    if(${var}_COMPILED)
      set(${var} 0)
    else()
      set(${var} 1)
    endif()
  else()
    if(${var}_COMPILED)
      set(${var} 1)
    else()
      set(${var} 0)
    endif()
  endif()
endmacro()

macro(KWSYS_PLATFORM_TEST_RUN lang var description invert)
  if(NOT DEFINED ${var})
    message(STATUS "${description}")
    try_run(${var} ${var}_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/${KWSYS_PLATFORM_TEST_FILE_${lang}}
      COMPILE_DEFINITIONS -DTEST_${var} ${KWSYS_PLATFORM_TEST_DEFINES} ${KWSYS_PLATFORM_TEST_EXTRA_FLAGS}
      OUTPUT_VARIABLE OUTPUT)

    # Note that ${var} will be a 0 return value on success.
    if(${var}_COMPILED)
      if(${var})
        file(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "${description} compiled but failed to run with the following output:\n${OUTPUT}\n\n")
      else()
        file(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
          "${description} compiled and ran with the following output:\n${OUTPUT}\n\n")
      endif()
    else()
      file(APPEND
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "${description} failed to compile with the following output:\n${OUTPUT}\n\n")
      set(${var} -1 CACHE INTERNAL "${description} failed to compile.")
    endif()

    if(${invert} MATCHES INVERT)
      if(${var}_COMPILED)
        if(${var})
          message(STATUS "${description} - yes")
        else()
          message(STATUS "${description} - no")
        endif()
      else()
        message(STATUS "${description} - failed to compile")
      endif()
    else()
      if(${var}_COMPILED)
        if(${var})
          message(STATUS "${description} - no")
        else()
          message(STATUS "${description} - yes")
        endif()
      else()
        message(STATUS "${description} - failed to compile")
      endif()
    endif()
  endif()

  if(${invert} MATCHES INVERT)
    if(${var}_COMPILED)
      if(${var})
        set(${var} 1)
      else()
        set(${var} 0)
      endif()
    else()
      set(${var} 1)
    endif()
  else()
    if(${var}_COMPILED)
      if(${var})
        set(${var} 0)
      else()
        set(${var} 1)
      endif()
    else()
      set(${var} 0)
    endif()
  endif()
endmacro()

macro(KWSYS_PLATFORM_C_TEST var description invert)
  set(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_C_TEST_DEFINES})
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_C_TEST_EXTRA_FLAGS})
  KWSYS_PLATFORM_TEST(C "${var}" "${description}" "${invert}")
  set(KWSYS_PLATFORM_TEST_DEFINES)
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
endmacro()

macro(KWSYS_PLATFORM_C_TEST_RUN var description invert)
  set(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_C_TEST_DEFINES})
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_C_TEST_EXTRA_FLAGS})
  KWSYS_PLATFORM_TEST_RUN(C "${var}" "${description}" "${invert}")
  set(KWSYS_PLATFORM_TEST_DEFINES)
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
endmacro()

macro(KWSYS_PLATFORM_CXX_TEST var description invert)
  set(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_CXX_TEST_DEFINES})
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_CXX_TEST_EXTRA_FLAGS})
  set(KWSYS_PLATFORM_TEST_LINK_LIBRARIES ${KWSYS_PLATFORM_CXX_TEST_LINK_LIBRARIES})
  KWSYS_PLATFORM_TEST(CXX "${var}" "${description}" "${invert}")
  set(KWSYS_PLATFORM_TEST_DEFINES)
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
  set(KWSYS_PLATFORM_TEST_LINK_LIBRARIES)
endmacro()

macro(KWSYS_PLATFORM_CXX_TEST_RUN var description invert)
  set(KWSYS_PLATFORM_TEST_DEFINES ${KWSYS_PLATFORM_CXX_TEST_DEFINES})
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS ${KWSYS_PLATFORM_CXX_TEST_EXTRA_FLAGS})
  KWSYS_PLATFORM_TEST_RUN(CXX "${var}" "${description}" "${invert}")
  set(KWSYS_PLATFORM_TEST_DEFINES)
  set(KWSYS_PLATFORM_TEST_EXTRA_FLAGS)
endmacro()

#-----------------------------------------------------------------------------
# KWSYS_PLATFORM_INFO_TEST(lang var description)
#
# Compile test named by ${var} and store INFO strings extracted from binary.
macro(KWSYS_PLATFORM_INFO_TEST lang var description)
  # We can implement this macro on CMake 2.6 and above.
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.6)
    set(${var} "")
  else()
    # Choose a location for the result binary.
    set(KWSYS_PLATFORM_INFO_FILE
      ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${var}.bin)

    # Compile the test binary.
    if(NOT EXISTS ${KWSYS_PLATFORM_INFO_FILE})
      message(STATUS "${description}")
      try_compile(${var}_COMPILED
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${KWSYS_PLATFORM_TEST_FILE_${lang}}
        COMPILE_DEFINITIONS -DTEST_${var}
          ${KWSYS_PLATFORM_${lang}_TEST_DEFINES}
          ${KWSYS_PLATFORM_${lang}_TEST_EXTRA_FLAGS}
        OUTPUT_VARIABLE OUTPUT
        COPY_FILE ${KWSYS_PLATFORM_INFO_FILE}
        )
      if(${var}_COMPILED)
        file(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
          "${description} compiled with the following output:\n${OUTPUT}\n\n")
      else()
        file(APPEND
          ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "${description} failed to compile with the following output:\n${OUTPUT}\n\n")
      endif()
      if(${var}_COMPILED)
        message(STATUS "${description} - compiled")
      else()
        message(STATUS "${description} - failed")
      endif()
    endif()

    # Parse info strings out of the compiled binary.
    if(${var}_COMPILED)
      file(STRINGS ${KWSYS_PLATFORM_INFO_FILE} ${var} REGEX "INFO:[A-Za-z0-9]+\\[[^]]*\\]")
    else()
      set(${var} "")
    endif()

    set(KWSYS_PLATFORM_INFO_FILE)
  endif()
endmacro()
