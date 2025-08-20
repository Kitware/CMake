try_compile(out
  SOURCES ${CMAKE_CURRENT_LIST_DIR}/inspect.cpp
  CXX_STANDARD 20
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake"
  "set(can_build_cxx20_tutorial ${out})"
)
