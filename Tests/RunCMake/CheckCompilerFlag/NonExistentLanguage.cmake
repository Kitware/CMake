
include(CheckCompilerFlag)
check_compiler_flag(FAKE_LANG "int main() {return 0;}" SHOULD_BUILD)
