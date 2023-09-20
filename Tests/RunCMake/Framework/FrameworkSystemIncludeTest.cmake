enable_language(C)

add_library(Example::Example SHARED IMPORTED)
set_target_properties(Example::Example PROPERTIES
  FRAMEWORK 1
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework/Example.tbd"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework;${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework/Headers"
)

add_library(testcase FrameworkSystemIncludeTest.c)
target_compile_options(testcase PRIVATE "$<IF:$<C_COMPILER_ID:GNU>,-Werror,-Werror=#pragma-messages>")
target_link_libraries(testcase PRIVATE Example::Example)



add_library(Example::Example2 SHARED IMPORTED)
set_target_properties(Example::Example2 PROPERTIES
  FRAMEWORK 1
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework/Example.tbd"
)

add_library(testcase2 FrameworkSystemIncludeTest.c)
target_compile_options(testcase2 PRIVATE "$<IF:$<C_COMPILER_ID:GNU>,-Werror,-Werror=#pragma-messages>")
target_link_libraries(testcase2 PRIVATE Example::Example2)



add_library(Example::Example3 SHARED IMPORTED)
set_target_properties(Example::Example3 PROPERTIES
  FRAMEWORK 1
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/subdir/Example.framework"
)

add_library(testcase3 FrameworkSystemIncludeTest.c)
target_compile_options(testcase3 PRIVATE "$<IF:$<C_COMPILER_ID:GNU>,-Werror,-Werror=#pragma-messages>")
target_link_libraries(testcase3 PRIVATE Example::Example3)
