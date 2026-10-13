enable_language(C)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_library(runtime_dependencies_root MODULE EXCLUDE_FROM_ALL lib.c)
  set(runtime_component RuntimeDependencies)
  set(library_component RuntimeDependenciesSeed)
else()
  add_executable(runtime_dependencies_root EXCLUDE_FROM_ALL runtime_main.c)
  set(runtime_component RuntimeDependenciesSeed)
  set(library_component RuntimeDependencies)
endif()

install(TARGETS runtime_dependencies_root
  RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES ".*"
  RUNTIME DESTINATION bin
    COMPONENT ${runtime_component}
  LIBRARY DESTINATION lib
    COMPONENT ${library_component}
  FRAMEWORK DESTINATION Frameworks
    COMPONENT RuntimeDependencies
)

add_library(unit_tests STATIC neverbuild.c)
