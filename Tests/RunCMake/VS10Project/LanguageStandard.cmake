enable_language(C)
enable_language(CXX)

add_library(foo empty.c empty.cxx)
target_compile_features(foo PRIVATE c_std_11 cxx_std_17)
