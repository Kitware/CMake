include(RunCMake)

find_program(DOT dot)

# Set to TRUE to re-generate the reference files from the actual outputs.
# Make sure you verify them!
set(REPLACE_REFERENCE_FILES FALSE)

# Set to TRUE to generate PNG files from the .dot files, using Graphviz (dot).
# Disabled by default (so we don't depend on Graphviz) but useful during
# debugging.
set(GENERATE_PNG_FILES FALSE)

# 1. Generate the Graphviz (.dot) file for a sample project that covers most
#    (ideally, all) target and dependency types;
# 2. Compare that generated file with a reference file.
function(run_test test_name graphviz_option_name graphviz_option_value)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test_name})
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  # Set ${graphviz_option_name} to ${graphviz_option_value}.
  if(graphviz_option_name)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeGraphVizOptions.cmake.in
      ${RunCMake_TEST_BINARY_DIR}/CMakeGraphVizOptions.cmake
    )
  endif()

  run_cmake(GraphvizTestProject)

  if(REPLACE_REFERENCE_FILES)
    run_cmake_command(${test_name}-create_dot_files ${CMAKE_COMMAND}
      --graphviz=generated_dependency_graph.dot .
    )

    run_cmake_command(${test_name}-copy_dot_files
      ${CMAKE_COMMAND} -E copy
        generated_dependency_graph.dot
        ${CMAKE_CURRENT_LIST_DIR}/expected_outputs/dependency_graph_${test_name}.dot
    )
  endif()

  run_cmake_command(${test_name} ${CMAKE_COMMAND}
    --graphviz=generated_dependency_graph.dot .
  )

  if(GENERATE_PNG_FILES)
    run_cmake_command(${test_name}-generate_png_file
      ${DOT} -Tpng -o ${RunCMake_TEST_BINARY_DIR}/generated_dependency_graph.png
      ${RunCMake_TEST_BINARY_DIR}/generated_dependency_graph.dot
    )
  endif()

endfunction()

run_test(default_options "" "")

run_test(set_graph_name GRAPHVIZ_GRAPH_NAME "\"CMake Project Dependencies\"")
run_test(set_graph_header GRAPHVIZ_GRAPH_HEADER
  "\"node [\n  fontsize = \\\"16\\\"\n];\"")
run_test(set_node_prefix GRAPHVIZ_NODE_PREFIX "point")

run_test(no_executables GRAPHVIZ_EXECUTABLES FALSE)

run_test(no_static_libs GRAPHVIZ_STATIC_LIBS FALSE)
run_test(no_shared_libs GRAPHVIZ_SHARED_LIBS FALSE)
run_test(no_module_libs GRAPHVIZ_MODULE_LIBS FALSE)

run_test(no_interface_libs GRAPHVIZ_INTERFACE_LIBS FALSE)
run_test(no_object_libs GRAPHVIZ_OBJECT_LIBS FALSE)
run_test(no_unknown_libs GRAPHVIZ_UNKNOWN_LIBS FALSE)

run_test(no_external_libs GRAPHVIZ_EXTERNAL_LIBS FALSE)

run_test(custom_targets GRAPHVIZ_CUSTOM_TARGETS TRUE)

run_test(no_graphic_libs GRAPHVIZ_IGNORE_TARGETS "Graphic")

run_test(no_per_target_files GRAPHVIZ_GENERATE_PER_TARGET FALSE)
run_test(no_dependers_files GRAPHVIZ_GENERATE_DEPENDERS FALSE)
