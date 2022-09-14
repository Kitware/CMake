enable_language(C)

file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/ProjectSearchPath")
file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathInc/TargetInc.framework")
file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathLib/TargetLib.framework")
file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathCMP0142OLD")
file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathCMP0142NEW")

set(CMAKE_XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${CMAKE_CURRENT_BINARY_DIR}/ProjectSearchPath")
set(CMAKE_XCODE_ATTRIBUTE_LIBRARY_SEARCH_PATHS "${CMAKE_CURRENT_BINARY_DIR}/ProjectSearchPath")

add_executable(neither main.c)

add_executable(both main.c)
target_include_directories(both PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathInc/TargetInc.framework")
target_link_libraries(both PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathLib/TargetLib.framework")

add_executable(include main.c)
target_include_directories(include PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathInc/TargetInc.framework")

add_executable(library main.c)
target_link_libraries(library PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathLib/TargetLib.framework")
target_link_directories(library PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathLib")

cmake_policy(SET CMP0142 OLD)
add_executable(cmp0142old main.c)
target_link_directories(cmp0142old PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathCMP0142OLD")

cmake_policy(SET CMP0142 NEW)
add_executable(cmp0142new main.c)
target_link_directories(cmp0142new PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathCMP0142NEW")
