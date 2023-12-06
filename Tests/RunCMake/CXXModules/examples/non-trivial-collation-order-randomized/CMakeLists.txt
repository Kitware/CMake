cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_non_trivial_collation_order_randomized CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(non_trivial_collation_order_randomized TRUE)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../non-trivial-collation-order/"
                 "${CMAKE_CURRENT_BINARY_DIR}/non-trivial-collation-order")
