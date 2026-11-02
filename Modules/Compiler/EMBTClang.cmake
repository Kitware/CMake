# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__COMPILER_EMBTCLANG)
  return()
endif()
set(__COMPILER_EMBTCLANG 1)

if("x${CMAKE_C_COMPILER_ID}" STREQUAL "xEMBTClang")
  set(__embt_driver "${CMAKE_C_COMPILER}")
elseif("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xEMBTClang")
  set(__embt_driver "${CMAKE_CXX_COMPILER}")
elseif("x${CMAKE_ASM_COMPILER_ID}" STREQUAL "xEMBTClang")
  set(__embt_driver "${CMAKE_ASM_COMPILER}")
endif()
if(__embt_driver)
  get_filename_component(EMBT_DRIVER_NAME "${__embt_driver}" NAME_WE)
endif()

if(WIN32 AND EMBT_DRIVER_NAME AND NOT CMAKE_RC_COMPILER_INIT AND NOT CMAKE_GENERATOR_RC)
  # Resolve the resource compiler that ships beside the driver to an absolute
  # path, not a bare name that would be taken from PATH at build time.
  get_filename_component(__embt_bindir "${__embt_driver}" DIRECTORY)
  find_program(__embt_rc NAMES ${EMBT_DRIVER_NAME}-windres
    HINTS "${__embt_bindir}"
    NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_DEFAULT_PATH)
  if(__embt_rc)
    set(CMAKE_RC_COMPILER_INIT "${__embt_rc}")
  else()
    set(CMAKE_RC_COMPILER_INIT ${EMBT_DRIVER_NAME}-windres)
  endif()
  unset(__embt_rc CACHE)
  unset(__embt_bindir)
endif()
unset(__embt_driver)
