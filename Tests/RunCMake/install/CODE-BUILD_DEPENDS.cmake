enable_language(C)

add_library(code_dependency_one STATIC obj1.c)
add_library(code_dependency_two STATIC obj1.c)

install(CODE [[message(STATUS "CODE-BUILD_DEPENDS executed")]]
  BUILD_DEPENDS code_dependency_one
  BUILD_DEPENDS code_dependency_two
  COMPONENT dev)

add_library(unrelated STATIC never-build.c)
add_library(other_dependency STATIC never-build.c)
install(CODE "message(STATUS other)" BUILD_DEPENDS other_dependency COMPONENT other)
