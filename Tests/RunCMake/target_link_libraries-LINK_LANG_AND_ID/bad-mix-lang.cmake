
enable_language(C)
enable_language(CXX)

add_library(static_CXX STATIC func.cxx)

add_executable(LinkLibraries_bad_mix_languages main.c)
target_link_libraries (LinkLibraries_bad_mix_languages PRIVATE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:static_CXX>)
