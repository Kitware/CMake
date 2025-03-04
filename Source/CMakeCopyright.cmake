# CMake license file and copyright line.
set(CMake_LICENSE_FILE "${CMake_SOURCE_DIR}/LICENSE.rst")
file(STRINGS "${CMake_LICENSE_FILE}" CMake_COPYRIGHT_LINE LIMIT_COUNT 1 REGEX "^Copyright ")
if(CMake_COPYRIGHT_LINE MATCHES [[^(Copyright 2000-2[0-9][0-9][0-9] Kitware, Inc\. and )`(Contributors) <CONTRIBUTORS.rst>`_$]])
  set(CMake_COPYRIGHT_LINE "${CMAKE_MATCH_1}${CMAKE_MATCH_2}")
else()
  message(FATAL_ERROR
    "The CMake license file:\n"
    "  ${CMake_LICENSE_FILE}\n"
    "does not contain a copyright line matching the expected pattern."
    )
endif()
