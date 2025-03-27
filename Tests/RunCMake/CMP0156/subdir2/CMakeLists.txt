
find_package_lib1()

# -----------------------------------
# This target depends on the third-party lib1 both
# directly and transitively.
add_executable(main ../main.c)
target_link_libraries(main PRIVATE lib1::lib1 lib2)

if (APPLE_TEST)
  target_link_options(main PRIVATE "LINKER:-fatal_warnings")
else()
  target_link_options(main PRIVATE "$<$<AND:$<NOT:$<TARGET_POLICY:CMP0156>>,$<C_COMPILER_ID:AppleClang>,$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,15.0>>:LINKER:-no_warn_duplicate_libraries>")
endif()
