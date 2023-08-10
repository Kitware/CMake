#
# list of targets to test.  to add a target: put its files in the data
# subdirectory and add it to this list...  we run each target's
# data/*.input file through the parser and check to see if it matches
# the corresponding data/*.output file.  note that the empty-* case
# has special handling (it should not parse).
#
set(targets
  aix-C-XL-13.1.3 aix-CXX-XL-13.1.3
  aix-C-XLClang-16.1.0.1 aix-CXX-XLClang-16.1.0.1
  craype-C-Cray-8.7 craype-CXX-Cray-8.7 craype-Fortran-Cray-8.7
  craype-C-Cray-9.0-hlist-ad craype-CXX-Cray-9.0-hlist-ad craype-Fortran-Cray-9.0-hlist-ad
  craype-C-GNU-7.3.0 craype-CXX-GNU-7.3.0 craype-Fortran-GNU-7.3.0
  craype-C-Intel-18.0.2.20180210 craype-CXX-Intel-18.0.2.20180210
    craype-Fortran-Intel-18.0.2.20180210
  darwin-C-AppleClang-8.0.0.8000042 darwin-CXX-AppleClang-8.0.0.8000042
    darwin_nostdinc-C-AppleClang-8.0.0.8000042
    darwin_nostdinc-CXX-AppleClang-8.0.0.8000042
  freebsd-C-Clang-3.3.0 freebsd-CXX-Clang-3.3.0 freebsd-Fortran-GNU-4.6.4
  hand-C-empty hand-CXX-empty
  hand-C-relative hand-CXX-relative
  linux-C-GNU-7.3.0 linux-CXX-GNU-7.3.0 linux-Fortran-GNU-7.3.0
  linux-C-GNU-10.2.1-static-libgcc
    linux-CXX-GNU-10.2.1-static-libstdc++
    linux-Fortran-GNU-10.2.1-static-libgfortran
  linux-C-Intel-18.0.0.20170811 linux-CXX-Intel-18.0.0.20170811
  linux-C-PGI-18.10.1 linux-CXX-PGI-18.10.1
    linux-Fortran-PGI-18.10.1 linux_pgf77-Fortran-PGI-18.10.1
    linux_nostdinc-C-PGI-18.10.1 linux_nostdinc-CXX-PGI-18.10.1
    linux_nostdinc-Fortran-PGI-18.10.1
  linux-C-NVHPC-21.1.0 linux-CXX-NVHPC-21.1.0
  linux-C-XL-12.1.0 linux-CXX-XL-12.1.0 linux-Fortran-XL-14.1.0
    linux_nostdinc-C-XL-12.1.0 linux_nostdinc-CXX-XL-12.1.0
    linux_nostdinc_i-C-XL-12.1.0 linux_nostdinc-CXX-XL-12.1.0
  linux-C-XL-16.1.0.0 linux-CXX-XL-16.1.0.0
  linux-CUDA-NVIDIA-10.1.168-CLANG linux-CUDA-NVIDIA-10.1.168-XLClang-v
    linux-CUDA-NVIDIA-9.2.148-GCC
  linux-Fortran-LLVMFlang-15.0.0
  linux-custom_clang-C-Clang-13.0.0 linux-custom_clang-CXX-Clang-13.0.0
  mingw.org-C-GNU-4.9.3 mingw.org-CXX-GNU-4.9.3
  netbsd-C-GNU-4.8.5 netbsd-CXX-GNU-4.8.5
    netbsd_nostdinc-C-GNU-4.8.5 netbsd_nostdinc-CXX-GNU-4.8.5
  openbsd-C-Clang-5.0.1 openbsd-CXX-Clang-5.0.1
  sunos-C-SunPro-5.13.0 sunos-CXX-SunPro-5.13.0 sunos-Fortran-SunPro-8.8.0
  )

if(CMAKE_HOST_WIN32)
  # The KWSys actual-case cache breaks case sensitivity on Windows.
  list(FILTER targets EXCLUDE REGEX "-XL|-SunPro")
else()
  # Windows drive letters are not recognized as absolute on other platforms.
  list(FILTER targets EXCLUDE REGEX "mingw")
endif()

include(${CMAKE_ROOT}/Modules/CMakeParseImplicitLinkInfo.cmake)
include(${CMAKE_ROOT}/Modules/CMakeParseLibraryArchitecture.cmake)

