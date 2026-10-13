enable_language(C)

add_executable(runtime_dependency_framework_root EXCLUDE_FROM_ALL
  runtime_main.c)
install(TARGETS runtime_dependency_framework_root
  RUNTIME_DEPENDENCY_SET runtime_dependency_framework_set
  RUNTIME DESTINATION bin
    COMPONENT RuntimeDependencySeed
)
install(RUNTIME_DEPENDENCY_SET runtime_dependency_framework_set
  PRE_EXCLUDE_REGEXES ".*"
  RUNTIME DESTINATION bin
    COMPONENT RuntimeDependencies
  LIBRARY DESTINATION lib
    COMPONENT RuntimeDependencies
  FRAMEWORK DESTINATION Frameworks
    COMPONENT FrameworkDependencies
)

add_library(unit_tests STATIC neverbuild.c)
