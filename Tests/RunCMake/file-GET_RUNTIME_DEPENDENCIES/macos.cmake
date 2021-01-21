enable_language(C)

set(testlib_names
  preexcluded
  executable_path
  executable_path_bundle
  executable_path_postexcluded
  loader_path
  loader_path_unresolved
  loader_path_postexcluded
  rpath
  rpath_unresolved
  rpath_postexcluded
  rpath_executable_path
  rpath_executable_path_bundle
  rpath_executable_path_postexcluded
  rpath_loader_path
  rpath_loader_path_unresolved
  rpath_loader_path_postexcluded
  normal
  normal_unresolved
  normal_postexcluded
  conflict
  )

file(REMOVE "${CMAKE_BINARY_DIR}/testlib.c")
add_library(testlib SHARED "${CMAKE_BINARY_DIR}/testlib.c")
foreach(name ${testlib_names})
  if(name STREQUAL "normal")
    file(WRITE "${CMAKE_BINARY_DIR}/normal.c" "extern void rpath(void);\nvoid normal(void)\n{\n  rpath();\n}\n")
  else()
    file(WRITE "${CMAKE_BINARY_DIR}/${name}.c" "void ${name}(void) {}\n")
  endif()
  add_library(${name} SHARED "${CMAKE_BINARY_DIR}/${name}.c")

  file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "extern void ${name}(void);\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "void testlib(void)\n{\n")
foreach(name ${testlib_names})
  file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "  ${name}();\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/testlib.c" "}\n")
set_property(TARGET ${testlib_names} PROPERTY BUILD_WITH_INSTALL_NAME_DIR 1)
target_link_libraries(normal PRIVATE rpath)
set_property(TARGET normal PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/executable/lib/normal/../rpath"
  )

file(WRITE "${CMAKE_BINARY_DIR}/testlib_conflict.c" "extern void conflict(void);\nvoid testlib_conflict(void)\n{\n  conflict();\n}\n")
add_library(testlib_conflict SHARED "${CMAKE_BINARY_DIR}/testlib_conflict.c")
target_link_libraries(testlib_conflict PRIVATE conflict)

set_property(TARGET testlib PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/executable/lib/rpath"
  "${CMAKE_BINARY_DIR}/root-all/executable/lib/rpath_unresolved"
  "${CMAKE_BINARY_DIR}/root-all/executable/lib/rpath_postexcluded"
  "${CMAKE_BINARY_DIR}/root-all/executable/lib/conflict"
  @executable_path/../lib/rpath_executable_path
  @executable_path/../lib/rpath_executable_path_unresolved
  @executable_path/../lib/rpath_executable_path_postexcluded
  @loader_path/rpath_loader_path
  @loader_path/rpath_loader_path_unresolved
  @loader_path/rpath_loader_path_postexcluded
  )
set_property(TARGET testlib_conflict PROPERTY INSTALL_RPATH
  "${CMAKE_BINARY_DIR}/root-all/executable/lib/conflict2"
  )

foreach(t
  executable_path
  executable_path_postexcluded
  loader_path
  loader_path_postexcluded
  rpath
  rpath_postexcluded
  rpath_executable_path
  rpath_executable_path_postexcluded
  rpath_loader_path
  rpath_loader_path_postexcluded
  conflict
  )
  install(TARGETS ${t} DESTINATION executable/lib/${t})
endforeach()
install(TARGETS conflict DESTINATION executable/lib/conflict2)

foreach(t
  executable_path_bundle
  executable_path_postexcluded
  loader_path_postexcluded
  rpath_postexcluded
  rpath_executable_path_bundle
  rpath_executable_path_postexcluded
  rpath_loader_path_postexcluded
  )
  install(TARGETS ${t} DESTINATION bundle_executable/lib/${t})
endforeach()

foreach(t executable_path executable_path_bundle executable_path_postexcluded)
  set_property(TARGET ${t} PROPERTY INSTALL_NAME_DIR @executable_path/../lib/${t})
endforeach()

foreach(t loader_path loader_path_unresolved loader_path_postexcluded)
  set_property(TARGET ${t} PROPERTY INSTALL_NAME_DIR @loader_path/${t})
endforeach()

foreach(t
  rpath
  rpath_unresolved
  rpath_postexcluded
  rpath_executable_path
  rpath_executable_path_bundle
  rpath_executable_path_postexcluded
  rpath_loader_path
  rpath_loader_path_unresolved
  rpath_loader_path_postexcluded
  conflict
  )
  set_property(TARGET ${t} PROPERTY INSTALL_NAME_DIR @rpath)
endforeach()

