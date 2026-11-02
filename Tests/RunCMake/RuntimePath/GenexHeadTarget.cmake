enable_language(C)

# Both forms must evaluate target-dependent expressions with the owning target.
# A static dependency keeps CMake from adding automatic runtime path entries.
add_library(A STATIC A.c)
foreach(name IN ITEMS first second)
  add_executable(${name} main.c)
  target_link_libraries(${name} PRIVATE A)
  set_target_properties(${name} PROPERTIES
    BUILD_RPATH "/cmake/build/$<TARGET_PROPERTY:NAME>/$<CONFIG>"
    INSTALL_RPATH "/cmake/install/$<TARGET_PROPERTY:NAME>/$<CONFIG>"
    )
  install(TARGETS ${name} DESTINATION bin)
endforeach()

if(APPLE)
  find_program(otool NAMES otool REQUIRED)
endif()

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/paths-$<CONFIG>.cmake"
  CONTENT "set(build_files \"$<TARGET_FILE:first>;$<TARGET_FILE:second>\")
set(format \"${CMAKE_EXECUTABLE_FORMAT}\")
set(otool \"${otool}\")
")
