enable_language(C)

set(test1_names path)
set(test2_names path)

file(WRITE "${CMAKE_BINARY_DIR}/path.c" "__declspec(dllexport) void path(void) {}\n")
add_library(path SHARED "${CMAKE_BINARY_DIR}/path.c")

file(REMOVE "${CMAKE_BINARY_DIR}/test1.c")
add_library(test1 SHARED "${CMAKE_BINARY_DIR}/test1.c")
foreach(name ${test1_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "__declspec(dllimport) extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "__declspec(dllexport) void test1(void)\n{\n")
foreach(name ${test1_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "}\n")

target_link_libraries(test1 PRIVATE ${test1_names})

file(REMOVE "${CMAKE_BINARY_DIR}/test2.c")
add_library(test2 SHARED "${CMAKE_BINARY_DIR}/test2.c")
foreach(name ${test2_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "__declspec(dllimport) extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "__declspec(dllexport) void test2(void)\n{\n")
foreach(name ${test2_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "}\n")

target_link_libraries(test2 PRIVATE ${test2_names})

install(TARGETS test1 path DESTINATION lib/test1)
install(TARGETS test2 path DESTINATION lib/test2)

install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/lib/test1/$<TARGET_FILE_NAME:test1>"
      "${CMAKE_INSTALL_PREFIX}/lib/test2/$<TARGET_FILE_NAME:test2>"
    PRE_INCLUDE_REGEXES "^(lib)?path\\.dll$"
    PRE_EXCLUDE_REGEXES ".*"
    )
  message(FATAL_ERROR "This message should not be displayed")
  ]])
