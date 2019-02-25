# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindCUDAToolkit
---------------

This script locates the NVIDIA CUDA toolkit, but does not require the ``CUDA``
language be enabled for a given project.  This module does not search for the
NVIDIA CUDA Samples or any specific CUDA libraries.  This module only searches
for the CUDA Toolkit root directory.

Search Behavior
^^^^^^^^^^^^^^^

Finding the CUDA Toolkit requires finding the ``nvcc`` executable, which is
searched for in the following order:

1. If the ``CUDAToolkit_ROOT`` cmake configuration variable (e.g.,
   ``-DCUDAToolkit_ROOT=/some/path``) *or* environment variable is defined, it
   will be searched first.  If both an environment variable **and** a
   configuration variable are specified, the *configuration* variable takes
   precedence.

   The directory specified here must be such that the executable ``nvcc`` can be
   found underneath the directory specified by ``CUDAToolkit_ROOT``.  If
   ``CUDAToolkit_ROOT`` is specified, but no ``nvcc`` is found underneath, this
   package is marked as **not** found.  No subsequent search attempts are
   performed.

2. The user's path is searched for ``nvcc`` using :command:`find_program`.  If
   this is found, no subsequent search attempts are performed.  Users are
   responsible for ensuring that the first ``nvcc`` to show up in the path is
   the desired path in the event that multiple CUDA Toolkits are installed.

3. On Unix systems, if the symbolic link ``/usr/local/cuda`` exists, this is
   used.  No subsequent search attempts are performed.  No default symbolic link
   location exists for the Windows platform.

4. The platform specific default install locations are searched.  If exactly one
   candidate is found, this is used.  The default CUDA Toolkit install locations
   searched are:

   +-------------+-------------------------------------------------------------+
   | Platform    | Search Pattern                                              |
   +=============+=============================================================+
   | macOS       | ``/Developer/NVIDIA/CUDA-X.Y``                              |
   +-------------+-------------------------------------------------------------+
   | Other Unix  | ``/usr/local/cuda-X.Y``                                     |
   +-------------+-------------------------------------------------------------+
   | Windows     | ``C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\vX.Y`` |
   +-------------+-------------------------------------------------------------+

   Where ``X.Y`` would be a specific version of the CUDA Toolkit, such as
   ``/usr/local/cuda-9.0`` or
   ``C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.0``

   .. note::

       When multiple CUDA Toolkits are installed in the default location of a
       system (e.g., both ``/usr/local/cuda-9.0`` and ``/usr/local/cuda-10.0``
       exist but the ``/usr/local/cuda`` symbolic link does **not** exist), this
       package is marked as **not** found.

       There are too many factors involved in making an automatic decision in
       the presence of multiple CUDA Toolkits being installed.  In this
       situation, users are encouraged to either (1) set ``CUDAToolkit_ROOT`` or
       (2) ensure that the correct ``nvcc`` executable shows up in ``$PATH`` for
       :command:`find_program` to find.

Options
^^^^^^^

``VERSION``
    If specified, describes the version of the CUDA Toolkit to search for.

``REQUIRED``
    If specified, configuration will error if a suitable CUDA Toolkit is not
    found.

``QUIET``
    If specified, the search for a suitable CUDA Toolkit will not produce any
    messages.

``EXACT``
    If specified, the CUDA Toolkit is considered found only if the exact
    ``VERSION`` specified is recovered.

Output Variables
^^^^^^^^^^^^^^^^

``CUDAToolkit_FOUND``
    A boolean specifying whether or not the CUDA Toolkit was found.

``CUDAToolkit_VERSION``
    The exact version of the CUDA Toolkit found (as reported by
    ``nvcc --version``).

``CUDAToolkit_ROOT_DIR``
    The root directory of the CUDA Toolkit found.  Note that this variable will
    be the same as ``CUDAToolkit_ROOT`` when specified *and* a suitable toolkit was
    found.

``CUDAToolkit_INCLUDE_DIRS``
    The path to the CUDA Toolkit ``include`` folder containing the header files
    required to compile a project linking against CUDA.

