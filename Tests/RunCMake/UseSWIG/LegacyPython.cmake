
set(language "python")

include (LegacyConfiguration.cmake)

add_custom_target (RunTest
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:${SWIG_MODULE_example_REAL_NAME}>"
  "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runme.py"
  DEPENDS ${SWIG_MODULE_example_REAL_NAME})
