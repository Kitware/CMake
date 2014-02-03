cmake_minimum_required(VERSION 2.8)

project(VisibilityInlinesHidden)

add_library(inlines_hidden SHARED foo.cpp bar.c)
set_property(TARGET inlines_hidden PROPERTY VISIBILITY_INLINES_HIDDEN ON)
target_compile_options(inlines_hidden PRIVATE -Werror)

add_custom_command(TARGET inlines_hidden POST_BUILD
  COMMAND ${CMAKE_COMMAND}
    -DCMAKE_NM=${CMAKE_NM}
    -DTEST_LIBRARY_PATH=$<TARGET_FILE:inlines_hidden>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/verify.cmake
)
