include(RunCMake)

ensure_files_match(
    ${RunCMake_TEST_SOURCE_DIR}/expected_outputs/dependency_graph_no_module_libs.dot
    ${RunCMake_TEST_BINARY_DIR}/generated_dependency_graph.dot)
