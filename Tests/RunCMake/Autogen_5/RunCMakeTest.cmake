include(RunCMake)
include(Autogen_common/utils)

if (DEFINED with_qt_version)
  set(RunCMake_TEST_OPTIONS
    -Dwith_qt_version=${with_qt_version}
    "-DQt${with_qt_version}_DIR:PATH=${Qt${with_qt_version}_DIR}"
    "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
  )
  autogen_executable_test(Rcc)
  if (QtCore_VERSION VERSION_GREATER_EQUAL 6)
    if (RunCMake_GENERATOR MATCHES "Make|Ninja")
      foreach(value IN ITEMS ON OFF)
        block()
          set(RunCMake_TEST_BINARY_DIR
            ${RunCMake_BINARY_DIR}/RccNoZTSD-${value}-build)
          run_cmake_with_options(RccExample ${RunCMake_TEST_OPTIONS}
            -DCMAKE_AUTOGEN_VERBOSE=ON -DZSTD_VALUE=${value})
          if (value STREQUAL "OFF")
            set(RunCMake_TEST_EXPECT_stdout "--no-zstd")
          else()
            set(RunCMake_TEST_NOT_EXPECT_stdout "--no-zstd")
          endif()
          set(RunCMake_TEST_NO_CLEAN 1)
          run_cmake_command(RccNoZTSD-${value}-build ${CMAKE_COMMAND}
            --build . --config Debug)
        endblock()
      endforeach()
    endif()
    if (RunCMake_GENERATOR MATCHES "Ninja")
      block()
        set(RunCMake_TEST_BINARY_DIR
          ${RunCMake_BINARY_DIR}/RccGlobalAutoRcc-build)
        run_cmake_with_options(RccExample ${RunCMake_TEST_OPTIONS}
          -DCMAKE_GLOBAL_AUTORCC_TARGET=ON)
        set(RunCMake_TEST_NO_CLEAN 1)
        set(RunCMake_TEST_VARIANT_DESCRIPTION "-First-build")
        run_cmake_command(RccGlobalAutoRcc-build ${CMAKE_COMMAND}
          --build . --config Debug)
        set(RunCMake_TEST_VARIANT_DESCRIPTION "-Second-build-nothing-to-do")
        set(RunCMake_TEST_NOT_EXPECT_stdout "Automatic RCC for data.qrc")
        run_cmake_command(RccGlobalAutoRcc-build ${CMAKE_COMMAND}
          --build . --config Debug)
      endblock()
    endif()
  endif()
endif ()
