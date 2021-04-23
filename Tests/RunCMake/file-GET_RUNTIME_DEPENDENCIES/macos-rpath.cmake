enable_language(C)

file(WRITE "${CMAKE_BINARY_DIR}/toplib.c" "extern void sublib1(void);\nextern void sublib2(void);\nvoid toplib(void)\n{\n  sublib1();\n  sublib2();\n}\n")
add_library(toplib SHARED "${CMAKE_BINARY_DIR}/toplib.c")
file(WRITE "${CMAKE_BINARY_DIR}/sublib1.c" "extern void sublib2(void);\nvoid sublib1(void)\n{\n  sublib2();\n}\n")
add_library(sublib1 SHARED "${CMAKE_BINARY_DIR}/sublib1.c")
file(WRITE "${CMAKE_BINARY_DIR}/sublib2.c" "void sublib2(void)\n{\n}\n")
add_library(sublib2 SHARED "${CMAKE_BINARY_DIR}/sublib2.c")
target_link_libraries(toplib PRIVATE sublib1 sublib2)
target_link_libraries(sublib1 PRIVATE sublib2)
set_property(TARGET toplib PROPERTY INSTALL_RPATH "@loader_path/d1;@loader_path/d2")
set_property(TARGET sublib1 PROPERTY INSTALL_RPATH "@loader_path/;@loader_path/../d2")
install(TARGETS toplib DESTINATION lib)
install(TARGETS sublib1 DESTINATION lib/d1)
install(TARGETS sublib2 DESTINATION lib/d2)

install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:toplib>"
    RPATH_PREFIX _rpaths
    )

  set(_expected_rpath "(^|;)@loader_path/;@loader_path/\\.\\./d2$")
  set(_actual_rpath "${_rpaths_${CMAKE_INSTALL_PREFIX}/lib/d1/$<TARGET_FILE_NAME:sublib1>}")
  if(NOT _actual_rpath MATCHES "${_expected_rpath}")
    message(FATAL_ERROR "Expected rpath:\n  ${_expected_rpath}\nActual rpath:\n  ${_actual_rpath}")
  endif()

  # Since RPATH_PREFIX is an undocumented option for install(), we don't really need the rpath
  # for the top files anyway.
  if(DEFINED "_rpaths_${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:toplib>")
    message(FATAL_ERROR "rpath for top library should not be defined")
  endif()
  ]])
