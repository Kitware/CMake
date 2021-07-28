include(RunCMake)

run_cmake(CREATE_LINK)
run_cmake(CREATE_LINK-COPY_ON_ERROR)
run_cmake(CREATE_LINK-noarg)
run_cmake(CREATE_LINK-noexist)
run_cmake(DOWNLOAD-hash-mismatch)
run_cmake(DOWNLOAD-unused-argument)
run_cmake(DOWNLOAD-httpheader-not-set)
run_cmake(DOWNLOAD-netrc-bad)
run_cmake(DOWNLOAD-tls-cainfo-not-set)
run_cmake(DOWNLOAD-tls-verify-not-set)
run_cmake(DOWNLOAD-pass-not-set)
run_cmake(DOWNLOAD-no-save-hash)
run_cmake(TOUCH)
run_cmake(TOUCH-error-in-source-directory)
run_cmake(TOUCH-error-missing-directory)
run_cmake(UPLOAD-unused-argument)
run_cmake(UPLOAD-httpheader-not-set)
run_cmake(UPLOAD-netrc-bad)
run_cmake(UPLOAD-tls-cainfo-not-set)
run_cmake(UPLOAD-tls-verify-not-set)
run_cmake(UPLOAD-pass-not-set)
run_cmake(INSTALL-DIRECTORY)
run_cmake(INSTALL-FILES_FROM_DIR)
run_cmake(INSTALL-FILES_FROM_DIR-bad)
run_cmake(INSTALL-MESSAGE-bad)
run_cmake(FileOpenFailRead)
run_cmake(LOCK)
run_cmake(LOCK-error-file-create-fail)
run_cmake(LOCK-error-guard-incorrect)
run_cmake(LOCK-error-incorrect-timeout)
run_cmake(LOCK-error-incorrect-timeout-trail)
run_cmake(LOCK-error-lock-fail)
run_cmake(LOCK-error-negative-timeout)
run_cmake(LOCK-error-no-function)
run_cmake(LOCK-error-no-guard)
run_cmake(LOCK-error-no-path)
run_cmake(LOCK-error-no-result-variable)
run_cmake(LOCK-error-no-timeout)
run_cmake(LOCK-error-timeout)
run_cmake(LOCK-error-unknown-option)
run_cmake(LOCK-lowercase)
run_cmake(READ_ELF)
run_cmake(GLOB)
run_cmake(GLOB_RECURSE)
run_cmake(GLOB_RECURSE-noexp-FOLLOW_SYMLINKS)
run_cmake(SIZE)
run_cmake(SIZE-error-does-not-exist)

run_cmake(REMOVE-empty)

run_cmake_script(COPY_FILE-file-replace)
run_cmake_script(COPY_FILE-dir-to-file-capture)
run_cmake_script(COPY_FILE-dir-to-file-fail)
run_cmake_script(COPY_FILE-dirlink-to-file-capture)
run_cmake_script(COPY_FILE-dirlink-to-file-fail)
run_cmake_script(COPY_FILE-file-to-file)
run_cmake_script(COPY_FILE-file-to-dir-capture)
run_cmake_script(COPY_FILE-file-to-dir-fail)
run_cmake_script(COPY_FILE-file-ONLY_IF_DIFFERENT-capture)
run_cmake_script(COPY_FILE-file-ONLY_IF_DIFFERENT-fail)
run_cmake_script(COPY_FILE-file-ONLY_IF_DIFFERENT-no-overwrite)
run_cmake_script(COPY_FILE-link-to-file)
run_cmake_script(COPY_FILE-arg-missing)
run_cmake_script(COPY_FILE-arg-unknown)

run_cmake_script(RENAME-file-replace)
run_cmake_script(RENAME-file-to-file)
run_cmake_script(RENAME-file-to-dir-capture)
run_cmake_script(RENAME-file-to-dir-fail)
run_cmake_script(RENAME-file-NO_REPLACE-capture)
run_cmake_script(RENAME-file-NO_REPLACE-fail)
run_cmake_script(RENAME-arg-missing)
run_cmake_script(RENAME-arg-unknown)

# tests are valid both for GLOB and GLOB_RECURSE
run_cmake(GLOB-sort-dedup)
run_cmake(GLOB-error-LIST_DIRECTORIES-not-boolean)
run_cmake(GLOB-error-LIST_DIRECTORIES-no-arg)
run_cmake(GLOB-error-RELATIVE-no-arg)
run_cmake(GLOB-error-CONFIGURE_DEPENDS-modified)
run_cmake(GLOB-noexp-CONFIGURE_DEPENDS)
run_cmake(GLOB-noexp-FOLLOW_SYMLINKS)
run_cmake(GLOB-noexp-LIST_DIRECTORIES)
run_cmake(GLOB-noexp-RELATIVE)
run_cmake_command(GLOB-error-CONFIGURE_DEPENDS-SCRIPT_MODE ${CMAKE_COMMAND} -P
  ${RunCMake_SOURCE_DIR}/GLOB-error-CONFIGURE_DEPENDS-SCRIPT_MODE.cmake)

