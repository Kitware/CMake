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
  endif()

  # The rcc listing of a qrc file is cached between runs.  It must be
  # refreshed when the qrc file changes, and must not be cached at all when
  # the qrc file refers to a directory, whose content may change without the
  # qrc file changing.
  block()
    set(data_dir "${RunCMake_BINARY_DIR}/RccListCache-data")
    file(REMOVE_RECURSE "${data_dir}")
    file(WRITE "${data_dir}/alpha.txt" "alpha\n")
    file(WRITE "${data_dir}/beta.txt" "beta\n")
    file(WRITE "${data_dir}/dir/gamma.txt" "gamma\n")
    function(write_qrc entry)
      file(WRITE "${data_dir}/res.qrc" "<RCC>
  <qresource prefix=\"/\">
    <file>${entry}</file>
  </qresource>
</RCC>
")
    endfunction()

    set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/RccListCache-build")
    list(APPEND RunCMake_TEST_OPTIONS "-DRCC_DATA_DIR=${data_dir}")

    write_qrc(alpha.txt)
    set(expect_inputs alpha.txt)
    set(expect_no_inputs beta.txt)
    set(expect_cache 1)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-1-initial")
    run_cmake(RccListCache)
    set(RunCMake_TEST_NO_CLEAN 1)

    # A changed qrc file is listed again, even if its modification time
    # did not change because it was modified within the time resolution of
    # the file system.
    write_qrc(beta.txt)
    set(expect_inputs beta.txt)
    set(expect_no_inputs alpha.txt)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-2-qrc-changed")
    run_cmake(RccListCache)

    # An unchanged qrc file is not listed again.  Prove it by adding an
    # entry to the cached listing.
    file(GLOB_RECURSE cache_files
      "${RunCMake_TEST_BINARY_DIR}/AutoRcc_res_*_List*.txt")
    foreach(cache_file IN LISTS cache_files)
      file(APPEND "${cache_file}" "${data_dir}/injected.txt\n")
    endforeach()
    set(expect_inputs beta.txt injected.txt)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-3-qrc-unchanged")
    run_cmake(RccListCache)

    # A qrc file referring to a directory is not cached.
    write_qrc(dir)
    set(expect_inputs gamma.txt)
    set(expect_no_inputs beta.txt injected.txt)
    set(expect_cache 0)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-4-qrc-directory")
    run_cmake(RccListCache)

    # New files in the directory are found without changing the qrc file.
    file(WRITE "${data_dir}/dir/delta.txt" "delta\n")
    set(expect_inputs gamma.txt delta.txt)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-5-directory-changed")
    run_cmake(RccListCache)
  endblock()
endif ()