``CUDAToolkit_LIBRARY_DIR``
    The path to the CUDA Toolkit library directory that contains the CUDA
    Runtime library ``cudart``.

``CUDAToolkit_NVCC_EXECUTABLE``
    The path to the NVIDIA CUDA compiler ``nvcc``.  Note that this path may not
    **not** be the same as
    :variable:`CMAKE_CUDA_COMPILER <CMAKE_<LANG>_COMPILER>`.  ``nvcc`` must be
    found to determine the CUDA Toolkit version as well as determining other
    features of the Toolkit.  This variable is set for the convenience of
    modules that depend on this one.

#]=======================================================================]

# NOTE: much of this was simply extracted from FindCUDA.cmake.

#   James Bigler, NVIDIA Corp (nvidia.com - jbigler)
#   Abe Stephens, SCI Institute -- http://www.sci.utah.edu/~abe/FindCuda.html
#
#   Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
#
#   Copyright (c) 2007-2009
#   Scientific Computing and Imaging Institute, University of Utah
#
#   This code is licensed under the MIT License.  See the FindCUDA.cmake script
#   for the text of the license.

# The MIT License
#
# License for the specific language governing rights and limitations under
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
###############################################################################

# Attempt 1: try user provided paths first.
find_path(CUDAToolkit_ROOT_DIR
  NAMES nvcc nvcc.exe
  PATHS
    ${CUDAToolkit_ROOT}
    ENV CUDAToolkit_ROOT
  PATH_SUFFIXES bin bin64
  NO_DEFAULT_PATH
)

message(FATAL_ERROR "CUDAToolkit_ROOT_DIR: ${CUDAToolkit_ROOT_DIR}")

