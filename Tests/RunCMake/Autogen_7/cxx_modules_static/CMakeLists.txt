cmake_minimum_required(VERSION 3.28)
project(cxx_modules_static_automoc CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

if(APPLE)
  # Archive with libtool, which warns about members without global symbols.
  get_filename_component(compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
  find_program(LIBTOOL libtool HINTS "${compiler_dir}" /usr/bin REQUIRED)
  set(CMAKE_CXX_CREATE_STATIC_LIBRARY
    "\"${LIBTOOL}\" -static -o <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

set(src ../cxx_modules)

# mod-empty.cppm has no meta-object macro, so its moc output is a
# placeholder that is archived into the library.
add_library(lib STATIC ${src}/mod-impl.cpp)
target_sources(lib
  PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${src} FILES
    ${src}/mod.cppm
    ${src}/mod-part.cppm
    ${src}/mod-internal.cppm
    ${src}/mod-empty.cppm)

set_target_properties(lib PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  AUTOMOC ON)

target_link_libraries(lib PUBLIC Qt${with_qt_version}::Core)

add_executable(app ${src}/main.cpp)
set_target_properties(app PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON)
target_link_libraries(app PRIVATE lib)
