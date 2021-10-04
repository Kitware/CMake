enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

# Detect `-NOTFOUND` libraries at generate time.
cmake_policy(SET CMP0111 NEW)

add_executable(imported::executable IMPORTED)
add_library(imported::shared SHARED IMPORTED)
add_library(imported::static STATIC IMPORTED)
add_library(imported::unknown UNKNOWN IMPORTED)
add_library(imported::interface INTERFACE IMPORTED)
add_library(imported::module MODULE IMPORTED)

function (set_location target name loc)
  set_property(TARGET "imported::${target}" PROPERTY
    "IMPORTED_${name}" "${loc}")
endfunction ()

set(CMAKE_AUTOMOC 1)

add_library(automoc
  empty.cpp)
target_link_libraries(automoc
  PRIVATE
    imported::shared
    imported::static
    imported::unknown
    imported::interface)
add_dependencies(automoc
  imported::executable
  imported::module)
