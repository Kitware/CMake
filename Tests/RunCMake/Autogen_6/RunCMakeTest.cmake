
include(RunCMake)
include(Autogen_common/utils)

if (DEFINED with_qt_version)
  set(RunCMake_TEST_OPTIONS
    -Dwith_qt_version=${with_qt_version}
    "-DQt${with_qt_version}_DIR:PATH=${Qt${with_qt_version}_DIR}"
    "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
  )
  if (QtCore_VERSION VERSION_GREATER_EQUAL 5.15.0)
    macro(set_test_variables_for_unwanted_builds)
      if (RunCMake_GENERATOR MATCHES "Ninja")
        set(RunCMake_TEST_EXPECT_stdout "ninja: no work to do.")
      elseif (RunCMake_GENERATOR MATCHES "Make")
        set(RunCMake_TEST_NOT_EXPECT_stdout "Building CXX object multi_ui_files/CMakeFiles/example.dir/src/main.cpp.o|\
                                             Building CXX object multi_ui_files/CMakeFiles/example.dir/src/widget.cpp.o")
      elseif (RunCMake_GENERATOR MATCHES "Visual Studio")
        set(RunCMake_TEST_NOT_EXPECT_stdout "widget.cpp")
      elseif (RunCMake_GENERATOR MATCHES "Xcode")
        set(RunCMake_TEST_NOT_EXPECT_stdout "widget.cpp")
      endif()
    endmacro()

    function(uic_incremental_build_test test_name binary_dir source_dir test_config)
      set(RunCMake_TEST_BINARY_DIR ${binary_dir})
      set(RunCMake_TEST_SOURCE_DIR ${source_dir})

      if (NOT RunCMake_GENERATOR MATCHES "Visual Studio")
        set(test_verbose_arg "--verbose")
      endif()
      if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
        set(config_desc "-${test_config}")
        set(RunCMake_TEST_VARIANT_DESCRIPTION "${config_desc}")
        set(multiconfig_config_arg "--config ${test_config}")
      else()
        set(RunCMake_TEST_VARIANT_DESCRIPTION "")
        set(config_arg "-DCMAKE_BUILD_TYPE=Debug")
      endif()
      run_cmake_with_options(${test_name} ${RunCMake_TEST_OPTIONS} ${config_arg})
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_VARIANT_DESCRIPTION "${RunCMake_TEST_VARIANT_DESCRIPTION}-First build")
      run_cmake_command("${test_name}-build" ${CMAKE_COMMAND} --build . ${test_verbose_arg} ${multiconfig_config_arg})
      set(RunCMake_TEST_VARIANT_DESCRIPTION "${config_desc}-Second build")
      run_cmake_command("${test_name}-build" ${CMAKE_COMMAND} --build . ${test_verbose_arg} ${multiconfig_config_arg})

      set(RunCMake_TEST_VARIANT_DESCRIPTION "${config_desc}-No files were built on the third build")
      set_test_variables_for_unwanted_builds()
      run_cmake_command("${test_name}-build" ${CMAKE_COMMAND} --build . ${test_verbose_arg} ${multiconfig_config_arg})
    endfunction()

    if(RunCMake_GENERATOR MATCHES "Make|Ninja|Visual Studio|Xcode")
      if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
       set(configs "Debug" "Release")
      else()
        set(configs "single_config")
      endif()

      foreach(config IN ITEMS ${configs})
        if (NOT ${config} STREQUAL "single_config")
          set(config_desc "-${config}")
        endif()

        uic_incremental_build_test(incremental_build_test ${RunCMake_BINARY_DIR}/incremental_build${config_desc}-build
          ${RunCMake_SOURCE_DIR}/incremental_build ${config})

      endforeach()
    endif()
  endif()
endif ()
