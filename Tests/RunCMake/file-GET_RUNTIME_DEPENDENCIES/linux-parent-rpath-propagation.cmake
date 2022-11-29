enable_language(C)
cmake_policy(SET CMP0095 NEW)

# Force linker to set RPATH instead of RUNPATH
add_link_options("-Wl,--disable-new-dtags")

# bin/exe (RPATH = "lib1:lib2:lib3")
# ^
# |
# lib1/libone.so (RPATH erased)
# ^
# |
# lib2/libtwo.so (RPATH erased)
# ^
# |
# lib3/libthree.so (RPATH erased)
# GET_RUNTIME_DEPENDENCIES(bin/exe) should resolve all three libraries

set(TEST_SOURCE_DIR "linux/parent-rpath-propagation")

add_library(three SHARED "${TEST_SOURCE_DIR}/three.c")

add_library(two SHARED "${TEST_SOURCE_DIR}/two.c")
target_link_libraries(two PUBLIC three)

add_library(one SHARED "${TEST_SOURCE_DIR}/one.c")
target_link_libraries(one PUBLIC two)

add_executable(exe "${TEST_SOURCE_DIR}/main.c")
target_link_libraries(exe PUBLIC one)

set_property(TARGET exe PROPERTY INSTALL_RPATH
  $ORIGIN/../lib1
  $ORIGIN/../lib2
  $ORIGIN/../lib3
)

install(TARGETS exe DESTINATION bin)
install(TARGETS one DESTINATION lib1)
install(TARGETS two DESTINATION lib2)
install(TARGETS three DESTINATION lib3)

install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:exe>"
    PRE_INCLUDE_REGEXES
      "^lib(one|two|three)\\.so$"
      "^libc\\.so"
    PRE_EXCLUDE_REGEXES ".*"
    POST_INCLUDE_REGEXES "^.*/lib(one|two|three)\\.so$"
    POST_EXCLUDE_REGEXES ".*"
    )
  ]])
