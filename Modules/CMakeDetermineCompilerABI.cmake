# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# Function to compile a source file to identify the compiler ABI.
# This is used internally by CMake and should not be included by user
# code.

include(${CMAKE_ROOT}/Modules/CMakeParseImplicitIncludeInfo.cmake)
include(${CMAKE_ROOT}/Modules/CMakeParseImplicitLinkInfo.cmake)
include(${CMAKE_ROOT}/Modules/CMakeParseLibraryArchitecture.cmake)
include(CMakeTestCompilerCommon)

function(CMAKE_DETERMINE_COMPILER_ABI lang src)
  if(NOT DEFINED CMAKE_${lang}_ABI_COMPILED)
    message(CHECK_START "Detecting ${lang} compiler ABI info")

    # Compile the ABI identification source.
    set(BIN "${CMAKE_PLATFORM_INFO_DIR}/CMakeDetermineCompilerABI_${lang}.bin")
    set(CMAKE_FLAGS )
    set(COMPILE_DEFINITIONS )
    if(DEFINED CMAKE_${lang}_VERBOSE_FLAG)
      set(CMAKE_FLAGS "-DEXE_LINKER_FLAGS=${CMAKE_${lang}_VERBOSE_FLAG}")
      set(COMPILE_DEFINITIONS "${CMAKE_${lang}_VERBOSE_FLAG}")
    endif()
    if(DEFINED CMAKE_${lang}_VERBOSE_COMPILE_FLAG)
      set(COMPILE_DEFINITIONS "${CMAKE_${lang}_VERBOSE_COMPILE_FLAG}")
    endif()
    if(lang MATCHES "^(CUDA|HIP)$")
      if(CMAKE_${lang}_ARCHITECTURES STREQUAL "native")
        # We are about to detect the native architectures, so we do
        # not yet know them.  Use all architectures during detection.
        set(CMAKE_${lang}_ARCHITECTURES "all")
      endif()
      set(CMAKE_${lang}_RUNTIME_LIBRARY "Static")
    endif()
    if(NOT "x${CMAKE_${lang}_COMPILER_ID}" STREQUAL "xMSVC")
      # Avoid adding our own platform standard libraries for compilers
      # from which we might detect implicit link libraries.
      list(APPEND CMAKE_FLAGS "-DCMAKE_${lang}_STANDARD_LIBRARIES=")
    endif()
    __TestCompiler_setTryCompileTargetType()

    # Avoid failing ABI detection on warnings.
    string(REGEX REPLACE "(^| )-Werror([= ][^-][^ ]*)?( |$)" " " CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS}")

    # Save the current LC_ALL, LC_MESSAGES, and LANG environment variables
    # and set them to "C" that way GCC's "search starts here" text is in
    # English and we can grok it.
    set(_orig_lc_all      $ENV{LC_ALL})
    set(_orig_lc_messages $ENV{LC_MESSAGES})
    set(_orig_lang        $ENV{LANG})
    set(ENV{LC_ALL}      C)
    set(ENV{LC_MESSAGES} C)
    set(ENV{LANG}        C)

    try_compile(CMAKE_${lang}_ABI_COMPILED
      SOURCES ${src}
      CMAKE_FLAGS ${CMAKE_FLAGS}
                  # Ignore unused flags when we are just determining the ABI.
                  "--no-warn-unused-cli"
      COMPILE_DEFINITIONS ${COMPILE_DEFINITIONS}
      OUTPUT_VARIABLE OUTPUT
      COPY_FILE "${BIN}"
      COPY_FILE_ERROR _copy_error
      __CMAKE_INTERNAL ABI
      )

    # Restore original LC_ALL, LC_MESSAGES, and LANG
    set(ENV{LC_ALL}      ${_orig_lc_all})
    set(ENV{LC_MESSAGES} ${_orig_lc_messages})
    set(ENV{LANG}        ${_orig_lang})

    # Move result from cache to normal variable.
    set(CMAKE_${lang}_ABI_COMPILED ${CMAKE_${lang}_ABI_COMPILED})
    unset(CMAKE_${lang}_ABI_COMPILED CACHE)
    if(CMAKE_${lang}_ABI_COMPILED AND _copy_error)
      set(CMAKE_${lang}_ABI_COMPILED 0)
    endif()
    set(CMAKE_${lang}_ABI_COMPILED ${CMAKE_${lang}_ABI_COMPILED} PARENT_SCOPE)

    # Load the resulting information strings.
    if(CMAKE_${lang}_ABI_COMPILED)
      message(CHECK_PASS "done")
      file(STRINGS "${BIN}" ABI_STRINGS LIMIT_COUNT 32 REGEX "INFO:[A-Za-z0-9_]+\\[[^]]*\\]")
      set(ABI_SIZEOF_DPTR "NOTFOUND")
      set(ABI_BYTE_ORDER "NOTFOUND")
      set(ABI_NAME "NOTFOUND")
      foreach(info ${ABI_STRINGS})
        if("${info}" MATCHES "INFO:sizeof_dptr\\[0*([^]]*)\\]" AND NOT ABI_SIZEOF_DPTR)
          set(ABI_SIZEOF_DPTR "${CMAKE_MATCH_1}")
        endif()
        if("${info}" MATCHES "INFO:byte_order\\[(BIG_ENDIAN|LITTLE_ENDIAN)\\]")
          set(byte_order "${CMAKE_MATCH_1}")
          if(ABI_BYTE_ORDER STREQUAL "NOTFOUND")
            # Tentatively use the value because this is the first occurrence.
            set(ABI_BYTE_ORDER "${byte_order}")
          elseif(NOT ABI_BYTE_ORDER STREQUAL "${byte_order}")
            # Drop value because multiple occurrences do not match.
            set(ABI_BYTE_ORDER "")
          endif()
        endif()
        if("${info}" MATCHES "INFO:abi\\[([^]]*)\\]" AND NOT ABI_NAME)
          set(ABI_NAME "${CMAKE_MATCH_1}")
        endif()
      endforeach()

      if(ABI_SIZEOF_DPTR)
        set(CMAKE_${lang}_SIZEOF_DATA_PTR "${ABI_SIZEOF_DPTR}" PARENT_SCOPE)
      elseif(CMAKE_${lang}_SIZEOF_DATA_PTR_DEFAULT)
        set(CMAKE_${lang}_SIZEOF_DATA_PTR "${CMAKE_${lang}_SIZEOF_DATA_PTR_DEFAULT}" PARENT_SCOPE)
      endif()

      if(ABI_BYTE_ORDER)
        set(CMAKE_${lang}_BYTE_ORDER "${ABI_BYTE_ORDER}" PARENT_SCOPE)
      endif()

      if(ABI_NAME)
        set(CMAKE_${lang}_COMPILER_ABI "${ABI_NAME}" PARENT_SCOPE)
      endif()

      # Parse implicit include directory for this language, if available.
      if(CMAKE_${lang}_VERBOSE_FLAG)
        set (implicit_incdirs "")
        cmake_parse_implicit_include_info("${OUTPUT}" "${lang}"
          implicit_incdirs log rv)
        message(CONFIGURE_LOG
          "Parsed ${lang} implicit include dir info: rv=${rv}\n${log}\n\n")
        if("${rv}" STREQUAL "done")
          # Entries that we have been told to explicitly pass as standard include
          # directories will not be implicitly added by the compiler.
          if(CMAKE_${lang}_STANDARD_INCLUDE_DIRECTORIES)
            list(REMOVE_ITEM implicit_incdirs ${CMAKE_${lang}_STANDARD_INCLUDE_DIRECTORIES})
          endif()

          # We parsed implicit include directories, so override the default initializer.
          set(_CMAKE_${lang}_IMPLICIT_INCLUDE_DIRECTORIES_INIT "${implicit_incdirs}")
        endif()
      endif()
      set(CMAKE_${lang}_IMPLICIT_INCLUDE_DIRECTORIES "${_CMAKE_${lang}_IMPLICIT_INCLUDE_DIRECTORIES_INIT}" PARENT_SCOPE)

      if(_CMAKE_${lang}_IMPLICIT_LINK_INFORMATION_DETERMINED_EARLY)
        # Use implicit linker information detected during compiler id step.
        set(implicit_dirs "${CMAKE_${lang}_IMPLICIT_LINK_DIRECTORIES}")
        set(implicit_objs "")
        set(implicit_libs "${CMAKE_${lang}_IMPLICIT_LINK_LIBRARIES}")
        set(implicit_fwks "${CMAKE_${lang}_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES}")
      else()
        # Parse implicit linker information for this language, if available.
        set(implicit_dirs "")
        set(implicit_objs "")
        set(implicit_libs "")
        set(implicit_fwks "")
        if(CMAKE_${lang}_VERBOSE_FLAG)
          CMAKE_PARSE_IMPLICIT_LINK_INFO("${OUTPUT}" implicit_libs implicit_dirs implicit_fwks log
            "${CMAKE_${lang}_IMPLICIT_OBJECT_REGEX}"
            COMPUTE_IMPLICIT_OBJECTS implicit_objs
            LANGUAGE ${lang})
          message(CONFIGURE_LOG
            "Parsed ${lang} implicit link information:\n${log}\n\n")
        endif()
        # for VS IDE Intel Fortran we have to figure out the
        # implicit link path for the fortran run time using
        # a try-compile
        if("${lang}" MATCHES "Fortran"
            AND "${CMAKE_GENERATOR}" MATCHES "Visual Studio")
          message(CHECK_START "Determine Intel Fortran Compiler Implicit Link Path")
          # Build a sample project which reports symbols.
          try_compile(IFORT_LIB_PATH_COMPILED
            PROJECT IntelFortranImplicit
            SOURCE_DIR ${CMAKE_ROOT}/Modules/IntelVSImplicitPath
            BINARY_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/IntelVSImplicitPath
            CMAKE_FLAGS
            "-DCMAKE_Fortran_FLAGS:STRING=${CMAKE_Fortran_FLAGS}"
            OUTPUT_VARIABLE _output)
          file(WRITE
            "${CMAKE_BINARY_DIR}/CMakeFiles/IntelVSImplicitPath/output.txt"
            "${_output}")
          include(${CMAKE_BINARY_DIR}/CMakeFiles/IntelVSImplicitPath/output.cmake OPTIONAL)
          message(CHECK_PASS "done")
        endif()
      endif()

      # Implicit link libraries cannot be used explicitly for multiple
      # OS X architectures, so we skip it.
      if(DEFINED CMAKE_OSX_ARCHITECTURES)
        if("${CMAKE_OSX_ARCHITECTURES}" MATCHES ";")
          set(implicit_libs "")
        endif()
      endif()

      if(DEFINED ENV{CMAKE_${lang}_IMPLICIT_LINK_DIRECTORIES_EXCLUDE})
        list(REMOVE_ITEM implicit_dirs $ENV{CMAKE_${lang}_IMPLICIT_LINK_DIRECTORIES_EXCLUDE})
      endif()

      set(CMAKE_${lang}_IMPLICIT_LINK_LIBRARIES "${implicit_libs}" PARENT_SCOPE)
      set(CMAKE_${lang}_IMPLICIT_LINK_DIRECTORIES "${implicit_dirs}" PARENT_SCOPE)
      set(CMAKE_${lang}_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES "${implicit_fwks}" PARENT_SCOPE)

      cmake_parse_library_architecture(${lang} "${implicit_dirs}" "${implicit_objs}" architecture_flag)
      if(architecture_flag)
        set(CMAKE_${lang}_LIBRARY_ARCHITECTURE "${architecture_flag}" PARENT_SCOPE)
      endif()

    else()
      message(CHECK_FAIL "failed")
    endif()
  endif()
endfunction()
