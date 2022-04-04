enable_language(CXX)

add_library(tgt STATIC empty.cxx)
target_compile_options(tgt PRIVATE "SHELL:/FI force_include_1.h")
target_compile_options(tgt PRIVATE "/FIforce_include_2.h")
