enable_language(CXX)

add_library(NormalPlatformToolset foo.cpp)
add_library(OverridenPlatformToolset foo.cpp)
set_target_properties(OverridenPlatformToolset
                      PROPERTIES VS_PLATFORM_TOOLSET MyCustomToolset)
