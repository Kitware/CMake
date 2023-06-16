cmake_minimum_required(VERSION 3.5)

project(TestSOABI LANGUAGES C)

if(CMake_TEST_FindPython3)
  find_package(Python3 COMPONENTS ${CMake_TEST_FindPython_COMPONENT})
  if (NOT Python3_FOUND)
    message (FATAL_ERROR "Failed to find Python 3")
  endif()

  if(NOT DEFINED Python3_SOABI)
    message(FATAL_ERROR "Python3_SOABI for ${CMake_TEST_FindPython_COMPONENT} not found")
  endif()

  if (Python3_Development_FOUND AND Python3_SOABI)
    Python3_add_library (spam3 MODULE WITH_SOABI ../spam.c)
    target_compile_definitions (spam3 PRIVATE PYTHON3)

    get_property (suffix TARGET spam3 PROPERTY SUFFIX)
    if (NOT suffix MATCHES "^.${Python3_SOABI}")
      message(FATAL_ERROR "Module suffix do not include Python3_SOABI")
    endif()
  endif()
endif()

if(CMake_TEST_FindPython2)
  find_package(Python2 COMPONENTS ${CMake_TEST_FindPython_COMPONENT})
  if(NOT DEFINED Python2_SOABI)
    message(FATAL_ERROR "Python2_SOABI for ${CMake_TEST_FindPython_COMPONENT} not found")
  endif()

  if (Python2_Development_FOUND AND Python2_SOABI)
    Python2_add_library (spam2 MODULE WITH_SOABI ../spam.c)
    target_compile_definitions (spam2 PRIVATE PYTHON2)

    get_property (suffix TARGET spam2 PROPERTY SUFFIX)
    if (NOT suffix MATCHES "^.${Python2_SOABI}")
      message(FATAL_ERROR "Module suffix do not include Python2_SOABI")
    endif()
  endif()
endif()