foreach(t normal normal_unresolved normal_postexcluded)
  set_property(TARGET ${t} PROPERTY INSTALL_NAME_DIR "${CMAKE_BINARY_DIR}/root-all/executable/lib/${t}")
  if(NOT t STREQUAL "normal_unresolved")
    install(TARGETS ${t} DESTINATION executable/lib/${t})
  endif()
endforeach()

target_link_libraries(testlib PRIVATE ${testlib_names})

add_executable(topexe macos/topexe.c)
add_executable(topexe_weak macos/topexe.c)
add_library(toplib SHARED macos/toplib.c)
add_library(topmod MODULE macos/toplib.c)
target_link_libraries(topexe PRIVATE testlib)
target_link_libraries(topexe_weak PRIVATE "-weak_library" testlib)
target_link_libraries(toplib PRIVATE testlib)
target_link_libraries(topmod PRIVATE testlib)

set_property(TARGET topexe toplib topmod PROPERTY INSTALL_RPATH "${CMAKE_BINARY_DIR}/root-all/executable/lib")
set_property(TARGET topexe_weak toplib topmod PROPERTY INSTALL_RPATH "${CMAKE_BINARY_DIR}/root-all/executable/lib")

install(TARGETS topexe topexe_weak toplib topmod testlib testlib_conflict RUNTIME DESTINATION executable/bin LIBRARY DESTINATION executable/lib)
install(TARGETS topexe topexe_weak toplib topmod testlib testlib_conflict RUNTIME DESTINATION bundle_executable/bin LIBRARY DESTINATION bundle_executable/lib)

install(CODE [[
  function(exec_get_runtime_dependencies depsfile udepsfile cdepsfile)
    file(GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR deps
      UNRESOLVED_DEPENDENCIES_VAR udeps
      CONFLICTING_DEPENDENCIES_PREFIX cdeps
      PRE_INCLUDE_REGEXES "^.*/lib(testlib|executable_path|executable_path_bundle|executable_path_postexcluded|loader_path|loader_path_unresolved|loader_path_postexcluded|rpath|rpath_unresolved|rpath_postexcluded|rpath_executable_path|rpath_executable_path_bundle|rpath_executable_path_postexcluded|rpath_loader_path|rpath_loader_path_unresolved|rpath_loader_path_postexcluded|normal|normal_unresolved|normal_postexcluded|conflict|System\\.B)\\.dylib$"
      PRE_EXCLUDE_REGEXES ".*"
      POST_INCLUDE_REGEXES "^.*/lib(testlib|executable_path|executable_path_bundle|loader_path|rpath|rpath_executable_path|rpath_executable_path_bundle|rpath_loader_path|normal|conflict|System\\.B)\\.dylib$"
      POST_EXCLUDE_REGEXES ".*"
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
      "${CMAKE_INSTALL_PREFIX}/executable/bin/$<TARGET_FILE_NAME:topexe>"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:testlib_conflict>"
    )

  exec_get_runtime_dependencies(
    deps2.txt udeps2.txt cdeps2.txt
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:toplib>"
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:testlib_conflict>"
    )

  exec_get_runtime_dependencies(
    deps3.txt udeps3.txt cdeps3.txt
    MODULES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:topmod>"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:testlib_conflict>"
    )

  exec_get_runtime_dependencies(
    deps4.txt udeps4.txt cdeps4.txt
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/executable/bin/$<TARGET_FILE_NAME:topexe>"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:testlib_conflict>"
    BUNDLE_EXECUTABLE
      "${CMAKE_INSTALL_PREFIX}/bundle_executable/bin/$<TARGET_FILE_NAME:topexe>"
    )

  exec_get_runtime_dependencies(
    deps5.txt udeps5.txt cdeps5.txt
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:toplib>"
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:testlib_conflict>"
    BUNDLE_EXECUTABLE "${CMAKE_INSTALL_PREFIX}/bundle_executable/bin/$<TARGET_FILE_NAME:topexe>"
    )

  exec_get_runtime_dependencies(
    deps6.txt udeps6.txt cdeps6.txt
    MODULES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:topmod>"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:testlib_conflict>"
    BUNDLE_EXECUTABLE "${CMAKE_INSTALL_PREFIX}/bundle_executable/bin/$<TARGET_FILE_NAME:topexe>"
    )

  exec_get_runtime_dependencies(
    deps7.txt udeps7.txt cdeps7.txt
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/executable/bin/$<TARGET_FILE_NAME:topexe_weak>"
    LIBRARIES
      "${CMAKE_INSTALL_PREFIX}/executable/lib/$<TARGET_FILE_NAME:testlib_conflict>"
    )
  ]])
