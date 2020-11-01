
enable_language (C)
include(CheckCSourceRuns)
include(CheckSourceRuns)

check_c_source_runs("int main() {return 0;}" C_SHOULD_BUILD BAD)
check_source_runs(C "int main() {return 0;}" SHOULD_BUILD SRC_EXT c BAD)
