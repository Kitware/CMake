enable_language(C)

add_library(alias_dependency STATIC obj1.c)
add_library(Upstream::alias_dependency ALIAS alias_dependency)

install(FILES one.txt DESTINATION .
  BUILD_DEPENDS Upstream::alias_dependency
  COMPONENT dev)

add_library(unrelated STATIC never-build.c)
