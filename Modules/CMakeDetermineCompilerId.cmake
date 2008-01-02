
# Macro to compile a source file to identify the compiler.  This is
# used internally by CMake and should not be included by user code.
# If successful, sets CMAKE_<lang>_COMPILER_ID and CMAKE_<lang>_PLATFORM_ID

MACRO(CMAKE_DETERMINE_COMPILER_ID lang flagvar src)
  # Store the compiler identification source file.
  SET(CMAKE_${lang}_COMPILER_ID_SRC "${src}")
  IF(CMAKE_HOST_WIN32 AND NOT CMAKE_HOST_UNIX)
    # This seems to escape spaces:
    #FILE(TO_NATIVE_PATH "${CMAKE_${lang}_COMPILER_ID_SRC}"
    #  CMAKE_${lang}_COMPILER_ID_SRC)
    STRING(REGEX REPLACE "/" "\\\\" CMAKE_${lang}_COMPILER_ID_SRC
      "${CMAKE_${lang}_COMPILER_ID_SRC}")
  ENDIF(CMAKE_HOST_WIN32 AND NOT CMAKE_HOST_UNIX)

  # Make sure the compiler arguments are clean.
  STRING(STRIP "${CMAKE_${lang}_COMPILER_ARG1}" CMAKE_${lang}_COMPILER_ID_ARG1)

  # Make sure user-specified compiler flags are used.
  IF(CMAKE_${lang}_FLAGS)
    SET(CMAKE_${lang}_COMPILER_ID_FLAGS ${CMAKE_${lang}_FLAGS})
  ELSE(CMAKE_${lang}_FLAGS)
    SET(CMAKE_${lang}_COMPILER_ID_FLAGS $ENV{${flagvar}})
  ENDIF(CMAKE_${lang}_FLAGS)

  # Create an empty directory in which to run the test.
  SET(CMAKE_${lang}_COMPILER_ID_DIR ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CompilerId${lang})
  FILE(REMOVE_RECURSE ${CMAKE_${lang}_COMPILER_ID_DIR})
  FILE(MAKE_DIRECTORY ${CMAKE_${lang}_COMPILER_ID_DIR})

  # Compile the compiler identification source.
  STRING(REGEX REPLACE " " ";" CMAKE_${lang}_COMPILER_ID_FLAGS_LIST "${CMAKE_${lang}_COMPILER_ID_FLAGS}")
  IF(COMMAND EXECUTE_PROCESS)
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_${lang}_COMPILER} ${CMAKE_${lang}_COMPILER_ID_ARG1} ${CMAKE_${lang}_COMPILER_ID_FLAGS_LIST} ${CMAKE_${lang}_COMPILER_ID_SRC}
      WORKING_DIRECTORY ${CMAKE_${lang}_COMPILER_ID_DIR}
      OUTPUT_VARIABLE CMAKE_${lang}_COMPILER_ID_OUTPUT
      ERROR_VARIABLE CMAKE_${lang}_COMPILER_ID_OUTPUT
      RESULT_VARIABLE CMAKE_${lang}_COMPILER_ID_RESULT
      )
  ELSE(COMMAND EXECUTE_PROCESS)
    EXEC_PROGRAM(
      ${CMAKE_${lang}_COMPILER} ${CMAKE_${lang}_COMPILER_ID_DIR}
      ARGS ${CMAKE_${lang}_COMPILER_ID_ARG1} ${CMAKE_${lang}_COMPILER_ID_FLAGS_LIST} \"${CMAKE_${lang}_COMPILER_ID_SRC}\"
      OUTPUT_VARIABLE CMAKE_${lang}_COMPILER_ID_OUTPUT
      RETURN_VALUE CMAKE_${lang}_COMPILER_ID_RESULT
      )
  ENDIF(COMMAND EXECUTE_PROCESS)

  # Check the result of compilation.
  IF(CMAKE_${lang}_COMPILER_ID_RESULT)
    # Compilation failed.
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Compiling the ${lang} compiler identification source file \""
      "${CMAKE_${lang}_COMPILER_ID_SRC}\" failed with the following output:\n"
      "${CMAKE_${lang}_COMPILER_ID_RESULT}\n"
      "${CMAKE_${lang}_COMPILER_ID_OUTPUT}\n\n")
    IF(NOT CMAKE_${lang}_COMPILER_ID_ALLOW_FAIL)
      MESSAGE(FATAL_ERROR "Compiling the ${lang} compiler identification source file \""
        "${CMAKE_${lang}_COMPILER_ID_SRC}\" failed with the following output:\n"
        "${CMAKE_${lang}_COMPILER_ID_RESULT}\n"
        "${CMAKE_${lang}_COMPILER_ID_OUTPUT}\n\n")
    ENDIF(NOT CMAKE_${lang}_COMPILER_ID_ALLOW_FAIL)
  ELSE(CMAKE_${lang}_COMPILER_ID_RESULT)
    # Compilation succeeded.
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Compiling the ${lang} compiler identification source file \""
      "${CMAKE_${lang}_COMPILER_ID_SRC}\" succeeded with the following output:\n"
      "${CMAKE_${lang}_COMPILER_ID_OUTPUT}\n\n")

    # Find the executable produced by the compiler, try all files in the binary dir
    SET(CMAKE_${lang}_COMPILER_ID)
    FILE(GLOB COMPILER_${lang}_PRODUCED_FILES ${CMAKE_${lang}_COMPILER_ID_DIR}/*)
    FOREACH(CMAKE_${lang}_COMPILER_ID_EXE ${COMPILER_${lang}_PRODUCED_FILES})
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Compilation of the ${lang} compiler identification source \""
        "${CMAKE_${lang}_COMPILER_ID_SRC}\" produced \""
        "${CMAKE_${lang}_COMPILER_ID_EXE}\"\n\n")

      # try to figure out the executable format: ELF, COFF, Mach-O
      IF(NOT CMAKE_EXECUTABLE_FORMAT)
        FILE(READ ${CMAKE_${lang}_COMPILER_ID_EXE} CMAKE_EXECUTABLE_MAGIC LIMIT 4 HEX)

        # ELF files start with 0x7f"ELF"
        IF("${CMAKE_EXECUTABLE_MAGIC}" STREQUAL "7f454c46")
          SET(CMAKE_EXECUTABLE_FORMAT "ELF" CACHE STRING "Executable file format")
        ENDIF("${CMAKE_EXECUTABLE_MAGIC}" STREQUAL "7f454c46")

#        # COFF (.exe) files start with "MZ"
#        IF("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "4d5a....")
#          SET(CMAKE_EXECUTABLE_FORMAT "COFF" CACHE STRING "Executable file format")
#        ENDIF("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "4d5a....")
#
#        # Mach-O files start with CAFEBABE or FEEDFACE, according to http://radio.weblogs.com/0100490/2003/01/28.html
#        IF("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "cafebabe")
#          SET(CMAKE_EXECUTABLE_FORMAT "MACHO" CACHE STRING "Executable file format")
#        ENDIF("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "cafebabe")
#        IF("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "feedface")
#          SET(CMAKE_EXECUTABLE_FORMAT "MACHO" CACHE STRING "Executable file format")
#        ENDIF("${CMAKE_EXECUTABLE_MAGIC}" MATCHES "feedface")

      ENDIF(NOT CMAKE_EXECUTABLE_FORMAT)

      # only check if we don't have it yet
      IF(NOT CMAKE_${lang}_COMPILER_ID)
        # Read the compiler identification string from the executable file.
        FILE(STRINGS ${CMAKE_${lang}_COMPILER_ID_EXE}
          CMAKE_${lang}_COMPILER_ID_STRINGS LIMIT_COUNT 2 REGEX "INFO:")
        FOREACH(info ${CMAKE_${lang}_COMPILER_ID_STRINGS})
          IF("${info}" MATCHES ".*INFO:compiler\\[([^]]*)\\].*")
            STRING(REGEX REPLACE ".*INFO:compiler\\[([^]]*)\\].*" "\\1"
              CMAKE_${lang}_COMPILER_ID "${info}")
          ENDIF("${info}" MATCHES ".*INFO:compiler\\[([^]]*)\\].*")
          IF("${info}" MATCHES ".*INFO:platform\\[([^]]*)\\].*")
            STRING(REGEX REPLACE ".*INFO:platform\\[([^]]*)\\].*" "\\1"
              CMAKE_${lang}_PLATFORM_ID "${info}")
          ENDIF("${info}" MATCHES ".*INFO:platform\\[([^]]*)\\].*")
        ENDFOREACH(info)

        # Check the compiler identification string.
        IF(CMAKE_${lang}_COMPILER_ID)
          # The compiler identification was found.
          FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
            "The ${lang} compiler identification is ${CMAKE_${lang}_COMPILER_ID}, found in \""
            "${CMAKE_${lang}_COMPILER_ID_EXE}\"\n\n")
        ELSE(CMAKE_${lang}_COMPILER_ID)
          # The compiler identification could not be found.
          FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
            "The ${lang} compiler identification could not be found in \""
            "${CMAKE_${lang}_COMPILER_ID_EXE}\"\n\n")
        ENDIF(CMAKE_${lang}_COMPILER_ID)
      ENDIF(NOT CMAKE_${lang}_COMPILER_ID)
    ENDFOREACH(CMAKE_${lang}_COMPILER_ID_EXE)

    # if the format is unknown after all files have been checked, put "Unknown" in the cache
    IF(NOT CMAKE_EXECUTABLE_FORMAT)
      SET(CMAKE_EXECUTABLE_FORMAT "Unknown" CACHE STRING "Executable file format")
    ELSE(NOT CMAKE_EXECUTABLE_FORMAT)
      MESSAGE(STATUS "The executable file format is ${CMAKE_EXECUTABLE_FORMAT}")
    ENDIF(NOT CMAKE_EXECUTABLE_FORMAT)

    IF(NOT COMPILER_${lang}_PRODUCED_FILES)
      # No executable was found.
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Compilation of the ${lang} compiler identification source \""
        "${CMAKE_${lang}_COMPILER_ID_SRC}\" did not produce an executable in "
        "${CMAKE_${lang}_COMPILER_ID_DIR} .\n\n")
    ENDIF(NOT COMPILER_${lang}_PRODUCED_FILES)

    IF(CMAKE_${lang}_COMPILER_ID)
      MESSAGE(STATUS "The ${lang} compiler identification is "
        "${CMAKE_${lang}_COMPILER_ID}")
    ELSE(CMAKE_${lang}_COMPILER_ID)
      MESSAGE(STATUS "The ${lang} compiler identification is unknown")
    ENDIF(CMAKE_${lang}_COMPILER_ID)
  ENDIF(CMAKE_${lang}_COMPILER_ID_RESULT)
ENDMACRO(CMAKE_DETERMINE_COMPILER_ID)
