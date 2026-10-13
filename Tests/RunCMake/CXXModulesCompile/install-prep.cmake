function (run_cxx_module_install_prep kind case component config expect_build)
  cmake_parse_arguments(PARSE_ARGV 5 prep
    "EXCLUDE_FROM_ALL;EMPTY_DESTINATION;NO_MODULE_DIRECTORY" "CONFIGURATIONS" "")
  set(test "install-prep-${kind}-${case}")
  if (kind STREQUAL "GeneratedSource")
    set(directory install-prep)
    set(source_destination sources)
  elseif (kind STREQUAL "BMI")
    set(directory install-prep-BMI)
    set(module_target install_bmi)
    set(source_destination sources)
  else ()
    set(directory install-prep-Metadata)
    set(module_target export_interfaces)
    set(source_destination lib/cxx/miu)
  endif ()
  set(RunCMake_TEST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/${directory}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test}-build")
  set(prefix "${RunCMake_BINARY_DIR}/${test}-install")
  file(REMOVE_RECURSE "${prefix}")

  set(rule_options)
  if (prep_EXCLUDE_FROM_ALL)
    list(APPEND rule_options EXCLUDE_FROM_ALL)
  endif ()
  if (prep_CONFIGURATIONS)
    list(APPEND rule_options CONFIGURATIONS "${prep_CONFIGURATIONS}")
  endif ()
  string(REPLACE ";" "\\;" rule_options "${rule_options}")
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_INSTALL_PREFIX=${prefix}"
    "-Dprep_test=ON"
    "-Dprep_rule_options=${rule_options}"
    "-Dprep_empty_destination=${prep_EMPTY_DESTINATION}"
    "-Dprep_no_module_directory=${prep_NO_MODULE_DIRECTORY}"
    "-DCMake_TEST_MODULE_COMPILATION_RULES=${CMake_TEST_MODULE_COMPILATION_RULES}"
    )
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS "-DCMAKE_CONFIGURATION_TYPES=${config}")
  else ()
    list(APPEND RunCMake_TEST_OPTIONS "-DCMAKE_BUILD_TYPE=${config}")
  endif ()
  run_cmake("${test}")

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake-check-file "install-prep-build-check.cmake")
  run_cmake_command("${test}-build" "${CMAKE_COMMAND}"
    --build . --config "${config}" --target "install_prep/${component}")
  unset(RunCMake-check-file)

  # Component-specific BMI installation has separate limitations.  Check the
  # prepared BMI above; exercise installation with the full-install case.
  if ((expect_build AND (kind STREQUAL "Metadata" OR component STREQUAL "all"))
      OR kind STREQUAL "GeneratedSource")
    set(install_args)
    if (NOT component STREQUAL "all")
      list(APPEND install_args --component "${component}")
    endif ()
    set(RunCMake-check-file "install-prep-install-check.cmake")
    run_cmake_command("${test}-install" "${CMAKE_COMMAND}"
      --install . --config "${config}" ${install_args})
  endif ()
endfunction ()

# Preparing a generated module source must not compile it to a BMI.
if ("named" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_install_prep(GeneratedSource sources Sources Debug 0)
endif ()

if ("install_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_install_prep(BMI all all Debug 1)
  run_cxx_module_install_prep(BMI component BMI Debug 1)
  run_cxx_module_install_prep(BMI excluded-all all Debug 0 EXCLUDE_FROM_ALL)
  run_cxx_module_install_prep(BMI excluded-component BMI Debug 1 EXCLUDE_FROM_ALL)
  run_cxx_module_install_prep(BMI config-inactive BMI Debug 0 CONFIGURATIONS Release)
  run_cxx_module_install_prep(BMI config-active BMI Release 1 CONFIGURATIONS Release)
  run_cxx_module_install_prep(BMI sources Sources Debug 0)
  run_cxx_module_install_prep(BMI empty-destination BMI Debug 0 EMPTY_DESTINATION)
endif ()

if ("export_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_install_prep(Metadata all all Debug 1)
  run_cxx_module_install_prep(Metadata component Metadata Debug 1)
  run_cxx_module_install_prep(Metadata excluded-all all Debug 0 EXCLUDE_FROM_ALL)
  run_cxx_module_install_prep(Metadata excluded-component Metadata Debug 1 EXCLUDE_FROM_ALL)
  run_cxx_module_install_prep(Metadata config-inactive Metadata Debug 0 CONFIGURATIONS Release)
  run_cxx_module_install_prep(Metadata config-active Metadata Release 1 CONFIGURATIONS Release)
  run_cxx_module_install_prep(Metadata sources Sources Debug 0)
  run_cxx_module_install_prep(Metadata no-module-directory Metadata Debug 0 NO_MODULE_DIRECTORY)
endif ()
