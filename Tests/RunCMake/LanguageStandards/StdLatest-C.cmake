enable_language(C)
message(STATUS "CMAKE_C_STANDARD_LATEST='${CMAKE_C_STANDARD_LATEST}'")
add_executable(StdLatest StdLatest-C.c)
target_compile_features(StdLatest PRIVATE c_std_${CMAKE_C_STANDARD_LATEST})
