project(unitybuild_anon_ns_group_mode CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/unitybuild_anon_ns_test_files.cmake)

write_unity_build_anon_ns_test_files(srcs)

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                                     UNITY_BUILD_MODE GROUP)

set_target_properties(tgt PROPERTIES UNITY_BUILD_UNIQUE_ID MY_ANON_ID)

set_source_files_properties(s1.cpp s2.cpp s5.cpp s7.cpp s8.cpp
                            PROPERTIES UNITY_GROUP "a"
                            )
set_source_files_properties(s3.cpp s4.cpp s6.cpp
                            PROPERTIES UNITY_GROUP "b"
                            )
