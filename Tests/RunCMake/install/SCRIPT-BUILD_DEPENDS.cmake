enable_language(C)

add_library(script_dependency STATIC obj1.c)
install(SCRIPT install_script.cmake BUILD_DEPENDS script_dependency)

# Full preparation must omit both unrelated and excluded dependencies.
add_library(unrelated STATIC never-build.c)
add_library(excluded_dependency STATIC never-build.c)
install(CODE "message(STATUS hidden)" BUILD_DEPENDS excluded_dependency
  COMPONENT hidden EXCLUDE_FROM_ALL)
