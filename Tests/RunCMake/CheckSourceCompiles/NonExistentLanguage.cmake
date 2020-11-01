
include(CheckSourceCompiles)
check_source_compiles(FAKE_LANG "int main() {return 0;}" SHOULD_BUILD)
