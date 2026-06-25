enable_language (C)
include(CheckSourceRuns)

# If CMAKE_REQUIRED_FLAGS is a list, all items but the first are passed as
# arguments to 'cmake'. This was never supported, but arguments that start with
# '-W' were silently ignored by CMake < 4.4. Starting with 4.4, CMake complains
# about unknown warning flags. Therefore, for compatibility, we strip them.
set(CMAKE_REQUIRED_FLAGS -DFOO -Wseen-by-cmake)
check_source_runs(C "int main() {return 0;}" SHOULD_RUN)
