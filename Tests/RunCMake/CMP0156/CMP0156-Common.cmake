
enable_language(C)

# ensure link is successful in case of circular dependency
add_library(lib1 STATIC lib1.c)
add_library(lib2 STATIC lib2.c)

target_link_libraries(lib1 PRIVATE lib2)
target_link_libraries(lib2 PRIVATE lib1)

add_executable(main main.c)
target_link_libraries(main PRIVATE lib1)
if (APPLE_TEST)
  target_link_options(main PRIVATE "LINKER:-fatal_warnings")
else()
  target_link_options(main PRIVATE "$<$<AND:$<NOT:$<TARGET_POLICY:CMP0156>>,$<C_COMPILER_ID:AppleClang>,$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,15.0>>:LINKER:-no_warn_duplicate_libraries>")
endif()


add_library(lib3 SHARED lib3.c)
add_library(lib4 STATIC lib4.c)
target_link_libraries(lib4 PRIVATE lib3)

# link specifying a shared library not directly used by the target
# on Windows, with CMP0156=NEW, lib3 is specified before lib4 on link step
add_executable(main2 main2.c)
target_link_libraries(main2 PRIVATE lib3 lib4)
