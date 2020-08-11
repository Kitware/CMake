enable_language(C)
cmake_policy(SET CMP0095 NEW)

set(test_rpath_names
  preexcluded
  rpath_postexcluded
  rpath
  rpath_parent_postexcluded
  rpath_parent
  rpath_origin_postexcluded
  rpath_origin
  rpath_search_postexcluded
  rpath_search
  rpath_unresolved
  conflict
  )
set(test_runpath_names
  runpath_postexcluded
  runpath
  runpath_origin_postexcluded
  runpath_origin
  runpath_parent_unresolved
  runpath_search_postexcluded
  runpath_search
  runpath_unresolved
  )

file(REMOVE "${CMAKE_BINARY_DIR}/test_rpath.c")
add_library(test_rpath SHARED "${CMAKE_BINARY_DIR}/test_rpath.c")
foreach(name ${test_rpath_names})
  file(WRITE "${CMAKE_BINARY_DIR}/${name}.c" "void ${name}(void) {}\n")
  add_library(${name} SHARED "${CMAKE_BINARY_DIR}/${name}.c")

  file(APPEND "${CMAKE_BINARY_DIR}/test_rpath.c" "extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test_rpath.c" "void test_rpath(void)\n{\n")
foreach(name ${test_rpath_names})
  file(APPEND "${CMAKE_BINARY_DIR}/test_rpath.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test_rpath.c" "}\n")

install(TARGETS rpath_postexcluded DESTINATION lib/rpath_postexcluded)
install(TARGETS rpath DESTINATION lib/rpath)
install(TARGETS rpath_origin_postexcluded DESTINATION lib/rpath_origin_postexcluded)
install(TARGETS rpath_origin DESTINATION lib/rpath_origin)
install(TARGETS rpath_parent_postexcluded DESTINATION lib/rpath_parent_postexcluded)
install(TARGETS rpath rpath_origin rpath_parent DESTINATION lib/rpath_parent)
install(TARGETS rpath_search_postexcluded DESTINATION lib/rpath_search_postexcluded)
install(TARGETS rpath rpath_origin rpath_parent rpath_search DESTINATION lib/rpath_search)
install(TARGETS conflict DESTINATION lib/conflict)

target_link_libraries(test_rpath PRIVATE ${test_rpath_names})
set_property(TARGET test_rpath PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/lib/rpath_postexcluded"
  "${CMAKE_BINARY_DIR}/root-all/lib/rpath"
  "\$ORIGIN/rpath_origin_postexcluded"
  "\${ORIGIN}/rpath_origin"
  "${CMAKE_BINARY_DIR}/root-all/lib/conflict"
  )
target_link_options(test_rpath PRIVATE -Wl,--disable-new-dtags)

file(REMOVE "${CMAKE_BINARY_DIR}/test_runpath.c")
add_library(test_runpath SHARED "${CMAKE_BINARY_DIR}/test_runpath.c")
foreach(name ${test_runpath_names} rpath conflict)
  file(WRITE "${CMAKE_BINARY_DIR}/${name}.c" "void ${name}(void) {}\n")
  if(NOT name MATCHES "^(rpath|conflict)$")
    add_library(${name} SHARED "${CMAKE_BINARY_DIR}/${name}.c")
  endif()

  file(APPEND "${CMAKE_BINARY_DIR}/test_runpath.c" "extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test_runpath.c" "void test_runpath(void)\n{\n")
foreach(name ${test_runpath_names} rpath conflict)
  file(APPEND "${CMAKE_BINARY_DIR}/test_runpath.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/test_runpath.c" "}\n")

install(TARGETS runpath_postexcluded DESTINATION lib/runpath_postexcluded)
install(TARGETS runpath DESTINATION lib/runpath)
install(TARGETS runpath_origin_postexcluded DESTINATION lib/runpath_origin_postexcluded)
install(TARGETS runpath_origin DESTINATION lib/runpath_origin)
install(TARGETS runpath_parent_unresolved DESTINATION lib/runpath_parent_unresolved)
install(TARGETS runpath_search_postexcluded DESTINATION lib/runpath_search_postexcluded)
install(TARGETS runpath runpath_origin runpath_search DESTINATION lib/runpath_search)
install(TARGETS conflict DESTINATION lib/conflict2)

