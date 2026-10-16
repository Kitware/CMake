try_compile(out_cxx20
  SOURCES ${CMAKE_CURRENT_LIST_DIR}/inspect_cxx20.cpp
  CXX_STANDARD 20
)

try_compile(out_sse2
  SOURCES ${CMAKE_CURRENT_LIST_DIR}/inspect_sse2.cpp
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake"
  "set(can_build_cxx20_tutorial ${out_cxx20})\n"
  "set(can_build_sse2_tutorial ${out_sse2})\n"
)
