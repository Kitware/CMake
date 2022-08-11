
enable_language(C)

set(CMAKE_C_LINK_LIBRARY_USING_load_archive_SUPPORTED TRUE)
if(CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
  set(CMAKE_C_LINK_LIBRARY_USING_load_archive "-force_load <LIB_ITEM>")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  execute_process(COMMAND "${CMAKE_LINKER}" --help
                          OUTPUT_VARIABLE linker_help
                          ERROR_VARIABLE linker_help)
  if(linker_help MATCHES "--push-state" AND linker_help MATCHES "--pop-state")
    set(CMAKE_C_LINK_LIBRARY_USING_load_archive "LINKER:--push-state,--whole-archive"
                                                "<LINK_ITEM>"
                                                "LINKER:--pop-state")
  else()
    set(CMAKE_C_LINK_LIBRARY_USING_load_archive "LINKER:--whole-archive"
                                                "<LINK_ITEM>"
                                                "LINKER:--no-whole-archive")
  endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_C_LINK_LIBRARY_USING_load_archive "/WHOLEARCHIVE:<LIBRARY>")
else()
  # feature not yet supported for the other environments
  set(CMAKE_C_LINK_LIBRARY_USING_load_archive_SUPPORTED FALSE)
endif()

add_library(base STATIC base.c unref.c)
target_compile_definitions(base PUBLIC STATIC_BASE)

add_library(lib SHARED lib.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:load_archive,base>")

add_executable(main main.c)
target_link_libraries(main PRIVATE lib)
