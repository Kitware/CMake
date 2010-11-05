get_filename_component(dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(exe "${CMAKE_COMMAND}")
set(args -P "${dir}/ValidateBuild.cmake")
set(Trilinos-10-6_RUN_TEST ${exe} ${args})
