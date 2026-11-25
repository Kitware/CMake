source .gitlab/ci/bullseye-env.sh

# Store Bullseye activation state in the work directory.
export COVAPPDATADIR="$CI_PROJECT_DIR/build/Bullseye"

# Collect all coverage in a single location.
export COVFILE="$COVAPPDATADIR/CMake.cov"

# Suppress the Bullseye startup banner on compilation.
export COVCOPT="--no-banner"

# Shrink stress tests when running with Bullseye.
export KWSYS_TEST_PROCESS_1_COUNT=11
