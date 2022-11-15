. .gitlab/ci/openmp-env.sh
if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/ispc-env.sh
fi
