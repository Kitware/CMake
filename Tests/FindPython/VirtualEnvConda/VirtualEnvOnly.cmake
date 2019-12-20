
#
# Virtual environment is defined for python3
# Trying to find a python2 using only virtual environment
# It is expecting to fail if a virtual environment is active and to success otherwise.
#
set (Python2_FIND_VIRTUALENV ONLY)
find_package (Python2 QUIET)

if (PYTHON3_VIRTUAL_ENV AND Python2_FOUND)
  message (FATAL_ERROR "Python2 unexpectedly found.")
endif()

if (NOT PYTHON3_VIRTUAL_ENV AND NOT Python2_FOUND)
  message (FATAL_ERROR "Fail to find Python2.")
endif()
