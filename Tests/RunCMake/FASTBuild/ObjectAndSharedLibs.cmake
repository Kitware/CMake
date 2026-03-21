# Setup:
# 1. "foo" shared library links against "-dummy-ext-lib-1" external lib
# and "foo_obj" object library.
# 2. "bar" shared library links against another external lib
# ("-dummy-ext-lib-2") and "bar_object" object library, which, in turn,
# links against "foo" shared library from step 1.
# Test that "foo_obj" stays within "foo" and is not propagated to "bar".

add_library(foo_obj OBJECT some_source_file_1.cpp)

add_library(foo SHARED)
target_link_libraries(foo PUBLIC "-dummy-ext-lib-1" foo_obj)

add_library(bar_obj OBJECT some_source_file_2.cpp)

target_link_libraries(bar_obj PUBLIC foo)

add_library(bar SHARED)
target_link_libraries(bar PUBLIC "-dummy-ext-lib-2" bar_obj)
