cmake_minimum_required(VERSION 3.31)

project(TestArtifactsPrefix C)

set(Python_ARTIFACTS_PREFIX "_V2")
find_package (Python 2 EXACT REQUIRED)

if(NOT Python_V2_FOUND OR NOT Python_FOUND)
  message(FATAL_ERROR "Python v2 interpreter not found.")
endif()
if(NOT Python_V2_VERSION_MAJOR VERSION_EQUAL 2)
  message(FATAL_ERROR "Python v2 interpreter: wrong major version.")
endif()


set(Python_ARTIFACTS_PREFIX "_V3")
find_package (Python 3 EXACT REQUIRED)

if(NOT Python_V3_FOUND OR NOT Python_FOUND)
  message(FATAL_ERROR "Python v3 interpreter not found.")
endif()
if(NOT Python_V3_VERSION_MAJOR VERSION_EQUAL 3)
  message(FATAL_ERROR "Python v3 interpreter: wrong major version.")
endif()
