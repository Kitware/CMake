
enable_language(C)

add_library(External SHARED "${CMAKE_CURRENT_BINARY_DIR}/external.c")

file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/ExternalLibrary-$<LOWER_CASE:$<CONFIG>>.cmake"
  CONTENT "set(EXTERNAL_LIBRARY \"$<TARGET_LINKER_FILE:External>\")\n")


file(GENERATE  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake"
  CONTENT "
# no required actions
")
