if (CMake_TEST_MODULE_COMPILATION_RULES)
  include("${CMake_TEST_MODULE_COMPILATION_RULES}")
endif ()

include(CTest)
enable_testing()
