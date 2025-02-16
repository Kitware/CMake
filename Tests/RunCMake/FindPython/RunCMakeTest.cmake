include(RunCMake)

if(RunCMake_BUILD_TYPE)
  set(Python_BUILD_TYPE "${RunCMake_BUILD_TYPE}")
else()
  set(Python_BUILD_TYPE "Release")
endif()

function(run_python test)
  set(options_args CHECK_RESULT)
  set(one_value_args TYPE ACTION VARIANT STRATEGY)
  set(multi_value_args OPTIONS)
  cmake_parse_arguments(PARSE_ARGV 1 RP "${options_args}" "${one_value_args}" "${multi_value_args}")

  if(RP_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "run_python: unparsed arguments: ${RP_UNPARSED_ARGUMENTS}")
  endif()

  set(test_name "${test}")
  if(RP_VARIANT)
    string(APPEND test_name ".${RP_VARIANT}")
    set(RunCMake_TEST_VARIANT_DESCRIPTION ".${RP_VARIANT}")
  endif()

  set(options ${RP_OPTIONS})
  if(RP_STRATEGY)
    string(APPEND test_name ".${RP_STRATEGY}")
    string(APPEND RunCMake_TEST_VARIANT_DESCRIPTION ".${RP_STRATEGY}")
    if (NOT RP_TYPE)
      set(RP_TYPE ${test})
    endif()
    list(APPEND options -D${RP_TYPE}_FIND_STRATEGY=${RP_STRATEGY})
  endif()
  if(RP_TYPE)
    list(APPEND options -DPYTHON=${RP_TYPE})
  endif()
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND options -DCMAKE_BUILD_TYPE=${Python_BUILD_TYPE})
  endif()

  if(RP_CHECK_RESULT)
    set(RunCMake_TEST_EXPECT_RESULT 1)
    file(READ "${RunCMake_SOURCE_DIR}/${test_name}-stderr.txt" RunCMake_TEST_EXPECT_stderr)
  endif()

  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test_name}-build")
  if(options)
    run_cmake_with_options(${test} ${options})
  else()
    run_cmake(${test})
  endif()
  if(NOT RP_ACTION)
    return()
  endif()

  set(RunCMake_TEST_NO_CLEAN 1)
  unset(RunCMake_TEST_VARIANT_DESCRIPTION)
  run_cmake_command(${test_name}-build ${CMAKE_COMMAND} --build . --config ${Python_BUILD_TYPE})
  if(RP_ACTION STREQUAL "BUILD")
    return()
  endif()

  run_cmake_command(${test_name}-run ${CMAKE_CTEST_COMMAND} -C ${Python_BUILD_TYPE} -V)
endfunction()


function(run_python_with_virtualenv test)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test}-build")
  set(virtual_env "${RunCMake_TEST_BINARY_DIR}/py3venv")
  run_cmake_with_options(${test} "-DPYTHON3_VIRTUAL_ENV=${virtual_env}")
  set(RunCMake_TEST_NO_CLEAN 1)
  set(tests Default Standard)
  if(CMake_TEST_FindPython2)
    list(APPEND tests Only)
  endif()
  if(test MATCHES "Conda")
    set(RunCMake_TEST_VARIANT_DESCRIPTION ".Conda")
    set(init_venv --unset=VIRTUAL_ENV "CONDA_PREFIX=${virtual_env}")
  else()
    set(RunCMake_TEST_VARIANT_DESCRIPTION ".CPython")
    set(init_venv --unset=CONDA_PREFIX "VIRTUAL_ENV=${virtual_env}")
  endif()
  foreach(test IN LISTS tests)
    run_cmake_script(VirtualEnv${test} -E env --unset=PYTHONHOME ${init_venv}
                                       "${CMAKE_COMMAND}" "-DPYTHON3_VIRTUAL_ENV=${virtual_env}")
  endforeach()
  if(CMake_TEST_FindPython2)
    string(APPEND RunCMake_TEST_VARIANT_DESCRIPTION ".Unset")
    run_cmake_script(VirtualEnvOnly -E env --unset=PYTHONHOME --unset=VIRTUAL_ENV --unset=CONDA_PREFIX
                                    "${CMAKE_COMMAND}")
  endif()
endfunction()

macro(required_artifacts_check variant)
  run_python(RequiredArtifactsCheck VARIANT ${variant}
                                    OPTIONS "-DPYTHON_ARTIFACTS=${RunCMake_BINARY_DIR}/RequiredArtifacts-build/PythonArtifacts.cmake"
                                            ${ARGN})
