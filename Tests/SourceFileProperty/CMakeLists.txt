cmake_minimum_required(VERSION 3.1)
project(SourceFileProperty C)

if (EXISTS icasetest.c)
  # If a file exists by this name, use it.
  set_source_files_properties(icasetest.c
    PROPERTIES
      COMPILE_DEFINITIONS NEEDED_TO_WORK)
else ()
  # Work on case-sensitive file systems as well.
  set_source_files_properties(main.c
    PROPERTIES
      COMPILE_DEFINITIONS NO_NEED_TO_CALL)
endif ()

add_executable(SourceFileProperty main.c)
target_sources(SourceFileProperty PRIVATE ICaseTest.c)

get_source_file_property(LANG_MAIN main.c LANGUAGE)
if(NOT "${LANG_MAIN}" STREQUAL "C")
  message(FATAL_ERROR "Bad language for file main.c")
endif()

get_property(LANG_TEST SOURCE ICaseTest.c PROPERTY LANGUAGE)
if (NOT "${LANG_TEST}" STREQUAL "C")
  message(FATAL_ERROR "Bad language for file ICaseTest.c")
endif ()
