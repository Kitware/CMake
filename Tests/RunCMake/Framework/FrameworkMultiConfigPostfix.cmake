enable_language(C)

set(CMAKE_FRAMEWORK_MULTI_CONFIG_POSTFIX_DEBUG "_debug")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/lib)

set(target_name "mylib")
add_library(${target_name} SHARED foo.c)
set_property(TARGET ${target_name} PROPERTY FRAMEWORK ON)
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

string(APPEND content
       "set(is_multi_config \"${is_multi_config}\")\n"
       "set(framework_dir \"$<TARGET_BUNDLE_DIR:${target_name}>\")\n"
       "set(target_file_name ${target_name})\n")
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/FrameworkMultiConfigPostfixInfo.cmake
     CONTENT "${content}")


# Try to link this framework to ensure postfix is correctly handled
add_library(otherlib SHARED foo.c)
target_link_libraries(otherlib PRIVATE ${target_name})
