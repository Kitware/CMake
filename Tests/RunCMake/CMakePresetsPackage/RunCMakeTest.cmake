include(RunCMake)

# Presets do not support legacy VS generator name architecture suffix.
if(RunCMake_GENERATOR MATCHES "^(Visual Studio [0-9]+ [0-9]+) ")
  set(RunCMake_GENERATOR "${CMAKE_MATCH_1}")
endif()

function(run_cmake_package_presets name CMakePresetsPackage_CONFIGURE_PRESETS CMakePresetsPackage_BUILD_PRESETS CMakePresetsPackage_PACKAGE_PRESETS)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_TEST_SOURCE_DIR}/build")
  set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")

  set(RunCMake_TEST_NO_CLEAN TRUE)

  file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  set(CASE_NAME "${name}")
  set(CASE_SOURCE_DIR "${RunCMake_SOURCE_DIR}")
  configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt.in" "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt" @ONLY)

  if(NOT CMakePresetsPackage_FILE)
    set(CMakePresetsPackage_FILE "${RunCMake_SOURCE_DIR}/${name}.json.in")
  endif()
  if(EXISTS "${CMakePresetsPackage_FILE}")
    configure_file("${CMakePresetsPackage_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json" @ONLY)
  endif()

  if(NOT CMakeUserPresets_FILE)
    set(CMakeUserPresets_FILE "${RunCMake_SOURCE_DIR}/${name}User.json.in")
  endif()
  if(EXISTS "${CMakeUserPresets_FILE}")
    configure_file("${CMakeUserPresets_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakeUserPresets.json" @ONLY)
  endif()

  foreach(ASSET ${CMakePresetsPackage_ASSETS})
    configure_file("${RunCMake_SOURCE_DIR}/${ASSET}" "${RunCMake_TEST_SOURCE_DIR}" COPYONLY)
  endforeach()

  if (NOT CMakePresetsPackage_NO_CONFIGURE)
    foreach(CONFIGURE_PRESET ${CMakePresetsPackage_CONFIGURE_PRESETS})
      run_cmake_command("${name}-configure-${CONFIGURE_PRESET}"
        "${CMAKE_COMMAND}" "--preset" "${CONFIGURE_PRESET}")
    endforeach()
  endif()

  if (NOT CMakePresetsPackage_NO_BUILD)
    foreach(BUILD_PRESET ${CMakePresetsPackage_BUILD_PRESETS})
      run_cmake_command("${name}-build-${BUILD_PRESET}"
        "${CMAKE_COMMAND}" "--build" "--preset" "${BUILD_PRESET}")
    endforeach()
  endif()

  set(eq 0)
  foreach(PACKAGE_PRESET ${CMakePresetsPackage_PACKAGE_PRESETS})
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}/default/_CPack_Packages")

    if (EXISTS "${RunCMake_SOURCE_DIR}/${name}-package-${PACKAGE_PRESET}-check.cmake")
      set(RunCMake-check-file "${name}-package-${PACKAGE_PRESET}-check.cmake")
    else()
      set(RunCMake-check-file "check.cmake")
    endif()

    if(eq)
      run_cmake_command(${name}-package-${PACKAGE_PRESET}
        ${CMAKE_CPACK_COMMAND} "--preset=${PACKAGE_PRESET}" ${ARGN})
      set(eq 0)
    else()
      run_cmake_command(${name}-package-${PACKAGE_PRESET}
        ${CMAKE_CPACK_COMMAND} "--preset" "${PACKAGE_PRESET}" ${ARGN})
      set(eq 1)
    endif()
  endforeach()
endfunction()

function(check_cpack_packages generators contents)
  include("${RunCMake_TEST_BINARY_DIR}/default/CPackConfig.cmake")

  set(cpack_dir "${RunCMake_TEST_BINARY_DIR}/default/_CPack_Packages/${CPACK_TOPLEVEL_TAG}")
  file(GLOB dirs RELATIVE "${cpack_dir}" "${cpack_dir}/*")
  if(NOT dirs STREQUAL generators)
    string(APPEND RunCMake_TEST_FAILED "Expected CPack generators: ${generators}\nActual CPack generators: ${dirs}\n")
  endif()

  if(contents)
    foreach(dir IN LISTS dirs)
      set(env_file "${cpack_dir}/${dir}/${CPACK_PACKAGE_FILE_NAME}/env.txt")
      file(READ "${env_file}" actual_contents)
      if(NOT contents STREQUAL actual_contents)
        string(REPLACE "\n" "\n  " contents_formatted "${contents}")
        string(REPLACE "\n" "\n  " actual_contents_formatted "${actual_contents}")
        string(APPEND RunCMake_TEST_FAILED "Expected contents of ${env_file}:\n  ${contents_formatted}\nActual contents:\n  ${actual_contents_formatted}\n")
      endif()
    endforeach()
  endif()

  set(RunCMake_TEST_FAILED ${RunCMake_TEST_FAILED} PARENT_SCOPE)
endfunction()

run_cmake_package_presets(UnsupportedVersion "x" "" "")
run_cmake_package_presets(Good "default" "build-default-debug" "no-environment;with-environment;generators;configurations;variables;config-file;debug;verbose;package-name;package-version;package-directory;vendor-name")
run_cmake_package_presets(ListPresets "default" "" "x" "--list-presets")
