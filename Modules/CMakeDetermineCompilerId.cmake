
#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Function to compile a source file to identify the compiler.  This is
# used internally by CMake and should not be included by user code.
# If successful, sets CMAKE_<lang>_COMPILER_ID and CMAKE_<lang>_PLATFORM_ID

function(CMAKE_DETERMINE_COMPILER_ID lang flagvar src)
  # Make sure the compiler arguments are clean.
  string(STRIP "${CMAKE_${lang}_COMPILER_ARG1}" CMAKE_${lang}_COMPILER_ID_ARG1)
  string(REGEX REPLACE " +" ";" CMAKE_${lang}_COMPILER_ID_ARG1 "${CMAKE_${lang}_COMPILER_ID_ARG1}")

  # Make sure user-specified compiler flags are used.
  if(CMAKE_${lang}_FLAGS)
    set(CMAKE_${lang}_COMPILER_ID_FLAGS ${CMAKE_${lang}_FLAGS})
  else(CMAKE_${lang}_FLAGS)
    set(CMAKE_${lang}_COMPILER_ID_FLAGS $ENV{${flagvar}})
  endif(CMAKE_${lang}_FLAGS)
  string(REGEX REPLACE " " ";" CMAKE_${lang}_COMPILER_ID_FLAGS_LIST "${CMAKE_${lang}_COMPILER_ID_FLAGS}")

  # Compute the directory in which to run the test.
  set(CMAKE_${lang}_COMPILER_ID_DIR ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CompilerId${lang})

  # Try building with no extra flags and then try each set
  # of helper flags.  Stop when the compiler is identified.
  foreach(flags "" ${CMAKE_${lang}_COMPILER_ID_TEST_FLAGS})
    if(NOT CMAKE_${lang}_COMPILER_ID)
      CMAKE_DETERMINE_COMPILER_ID_BUILD("${lang}" "${flags}" "${src}")
      foreach(file ${COMPILER_${lang}_PRODUCED_FILES})
        CMAKE_DETERMINE_COMPILER_ID_CHECK("${lang}" "${CMAKE_${lang}_COMPILER_ID_DIR}/${file}" "${src}")
      endforeach(file)
    endif(NOT CMAKE_${lang}_COMPILER_ID)
  endforeach(flags)

  # If the compiler is still unknown, try to query its vendor.
  if(NOT CMAKE_${lang}_COMPILER_ID)
    CMAKE_DETERMINE_COMPILER_ID_VENDOR(${lang})
  endif()

  # if the format is unknown after all files have been checked, put "Unknown" in the cache
  if(NOT CMAKE_EXECUTABLE_FORMAT)
    set(CMAKE_EXECUTABLE_FORMAT "Unknown" CACHE INTERNAL "Executable file format")
  endif(NOT CMAKE_EXECUTABLE_FORMAT)

  # Display the final identification result.
  if(CMAKE_${lang}_COMPILER_ID)
    if(CMAKE_${lang}_COMPILER_VERSION)
      set(_version " ${CMAKE_${lang}_COMPILER_VERSION}")
    else()
      set(_version "")
    endif()
    message(STATUS "The ${lang} compiler identification is "
      "${CMAKE_${lang}_COMPILER_ID}${_version}")
  else(CMAKE_${lang}_COMPILER_ID)
    message(STATUS "The ${lang} compiler identification is unknown")
  endif(CMAKE_${lang}_COMPILER_ID)

  set(CMAKE_${lang}_COMPILER_ID "${CMAKE_${lang}_COMPILER_ID}" PARENT_SCOPE)
  set(CMAKE_${lang}_PLATFORM_ID "${CMAKE_${lang}_PLATFORM_ID}" PARENT_SCOPE)
  set(MSVC_${lang}_ARCHITECTURE_ID "${MSVC_${lang}_ARCHITECTURE_ID}"
    PARENT_SCOPE)
  set(CMAKE_${lang}_COMPILER_VERSION "${CMAKE_${lang}_COMPILER_VERSION}" PARENT_SCOPE)
endfunction(CMAKE_DETERMINE_COMPILER_ID)

#-----------------------------------------------------------------------------
# Function to write the compiler id source file.
function(CMAKE_DETERMINE_COMPILER_ID_WRITE lang src)
  file(READ ${CMAKE_ROOT}/Modules/${src}.in ID_CONTENT_IN)
  string(CONFIGURE "${ID_CONTENT_IN}" ID_CONTENT_OUT @ONLY)
  file(WRITE ${CMAKE_${lang}_COMPILER_ID_DIR}/${src} "${ID_CONTENT_OUT}")
