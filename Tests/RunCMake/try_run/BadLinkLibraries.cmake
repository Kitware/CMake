include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

add_custom_target(not_a_library)

try_run(RUN_RESULT COMPILE_RESULT ${try_compile_bindir_or_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  LINK_LIBRARIES not_a_library)
