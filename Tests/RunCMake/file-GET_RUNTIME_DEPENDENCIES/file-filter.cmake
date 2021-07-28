enable_language(C)

set(dep_list)
set(import_list)
set(call_list)
foreach(i 1 2 3 4 5 6 7 8 9)
  file(WRITE "${CMAKE_BINARY_DIR}/dep${i}.c"
"#ifdef _WIN32
__declspec(dllexport)
#endif
  void dep${i}(void)
{
}
")
  add_library(dep${i} SHARED "${CMAKE_BINARY_DIR}/dep${i}.c")
  list(APPEND dep_list dep${i})
  string(APPEND import_list "EXE_IMPORT extern void dep${i}(void);\n")
  string(APPEND call_list "  dep${i}();\n")
endforeach()
set_target_properties(dep5 PROPERTIES
  VERSION 1.2.3
  SOVERSION 1
  )

file(WRITE "${CMAKE_BINARY_DIR}/main.c"
"#ifdef _WIN32
#  define EXE_IMPORT __declspec(dllimport)
#else
#  define EXE_IMPORT
#endif

${import_list}
int main(void)
{
${call_list}
  return 0;
}
")

add_executable(exe "${CMAKE_BINARY_DIR}/main.c")
target_link_libraries(exe PRIVATE ${dep_list})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set_property(TARGET exe PROPERTY INSTALL_RPATH "\${ORIGIN}/../lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set_property(TARGET exe PROPERTY INSTALL_RPATH "@loader_path/../lib")
endif()

install(TARGETS exe ${dep_list})

install(CODE
  [[
  function(exec_get_runtime_dependencies depsfile udepsfile cdepsfile)
    file(GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR deps
      UNRESOLVED_DEPENDENCIES_VAR udeps
      CONFLICTING_DEPENDENCIES_PREFIX cdeps
      PRE_INCLUDE_REGEXES "dep[123456789]"
      PRE_EXCLUDE_REGEXES ".*"
      POST_INCLUDE_REGEXES "dep9"
      POST_INCLUDE_FILES
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep1>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep1>"
        "${CMAKE_INSTALL_PREFIX}/bin/../bin/$<TARGET_FILE_NAME:dep2>"
        "${CMAKE_INSTALL_PREFIX}/bin/../lib/$<TARGET_FILE_NAME:dep2>"
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep3>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep3>"
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep8>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep8>"
      POST_EXCLUDE_FILES
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep3>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep3>"
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep5>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep5>"
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep6>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep6>"
        "${CMAKE_INSTALL_PREFIX}/bin/../bin/$<TARGET_FILE_NAME:dep7>"
        "${CMAKE_INSTALL_PREFIX}/bin/../lib/$<TARGET_FILE_NAME:dep7>"
      POST_EXCLUDE_FILES_STRICT
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep8>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep8>"
        "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:dep9>"
        "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:dep9>"
      ${ARGN}
      )
    list(SORT deps)
    list(SORT udeps)
    list(SORT cdeps_FILENAMES)
    file(WRITE "${CMAKE_INSTALL_PREFIX}/deps/${depsfile}" "${deps}")
    file(WRITE "${CMAKE_INSTALL_PREFIX}/deps/${udepsfile}" "${udeps}")
    file(WRITE "${CMAKE_INSTALL_PREFIX}/deps/${cdepsfile}" "")
    foreach(cdep IN LISTS cdeps_FILENAMES)
      set(cdep_values ${cdeps_${cdep}})
      list(SORT cdep_values)
      file(APPEND "${CMAKE_INSTALL_PREFIX}/deps/${cdepsfile}" "${cdep}:${cdep_values}\n")
    endforeach()
  endfunction()

  exec_get_runtime_dependencies(
    deps.txt udeps.txt cdeps.txt
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:exe>"
    )
  ]])
