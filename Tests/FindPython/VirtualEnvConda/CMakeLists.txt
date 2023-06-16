cmake_minimum_required(VERSION 3.5)

project(TestVirtualEnvConda LANGUAGES NONE)

include(CTest)

find_program(CONDA_EXECUTABLE conda)
if (CONDA_EXECUTABLE EQUAL NOTFOUND)
  message (FATAL_ERROR "Failed to find Conda")
endif()

set (Python3_VIRTUAL_ENV "${CMAKE_CURRENT_BINARY_DIR}/condaenv")

execute_process (COMMAND "${CONDA_EXECUTABLE}" create --no-default-packages --prefix "${Python3_VIRTUAL_ENV}" --yes python=3
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE outputs
                 ERROR_VARIABLE outputs)
if (result)
  message (FATAL_ERROR "Fail to create virtual environment: ${outputs}")
endif()

add_test(NAME FindPython3.VirtualEnvDefaultConda
         COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                           --unset=VIRTUAL_ENV
                                           "CONDA_PREFIX=${Python3_VIRTUAL_ENV}"
                 "${CMAKE_COMMAND}" "-DPYTHON3_VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvDefault.cmake")

add_test(NAME FindPython3.VirtualEnvStandardConda
         COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                           --unset=VIRTUAL_ENV
                                           "CONDA_PREFIX=${Python3_VIRTUAL_ENV}"
                 "${CMAKE_COMMAND}" "-DPYTHON3_VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvStandard.cmake")

if(Cmake_TEST_FindPython2)
  add_test(NAME FindPython3.VirtualEnvOnlyConda
           COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                             --unset=VIRTUAL_ENV
                                             "CONDA_PREFIX=${Python3_VIRTUAL_ENV}"
                   "${CMAKE_COMMAND}" "-DPYTHON3_VIRTUAL_ENV=${Python3_VIRTUAL_ENV}"
                   -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvOnly.cmake")
  add_test(NAME FindPython3.UnsetVirtualEnvOnlyConda
           COMMAND "${CMAKE_COMMAND}" -E env --unset=PYTHONHOME
                                             --unset=CONDA_PREFIX
                                             --unset=VIRTUAL_ENV
                   "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/VirtualEnvOnly.cmake")
endif()
