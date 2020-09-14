
include(CheckSourceRuns)
check_source_runs(C "int main() {return 0;}" SHOULD_BUILD)
