enable_language(HIP)
message(STATUS "CMAKE_HIP_STANDARD_LATEST='${CMAKE_HIP_STANDARD_LATEST}'")
add_executable(StdLatest StdLatest-HIP.hip)
target_compile_features(StdLatest PRIVATE hip_std_${CMAKE_HIP_STANDARD_LATEST})
