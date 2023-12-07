include(RunCMake)

function(compile_source test target)
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(config "Debug/")
  endif()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-Build-${target}-Source ${CMAKE_COMMAND} --build .
    --target CMakeFiles/Hello_${target}.dir/${config}hello.cpp${CMAKE_C_OUTPUT_EXTENSION})
endfunction()

function(compile_target test target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-Build-${target} ${CMAKE_COMMAND} --build .
    --target Hello_${target})
endfunction()

run_cmake(CMP0154-OLD)
compile_source(CMP0154-OLD PrivateFileSet)
compile_source(CMP0154-OLD EmptyFileSet)

run_cmake(CMP0154-NEW)
compile_source(CMP0154-NEW PrivateFileSet)
compile_target(CMP0154-NEW PrivateFileSet)
compile_source(CMP0154-NEW PublicFileSet)
compile_source(CMP0154-NEW NoFileSet)
compile_source(CMP0154-NEW EmptyFileSet)
compile_target(CMP0154-NEW EmptyFileSet)
