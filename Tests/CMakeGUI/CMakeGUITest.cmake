function(run_cmake_gui_test name)
  if(DEFINED ENV{CMakeGUITest_TEST_FILTER} AND NOT name MATCHES "$ENV{CMakeGUITest_TEST_FILTER}")
    return()
  endif()

  set(_fail)

  cmake_parse_arguments(_rcgt
    "DO_CONFIGURE"
    "GENERATOR"
    "ARGS;CONFIGURE_ARGS"
    ${ARGN}
    )

  string(REPLACE ":" "-" _file_name "${name}")
  set(_srcdir "${CMakeGUITest_SOURCE_DIR}/${_file_name}")
  set(_workdir "${CMakeGUITest_BINARY_DIR}/${_file_name}")

  file(REMOVE_RECURSE "${_workdir}")
  file(MAKE_DIRECTORY "${_workdir}")

  set(_ini_in "${_srcdir}/CMakeSetup.ini.in")
  if(EXISTS "${_ini_in}")
    configure_file("${_ini_in}" "${_workdir}/config/Kitware/CMakeSetup.ini" @ONLY)
  endif()
  set(_cmakelists_in "${_srcdir}/CMakeLists.txt.in")
  if(EXISTS "${_cmakelists_in}")
    configure_file("${_cmakelists_in}" "${_workdir}/src/CMakeLists.txt" @ONLY)
  endif()
  set(_cmakepresets_in "${_srcdir}/CMakePresets.json.in")
  if(EXISTS "${_cmakepresets_in}")
    configure_file("${_cmakepresets_in}" "${_workdir}/src/CMakePresets.json" @ONLY)
  endif()
  if(_rcgt_DO_CONFIGURE)
    if(NOT _rcgt_GENERATOR)
      set(_rcgt_GENERATOR "${CMakeGUITest_GENERATOR}")
    endif()
    execute_process(
      COMMAND "${CMAKE_COMMAND}"
        -S "${_workdir}/src"
        -B "${_workdir}/build"
        -G "${_rcgt_GENERATOR}"
        ${_rcgt_CONFIGURE_ARGS}
      RESULT_VARIABLE _result
      OUTPUT_VARIABLE _output
      ERROR_VARIABLE _error
      )
    if(_result)
      set(_fail 1)
      string(REPLACE "\n" "\n  " _formatted_output "${_output}")
      string(REPLACE "\n" "\n  " _formatted_error "${_error}")
      message(SEND_ERROR
        "Configuring ${_workdir}/src failed with exit code ${_result}, should be 0\n"
        "stdout:\n  ${_formatted_output}\n"
        "stderr:\n  ${_formatted_error}"
        )
    endif()
  endif()

  set(ENV{CMake_GUI_TEST_NAME} "${name}")
  set(ENV{CMake_GUI_CONFIG_DIR} "${_workdir}/config")
  execute_process(
    COMMAND "${CMakeGUITest_COMMAND}" ${_rcgt_ARGS}
    WORKING_DIRECTORY "${_workdir}"
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _output
    ERROR_VARIABLE _error
    )
  if(_result)
    set(_fail 1)
    string(REPLACE "\n" "\n  " _formatted_output "${_output}")
    string(REPLACE "\n" "\n  " _formatted_error "${_error}")
    message(SEND_ERROR "CMake GUI test ${name} failed with exit code ${_result}, should be 0\n"
    "stdout:\n  ${_formatted_output}\n"
    "stderr:\n  ${_formatted_error}"
    )
  endif()

  if(NOT _fail)
    message(STATUS "${name} -- passed")
  endif()
endfunction()

run_cmake_gui_test(sourceBinaryArgs:sourceAndBinaryDir
  ARGS
    -S "${CMakeGUITest_BINARY_DIR}/sourceBinaryArgs-sourceAndBinaryDir/src"
    -B "${CMakeGUITest_BINARY_DIR}/sourceBinaryArgs-sourceAndBinaryDir/build"
  )
run_cmake_gui_test(sourceBinaryArgs:sourceAndBinaryDirRelative
  ARGS
    "-Ssrc"
    "-Bbuild"
  )
run_cmake_gui_test(sourceBinaryArgs:sourceDir
  ARGS
    "${CMakeGUITest_BINARY_DIR}/sourceBinaryArgs-sourceDir/src"
  )
run_cmake_gui_test(sourceBinaryArgs:binaryDir
  DO_CONFIGURE
  ARGS
    "${CMakeGUITest_BINARY_DIR}/sourceBinaryArgs-binaryDir/build"
  )
run_cmake_gui_test(sourceBinaryArgs:noExist
  ARGS
    "${CMakeGUITest_BINARY_DIR}/sourceBinaryArgs-noExist/noexist"
  )
run_cmake_gui_test(sourceBinaryArgs:noExistConfig
  ARGS
    "${CMakeGUITest_BINARY_DIR}/sourceBinaryArgs-noExistConfig/noexist"
  )
run_cmake_gui_test(sourceBinaryArgs:noExistConfigExists
  DO_CONFIGURE
  ARGS
    "${CMakeGUITest_BINARY_DIR}/sourceBinaryArgs-noExistConfigExists/noexist"
  )

run_cmake_gui_test(simpleConfigure:success)
run_cmake_gui_test(simpleConfigure:fail)

unset(ENV{ADDED_VARIABLE})
set(ENV{KEPT_VARIABLE} "Kept variable")
set(ENV{CHANGED_VARIABLE} "This variable will be changed")
set(ENV{REMOVED_VARIABLE} "Removed variable")
run_cmake_gui_test(environment)

run_cmake_gui_test(presetArg:preset
  ARGS
    -S "${CMakeGUITest_BINARY_DIR}/presetArg-preset/src"
    "--preset=ninja"
  )
run_cmake_gui_test(presetArg:presetBinary
  ARGS
    -S "${CMakeGUITest_BINARY_DIR}/presetArg-presetBinary/src"
    -B "${CMakeGUITest_BINARY_DIR}/presetArg-presetBinary/build"
    "--preset=ninja"
  )
run_cmake_gui_test(presetArg:presetBinaryChange
  ARGS
    -S "${CMakeGUITest_BINARY_DIR}/presetArg-presetBinaryChange/src"
    -B "${CMakeGUITest_BINARY_DIR}/presetArg-presetBinaryChange/build"
    "--preset=ninja"
  )
run_cmake_gui_test(presetArg:noPresetBinaryChange
  ARGS
    -S "${CMakeGUITest_BINARY_DIR}/presetArg-noPresetBinaryChange/src"
    -B "${CMakeGUITest_BINARY_DIR}/presetArg-noPresetBinaryChange/build"
  )
run_cmake_gui_test(presetArg:presetConfigExists
  ARGS
    -S "${CMakeGUITest_BINARY_DIR}/presetArg-presetConfigExists/src"
    "--preset=ninja"
  )
run_cmake_gui_test(presetArg:noExist
  ARGS
    -S "${CMakeGUITest_BINARY_DIR}/presetArg-noExist/src"
    "--preset=noExist"
  )
run_cmake_gui_test(changingPresets)
