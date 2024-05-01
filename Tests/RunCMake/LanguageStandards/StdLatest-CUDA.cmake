enable_language(CUDA)
message(STATUS "CMAKE_CUDA_STANDARD_LATEST='${CMAKE_CUDA_STANDARD_LATEST}'")
add_executable(StdLatest StdLatest-CUDA.cu)
target_compile_features(StdLatest PRIVATE cuda_std_${CMAKE_CUDA_STANDARD_LATEST})
