enable_language(C)

add_library(all_components_dependency STATIC obj1.c)
install(CODE [[message(STATUS "CODE-BUILD_DEPENDS-all-components executed")]]
  BUILD_DEPENDS all_components_dependency ALL_COMPONENTS)

install(CODE "message(STATUS dev)" COMPONENT dev)
install(CODE "message(STATUS other)" COMPONENT other)
add_library(unrelated STATIC never-build.c)
