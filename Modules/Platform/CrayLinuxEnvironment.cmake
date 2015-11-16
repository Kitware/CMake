# Compute Node Linux doesn't quite work the same as native Linux so all of this
# needs to be custom.  We use the variables defined through Cray's environment
# modules to set up the right paths for things.

# Guard against multiple inclusions
if(__CrayLinuxEnvironment)
  return()
endif()
set(__CrayLinuxEnvironment 1)

set(UNIX 1)

if(DEFINED ENV{CRAYOS_VERSION})
  set(CMAKE_SYSTEM_VERSION "$ENV{CRAYOS_VERSION}")
elseif(DEFINED ENV{XTOS_VERSION})
  set(CMAKE_SYSTEM_VERSION "$ENV{XTOS_VERSION}")
else()
  message(FATAL_ERROR "Neither the CRAYOS_VERSION or XTOS_VERSION environment variables are defined.  This platform file should be used inside the Cray Linux Environment for targeting compute nodes (NIDs)")
endif()
message(STATUS "Cray Linux Environment ${CMAKE_SYSTEM_VERSION}")

# All cray systems are x86 CPUs and have been for quite some time
# Note: this may need to change in the future with 64-bit ARM
set(CMAKE_SYSTEM_PROCESSOR "x86_64")

set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

set(CMAKE_DL_LIBS dl)

# Note: Much of this is pulled from UnixPaths.cmake but adjusted to the Cray
# environment accordingly

# Get the install directory of the running cmake to the search directories
# CMAKE_ROOT is CMAKE_INSTALL_PREFIX/share/cmake, so we need to go two levels up
get_filename_component(__cmake_install_dir "${CMAKE_ROOT}" PATH)
get_filename_component(__cmake_install_dir "${__cmake_install_dir}" PATH)


# Note: Some Cray's have the SYSROOT_DIR variable defined, pointing to a copy
# of the NIDs userland.  If so, then we'll use it.  Otherwise, just assume
# the userland from the login node is ok

# List common installation prefixes.  These will be used for all
# search types.
list(APPEND CMAKE_SYSTEM_PREFIX_PATH
  # Standard
  $ENV{SYSROOT_DIR}/usr/local $ENV{SYSROOT_DIR}/usr $ENV{SYSROOT_DIR}/

  # CMake install location
  "${__cmake_install_dir}"
  )
if (NOT CMAKE_FIND_NO_INSTALL_PREFIX)
  list(APPEND CMAKE_SYSTEM_PREFIX_PATH
    # Project install destination.
    "${CMAKE_INSTALL_PREFIX}"
  )
  if(CMAKE_STAGING_PREFIX)
    list(APPEND CMAKE_SYSTEM_PREFIX_PATH
      # User-supplied staging prefix.
      "${CMAKE_STAGING_PREFIX}"
    )
  endif()
endif()

list(APPEND CMAKE_SYSTEM_INCLUDE_PATH
  $ENV{SYSROOT_DIR}/usr/include
  $ENV{SYSROOT_DIR}/usr/include/X11
)
list(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  $ENV{SYSROOT_DIR}/usr/local/lib64
  $ENV{SYSROOT_DIR}/usr/lib64
  $ENV{SYSROOT_DIR}/lib64
)

list(APPEND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
  $ENV{SYSROOT_DIR}/usr/local/lib64
  $ENV{SYSROOT_DIR}/usr/lib64
  $ENV{SYSROOT_DIR}/lib64
)
list(APPEND CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES
  $ENV{SYSROOT_DIR}/usr/include
)
list(APPEND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES
  $ENV{SYSROOT_DIR}/usr/include
)
list(APPEND CMAKE_Fortran_IMPLICIT_INCLUDE_DIRECTORIES
  $ENV{SYSROOT_DIR}/usr/include
)

# Enable use of lib64 search path variants by default.
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

# Check to see if we're using the cray compiler wrappers and load accordingly
# if we are
if(DEFINED ENV{CRAYPE_DIR})
  set(_CRAYPE_ROOT "$ENV{CRAYPE_DIR}")
elseif(DEFINED ENV{ASYNCPE_DIR})
  set(_CRAYPE_ROOT "$ENV{ASYNCPE_DIR}")
endif()
if(_CRAYPE_ROOT AND
   ((CMAKE_C_COMPILER MATCHES "${_CRAYPE_ROOT}") OR
    (CMAKE_CXX_COMPILER MATCHES "${_CRAYPE_ROOT}") OR
    (CMAKE_Fortran_COMPILER MATCHES "${_CRAYPE_ROOT}")))
  include(Platform/CrayPrgEnv)
endif()
