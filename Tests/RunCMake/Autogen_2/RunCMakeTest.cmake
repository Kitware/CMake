include(RunCMake)

if (DEFINED with_qt_version)
  set(RunCMake_TEST_OPTIONS
    -Dwith_qt_version=${with_qt_version}
    "-DQt${with_qt_version}_DIR:PATH=${Qt${with_qt_version}_DIR}"
    "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
  )
  if(RunCMake_GENERATOR MATCHES "Make|Ninja")
    block()
      if(QtCore_VERSION VERSION_GREATER_EQUAL 5.15.0)
        if (RunCMake_GENERATOR MATCHES "Ninja Multi-Config")
          set(config_list Debug Release RelWithDebInfo)
          set(use_better_graph_list ON OFF)
        else()
          set(config_list single-config)
          set(use_better_graph_list OFF)
        endif()

        foreach(use_better_graph IN ITEMS ${use_better_graph_list})
          foreach(config IN ITEMS ${config_list})
            block()
              if (config STREQUAL "single-config")
                set(config_suffix "")
              else()
                set(config_path "_${config}")
                if (use_better_graph)
                  set(config_suffix "_${config}")
                endif()
              endif()

              set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/QtAutoMocDeps${config_path}-build)
              run_cmake_with_options(QtAutoMocDeps ${RunCMake_TEST_OPTIONS} -DCMAKE_AUTOGEN_BETTER_GRAPH_MULTI_CONFIG=${use_better_graph})
              set(RunCMake_TEST_NO_CLEAN 1)
              # Build the project.
              if (config STREQUAL "single-config")
                set(config_param "")
              else()
                set(config_param "--config ${config}")
              endif()
              run_cmake_command(QtAutoMocDeps-build ${CMAKE_COMMAND} --build . --verbose ${config_param})
              # Touch just the library source file, which shouldn't cause a rerun of AUTOMOC
              # for app_with_qt target.
              file(TOUCH "${RunCMake_SOURCE_DIR}/simple_lib.cpp")
              set(RunCMake_TEST_NOT_EXPECT_stdout "Automatic MOC for target app_with_qt|\
Automatic MOC for target sub_exe_1|\
Automatic MOC for target sub_exe_2")
              set(RunCMake_TEST_VARIANT_DESCRIPTION "-Don't execute AUTOMOC for 'app_with_qt', 'sub_exe_1' and 'sub_exe_2'")
              # Build and assert that AUTOMOC was not run for app_with_qt, sub_exe_1 and sub_exe_2.
              run_cmake_command(QtAutoMocDeps-build ${CMAKE_COMMAND} --build . --verbose ${config_param})
              unset(RunCMake_TEST_VARIANT_DESCRIPTION)
              unset(RunCMake_TEST_NOT_EXPECT_stdout)

              macro(check_file_exists file)
                if (EXISTS "${file}")
                  set(check_result "PASSED")
                  set(message_type "STATUS")
                else()
                  set(check_result "FAILED")
                  set(message_type "FATAL_ERROR")
                endif()

                message(${message_type} "QtAutoMocDeps-build-\"${file}\" was generated - ${check_result}")
              endmacro()

              check_file_exists("${RunCMake_TEST_BINARY_DIR}/app_with_qt_autogen/deps${config_suffix}")
              check_file_exists("${RunCMake_TEST_BINARY_DIR}/QtSubDir1/sub_exe_1_autogen/deps${config_suffix}")
              check_file_exists("${RunCMake_TEST_BINARY_DIR}/QtSubDir2/sub_exe_2_autogen/deps${config_suffix}")

              check_file_exists("${RunCMake_TEST_BINARY_DIR}/app_with_qt_autogen/timestamp${config_suffix}")
              check_file_exists("${RunCMake_TEST_BINARY_DIR}/QtSubDir1/sub_exe_1_autogen/timestamp${config_suffix}")
              check_file_exists("${RunCMake_TEST_BINARY_DIR}/QtSubDir2/sub_exe_2_autogen/timestamp${config_suffix}")

              # Touch a header file to make sure an automoc dependency cycle is not introduced.
              file(TOUCH "${RunCMake_SOURCE_DIR}/MyWindow.h")
              set(RunCMake_TEST_VARIANT_DESCRIPTION "-First build after touch to detect dependency cycle")
              run_cmake_command(QtAutoMocDeps-build ${CMAKE_COMMAND} --build . --verbose)
              # Need to run a second time to hit the dependency cycle.
              set(RunCMake_TEST_VARIANT_DESCRIPTION "-Don't hit dependency cycle")
              run_cmake_command(QtAutoMocDeps-build ${CMAKE_COMMAND} --build . --verbose)
            endblock()
          endforeach()
        endforeach()
      endif()
    endblock()
  endif()
endif ()
