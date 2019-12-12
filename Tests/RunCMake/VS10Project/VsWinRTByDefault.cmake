set(CMAKE_VS_WINRT_BY_DEFAULT true)

enable_language(C)
enable_language(CXX)

add_library(noFlagOnlyC empty.c)
add_library(noFlagMixedCAndCXX empty.c foo.cpp)
add_library(noFlagOnlyCXX foo.cpp)

add_library(flagOnlyC empty.c)
add_library(flagMixedCAndCXX empty.c foo.cpp)
add_library(flagOnlyCXX foo.cpp)

target_compile_options(flagOnlyC PRIVATE /ZW)
target_compile_options(flagMixedCAndCXX PRIVATE /ZW)
target_compile_options(flagOnlyCXX PRIVATE /ZW)
