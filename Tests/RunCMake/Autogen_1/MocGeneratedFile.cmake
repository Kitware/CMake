enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)

set(GEN_SRC "class_$<CONFIG>.cpp")
add_custom_command(
  OUTPUT "${GEN_SRC}"
  COMMAND ${CMAKE_COMMAND} -E echo "// cpp src" > "${GEN_SRC}"
  VERBATIM
)

add_library(libgen STATIC ${GEN_SRC})
target_link_libraries(libgen Qt${with_qt_version}::Core)
