# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


# Runs immediately after CMakeFindBinUtils during compiler determination.
# CMakeFindBinUtils does not match EMBTClang in its Clang/IntelLLVM/LLVMFlang
# regex, so it falls through to the generic branch and finds whatever "ar"
# happens to be on PATH (often a stray ar.exe from MSYS, MinGW, etc.).
# Override CMAKE_AR/CMAKE_RANLIB with the llvm-ar/llvm-ranlib that ship
# alongside bcc64x in the Embarcadero installation.

if(NOT DEFINED _CMAKE_PROCESSING_LANGUAGE OR _CMAKE_PROCESSING_LANGUAGE STREQUAL "")
  message(FATAL_ERROR "Internal error: _CMAKE_PROCESSING_LANGUAGE is not set")
endif()

# The compiler path is fully resolved by this point in the determination
# phase, so get_filename_component yields a real directory.
get_filename_component(__embt_hint
  "${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER}" REALPATH)
get_filename_component(__embt_hint "${__embt_hint}" DIRECTORY)

find_program(__embt_ar NAMES llvm-ar
  HINTS "${__embt_hint}"
  NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_DEFAULT_PATH)
if(__embt_ar)
  set(CMAKE_AR "${__embt_ar}" CACHE FILEPATH "Archiver" FORCE)
  set(CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_AR "${__embt_ar}"
      CACHE FILEPATH "LLVM archiver" FORCE)
  mark_as_advanced(CMAKE_AR
    CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_AR)
endif()
unset(__embt_ar CACHE)

find_program(__embt_ranlib NAMES llvm-ranlib
  HINTS "${__embt_hint}"
  NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_DEFAULT_PATH)
if(__embt_ranlib)
  set(CMAKE_RANLIB "${__embt_ranlib}" CACHE FILEPATH "Archive index tool" FORCE)
  set(CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_RANLIB "${__embt_ranlib}"
      CACHE FILEPATH "Generate index for LLVM archive" FORCE)
  mark_as_advanced(CMAKE_RANLIB
    CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_RANLIB)
endif()
unset(__embt_ranlib CACHE)

# Pin the linker and the remaining LLVM binutils that ship beside bcc64x, for the
# same reason as CMAKE_AR above: CMakeFindBinUtils leaves them unset (or, for a
# bare name, resolvable from a stray PATH entry).  Linking itself goes through
# the bcc64x driver, so CMAKE_LINKER is informational (CheckLinkerFlag and the
# like consume it).
foreach(__embt_tool "LINKER=ld.lld" "NM=llvm-nm" "OBJDUMP=llvm-objdump"
                    "STRIP=llvm-strip" "DLLTOOL=llvm-dlltool")
  string(REPLACE "=" ";" __embt_tool "${__embt_tool}")
  list(GET __embt_tool 0 __embt_var)
  list(GET __embt_tool 1 __embt_prog)
  find_program(__embt_found NAMES ${__embt_prog}
    HINTS "${__embt_hint}"
    NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_DEFAULT_PATH)
  if(__embt_found)
    set(CMAKE_${__embt_var} "${__embt_found}" CACHE FILEPATH "${__embt_prog}" FORCE)
    mark_as_advanced(CMAKE_${__embt_var})
  endif()
  unset(__embt_found CACHE)
endforeach()
unset(__embt_var)
unset(__embt_prog)
unset(__embt_tool)

unset(__embt_hint)
