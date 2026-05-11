set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

project(unitybuild_skip C)

set(srcs "")
foreach(s RANGE 0 9)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(fileset SHARED)
set_target_properties(fileset PROPERTIES UNITY_BUILD ON)

target_sources(fileset PUBLIC FILE_SET s0 TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s0.c)

target_sources(fileset PUBLIC FILE_SET s1 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s1.c)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/s1.c PROPERTY HEADER_FILE_ONLY ON)


target_sources(fileset PUBLIC FILE_SET s2 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s2.c)
set_property(FILE_SET s2 TARGET fileset PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)

target_sources(fileset PUBLIC FILE_SET s3 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s3.c)
set_property(FILE_SET s3 TARGET fileset PROPERTY COMPILE_OPTIONS "opt")
set_property(FILE_SET s3 TARGET fileset PROPERTY INTERFACE_COMPILE_OPTIONS "opt")

target_sources(fileset PUBLIC FILE_SET s4 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s4.c)
set_property(FILE_SET s4 TARGET fileset PROPERTY COMPILE_DEFINITIONS "def")
set_property(FILE_SET s4 TARGET fileset PROPERTY INTERFACE_COMPILE_DEFINITIONS "def")

target_sources(fileset PUBLIC FILE_SET s5 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s5.c)
set_property(FILE_SET s5 TARGET fileset PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")
set_property(FILE_SET s5 TARGET fileset PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")


target_sources(fileset PUBLIC FILE_SET s6 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s6.c)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/s6.c PROPERTY COMPILE_OPTIONS "opt")

target_sources(fileset PUBLIC FILE_SET s7 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s7.c)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/s7.c PROPERTY COMPILE_DEFINITIONS "def")

target_sources(fileset PUBLIC FILE_SET s8 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s8.c)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/s8.c PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")


target_sources(fileset PUBLIC FILE_SET s9 TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                                       FILES ${CMAKE_CURRENT_BINARY_DIR}/s9.c)


add_library(fileset2 SHARED)
set_target_properties(fileset2 PROPERTIES UNITY_BUILD ON)
target_link_libraries(fileset2 PRIVATE fileset)
