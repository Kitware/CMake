# Guard against multiple inclusions
if(__CrayPrgEnv)
  return()
endif()
set(__CrayPrgEnv 1)
if(DEFINED ENV{CRAYPE_VERSION})
  message(STATUS "Cray Programming Environment $ENV{CRAYPE_VERSION}")
  set(__verbose_flag "-craype-verbose")
elseif(DEFINED ENV{ASYNCPE_VERSION})
  message(STATUS "Cray Programming Environment $ENV{ASYNCPE_VERSION}")
  set(__verbose_flag "-v")
else()
  message(STATUS "Cray Programming Environment")
endif()

if(NOT __CrayLinuxEnvironment)
  message(FATAL_ERROR "The CrayPrgEnv platform file must not be used on its own and is intented to be included by the CrayLinuxEnvironment platform file")
endif()

# Flags for the Cray wrappers
foreach(__lang C CXX Fortran)
  set(CMAKE_STATIC_LIBRARY_LINK_${__lang}_FLAGS "-static")
  set(CMAKE_SHARED_LIBRARY_${__lang}_FLAGS "")
  set(CMAKE_SHARED_LIBRARY_CREATE_${__lang}_FLAGS "-shared")
  set(CMAKE_SHARED_LIBRARY_LINK_${__lang}_FLAGS "-dynamic")
endforeach()

# If the link type is not explicitly specified in the environment then
# the Cray wrappers assume that the code will be built staticly so
# we check the following condition(s) are NOT met
#  Compiler flags are explicitly dynamic
#  Env var is dynamic and compiler flags are not explicitly static
if(NOT (((CMAKE_C_FLAGS MATCHES "(^| )-dynamic($| )") OR
         (CMAKE_CXX_FLAGS MATCHES "(^| )-dynamic($| )") OR
         (CMAKE_Fortran_FLAGS MATCHES "(^| )-dynamic($| )") OR
         (CMAKE_EXE_LINKER_FLAGS MATCHES "(^| )-dynamic($| )"))
        OR
       (("$ENV{CRAYPE_LINK_TYPE}" STREQUAL "dynamic") AND
         NOT ((CMAKE_C_FLAGS MATCHES "(^| )-static($| )") OR
              (CMAKE_CXX_FLAGS MATCHES "(^| )-static($| )") OR
              (CMAKE_Fortran_FLAGS MATCHES "(^| )-static($| )") OR
              (CMAKE_EXE_LINKER_FLAGS MATCHES "(^| )-static($| )")))))
  set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
  set(BUILD_SHARED_LIBS FALSE CACHE BOOL "")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_LINK_SEARCH_START_STATIC TRUE)
endif()

function(__cray_parse_flags_with_sep OUTPUT FLAG_TAG SEP INPUT)
  string(REGEX MATCHALL "${SEP}${FLAG_TAG}([^${SEP}]+)" FLAG_ARGS "${INPUT}")
  foreach(FLAG_ARG IN LISTS FLAG_ARGS)
    string(REGEX REPLACE
      "^${SEP}${FLAG_TAG}([^${SEP}]+)" "\\1" FLAG_VALUE
      "${FLAG_ARG}")
    list(APPEND ${OUTPUT} ${FLAG_VALUE})
  endforeach()
  set(${OUTPUT} ${${OUTPUT}} PARENT_SCOPE)
endfunction()
macro(__cray_parse_flags OUTPUT FLAG_TAG INPUT)
  __cray_parse_flags_with_sep(${OUTPUT} ${FLAG_TAG} " " "${INPUT}")
endmacro()

# Remove duplicates in a list
macro(__cray_list_remove_duplicates VAR)
  if(${VAR})
    list(REMOVE_DUPLICATES ${VAR})
  endif()
endmacro()

# Compute the intersection of several lists
function(__cray_list_intersect OUTPUT INPUT0)
  if(ARGC EQUAL 2)
    list(APPEND ${OUTPUT} ${${INPUT0}})
  else()
    foreach(I IN LISTS ${INPUT0})
      set(__is_common 1)
      foreach(L IN LISTS ARGN)
        list(FIND ${L} "${I}" __idx)
        if(__idx EQUAL -1)
          set(__is_common 0)
          break()
        endif()
      endforeach()
      if(__is_common)
        list(APPEND ${OUTPUT}  "${I}")
      endif()
    endforeach()
  endif()
  set(${OUTPUT} ${${OUTPUT}} PARENT_SCOPE)
endfunction()

# Parse the implicit directories used by the wrappers
get_property(__langs GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(__lang IN LISTS __langs)
  if(__lang STREQUAL "C")
    set(__empty empty.c)
  elseif(__lang STREQUAL CXX)
    set(__empty empty.cxx)
  elseif(__lang STREQUAL Fortran)
    set(__empty empty.f90)
  else()
    continue()
  endif()

  execute_process(
    COMMAND ${CMAKE_${__lang}_COMPILER} ${__verbose_flag} ${__empty}
    OUTPUT_VARIABLE __cmd_out
    ERROR_QUIET
  )
  string(REGEX MATCH "(^|\n)[^\n]*${__empty}[^\n]*" __driver "${__cmd_out}")

  # Parse include paths
  set(__cray_flag_args)
  __cray_parse_flags(__cray_flag_args "-I" "${__driver}")
  __cray_parse_flags(__cray_flag_args "-isystem " "${__driver}")
  list(APPEND CMAKE_${__lang}_IMPLICIT_INCLUDE_DIRECTORIES ${__cray_flag_args})
  __cray_list_remove_duplicates(CMAKE_${__lang}_IMPLICIT_INCLUDE_DIRECTORIES)

  # Parse library paths
  set(__cray_flag_args)
  __cray_parse_flags(__cray_flag_args "-L" "${__driver}")
  list(APPEND CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES ${__cray_flag_args})
  __cray_list_remove_duplicates(CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES)

  # Parse libraries
  set(__cray_flag_args)
  __cray_parse_flags(__cray_flag_args "-l" "${__driver}")
  __cray_parse_flags(__cray_linker_flags "-Wl" "${__driver}")
  foreach(F IN LISTS __cray_linker_flags)
    __cray_parse_flags_with_sep(__cray_flag_args "-l" "," "${F}")
  endforeach()
  list(APPEND CMAKE_${__lang}_IMPLICIT_LINK_LIBRARIES ${__cray_flag_args})
  __cray_list_remove_duplicates(CMAKE_${__lang}_IMPLICIT_LINK_LIBRARIES)
endforeach()

# Determine the common directories between all languages and add them
# as system search paths
set(__cray_inc_path_vars)
set(__cray_lib_path_vars)
foreach(__lang IN LISTS __langs)
  list(APPEND __cray_inc_path_vars CMAKE_${__lang}_IMPLICIT_INCLUDE_DIRECTORIES)
  list(APPEND __cray_lib_path_vars CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES)
endforeach()
if(__cray_inc_path_vars)
  __cray_list_intersect(CMAKE_SYSTEM_INCLUDE_PATH ${__cray_inc_path_vars})
endif()
if(__cray_lib_path_vars)
  __cray_list_intersect(CMAKE_SYSTEM_LIBRARY_PATH ${__cray_lib_path_vars})
endif()
