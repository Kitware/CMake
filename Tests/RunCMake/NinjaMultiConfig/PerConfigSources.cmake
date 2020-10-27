enable_language(C)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/main_$<CONFIG>.c" CONTENT
[[int main(void)
{
  return 0;
}
]])
add_executable(exe "${CMAKE_BINARY_DIR}/main_$<CONFIG>.c")
