cmake_minimum_required(VERSION 3.0)
project(Minimal NONE)

#
# list of targets to test.  to add a target: put its files in the data
# subdirectory and add it to this list...  we run each target's
# data/*.input file through the parser and check to see if it matches
# the corresponding data/*.output file.  note that the empty-* case
# has special handling (it should not parse).
#
set(targets
  craype-C-Cray-8.7 craype-CXX-Cray-8.7
  craype-C-GNU-7.3.0 craype-CXX-GNU-7.3.0
  craype-C-Intel-18.0.2.20180210 craype-CXX-Intel-18.0.2.20180210
  darwin-C-AppleClang-8.0.0.8000042 darwin-CXX-AppleClang-8.0.0.8000042
  empty-C empty-CXX
  freebsd-C-Clang-3.3.0 freebsd-CXX-Clang-3.3.0
  linux-C-GNU-7.3.0 linux-CXX-GNU-7.3.0
  linux-C-Intel-18.0.0.20170811 linux-CXX-Intel-18.0.0.20170811
  linux-CUDA-NVIDIA-9.2.148
  mingw.org-C-GNU-4.9.3 mingw.org-CXX-GNU-4.9.3
  netbsd-C-GNU-4.8.5 netbsd-CXX-GNU-4.8.5
  openbsd-C-Clang-5.0.1 openbsd-CXX-Clang-5.0.1
  #FIXME: The parser does not handle SunPro yet.
  #sunos-C-SunPro-5.13.0 sunos-CXX-SunPro-5.13.0
  )

include(${CMAKE_ROOT}/Modules/CMakeParseImplicitIncludeInfo.cmake)

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
    if("${line}" MATCHES "^CMAKE_([_A-Za-z0-9]+)=(.*)$")
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
# main test loop
#
foreach(t ${targets})
  set(infile "${CMAKE_SOURCE_DIR}/data/${t}.input")
  set(outfile "${CMAKE_SOURCE_DIR}/data/${t}.output")
  if (NOT EXISTS ${infile} OR NOT EXISTS ${outfile})
    message("missing files for target ${t} in ${CMAKE_SOURCE_DIR}/data")
    continue()
  endif()
  load_compiler_info(${infile} lang cmvars input)
  file(READ ${outfile} output)
  string(STRIP "${output}" output)
  cmake_parse_implicit_include_info("${input}" "${lang}" idirs log state)
  if(t MATCHES "^empty-")          # empty isn't supposed to parse
    if("${state}" STREQUAL "done")
      message("empty parse failed: ${idirs}, log=${log}")
    endif()
  elseif(NOT "${state}" STREQUAL "done" OR NOT "${output}" STREQUAL "${idirs}")
    message("parse failed: state=${state}, ${output} != ${idirs}, log=${log}")
  endif()
  unload_compiler_info("${cmvars}")
endforeach(t)
