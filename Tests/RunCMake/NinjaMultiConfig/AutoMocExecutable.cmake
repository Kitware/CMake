enable_language(C)

add_executable(badmoc badmoc.c)
target_compile_definitions(badmoc PRIVATE "CONFIG=\"$<CONFIG>\"")

add_executable(exe main.c)
set_target_properties(exe PROPERTIES
  AUTOMOC ON
  AUTOMOC_EXECUTABLE $<TARGET_FILE:badmoc>
  )
