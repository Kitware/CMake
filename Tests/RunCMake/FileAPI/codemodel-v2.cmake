enable_language(C)

include("${CMAKE_CURRENT_LIST_DIR}/include_test.cmake")

add_library(c_lib empty.c)
add_executable(c_exe empty.c)
target_link_libraries(c_exe PRIVATE c_lib)

add_library(c_shared_lib SHARED empty.c)
add_executable(c_shared_exe empty.c)
target_link_libraries(c_shared_exe PRIVATE c_shared_lib)

add_library(c_static_lib STATIC empty.c)
add_executable(c_static_exe empty.c)
target_link_libraries(c_static_exe PRIVATE c_static_lib)

add_subdirectory(cxx)
add_subdirectory(alias)
add_subdirectory(object)
add_subdirectory(imported)
add_subdirectory(custom)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../FileAPIExternalSource" "${CMAKE_CURRENT_BINARY_DIR}/../FileAPIExternalBuild")
add_subdirectory(dir)

set_property(TARGET c_shared_lib PROPERTY LIBRARY_OUTPUT_DIRECTORY lib)
set_property(TARGET c_shared_lib PROPERTY RUNTIME_OUTPUT_DIRECTORY lib)

include(CheckIPOSupported)
check_ipo_supported(RESULT _ipo LANGUAGES C)
if(_ipo)
  set_property(TARGET c_shared_lib PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
  set_property(TARGET c_shared_exe PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
  set_property(TARGET c_static_lib PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
  file(WRITE "${CMAKE_BINARY_DIR}/ipo_enabled.txt" "")
endif()

install(TARGETS cxx_exe)
