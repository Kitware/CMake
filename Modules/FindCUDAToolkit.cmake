# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindCUDAToolkit
---------------

This script locates the NVIDIA CUDA toolkit and the associated libraries, but
does not require the ``CUDA`` language be enabled for a given project. This
module does not search for the NVIDIA CUDA Samples.

Search Behavior
^^^^^^^^^^^^^^^

Finding the CUDA Toolkit requires finding the ``nvcc`` executable, which is
searched for in the following order:

1. If the ``CUDA`` language has been enabled we will use the directory
   containing the compiler as the first search location for ``nvcc``.

2. If the ``CUDAToolkit_ROOT`` cmake configuration variable (e.g.,
   ``-DCUDAToolkit_ROOT=/some/path``) *or* environment variable is defined, it
   will be searched.  If both an environment variable **and** a
   configuration variable are specified, the *configuration* variable takes
   precedence.

   The directory specified here must be such that the executable ``nvcc`` can be
   found underneath the directory specified by ``CUDAToolkit_ROOT``.  If
   ``CUDAToolkit_ROOT`` is specified, but no ``nvcc`` is found underneath, this
   package is marked as **not** found.  No subsequent search attempts are
   performed.

3. If the CUDA_PATH environment variable is defined, it will be searched.

4. The user's path is searched for ``nvcc`` using :command:`find_program`.  If
   this is found, no subsequent search attempts are performed.  Users are
   responsible for ensuring that the first ``nvcc`` to show up in the path is
   the desired path in the event that multiple CUDA Toolkits are installed.

5. On Unix systems, if the symbolic link ``/usr/local/cuda`` exists, this is
   used.  No subsequent search attempts are performed.  No default symbolic link
   location exists for the Windows platform.

6. The platform specific default install locations are searched.  If exactly one
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

Imported targets
^^^^^^^^^^^^^^^^

An :ref:`imported target <Imported targets>` named ``CUDA::toolkit`` is provided.

This module defines :prop_tgt:`IMPORTED` targets for each
of the following libraries that are part of the CUDAToolkit:

- :ref:`CUDA Runtime Library<cuda_toolkit_rt_lib>`
- :ref:`CUDA Driver Library<cuda_toolkit_driver_lib>`
- :ref:`cuBLAS<cuda_toolkit_cuBLAS>`
- :ref:`cuFFT<cuda_toolkit_cuFFT>`
- :ref:`cuRAND<cuda_toolkit_cuRAND>`
- :ref:`cuSOLVER<cuda_toolkit_cuSOLVER>`
- :ref:`cuSPARSE<cuda_toolkit_cuSPARSE>`
- :ref:`cuPTI<cuda_toolkit_cupti>`
- :ref:`NPP<cuda_toolkit_NPP>`
- :ref:`nvBLAS<cuda_toolkit_nvBLAS>`
- :ref:`nvGRAPH<cuda_toolkit_nvGRAPH>`
- :ref:`nvJPEG<cuda_toolkit_nvJPEG>`
- :ref:`nvidia-ML<cuda_toolkit_nvML>`
- :ref:`nvRTC<cuda_toolkit_nvRTC>`
- :ref:`nvToolsExt<cuda_toolkit_nvToolsExt>`
- :ref:`OpenCL<cuda_toolkit_opencl>`
- :ref:`cuLIBOS<cuda_toolkit_cuLIBOS>`

.. _`cuda_toolkit_rt_lib`:

CUDA Runtime Library
""""""""""""""""""""

The CUDA Runtime library (cudart) are what most applications will typically
need to link against to make any calls such as `cudaMalloc`, and `cudaFree`.

Targets Created:

- ``CUDA::cudart``
- ``CUDA::cudart_static``

.. _`cuda_toolkit_driver_lib`:

CUDA Driver Library
""""""""""""""""""""

The CUDA Driver library (cuda) are used by applications that use calls
such as `cuMemAlloc`, and `cuMemFree`. This is generally used by advanced


Targets Created:

- ``CUDA::cuda_driver``
- ``CUDA::cuda_driver``

.. _`cuda_toolkit_cuBLAS`:

cuBLAS
""""""

The `cuBLAS <https://docs.nvidia.com/cuda/cublas/index.html>`_ library.

Targets Created:

- ``CUDA::cublas``
- ``CUDA::cublas_static``

.. _`cuda_toolkit_cuFFT`:

