
include(CheckSourceCompiles)
check_source_compiles(C "int main() {return 0;}" SHOULD_BUILD)
