include(RunCMake)

run_cmake(InstallBeforeFramework)

function(framework_layout_test Name Toolchain Type)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${Toolchain}${Type}FrameworkLayout-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/${Toolchain}.cmake")
  list(APPEND RunCMake_TEST_OPTIONS "-DFRAMEWORK_TYPE=${Type}")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(FrameworkLayout)
  run_cmake_command(${Name} ${CMAKE_COMMAND} --build .)
endfunction()

framework_layout_test(iOSFrameworkLayout-build ios SHARED)
framework_layout_test(iOSFrameworkLayout-build ios STATIC)
framework_layout_test(OSXFrameworkLayout-build osx SHARED)
framework_layout_test(OSXFrameworkLayout-build osx STATIC)

function(framework_type_test Toolchain Type UseProperty)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${Toolchain}${Type}FrameworkType-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/${Toolchain}.cmake")
  list(APPEND RunCMake_TEST_OPTIONS "-DFRAMEWORK_TYPE=${Type}")
  if(NOT UseProperty)
    list(APPEND RunCMake_TEST_OPTIONS "-DCMAKE_FRAMEWORK=YES")
  endif()

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(FrameworkLayout)
  run_cmake_command(FrameworkType${Type}-build ${CMAKE_COMMAND} --build .)
endfunction()

framework_type_test(ios SHARED NO)
framework_type_test(ios STATIC NO)
framework_type_test(osx SHARED NO)
framework_type_test(osx STATIC NO)

framework_type_test(ios SHARED YES)
framework_type_test(ios STATIC YES)
framework_type_test(osx SHARED YES)
framework_type_test(osx STATIC YES)
