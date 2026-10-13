enable_language(C)

install(DIRECTORY dir/
  DESTINATION .
  BUILD_DEPENDS dir_dependency_one
  BUILD_DEPENDS dir_dependency_two
  COMPONENT dev)

# Resolve dependency names during generation, after these targets are defined.
add_library(dir_dependency_one STATIC obj1.c)
add_library(dir_dependency_two STATIC obj1.c)

# Only rules for the selected component and configuration may add dependencies.
add_library(unrelated STATIC never-build.c)
add_library(other_dependency STATIC never-build.c)
install(DIRECTORY missing/ DESTINATION . OPTIONAL
  BUILD_DEPENDS other_dependency COMPONENT other)
install(DIRECTORY missing/ DESTINATION . OPTIONAL
  BUILD_DEPENDS other_dependency COMPONENT dev CONFIGURATIONS Release)
