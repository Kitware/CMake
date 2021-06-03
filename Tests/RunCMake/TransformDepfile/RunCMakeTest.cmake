include(RunCMake)

function(run_transform_depfile name)
  set(RunCMake-check-file gccdepfile.cmake)
  run_cmake_command(${name}-gcc
    ${CMAKE_COMMAND} -E cmake_transform_depfile "${RunCMake_GENERATOR}" gccdepfile "${RunCMake_SOURCE_DIR}" "${RunCMake_SOURCE_DIR}/subdir" "${RunCMake_BINARY_DIR}" "${RunCMake_BINARY_DIR}/subdir" "${CMAKE_CURRENT_LIST_DIR}/${name}.d" out.d
    )
endfunction()

if(WIN32)
  run_transform_depfile(deps-windows)
else()
  run_transform_depfile(deps-unix)
endif()
run_transform_depfile(noexist)
run_transform_depfile(empty)
run_transform_depfile(invalid)
