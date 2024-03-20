enable_language(CXX)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF) # In case C++20 or higher is the latest.
message(STATUS "CMAKE_CXX_STANDARD_LATEST='${CMAKE_CXX_STANDARD_LATEST}'")
add_executable(StdLatest StdLatest-CXX.cxx)
target_compile_features(StdLatest PRIVATE cxx_std_${CMAKE_CXX_STANDARD_LATEST})
