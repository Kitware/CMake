
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()

file (REMOVE_RECURSE "${PYTHON3_VIRTUAL_ENV}")

execute_process (COMMAND "${Python3_EXECUTABLE}" -m venv "${PYTHON3_VIRTUAL_ENV}"
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE outputs
                 ERROR_VARIABLE outputs)
if (result)
  message (FATAL_ERROR "Fail to create virtual environment: ${outputs}")
endif()
