enable_language(C)

function(change_build_type_subdir)
  set(CMAKE_BUILD_TYPE Release)
  add_subdirectory(SubDirPrefix)
endfunction()

change_build_type_subdir()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(hello hello_sub_greeting.c)
target_link_libraries(hello greeting)
