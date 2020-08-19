enable_language(C)

add_executable(Win32GenEx main.c)
set_target_properties(Win32GenEx PROPERTIES
  WIN32_EXECUTABLE $<CONFIG:Release>
  )
target_compile_definitions(Win32GenEx PRIVATE $<$<CONFIG:Release>:USE_WIN32_MAIN>)
