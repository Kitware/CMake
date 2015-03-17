include(ExternalProject)

set(_tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/tmp")
set(_cache_file "${_tmp_dir}/FOO-cache.cmake")

ExternalProject_Add(FOO TMP_DIR "${_tmp_dir}"
                        DOWNLOAD_COMMAND ""
                        CMAKE_CACHE_DEFAULT_ARGS "-DFOO:STRING=BAR")

if(NOT EXISTS "${_cache_file}")
  message(FATAL_ERROR "Initial cache not created")
endif()

file(READ "${_cache_file}" _cache)

if(NOT "${_cache}" MATCHES "set\\(FOO \"BAR\".+\\)") # \(\)
  message(FATAL_ERROR "Cannot find FOO argument in cache")
endif()
if("${CMAKE_MATCH_0}" MATCHES FORCE)
  message(FATAL_ERROR "Expected not forced FOO argument")
endif()