cuFFT
"""""

The `cuFFT <https://docs.nvidia.com/cuda/cufft/index.html>`_ library.

Targets Created:

- ``CUDA::cufft``
- ``CUDA::cufftw``
- ``CUDA::cufft_static``
- ``CUDA::cufftw_static``

cuRAND
""""""

The `cuRAND <https://docs.nvidia.com/cuda/curand/index.html>`_ library.

Targets Created:

- ``CUDA::curand``
- ``CUDA::curand_static``

.. _`cuda_toolkit_cuSOLVER`:

cuSOLVER
""""""""

The `cuSOLVER <https://docs.nvidia.com/cuda/cusolver/index.html>`_ library.

Targets Created:

- ``CUDA::cusolver``
- ``CUDA::cusolver_static``

.. _`cuda_toolkit_cuSPARSE`:

cuSPARSE
""""""""

The `cuSPARSE <https://docs.nvidia.com/cuda/cusparse/index.html>`_ library.

Targets Created:

- ``CUDA::cusparse``
- ``CUDA::cusparse_static``

.. _`cuda_toolkit_cupti`:

cupti
"""""

The `NVIDIA CUDA Profiling Tools Interface <https://developer.nvidia.com/CUPTI>`_.

Targets Created:

- ``CUDA::cupti``
- ``CUDA::cupti_static``

.. _`cuda_toolkit_NPP`:

NPP
"""

The `NPP <https://docs.nvidia.com/cuda/npp/index.html>`_ libraries.

Targets Created:

- `nppc`:

  - ``CUDA::nppc``
  - ``CUDA::nppc_static``

- `nppial`: Arithmetic and logical operation functions in `nppi_arithmetic_and_logical_operations.h`

  - ``CUDA::nppial``
  - ``CUDA::nppial_static``

- `nppicc`: Color conversion and sampling functions in `nppi_color_conversion.h`

  - ``CUDA::nppicc``
  - ``CUDA::nppicc_static``

- `nppicom`: JPEG compression and decompression functions in `nppi_compression_functions.h`

  - ``CUDA::nppicom``
  - ``CUDA::nppicom_static``

- `nppidei`: Data exchange and initialization functions in `nppi_data_exchange_and_initialization.h`

  - ``CUDA::nppidei``
  - ``CUDA::nppidei_static``

- `nppif`: Filtering and computer vision functions in `nppi_filter_functions.h`

  - ``CUDA::nppif``
  - ``CUDA::nppif_static``

- `nppig`: Geometry transformation functions found in `nppi_geometry_transforms.h`

  - ``CUDA::nppig``
  - ``CUDA::nppig_static``

- `nppim`: Morphological operation functions found in `nppi_morphological_operations.h`

  - ``CUDA::nppim``
  - ``CUDA::nppim_static``

- `nppist`: Statistics and linear transform in `nppi_statistics_functions.h` and `nppi_linear_transforms.h`

  - ``CUDA::nppist``
  - ``CUDA::nppist_static``

- `nppisu`: Memory support functions in `nppi_support_functions.h`

  - ``CUDA::nppisu``
  - ``CUDA::nppisu_static``

- `nppitc`: Threshold and compare operation functions in `nppi_threshold_and_compare_operations.h`

  - ``CUDA::nppitc``
  - ``CUDA::nppitc_static``

- `npps`:

  - ``CUDA::npps``
  - ``CUDA::npps_static``

.. _`cuda_toolkit_nvBLAS`:

nvBLAS
""""""

The `nvBLAS <https://docs.nvidia.com/cuda/nvblas/index.html>`_ libraries.
This is a shared library only.

Targets Created:

- ``CUDA::nvblas``

.. _`cuda_toolkit_nvGRAPH`:

nvGRAPH
"""""""

The `nvGRAPH <https://docs.nvidia.com/cuda/nvgraph/index.html>`_ library.

Targets Created:

- ``CUDA::nvgraph``
- ``CUDA::nvgraph_static``


.. _`cuda_toolkit_nvJPEG`:

nvJPEG
""""""

The `nvJPEG <https://docs.nvidia.com/cuda/nvjpeg/index.html>`_ library.
Introduced in CUDA 10.

Targets Created:

- ``CUDA::nvjpeg``
- ``CUDA::nvjpeg_static``

.. _`cuda_toolkit_nvRTC`:

nvRTC
"""""

The `nvRTC <https://docs.nvidia.com/cuda/nvrtc/index.html>`_ (Runtime Compilation) library.
This is a shared library only.

