# Borrowed from caffe
# https://github.com/BVLC/caffe/blob/master/cmake/Cuda.cmake

# Known NVIDIA GPU achitectures
# This list will be used for CUDA_ARCH_NAME = All option
SET(KNOWN_GPU_ARCHITECTURES "2.0 2.1(2.0) 3.0 3.2 3.5 5.0 5.2 5.3")

################################################################################################
# Removes duplicates from LIST(s)
# Usage:
#   LIST_UNIQUE(<list_variable> [<list_variable>] [...])
MACRO(LIST_UNIQUE)
  FOREACH(__lst ${ARGN})
    IF(${__lst})
      LIST(REMOVE_DUPLICATES ${__lst})
    ENDIF()
  ENDFOREACH()
ENDMACRO()

################################################################################################
# A function for automatic detection of GPUs installed  (IF autodetection is enabled)
# Usage:
#   DETECT_INSTALLED_GPUS(OUT_VARIABLE)
FUNCTION(DETECT_INSTALLED_GPUS OUT_VARIABLE)
  IF(NOT CUDA_GPU_DETECT_OUTPUT)
    SET(__cufile ${PROJECT_BINARY_DIR}/detect_cuda_archs.cu)

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

    EXECUTE_PROCESS(COMMAND "${CUDA_NVCC_EXECUTABLE}" "--run" "${__cufile}"
                    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/CMakeFiles/"
                    RESULT_VARIABLE __nvcc_res OUTPUT_VARIABLE __nvcc_out
                    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(__nvcc_res EQUAL 0)
      STRING(REPLACE "2.1" "2.1(2.0)" __nvcc_out "${__nvcc_out}")
      SET(CUDA_GPU_DETECT_OUTPUT ${__nvcc_out} CACHE INTERNAL "Returned GPU architetures from detect_gpus tool" FORCE)
    ENDIF()
  ENDIF()

  IF(NOT CUDA_GPU_DETECT_OUTPUT)
    message(STATUS "Automatic GPU detection failed. Building for all known architectures.")
    SET(${OUT_VARIABLE} ${KNOWN_GPU_ARCHITECTURES} PARENT_SCOPE)
  ELSE()
    SET(${OUT_VARIABLE} ${CUDA_GPU_DETECT_OUTPUT} PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()


################################################################################################
# Function for selecting GPU arch flags for nvcc based on CUDA_ARCH_NAME
# Usage:
#   SELECT_NVCC_ARCH_FLAGS(out_variable)
FUNCTION(SELECT_NVCC_ARCH_FLAGS out_variable)
  # List of arch names
  SET(__archs_names "Fermi" "Kepler" "Maxwell" "All" "Manual")
  SET(__archs_name_default "All")
  IF(NOT CMAKE_CROSSCOMPILING)
    LIST(APPEND __archs_names "Auto")
    SET(__archs_name_default "Auto")
  ENDIF()

  # SET CUDA_ARCH_NAME strings (so it will be seen as dropbox in CMake-Gui)
  SET(CUDA_ARCH_NAME ${__archs_name_default} CACHE STRING "Select target NVIDIA GPU achitecture.")
  SET_property( CACHE CUDA_ARCH_NAME PROPERTY STRINGS "" ${__archs_names} )
  mark_as_advanced(CUDA_ARCH_NAME)

  # verIFy CUDA_ARCH_NAME value
  IF(NOT ";${__archs_names};" MATCHES ";${CUDA_ARCH_NAME};")
    STRING(REPLACE ";" ", " __archs_names "${__archs_names}")
    message(FATAL_ERROR "Only ${__archs_names} architeture names are supported.")
  ENDIF()

  IF(${CUDA_ARCH_NAME} STREQUAL "Manual")
    SET(CUDA_ARCH_BIN ${KNOWN_GPU_ARCHITECTURES} CACHE STRING "SpecIFy 'real' GPU architectures to build binaries for, BIN(PTX) format is supported")
    SET(CUDA_ARCH_PTX "50"                     CACHE STRING "SpecIFy 'virtual' PTX architectures to build PTX intermediate code for")
    mark_as_advanced(CUDA_ARCH_BIN CUDA_ARCH_PTX)
  else()
    unSET(CUDA_ARCH_BIN CACHE)
    unSET(CUDA_ARCH_PTX CACHE)
  ENDIF()

  # Allow a user to specify architecture from env
  IF($ENV{CUDA_ARCH_BIN})
    SET(CUDA_ARCH_NAME "Manual")
    SET(CUDA_ARCH_BIN $ENV{CUDA_ARCH_BIN})
    unSET(CUDA_ARCH_PTX)
  ENDIF()

  IF(${CUDA_ARCH_NAME} STREQUAL "Fermi")
    SET(__cuda_arch_bin "2.0 2.1(2.0)")
  elseIF(${CUDA_ARCH_NAME} STREQUAL "Kepler")
    SET(__cuda_arch_bin "3.0 3.5")
  elseIF(${CUDA_ARCH_NAME} STREQUAL "Maxwell")
    SET(__cuda_arch_bin "5.0 5.2")
  elseIF(${CUDA_ARCH_NAME} STREQUAL "All")
    SET(__cuda_arch_bin ${KNOWN_GPU_ARCHITECTURES})
  elseIF(${CUDA_ARCH_NAME} STREQUAL "Auto")
    DETECT_INSTALLED_GPUS(__cuda_arch_bin)
  else()  # (${CUDA_ARCH_NAME} STREQUAL "Manual")
    SET(__cuda_arch_bin ${CUDA_ARCH_BIN})
  ENDIF()

  MESSAGE(STATUS "Compiling for CUDA architecture: ${__cuda_arch_bin}")

  # remove dots and convert to lists
  STRING(REGEX REPLACE "\\." "" __cuda_arch_bin "${__cuda_arch_bin}")
  STRING(REGEX REPLACE "\\." "" __cuda_arch_ptx "${CUDA_ARCH_PTX}")
  STRING(REGEX MATCHALL "[0-9()]+" __cuda_arch_bin "${__cuda_arch_bin}")
  STRING(REGEX MATCHALL "[0-9]+"   __cuda_arch_ptx "${__cuda_arch_ptx}")
  LIST_UNIQUE(__cuda_arch_bin __cuda_arch_ptx)

  SET(__nvcc_flags "")
  SET(__nvcc_archs_readable "")

  # Tell NVCC to add binaries for the specIFied GPUs
  FOREACH(__arch ${__cuda_arch_bin})
    IF(__arch MATCHES "([0-9]+)\\(([0-9]+)\\)")
      # User explicitly specIFied PTX for the concrete BIN
      LIST(APPEND __nvcc_flags -gencode arch=compute_${CMAKE_MATCH_2},code=sm_${CMAKE_MATCH_1})
      LIST(APPEND __nvcc_archs_readable sm_${CMAKE_MATCH_1})
    else()
      # User didn't explicitly specIFy PTX for the concrete BIN, we assume PTX=BIN
      LIST(APPEND __nvcc_flags -gencode arch=compute_${__arch},code=sm_${__arch})
      LIST(APPEND __nvcc_archs_readable sm_${__arch})
    ENDIF()
  ENDFOREACH()

  # Tell NVCC to add PTX intermediate code for the specIFied architectures
  FOREACH(__arch ${__cuda_arch_ptx})
    LIST(APPEND __nvcc_flags -gencode arch=compute_${__arch},code=compute_${__arch})
    LIST(APPEND __nvcc_archs_readable compute_${__arch})
  ENDFOREACH()

  STRING(REPLACE ";" " " __nvcc_archs_readable "${__nvcc_archs_readable}")
  SET(${out_variable}          ${__nvcc_flags}          PARENT_SCOPE)
  SET(${out_variable}_readable ${__nvcc_archs_readable} PARENT_SCOPE)
ENDFUNCTION()
