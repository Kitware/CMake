cmake_policy(SET CMP0141 NEW)
cmake_policy(SET CMP0235 NEW)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
enable_language(C CXX)

# Any damage to the objects' debug info fails the link.
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG:FULL /INCREMENTAL:NO /WX")

add_library(creator STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/creator.c
  ${CMAKE_CURRENT_BINARY_DIR}/creator.cxx
  )
target_precompile_headers(creator PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/pch.h)

add_executable(app
  ${CMAKE_CURRENT_BINARY_DIR}/uses_c.c
  ${CMAKE_CURRENT_BINARY_DIR}/uses_cxx.cxx
  )
target_precompile_headers(app REUSE_FROM creator)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(check_pairs)
set(check_exes
  \"$<TARGET_FILE:app>\"
  )
")
