project(Cxx)
enable_language(CXX)

add_library(cxx_lib ../empty.cxx)
add_executable(cxx_exe ../empty.cxx)
target_link_libraries(cxx_exe PRIVATE cxx_lib)
set_property(TARGET cxx_exe PROPERTY FOLDER bin)

add_library(cxx_shared_lib SHARED ../empty.cxx)
add_executable(cxx_shared_exe ../empty.cxx)
target_link_libraries(cxx_shared_exe PRIVATE cxx_shared_lib)

add_library(cxx_static_lib STATIC ../empty.cxx)
add_executable(cxx_static_exe ../empty.cxx)
target_link_libraries(cxx_static_exe PRIVATE cxx_static_lib)

target_compile_options(cxx_exe PUBLIC TargetCompileOptions)
target_link_options(cxx_exe PUBLIC TargetLinkOptions)
target_link_directories(cxx_exe PUBLIC "${CMAKE_BINARY_DIR}/TargetLinkDir")