endmacro()

function(required_artifacts)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/RequiredArtifacts-build")
  run_cmake_with_options(RequiredArtifacts "-DCMake_TEST_FindPython2=${CMake_TEST_FindPython2}"
                                           "-DCMake_TEST_FindPython3_SABIModule=${CMake_TEST_FindPython3_SABIModule}")
  if(EXISTS "${RunCMake_TEST_BINARY_DIR}/PythonArtifacts.cmake")
    required_artifacts_check("Interpreter.VALID" -DPYTHON_IS_FOUND=TRUE
                                                 -DCHECK_INTERPRETER=ON)
    required_artifacts_check("Interpreter.INVALID" -DPYTHON_IS_FOUND=FALSE -DINVALID_ARTIFACTS=INTERPRETER
                                                   -DCHECK_INTERPRETER=ON)
    required_artifacts_check("Library.VALID" -DPYTHON_IS_FOUND=TRUE
                                             -DCHECK_LIBRARY=ON)
    required_artifacts_check("Library.INVALID" -DPYTHON_IS_FOUND=FALSE -DINVALID_ARTIFACTS=LIBRARY
                                               -DCHECK_LIBRARY=ON)
    required_artifacts_check("Include.VALID" -DPYTHON_IS_FOUND=TRUE
                                             -DCHECK_INCLUDE=ON)
    required_artifacts_check("Include.INVALID" -DPYTHON_IS_FOUND=FALSE -DINVALID_ARTIFACTS=INCLUDE
                                               -DCHECK_INCLUDE=ON)
    required_artifacts_check("Interpreter.VALID,Library.INVALID" -DPYTHON_IS_FOUND=FALSE -DINVALID_ARTIFACTS=LIBRARY
                                                                 -DCHECK_INTERPRETER=ON -DCHECK_LIBRARY=ON)
    required_artifacts_check("Library.VALID,Include.INVALID" -DPYTHON_IS_FOUND=FALSE -DINVALID_ARTIFACTS=INCLUDE
                                                             -DCHECK_LIBRARY=ON -DCHECK_INCLUDE=ON)
    if (CMake_TEST_FindPython3_SABIModule AND WIN32)
      required_artifacts_check("SABILibrary.VALID" -DPYTHON_IS_FOUND=TRUE
                                                   -DCHECK_SABI_LIBRARY=ON)
      required_artifacts_check("SABILibrary.INVALID" -DPYTHON_IS_FOUND=FALSE -DINVALID_ARTIFACTS=SABI_LIBRARY
                                                     -DCHECK_SABI_LIBRARY=ON)
    endif()
  endif()
endfunction()

macro(custom_failure_message_check name components)
  run_python(CustomFailureMessage VARIANT "${name}" CHECK_RESULT OPTIONS "-DCHECK_COMPONENTS=${components}" ${ARGN})
endmacro()


