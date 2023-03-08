cmake_minimum_required(VERSION 3.5)
project(TestFindGnuTLS C)
include(CTest)

find_package(GnuTLS REQUIRED)

add_definitions(-DCMAKE_EXPECTED_GNUTLS_VERSION="${GNUTLS_VERSION}")

add_executable(test_tgt main.c)
target_link_libraries(test_tgt GnuTLS::GnuTLS)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${GNUTLS_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${GNUTLS_LIBRARIES})
target_compile_definitions(test_var PRIVATE ${GNUTLS_DEFINITIONS})
add_test(NAME test_var COMMAND test_var)
