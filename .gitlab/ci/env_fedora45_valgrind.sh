source .gitlab/ci/env_fedora45_ninja_common.sh

# Shrink stress tests when running with valgrind.
export KWSYS_TEST_PROCESS_1_COUNT=11
