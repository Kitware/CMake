enable_language(C)

add_library(program_dependency STATIC obj1.c)
install(PROGRAMS program.sh
  DESTINATION . BUILD_DEPENDS program_dependency COMPONENT tools)

add_library(unrelated STATIC never-build.c)
add_library(excluded_dependency STATIC never-build.c)
install(PROGRAMS missing.sh DESTINATION .
  BUILD_DEPENDS excluded_dependency COMPONENT hidden EXCLUDE_FROM_ALL)
