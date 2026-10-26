set(OpenMP_C_OFFLOAD_FLAGS -DMARK_A)
include(${CMAKE_CURRENT_LIST_DIR}/Offload-common.cmake)

set(OpenMP_C_OFFLOAD OFF)
find_package(OpenMP REQUIRED COMPONENTS C CXX)
