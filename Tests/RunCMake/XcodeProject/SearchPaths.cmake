enable_language(C)

file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/ProjectSearchPath")
file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathInc/TargetInc.framework")
file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/TargetSearchPathLib/TargetLib.framework")

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
