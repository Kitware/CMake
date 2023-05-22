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

add_library(c_subdir STATIC)
add_subdirectory(subdir)

add_subdirectory(cxx)
add_subdirectory(alias)
add_subdirectory(object)
add_subdirectory(imported)
add_subdirectory(interface)
add_subdirectory(custom)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../FileAPIExternalSource" "${CMAKE_CURRENT_BINARY_DIR}/../FileAPIExternalBuild")
add_subdirectory(dir)
add_subdirectory(fileset)
add_subdirectory(framework)

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

install(TARGETS cxx_exe COMPONENT Tools EXPORT FooTargets)

set_target_properties(c_shared_lib PROPERTIES VERSION 1.2.3 SOVERSION 1)
install(TARGETS c_shared_lib cxx_shared_lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib
  LIBRARY DESTINATION lib NAMELINK_SKIP
  )
install(TARGETS c_shared_lib cxx_shared_lib LIBRARY NAMELINK_ONLY)

install(FILES empty.h TYPE INCLUDE RENAME empty-renamed.h OPTIONAL)
install(FILES codemodel-v2.cmake empty.h DESTINATION include)
install(DIRECTORY . dir cxx/ OPTIONAL DESTINATION dir1)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/dir" "${CMAKE_CURRENT_SOURCE_DIR}/cxx/" DESTINATION dir2)
install(EXPORT FooTargets DESTINATION lib/cmake/foo)
install(SCRIPT InstallScript.cmake)
install(CODE "message(foo)" ALL_COMPONENTS)
