# Borrowed from caffe
# https://github.com/BVLC/caffe/blob/master/cmake/Cuda.cmake

# Synopsis:
# Function for selecting GPU arch flags for nvcc based on CUDA_ARCH_NAME
# Usage:
#   SELECT_NVCC_ARCH_FLAGS(out_variable)
#
# Variables affecting the choice:
#
# CUDA_ARCH_NAME: One of ("Fermi" "Kepler" "Kepler-M" "Maxwell" "Maxwell-M" "All" "Manual")
# ENV{CUDA_ARCH_BIN} : set if CUDA_ARCH_NAME set to "Manual"
#

# Known NVIDIA GPU achitectures
# This list will be used for CUDA_ARCH_NAME = All option

if (NOT CUDA_arch_select_included)

set(KNOWN_GPU_ARCHITECTURES "2.0 2.1(2.0) 3.0 3.2 3.5 5.0 5.2 5.3")

################################################################################################
# Removes duplicates from list(s)
# Usage:
#   LIST_UNIQUE(<list_variable> [<list_variable>] [...])
macro(LIST_UNIQUE)
  foreach(__lst ${ARGN})
    if(${__lst})
      list(REMOVE_DUPLICATES ${__lst})
    endif()
  endforeach()
endmacro()

################################################################################################
# A function for automatic detection of GPUs installed  (IF autodetection is enabled)
# Usage:
#   DETECT_INSTALLED_GPUS(OUT_VARIABLE)
function(DETECT_INSTALLED_GPUS OUT_VARIABLE)
  if(NOT CUDA_GPU_DETECT_OUTPUT)
    set(__cufile ${PROJECT_BINARY_DIR}/detect_cuda_archs.cu)

    file(WRITE ${__cufile} ""
      "#include <cstdio>\n"
      "int main()\n"
      "{\n"
      "  int count = 0;\n"
      "  if (cudaSuccess != cudaGetDeviceCount(&count)) return -1;\n"
      "  if (count == 0) return -1;\n"
      "  for (int device = 0; device < count; ++device)\n"
      "  {\n"
      "    cudaDeviceProp prop;\n"
      "    if (cudaSuccess == cudaGetDeviceProperties(&prop, device))\n"
      "      std::printf(\"%d.%d \", prop.major, prop.minor);\n"
      "  }\n"
      "  return 0;\n"
      "}\n")

    execute_process(COMMAND "${CUDA_NVCC_EXECUTABLE}" "--run" "${__cufile}"
                    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/CMakeFiles/"
                    RESULT_VARIABLE __nvcc_res OUTPUT_VARIABLE __nvcc_out
                    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(__nvcc_res EQUAL 0)
      string(REPLACE "2.1" "2.1(2.0)" __nvcc_out "${__nvcc_out}")
      set(CUDA_GPU_DETECT_OUTPUT ${__nvcc_out} CACHE INTERNAL "Returned GPU architetures from detect_gpus tool" FORCE)
    endif()
  endif()

  if(NOT CUDA_GPU_DETECT_OUTPUT)
    message(STATUS "Automatic GPU detection failed. Building for all known architectures.")
    set(${OUT_VARIABLE} ${KNOWN_GPU_ARCHITECTURES} PARENT_SCOPE)
  else()
    set(${OUT_VARIABLE} ${CUDA_GPU_DETECT_OUTPUT} PARENT_SCOPE)
  endif()
endfunction()


################################################################################################
# Function for selecting GPU arch flags for nvcc based on CUDA_ARCH_NAME
# Usage:
#   SELECT_NVCC_ARCH_FLAGS(out_variable)
function(SELECT_NVCC_ARCH_FLAGS out_variable)
  # List of arch names
# CUDA_ARCH_NAME: One of

  set(__archs_names "Fermi" "Kepler" "Kepler-M" "Maxwell" "Maxwell-M" "All" "Manual")
  set(__archs_name_default "All")
  if(NOT CMAKE_CROSSCOMPILING)
    list(APPEND __archs_names "Auto")
    set(__archs_name_default "Auto")
  endif()

  # SET CUDA_ARCH_NAME strings (so it will be seen as dropbox in CMake-Gui)
  set(CUDA_ARCH_NAME ${__archs_name_default} CACHE STRING "Select target NVIDIA GPU achitecture.")
  SET_property( CACHE CUDA_ARCH_NAME PROPERTY STRINGS "" ${__archs_names} )
  mark_as_advanced(CUDA_ARCH_NAME)

  # verIFy CUDA_ARCH_NAME value
  if(NOT ";${__archs_names};" MATCHES ";${CUDA_ARCH_NAME};")
    string(REPLACE ";" ", " __archs_names "${__archs_names}")
    message(FATAL_ERROR "Only ${__archs_names} architeture names are supported.")
  endif()

  if(${CUDA_ARCH_NAME} STREQUAL "Manual")
    set(CUDA_ARCH_BIN ${KNOWN_GPU_ARCHITECTURES} CACHE STRING "SpecIFy 'real' GPU architectures to build binaries for, BIN(PTX) format is supported")
    set(CUDA_ARCH_PTX "50"                     CACHE STRING "SpecIFy 'virtual' PTX architectures to build PTX intermediate code for")
    mark_as_advanced(CUDA_ARCH_BIN CUDA_ARCH_PTX)
  else()
    unSET(CUDA_ARCH_BIN CACHE)
    unSET(CUDA_ARCH_PTX CACHE)
  endif()

  # Allow a user to specify architecture from env
  if($ENV{CUDA_ARCH_BIN})
    set(CUDA_ARCH_NAME "Manual")
    set(CUDA_ARCH_BIN $ENV{CUDA_ARCH_BIN})
    unSET(CUDA_ARCH_PTX)
  endif()

  if(${CUDA_ARCH_NAME} STREQUAL "Fermi")
    set(__cuda_arch_bin "2.0 2.1(2.0)")
  elseif(${CUDA_ARCH_NAME} STREQUAL "Kepler-M")
    set(__cuda_arch_bin "3.2 3.2")
  elseif(${CUDA_ARCH_NAME} STREQUAL "Kepler")
    set(__cuda_arch_bin "3.0 3.5")
  elseif(${CUDA_ARCH_NAME} STREQUAL "Maxwell-M")
    set(__cuda_arch_bin "5.3 5.3")
  elseif(${CUDA_ARCH_NAME} STREQUAL "Maxwell")
    set(__cuda_arch_bin "5.0 5.2")
  elseif(${CUDA_ARCH_NAME} STREQUAL "All")
    set(__cuda_arch_bin ${KNOWN_GPU_ARCHITECTURES})
  elseif(${CUDA_ARCH_NAME} STREQUAL "Auto")
    DETECT_INSTALLED_GPUS(__cuda_arch_bin)
  else()  # (${CUDA_ARCH_NAME} STREQUAL "Manual")
    set(__cuda_arch_bin ${CUDA_ARCH_BIN})
  endif()

  message(STATUS "Compiling for CUDA architecture: ${__cuda_arch_bin}")

  # remove dots and convert to lists
  string(REGEX REPLACE "\\." "" __cuda_arch_bin "${__cuda_arch_bin}")
  string(REGEX REPLACE "\\." "" __cuda_arch_ptx "${CUDA_ARCH_PTX}")
  string(REGEX MATCHALL "[0-9()]+" __cuda_arch_bin "${__cuda_arch_bin}")
  string(REGEX MATCHALL "[0-9]+"   __cuda_arch_ptx "${__cuda_arch_ptx}")
  LIST_UNIQUE(__cuda_arch_bin __cuda_arch_ptx)

  set(__nvcc_flags "")
  set(__nvcc_archs_readable "")

  # Tell NVCC to add binaries for the specIFied GPUs
  foreach(__arch ${__cuda_arch_bin})
    if(__arch MATCHES "([0-9]+)\\(([0-9]+)\\)")
      # User explicitly specIFied PTX for the concrete BIN
      list(APPEND __nvcc_flags -gencode arch=compute_${CMAKE_MATCH_2},code=sm_${CMAKE_MATCH_1})
      list(APPEND __nvcc_archs_readable sm_${CMAKE_MATCH_1})
    else()
      # User didn't explicitly specIFy PTX for the concrete BIN, we assume PTX=BIN
      list(APPEND __nvcc_flags -gencode arch=compute_${__arch},code=sm_${__arch})
      list(APPEND __nvcc_archs_readable sm_${__arch})
    endif()
  endforeach()

  # Tell NVCC to add PTX intermediate code for the specIFied architectures
  foreach(__arch ${__cuda_arch_ptx})
    list(APPEND __nvcc_flags -gencode arch=compute_${__arch},code=compute_${__arch})
    list(APPEND __nvcc_archs_readable compute_${__arch})
  endforeach()

  string(REPLACE ";" " " __nvcc_archs_readable "${__nvcc_archs_readable}")
  set(${out_variable}          ${__nvcc_flags}          PARENT_SCOPE)
  set(${out_variable}_readable ${__nvcc_archs_readable} PARENT_SCOPE)
endfunction()

SET(CUDA_arch_select_included ON)

endif (NOT CUDA_arch_select_included)
