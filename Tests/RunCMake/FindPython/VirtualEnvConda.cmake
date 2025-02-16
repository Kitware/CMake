
if(DEFINED ENV{CONDA_EXE})
  set(CONDA_EXECUTABLE "$ENV{CONDA_EXE}")
else()
  find_program(CONDA_EXECUTABLE conda NO_CACHE)
  if (NOT CONDA_EXECUTABLE)
    message (FATAL_ERROR "Failed to find Conda")
  endif()
endif()

execute_process (COMMAND "${CONDA_EXECUTABLE}" create --no-default-packages --prefix "${PYTHON3_VIRTUAL_ENV}" --yes python=3
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE outputs
                 ERROR_VARIABLE outputs)
if (result)
  message (FATAL_ERROR "Fail to create virtual environment: ${outputs}")
endif()
