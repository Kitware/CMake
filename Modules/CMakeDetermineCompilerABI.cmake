
# Function to compile a source file to identify the compiler ABI.
# This is used internally by CMake and should not be included by user
# code.

FUNCTION(CMAKE_DETERMINE_COMPILER_ABI lang src)
  IF(NOT DEFINED CMAKE_DETERMINE_${lang}_ABI_COMPILED)
    MESSAGE(STATUS "Detecting ${lang} compiler ABI info")

    # Compile the ABI identification source.
    SET(BIN "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeDetermineCompilerABI_${lang}.bin")
    TRY_COMPILE(CMAKE_DETERMINE_${lang}_ABI_COMPILED
      ${CMAKE_BINARY_DIR} ${src}
      OUTPUT_VARIABLE OUTPUT
      COPY_FILE "${BIN}"
      )

    # Load the resulting information strings.
    IF(CMAKE_DETERMINE_${lang}_ABI_COMPILED)
      MESSAGE(STATUS "Detecting ${lang} compiler ABI info - done")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Detecting ${lang} compiler ABI info compiled with the following output:\n${OUTPUT}\n\n")
      FILE(STRINGS "${BIN}" ABI_STRINGS LIMIT_COUNT 2 REGEX "INFO:[^[]*\\[")
      FOREACH(info ${ABI_STRINGS})
        IF("${info}" MATCHES ".*INFO:sizeof_dptr\\[0*([^]]*)\\].*")
          STRING(REGEX REPLACE ".*INFO:sizeof_dptr\\[0*([^]]*)\\].*" "\\1" ABI_SIZEOF_DPTR "${info}")
        ENDIF("${info}" MATCHES ".*INFO:sizeof_dptr\\[0*([^]]*)\\].*")
        IF("${info}" MATCHES ".*INFO:abi\\[([^]]*)\\].*")
          STRING(REGEX REPLACE ".*INFO:abi\\[([^]]*)\\].*" "\\1" ABI_NAME "${info}")
        ENDIF("${info}" MATCHES ".*INFO:abi\\[([^]]*)\\].*")
      ENDFOREACH(info)

      IF(ABI_SIZEOF_DPTR)
        SET(CMAKE_${lang}_SIZEOF_DATA_PTR "${ABI_SIZEOF_DPTR}" PARENT_SCOPE)
        SET(CMAKE_SIZEOF_VOID_P "${ABI_SIZEOF_DPTR}" PARENT_SCOPE)
      ENDIF(ABI_SIZEOF_DPTR)

      IF(ABI_NAME)
        SET(CMAKE_${lang}_COMPILER_ABI "${ABI_NAME}" PARENT_SCOPE)
        SET(CMAKE_INTERNAL_PLATFORM_ABI "${ABI_NAME}" PARENT_SCOPE)
      ENDIF(ABI_NAME)

    ELSE(CMAKE_DETERMINE_${lang}_ABI_COMPILED)
      MESSAGE(STATUS "Detecting ${lang} compiler ABI info - failed")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Detecting ${lang} compiler ABI info failed to compile with the following output:\n${OUTPUT}\n\n")
    ENDIF(CMAKE_DETERMINE_${lang}_ABI_COMPILED)
  ENDIF(NOT DEFINED CMAKE_DETERMINE_${lang}_ABI_COMPILED)
ENDFUNCTION(CMAKE_DETERMINE_COMPILER_ABI)
