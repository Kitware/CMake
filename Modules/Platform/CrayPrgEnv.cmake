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
# the Cray wrappers assume that the code will be built staticly
if(NOT ((CMAKE_C_FLAGS MATCHES "(^| )-dynamic($| )") OR
        (CMAKE_EXE_LINKER_FLAGS MATCHES "(^| )-dynamic($| )") OR
        ("$ENV{CRAYPE_LINK_TYPE}" STREQUAL "dynamic")))
  set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
  set(BUILD_SHARED_LIBS FALSE CACHE BOOL "")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_LINK_SEARCH_START_STATIC TRUE)
endif()

# Parse the implicit directories used by the wrappers
get_property(__langs GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(__lang IN LISTS __langs)
  if(__lang STREQUAL "C")
    set(__empty_fname empty.c)
  elseif(__lang STREQUAL CXX)
    set(__empty_fname empty.cxx)
  elseif(__lang STREQUAL Fortran)
    set(__empty_fname empty.f90)
  else()
    continue()
  endif()

  execute_process(
    COMMAND ${CMAKE_${__lang}_COMPILER} ${__verbose_flag} ${__empty_fname}
    OUTPUT_VARIABLE __cray_output
    ERROR_QUIET
  )
  string(REGEX MATCH "(^|\n)[^\n]*${__empty_fname}[^\n]*" __cray_driver_cmd "${__cray_output}")

  # Parse include paths
  string(REGEX MATCHALL " -I([^ ]+)" __cray_include_flags "${__cray_driver_cmd}")
  foreach(_flag IN LISTS __cray_include_flags)
    string(REGEX REPLACE "^ -I([^ ]+)" "\\1" _dir "${_flag}")
    list(APPEND CMAKE_${__lang}_IMPLICIT_INCLUDE_DIRECTORIES ${_dir})
  endforeach()
  if(CMAKE_${__lang}_IMPLICIT_INCLUDE_DIRECTORIES)
    list(REMOVE_DUPLICATES CMAKE_${__lang}_IMPLICIT_INCLUDE_DIRECTORIES)
  endif()

  # Parse library paths
  string(REGEX MATCHALL " -L([^ ]+)" __cray_library_dir_flags "${__cray_driver_cmd}")
  foreach(_flag IN LISTS __cray_library_dir_flags)
    string(REGEX REPLACE "^ -L([^ ]+)" "\\1" _dir "${_flag}")
    list(APPEND CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES ${_dir})
  endforeach()
  if(CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES)
    list(REMOVE_DUPLICATES CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES)
  endif()

  # Parse library paths
  string(REGEX MATCHALL " -l([^ ]+)" __cray_library_flags "${__cray_driver_cmd}")
  foreach(_flag IN LISTS __cray_library_flags)
    string(REGEX REPLACE "^ -l([^ ]+)" "\\1" _dir "${_flag}")
    list(APPEND CMAKE_${__lang}_IMPLICIT_LINK_LIBRARIES ${_dir})
  endforeach()
  if(CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES)
    list(REMOVE_DUPLICATES CMAKE_${__lang}_IMPLICIT_LINK_LIBRARIES)
  endif()
endforeach()

# Compute the intersection of several lists
macro(__list_intersection L_OUT L0)
  if(ARGC EQUAL 2)
    list(APPEND ${L_OUT} ${${L0}})
  else()
    foreach(I IN LISTS ${L0})
      set(__is_common 1)
      foreach(L IN LISTS ARGN)
        list(FIND ${L} "${I}" __idx)
        if(__idx EQUAL -1)
          set(__is_common 0)
          break()
        endif()
      endforeach()
      if(__is_common)
        list(APPEND ${L_OUT}  "${I}")
      endif()
    endforeach()
  endif()
  if(${L_OUT})
    list(REMOVE_DUPLICATES ${L_OUT})
  endif()
endmacro()

# Determine the common directories between all languages and add them
# as system search paths
set(__cray_include_path_vars)
set(__cray_library_path_vars)
foreach(__lang IN LISTS __langs)
  list(APPEND __cray_include_path_vars CMAKE_${__lang}_IMPLICIT_INCLUDE_DIRECTORIES)
  list(APPEND __cray_library_path_vars CMAKE_${__lang}_IMPLICIT_LINK_DIRECTORIES)
endforeach()
if(__cray_include_path_vars)
  __list_intersection(CMAKE_SYSTEM_INCLUDE_PATH ${__cray_include_path_vars})
endif()
if(__cray_library_path_vars)
  __list_intersection(CMAKE_SYSTEM_LIBRARY_PATH ${__cray_library_path_vars})
endif()
