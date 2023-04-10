cmake_minimum_required(VERSION 3.16)
project(MocInterfaceMacroNames)

include("../AutogenCoreTest.cmake")

set(CMAKE_AUTOMOC ON)

add_executable(dummy dummy.cpp)
target_link_libraries(dummy PRIVATE static_lib interface_lib shared_lib)

add_library(shared_lib SHARED shared_lib.cpp)
set_target_properties(shared_lib PROPERTIES INTERFACE_AUTOMOC_MACRO_NAMES "SHARED_LIB_MACRO")

add_library(interface_lib INTERFACE)
set_target_properties(interface_lib PROPERTIES INTERFACE_AUTOMOC_MACRO_NAMES "INTERFACE_LIB_MACRO")

add_library(static_lib STATIC static_lib.cpp)
set_target_properties(static_lib PROPERTIES INTERFACE_AUTOMOC_MACRO_NAMES "STATIC_LIB_MACRO")

set(AUTOGEN_INFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/dummy_autogen.dir/AutogenInfo.json")
set(CHECK_AUTOGEN_JSON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CheckAutogenJson.cmake")
message(STATUS "AutogenInfo.json: ${AUTOGEN_INFO_FILE}")

add_custom_command(TARGET dummy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DFILE_PATH=${AUTOGEN_INFO_FILE} -P ${CHECK_AUTOGEN_JSON_PATH}
)

install(TARGETS shared_lib EXPORT shared_lib)
install(TARGETS interface_lib EXPORT interface_lib)
install(TARGETS static_lib EXPORT static_lib)

install(EXPORT shared_lib FILE shared_libTargets.cmake DESTINATION lib/cmake/shared_lib)
install(EXPORT interface_lib FILE interface_libTargets.cmake DESTINATION lib/cmake/interface_lib)
install(EXPORT static_lib FILE static_libTargets.cmake DESTINATION lib/cmake/static_lib)

set(CHECK_EXPORT_TARGETS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CheckExportTargets.cmake")
set(EXPORT_FOLDER_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Export")

add_custom_command(TARGET dummy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DFOLDER_PATH=${EXPORT_FOLDER_PATH} -P ${CHECK_EXPORT_TARGETS_PATH}
)

# check if INTERFACE_AUTOMOC_MACRO_NAMES were transferred to the *_link libraries correctly
add_executable(dummy_link dummy.cpp)
target_link_libraries(dummy_link PRIVATE static_link_lib interface_link_lib shared_link_lib)

add_library(shared_link_lib SHARED shared_lib.cpp)
target_link_libraries(shared_link_lib PUBLIC shared_lib)

add_library(interface_link_lib INTERFACE)
target_link_libraries(interface_link_lib INTERFACE interface_lib)

add_library(static_link_lib STATIC static_lib.cpp)
target_link_libraries(static_link_lib PUBLIC static_lib)

set(AUTOGEN_INFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/dummy_link_autogen.dir/AutogenInfo.json")
message(STATUS "AutogenInfo.json: ${AUTOGEN_INFO_FILE}")

add_custom_command(TARGET dummy_link POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DFILE_PATH=${AUTOGEN_INFO_FILE} -P ${CHECK_AUTOGEN_JSON_PATH}
)
