enable_language(SYCL)
add_library(sycl_lib STATIC src.sycl)
target_include_directories(sycl_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
