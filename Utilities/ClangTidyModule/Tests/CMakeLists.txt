configure_file("${CMake_SOURCE_DIR}/.clang-format" ".clang-format" COPYONLY)

function(add_run_clang_tidy_test check_name)
  add_test(NAME "RunClangTidy.${check_name}" COMMAND ${CMAKE_COMMAND}
    "-DCLANG_TIDY_COMMAND=$<TARGET_FILE:clang-tidy>"
    "-DCLANG_TIDY_MODULE=$<TARGET_FILE:cmake-clang-tidy-module>"
    "-DCHECK_NAME=${check_name}"
    "-DRunClangTidy_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/RunClangTidy.cmake"
    )
endfunction()

add_run_clang_tidy_test(cmake-use-cmstrlen)
add_run_clang_tidy_test(cmake-use-cmsys-fstream)
add_run_clang_tidy_test(cmake-use-bespoke-enum-class)
add_run_clang_tidy_test(cmake-ostringstream-use-cmstrcat)
add_run_clang_tidy_test(cmake-use-pragma-once)
add_run_clang_tidy_test(cmake-string-concatenation-use-cmstrcat)
