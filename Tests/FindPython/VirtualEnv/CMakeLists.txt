cmake_minimum_required(VERSION 3.5)

project(TestVirtualEnv LANGUAGES NONE)

include(CTest)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()

set (Python3_VIRTUAL_ENV "${CMAKE_CURRENT_BINARY_DIR}/py3venv")
file (REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/py3venv")

execute_process (COMMAND "${Python3_EXECUTABLE}" -m venv "${Python3_VIRTUAL_ENV}"
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE outputs
                 ERROR_VARIABLE outputs)
if (result)
  message (FATAL_ERROR "Fail to create virtual environment: ${outputs}")
endif()

add_test(NAME FindPython3.VirtualEnvDefault
         COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                           --unset=CONDA_PREFIX
                                           "VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                 "${CMAKE_COMMAND}" "-DPYTHON3_VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvDefault.cmake")

add_test(NAME FindPython3.VirtualEnvStandard
         COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                           --unset=CONDA_PREFIX
                                           "VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                 "${CMAKE_COMMAND}" "-DPYTHON3_VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvStandard.cmake")

if(CMake_TEST_FindPython2)
  add_test(NAME FindPython3.VirtualEnvOnly
           COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                             --unset=CONDA_PREFIX
                                             "VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                   "${CMAKE_COMMAND}" "-DPYTHON3_VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                   -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvOnly.cmake")
  add_test(NAME FindPython3.UnsetVirtualEnvOnly
           COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                             --unset=VIRTUAL_ENV
                                             --unset=CONDA_PREFIX
                   "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvOnly.cmake")
endif()
