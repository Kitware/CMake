enable_language(C)

install(FILES one.txt two.txt
  DESTINATION .
  BUILD_DEPENDS file_dependency_one
  BUILD_DEPENDS file_dependency_two
  COMPONENT dev)

# Resolve dependency names during generation, after these targets are defined.
add_library(file_dependency_one STATIC obj1.c)
add_library(file_dependency_two STATIC obj1.c)

# Only rules for the selected component and configuration may add dependencies.
add_library(unrelated STATIC never-build.c)
add_library(other_dependency STATIC never-build.c)
install(FILES missing.txt DESTINATION .
  BUILD_DEPENDS other_dependency COMPONENT other)
install(FILES missing.txt DESTINATION .
  BUILD_DEPENDS other_dependency COMPONENT dev CONFIGURATIONS Release)
