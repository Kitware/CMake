cmake_minimum_required(VERSION 3.1)

project(TestPython3Embedded LANGUAGES C)

include(CTest)

find_package(Python3 REQUIRED COMPONENTS Development.Embed)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()
if (Python3_Development_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development' unexpectedly found")
endif()
if (Python3_Development.Module_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.Module' unexpectedly found")
endif()
if (NOT Python3_Development.Embed_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.Embed' not found")
endif()

if(TARGET Python3::Module)
  message(SEND_ERROR "Python3::Module unexpectedly found")
endif()

if(NOT TARGET Python3::Python)
  message(SEND_ERROR "Python3::Python not found")
endif()

Python3_add_library (display_time3 SHARED ../display_time.c)
set_property (TARGET display_time3 PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions (display_time3 PRIVATE PYTHON3)

add_executable (main3 ../main.c)
target_link_libraries (main3 PRIVATE display_time3)

if (WIN32 OR CYGWIN OR MSYS OR MINGW)
  list (JOIN Python3_RUNTIME_LIBRARY_DIRS "$<SEMICOLON>" RUNTIME_DIRS)
  add_test (NAME Python3.Embedded COMMAND "${CMAKE_COMMAND}" -E env "PATH=${RUNTIME_DIRS}" $<TARGET_FILE:main3>)
else()
  add_test (NAME Python3.Embedded COMMAND main3)
endif()
set_property (TEST Python3.Embedded PROPERTY PASS_REGULAR_EXPRESSION "Today is")
