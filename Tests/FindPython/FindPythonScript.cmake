
if (PYTHON_MUST_NOT_BE_FOUND)
  find_package(${PYTHON_PACKAGE_NAME} QUIET)
  if (${PYTHON_PACKAGE_NAME}_FOUND)
    message(FATAL_ERROR "${PYTHON_PACKAGE_NAME}: unexpectedly founded.")
  endif()
else()
  find_package(${PYTHON_PACKAGE_NAME} REQUIRED QUIET)
endif()
