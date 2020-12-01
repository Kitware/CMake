
include(CheckSourceRuns)
check_source_runs(FAKE_LANG "int main() {return 0;}" SHOULD_BUILD)
