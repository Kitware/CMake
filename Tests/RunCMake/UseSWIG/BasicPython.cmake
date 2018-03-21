
set(language "python")

include (BasicConfiguration.cmake)

add_custom_target (RunTest
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:example>"
  "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runme.py"
  DEPENDS example)