if(CMake_TEST_FindPython2_CPython)
  run_cmake(Python2-BadComponent)
  run_python(Python2Module ACTION RUN)
  run_python(Python2Embedded ACTION RUN)
  run_python(Python2 STRATEGY LOCATION ACTION RUN)
  run_python(Python2 STRATEGY VERSION ACTION RUN)
  run_python(Python STRATEGY LOCATION VARIANT V2 ACTION RUN OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(Python STRATEGY VERSION VARIANT V2 OPTIONS ACTION RUN OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(ExactVersion TYPE Python2 STRATEGY LOCATION VARIANT Python2
                          OPTIONS -DPython_REQUESTED_VERSION=2.1.2)
  run_python(ExactVersion TYPE Python2 STRATEGY VERSION VARIANT Python2
                          OPTIONS -DPython_REQUESTED_VERSION=2.1.2)
  run_python(ExactVersion TYPE Python STRATEGY LOCATION VARIANT Python.V2
                          OPTIONS -DPython_REQUESTED_VERSION=2.1.2)
  run_python(ExactVersion TYPE Python STRATEGY VERSION VARIANT Python.V2
                          OPTIONS -DPython_REQUESTED_VERSION=2.1.2)
  run_python(VersionRange TYPE Python2 STRATEGY LOCATION VARIANT Python2
                          OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(VersionRange TYPE Python2 STRATEGY VERSION VARIANT Python2
                          OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(VersionRange TYPE Python STRATEGY LOCATION VARIANT Python.V2
                          OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(VersionRange TYPE Python STRATEGY VERSION VARIANT Python.V2
                          OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(CrossCompiling-CMP0190-OLD TYPE Python2 VARIANT Python2)
  run_python(CrossCompiling-CMP0190-NEW TYPE Python2 VARIANT Python2 CHECK_RESULT)
  run_python(CrossCompiling-CMP0190-OLD TYPE Python VARIANT Python.V2
                                        OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(CrossCompiling-CMP0190-NEW TYPE Python VARIANT Python.V2 CHECK_RESULT
                                        OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(CrossCompiling-HOST TYPE Python2 VARIANT Python2)
  run_python(CrossCompiling-HOST TYPE Python VARIANT Python.V2
                                 OPTIONS -DPython_REQUESTED_VERSION=2)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin" AND NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    run_python(CrossCompiling-TARGET TYPE Python2 VARIANT Python2)
    run_python(CrossCompiling-TARGET TYPE Python VARIANT Python.V2
                                     OPTIONS -DPython_REQUESTED_VERSION=2)
    run_python(CrossCompiling-BOTH TYPE Python2 VARIANT Python2)
    run_python(CrossCompiling-BOTH TYPE Python VARIANT Python.V2
                                   OPTIONS -DPython_REQUESTED_VERSION=2)
  endif()
endif()

if(CMake_TEST_FindPython3_CPython)
  run_cmake(Python3-BadComponent)
  run_cmake(DifferentComponents)
  run_python(Python3Module ACTION RUN)
  if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8)
    run_python(Python3Embedded ACTION RUN)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin")
    run_cmake(UnversionedNames)
  endif()
  run_python(Python3 STRATEGY LOCATION ACTION RUN)
  run_python(Python3 STRATEGY VERSION ACTION RUN)
  run_python(Python STRATEGY LOCATION VARIANT V3 ACTION RUN OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(Python STRATEGY VERSION VARIANT V3 ACTION RUN OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(ExactVersion TYPE Python3 STRATEGY LOCATION VARIANT Python3
                          OPTIONS -DPython_REQUESTED_VERSION=3.1.2)
  run_python(ExactVersion TYPE Python3 STRATEGY VERSION VARIANT Python3
                          OPTIONS -DPython_REQUESTED_VERSION=3.1.2)
  run_python(ExactVersion TYPE Python STRATEGY LOCATION VARIANT Python.V3
                          OPTIONS -DPython_REQUESTED_VERSION=3.1.2)
  run_python(ExactVersion TYPE Python STRATEGY VERSION VARIANT Python.V3
                          OPTIONS -DPython_REQUESTED_VERSION=3.1.2)
  run_python(VersionRange TYPE Python3 STRATEGY LOCATION VARIANT Python3
                          OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(VersionRange TYPE Python3 STRATEGY VERSION VARIANT Python3
                          OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(VersionRange TYPE Python STRATEGY LOCATION VARIANT Python.V3
                          OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(VersionRange TYPE Python STRATEGY VERSION VARIANT Python.V3
                          OPTIONS -DPython_REQUESTED_VERSION=3)
  custom_failure_message_check("Interpreter" "Interpreter" -DPython3_EXECUTABLE=/not/found/interpreter)
  custom_failure_message_check("Library" "Development" -DPython3_LIBRARY=/not/found/library)
  custom_failure_message_check("Include" "Development" -DPython3_INCLUDE_DIR=/not/found/include)
  custom_failure_message_check("Multiple" "Interpreter:Development" -DPython3_EXECUTABLE=/not/found/interpreter
                                                                    -DPython3_LIBRARY=/not/found/library)
  run_python(CrossCompiling-CMP0190-OLD TYPE Python3 VARIANT Python3)
  run_python(CrossCompiling-CMP0190-NEW TYPE Python3 VARIANT Python3 CHECK_RESULT)
  run_python(CrossCompiling-CMP0190-OLD TYPE Python VARIANT Python.V3
                                        OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(CrossCompiling-CMP0190-NEW TYPE Python VARIANT Python.V3 CHECK_RESULT
                                        OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(CrossCompiling-HOST TYPE Python3 VARIANT Python3)
  run_python(CrossCompiling-HOST TYPE Python VARIANT Python.V3
                                 OPTIONS -DPython_REQUESTED_VERSION=3)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin" AND NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    run_python(CrossCompiling-TARGET TYPE Python3 VARIANT Python3)
    run_python(CrossCompiling-TARGET TYPE Python VARIANT Python.V3
                                     OPTIONS -DPython_REQUESTED_VERSION=3)
    run_python(CrossCompiling-BOTH TYPE Python3 VARIANT Python3)
    run_python(CrossCompiling-BOTH TYPE Python VARIANT Python.V3
                                        OPTIONS -DPython_REQUESTED_VERSION=3)
  endif()
endif()

if(CMake_TEST_FindPython2_CPython OR CMake_TEST_FindPython3_CPython)
  run_python(Python STRATEGY LOCATION VARIANT Python ACTION RUN)
  run_python(Python STRATEGY VERSION VARIANT Python ACTION RUN)
endif()

if(CMake_TEST_FindPython2_IronPython)
  run_python(IronPython TYPE Python2 STRATEGY LOCATION VARIANT IronPython2)
  run_python(IronPython TYPE Python2 STRATEGY VERSION VARIANT IronPython2)
  run_python(IronPython TYPE Python STRATEGY LOCATION VARIANT V2 OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(IronPython TYPE Python STRATEGY VERSION VARIANT V2 OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(VersionRange TYPE Python2 STRATEGY LOCATION VARIANT IronPython2
                          OPTIONS -DPython_REQUESTED_VERSION=2
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
  run_python(VersionRange TYPE Python2 STRATEGY VERSION VARIANT IronPython2
                          OPTIONS -DPython_REQUESTED_VERSION=2
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
  run_python(VersionRange TYPE Python STRATEGY LOCATION VARIANT IronPython.V2
                          OPTIONS -DPython_REQUESTED_VERSION=2
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
  run_python(VersionRange TYPE Python STRATEGY VERSION VARIANT IronPython.V2
                          OPTIONS -DPython_REQUESTED_VERSION=2
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
endif()

if(CMake_TEST_FindPython3_IronPython)
  run_python(IronPython TYPE Python3 STRATEGY LOCATION VARIANT IronPython3)
  run_python(IronPython TYPE Python3 STRATEGY VERSION VARIANT IronPython3)
  run_python(IronPython TYPE Python STRATEGY LOCATION VARIANT V3 OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(IronPython TYPE Python STRATEGY VERSION VARIANT V3 OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(VersionRange TYPE Python3 STRATEGY LOCATION VARIANT IronPython3
                          OPTIONS -DPython_REQUESTED_VERSION=3
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
  run_python(VersionRange TYPE Python3 STRATEGY VERSION VARIANT IronPython3
                          OPTIONS -DPython_REQUESTED_VERSION=3
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
  run_python(VersionRange TYPE Python STRATEGY LOCATION VARIANT IronPython.V3
                          OPTIONS -DPython_REQUESTED_VERSION=3
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
  run_python(VersionRange TYPE Python STRATEGY VERSION VARIANT IronPython.V3
                          OPTIONS -DPython_REQUESTED_VERSION=3
                                  -DPython2_FIND_IMPLEMENTATIONS=IronPython)
endif()

if(CMake_TEST_FindPython2_IronPython OR CMake_TEST_FindPython3_IronPython)
  run_python(IronPython TYPE Python STRATEGY LOCATION VARIANT IronPython)
  run_python(IronPython TYPE Python STRATEGY VERSION VARIANT IronPython)
endif()

if (CMake_TEST_FindPython2 AND CMake_TEST_FindPython2_IronPython)
  run_python(Implementation VARIANT CPython2 OPTIONS -DPython_REQUESTED_VERSION=2
                                                     -DPython_REQUESTED_IMPLEMENTATIONS=CPython)
  run_python(Implementation VARIANT IronPython2 OPTIONS -DPython_REQUESTED_VERSION=2
                                                        -DPython_REQUESTED_IMPLEMENTATIONS=IronPython)
endif()

if (CMake_TEST_FindPython3 AND CMake_TEST_FindPython3_IronPython)
  run_python(Implementation VARIANT CPython3 OPTIONS -DPython_REQUESTED_VERSION=3
                                                     -DPython_REQUESTED_IMPLEMENTATIONS=CPython)
  run_python(Implementation VARIANT IronPython3 OPTIONS -DPython_REQUESTED_VERSION=3
                                                        -DPython_REQUESTED_IMPLEMENTATIONS=IronPython)
endif()

if(CMake_TEST_FindPython2_PyPy)
  run_python(PyPy TYPE Python2 STRATEGY LOCATION VARIANT PyPy2)
  run_python(PyPy TYPE Python2 STRATEGY VERSION VARIANT PyPy2)
  run_python(PyPy TYPE Python STRATEGY LOCATION VARIANT V2 OPTIONS -DPython_REQUESTED_VERSION=2)
  run_python(PyPy TYPE Python STRATEGY VERSION VARIANT V2 OPTIONS -DPython_REQUESTED_VERSION=2)
endif()

if(CMake_TEST_FindPython3_PyPy)
  run_python(PyPy TYPE Python3 STRATEGY LOCATION VARIANT PyPy3)
  run_python(PyPy TYPE Python3 STRATEGY VERSION VARIANT PyPy3)
  run_python(PyPy TYPE Python STRATEGY LOCATION VARIANT V3 OPTIONS -DPython_REQUESTED_VERSION=3)
  run_python(PyPy TYPE Python STRATEGY VERSION VARIANT V3 OPTIONS -DPython_REQUESTED_VERSION=3)
endif()

if(CMake_TEST_FindPython2_PyPy OR CMake_TEST_FindPython3_PyPy)
  run_python(PyPy TYPE Python STRATEGY LOCATION VARIANT PyPy)
  run_python(PyPy TYPE Python STRATEGY VERSION VARIANT PyPy)
endif()

if(CMake_TEST_FindPython_Various)
  if(CMake_TEST_FindPython3)
    run_python_with_virtualenv(VirtualEnv)
    required_artifacts()
    run_python(ArtifactsInteractive VARIANT "ON"
                                    OPTIONS -DCMake_TEST_FindPython3_NumPy=${CMake_TEST_FindPython3_NumPy}
                                            -DPython3_ARTIFACTS_INTERACTIVE=ON)
    run_python(ArtifactsInteractive VARIANT "OFF"
                                    OPTIONS -DCMake_TEST_FindPython3_NumPy=${CMake_TEST_FindPython3_NumPy}
                                            -DPython3_ARTIFACTS_INTERACTIVE=OFF)
  endif()

  if(CMake_TEST_FindPython2 OR CMake_TEST_FindPython3)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin")
      run_python(SOABI VARIANT "Interpreter" ACTION BUILD
                       OPTIONS -DCMake_TEST_FindPython2=${CMake_TEST_FindPython2}
                               -DCMake_TEST_FindPython3=${CMake_TEST_FindPython3}
                               -DCMake_TEST_FindPython_COMPONENT=Interpreter)
      run_python(SOABI VARIANT "Development" ACTION BUILD
                       OPTIONS -DCMake_TEST_FindPython2=${CMake_TEST_FindPython2}
                               -DCMake_TEST_FindPython3=${CMake_TEST_FindPython3}
                               -DCMake_TEST_FindPython_COMPONENT=Development)
    endif()
    run_python(MultiplePackages ACTION RUN
                                OPTIONS -DCMake_TEST_FindPython2=${CMake_TEST_FindPython2}
                                        -DCMake_TEST_FindPython3=${CMake_TEST_FindPython3})
  endif()

  if(CMake_TEST_FindPython2 AND CMake_TEST_FindPython3)
    run_cmake(ArtifactsPrefix)
  endif()

  if(CMake_TEST_FindPython2_SABIModule)
    run_cmake(Python2SABIModule)
  endif()
  if(CMake_TEST_FindPython3_SABIModule)
    run_python(Python3SABIModule)
  endif()

  if(CMake_TEST_FindPython2_NumPy OR CMake_TEST_FindPython3_NumPy)
    run_python(NumPy ACTION RUN
                     OPTIONS -DCMake_TEST_FindPython2_NumPy=${CMake_TEST_FindPython2_NumPy}
                             -DCMake_TEST_FindPython3_NumPy=${CMake_TEST_FindPython3_NumPy})
    run_python(NumPyOnly ACTION RUN
                         OPTIONS -DCMake_TEST_FindPython2_NumPy=${CMake_TEST_FindPython2_NumPy}
                                 -DCMake_TEST_FindPython3_NumPy=${CMake_TEST_FindPython3_NumPy})
    if(CMake_TEST_FindPython3_NumPy)
      custom_failure_message_check("NumPy" "Interpreter:Development:NumPy" -DPython3_NumPy_INCLUDE_DIR=/not/found/numpy/include)
    endif()
  endif()

  if(CMake_TEST_FindPython3_Conda)
    run_python_with_virtualenv(VirtualEnvConda)
  endif()
endif()