Targets Created:

- ``CUDA::nvrtc``

.. _`cuda_toolkit_nvml`:

nvidia-ML
"""""""""

The `NVIDIA Management Library <https://developer.nvidia.com/nvidia-management-library-nvml>`_.
This is a shared library only.

Targets Created:

- ``CUDA::nvml``

.. _`cuda_toolkit_nvToolsExt`:

nvToolsExt
""""""""""

The `NVIDIA Tools Extension <https://docs.nvidia.com/gameworks/content/gameworkslibrary/nvtx/nvidia_tools_extension_library_nvtx.htm>`_.
This is a shared library only.

Targets Created:

- ``CUDA::nvToolsExt``

.. _`cuda_toolkit_opencl`:

OpenCL
""""""

The `NVIDIA OpenCL Library <https://developer.nvidia.com/opencl>`_.
This is a shared library only.

Targets Created:

- ``CUDA::OpenCL``

.. _`cuda_toolkit_cuLIBOS`:

cuLIBOS
"""""""

The cuLIBOS library is a backend thread abstraction layer library which is
static only.  The ``CUDA::cublas_static``, ``CUDA::cusparse_static``,
``CUDA::cufft_static``, ``CUDA::curand_static``, and (when implemented) NPP
libraries all automatically have this dependency linked.

Target Created:

- ``CUDA::culibos``

**Note**: direct usage of this target by consumers should not be necessary.

.. _`cuda_toolkit_cuRAND`:



Result variables
^^^^^^^^^^^^^^^^

``CUDAToolkit_FOUND``
    A boolean specifying whether or not the CUDA Toolkit was found.

``CUDAToolkit_VERSION``
    The exact version of the CUDA Toolkit found (as reported by
    ``nvcc --version``).

``CUDAToolkit_VERSION_MAJOR``
    The major version of the CUDA Toolkit.

``CUDAToolkit_VERSION_MAJOR``
    The minor version of the CUDA Toolkit.

``CUDAToolkit_VERSION_PATCH``
    The patch version of the CUDA Toolkit.

``CUDAToolkit_BIN_DIR``
    The path to the CUDA Toolkit library directory that contains the CUDA
    executable ``nvcc``.

``CUDAToolkit_INCLUDE_DIRS``
    The path to the CUDA Toolkit ``include`` folder containing the header files
    required to compile a project linking against CUDA.

``CUDAToolkit_LIBRARY_DIR``
    The path to the CUDA Toolkit library directory that contains the CUDA
    Runtime library ``cudart``.

``CUDAToolkit_TARGET_DIR``
    The path to the CUDA Toolkit directory including the target architecture
    when cross-compiling. When not cross-compiling this will be equivalant to
    ``CUDAToolkit_ROOT_DIR``.

``CUDAToolkit_NVCC_EXECUTABLE``
    The path to the NVIDIA CUDA compiler ``nvcc``.  Note that this path may
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

# Include shared CUDA toolkit location code.
include(Internal/CUDAToolkit)

