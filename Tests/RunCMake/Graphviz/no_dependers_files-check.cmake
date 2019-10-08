file(GLOB dependers_files ${RunCMake_TEST_BINARY_DIR}/*.dependers)
if(${dependers_files})
    set(RunCMake_TEST_FAILED "Found *.dependers files despite GRAPHVIZ_GENERATE_DEPENDERS set to FALSE.")
endif()
