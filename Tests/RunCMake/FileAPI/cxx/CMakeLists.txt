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
target_link_directories(cxx_exe PUBLIC "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/TargetLinkDir>")

target_precompile_headers(cxx_exe PUBLIC ../empty.h)

add_executable(cxx_standard_exe ../empty.cxx)
set_property(TARGET cxx_standard_exe PROPERTY CXX_STANDARD 17)

add_executable(cxx_standard_compile_feature_exe ../empty.cxx)
set_property(TARGET cxx_standard_compile_feature_exe PROPERTY CXX_STANDARD 98)
if(CMAKE_CXX_STANDARD_DEFAULT AND DEFINED CMAKE_CXX11_STANDARD_COMPILE_OPTION)
  target_compile_features(cxx_standard_compile_feature_exe PRIVATE cxx_std_11)
  target_compile_features(cxx_standard_compile_feature_exe PRIVATE cxx_decltype)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cxx_std_11.txt" "")
endif()

set(_rdeps)
if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|Windows|Darwin)$")
  set(_rdeps RUNTIME_DEPENDENCIES)
endif()
install(TARGETS cxx_exe ${_rdeps}
  DESTINATION lib
  FRAMEWORK DESTINATION fw
  )
if(_rdeps)
  install(RUNTIME_DEPENDENCY_SET deps
    DESTINATION lib
    FRAMEWORK DESTINATION fw
    )
endif()

add_executable(cxx_exe_test_launcher ../empty.cxx)
set_property(TARGET cxx_exe_test_launcher PROPERTY TEST_LAUNCHER no-such-launcher)

add_subdirectory(cross)
