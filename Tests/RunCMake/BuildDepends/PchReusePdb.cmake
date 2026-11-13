cmake_policy(SET CMP0141 NEW)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
enable_language(C)

# Any damage to the objects' debug info fails the link.
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG:FULL /INCREMENTAL:NO /WX")

add_library(creator STATIC ${CMAKE_CURRENT_BINARY_DIR}/creator.c)
target_precompile_headers(creator PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/pch.h)

# Created with CMP0235 unset, where every source writes to the copy, so
# none skips the PCH.  A change to creator alone leaves the copy as is.
add_executable(app_old
  ${CMAKE_CURRENT_BINARY_DIR}/uses_pch.c
  ${CMAKE_CURRENT_BINARY_DIR}/main_old.c
  )
target_precompile_headers(app_old REUSE_FROM creator)
target_link_libraries(app_old PRIVATE creator)

cmake_policy(SET CMP0235 NEW)
add_executable(app
  ${CMAKE_CURRENT_BINARY_DIR}/uses_pch.c
  ${CMAKE_CURRENT_BINARY_DIR}/main.c
  )
target_precompile_headers(app REUSE_FROM creator)
target_link_libraries(app PRIVATE creator)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/main.c
  PROPERTY SKIP_PRECOMPILE_HEADERS ON)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(check_pairs)
set(check_exes
  \"$<TARGET_FILE:app>\"
  \"$<TARGET_FILE:app_old>\"
  )
")
