enable_language(C)

set(test1_names rpath)
set(test2_names rpath)

file(WRITE "${CMAKE_BINARY_DIR}/rpath.c" "void rpath(void) {}\n")
add_library(rpath SHARED "${CMAKE_BINARY_DIR}/rpath.c")
install(TARGETS rpath DESTINATION lib/rpath1)
install(TARGETS rpath DESTINATION lib/rpath2)

file(REMOVE "${CMAKE_BINARY_DIR}/test1.c")
add_library(test1 SHARED "${CMAKE_BINARY_DIR}/test1.c")
foreach(name ${test1_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "void test1(void)\n{\n")
foreach(name ${test1_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test1.c" "}\n")

target_link_libraries(test1 PRIVATE ${test1_names})
set_property(TARGET test1 PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/lib/rpath1"
  )

file(REMOVE "${CMAKE_BINARY_DIR}/test2.c")
add_library(test2 SHARED "${CMAKE_BINARY_DIR}/test2.c")
foreach(name ${test2_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "void test2(void)\n{\n")
foreach(name ${test2_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test2.c" "}\n")

target_link_libraries(test2 PRIVATE ${test2_names})
set_property(TARGET test2 PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/lib/rpath2"
  )

install(TARGETS test1 test2 DESTINATION lib)

install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:test1>"
      "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:test2>"
    PRE_INCLUDE_REGEXES "^librpath\\.so$"
    PRE_EXCLUDE_REGEXES ".*"
    )
  message(FATAL_ERROR "This message should not be displayed")
  ]])
