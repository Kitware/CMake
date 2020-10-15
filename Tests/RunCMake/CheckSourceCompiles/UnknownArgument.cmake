
enable_language (C)
include(CheckSourceCompiles)

check_source_compiles(C "int main() {return 0;}" SHOULD_BUILD SRC_EXT C BAD)
