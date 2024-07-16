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
        set(RunCMake_TEST_NOT_EXPECT_stdout "widget2.cpp.o.d|mainwindow.cpp.o.d")
      elseif (RunCMake_GENERATOR MATCHES "Make")
        set(RunCMake_TEST_NOT_EXPECT_stdout "Building CXX object multi_ui_files/CMakeFiles/example.dir/src/widget2.cpp.o|\
                                             Building CXX object multi_ui_files/CMakeFiles/example.dir/src/mainwindow.cpp.o")
      elseif (RunCMake_GENERATOR MATCHES "Visual Studio")
        set(RunCMake_TEST_NOT_EXPECT_stdout "widget2.cpp|mainwindow.cpp")
      elseif (RunCMake_GENERATOR MATCHES "Xcode")
        set(RunCMake_TEST_NOT_EXPECT_stdout "widget2.cpp|mainwindow.cpp")
      endif()
    endmacro()

    function(uic_build_test test_name binary_dir source_dir file_to_touch test_config)
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
      run_cmake_command("${test_name}-build" ${CMAKE_COMMAND} --build . ${test_verbose_arg} ${multiconfig_config_arg})

      file(TOUCH ${file_to_touch})
      set(RunCMake_TEST_VARIANT_DESCRIPTION "${config_desc}-first_build_after_touching")
      set_test_variables_for_unwanted_builds()
      run_cmake_command("${test_name}-build" ${CMAKE_COMMAND} --build . ${test_verbose_arg} ${multiconfig_config_arg})
      message(STATUS "${test_name}-build${config_desc}-Only build files that were touched were built - PASSED")
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

        uic_build_test(multi_ui_files_touch_ui ${RunCMake_BINARY_DIR}/multi_ui_files_touch_ui${config_desc}-build
          ${RunCMake_SOURCE_DIR}/multi_ui_files ${RunCMake_SOURCE_DIR}/multi_ui_files/src/widget1.ui ${config})

        uic_build_test(multi_ui_files_touch_cpp ${RunCMake_BINARY_DIR}/multi_ui_files_touch_cpp${config_desc}-build
          ${RunCMake_SOURCE_DIR}/multi_ui_files ${RunCMake_SOURCE_DIR}/multi_ui_files/src/widget1.cpp ${config})
      endforeach()
    endif()
  endif()
endif ()
