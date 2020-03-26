include(RunCMake)

ensure_files_match(
    ${RunCMake_TEST_SOURCE_DIR}/expected_outputs/dependency_graph_set_graph_header.dot
    ${RunCMake_TEST_BINARY_DIR}/generated_dependency_graph.dot)
