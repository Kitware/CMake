
enable_language (C)
include(CheckSourceRuns)

check_source_runs(C "int main() {return 0;}" SHOULD_BUILD SRC_EXT C BAD)
