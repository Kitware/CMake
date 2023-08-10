enable_language(C)

set(testlib_names
  preexcluded
  libdir_postexcluded
  libdir
  search_postexcluded
  search
  unresolved
  conflict
  MixedCase
  )

file(REMOVE "${CMAKE_BINARY_DIR}/testlib.c")
add_library(testlib SHARED "${CMAKE_BINARY_DIR}/testlib.c")
foreach(name ${testlib_names})
  file(WRITE "${CMAKE_BINARY_DIR}/${name}.c" "__declspec(dllexport) void ${name}(void) {}\n")
  add_library(${name} SHARED "${CMAKE_BINARY_DIR}/${name}.c")

  file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "__declspec(dllimport) extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "__declspec(dllexport) void testlib(void)\n{\n")
foreach(name ${testlib_names})
  file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "}\n")

target_link_libraries(testlib PRIVATE ${testlib_names})

file(WRITE "${CMAKE_BINARY_DIR}/testlib_conflict.c" "__declspec(dllimport) extern void conflict(void);\n__declspec(dllexport) void testlib_conflict(void)\n{\n  conflict();\n}\n")
add_library(testlib_conflict SHARED "${CMAKE_BINARY_DIR}/testlib_conflict.c")
target_link_libraries(testlib_conflict PRIVATE conflict)

file(WRITE "${CMAKE_BINARY_DIR}/testlib_noconflict.c" "__declspec(dllimport) extern void libdir(void);\n__declspec(dllexport) void testlib_noconflict(void)\n{\n  libdir();\n}\n")
add_library(testlib_noconflict SHARED "${CMAKE_BINARY_DIR}/testlib_noconflict.c")
target_link_libraries(testlib_noconflict PRIVATE libdir)

install(TARGETS testlib libdir_postexcluded libdir conflict MixedCase testlib_noconflict DESTINATION bin)
install(TARGETS libdir search_postexcluded search DESTINATION bin/.search) # Prefixing with "." ensures it is the first item after list(SORT)
install(TARGETS testlib_conflict conflict DESTINATION bin/.conflict)

add_executable(topexe windows/topexe.c)
add_library(toplib SHARED windows/toplib.c)
add_library(topmod MODULE windows/toplib.c)
target_link_libraries(topexe PRIVATE testlib)
target_link_libraries(toplib PRIVATE testlib)
target_link_libraries(topmod PRIVATE testlib)

install(TARGETS topexe toplib topmod DESTINATION bin)

install(CODE [[
  function(exec_get_runtime_dependencies depsfile udepsfile cdepsfile)
    file(GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR deps
      UNRESOLVED_DEPENDENCIES_VAR udeps
      CONFLICTING_DEPENDENCIES_PREFIX cdeps
      PRE_INCLUDE_REGEXES
        "^(lib)?testlib\\.dll$"
        "^(lib)?libdir_postexcluded\\.dll$"
        "^(lib)?libdir\\.dll$"
        "^(lib)?search_postexcluded\\.dll$"
        "^(lib)?search\\.dll$"
        "^(lib)?unresolved\\.dll$"
        "^(lib)?conflict\\.dll$"
        "^(lib)?mixedcase\\.dll$"
        "^kernel32\\.dll$"
      PRE_EXCLUDE_REGEXES ".*"
      POST_INCLUDE_REGEXES
        "^.*/(lib)?testlib\\.dll$"
        "^.*/(lib)?libdir\\.dll$"
        "^.*/(lib)?search\\.dll$"
        "^.*/(lib)?conflict\\.dll$"
        "^.*/(lib)?mixedcase\\.dll$"
      POST_EXCLUDE_REGEXES ".*"
      DIRECTORIES
        "${CMAKE_INSTALL_PREFIX}/bin/.search"
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
    deps1.txt udeps1.txt cdeps1.txt
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:topexe>"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/bin/.conflict/$<TARGET_FILE_NAME:testlib_conflict>"
      "${CMAKE_INSTALL_PREFIX}/bin/.conflict/../$<TARGET_FILE_NAME:testlib_noconflict>"
    )

  exec_get_runtime_dependencies(
    deps2.txt udeps2.txt cdeps2.txt
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:toplib>"
      "${CMAKE_INSTALL_PREFIX}/bin/.conflict/$<TARGET_FILE_NAME:testlib_conflict>"
      "${CMAKE_INSTALL_PREFIX}/bin/.conflict/../$<TARGET_FILE_NAME:testlib_noconflict>"
    )

  exec_get_runtime_dependencies(
    deps3.txt udeps3.txt cdeps3.txt
    MODULES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:topmod>"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/bin/.conflict/$<TARGET_FILE_NAME:testlib_conflict>"
      "${CMAKE_INSTALL_PREFIX}/bin/.conflict/../$<TARGET_FILE_NAME:testlib_noconflict>"
    )
  ]])
