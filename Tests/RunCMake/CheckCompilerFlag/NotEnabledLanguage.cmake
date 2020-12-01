
include(CheckCompilerFlag)
check_compiler_flag(C "int main() {return 0;}" SHOULD_BUILD)
