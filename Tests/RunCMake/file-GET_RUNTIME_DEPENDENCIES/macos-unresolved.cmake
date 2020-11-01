enable_language(C)

file(WRITE "${CMAKE_BINARY_DIR}/testlib.c" "extern void unresolved(void);\nvoid testlib(void)\n{\n  unresolved();\n}\n")
add_library(testlib SHARED "${CMAKE_BINARY_DIR}/testlib.c")
file(WRITE "${CMAKE_BINARY_DIR}/unresolved.c" "void unresolved(void) {}\n")
add_library(unresolved SHARED "${CMAKE_BINARY_DIR}/unresolved.c")
target_link_libraries(testlib PRIVATE unresolved)
install(TARGETS testlib DESTINATION lib)

install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    PRE_INCLUDE_REGEXES "^@rpath/libunresolved\\.dylib$"
    PRE_EXCLUDE_REGEXES ".*"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:testlib>"
    )
  message(FATAL_ERROR "This message should not be displayed")
  ]])
