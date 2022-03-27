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

function(framework_multi_config_postfix_test)
    set(configure_name "FrameworkMultiConfigPostfix")
    set(build_name "${configure_name}-build-intermediate")
    set(build_name_final "${configure_name}-build-final")

    if(RunCMake_GENERATOR MATCHES "Ninja Multi-Config")
        set(RunCMake_TEST_OPTIONS
            "-DCMAKE_CONFIGURATION_TYPES=Debug\\;Release;-DCMAKE_CROSS_CONFIGS=all")
    elseif(RunCMake_GENERATOR MATCHES "Xcode")
        set(RunCMake_TEST_OPTIONS
            "-DCMAKE_CONFIGURATION_TYPES=Debug\\;Release")
    else()
        set(RunCMake_TEST_OPTIONS "-DCMAKE_BUILD_TYPE=Debug")
    endif()

    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${configure_name})
    set(RunCMake_TEST_NO_CLEAN 1)

    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
    run_cmake(${configure_name})
    unset(RunCMake_TEST_OPTIONS)

    if(RunCMake_GENERATOR MATCHES "Ninja Multi-Config")
        run_cmake_command(${build_name_final} ${CMAKE_COMMAND} --build . --target all:all)
    elseif(RunCMake_GENERATOR MATCHES "Xcode")
        run_cmake_command(${build_name} ${CMAKE_COMMAND} --build . --config Release)
        run_cmake_command(${build_name} ${CMAKE_COMMAND} --build . --config Debug)
        run_cmake_command(${build_name_final} ${CMAKE_COMMAND} --build . --config Debug)
    else()
        run_cmake_command(${build_name_final} ${CMAKE_COMMAND} --build .)
    endif()
endfunction()

framework_multi_config_postfix_test()

function(imported_framework_test)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/ImportedFrameworkTest-build")
  set(RunCMake_TEST_NO_CLEAN 1)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(ImportedFrameworkTest)
  run_cmake_command(ImportedFrameworkTest-build ${CMAKE_COMMAND} --build .)
endfunction()

imported_framework_test()

function(framework_system_include_test)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/FrameworkSystemIncludeTest-build")
  set(RunCMake_TEST_NO_CLEAN 1)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(FrameworkSystemIncludeTest)
  run_cmake_command(FrameworkSystemIncludeTest-build ${CMAKE_COMMAND} --build .)
endfunction()

framework_system_include_test()

function(framework_consumption)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/FrameworkConsumption-build")
  set(RunCMake_TEST_NO_CLEAN 1)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(FrameworkConsumption)
  run_cmake_command(FrameworkConsumption-build ${CMAKE_COMMAND} --build .)
endfunction()

framework_consumption()