# If the user specified CUDAToolkit_ROOT but nvcc could not be found, this is an error.
if (NOT CUDAToolkit_ROOT_DIR AND (DEFINED CUDAToolkit_ROOT OR DEFINED ENV{CUDAToolkit_ROOT}))
  # Declare error messages now, print later depending on find_package args.
  set(fail_base "Could not find nvcc executable in path specified by")
  set(cuda_root_fail "${fail_base} CUDAToolkit_ROOT=${CUDAToolkit_ROOT}")
  set(env_cuda_root_fail "${fail_base} environment variable CUDAToolkit_ROOT=$ENV{CUDAToolkit_ROOT}")

  if (CUDAToolkit_FIND_REQUIRED)
    if (DEFINED CUDAToolkit_ROOT)
      message(FATAL_ERROR ${cuda_root_fail})
    elseif (DEFINED ENV{CUDAToolkit_ROOT})
      message(FATAL_ERROR ${env_cuda_root_fail})
    endif()
  else()
    if (NOT CUDAToolkit_FIND_QUIETLY)
      if (DEFINED CUDAToolkit_ROOT)
        message(STATUS ${cuda_root_fail})
      elseif (DEFINED ENV{CUDAToolkit_ROOT})
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
if (NOT CUDAToolkit_ROOT_DIR)
  if (UNIX)
    if (NOT APPLE)
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
    if (IS_DIRECTORY ${p} AND p_version)
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
    if (num_versions EQUAL 0)
      list(APPEND versions ${v})
    else()
      # Loop through list.  Insert at an index when comparison is
      # VERSION_GREATER since we want a descending list.  Duplicates will not
      # happen since this came from a glob list of directories.
      set(i 0)
      set(early_terminate FALSE)
      while (i LESS num_versions)
        list(GET versions ${i} curr)
        if (v VERSION_GREATER curr)
          list(INSERT versions ${i} ${v})
          set(early_terminate TRUE)
          break()
        endif()
        math(EXPR i "${i} + 1")
      endwhile()
      # If it did not get inserted, place it at the end.
      if (NOT early_terminate)
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
  if (UNIX)
    list(INSERT search_paths 0 "/usr/local/cuda")
  endif()

  # Now search for nvcc again using the platform default search paths.
  find_path(CUDAToolkit_ROOT_DIR
    NAMES nvcc nvcc.exe
    PATHS ${search_paths}
    PATH_SUFFIXES bin bin64
    NO_DEFAULT_PATH
  )

  # We are done with these variables now, cleanup for caller.
  unset(platform_base)
  unset(possible_paths)
  unset(possible_versions)
  unset(versions)
  unset(i)
  unset(early_terminate)
  unset(search_paths)

  if (NOT CUDAToolkit_ROOT_DIR)
    if (CUDAToolkit_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find nvcc, please set CUDAToolkit_ROOT.")
    elseif(NOT CUDAToolkit_FIND_QUIETLY)
      message(STATUS "Could not find nvcc, please set CUDAToolkit_ROOT.")
    endif()

    set(CUDAToolkit_FOUND FALSE)
    return()
  endif()
endif()

# TODO: why does FindCUDA.cmake go through effor tof `cuda_find_host_program`
#       when CMAKE_CROSSCOMPILING // is that still relevant?
#       https://gitlab.kitware.com/cmake/cmake/issues/16509
# NOTE: search before trimming bin / bin64 from CUDAToolkit_ROOT_DIR
find_program(CUDAToolkit_NVCC_EXECUTABLE
  NAMES nvcc nvcc.exe
  PATHS ${CUDAToolkit_ROOT_DIR}
  NO_DEFAULT_PATH
)
# Compute the version.
execute_process(
  COMMAND ${CUDAToolkit_NVCC_EXECUTABLE} "--version"
  OUTPUT_VARIABLE NVCC_OUT
)
string(
  REGEX REPLACE ".*release ([0-9]+)\\.([0-9]+).*" "\\1"
  CUDAToolkit_VERSION_MAJOR ${NVCC_OUT}
)
string(
  REGEX REPLACE ".*release ([0-9]+)\\.([0-9]+).*" "\\2"
  CUDAToolkit_VERSION_MINOR ${NVCC_OUT}
)
set(
  CUDAToolkit_VERSION "${CUDAToolkit_VERSION_MAJOR}.${CUDAToolkit_VERSION_MINOR}"
  CACHE STRING "Version of CUDA as computed from nvcc."
)
unset(NVCC_OUT)

# CUDAToolkit_ROOT_DIR should have the path to the bin or bin64 folder from the
# find_path calls above.  Failure to find nvcc should have had an early return.
# So now we need to remove bin / bin64, as well as reset the cache entry that
# find_path creates.
string(REGEX REPLACE "[/\\\\]?bin[64]*[/\\\\]?$" "" CUDAToolkit_ROOT_DIR ${CUDAToolkit_ROOT_DIR})
set(CUDAToolkit_ROOT_DIR ${CUDAToolkit_ROOT_DIR} CACHE PATH "Toolkit location." FORCE)

# Now that we have the real ROOT_DIR, find the include/ directory
find_path(CUDAToolkit_INCLUDE_DIRS
  cuda_runtime.h
  # TODO: FindCUDA.cmake has special TARGET_DIR for cross compiling, is that needed?
  PATHS ${CUDAToolkit_ROOT_DIR}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH
)

# And find the CUDA Runtime Library libcudart
find_library(libcudart
  cudart
  PATHS ${CUDAToolkit_ROOT_DIR}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH
)
if (libcudart)
  get_filename_component(CUDAToolkit_LIBRARY_DIR ${libcudart} DIRECTORY ABSOLUTE)
else()
  if (NOT CUDAToolkit_FIND_QUIETLY)
    message(STATUS "Unable to find cudart library under ${CUDAToolkit_ROOT_DIR}/lib[64].")
  endif()
  set(CUDAToolkit_LIBRARY_DIR CUDAToolkit_LIBRARY_DIR-NOTFOUND)
endif()
unset(libcudart)

# Perform version comparison and validate all required variables are set.
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(CUDAToolkit
  REQUIRED_VARS
    CUDAToolkit_ROOT_DIR
    CUDAToolkit_INCLUDE_DIRS
    CUDAToolkit_LIBRARY_DIR
    CUDAToolkit_NVCC_EXECUTABLE
  VERSION_VAR
    CUDAToolkit_VERSION
)