if(NOT WIN32
    AND NOT MSYS # FIXME: This works on CYGWIN but not on MSYS
    )
  run_cmake(CREATE_LINK-SYMBOLIC)
  run_cmake(CREATE_LINK-SYMBOLIC-noexist)
  run_cmake(GLOB_RECURSE-cyclic-recursion)
  run_cmake(INSTALL-SYMLINK)
  run_cmake(READ_SYMLINK)
  run_cmake(READ_SYMLINK-noexist)
  run_cmake(READ_SYMLINK-notsymlink)
  run_cmake(INSTALL-FOLLOW_SYMLINK_CHAIN)
endif()

run_cmake(REAL_PATH-unexpected-arg)
run_cmake(REAL_PATH-no-base-dir)
run_cmake(REAL_PATH)

if(RunCMake_GENERATOR MATCHES "Ninja")
  # Detect ninja version so we know what tests can be supported.
  execute_process(
    COMMAND "${RunCMake_MAKE_PROGRAM}" --version
    OUTPUT_VARIABLE ninja_out
    ERROR_VARIABLE ninja_out
    RESULT_VARIABLE ninja_res
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(ninja_res EQUAL 0 AND "x${ninja_out}" MATCHES "^x[0-9]+\\.[0-9]+")
    set(ninja_version "${ninja_out}")
    message(STATUS "ninja version: ${ninja_version}")
  else()
    message(FATAL_ERROR "'ninja --version' reported:\n${ninja_out}")
  endif()

  if("${ninja_version}" VERSION_LESS 1.8)
    message(STATUS "Ninja is too old for GLOB CONFIGURE_DEPENDS; expect a warning.")
  endif()
endif()

if(RunCMake_GENERATOR MATCHES "Ninja" AND "${ninja_version}" VERSION_LESS 1.8)
  run_cmake(GLOB_RECURSE-warn-CONFIGURE_DEPENDS-ninja-version)
else()
  run_cmake(GLOB-warn-CONFIGURE_DEPENDS-late)

  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/GLOB-CONFIGURE_DEPENDS-RerunCMake-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_DEFAULT_stderr ".*")
  if(RunCMake_GENERATOR STREQUAL "Borland Makefiles" OR
     RunCMake_GENERATOR STREQUAL "Watcom WMake")
    set(fs_delay 3)
  else()
    set(fs_delay 1.125)
  endif()

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/test")
  set(tf_1  "${RunCMake_TEST_BINARY_DIR}/test/1.txt")
  file(WRITE "${tf_1}" "1")

  message(STATUS "GLOB-CONFIGURE_DEPENDS-RerunCMake: first configuration...")
  run_cmake(GLOB-CONFIGURE_DEPENDS-RerunCMake)
  run_cmake_command(GLOB-CONFIGURE_DEPENDS-RerunCMake-build ${CMAKE_COMMAND} --build .)

  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay})
  message(STATUS "GLOB-CONFIGURE_DEPENDS-RerunCMake: add another file...")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/test/sub")
  set(tf_2  "${RunCMake_TEST_BINARY_DIR}/test/sub/2.txt")
  file(WRITE "${tf_2}" "2")
  run_cmake_command(GLOB-CONFIGURE_DEPENDS-RerunCMake-rebuild_first ${CMAKE_COMMAND} --build .)
  run_cmake_command(GLOB-CONFIGURE_DEPENDS-RerunCMake-nowork ${CMAKE_COMMAND} --build .)

  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay})
  message(STATUS "GLOB-CONFIGURE_DEPENDS-RerunCMake: remove first test file...")
  file(REMOVE "${tf_1}")
  run_cmake_command(GLOB-CONFIGURE_DEPENDS-RerunCMake-rebuild_second ${CMAKE_COMMAND} --build .)
  run_cmake_command(GLOB-CONFIGURE_DEPENDS-RerunCMake-nowork ${CMAKE_COMMAND} --build .)

  if(NOT WIN32
      AND NOT MSYS # FIXME: This works on CYGWIN but not on MSYS
      )
    message(STATUS "GLOB-CONFIGURE_DEPENDS-CMP0009-RerunCMake: link the first test directory into a new directory...")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/test2")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${RunCMake_TEST_BINARY_DIR}/test" "${RunCMake_TEST_BINARY_DIR}/test2/test_folder_symlink")

    message(STATUS "GLOB-CONFIGURE_DEPENDS-CMP0009-RerunCMake: first configuration...")
    run_cmake(GLOB-CONFIGURE_DEPENDS-CMP0009-RerunCMake)
    run_cmake_command(GLOB-CONFIGURE_DEPENDS-CMP0009-RerunCMake-build ${CMAKE_COMMAND} --build .)

    message(STATUS "GLOB-CONFIGURE_DEPENDS-CMP0009-RerunCMake: add another file in the linked directory...")
    set(tf_3  "${RunCMake_TEST_BINARY_DIR}/test/3.txt")
    file(WRITE "${tf_3}" "3")
    run_cmake_command(GLOB-CONFIGURE_DEPENDS-CMP0009-RerunCMake-rebuild ${CMAKE_COMMAND} --build .)
  endif()

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_DEFAULT_stderr)
endif()
