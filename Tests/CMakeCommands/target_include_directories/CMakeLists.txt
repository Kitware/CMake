cmake_minimum_required(VERSION 2.8)

project(target_include_directories)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/privateinclude")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/privateinclude/privateinclude.h" "#define PRIVATEINCLUDE_DEFINE\n")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/publicinclude")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/publicinclude/publicinclude.h" "#define PUBLICINCLUDE_DEFINE\n")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/interfaceinclude")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/interfaceinclude/interfaceinclude.h" "#define INTERFACEINCLUDE_DEFINE\n")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/poison")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/poison/common.h" "#error Should not be included\n")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cure")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cure/common.h" "#define CURE_DEFINE\n")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/linkedinclude")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/linkedinclude/linkedinclude.h" "#define LINKEDINCLUDE_DEFINE\n")

add_executable(target_include_directories
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_include_directories(target_include_directories
  PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/privateinclude"
  PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/publicinclude"
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/interfaceinclude"
)

target_include_directories(target_include_directories
  PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/poison"
)
target_include_directories(target_include_directories
  BEFORE PUBLIC "$<$<TARGET_DEFINED:target_include_directories>:${CMAKE_CURRENT_BINARY_DIR}/cure>"
)

# Has no effect because the target is not defined:
target_include_directories(target_include_directories
  BEFORE PUBLIC "$<$<TARGET_DEFINED:notdefined>:${CMAKE_CURRENT_BINARY_DIR}/poison>"
)

add_executable(consumer
  "${CMAKE_CURRENT_SOURCE_DIR}/consumer.cpp"
)

add_library(linked UNKNOWN IMPORTED)
set_property(TARGET linked PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/linkedinclude")

target_include_directories(consumer
  PRIVATE
    $<TARGET_PROPERTY:target_include_directories,INTERFACE_INCLUDE_DIRECTORIES>
    $<LINKED:linked>
  relative_dir
)
