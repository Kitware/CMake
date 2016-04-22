# Synopsis:
# Function for selecting GPU arch flags for nvcc based on CUDA_ARCH_NAME
# Usage:
#   CUDA_SELECT_NVCC_ARCH_FLAGS(out_variable)
#
# Variables affecting the choice:
#
# CUDA_ARCH_NAME: One of CUDA_KNOWN_GPU_ARCH_NAMES list below.
# ENV{CUDA_ARCH_BIN} : Only tested if CUDA_ARCH_NAME set to "Manual".
#                      CUDA_ARCH_BIN entries have to be from CUDA_KNOWN_GPU_ARCHITECTURES list
#
#

# NVIDIA GPU achitectures:
# https://en.wikipedia.org/wiki/CUDA

set(CUDA_KNOWN_GPU_ARCH_NAMES "Fermi" "Kepler" "Maxwell" "All" "Common" "Manual")

# This list will be used for CUDA_ARCH_NAME = All option
set(CUDA_KNOWN_GPU_ARCHITECTURES  "2.0" "2.1(2.0)" "3.0" "3.5" "5.0")

# This list will be used for CUDA_ARCH_NAME = Common option (enabled by default)
set(CUDA_COMMON_GPU_ARCHITECTURES "3.0" "3.5" "5.0")

if (CUDA_VERSION VERSION_GREATER "6.5")
  list(APPEND CUDA_KNOWN_GPU_ARCHITECTURES "3.2" "3.7" "5.2" "5.3")
  list(APPEND CUDA_COMMON_GPU_ARCHITECTURES "3.7" "5.2")
  list(APPEND CUDA_KNOWN_GPU_ARCH_NAMES "Kepler+Tegra" "Kepler+Tesla" "Maxwell+Tegra")
endif ()
if (CUDA_VERSION VERSION_GREATER "7.5")
  list(APPEND CUDA_KNOWN_GPU_ARCHITECTURES "6.0" "6.2")
  list(APPEND CUDA_COMMON_GPU_ARCHITECTURES "6.0" "6.2")
  list(APPEND CUDA_KNOWN_GPU_ARCH_NAMES "Pascal")
endif ()



