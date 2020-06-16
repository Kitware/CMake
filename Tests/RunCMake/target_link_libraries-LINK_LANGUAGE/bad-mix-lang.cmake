
enable_language(C)
enable_language(CXX)

add_library(static_CXX STATIC func.cxx)

add_executable(LinkLibraries_bad_mix_languages main.c)
target_link_libraries (LinkLibraries_bad_mix_languages PRIVATE $<$<LINK_LANGUAGE:C>:static_CXX>)
