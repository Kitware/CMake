cmake_minimum_required(VERSION 3.23)

project(BundleLinkBundle CXX)

add_subdirectory(lib_bundle)

add_executable(MainBundle MACOSX_BUNDLE main_bundle.cpp)

target_link_libraries(MainBundle PRIVATE LibBundle)

set_target_properties(MainBundle PROPERTIES
    MACOSX_BUNDLE "YES"
    XCODE_LINK_BUILD_PHASE_MODE BUILT_ONLY
)