endfunction(CMAKE_DETERMINE_COMPILER_ID_WRITE)

#-----------------------------------------------------------------------------
# Function to build the compiler id source file and look for output
# files.
function(CMAKE_DETERMINE_COMPILER_ID_BUILD lang testflags src)
  # Create a clean working directory.
  file(REMOVE_RECURSE ${CMAKE_${lang}_COMPILER_ID_DIR})
  file(MAKE_DIRECTORY ${CMAKE_${lang}_COMPILER_ID_DIR})
  CMAKE_DETERMINE_COMPILER_ID_WRITE("${lang}" "${src}")

  # Construct a description of this test case.
  set(COMPILER_DESCRIPTION
    "Compiler: ${CMAKE_${lang}_COMPILER} ${CMAKE_${lang}_COMPILER_ID_ARG1}
Build flags: ${CMAKE_${lang}_COMPILER_ID_FLAGS_LIST}
Id flags: ${testflags}
")

  # Compile the compiler identification source.
  if(COMMAND EXECUTE_PROCESS)
    execute_process(
      COMMAND ${CMAKE_${lang}_COMPILER}
              ${CMAKE_${lang}_COMPILER_ID_ARG1}
              ${CMAKE_${lang}_COMPILER_ID_FLAGS_LIST}
              ${testflags}
              "${src}"
      WORKING_DIRECTORY ${CMAKE_${lang}_COMPILER_ID_DIR}
      OUTPUT_VARIABLE CMAKE_${lang}_COMPILER_ID_OUTPUT
      ERROR_VARIABLE CMAKE_${lang}_COMPILER_ID_OUTPUT
      RESULT_VARIABLE CMAKE_${lang}_COMPILER_ID_RESULT
      )
  else(COMMAND EXECUTE_PROCESS)
    exec_program(
      ${CMAKE_${lang}_COMPILER} ${CMAKE_${lang}_COMPILER_ID_DIR}
      ARGS ${CMAKE_${lang}_COMPILER_ID_ARG1}
           ${CMAKE_${lang}_COMPILER_ID_FLAGS_LIST}
           ${testflags}
           \"${src}\"
      OUTPUT_VARIABLE CMAKE_${lang}_COMPILER_ID_OUTPUT
      RETURN_VALUE CMAKE_${lang}_COMPILER_ID_RESULT
      )
  endif(COMMAND EXECUTE_PROCESS)

  # Check the result of compilation.
  if(CMAKE_${lang}_COMPILER_ID_RESULT)
    # Compilation failed.
    set(MSG
      "Compiling the ${lang} compiler identification source file \"${src}\" failed.
${COMPILER_DESCRIPTION}
The output was:
${CMAKE_${lang}_COMPILER_ID_RESULT}
${CMAKE_${lang}_COMPILER_ID_OUTPUT}

")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log "${MSG}")
    #if(NOT CMAKE_${lang}_COMPILER_ID_ALLOW_FAIL)
    #  message(FATAL_ERROR "${MSG}")
    #endif(NOT CMAKE_${lang}_COMPILER_ID_ALLOW_FAIL)

    # No output files should be inspected.
    set(COMPILER_${lang}_PRODUCED_FILES)
  else(CMAKE_${lang}_COMPILER_ID_RESULT)
    # Compilation succeeded.
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Compiling the ${lang} compiler identification source file \"${src}\" succeeded.
${COMPILER_DESCRIPTION}
The output was:
${CMAKE_${lang}_COMPILER_ID_RESULT}
${CMAKE_${lang}_COMPILER_ID_OUTPUT}

")

    # Find the executable produced by the compiler, try all files in the
    # binary dir.
    file(GLOB COMPILER_${lang}_PRODUCED_FILES
      RELATIVE ${CMAKE_${lang}_COMPILER_ID_DIR}
      ${CMAKE_${lang}_COMPILER_ID_DIR}/*)
    list(REMOVE_ITEM COMPILER_${lang}_PRODUCED_FILES "${src}")
    foreach(file ${COMPILER_${lang}_PRODUCED_FILES})
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Compilation of the ${lang} compiler identification source \""
        "${src}\" produced \"${file}\"\n\n")
    endforeach(file)

    if(NOT COMPILER_${lang}_PRODUCED_FILES)
      # No executable was found.
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Compilation of the ${lang} compiler identification source \""
        "${src}\" did not produce an executable in \""
        "${CMAKE_${lang}_COMPILER_ID_DIR}\".\n\n")
    endif(NOT COMPILER_${lang}_PRODUCED_FILES)
  endif(CMAKE_${lang}_COMPILER_ID_RESULT)

  # Return the files produced by the compilation.
  set(COMPILER_${lang}_PRODUCED_FILES "${COMPILER_${lang}_PRODUCED_FILES}" PARENT_SCOPE)
endfunction(CMAKE_DETERMINE_COMPILER_ID_BUILD lang testflags src)

#-----------------------------------------------------------------------------
# Function to extract the compiler id from an executable.
function(CMAKE_DETERMINE_COMPILER_ID_CHECK lang file)
  # Look for a compiler id if not yet known.
  if(NOT CMAKE_${lang}_COMPILER_ID)
    # Read the compiler identification string from the executable file.
    set(COMPILER_ID)
    set(COMPILER_VERSION)
    set(PLATFORM_ID)
    file(STRINGS ${file}
      CMAKE_${lang}_COMPILER_ID_STRINGS LIMIT_COUNT 4 REGEX "INFO:")
    set(HAVE_COMPILER_TWICE 0)
    foreach(info ${CMAKE_${lang}_COMPILER_ID_STRINGS})
      if("${info}" MATCHES ".*INFO:compiler\\[([^]\"]*)\\].*")
        if(COMPILER_ID)
          set(COMPILER_ID_TWICE 1)
        endif(COMPILER_ID)
        string(REGEX REPLACE ".*INFO:compiler\\[([^]]*)\\].*" "\\1"
          COMPILER_ID "${info}")
      endif("${info}" MATCHES ".*INFO:compiler\\[([^]\"]*)\\].*")
      if("${info}" MATCHES ".*INFO:platform\\[([^]\"]*)\\].*")
        string(REGEX REPLACE ".*INFO:platform\\[([^]]*)\\].*" "\\1"
          PLATFORM_ID "${info}")
      endif("${info}" MATCHES ".*INFO:platform\\[([^]\"]*)\\].*")
      if("${info}" MATCHES ".*INFO:arch\\[([^]\"]*)\\].*")
        string(REGEX REPLACE ".*INFO:arch\\[([^]]*)\\].*" "\\1"
          ARCHITECTURE_ID "${info}")
      endif("${info}" MATCHES ".*INFO:arch\\[([^]\"]*)\\].*")
      if("${info}" MATCHES ".*INFO:compiler_version\\[([^]\"]*)\\].*")
        string(REGEX REPLACE ".*INFO:compiler_version\\[([^]]*)\\].*" "\\1" COMPILER_VERSION "${info}")
        string(REGEX REPLACE "^0+([0-9])" "\\1" COMPILER_VERSION "${COMPILER_VERSION}")
        string(REGEX REPLACE "\\.0+([0-9])" ".\\1" COMPILER_VERSION "${COMPILER_VERSION}")
      endif("${info}" MATCHES ".*INFO:compiler_version\\[([^]\"]*)\\].*")
    endforeach(info)

    # Check if a valid compiler and platform were found.
    if(COMPILER_ID AND NOT COMPILER_ID_TWICE)
      set(CMAKE_${lang}_COMPILER_ID "${COMPILER_ID}")
      set(CMAKE_${lang}_PLATFORM_ID "${PLATFORM_ID}")
      set(MSVC_${lang}_ARCHITECTURE_ID "${ARCHITECTURE_ID}")
      set(CMAKE_${lang}_COMPILER_VERSION "${COMPILER_VERSION}")
    endif(COMPILER_ID AND NOT COMPILER_ID_TWICE)

    # Check the compiler identification string.
    if(CMAKE_${lang}_COMPILER_ID)
      # The compiler identification was found.
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "The ${lang} compiler identification is ${CMAKE_${lang}_COMPILER_ID}, found in \""
        "${file}\"\n\n")
    else(CMAKE_${lang}_COMPILER_ID)
      # The compiler identification could not be found.
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "The ${lang} compiler identification could not be found in \""
        "${file}\"\n\n")
    endif(CMAKE_${lang}_COMPILER_ID)
  endif(NOT CMAKE_${lang}_COMPILER_ID)

  # try to figure out the executable format: ELF, COFF, Mach-O
  if(NOT CMAKE_EXECUTABLE_FORMAT)
    file(READ ${file} CMAKE_EXECUTABLE_MAGIC LIMIT 4 HEX)

    # ELF files start with 0x7f"ELF"
    if("${CMAKE_EXECUTABLE_MAGIC}" STREQUAL "7f454c46")
      set(CMAKE_EXECUTABLE_FORMAT "ELF" CACHE INTERNAL "Executable file format")
    endif("${CMAKE_EXECUTABLE_MAGIC}" STREQUAL "7f454c46")

#    # COFF (.exe) files start with "MZ"
#    if("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "4d5a....")
#      set(CMAKE_EXECUTABLE_FORMAT "COFF" CACHE STRING "Executable file format")
#    endif("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "4d5a....")
#
#    # Mach-O files start with CAFEBABE or FEEDFACE, according to http://radio.weblogs.com/0100490/2003/01/28.html
#    if("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "cafebabe")
#      set(CMAKE_EXECUTABLE_FORMAT "MACHO" CACHE STRING "Executable file format")
#    endif("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "cafebabe")
#    if("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "feedface")
#      set(CMAKE_EXECUTABLE_FORMAT "MACHO" CACHE STRING "Executable file format")
#    endif("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "feedface")

  endif(NOT CMAKE_EXECUTABLE_FORMAT)
  if(NOT DEFINED CMAKE_EXECUTABLE_FORMAT)
    set(CMAKE_EXECUTABLE_FORMAT)
  endif()
  # Return the information extracted.
  set(CMAKE_${lang}_COMPILER_ID "${CMAKE_${lang}_COMPILER_ID}" PARENT_SCOPE)
  set(CMAKE_${lang}_PLATFORM_ID "${CMAKE_${lang}_PLATFORM_ID}" PARENT_SCOPE)
  set(MSVC_${lang}_ARCHITECTURE_ID "${MSVC_${lang}_ARCHITECTURE_ID}"
    PARENT_SCOPE)
  set(CMAKE_${lang}_COMPILER_VERSION "${CMAKE_${lang}_COMPILER_VERSION}" PARENT_SCOPE)
  set(CMAKE_EXECUTABLE_FORMAT "${CMAKE_EXECUTABLE_FORMAT}" PARENT_SCOPE)
endfunction(CMAKE_DETERMINE_COMPILER_ID_CHECK lang)

#-----------------------------------------------------------------------------
# Function to query the compiler vendor.
# This uses a table with entries of the form
#   list(APPEND CMAKE_${lang}_COMPILER_ID_VENDORS ${vendor})
#   set(CMAKE_${lang}_COMPILER_ID_VENDOR_FLAGS_${vendor} -some-vendor-flag)
#   set(CMAKE_${lang}_COMPILER_ID_VENDOR_REGEX_${vendor} "Some Vendor Output")
# We try running the compiler with the flag for each vendor and
# matching its regular expression in the output.
function(CMAKE_DETERMINE_COMPILER_ID_VENDOR lang)

  if(NOT CMAKE_${lang}_COMPILER_ID_DIR)
    # We get here when this function is called not from within CMAKE_DETERMINE_COMPILER_ID()
    # This is done e.g. for detecting the compiler ID for assemblers.
    # Compute the directory in which to run the test and Create a clean working directory.
    set(CMAKE_${lang}_COMPILER_ID_DIR ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CompilerId${lang})
    file(REMOVE_RECURSE ${CMAKE_${lang}_COMPILER_ID_DIR})
    file(MAKE_DIRECTORY ${CMAKE_${lang}_COMPILER_ID_DIR})
  endif(NOT CMAKE_${lang}_COMPILER_ID_DIR)


  foreach(vendor ${CMAKE_${lang}_COMPILER_ID_VENDORS})
    set(flags ${CMAKE_${lang}_COMPILER_ID_VENDOR_FLAGS_${vendor}})
    set(regex ${CMAKE_${lang}_COMPILER_ID_VENDOR_REGEX_${vendor}})
    execute_process(
      COMMAND ${CMAKE_${lang}_COMPILER}
      ${CMAKE_${lang}_COMPILER_ID_ARG1}
      ${CMAKE_${lang}_COMPILER_ID_FLAGS_LIST}
      ${flags}
      WORKING_DIRECTORY ${CMAKE_${lang}_COMPILER_ID_DIR}
      OUTPUT_VARIABLE output ERROR_VARIABLE output
      RESULT_VARIABLE result
      TIMEOUT 10
      )

    if("${output}" MATCHES "${regex}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Checking whether the ${lang} compiler is ${vendor} using \"${flags}\" "
        "matched \"${regex}\":\n${output}")
      set(CMAKE_${lang}_COMPILER_ID "${vendor}" PARENT_SCOPE)
      break()
    else()
      if("${result}" MATCHES  "timeout")
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "Checking whether the ${lang} compiler is ${vendor} using \"${flags}\" "
          "terminated after 10 s due to timeout.")
      else()
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "Checking whether the ${lang} compiler is ${vendor} using \"${flags}\" "
          "did not match \"${regex}\":\n${output}")
       endif()
    endif()
  endforeach()
endfunction(CMAKE_DETERMINE_COMPILER_ID_VENDOR)
