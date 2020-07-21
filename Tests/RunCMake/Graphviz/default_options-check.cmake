include(RunCMake)

ensure_files_match(
        ${RunCMake_TEST_SOURCE_DIR}/expected_outputs/dependency_graph_default_options.dot
        ${RunCMake_TEST_BINARY_DIR}/generated_dependency_graph.dot)

ensure_files_match(
        ${RunCMake_TEST_SOURCE_DIR}/expected_outputs/dependency_graph_target_dependencies.dot.GraphicApplication
        ${RunCMake_TEST_BINARY_DIR}/generated_dependency_graph.dot.GraphicApplication)

ensure_files_match(
        ${RunCMake_TEST_SOURCE_DIR}/expected_outputs/dependency_graph_target_dependers.dot.CompilerFlags.dependers
        ${RunCMake_TEST_BINARY_DIR}/generated_dependency_graph.dot.CompilerFlags.dependers)