################################################################################################
# A function for automatic detection of GPUs installed  (if autodetection is enabled)
# Usage:
#   CUDA_DETECT_INSTALLED_GPUS(OUT_VARIABLE)
#
function(CUDA_DETECT_INSTALLED_GPUS OUT_VARIABLE)
  if(NOT CUDA_GPU_DETECT_OUTPUT)
    set(cufile ${PROJECT_BINARY_DIR}/detect_cuda_archs.cu)

    file(WRITE ${cufile} ""
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

    execute_process(COMMAND "${CUDA_NVCC_EXECUTABLE}" "--run" "${cufile}"
                    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/CMakeFiles/"
                    RESULT_VARIABLE nvcc_res OUTPUT_VARIABLE nvcc_out
                    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(nvcc_res EQUAL 0)
      string(REPLACE "2.1" "2.1(2.0)" nvcc_out "${nvcc_out}")
      set(CUDA_GPU_DETECT_OUTPUT ${nvcc_out} CACHE INTERNAL "Returned GPU architetures from detect_gpus tool" FORCE)
    endif()
  endif()

  if(NOT CUDA_GPU_DETECT_OUTPUT)
    message(STATUS "Automatic GPU detection failed. Building for common architectures.")
    set(${OUT_VARIABLE} ${CUDA_COMMON_GPU_ARCHITECTURES} PARENT_SCOPE)
  else()
    set(${OUT_VARIABLE} ${CUDA_GPU_DETECT_OUTPUT} PARENT_SCOPE)
  endif()
endfunction()


################################################################################################
# Function for selecting GPU arch flags for nvcc based on CUDA_ARCH_NAME
# Usage:
#   SELECT_NVCC_ARCH_FLAGS(out_variable)
function(CUDA_SELECT_NVCC_ARCH_FLAGS out_variable)

  set(archs_names ${CUDA_KNOWN_GPU_ARCH_NAMES})

  if(NOT CMAKE_CROSSCOMPILING)
    list(APPEND archs_names "Auto")
    set(archs_name_default "Auto")
  else()
    set(archs_name_default "Manual")
  endif()

  # SET CUDA_ARCH_NAME strings (so it will be seen as dropbox in CMake-Gui)
  set(CUDA_ARCH_NAME ${archs_name_default} CACHE STRING "Select target NVIDIA GPU achitecture.")
  set_property( CACHE CUDA_ARCH_NAME PROPERTY STRINGS "" ${archs_names} )
  mark_as_advanced(CUDA_ARCH_NAME)

  if("${CUDA_ARCH_NAME}" STREQUAL "Manual")
    set(CUDA_ARCH_BIN ${CUDA_KNOWN_GPU_ARCHITECTURES} CACHE STRING "Specify 'real' GPU architectures to build binaries for, BIN(PTX) format is supported")
    set(CUDA_ARCH_PTX "50"                     CACHE STRING "Specify 'virtual' PTX architectures to build PTX intermediate code for")
    mark_as_advanced(CUDA_ARCH_BIN CUDA_ARCH_PTX)
    # Allow a user to specify architecture from env
    if($ENV{CUDA_ARCH_BIN})
      set(CUDA_ARCH_BIN $ENV{CUDA_ARCH_BIN})
      unset(CUDA_ARCH_PTX)
    endif()
  else()
    unset(CUDA_ARCH_BIN CACHE)
    unset(CUDA_ARCH_PTX CACHE)

    if("${CUDA_ARCH_NAME}" STREQUAL "All")
      set(cuda_arch_bin ${CUDA_KNOWN_GPU_ARCHITECTURES})
    elseif("${CUDA_ARCH_NAME}" STREQUAL "Common")
      set(cuda_arch_bin ${CUDA_COMMON_GPU_ARCHITECTURES})
    elseif("${CUDA_ARCH_NAME}" STREQUAL "Auto")
      CUDA_DETECT_INSTALLED_GPUS(cuda_arch_bin)
      set(cuda_arch_bin ${CUDA_ARCH_BIN})
    else()
      set(cuda_arch_bin "")
      list(REMOVE_DUPLICATES )
      list(REMOVE_DUPLICATES CUDA_ARCH_NAME)
      foreach(arch_name ${CUDA_ARCH_NAME})
        if(${arch_name} STREQUAL "Fermi")
          list(APPEND cuda_arch_bin "2.0 2.1(2.0)")
        elseif(${arch_name} STREQUAL "Kepler+Tegra")
          list(APPEND cuda_arch_bin "3.2 3.2")
        elseif(${arch_name} STREQUAL "Kepler+Tesla")
          list(APPEND cuda_arch_bin "3.7 3.7")
        elseif(${arch_name} STREQUAL "Kepler")
          list(APPEND cuda_arch_bin "3.0 3.5")
        elseif(${arch_name} STREQUAL "Maxwell+Tegra")
          list(APPEND cuda_arch_bin "5.3 5.3")
        elseif(${arch_name} STREQUAL "Maxwell")
          list(APPEND cuda_arch_bin "5.0 5.2")
        elseif(${arch_name} STREQUAL "Pascal")
          list(APPEND cuda_arch_bin "6.0 6.2")
        else()
          message(SEND_ERROR "Unknown CUDA Architecture Name in (${CUDA_ARCH_NAME})")
        endif()
      endforeach()
    endif()
  endif()

  message(STATUS "Compiling for CUDA architecture(s): ${cuda_arch_bin}")

  # remove dots and convert to lists
  string(REGEX REPLACE "\\." "" cuda_arch_bin "${cuda_arch_bin}")
  string(REGEX REPLACE "\\." "" cuda_arch_ptx "${CUDA_ARCH_PTX}")
  string(REGEX MATCHALL "[0-9()]+" cuda_arch_bin "${cuda_arch_bin}")
  string(REGEX MATCHALL "[0-9]+"   cuda_arch_ptx "${cuda_arch_ptx}")

  list(REMOVE_DUPLICATES cuda_arch_bin)
  list(REMOVE_DUPLICATES cuda_arch_ptx)

  set(nvcc_flags "")
  set(nvcc_archs_readable "")

  # Tell NVCC to add binaries for the specified GPUs
  foreach(arch ${cuda_arch_bin})
    if(arch MATCHES "([0-9]+)\\(([0-9]+)\\)")
      # User explicitly specified PTX for the concrete BIN
      list(APPEND nvcc_flags -gencode arch=compute_${CMAKE_MATCH_2},code=sm_${CMAKE_MATCH_1})
      list(APPEND nvcc_archs_readable sm_${CMAKE_MATCH_1})
    else()
      # User didn't explicitly specify PTX for the concrete BIN, we assume PTX=BIN
      list(APPEND nvcc_flags -gencode arch=compute_${arch},code=sm_${arch})
      list(APPEND nvcc_archs_readable sm_${arch})
    endif()
  endforeach()

  # Tell NVCC to add PTX intermediate code for the specified architectures
  foreach(arch ${cuda_arch_ptx})
    list(APPEND nvcc_flags -gencode arch=compute_${arch},code=compute_${arch})
    list(APPEND nvcc_archs_readable compute_${arch})
  endforeach()

  string(REPLACE ";" " " nvcc_archs_readable "${nvcc_archs_readable}")
  set(${out_variable}          ${nvcc_flags}          PARENT_SCOPE)
  set(${out_variable}_readable ${nvcc_archs_readable} PARENT_SCOPE)
endfunction()
