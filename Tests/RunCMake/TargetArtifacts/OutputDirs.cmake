enable_language(C)

if(CMAKE_IMPORT_LIBRARY_SUFFIX)
  set(expect_dll 1)
else()
  set(expect_dll 0)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/$<IF:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>,rtlib,rtbin>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/$<IF:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>,sharedlib,others>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/$<IF:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,STATIC_LIBRARY>,staticlib,others>")

add_executable(exe_tgt main.c)
add_library(shared_tgt SHARED lib.c)
add_library(static_tgt STATIC lib.c)

add_custom_target(checkDirs ALL
  COMMAND ${CMAKE_COMMAND}
    -Dartifact_path=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
    -Dexe_name=$<TARGET_FILE_NAME:exe_tgt>
    -Dshared_name=$<TARGET_FILE_NAME:shared_tgt>
    -Dstatic_name=$<TARGET_FILE_NAME:static_tgt>
    -Dexpect_dll=${expect_dll}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake
  )

add_dependencies(checkDirs exe_tgt shared_tgt static_tgt)
