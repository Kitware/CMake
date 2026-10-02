enable_language(C)

# Use different dummy RPATHs and disable binary RPATH editing to force the
# Makefile generator's relink-before-install path.
set(CMAKE_NO_BUILTIN_CHRPATH 1)

add_executable(relink EXCLUDE_FROM_ALL runtime_main.c)
set_target_properties(relink PROPERTIES
  BUILD_RPATH foo
  INSTALL_RPATH bar
)
install(TARGETS relink
  RUNTIME DESTINATION bin
  COMPONENT Relink
)

add_library(unit_tests STATIC neverbuild.c)
