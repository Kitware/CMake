cmake_policy(SET CMP0057 NEW)

function(run_cpack_test_common_ TEST_NAME types build SUBTEST_SUFFIX source PACKAGING_TYPE package_target)
  if(TEST_TYPE IN_LIST types)
    string(REGEX MATCH "^[^.]*" GENERATOR_TYPE "${TEST_TYPE}")
    set(RunCMake_TEST_NO_CLEAN TRUE)
    if(package_target)
      set(full_test_name_ "${TEST_NAME}-package-target")
    else()
      set(full_test_name_ "${TEST_NAME}")
    endif()
    set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${full_test_name_}-build")

    if(SUBTEST_SUFFIX)
      set(RunCMake_TEST_BINARY_DIR "${RunCMake_TEST_BINARY_DIR}-${SUBTEST_SUFFIX}-subtest")
      set(full_test_name_ "${full_test_name_}-${SUBTEST_SUFFIX}-subtest")
    endif()

    string(APPEND full_test_name_ "-${PACKAGING_TYPE}-type")

     # TODO this should be executed only once per ctest run (not per generator)
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

    if(EXISTS "${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/${GENERATOR_TYPE}-Prerequirements.cmake")
      include("${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/${GENERATOR_TYPE}-Prerequirements.cmake")

      set(FOUND_PREREQUIREMENTS false)
      get_test_prerequirements("FOUND_PREREQUIREMENTS" "${config_file}")

      # skip the test if prerequirements are not met
      if(NOT FOUND_PREREQUIREMENTS)
        message(STATUS "${TEST_NAME} - SKIPPED")
        return()
      endif()
    endif()

    # execute cmake
    set(RunCMake_TEST_OPTIONS "-DGENERATOR_TYPE=${GENERATOR_TYPE}"
      "-DRunCMake_TEST_FILE_PREFIX=${TEST_NAME}"
      "-DRunCMake_SUBTEST_SUFFIX=${SUBTEST_SUFFIX}"
      "-DPACKAGING_TYPE=${PACKAGING_TYPE}")

    foreach(o out err)
      if(SUBTEST_SUFFIX AND EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/configure-${PACKAGING_TYPE}-${SUBTEST_SUFFIX}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/configure-${PACKAGING_TYPE}-${SUBTEST_SUFFIX}-std${o}.txt")
      elseif(SUBTEST_SUFFIX AND EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/configure-${SUBTEST_SUFFIX}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/configure-${SUBTEST_SUFFIX}-std${o}.txt")
      elseif(EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/configure-${PACKAGING_TYPE}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/configure-${PACKAGING_TYPE}-std${o}.txt")
      elseif(EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/configure-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/configure-std${o}.txt")
      else()
        unset(RunCMake-std${o}-file)
      endif()
    endforeach()

    run_cmake(${full_test_name_})

    # execute optional build step
    if(build)
      unset(RunCMake-stdout-file)
      unset(RunCMake-stderr-file)
      run_cmake_command(${full_test_name_}-Build "${CMAKE_COMMAND}" --build "${RunCMake_TEST_BINARY_DIR}")
    endif()

    if(source)
      set(pack_params_ -G ${GENERATOR_TYPE} --config ./CPackSourceConfig.cmake)
      FILE(APPEND ${RunCMake_TEST_BINARY_DIR}/CPackSourceConfig.cmake
        "\nset(CPACK_RPM_SOURCE_PKG_BUILD_PARAMS \"-DRunCMake_TEST:STRING=${full_test_name_} -DRunCMake_TEST_FILE_PREFIX:STRING=${TEST_NAME} -DGENERATOR_TYPE:STRING=${GENERATOR_TYPE}\")")
    else()
      unset(pack_params_)
    endif()

    if(package_target)
      set(cpack_command_ ${CMAKE_COMMAND} --build "${RunCMake_TEST_BINARY_DIR}" --target package)
    else()
      set(cpack_command_ ${CMAKE_CPACK_COMMAND} ${pack_params_} -C Debug)
    endif()

    # execute cpack
    set(SETENV)
    if(ENVIRONMENT)
      set(SETENV ${CMAKE_COMMAND} -E env "${ENVIRONMENT}")
    endif()
    execute_process(
      COMMAND ${SETENV} ${cpack_command_}
      WORKING_DIRECTORY "${RunCMake_TEST_BINARY_DIR}"
      RESULT_VARIABLE "result_"
      OUTPUT_FILE "${RunCMake_TEST_BINARY_DIR}/test_output.txt"
      ERROR_FILE "${RunCMake_TEST_BINARY_DIR}/test_error.txt"
      )

    foreach(o out err)
      if(SUBTEST_SUFFIX AND EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/${GENERATOR_TYPE}-${PACKAGING_TYPE}-${SUBTEST_SUFFIX}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/${GENERATOR_TYPE}-${PACKAGING_TYPE}-${SUBTEST_SUFFIX}-std${o}.txt")
      elseif(EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/${GENERATOR_TYPE}-${PACKAGING_TYPE}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/${GENERATOR_TYPE}-${PACKAGING_TYPE}-std${o}.txt")
      elseif(SUBTEST_SUFFIX AND EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/${GENERATOR_TYPE}-${SUBTEST_SUFFIX}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/${GENERATOR_TYPE}-${SUBTEST_SUFFIX}-std${o}.txt")
      elseif(EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/${GENERATOR_TYPE}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/${GENERATOR_TYPE}-std${o}.txt")
      elseif(SUBTEST_SUFFIX AND EXISTS ${RunCMake_SOURCE_DIR}/tests/${TEST_NAME}/${SUBTEST_SUFFIX}-std${o}.txt)
        set(RunCMake-std${o}-file "tests/${TEST_NAME}/${SUBTEST_SUFFIX}-std${o}.txt")
      elseif(EXISTS ${RunCMake_SOURCE_DIR}/${GENERATOR_TYPE}/default_expected_std${o}.txt)
        set(RunCMake-std${o}-file "${GENERATOR_TYPE}/default_expected_std${o}.txt")
      else()
        unset(RunCMake-std${o}-file)
      endif()
    endforeach()

    # verify result
    run_cmake_command(
      ${GENERATOR_TYPE}/${full_test_name_}
      "${CMAKE_COMMAND}"
        -DRunCMake_TEST=${full_test_name_}
        -DRunCMake_TEST_FILE_PREFIX=${TEST_NAME}
        -DRunCMake_SUBTEST_SUFFIX=${SUBTEST_SUFFIX}
        -DGENERATOR_TYPE=${GENERATOR_TYPE}
        -DPACKAGING_TYPE=${PACKAGING_TYPE}
        "-Dsrc_dir=${RunCMake_SOURCE_DIR}"
        "-Dbin_dir=${RunCMake_TEST_BINARY_DIR}"
        "-Dconfig_file=${config_file}"
        -P "${RunCMake_SOURCE_DIR}/VerifyResult.cmake"
      )
  endif()
endfunction()

function(run_cpack_test TEST_NAME types build PACKAGING_TYPES)
  foreach(packaging_type_ IN LISTS PACKAGING_TYPES)
    run_cpack_test_common_("${TEST_NAME}" "${types}" "${build}" "" false "${packaging_type_}" false)
  endforeach()
endfunction()

function(run_cpack_test_package_target TEST_NAME types build PACKAGING_TYPES)
  foreach(packaging_type_ IN LISTS PACKAGING_TYPES)
    run_cpack_test_common_("${TEST_NAME}" "${types}" "${build}" "" false "${packaging_type_}" true)
  endforeach()
endfunction()

function(run_cpack_test_subtests TEST_NAME SUBTEST_SUFFIXES types build PACKAGING_TYPES)
  foreach(suffix_ IN LISTS SUBTEST_SUFFIXES)
    foreach(packaging_type_ IN LISTS PACKAGING_TYPES)
      run_cpack_test_common_("${TEST_NAME}" "${types}" "${build}" "${suffix_}" false "${packaging_type_}" false)
    endforeach()
  endforeach()
endfunction()

function(run_cpack_source_test TEST_NAME types)
  run_cpack_test_common_("${TEST_NAME}" "${types}" false "" true "" false)
endfunction()
