# Guard against multiple inclusions
if(__craylinux_crayprgenv)
  return()
endif()
set(__craylinux_crayprgenv 1)

macro(__CrayPrgEnv_setup lang)
  if(DEFINED ENV{CRAYPE_VERSION})
    message(STATUS "Cray Programming Environment $ENV{CRAYPE_VERSION} ${lang}")
  elseif(DEFINED ENV{ASYNCPE_VERSION})
    message(STATUS "Cray XT Programming Environment $ENV{ASYNCPE_VERSION} ${lang}")
  else()
    message(STATUS "Cray Programming Environment (unknown version) ${lang}")
  endif()

  # Flags for the Cray wrappers
  set(CMAKE_STATIC_LIBRARY_LINK_${lang}_FLAGS "-static")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-shared")
  set(CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "-dynamic")

  # If the link type is not explicitly specified in the environment then
  # the Cray wrappers assume that the code will be built statically so
  # we check the following condition(s) are NOT met
  #  Compiler flags are explicitly dynamic
  #  Env var is dynamic and compiler flags are not explicitly static
  if(NOT (((CMAKE_${lang}_FLAGS MATCHES "(^| )-dynamic($| )") OR
         (CMAKE_EXE_LINKER_FLAGS MATCHES "(^| )-dynamic($| )"))
         OR
         (("$ENV{CRAYPE_LINK_TYPE}" STREQUAL "dynamic") AND
          NOT ((CMAKE_${lang}_FLAGS MATCHES "(^| )-static($| )") OR
               (CMAKE_EXE_LINKER_FLAGS MATCHES "(^| )-static($| )")))))
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
    set(BUILD_SHARED_LIBS FALSE CACHE BOOL "")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_LINK_SEARCH_START_STATIC TRUE)
  endif()
endmacro()
