cmake_minimum_required(VERSION 3.1)

project(TestPython2Embedded LANGUAGES C)

include(CTest)

find_package(Python2 REQUIRED COMPONENTS Development.Embed)
if (NOT Python2_FOUND)
  message (FATAL_ERROR "Failed to find Python 2")
endif()
if (Python2_Development_FOUND)
  message (FATAL_ERROR "Python 2, COMPONENT 'Development' unexpectedly found")
endif()
if (Python2_Development.Module_FOUND)
  message (FATAL_ERROR "Python 2, COMPONENT 'Development.Module' unexpectedly found")
endif()
if (NOT Python2_Development.Embed_FOUND)
  message (FATAL_ERROR "Python 2, COMPONENT 'Development.Embed' not found")
endif()

if(TARGET Python2::Module)
  message(SEND_ERROR "Python2::Module unexpectedly found")
endif()

if(NOT TARGET Python2::Python)
  message(SEND_ERROR "Python2::Python not found")
endif()

Python2_add_library (display_time2 SHARED ../display_time.c)
set_property (TARGET display_time2 PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions (display_time2 PRIVATE PYTHON2)

add_executable (main2 ../main.c)
target_link_libraries (main2 PRIVATE display_time2)

if (WIN32 OR CYGWIN OR MSYS OR MINGW)
  list (JOIN Python2_RUNTIME_LIBRARY_DIRS "$<SEMICOLON>" RUNTIME_DIRS)
  add_test (NAME Python2.Embedded COMMAND "${CMAKE_COMMAND}" -E env "PATH=${RUNTIME_DIRS}" $<TARGET_FILE:main2>)
else()
  add_test (NAME Python2.Embedded COMMAND main2)
endif()
set_property (TEST Python2.Embedded PROPERTY PASS_REGULAR_EXPRESSION "Today is")
