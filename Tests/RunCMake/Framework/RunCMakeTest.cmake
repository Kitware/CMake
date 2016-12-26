include(RunCMake)

function(framework_layout_test Name Toolchain)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${Toolchain}FrameworkLayout-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/${Toolchain}.cmake")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(FrameworkLayout)
  run_cmake_command(${Name} ${CMAKE_COMMAND} --build .)
endfunction()

# build check cannot cope with multi-configuration generators directory layout
if(NOT RunCMake_GENERATOR STREQUAL "Xcode")
  framework_layout_test(iOSFrameworkLayout-build ios)
  framework_layout_test(OSXFrameworkLayout-build osx)
endif()
