source .gitlab/ci/bullseye-env.sh

# Store Bullseye activation state in the work directory.
export COVAPPDATADIR="$CI_PROJECT_DIR/build/Bullseye"

# Collect all coverage in a single location.
export COVFILE="$COVAPPDATADIR/CMake.cov"

# Suppress the Bullseye startup banner on compilation.
export COVCOPT="--no-banner"
