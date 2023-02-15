include(RunCMake)

foreach(
  extraGenerator
  IN ITEMS
    "CodeBlocks"
    "CodeLite"
    "Eclipse CDT4"
    "Kate"
    "Sublime Text 2"
  )
  block()
    set(RunCMake_GENERATOR "${extraGenerator} - ${RunCMake_GENERATOR}")
    set(RunCMake_TEST_VARIANT_DESCRIPTION ": ${RunCMake_GENERATOR}")
    string(REPLACE " " "" extraGeneratorNoSpaces "${extraGenerator}")
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Simple-${extraGeneratorNoSpaces})
    run_cmake(Simple)
  endblock()
endforeach()