#
# load_compiler_info: read infile, parsing out cmake compiler info
# variables as we go.  returns language, a list of variables we set
# (so we can clear them later), and the remaining verbose output
# from the compiler.
#
function(load_compiler_info infile lang_var outcmvars_var outstr_var)
  unset(lang)
  unset(outcmvars)
  unset(outstr)
  file(READ "${infile}" in)
  string(REGEX REPLACE "\r?\n" ";" in_lines "${in}")
  foreach(line IN LISTS in_lines)
    # check for special CMAKE variable lines and parse them if found
    if("${line}" MATCHES "^CMAKE_([_A-Za-z0-9+]+)=(.*)$")
      if("${CMAKE_MATCH_1}" STREQUAL "LANG")   # handle CMAKE_LANG here
        set(lang "${CMAKE_MATCH_2}")
      else()
        set(CMAKE_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" PARENT_SCOPE)
        list(APPEND outcmvars "CMAKE_${CMAKE_MATCH_1}")
      endif()
    else()
      string(APPEND outstr "${line}\n")
    endif()
  endforeach()
  if(NOT lang)
    message("load_compiler_info: ${infile} no LANG info; default to C")
    set(lang C)
  endif()

  set(${lang_var} "${lang}" PARENT_SCOPE)
  set(${outcmvars_var} "${outcmvars}" PARENT_SCOPE)
  set(${outstr_var} "${outstr}" PARENT_SCOPE)
endfunction()

#
# unload_compiler_info: clear out any CMAKE_* vars load previously set
#
function(unload_compiler_info cmvars)
  foreach(var IN LISTS cmvars)
    unset("${var}" PARENT_SCOPE)
  endforeach()
endfunction()


#
# load_platform_info: establish CMAKE_LIBRARY_ARCHITECTURE_REGEX
# based on the target platform.
#
function(load_platform_info target)
  if(target MATCHES "linux-")
    set(CMAKE_LIBRARY_ARCHITECTURE_REGEX "[a-z0-9_]+(-[a-z0-9_]+)?-linux-gnu[a-z0-9_]*" PARENT_SCOPE)
  else()
    unset(CMAKE_LIBRARY_ARCHITECTURE_REGEX PARENT_SCOPE)
  endif()
endfunction()

#
# main test loop
#
foreach(t ${targets})
  set(infile "${CMAKE_SOURCE_DIR}/../ParseImplicitData/${t}.input")
  set(outfile "${CMAKE_SOURCE_DIR}/results/${t}.output")
  if (NOT EXISTS ${infile})
    string(REPLACE  "-empty" "" infile "${infile}")
    if (NOT EXISTS ${infile})
      message("missing input file for target ${t} in ${CMAKE_SOURCE_DIR}/../ParseImplicitData/")
      continue()
    endif()
  elseif(NOT EXISTS ${outfile})
    message("missing files for target ${t} in ${CMAKE_SOURCE_DIR}/results/")
    continue()
  endif()

  load_compiler_info(${infile} lang cmvars input)
  load_platform_info(${t})

  # Need to handle files with empty entries for both libs or dirs
  set(implicit_lib_output "")
  set(idirs_output "")
  set(implicit_objs "")
  set(library_arch_output "")
  file(STRINGS ${outfile} outputs)
  foreach(line IN LISTS outputs)
    if(line MATCHES "libs=")
      string(REPLACE "libs=" "" implicit_lib_output "${line}")
    endif()
    if(line MATCHES "dirs=")
      string(REPLACE "dirs=" "" idirs_output "${line}")
    endif()
    if(line MATCHES "library_arch=")
      string(REPLACE "library_arch=" "" library_arch_output "${line}")
    endif()
  endforeach()

  cmake_parse_implicit_link_info("${input}" implicit_libs idirs implicit_fwks log
      "${CMAKE_${lang}_IMPLICIT_OBJECT_REGEX}"
      LANGUAGE ${lang}
      COMPUTE_IMPLICIT_OBJECTS implicit_objs)

  set(library_arch)
  cmake_parse_library_architecture(${lang} "${idirs}" "${implicit_objs}" library_arch)

  # File format
  # file(WRITE ${outfile} "libs=${implicit_libs}\ndirs=${idirs}\nlibrary_arch=${library_arch}")

  if(t MATCHES "-empty$")          # empty isn't supposed to parse
    if("${state}" STREQUAL "done")
      message("empty parse failed: ${idirs}, log=${log}")
    endif()
  elseif(NOT "${idirs}" MATCHES "^${idirs_output}$")
    message("${t} parse failed: state=${state}, '${idirs}' does not match '^${idirs_output}$'")
  elseif(NOT "${implicit_libs}" MATCHES "^${implicit_lib_output}$")
    message("${t} parse failed: state=${state}, '${implicit_libs}' does not match '^${implicit_lib_output}$'")
  elseif((library_arch OR library_arch_output) AND NOT "${library_arch}" MATCHES "^${library_arch_output}$")
    message("${t} parse failed: state=${state}, '${library_arch}' does not match '^${library_arch_output}$'")
  endif()

  unload_compiler_info("${cmvars}")
endforeach(t)
