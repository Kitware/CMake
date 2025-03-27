enable_language(C)

include("${CMAKE_BINARY_DIR}/../LinkDependsExternalLibrary-build/ExternalLibrary-debug.cmake")
cmake_path(GET EXTERNAL_LIBRARY PARENT_PATH EXTERNAL_DIR)

add_library(LinkDependsLib SHARED LinkDependsLib.c)
target_link_directories(LinkDependsLib PRIVATE "${EXTERNAL_DIR}")
target_link_libraries(LinkDependsLib PRIVATE External)

add_executable(LinkDependsExe LinkDependsExe.c)
target_link_directories(LinkDependsExe PRIVATE "${EXTERNAL_DIR}")
target_link_libraries(LinkDependsExe PRIVATE External)

set(CODE_WITH_SPACE [[
add_library(LinkDependsLib2 SHARED ../LinkDependsLib.c)
target_link_directories(LinkDependsLib2 PRIVATE "${EXTERNAL_DIR}")
target_link_libraries(LinkDependsLib2 PRIVATE External)

add_executable(LinkDependsExe2 ../LinkDependsExe.c)
target_link_directories(LinkDependsExe2 PRIVATE "${EXTERNAL_DIR}")
target_link_libraries(LinkDependsExe2 PRIVATE External)
]])
if(MAKE_SUPPORTS_SPACES)
  add_subdirectory("With Space")
  set(check_pairs_with_space "
    \"$<TARGET_FILE:LinkDependsLib2>|${EXTERNAL_LIBRARY}\"
    \"$<TARGET_FILE:LinkDependsExe2>|${EXTERNAL_LIBRARY}\"
  ")
endif()

file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake"
  CONTENT "
set(check_pairs
  \"$<TARGET_FILE:LinkDependsLib>|${EXTERNAL_LIBRARY}\"
  \"$<TARGET_FILE:LinkDependsExe>|${EXTERNAL_LIBRARY}\"
  ${check_pairs_with_space}
  )
")