target_link_libraries(test_runpath PRIVATE ${test_runpath_names} rpath conflict)
set_property(TARGET test_runpath PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/lib/runpath/../rpath" # Ensure that files that don't conflict are treated correctly
  "${CMAKE_BINARY_DIR}/root-all/lib/runpath_postexcluded"
  "${CMAKE_BINARY_DIR}/root-all/lib/runpath"
  "\${ORIGIN}/runpath_origin_postexcluded"
  "\$ORIGIN/runpath_origin"
  "${CMAKE_BINARY_DIR}/root-all/lib/conflict2"
  )
target_link_options(test_runpath PRIVATE -Wl,--enable-new-dtags)

set_property(TARGET test_rpath ${test_rpath_names} test_runpath ${test_runpath_names} PROPERTY LIBRARY_OUTPUT_DIRECTORY lib)
install(TARGETS test_rpath test_runpath DESTINATION lib)

add_executable(topexe linux/topexe.c)
add_library(toplib SHARED linux/toplib.c)
add_library(topmod MODULE linux/toplib.c)
target_link_libraries(topexe PRIVATE test_rpath test_runpath)
target_link_libraries(toplib PRIVATE test_rpath test_runpath)
target_link_libraries(topmod PRIVATE test_rpath test_runpath)
set_property(TARGET topexe toplib topmod PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/lib"
  "${CMAKE_BINARY_DIR}/root-all/lib/rpath_parent_postexcluded"
  "${CMAKE_BINARY_DIR}/root-all/lib/rpath_parent"
  "${CMAKE_BINARY_DIR}/root-all/lib/runpath_parent_unresolved"
  )
target_link_options(topexe PRIVATE -Wl,--disable-new-dtags)
target_link_options(toplib PRIVATE -Wl,--disable-new-dtags)
target_link_options(topmod PRIVATE -Wl,--disable-new-dtags)

install(TARGETS topexe toplib RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
install(TARGETS topmod LIBRARY DESTINATION lib/modules)

install(CODE [[
  function(exec_get_runtime_dependencies depsfile udepsfile cdepsfile)
    file(GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR deps
      UNRESOLVED_DEPENDENCIES_VAR udeps
      CONFLICTING_DEPENDENCIES_PREFIX cdeps
      PRE_INCLUDE_REGEXES
        "^lib(test_rpath|rpath_postexcluded|rpath|rpath_parent_postexcluded|rpath_parent|rpath_origin_postexcluded|rpath_origin|rpath_search_postexcluded|rpath_search|rpath_unresolved|test_runpath|runpath_postexcluded|runpath|runpath_origin_postexcluded|runpath_origin|runpath_parent_unresolved|runpath_search_postexcluded|runpath_search|runpath_unresolved|conflict)\\.so$"
        "^libc\\.so"
      PRE_EXCLUDE_REGEXES ".*"
      POST_INCLUDE_REGEXES "^.*/(libtest_rpath|rpath/librpath|rpath_parent/librpath_parent|rpath_search/librpath_search|libtest_runpath|runpath/librunpath|runpath_origin_postexcluded|runpath_origin|runpath_search/librunpath_search|conflict2?/libconflict)\\.so$"
      POST_EXCLUDE_REGEXES ".*"
      DIRECTORIES
        "${CMAKE_INSTALL_PREFIX}/lib/rpath_search_postexcluded"
        "${CMAKE_INSTALL_PREFIX}/lib/rpath_search"
        "${CMAKE_INSTALL_PREFIX}/lib/runpath_search_postexcluded"
        "${CMAKE_INSTALL_PREFIX}/lib/runpath_search"
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
    )

  exec_get_runtime_dependencies(
    deps2.txt udeps2.txt cdeps2.txt
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:toplib>"
    )

  exec_get_runtime_dependencies(
    deps3.txt udeps3.txt cdeps3.txt
    MODULES
      "${CMAKE_INSTALL_PREFIX}/lib/modules/$<TARGET_FILE_NAME:topmod>"
    )
  ]])
