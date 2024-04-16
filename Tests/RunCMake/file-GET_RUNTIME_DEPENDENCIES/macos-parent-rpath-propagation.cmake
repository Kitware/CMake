enable_language(C)

# bin/exe (RPATH = "lib1:lib2:lib3")
# ^
# |
# lib1/libone.dylib (RPATH erased)
# ^
# |
# lib2/libtwo.dylib (RPATH erased)
# ^
# |
# lib3/libthree.dylib (RPATH erased)
# GET_RUNTIME_DEPENDENCIES(bin/exe) should resolve all three libraries

set(TEST_SOURCE_DIR "macos/parent-rpath-propagation")

add_library(three SHARED "${TEST_SOURCE_DIR}/three.c")

add_library(two SHARED "${TEST_SOURCE_DIR}/two.c")
target_link_libraries(two PRIVATE three)

add_library(one SHARED "${TEST_SOURCE_DIR}/one.c")
target_link_libraries(one PRIVATE two)

add_executable(exe "${TEST_SOURCE_DIR}/main.c")
target_link_libraries(exe PUBLIC one)

set_property(TARGET exe PROPERTY INSTALL_RPATH
  @loader_path/../lib1
  @loader_path/../lib2
  @loader_path/../lib3
)

install(TARGETS exe DESTINATION bin)
install(TARGETS one DESTINATION lib1)
install(TARGETS two DESTINATION lib2)
install(TARGETS three DESTINATION lib3)

install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:exe>"
    )
  ]])
