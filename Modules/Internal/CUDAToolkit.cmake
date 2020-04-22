# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# This file is for sharing code for finding basic CUDA toolkit information between
# CMakeDetermineCUDACompiler.cmake and FindCUDAToolkit.cmake.

# For NVCC we can easily deduce the SDK binary directory from the compiler path.
if(CMAKE_CUDA_COMPILER_LOADED AND NOT CUDAToolkit_BIN_DIR AND CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  get_filename_component(cuda_dir "${CMAKE_CUDA_COMPILER}" DIRECTORY)
  set(CUDAToolkit_BIN_DIR "${cuda_dir}" CACHE PATH "")
  mark_as_advanced(CUDAToolkit_BIN_DIR)
  unset(cuda_dir)
endif()

# Try language- or user-provided path first.
if(CUDAToolkit_BIN_DIR)
  find_program(CUDAToolkit_NVCC_EXECUTABLE
    NAMES nvcc nvcc.exe
    PATHS ${CUDAToolkit_BIN_DIR}
    NO_DEFAULT_PATH
    )
endif()

# Search using CUDAToolkit_ROOT
find_program(CUDAToolkit_NVCC_EXECUTABLE
  NAMES nvcc nvcc.exe
  PATHS ENV CUDA_PATH
  PATH_SUFFIXES bin
)

# If the user specified CUDAToolkit_ROOT but nvcc could not be found, this is an error.
if(NOT CUDAToolkit_NVCC_EXECUTABLE AND (DEFINED CUDAToolkit_ROOT OR DEFINED ENV{CUDAToolkit_ROOT}))
  # Declare error messages now, print later depending on find_package args.
  set(fail_base "Could not find nvcc executable in path specified by")
  set(cuda_root_fail "${fail_base} CUDAToolkit_ROOT=${CUDAToolkit_ROOT}")
  set(env_cuda_root_fail "${fail_base} environment variable CUDAToolkit_ROOT=$ENV{CUDAToolkit_ROOT}")

  if(CUDAToolkit_FIND_REQUIRED)
    if(DEFINED CUDAToolkit_ROOT)
      message(FATAL_ERROR ${cuda_root_fail})
    elseif(DEFINED ENV{CUDAToolkit_ROOT})
      message(FATAL_ERROR ${env_cuda_root_fail})
    endif()
  else()
    if(NOT CUDAToolkit_FIND_QUIETLY)
      if(DEFINED CUDAToolkit_ROOT)
        message(STATUS ${cuda_root_fail})
      elseif(DEFINED ENV{CUDAToolkit_ROOT})
        message(STATUS ${env_cuda_root_fail})
      endif()
    endif()
    set(CUDAToolkit_FOUND FALSE)
    unset(fail_base)
    unset(cuda_root_fail)
    unset(env_cuda_root_fail)
    return()
  endif()
endif()

# CUDAToolkit_ROOT cmake / env variable not specified, try platform defaults.
#
# - Linux: /usr/local/cuda-X.Y
# - macOS: /Developer/NVIDIA/CUDA-X.Y
# - Windows: C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\vX.Y
#
# We will also search the default symlink location /usr/local/cuda first since
# if CUDAToolkit_ROOT is not specified, it is assumed that the symlinked
# directory is the desired location.
if(NOT CUDAToolkit_NVCC_EXECUTABLE)
  if(UNIX)
    if(NOT APPLE)
      set(platform_base "/usr/local/cuda-")
    else()
      set(platform_base "/Developer/NVIDIA/CUDA-")
    endif()
  else()
    set(platform_base "C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v")
  endif()

  # Build out a descending list of possible cuda installations, e.g.
  file(GLOB possible_paths "${platform_base}*")
  # Iterate the glob results and create a descending list.
  set(possible_versions)
  foreach (p ${possible_paths})
    # Extract version number from end of string
    string(REGEX MATCH "[0-9][0-9]?\\.[0-9]$" p_version ${p})
    if(IS_DIRECTORY ${p} AND p_version)
      list(APPEND possible_versions ${p_version})
    endif()
  endforeach()

  # Cannot use list(SORT) because that is alphabetical, we need numerical.
  # NOTE: this is not an efficient sorting strategy.  But even if a user had
  # every possible version of CUDA installed, this wouldn't create any
  # significant overhead.
  set(versions)
  foreach (v ${possible_versions})
    list(LENGTH versions num_versions)
    # First version, nothing to compare with so just append.
    if(num_versions EQUAL 0)
      list(APPEND versions ${v})
    else()
      # Loop through list.  Insert at an index when comparison is
      # VERSION_GREATER since we want a descending list.  Duplicates will not
      # happen since this came from a glob list of directories.
      set(i 0)
      set(early_terminate FALSE)
      while (i LESS num_versions)
        list(GET versions ${i} curr)
        if(v VERSION_GREATER curr)
          list(INSERT versions ${i} ${v})
          set(early_terminate TRUE)
          break()
        endif()
        math(EXPR i "${i} + 1")
      endwhile()
      # If it did not get inserted, place it at the end.
      if(NOT early_terminate)
        list(APPEND versions ${v})
      endif()
    endif()
  endforeach()

  # With a descending list of versions, populate possible paths to search.
  set(search_paths)
  foreach (v ${versions})
    list(APPEND search_paths "${platform_base}${v}")
  endforeach()

  # Force the global default /usr/local/cuda to the front on Unix.
  if(UNIX)
    list(INSERT search_paths 0 "/usr/local/cuda")
  endif()

  # Now search for nvcc again using the platform default search paths.
  find_program(CUDAToolkit_NVCC_EXECUTABLE
    NAMES nvcc nvcc.exe
    PATHS ${search_paths}
    PATH_SUFFIXES bin
  )

  # We are done with these variables now, cleanup for caller.
  unset(platform_base)
  unset(possible_paths)
  unset(possible_versions)
  unset(versions)
  unset(i)
  unset(early_terminate)
  unset(search_paths)

  if(NOT CUDAToolkit_NVCC_EXECUTABLE)
    if(CUDAToolkit_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find nvcc, please set CUDAToolkit_ROOT.")
    elseif(NOT CUDAToolkit_FIND_QUIETLY)
      message(STATUS "Could not find nvcc, please set CUDAToolkit_ROOT.")
    endif()

    set(CUDAToolkit_FOUND FALSE)
    return()
  endif()
endif()

if(NOT CUDAToolkit_BIN_DIR AND CUDAToolkit_NVCC_EXECUTABLE)
  get_filename_component(cuda_dir "${CUDAToolkit_NVCC_EXECUTABLE}" DIRECTORY)
  set(CUDAToolkit_BIN_DIR "${cuda_dir}" CACHE PATH "" FORCE)
  mark_as_advanced(CUDAToolkit_BIN_DIR)
  unset(cuda_dir)
endif()

get_filename_component(CUDAToolkit_ROOT_DIR ${CUDAToolkit_BIN_DIR} DIRECTORY ABSOLUTE)

# Handle cross compilation
if(CMAKE_CROSSCOMPILING)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7-a")
    # Support for NVPACK
    set(CUDAToolkit_TARGET_NAME "armv7-linux-androideabi")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    # Support for arm cross compilation
    set(CUDAToolkit_TARGET_NAME "armv7-linux-gnueabihf")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    # Support for aarch64 cross compilation
    if(ANDROID_ARCH_NAME STREQUAL "arm64")
      set(CUDAToolkit_TARGET_NAME "aarch64-linux-androideabi")
    else()
      set(CUDAToolkit_TARGET_NAME "aarch64-linux")
    endif(ANDROID_ARCH_NAME STREQUAL "arm64")
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
      set(CUDAToolkit_TARGET_NAME "x86_64-linux")
  endif()

  if(EXISTS "${CUDAToolkit_ROOT_DIR}/targets/${CUDAToolkit_TARGET_NAME}")
    set(CUDAToolkit_TARGET_DIR "${CUDAToolkit_ROOT_DIR}/targets/${CUDAToolkit_TARGET_NAME}")
    # add known CUDA target root path to the set of directories we search for programs, libraries and headers
    list(PREPEND CMAKE_FIND_ROOT_PATH "${CUDAToolkit_TARGET_DIR}")

    # Mark that we need to pop the root search path changes after we have
    # found all cuda libraries so that searches for our cross-compilation
    # libraries work when another cuda sdk is in CMAKE_PREFIX_PATH or
    # PATh
    set(_CUDAToolkit_Pop_ROOT_PATH True)
  endif()
else()
  # Not cross compiling
  set(CUDAToolkit_TARGET_DIR "${CUDAToolkit_ROOT_DIR}")
  # Now that we have the real ROOT_DIR, find components inside it.
  list(APPEND CMAKE_PREFIX_PATH ${CUDAToolkit_ROOT_DIR})

  # Mark that we need to pop the prefix path changes after we have
  # found the cudart library.
  set(_CUDAToolkit_Pop_Prefix True)
endif()

# Find the include/ directory
find_path(CUDAToolkit_INCLUDE_DIR
  NAMES cuda_runtime.h
)

# Find a tentative CUDAToolkit_LIBRARY_DIR. FindCUDAToolkit overrides it by searching for the CUDA runtime,
# but we can't do that here, as CMakeDetermineCUDACompiler wants to use it before the variables necessary
# for find_library() have been initialized.
if(EXISTS "${CUDAToolkit_TARGET_DIR}/lib64")
  set(CUDAToolkit_LIBRARY_DIR "${CUDAToolkit_TARGET_DIR}/lib64")
elseif(EXISTS "${CUDAToolkit_TARGET_DIR}/lib")
  set(CUDAToolkit_LIBRARY_DIR "${CUDAToolkit_TARGET_DIR}/lib")
endif()
