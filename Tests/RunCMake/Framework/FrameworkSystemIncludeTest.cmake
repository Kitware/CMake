enable_language(C)

add_library(Example::Example SHARED IMPORTED)
set_target_properties(Example::Example PROPERTIES
  FRAMEWORK 1
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework/Example.tbd"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework;${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework/Headers"
)

add_library(testcase FrameworkSystemIncludeTest.c)
target_compile_options(testcase PRIVATE "-Werror=#pragma-messages")
target_link_libraries(testcase PRIVATE Example::Example)