if(CUDAToolkit_NVCC_EXECUTABLE AND
   CUDAToolkit_NVCC_EXECUTABLE STREQUAL CMAKE_CUDA_COMPILER)
  # Need to set these based off the already computed CMAKE_CUDA_COMPILER_VERSION value
  # This if statement will always match, but is used to provide variables for MATCH 1,2,3...
  if(CMAKE_CUDA_COMPILER_VERSION MATCHES [=[([0-9]+)\.([0-9]+)\.([0-9]+)]=])
    set(CUDAToolkit_VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(CUDAToolkit_VERSION_MINOR "${CMAKE_MATCH_2}")
    set(CUDAToolkit_VERSION_PATCH "${CMAKE_MATCH_3}")
    set(CUDAToolkit_VERSION "${CMAKE_CUDA_COMPILER_VERSION}")
  endif()
else()
  # Compute the version by invoking nvcc
  execute_process (COMMAND ${CUDAToolkit_NVCC_EXECUTABLE} "--version" OUTPUT_VARIABLE NVCC_OUT)
  if(NVCC_OUT MATCHES [=[ V([0-9]+)\.([0-9]+)\.([0-9]+)]=])
    set(CUDAToolkit_VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(CUDAToolkit_VERSION_MINOR "${CMAKE_MATCH_2}")
    set(CUDAToolkit_VERSION_PATCH "${CMAKE_MATCH_3}")
    set(CUDAToolkit_VERSION  "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
  endif()
  unset(NVCC_OUT)
endif()

if(NOT CUDA_CUDART AND NOT CUDAToolkit_FIND_QUIETLY)
  message(STATUS "Unable to find cudart library.")
endif()

# Find the CUDA Runtime Library libcudart
find_library(CUDA_CUDART
  NAMES cudart
  PATH_SUFFIXES lib64 lib/x64
)
if(NOT CUDA_CUDART)
  find_library(CUDA_CUDART
    NAMES cudart
    PATH_SUFFIXES lib64/stubs lib/x64/stubs
  )
endif()

unset(CUDAToolkit_ROOT_DIR)
if(_CUDAToolkit_Pop_Prefix)
  list(REMOVE_AT CMAKE_PREFIX_PATH -1)
  unset(_CUDAToolkit_Pop_Prefix)
endif()

#-----------------------------------------------------------------------------
# Perform version comparison and validate all required variables are set.
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(CUDAToolkit
  REQUIRED_VARS
    CUDAToolkit_INCLUDE_DIR
    CUDA_CUDART
    CUDAToolkit_NVCC_EXECUTABLE
  VERSION_VAR
    CUDAToolkit_VERSION
)
mark_as_advanced(CUDA_CUDART
                 CUDAToolkit_INCLUDE_DIR
                 CUDAToolkit_NVCC_EXECUTABLE
                 )

#-----------------------------------------------------------------------------
# Construct result variables
if(CUDAToolkit_FOUND)
  set(CUDAToolkit_INCLUDE_DIRS ${CUDAToolkit_INCLUDE_DIR})
  get_filename_component(CUDAToolkit_LIBRARY_DIR ${CUDA_CUDART} DIRECTORY ABSOLUTE)
endif()

#-----------------------------------------------------------------------------
# Construct import targets
if(CUDAToolkit_FOUND)

  function(_CUDAToolkit_find_and_add_import_lib lib_name)
    cmake_parse_arguments(arg "" "" "ALT;DEPS;EXTRA_PATH_SUFFIXES" ${ARGN})

    set(search_names ${lib_name} ${arg_ALT})

    find_library(CUDA_${lib_name}_LIBRARY
      NAMES ${search_names}
      HINTS ${CUDAToolkit_LIBRARY_DIR}
            ENV CUDA_PATH
      PATH_SUFFIXES nvidia/current lib64 lib/x64 lib
                    ${arg_EXTRA_PATH_SUFFIXES}
    )
    # Don't try any stub directories intil we have exhausted all other
    # search locations.
    if(NOT CUDA_${lib_name}_LIBRARY)
      find_library(CUDA_${lib_name}_LIBRARY
        NAMES ${search_names}
        HINTS ${CUDAToolkit_LIBRARY_DIR}
              ENV CUDA_PATH
        PATH_SUFFIXES lib64/stubs lib/x64/stubs lib/stubs stubs
      )
    endif()

    mark_as_advanced(CUDA_${lib_name}_LIBRARY)

    if (NOT TARGET CUDA::${lib_name} AND CUDA_${lib_name}_LIBRARY)
      add_library(CUDA::${lib_name} IMPORTED INTERFACE)
      target_include_directories(CUDA::${lib_name} SYSTEM INTERFACE "${CUDAToolkit_INCLUDE_DIRS}")
      target_link_libraries(CUDA::${lib_name} INTERFACE "${CUDA_${lib_name}_LIBRARY}")
      foreach(dep ${arg_DEPS})
        if(TARGET CUDA::${dep})
          target_link_libraries(CUDA::${lib_name} INTERFACE CUDA::${dep})
        endif()
      endforeach()
    endif()
  endfunction()

  if(NOT TARGET CUDA::toolkit)
    add_library(CUDA::toolkit IMPORTED INTERFACE)
    target_include_directories(CUDA::toolkit SYSTEM INTERFACE "${CUDAToolkit_INCLUDE_DIRS}")
    target_link_directories(CUDA::toolkit INTERFACE "${CUDAToolkit_LIBRARY_DIR}")
  endif()

  _CUDAToolkit_find_and_add_import_lib(cuda_driver ALT cuda)

  _CUDAToolkit_find_and_add_import_lib(cudart)
  _CUDAToolkit_find_and_add_import_lib(cudart_static)

  # setup dependencies that are required for cudart_static when building
  # on linux. These are generally only required when using the CUDA toolkit
  # when CUDA language is disabled
  if(NOT TARGET CUDA::cudart_static_deps
     AND TARGET CUDA::cudart_static)

    add_library(CUDA::cudart_static_deps IMPORTED INTERFACE)
    target_link_libraries(CUDA::cudart_static INTERFACE CUDA::cudart_static_deps)

    if(UNIX AND (CMAKE_C_COMPILER OR CMAKE_CXX_COMPILER))
      find_package(Threads REQUIRED)
      target_link_libraries(CUDA::cudart_static_deps INTERFACE Threads::Threads ${CMAKE_DL_LIBS})
    endif()

    if(UNIX AND NOT APPLE)
      # On Linux, you must link against librt when using the static cuda runtime.
      find_library(CUDAToolkit_rt_LIBRARY rt)
      mark_as_advanced(CUDAToolkit_rt_LIBRARY)
      if(NOT CUDAToolkit_rt_LIBRARY)
        message(WARNING "Could not find librt library, needed by CUDA::cudart_static")
      else()
        target_link_libraries(CUDA::cudart_static_deps INTERFACE ${CUDAToolkit_rt_LIBRARY})
      endif()
    endif()
  endif()

  _CUDAToolkit_find_and_add_import_lib(culibos) # it's a static library
  foreach (cuda_lib cublas cufft curand cusparse nppc nvjpeg)
    _CUDAToolkit_find_and_add_import_lib(${cuda_lib})
    _CUDAToolkit_find_and_add_import_lib(${cuda_lib}_static DEPS culibos)
  endforeach()

  # cuFFTW depends on cuFFT
  _CUDAToolkit_find_and_add_import_lib(cufftw DEPS cufft)
  _CUDAToolkit_find_and_add_import_lib(cufftw DEPS cufft_static)

  # cuSOLVER depends on cuBLAS, and cuSPARSE
  _CUDAToolkit_find_and_add_import_lib(cusolver DEPS cublas cusparse)
  _CUDAToolkit_find_and_add_import_lib(cusolver_static DEPS cublas_static cusparse_static culibos)

  # nvGRAPH depends on cuRAND, and cuSOLVER.
  _CUDAToolkit_find_and_add_import_lib(nvgraph DEPS curand cusolver)
  _CUDAToolkit_find_and_add_import_lib(nvgraph_static DEPS curand_static cusolver_static)

  # Process the majority of the NPP libraries.
  foreach (cuda_lib nppial nppicc nppidei nppif nppig nppim nppist nppitc npps nppicom nppisu)
    _CUDAToolkit_find_and_add_import_lib(${cuda_lib} DEPS nppc)
    _CUDAToolkit_find_and_add_import_lib(${cuda_lib}_static DEPS nppc_static)
  endforeach()

  _CUDAToolkit_find_and_add_import_lib(cupti
                                       EXTRA_PATH_SUFFIXES ../extras/CUPTI/lib64/
                                                           ../extras/CUPTI/lib/)
  _CUDAToolkit_find_and_add_import_lib(cupti_static
                                       EXTRA_PATH_SUFFIXES ../extras/CUPTI/lib64/
                                                           ../extras/CUPTI/lib/)

  _CUDAToolkit_find_and_add_import_lib(nvrtc DEPS cuda_driver)

  _CUDAToolkit_find_and_add_import_lib(nvml ALT nvidia-ml nvml)

  if(WIN32)
    # nvtools can be installed outside the CUDA toolkit directory
    # so prefer the NVTOOLSEXT_PATH windows only environment variable
    # In addition on windows the most common name is nvToolsExt64_1
    find_library(CUDA_nvToolsExt_LIBRARY
      NAMES nvToolsExt64_1 nvToolsExt64 nvToolsExt
      PATHS ENV NVTOOLSEXT_PATH
            ENV CUDA_PATH
      PATH_SUFFIXES lib/x64 lib
    )
  endif()
  _CUDAToolkit_find_and_add_import_lib(nvToolsExt ALT nvToolsExt64)

  _CUDAToolkit_find_and_add_import_lib(OpenCL)
endif()

if(_CUDAToolkit_Pop_ROOT_PATH)
  list(REMOVE_AT CMAKE_FIND_ROOT_PATH 0)
  unset(_CUDAToolkit_Pop_ROOT_PATH)
endif()
