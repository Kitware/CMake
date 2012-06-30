
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

# determine the compiler to use for ASM programs

if(NOT CMAKE_ASM${ASM_DIALECT}_COMPILER)
  # prefer the environment variable ASM
  if($ENV{ASM${ASM_DIALECT}} MATCHES ".+")
    set(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT "$ENV{ASM${ASM_DIALECT}}")
  endif($ENV{ASM${ASM_DIALECT}} MATCHES ".+")

  # finally list compilers to try
  if("ASM${ASM_DIALECT}" STREQUAL "ASM") # the generic assembler support

    if(CMAKE_ASM_COMPILER_INIT)
      set(CMAKE_ASM_COMPILER_LIST ${CMAKE_ASM_COMPILER_INIT})
    else(CMAKE_ASM_COMPILER_INIT)

      if(CMAKE_C_COMPILER)
        set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "The ASM compiler")
        set(CMAKE_ASM_COMPILER_ID "${CMAKE_C_COMPILER_ID}")
      elseif(CMAKE_CXX_COMPILER)
        set(CMAKE_ASM_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "The ASM compiler")
        set(CMAKE_ASM_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
      else(CMAKE_CXX_COMPILER)
        # List all default C and CXX compilers
        set(CMAKE_ASM${ASM_DIALECT}_COMPILER_LIST ${_CMAKE_TOOLCHAIN_PREFIX}gcc ${_CMAKE_TOOLCHAIN_PREFIX}cc cl bcc xlc
                                                  ${_CMAKE_TOOLCHAIN_PREFIX}c++ ${_CMAKE_TOOLCHAIN_PREFIX}g++ CC aCC cl bcc xlC)
      endif(CMAKE_C_COMPILER)

    endif(CMAKE_ASM_COMPILER_INIT)


  else("ASM${ASM_DIALECT}" STREQUAL "ASM") # some specific assembler "dialect"

    if(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT)
      set(CMAKE_ASM${ASM_DIALECT}_COMPILER_LIST ${CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT})
    else(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT)
      message(FATAL_ERROR "CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT must be preset !")
    endif(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT)

  endif("ASM${ASM_DIALECT}" STREQUAL "ASM")


  # Find the compiler.
  if(_CMAKE_USER_CXX_COMPILER_PATH OR _CMAKE_USER_C_COMPILER_PATH)
    find_program(CMAKE_ASM${ASM_DIALECT}_COMPILER NAMES ${CMAKE_ASM${ASM_DIALECT}_COMPILER_LIST} PATHS ${_CMAKE_USER_C_COMPILER_PATH} ${_CMAKE_USER_CXX_COMPILER_PATH} DOC "Assembler" NO_DEFAULT_PATH)
  endif(_CMAKE_USER_CXX_COMPILER_PATH OR _CMAKE_USER_C_COMPILER_PATH)
  find_program(CMAKE_ASM${ASM_DIALECT}_COMPILER NAMES ${CMAKE_ASM${ASM_DIALECT}_COMPILER_LIST} PATHS ${_CMAKE_TOOLCHAIN_LOCATION} DOC "Assembler")

else(NOT CMAKE_ASM${ASM_DIALECT}_COMPILER)

  # we only get here if CMAKE_ASM${ASM_DIALECT}_COMPILER was specified using -D or a pre-made CMakeCache.txt
  # (e.g. via ctest) or set in CMAKE_TOOLCHAIN_FILE
  #
  # if a compiler was specified by the user but without path,
  # now try to find it with the full path
  # if it is found, force it into the cache,
  # if not, don't overwrite the setting (which was given by the user) with "NOTFOUND"
  get_filename_component(_CMAKE_USER_ASM${ASM_DIALECT}_COMPILER_PATH "${CMAKE_ASM${ASM_DIALECT}_COMPILER}" PATH)
  if(NOT _CMAKE_USER_ASM${ASM_DIALECT}_COMPILER_PATH)
    find_program(CMAKE_ASM${ASM_DIALECT}_COMPILER_WITH_PATH NAMES ${CMAKE_ASM${ASM_DIALECT}_COMPILER})
    mark_as_advanced(CMAKE_ASM${ASM_DIALECT}_COMPILER_WITH_PATH)
    if(CMAKE_ASM${ASM_DIALECT}_COMPILER_WITH_PATH)
      set(CMAKE_ASM${ASM_DIALECT}_COMPILER ${CMAKE_ASM${ASM_DIALECT}_COMPILER_WITH_PATH} CACHE FILEPATH "Assembler" FORCE)
    endif(CMAKE_ASM${ASM_DIALECT}_COMPILER_WITH_PATH)
  endif(NOT _CMAKE_USER_ASM${ASM_DIALECT}_COMPILER_PATH)
endif(NOT CMAKE_ASM${ASM_DIALECT}_COMPILER)
mark_as_advanced(CMAKE_ASM${ASM_DIALECT}_COMPILER)

if(NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_ASM${ASM_DIALECT}_COMPILER}" PATH)
endif(NOT _CMAKE_TOOLCHAIN_LOCATION)


if(NOT CMAKE_ASM${ASM_DIALECT}_COMPILER_ID)

  # Table of per-vendor compiler id flags with expected output.
  list(APPEND CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDORS GNU )
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_FLAGS_GNU "--version")
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_REGEX_GNU "(GNU assembler)|(GCC)|(Free Software Foundation)")

  list(APPEND CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDORS HP )
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_FLAGS_HP "-V")
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_REGEX_HP "HP C")

  list(APPEND CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDORS Intel )
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_FLAGS_Intel "--version")
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_REGEX_Intel "(ICC)")

  list(APPEND CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDORS SunPro )
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_FLAGS_SunPro "-V")
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_REGEX_SunPro "Sun C")

  list(APPEND CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDORS XL )
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_FLAGS_XL "-qversion")
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_REGEX_XL "XL C")

  list(APPEND CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDORS MSVC )
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_FLAGS_MSVC "/?")
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_REGEX_MSVC "Microsoft")

  list(APPEND CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDORS TI_DSP )
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_FLAGS_TI_DSP "-h")
  set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_REGEX_TI_DSP "Texas Instruments")

  include(CMakeDetermineCompilerId)
  CMAKE_DETERMINE_COMPILER_ID_VENDOR(ASM${ASM_DIALECT})

