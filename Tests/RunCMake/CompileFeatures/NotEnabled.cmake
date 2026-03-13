add_library(empty empty.cpp)
target_compile_definitions(empty PRIVATE FOO=$<COMPILE_FEATURES:c_std_99>)
