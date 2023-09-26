cmake_minimum_required(VERSION 3.18)
project (MixedLanguage C CXX HIP)

set(CMAKE_HIP_STANDARD 14)
set(CMAKE_CXX_STANDARD 14)

#Goal for this example:
#make sure that we can build multiple languages into targets
#and have the link language always be HIP
add_library(MixedSharedLib SHARED shared.c)
add_library(MixedObjectLib OBJECT shared.cxx shared.hip)
set_target_properties(MixedObjectLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(MixedSharedLib PRIVATE MixedObjectLib)

add_library(MixedStaticLib STATIC static.c static.cxx static.hip)
set_target_properties(MixedStaticLib PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(HIPMixedLanguage main.cxx)
target_link_libraries(HIPMixedLanguage PRIVATE MixedStaticLib MixedSharedLib)

add_executable(HIPMixedLanguageCXX main.cxx)
target_link_libraries(HIPMixedLanguageCXX PRIVATE MixedStaticLib MixedSharedLib)
set_property(TARGET HIPMixedLanguageCXX PROPERTY LINKER_LANGUAGE CXX)