endif()

if(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID)
  message(STATUS "The ASM${ASM_DIALECT} compiler identification is ${CMAKE_ASM${ASM_DIALECT}_COMPILER_ID}")
else(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID)
  message(STATUS "The ASM${ASM_DIALECT} compiler identification is unknown")
endif(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID)



# If we have a gas/as cross compiler, they have usually some prefix, like
# e.g. powerpc-linux-gas, arm-elf-gas or i586-mingw32msvc-gas , optionally
# with a 3-component version number at the end
# The other tools of the toolchain usually have the same prefix
# NAME_WE cannot be used since then this test will fail for names lile
# "arm-unknown-nto-qnx6.3.0-gas.exe", where BASENAME would be
# "arm-unknown-nto-qnx6" instead of the correct "arm-unknown-nto-qnx6.3.0-"
if(NOT _CMAKE_TOOLCHAIN_PREFIX)
  get_filename_component(COMPILER_BASENAME "${CMAKE_ASM${ASM_DIALECT}_COMPILER}" NAME)
  if(COMPILER_BASENAME MATCHES "^(.+-)g?as(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  endif(COMPILER_BASENAME MATCHES "^(.+-)g?as(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
endif(NOT _CMAKE_TOOLCHAIN_PREFIX)

# Now try the C compiler regexp:
if(NOT _CMAKE_TOOLCHAIN_PREFIX)
  if(COMPILER_BASENAME MATCHES "^(.+-)g?cc(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  endif(COMPILER_BASENAME MATCHES "^(.+-)g?cc(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
endif(NOT _CMAKE_TOOLCHAIN_PREFIX)

# Finally try the CXX compiler regexp:
if(NOT _CMAKE_TOOLCHAIN_PREFIX)
  if(COMPILER_BASENAME MATCHES "^(.+-)[gc]\\+\\+(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  endif(COMPILER_BASENAME MATCHES "^(.+-)[gc]\\+\\+(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
endif(NOT _CMAKE_TOOLCHAIN_PREFIX)


include(CMakeFindBinUtils)

set(CMAKE_ASM${ASM_DIALECT}_COMPILER_ENV_VAR "ASM${ASM_DIALECT}")

if(CMAKE_ASM${ASM_DIALECT}_COMPILER)
  message(STATUS "Found assembler: ${CMAKE_ASM${ASM_DIALECT}_COMPILER}")
else(CMAKE_ASM${ASM_DIALECT}_COMPILER)
  message(STATUS "Didn't find assembler")
endif(CMAKE_ASM${ASM_DIALECT}_COMPILER)


set(_CMAKE_ASM_COMPILER "${CMAKE_ASM${ASM_DIALECT}_COMPILER}")
set(_CMAKE_ASM_COMPILER_ID "${CMAKE_ASM${ASM_DIALECT}_COMPILER_ID}")
set(_CMAKE_ASM_COMPILER_ARG1 "${CMAKE_ASM${ASM_DIALECT}_COMPILER_ARG1}")
set(_CMAKE_ASM_COMPILER_ENV_VAR "${CMAKE_ASM${ASM_DIALECT}_COMPILER_ENV_VAR}")

# configure variables set in this file for fast reload later on
configure_file(${CMAKE_ROOT}/Modules/CMakeASMCompiler.cmake.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeASM${ASM_DIALECT}Compiler.cmake IMMEDIATE @ONLY)

set(_CMAKE_ASM_COMPILER)
set(_CMAKE_ASM_COMPILER_ARG1)
set(_CMAKE_ASM_COMPILER_ENV_VAR)
