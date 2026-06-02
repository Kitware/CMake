cmake_policy(SET CMP0219 OLD)

enable_language(C)
include(CheckCSourceCompiles)
check_c_source_compiles("int main() { return '\\0'; }" RESULT)
