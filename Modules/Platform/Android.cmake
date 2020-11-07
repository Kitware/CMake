# Include the NDK hook.
# It can be used by NDK to inject necessary fixes for an earlier cmake.
if(CMAKE_ANDROID_NDK)
  include(${CMAKE_ANDROID_NDK}/build/cmake/hooks/pre/Android.cmake OPTIONAL)
endif()

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

  # All paths added to CMAKE_SYSTEM_*_PATH below will be rerooted under
  # CMAKE_FIND_ROOT_PATH. This is set because:
  # 1. Users may structure their libraries in a way similar to NDK. When they do that,
  #    they can simply append another path to CMAKE_FIND_ROOT_PATH.
  # 2. CMAKE_FIND_ROOT_PATH must be non-empty for CMAKE_FIND_ROOT_PATH_MODE_* == ONLY
  #    to be meaningful. https://github.com/android-ndk/ndk/issues/890
  list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_ANDROID_NDK_TOOLCHAIN_UNIFIED}/sysroot")

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

  # Don't search paths in PATH environment variable.
  if(NOT DEFINED CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
  endif()

  # Allows CMake to find headers in the architecture-specific include directories.
  set(CMAKE_LIBRARY_ARCHITECTURE "${CMAKE_ANDROID_ARCH_TRIPLE}")

  # Instructs CMake to search the correct API level for libraries.
  # Besides the paths like <root>/<prefix>/lib/<arch>, cmake also searches <root>/<prefix>.
  # So we can add the API level specific directory directly.
  # https://github.com/android/ndk/issues/929
  list(PREPEND CMAKE_SYSTEM_PREFIX_PATH
    "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/${CMAKE_SYSTEM_VERSION}"
    )

  list(APPEND CMAKE_SYSTEM_PROGRAM_PATH "${CMAKE_ANDROID_NDK_TOOLCHAIN_UNIFIED}/bin")
endif()

# Include the NDK hook.
# It can be used by NDK to inject necessary fixes for an earlier cmake.
if(CMAKE_ANDROID_NDK)
  include(${CMAKE_ANDROID_NDK}/build/cmake/hooks/post/Android.cmake OPTIONAL)
endif()
