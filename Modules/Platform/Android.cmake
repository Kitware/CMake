include(Platform/Linux)

set(ANDROID 1)

# Natively compiling on an Android host doesn't need these flags to be reset.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Android")
  return()
endif()

# Conventionally Android does not use versioned soname
# But in modern versions it is acceptable
if(NOT DEFINED CMAKE_PLATFORM_NO_VERSIONED_SONAME)
  set(CMAKE_PLATFORM_NO_VERSIONED_SONAME 1)
endif()

# Android reportedly ignores RPATH, and we cannot predict the install
# location anyway.
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "")

# Nsight Tegra Visual Studio Edition takes care of
# prefixing library names with '-l'.
if(CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android")
  set(CMAKE_LINK_LIBRARY_FLAG "")
endif()

# Commonly used Android toolchain files that pre-date CMake upstream support
# set CMAKE_SYSTEM_VERSION to 1.  Avoid interfering with them.
if(CMAKE_SYSTEM_VERSION EQUAL 1)
  return()
endif()

if(CMAKE_ANDROID_NDK_TOOLCHAIN_UNIFIED)
  # Tell CMake not to search host sysroots for headers/libraries.
  # CMAKE_FIND_ROOT_PATH must be non-empty for CMAKE_FIND_ROOT_PATH_MODE_* == ONLY
  # to be meaningful.  The actual path  used here is fairly meaningless since CMake
  # doesn't handle the NDK sysroot layout (per-arch and per-verion subdirectories for
  # libraries), so find_library is handled separately by CMAKE_SYSTEM_LIBRARY_PATH.
  # https://github.com/android-ndk/ndk/issues/890
  if(NOT CMAKE_FIND_ROOT_PATH)
    list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_ANDROID_NDK}")
  endif()

  # Allow users to override these values in case they want more strict behaviors.
  # For example, they may want to prevent the NDK's libz from being picked up so
  # they can use their own.
  # https://github.com/android-ndk/ndk/issues/517
  if(NOT DEFINED CMAKE_FIND_ROOT_PATH_MODE_PROGRAM)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  endif()

  if(NOT DEFINED CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  endif()

  if(NOT DEFINED CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  endif()

  if(NOT DEFINED CMAKE_FIND_ROOT_PATH_MODE_PACKAGE)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  endif()

  # find_library's default search paths below a prefix do not match the Android
  # sysroot layout, so we need to give the direct path to the libraries
  # via CMAKE_SYSTEM_*_PATH.
  #
  # Ideally we'd set CMAKE_SYSTEM_PREFIX_PATH. But that causes the
  # non-api-level-specific path to be searched first for find_library, which will
  # cause libdl.a to be found before libdl.so.
  # https://github.com/android/ndk/issues/929

  # Clears the paths set by UnixPaths.cmake.
  set(CMAKE_SYSTEM_PREFIX_PATH)
  set(CMAKE_SYSTEM_INCLUDE_PATH)
  set(CMAKE_SYSTEM_LIBRARY_PATH)

  # Don't search paths in PATH environment variable.
  if(NOT DEFINED CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
  endif()

  # Allows CMake to find headers in the architecture-specific include directories.
  set(CMAKE_LIBRARY_ARCHITECTURE "${CMAKE_ANDROID_ARCH_TRIPLE}")

  set(_ANDROID_SYSROOT_PREFIX "${CMAKE_ANDROID_NDK_TOOLCHAIN_UNIFIED}/sysroot/usr")

  list(APPEND CMAKE_SYSTEM_INCLUDE_PATH
    "${_ANDROID_SYSROOT_PREFIX}/include/${CMAKE_LIBRARY_ARCHITECTURE}")
  list(APPEND CMAKE_SYSTEM_INCLUDE_PATH "${_ANDROID_SYSROOT_PREFIX}/include")

  # Instructs CMake to search the correct API level for libraries.
  list(APPEND CMAKE_SYSTEM_LIBRARY_PATH
    "${_ANDROID_SYSROOT_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}/${CMAKE_SYSTEM_VERSION}")
  list(APPEND CMAKE_SYSTEM_LIBRARY_PATH
    "${_ANDROID_SYSROOT_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}")

  list(APPEND CMAKE_SYSTEM_PROGRAM_PATH "${CMAKE_ANDROID_NDK_TOOLCHAIN_UNIFIED}/bin")
endif()
