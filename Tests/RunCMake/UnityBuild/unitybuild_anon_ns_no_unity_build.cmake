project(unitybuild_anon_ns_no_unity_build CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/unitybuild_anon_ns_test_files.cmake)

write_unity_build_anon_ns_test_files(srcs)

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD OFF)

set_target_properties(tgt PROPERTIES UNITY_BUILD_UNIQUE_ID MY_ANON_ID)
