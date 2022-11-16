# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__COMPILER_IBMClang)
  return()
endif()
set(__COMPILER_IBMClang 1)

include(Compiler/CMakeCommonCompilerMacros)

set(__pch_header_C "c-header")
set(__pch_header_CXX "c++-header")
set(__pch_header_OBJC "objective-c-header")
set(__pch_header_OBJCXX "objective-c++-header")

include(Compiler/GNU)

macro(__compiler_ibmclang lang)
  __compiler_gnu(${lang})

  # Feature flags.
  set(CMAKE_${lang}_VERBOSE_FLAG "-v")
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIC "-fPIC")
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIE "-fPIC")
  set(CMAKE_${lang}_RESPONSE_FILE_FLAG "@")
  set(CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG "@")

  set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang} "-isystem ")
  set(CMAKE_${lang}_COMPILE_OPTIONS_VISIBILITY "-fvisibility=")

  set(CMAKE_${lang}_COMPILE_OPTIONS_TARGET "--target=")
  set(CMAKE_${lang}_COMPILE_OPTIONS_EXTERNAL_TOOLCHAIN "--gcc-toolchain=")

  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG "-Xlinker" " ")
  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG_SEP)

  if(CMAKE_${lang}_COMPILER_TARGET AND "${lang}" STREQUAL "CXX")
    list(APPEND CMAKE_${lang}_COMPILER_PREDEFINES_COMMAND "--target=${CMAKE_${lang}_COMPILER_TARGET}")
  endif()

  set(_CMAKE_${lang}_IPO_SUPPORTED_BY_CMAKE YES)
  set(_CMAKE_${lang}_IPO_MAY_BE_SUPPORTED_BY_COMPILER YES)

  set(_CMAKE_LTO_THIN TRUE)

  if(_CMAKE_LTO_THIN)
    set(CMAKE_${lang}_COMPILE_OPTIONS_IPO "-flto=thin")
  else()
    set(CMAKE_${lang}_COMPILE_OPTIONS_IPO "-flto")
  endif()

  set(__ar "${CMAKE_${lang}_COMPILER_AR}")
  set(__ranlib "${CMAKE_${lang}_COMPILER_RANLIB}")

  set(CMAKE_${lang}_ARCHIVE_CREATE_IPO
    "\"${__ar}\" cr <TARGET> <LINK_FLAGS> <OBJECTS>"
  )

  set(CMAKE_${lang}_ARCHIVE_APPEND_IPO
    "\"${__ar}\" r <TARGET> <LINK_FLAGS> <OBJECTS>"
  )

  set(CMAKE_${lang}_ARCHIVE_FINISH_IPO
    "\"${__ranlib}\" <TARGET>"
  )

  if("${lang}" STREQUAL "CXX")
    list(APPEND CMAKE_${lang}_COMPILER_PREDEFINES_COMMAND "-dM" "-E" "-c" "${CMAKE_ROOT}/Modules/CMakeCXXCompilerABI.cpp")
  endif()

  set(CMAKE_PCH_EXTENSION .pch)

  set(CMAKE_PCH_PROLOGUE "#pragma clang system_header")

  set(CMAKE_${lang}_COMPILE_OPTIONS_INSTANTIATE_TEMPLATES_PCH -fpch-instantiate-templates)

  set(CMAKE_${lang}_COMPILE_OPTIONS_USE_PCH -Xclang -include-pch -Xclang <PCH_FILE> -Xclang -include -Xclang <PCH_HEADER>)
  set(CMAKE_${lang}_COMPILE_OPTIONS_CREATE_PCH -Xclang -emit-pch -Xclang -include -Xclang <PCH_HEADER> -x ${__pch_header_${lang}})
endmacro()
