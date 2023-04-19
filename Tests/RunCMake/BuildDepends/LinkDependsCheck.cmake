
enable_language(C)

file(WRITE "${CMAKE_BINARY_DIR}/LinkDependsUseLinker.cmake"
     "set(CMAKE_C_LINK_DEPENDS_USE_LINKER \"${CMAKE_C_LINK_DEPENDS_USE_LINKER}\")\n")


file(GENERATE  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake"
  CONTENT "
# no required actions
")
